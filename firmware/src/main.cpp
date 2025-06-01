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
// Network

#include <Network/WiFiManager.h>
#include <Network/TimeManager.h>
#include <Aziot/AziotDps.h>
#include <Aziot/AziotHub.h>
#include <ArduinoJson.h>
#include "CryptoUtils.h" // For HMAC SHA256 Utility

// For Edge Impulse HTTP Data Ingestion
#include <WiFiClientSecure.h> // For HTTPS
#include <HTTPClient.h>       // Standard Arduino HTTP Client

static bool isWifiConfigured = false;

static WiFiManager WifiManager_;
static TimeManager TimeManager_;
static std::string HubHost_;
static std::string DeviceId_;
static AziotHub AziotHub_;

static unsigned long TelemetryInterval_ = TELEMETRY_INTERVAL;   // [sec.]
static unsigned long nextTelemetrySendTime = 0;


/**
* @brief      Printf function uses vsnprintf and output using Arduino Serial
*
* @param[in]  format     Variable argument list
*/
void ei_printf(const char *format, ...)
{
  static char print_buf[512] = { 0 };

  va_list args;
  va_start(args, format);
  int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
  va_end(args);

  if (r > 0)
  {
    Serial.write(print_buf);
  }
}

static void ConnectWiFi()
{
  ei_printf("Connecting to SSID: %s\n", Storage_.WiFiSSID.c_str());
	WifiManager_.Connect(Storage_.WiFiSSID.c_str(), Storage_.WiFiPassword.c_str());
	while (!WifiManager_.IsConnected())
	{
		ei_printf(".");
		delay(500);
	}
	ei_printf("Connected\n");
}

static void SyncTimeServer()
{
	ei_printf("Synchronize time\n");
	while (!TimeManager_.Update())
	{
		ei_printf(".");
		delay(1000);
	}
	ei_printf("Synchronized\n");
}

static bool DeviceProvisioning()
{
	ei_printf("Device provisioning:\n");
  ei_printf(" Id scope = %s\n", Storage_.IdScope.c_str());
  ei_printf(" Registration id = %s\n", Storage_.RegistrationId.c_str());

	AziotDps aziotDps;
	aziotDps.SetMqttPacketSize(MQTT_PACKET_SIZE);

  if (aziotDps.RegisterDevice(DPS_GLOBAL_DEVICE_ENDPOINT_HOST, Storage_.IdScope, Storage_.RegistrationId, Storage_.SymmetricKey, MODEL_ID, TimeManager_.GetEpochTime() + TOKEN_LIFESPAN, &HubHost_, &DeviceId_) != 0)
  {
    ei_printf("ERROR: RegisterDevice()\n");
    return false;
  }

  ei_printf("Device provisioned:\n");
  ei_printf(" Hub host = %s\n", HubHost_.c_str());
  ei_printf(" Device id = %s\n", DeviceId_.c_str());

    return true;
}

static bool AziotIsConnected()
{
  return AziotHub_.IsConnected();
}

static void AziotDoWork()
{
    if(!isWifiConfigured) return;

    static unsigned long connectTime = 0;
    static unsigned long forceDisconnectTime;

    bool repeat;
    do {
      repeat = false;

      const auto now = TimeManager_.GetEpochTime();
      if (!AziotHub_.IsConnected()) {
        if (now >= connectTime) {
          // Serial.printf("Connecting to Azure IoT Hub...\n");
          if (AziotHub_.Connect(HubHost_,
                                DeviceId_,
                                Storage_.SymmetricKey,
                                MODEL_ID,
                                now + TOKEN_LIFESPAN) != 0) {
            // Serial.printf("ERROR: Try again in 5 seconds\n");
            connectTime = TimeManager_.GetEpochTime() + 5;
            return;
          }

          // Serial.printf("SUCCESS\n");
          forceDisconnectTime =
            TimeManager_.GetEpochTime() +
            static_cast<unsigned long>(TOKEN_LIFESPAN * RECONNECT_RATE);

          AziotHub_.RequestTwinDocument("get_twin");
        }
      } else {
        if (now >= forceDisconnectTime) {
          // Serial.printf("Disconnect\n");
          AziotHub_.Disconnect();
          connectTime = 0;

          repeat = true;
        } else {
          AziotHub_.DoWork();
        }
      }
    } while (repeat);
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
#include <artificial_nose_inferencing.h>

#include <Multichannel_Gas_GMXXX.h>
#include <Wire.h>
GAS_GMXXX<TwoWire>* gas = new GAS_GMXXX<TwoWire>();

#include "seeed_line_chart.h"
#include <TFT_eSPI.h>
TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft); // main sprite

#define DARK_BACKGROUND 0
#define TEXT_COLOR (DARK_BACKGROUND ? TFT_WHITE : TFT_BLACK)
#define BG_COLOR (DARK_BACKGROUND ? TFT_BLACK : TFT_WHITE)

#include "fonts/roboto_bold_28.h"

#include "images/icon_ambient.h"
#include "images/icon_anomaly.h"
#include "images/icon_coffee.h"
#include "images/icon_no_anomaly.h"
#include "images/icon_whiskey.h"

#include "images/icon_wifi.h"

#define USE_ICONS 1

#if USE_ICONS
const unsigned short* ICONS_MAP[] = { icon_ambient, icon_coffee, icon_whiskey };
#endif

typedef uint32_t (GAS_GMXXX<TwoWire>::*sensorGetFn)();

typedef struct SENSOR_INFO
{
  char* name;
  char* unit;
  std::function<uint32_t()> readFn;
  uint16_t color;
  uint32_t last_val;
} SENSOR_INFO;

SENSOR_INFO sensors[4] = {
  { "NO2", "ppm", std::bind(&GAS_GMXXX<TwoWire>::measure_NO2, gas), TFT_RED, 0 },
  { "CO", "ppm", std::bind(&GAS_GMXXX<TwoWire>::measure_CO, gas), TFT_GREEN, 0 },
  { "C2H5OH", "ppm", std::bind(&GAS_GMXXX<TwoWire>::measure_C2H5OH, gas), TFT_BLUE, 0 },
  { "VOC", "ppm", std::bind(&GAS_GMXXX<TwoWire>::measure_VOC, gas), TFT_PURPLE, 0 }
};
#define NB_SENSORS 4

char title_text[50] = "";

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

#define MAX_CHART_SIZE 50
std::vector<doubles> chart_series = std::vector<doubles>(NB_SENSORS, doubles());

// Allocate a buffer for the values we'll read from the gas sensor
CircularBuffer<float, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE> buffer;

uint64_t next_sampling_tick = micros();

// Edge Impulse Data Batching
#ifndef EDGE_IMPULSE_BATCH_SIZE // Guard against missing define, though it should be in Config.h
#define EDGE_IMPULSE_BATCH_SIZE 100
#endif
#ifndef EDGE_IMPULSE_SENSOR_INTERVAL_MS // Guard against missing define
#define EDGE_IMPULSE_SENSOR_INTERVAL_MS 100
#endif
static int sensor_batch_buffer[EDGE_IMPULSE_BATCH_SIZE][NB_SENSORS];
static int current_batch_sample_count = 0;
static uint64_t batch_start_time = 0; // For 'iat' timestamp for Edge Impulse
static unsigned long next_edge_impulse_sample_collection_time = 0; // Timer for EDGE_IMPULSE_SENSOR_INTERVAL_MS

#define INITIAL_FAN_STATE LOW
// static int fan_state = INITIAL_FAN_STATE;
uint8_t current_fan_speed_level = 0; // 0: Off, 1: Low, 2: Medium, 3: High
const int FAN_PWM_OFF = 0;
const int FAN_PWM_LOW = 100;
const int FAN_PWM_MEDIUM = 150;
const int FAN_PWM_HIGH = 200;

static bool debug_nn = false; // Set this to true to see e.g. features generated
                              // from the raw signal

void draw_chart();

enum class ButtonId
{
  A,
  C,
  LEFT,
  RIGHT,
  UP,
  DOWN,
  PRESS
};
static const int ButtonNumber = 6;
static AceButton Buttons[ButtonNumber];

static void ReceivedTwinDocument(const char* json, const char* requestId)
{
	StaticJsonDocument<JSON_MAX_SIZE> doc;
	if (deserializeJson(doc, json)) return;
    
  if (doc["desired"]["$version"].isNull()) return;

    if (AziotUpdateWritableProperty("telemetryInterval", &TelemetryInterval_, doc["desired"]["$version"], doc["desired"], doc["reported"]))
    {
      nextTelemetrySendTime = millis() + TelemetryInterval_;
      // ei_printf("New telemetryInterval = %d\n", TelemetryInterval_);
    }
}

static void ReceivedTwinDesiredPatch(const char* json, const char* version)
{
	StaticJsonDocument<JSON_MAX_SIZE> doc;
	if (deserializeJson(doc, json)) return;

	if (doc["$version"].isNull()) return;

  if (AziotUpdateWritableProperty("telemetryInterval", &TelemetryInterval_, doc["$version"], doc.as<JsonVariant>()))
  {
    nextTelemetrySendTime = millis() + TelemetryInterval_;
    // ei_printf("New telemetryInterval = %d\n", TelemetryInterval_);
  }

}

static void ButtonEventHandler(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
  const uint8_t id = button->getId();
  if (ButtonNumber <= id)
    return;

  switch (eventType) {
    case AceButton::kEventReleased:
      // Handle Button A press for fan control
      if (static_cast<ButtonId>(id) == ButtonId::A) {
        if (current_fan_speed_level == 0) { // Currently Off
            current_fan_speed_level = 1; // Set to Low
            analogWrite(D0, FAN_PWM_LOW);
        } else if (current_fan_speed_level == 1) { // Currently Low
            current_fan_speed_level = 2; // Set to Medium
            analogWrite(D0, FAN_PWM_MEDIUM);
        } else if (current_fan_speed_level == 2) { // Currently Medium
            current_fan_speed_level = 3; // Set to High
            analogWrite(D0, FAN_PWM_HIGH);
        } else { // Currently High (or any other state, cycle back to Off)
            current_fan_speed_level = 0; // Set to Off
            analogWrite(D0, FAN_PWM_OFF);
        }
      } 
      // Handle other buttons
      else { // <<< Add else here to ensure Button A logic is separate
        switch (static_cast<ButtonId>(id)) {
          case ButtonId::C:
            // Toggle Fan (This logic will be removed or changed in a later step)
            // For now, let's comment it out to avoid conflict and make it clear
            // that Button A is now the primary fan control.
            // fan_state = (fan_state == HIGH) ? LOW : HIGH ; 
            // digitalWrite(D0, fan_state); // Turn fan ON
            break;
          case ButtonId::PRESS:
            mode = (mode == INFERENCE) ? TRAINING : INFERENCE;
          spr.pushSprite(0, 0);
          break;
        case ButtonId::LEFT:
          switch (screen_mode) {
            case INFERENCE_RESULTS:
              screen_mode = GRAPH;
              break;
            case GRAPH:
              screen_mode = SENSORS;
              break;
          }
          break;
        case ButtonId::RIGHT:
          switch (screen_mode) {
            case SENSORS:
              screen_mode = GRAPH;
              break;
            case GRAPH:
              screen_mode = INFERENCE_RESULTS;
              break;
          }
          break;
      }
      break;
  }
}

static void ButtonInit()
{
  Buttons[static_cast<int>(ButtonId::A)].init(
    WIO_KEY_A, HIGH, static_cast<uint8_t>(ButtonId::A));
  Buttons[static_cast<int>(ButtonId::C)].init(
    WIO_KEY_C, HIGH, static_cast<uint8_t>(ButtonId::C));
  Buttons[static_cast<int>(ButtonId::LEFT)].init(
    WIO_5S_LEFT, HIGH, static_cast<uint8_t>(ButtonId::LEFT));
  Buttons[static_cast<int>(ButtonId::RIGHT)].init(
    WIO_5S_RIGHT, HIGH, static_cast<uint8_t>(ButtonId::RIGHT));
  Buttons[static_cast<int>(ButtonId::UP)].init(
    WIO_5S_UP, HIGH, static_cast<uint8_t>(ButtonId::UP));
  Buttons[static_cast<int>(ButtonId::DOWN)].init(
    WIO_5S_DOWN, HIGH, static_cast<uint8_t>(ButtonId::DOWN));
  Buttons[static_cast<int>(ButtonId::PRESS)].init(
    WIO_5S_PRESS, HIGH, static_cast<uint8_t>(ButtonId::PRESS));

  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(ButtonEventHandler);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
}

static void ButtonDoWork()
{
  for (int i = 0;
       static_cast<size_t>(i) < std::extent<decltype(Buttons)>::value;
       ++i) {
    Buttons[i].check();
  }
}

void setup()
{
  Storage_.Load();

  Serial.begin(115200);

  pinMode(D0, OUTPUT);
  digitalWrite(D0, fan_state);

  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);

  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);

  if (digitalRead(WIO_KEY_A) == LOW &&
      digitalRead(WIO_KEY_B) == LOW &&
      digitalRead(WIO_KEY_C) == LOW   )
  {
      ei_printf("In configuration mode\r\n");
      ConfigurationMode(Storage_);
  }

  ButtonInit();

  pinMode(D0, OUTPUT);
  analogWrite(D0, FAN_PWM_OFF);

  gas->begin(Wire, 0x08); // use the hardware I2C

  // put your setup code here, to run once:
  tft.begin();
  tft.setRotation(3);
  spr.setColorDepth(8);
  spr.createSprite(
    tft.width(),
    tft.height()); // /!\ this will allocate 320*240*2 = 153.6K of RAM

  spr.fillSprite(BG_COLOR);
  spr.setTextColor(TEXT_COLOR);

  spr.setFreeFont(&FreeSans12pt7b);

	if(! Storage_.WiFiSSID.empty()) {
    isWifiConfigured = true              ;
  
    spr.drawString("Wi-Fi", 20, 40);
    spr.pushSprite(0,0);
    ConnectWiFi();
    spr.drawXBitmap(320 - 24 - 4, 4, icon_wifi, 24, 24, TFT_GREEN, BG_COLOR);
    spr.drawString("Wi-Fi ... OK", 20, 40);
    spr.pushSprite(0,0);

    spr.drawString("Time sync.", 20, 70);
    spr.pushSprite(0,0);
    SyncTimeServer();
    spr.drawString("Time sync. ... OK", 20, 70);
    spr.pushSprite(0,0);

    spr.drawString("Provisioning", 20, 100);
    spr.pushSprite(0,0);
    if (!DeviceProvisioning()) abort();
    spr.drawString("Provisioning ... OK", 20, 100);
    spr.pushSprite(0,0);

    spr.drawString("Azure IoT Hub", 20, 130);
    spr.pushSprite(0,0);
    AziotDoWork();
    spr.drawString("Azure IoT Hub ... OK", 20, 130);
    spr.pushSprite(0,0);

    AziotHub_.SetMqttPacketSize(MQTT_PACKET_SIZE);

    AziotHub_.ReceivedTwinDocumentCallback = ReceivedTwinDocument;
    AziotHub_.ReceivedTwinDesiredPatchCallback = ReceivedTwinDesiredPatch;  

    // Initialize Edge Impulse batch collection timer
    next_edge_impulse_sample_collection_time = millis();
  }
 
}

int fan = 0;

void loop()
{
  spr.fillSprite(BG_COLOR);

  if(isWifiConfigured && WifiManager_.IsConnected()) {
    spr.drawXBitmap(320 - 24 - 4, 4, icon_wifi, 24, 24, TFT_GREEN, BG_COLOR);
  }

  ButtonDoWork();
  AziotDoWork();

  if (mode == TRAINING) {
    strcpy(title_text, "Training mode");
  }

  if (screen_mode != INFERENCE_RESULTS) {
    spr.setFreeFont(&Roboto_Bold_28);
    spr.setTextColor(TEXT_COLOR);
    spr.drawString(title_text, 15, 10, 1);
    for (int8_t line_index = 0; line_index <= 2; line_index++) {
      spr.drawLine(
        0, 50 + line_index, tft.width(), 50 + line_index, TEXT_COLOR);
    }

  }

  spr.setFreeFont(&FreeSansBoldOblique9pt7b); // Select the font
  spr.setTextColor(TEXT_COLOR);

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

    if (chart_series[i].size() == MAX_CHART_SIZE) {
      chart_series[i].pop();
    }
    chart_series[i].push(sensorVal);
    if (new_sampling_tick != -1) {
      buffer.unshift(sensorVal);
    }
  }

  switch (screen_mode) {
    case SENSORS: {
      for (int i = 0; i < NB_SENSORS; i++) {
        int x_ref = 60 + (i % 2 * 170);
        int y_ref = 100 + (i / 2 * 80);

        spr.setTextColor(TEXT_COLOR);
        spr.drawString(sensors[i].name, x_ref - 24, y_ref - 24, 1);
        spr.drawRoundRect(x_ref - 24, y_ref, 80, 40, 5, TEXT_COLOR);
        spr.drawNumber(chart_series[i].back(), x_ref - 20, y_ref + 10, 1);
        spr.setTextColor(TFT_GREEN);
        spr.drawString(sensors[i].unit, x_ref + 12, y_ref + 8, 1);
      }
      break;
    }
    case GRAPH: {
      auto content = line_chart(10, 60); //(x,y) where the line graph begins
      content.height(tft.height() - 70 * 1.2)
        .width(tft.width() - content.x() * 2)
        .based_on(0.0)
        .show_circle(false)
        .value(chart_series)
        .x_role_color(TEXT_COLOR)
        .y_role_color(TEXT_COLOR)
        .x_tick_color(TEXT_COLOR)
        .y_tick_color(TEXT_COLOR)
        .x_auxi_role(dash_line().color(TFT_DARKGREY))
        .color(sensors[0].color, sensors[1].color, sensors[2].color, sensors[3].color)
        .draw();

      for (int i = 0; i < NB_SENSORS; i++) {
        spr.setFreeFont(&FreeSans9pt7b);
        spr.setTextColor(sensors[i].color);
        spr.setTextDatum(BC_DATUM);
        spr.drawString(sensors[i].name, 70 + (i * 70), 236, 1);
      }

      spr.setTextDatum(TL_DATUM); // reset to default

      break;
    }

    case INFERENCE_RESULTS: {
      if (mode == TRAINING) {
        spr.drawString("Outputting sensor data to serial.", 16, 60, 1);
        spr.drawString("Use Edge Impulse CLI on your", 16, 100, 1);
        spr.drawString("computer to upload training", 16, 120, 1);
        spr.drawString("data to your project.", 16, 140, 1);
      }
    }

    default: {
      break; // nothing
    }
  }

  if(millis() >= nextTelemetrySendTime) 
  {
    if (AziotIsConnected())
    {
        StaticJsonDocument<JSON_MAX_SIZE> doc;
        doc["no2"] = sensors[0].last_val;
        doc["co"] = sensors[1].last_val;
        doc["c2h5oh"] = sensors[2].last_val;
        doc["voc"] = sensors[3].last_val;
        AziotSendTelemetry<JSON_MAX_SIZE>(doc, "gas_sensor");

        nextTelemetrySendTime = millis() + TelemetryInterval_ * 1000;
    }
  }

  if (mode == TRAINING) {
    ei_printf("%d,%d,%d,%d\n", sensors[0].last_val, sensors[1].last_val, sensors[2].last_val, sensors[3].last_val);
  }

  // Edge Impulse Data Batching Logic
  if (mode == TRAINING && WifiManager_.IsConnected() && !Storage_.EdgeImpulseHmacKey.empty()) {
      if (millis() >= next_edge_impulse_sample_collection_time) {
          next_edge_impulse_sample_collection_time = millis() + EDGE_IMPULSE_SENSOR_INTERVAL_MS;

          if (current_batch_sample_count < EDGE_IMPULSE_BATCH_SIZE) {
              // Record batch start time only for the first sample
              if (current_batch_sample_count == 0) {
                  if (!TimeManager_.IsSynchronized()) {
                       TimeManager_.Update(); // Attempt to update if not synced
                  }
                  if (TimeManager_.IsSynchronized()) {
                      batch_start_time = TimeManager_.GetEpochTime();
                  } else {
                      ei_printf("Warning: Time for Edge Impulse batch 'iat' not synchronized.\n");
                      batch_start_time = 0; // Indicate invalid time
                  }
              }

              // Store sensor readings into the batch buffer
              for (int j = 0; j < NB_SENSORS; ++j) {
                  // Assuming sensors array is ordered NO2, CO, C2H5OH, VOC
                  // and this order matches what Edge Impulse expects.
                  // The sensor data is already in sensors[j].last_val from the loop above.
                  sensor_batch_buffer[current_batch_sample_count][j] = sensors[j].last_val;
              }
              current_batch_sample_count++;

              if (current_batch_sample_count >= EDGE_IMPULSE_BATCH_SIZE) {
                  ei_printf("Edge Impulse Batch ready to be sent (%d samples).\n", current_batch_sample_count);
                  if (batch_start_time != 0) { // Only send if we have a valid timestamp
                    // processAndSendEdgeImpulseBatch(batch_start_time, sensor_batch_buffer, current_batch_sample_count); // Definition comes later
                  } else {
                    ei_printf("ERROR: Edge Impulse batch dropped due to invalid batch_start_time (time not synchronized).\n");
                  }
                  current_batch_sample_count = 0; // Reset for next batch
              }
          }
      }
  } else if (mode != TRAINING || !WifiManager_.IsConnected() || Storage_.EdgeImpulseHmacKey.empty()) {
      // If conditions for EI upload are not met, reset batch count
      if (current_batch_sample_count > 0) {
          current_batch_sample_count = 0;
          ei_printf("Edge Impulse batch collection reset due to mode/WiFi/HMAC key change.\n");
      }
  }

  if (mode != TRAINING) { // INFERENCE (else part of the original if)

    if (!buffer.isFull()) {
      ei_printf("Need more samples to start infering.\n");
    } else {
      // Turn the raw buffer into a signal which we can then classify
      float buffer2[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

      for (int i = 0; i < buffer.size(); i++) {
        buffer2[i] = buffer[i];
        ei_printf("%f, ", buffer[i]);
      }
      ei_printf("\n");

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
      char lineBuffer[60] = "";

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

      if (screen_mode == INFERENCE_RESULTS) {
        if (best_prediction != latest_inference_idx) {
          // clear icon background
          spr.pushSprite(0, 0);
        }
        #if USE_ICONS
        spr.pushImage(30, 35, 130, 130, (uint16_t*)ICONS_MAP[best_prediction]);
        #endif
        spr.setFreeFont(&Roboto_Bold_28);
        spr.setTextDatum(CC_DATUM);
        spr.setTextColor(TEXT_COLOR);
        spr.drawString(title_text, 160, 200, 1);
      }

#if EI_CLASSIFIER_HAS_ANOMALY == 1
      sprintf(lineBuffer, "    anomaly score: %.3f\n", result.anomaly);
      ei_printf(lineBuffer);
      if (mode == INFERENCE && screen_mode == INFERENCE_RESULTS) {
        spr.pushImage(160,
                      35,
                      130,
                      130,
                      (result.anomaly > 0.15) ? icon_anomaly : icon_no_anomaly);
      }
#endif

      sprintf(title_text,
              "%s (%d%%)",
              result.classification[best_prediction].label,
              (int)(result.classification[best_prediction].value * 100));

      ei_printf("Best prediction: %s\n", title_text);

      // check if we need to report a change in detected scent to the IoT platform. 
      // 2 cases: new scent has been detected, or confidence level of a scent previously reported as changed by 5+ percentage points
      if (best_prediction != latest_inference_idx ||
          best_prediction == latest_inference_idx &&
            (result.classification[best_prediction].value -
               latest_inference_confidence_level >
             .05)) {
        if (isWifiConfigured) {
        StaticJsonDocument<JSON_MAX_SIZE> doc;
        doc["latestInferenceResult"] = title_text;

        char json[JSON_MAX_SIZE];
        serializeJson(doc, json);

          static int requestId = 444;
          char b[12];
        AziotHub_.SendTwinPatch(itoa(requestId++, b, 10), json);
      }

      latest_inference_idx = best_prediction;
      latest_inference_confidence_level =
        result.classification[best_prediction].value;
      }
    }
  }

  spr.pushSprite(0, 0, TFT_MAGENTA);

  // Uncomment block below to dump hex-encoded TFT sprite to serial **/

  /**

    uint16_t width = tft.width(), height = tft.height();
    // Image data: width * height * 2 bytes
    for (int y = 0; y < height ; y++) {
      for (int x=0; x<width; x++) {
        Serial.printf("%04X", tft.readPixel(x,y));
      }
    }

    **/
}

// Function to process and send the Edge Impulse batch
// Definition will be expanded in subsequent steps
static void processAndSendEdgeImpulseBatch(uint64_t iat_timestamp, int readings_buffer[][NB_SENSORS], int num_samples) {
    if (num_samples <= 0) {
        ei_printf("Warning: processAndSendEdgeImpulseBatch called with num_samples = %d. Skipping.\n", num_samples);
        return;
    }

    ei_printf("Processing Edge Impulse batch with %d samples, iat: %llu\n", num_samples, iat_timestamp);

    // JSON Payload Construction (Step 3)
    // Using estimated sizes. Adjust if necessary.
    // Max ~3KB for payload with 100 samples, 4 sensors.
    StaticJsonDocument<4096> payload_doc;

    payload_doc["device_name"] = EDGE_IMPULSE_DEVICE_ID;
    payload_doc["device_type"] = EDGE_IMPULSE_DEVICE_TYPE;
    payload_doc["interval_ms"] = EDGE_IMPULSE_SENSOR_INTERVAL_MS;

    JsonArray sensors_array = payload_doc.createNestedArray("sensors");
    JsonObject sensor1 = sensors_array.createNestedObject();
    sensor1["name"] = "Nitrogen dioxide"; sensor1["units"] = "N/A"; // Units as per EI example, though we have ppm
    JsonObject sensor2 = sensors_array.createNestedObject();
    sensor2["name"] = "Carbon monoxide"; sensor2["units"] = "N/A";
    JsonObject sensor3 = sensors_array.createNestedObject();
    sensor3["name"] = "Ethyl alcohol"; sensor3["units"] = "N/A";
    JsonObject sensor4 = sensors_array.createNestedObject();
    sensor4["name"] = "Volatile organic compounds"; sensor4["units"] = "N/A";

    JsonArray values_array = payload_doc.createNestedArray("values");
    for (int i = 0; i < num_samples; ++i) {
        JsonArray single_sample_array = values_array.createNestedArray();
        for (int j = 0; j < NB_SENSORS; ++j) {
            single_sample_array.add(readings_buffer[i][j]);
        }
    }

    std::string payload_str;
    serializeJson(payload_doc, payload_str);

    // HMAC Signature Calculation (Step 4)
    if (Storage_.EdgeImpulseHmacKey.empty()) {
        ei_printf("ERROR: Edge Impulse HMAC Key is empty. Cannot sign payload.\n");
        return;
    }
    std::string signature = compute_hmac_sha256(Storage_.EdgeImpulseHmacKey.c_str(), payload_str.c_str());

    if (signature.empty()) {
        ei_printf("ERROR: Failed to compute HMAC SHA256 signature for Edge Impulse payload. Discarding batch.\n");
        return;
    }

    // Final JSON Object Construction (Step 5)
    // Max ~4KB for final doc (payload + protected + signature)
    StaticJsonDocument<5120> final_doc;

    JsonObject protected_obj = final_doc.createNestedObject("protected");
    protected_obj["ver"] = "v1";
    protected_obj["alg"] = "HS256";
    protected_obj["iat"] = iat_timestamp;

    final_doc["signature"] = signature.c_str();

    // Deep copy payload_doc into final_doc. This is important.
    // The payload_doc must exist when final_doc["payload"] is assigned if it's by reference.
    // Assigning the serialized string or parsing it into final_doc might be safer
    // depending on ArduinoJson version, but direct object assignment usually works via deep copy for StaticJsonDocument.
    final_doc["payload"] = payload_doc; // This should perform a deep copy.

    std::string final_json_str;
    serializeJson(final_doc, final_json_str);

    ei_printf("Edge Impulse Payload Size: %d bytes\n", final_json_str.length());
    // For brevity in logs, let's not print the full JSON unless a debug flag is set.
    // ei_printf("Edge Impulse Payload: %s\n", final_json_str.c_str());

    // HTTP POST Request (Step 6)
    WiFiClientSecure client; // Use WiFiClient for HTTP, WiFiClientSecure for HTTPS
    HTTPClient http;

    // Configure client for HTTPS, skip certificate validation for simplicity in embedded context
    // WARNING: Skipping certificate validation is insecure for production.
    // For Wio Terminal, specific functions might be needed if it uses a different SSL library.
    // client.setInsecure(); // Common for ESP32. May vary for RTL8720DN.
                          // If this doesn't exist, it might do basic validation or none by default.

    String server_url = String("https://") + EDGE_IMPULSE_INGESTION_HOST + EDGE_IMPULSE_INGESTION_PATH_TRAINING;
    ei_printf("Posting to URL: %s\n", server_url.c_str());

    if (http.begin(client, server_url)) { // HTTPS by default with WiFiClientSecure
        http.addHeader("x-api-key", EDGE_IMPULSE_API_KEY);
        http.addHeader("x-label", EDGE_IMPULSE_LABEL);
        String filename = String("WIO_TERMINAL_") + String((unsigned long)iat_timestamp) + ".json";
        http.addHeader("x-file-name", filename.c_str()); // Ensure .c_str() if needed
        http.addHeader("Content-Type", "application/json");
        // http.setReuse(true); // Optional: for keep-alive if sending frequently

        int httpCode = http.POST((uint8_t*)final_json_str.c_str(), final_json_str.length());

        if (httpCode > 0) {
            String response_payload = http.getString();
            ei_printf("Edge Impulse HTTP POST code: %d\n", httpCode);
            ei_printf("Edge Impulse Response: %s\n", response_payload.c_str());
            if (httpCode != 200) {
                ei_printf("ERROR: Edge Impulse upload failed. Discarding batch.\n");
            } else {
                ei_printf("Edge Impulse batch uploaded successfully.\n");
            }
        } else {
            ei_printf("ERROR: Edge Impulse HTTP POST failed: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        ei_printf("ERROR: HTTPClient begin failed for Edge Impulse.\n");
    }
}