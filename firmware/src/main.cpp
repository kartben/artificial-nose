#include <unistd.h>
int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

#include <Arduino.h>
#include "Config.h"
#include "ConfigurationMode.h"


////////////////////////////////////////////////////////////////////////////////
// Storage

#include <ExtFlashLoader.h>
#include "Storage.h"

static ExtFlashLoader::QSPIFlash Flash_;
static Storage Storage_(Flash_);

////////////////////////////////////////////////////////////////////////////////
// Display

#include <LovyanGFX.hpp>
#include "Display.h"

static LGFX Gfx_;
static Display Display_(Gfx_);

////////////////////////////////////////////////////////////////////////////////
// Network

#include <Network/WiFiManager.h>
#include <Network/TimeManager.h>
#include <Aziot/AziotDps.h>
#include <Aziot/AziotHub.h>
#include <ArduinoJson.h>

static TimeManager TimeManager_;
static std::string HubHost_;
static std::string DeviceId_;
static AziotHub AziotHub_;

static unsigned long TelemetryInterval_ = TELEMETRY_INTERVAL;   // [sec.]

static void ConnectWiFi()
{
    Display_.Printf("Connecting to SSID: %s\n", Storage_.WiFiSSID.c_str());
	WiFiManager wifiManager;
	wifiManager.Connect(Storage_.WiFiSSID.c_str(), Storage_.WiFiPassword.c_str());
	while (!wifiManager.IsConnected())
	{
		Display_.Printf(".");
		delay(500);
	}
	Display_.Printf("Connected\n");
}

static void SyncTimeServer()
{
	Display_.Printf("Synchronize time\n");
	while (!TimeManager_.Update())
	{
		Display_.Printf(".");
		delay(1000);
	}
	Display_.Printf("Synchronized\n");
}

static bool DeviceProvisioning()
{
	Display_.Printf("Device provisioning:\n");
    Display_.Printf(" Id scope = %s\n", Storage_.IdScope.c_str());
    Display_.Printf(" Registration id = %s\n", Storage_.RegistrationId.c_str());

	AziotDps aziotDps;
	aziotDps.SetMqttPacketSize(MQTT_PACKET_SIZE);

    if (aziotDps.RegisterDevice(DPS_GLOBAL_DEVICE_ENDPOINT_HOST, Storage_.IdScope, Storage_.RegistrationId, Storage_.SymmetricKey, MODEL_ID, TimeManager_.GetEpochTime() + TOKEN_LIFESPAN, &HubHost_, &DeviceId_) != 0)
    {
        Display_.Printf("ERROR: RegisterDevice()\n");
		return false;
    }

    Display_.Printf("Device provisioned:\n");
    Display_.Printf(" Hub host = %s\n", HubHost_.c_str());
    Display_.Printf(" Device id = %s\n", DeviceId_.c_str());

    return true;
}

static bool AziotIsConnected()
{
    return AziotHub_.IsConnected();
}

static void AziotDoWork()
{
    static unsigned long connectTime = 0;
    static unsigned long forceDisconnectTime;

    bool repeat;
    do
    {
        repeat = false;

        const auto now = TimeManager_.GetEpochTime();
        if (!AziotHub_.IsConnected())
        {
            if (now >= connectTime)
            {
                Serial.printf("Connecting to Azure IoT Hub...\n");
                if (AziotHub_.Connect(HubHost_, DeviceId_, Storage_.SymmetricKey, MODEL_ID, now + TOKEN_LIFESPAN) != 0)
                {
                    Serial.printf("ERROR: Try again in 5 seconds\n");
                    connectTime = TimeManager_.GetEpochTime() + 5;
                    return;
                }

                Serial.printf("SUCCESS\n");
                forceDisconnectTime = TimeManager_.GetEpochTime() + static_cast<unsigned long>(TOKEN_LIFESPAN * RECONNECT_RATE);

                AziotHub_.RequestTwinDocument("get_twin");
            }
        }
        else
        {
            if (now >= forceDisconnectTime)
            {
                Serial.printf("Disconnect\n");
                AziotHub_.Disconnect();
                connectTime = 0;

                repeat = true;
            }
            else
            {
                AziotHub_.DoWork();
            }
        }
    }
    while (repeat);
}

template <typename T>
static void AziotSendConfirm(const char* requestId, const char* name, T value, int ackCode, int ackVersion)
{
	StaticJsonDocument<JSON_MAX_SIZE> doc;
	doc[name]["value"] = value;
	doc[name]["ac"] = ackCode;
	doc[name]["av"] = ackVersion;

	char json[JSON_MAX_SIZE];
	serializeJson(doc, json);

	AziotHub_.SendTwinPatch(requestId, json);
}

template <typename T>
static bool AziotUpdateWritableProperty(const char* name, T* value, const JsonVariant& desiredVersion, const JsonVariant& desired, const JsonVariant& reported = JsonVariant())
{
    bool ret = false;

    JsonVariant desiredValue = desired[name];
    JsonVariant reportedProperty = reported[name];

	if (!desiredValue.isNull())
    {
        *value = desiredValue.as<T>();
        ret = true;
    }

    if (desiredValue.isNull())
    {
        if (reportedProperty.isNull())
        {
            AziotSendConfirm<T>("init", name, *value, 200, 1);
        }
    }
    else if (reportedProperty.isNull() || desiredVersion.as<int>() != reportedProperty["av"].as<int>())
    {
        AziotSendConfirm<T>("update", name, *value, 200, desiredVersion.as<int>());
    }

    return ret;
}


template <size_t desiredCapacity>
static void AziotSendTelemetry(const StaticJsonDocument<desiredCapacity>& jsonDoc, char* componentName)
{
	char json[jsonDoc.capacity()];
	serializeJson(jsonDoc, json, sizeof(json));

	AziotHub_.SendTelemetry(json, componentName);
}


#include <AceButton.h>
using namespace ace_button;

#include <CircularBuffer.h>
#include <artificial_nose_inference.h>

#include <Multichannel_Gas_GMXXX.h>
#include <Wire.h>
GAS_GMXXX<TwoWire>* gas = new GAS_GMXXX<TwoWire>();

typedef uint32_t (GAS_GMXXX<TwoWire>::*sensorGetFn)();

typedef struct SENSOR_INFO
{
  char* name;
  char* unit;
  std::function<uint32_t()> readFn;
  uint32_t last_val;
} SENSOR_INFO;

SENSOR_INFO sensors[4] = {
    {"NO2",     "ppm", std::bind(&GAS_GMXXX<TwoWire>::measure_NO2,    gas), 0 },
    {"C2H5NH",  "ppm", std::bind(&GAS_GMXXX<TwoWire>::measure_C2H5OH, gas), 0 },
    {"VOC",     "ppm", std::bind(&GAS_GMXXX<TwoWire>::measure_VOC,    gas), 0 },
    {"CO",      "ppm", std::bind(&GAS_GMXXX<TwoWire>::measure_CO,     gas), 0 }
  };
#define NB_SENSORS 4

char title_text[20] = "";

enum MODE
{
  TRAINING,
  INFERENCE
};
enum MODE mode = TRAINING;

enum SCREEN_MODE
{
  SENSORS,
  GRAPH,
  INFERENCE_RESULTS
};
enum SCREEN_MODE screen_mode = GRAPH;

int latest_inference_idx = -1;
float latest_inference_confidence_level = -1.;

// Allocate a buffer for the values we'll read from the gas sensor
CircularBuffer<float, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE> buffer;

uint64_t next_sampling_tick = micros();

#define INITIAL_FAN_STATE LOW

static bool debug_nn = false; // Set this to true to see e.g. features generated
                              // from the raw signal

void
draw_chart();

enum class ButtonId
{
  A = 0,
  B,
  C,
  PRESS
};
static const int ButtonNumber = 4;
static AceButton Buttons[ButtonNumber];



static void ReceivedTwinDocument(const char* json, const char* requestId)
{
	StaticJsonDocument<JSON_MAX_SIZE> doc;
	if (deserializeJson(doc, json)) return;
    
  if (doc["desired"]["$version"].isNull()) return;

    if (AziotUpdateWritableProperty("telemetryInterval", &TelemetryInterval_, doc["desired"]["$version"], doc["desired"], doc["reported"]))
    {
		Serial.printf("telemetryInterval = %d\n", TelemetryInterval_);
    }
}

static void ReceivedTwinDesiredPatch(const char* json, const char* version)
{
	StaticJsonDocument<JSON_MAX_SIZE> doc;
	if (deserializeJson(doc, json)) return;

	if (doc["$version"].isNull()) return;

  if (AziotUpdateWritableProperty("telemetryInterval", &TelemetryInterval_, doc["$version"], doc.as<JsonVariant>()))
  {
    Serial.printf("telemetryInterval = %d\n", TelemetryInterval_);
  }

}

/**
* @brief      Printf function uses vsnprintf and output using Arduino Serial
*
* @param[in]  format     Variable argument list
*/
void ei_printf(const char *format, ...)
{
  static char print_buf[200] = {0};

  va_list args;
  va_start(args, format);
  int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
  va_end(args);

  if (r > 0)
  {
    Serial.write(print_buf);
  }
}

static void ButtonEventHandler(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
  const uint8_t id = button->getId();
  if (ButtonNumber <= id)
    return;

  switch (eventType)
  {
  case AceButton::kEventReleased:
    switch (static_cast<ButtonId>(id))
    {
    case ButtonId::A:
      digitalWrite(D0, HIGH); // Turn fan ON
      break;
    case ButtonId::B:
      digitalWrite(D0, LOW); // Turn fan OFF
      break;
    case ButtonId::PRESS:
      mode = (mode == INFERENCE) ? TRAINING : INFERENCE;
      break;
    }
    break;
  }
}

static void
ButtonInit()
{
  Buttons[static_cast<int>(ButtonId::A)].init(WIO_KEY_A, HIGH, static_cast<uint8_t>(ButtonId::A));
  Buttons[static_cast<int>(ButtonId::B)].init(WIO_KEY_B, HIGH, static_cast<uint8_t>(ButtonId::B));
  Buttons[static_cast<int>(ButtonId::C)].init(WIO_KEY_C, HIGH, static_cast<uint8_t>(ButtonId::C));
  Buttons[static_cast<int>(ButtonId::PRESS)].init(WIO_5S_PRESS, HIGH, static_cast<uint8_t>(ButtonId::PRESS));

  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(ButtonEventHandler);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
}

static void
ButtonDoWork()
{
  for (int i = 0;
       static_cast<size_t>(i) < std::extent<decltype(Buttons)>::value;
       ++i) {
    Buttons[i].check();
  }
}


/**
 * @brief      Arduino setup function
 */
void
setup()
{
  Storage_.Load();

  Serial.begin(115200);

  Display_.Init();
  Display_.SetBrightness(127);

  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);

  pinMode(WIO_5S_PRESS, INPUT_PULLUP);

  delay(2000);

  if (digitalRead(WIO_KEY_A) == LOW &&
      digitalRead(WIO_KEY_B) == LOW &&
      digitalRead(WIO_KEY_C) == LOW   )
  {
      ei_printf("In configuration mode\r\n");
      ConfigurationMode(Storage_);
  }

  ButtonInit();

  pinMode(D0, OUTPUT);
  digitalWrite(D0, INITIAL_FAN_STATE);

  gas->begin(Wire, 0x08); // use the hardware I2C

  ConnectWiFi();
  SyncTimeServer();
  if (!DeviceProvisioning()) abort();

  AziotHub_.SetMqttPacketSize(MQTT_PACKET_SIZE);

  AziotHub_.ReceivedTwinDocumentCallback = ReceivedTwinDocument;
  AziotHub_.ReceivedTwinDesiredPatchCallback = ReceivedTwinDesiredPatch;  
  
}

int fan = 0;

/**
 * @brief      Get data and run inferencing
 *
 * @param[in]  debug  Get debug info if true
 */
void
loop()
{
  ButtonDoWork();

  AziotDoWork();

  if (mode == TRAINING)
  {
    strcpy(title_text, "Training mode");
  }

  uint64_t new_sampling_tick = -1;
  if (micros() > next_sampling_tick) {
    new_sampling_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);
    next_sampling_tick = new_sampling_tick;
  }
  for (int i = NB_SENSORS - 1; i >= 0; i--) {
    uint32_t sensorVal = sensors[i].readFn();
    if (sensorVal > 999) {
      sensorVal = 999;
    }
    sensors[i].last_val = sensorVal;

    if (new_sampling_tick != -1)
    {
      buffer.unshift(sensorVal);
    }
  }

  // send telemetry?
  static unsigned long nextTelemetrySendTime = 0;

  if(millis() >= nextTelemetrySendTime) 
  {
    if (AziotIsConnected())
    {
        StaticJsonDocument<JSON_MAX_SIZE> doc;
        doc["no2"] = sensors[0].last_val;
        doc["c2h5nh"] = sensors[1].last_val;
        doc["voc"] = sensors[2].last_val;
        doc["co"] = sensors[3].last_val;
        AziotSendTelemetry<JSON_MAX_SIZE>(doc, "gas_sensor");

        nextTelemetrySendTime = millis() + TelemetryInterval_ * 1000;
    }
  }

  if (mode == TRAINING)
  {
    ei_printf("%d,%d,%d,%d\n", sensors[0].last_val, sensors[1].last_val, sensors[2].last_val, sensors[3].last_val);
  }
  else
  { // INFERENCE

    if (!buffer.isFull()) {
      ei_printf("Need more samples to start infering.\n");
    }
    else
    {
      if(new_sampling_tick == -1) {
        // no new sample, no need to run a new inference
        return;
      }

      // Turn the raw buffer into a signal which we can then classify
      float buffer2[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

      for (int i = 0; i < buffer.size(); i++) {
        buffer2[i] = buffer[i];
      }

      signal_t signal;
      int err = numpy::signal_from_buffer(
        buffer2, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
      if (err != 0) {
        ei_printf("Failed to create signal from buffer (%d)\n", err);
        return;
      }

      // Run the classifier
      ei_impulse_result_t result = { 0 };

      err = run_classifier(&signal, &result, debug_nn);
      if (err != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        return;
      }

      // print the predictions
      size_t best_prediction = 0;
      ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d "
                "ms.): \n",
                result.timing.dsp,
                result.timing.classification,
                result.timing.anomaly);

      int lineNumber = 60;
      char lineBuffer[30] = "";

      for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        if (result.classification[ix].value >=
            result.classification[best_prediction].value) {
          best_prediction = ix;
        }

        sprintf(lineBuffer,
                "    %s: %.5f\n",
                result.classification[ix].label,
                result.classification[ix].value);
        ei_printf(lineBuffer);

      }

#if EI_CLASSIFIER_HAS_ANOMALY == 1
      ei_printf(lineBuffer);
#endif

      sprintf(title_text, "%s (%d%%)", result.classification[best_prediction].label, (int)(result.classification[best_prediction].value * 100));
      ei_printf("Best prediction: %s\n", title_text);

      // check if we need to report a change to the IoT platform. 
      // 2 cases: new scent has been detected, or confidence level of a scent previously reported as changed by >10 percentage points
      if(best_prediction != latest_inference_idx || 
         best_prediction == latest_inference_idx && (result.classification[best_prediction].value - latest_inference_confidence_level > .10) ) 
      {
        StaticJsonDocument<JSON_MAX_SIZE> doc;
        doc["latestInferenceResult"] = title_text;

        char json[JSON_MAX_SIZE];
        serializeJson(doc, json);

        static int requestId = 444; char b[12];
        AziotHub_.SendTwinPatch(itoa(requestId++, b, 10), json);

        Display_.Printf("Reporting: %s\r\n", title_text);

        latest_inference_idx = best_prediction;
        latest_inference_confidence_level = result.classification[best_prediction].value;
      }

    }
  }

}
