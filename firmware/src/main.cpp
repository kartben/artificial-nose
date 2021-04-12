#include <Arduino.h>

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


#include "SPI.h"

#include "Config.h"
#include "Storage.h"
#include "Signature.h"
#include "AzureDpsClient.h"
#include "CliMode.h"
#include <rpcWiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTP.h>
#include <azure/core/az_json.h>
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/iot/az_iot_hub_client.h>
#define MQTT_PACKET_SIZE 1024

WiFiClientSecure wifi_client;
PubSubClient mqtt_client(wifi_client);
WiFiUDP wifi_udp;
NTP ntp(wifi_udp);


#include <AceButton.h>
using namespace ace_button;

#include <artificial_nose_inference.h>
#include <CircularBuffer.h>

#include <Wire.h>
#include <Multichannel_Gas_GMXXX.h>
GAS_GMXXX<TwoWire> *gas = new GAS_GMXXX<TwoWire>();

typedef uint32_t (GAS_GMXXX<TwoWire>::*sensorGetFn)();

typedef struct SENSOR_INFO
{
  char *name;
  char *unit;
  std::function<uint32_t()> readFn;
  uint32_t last_val;
} SENSOR_INFO;

int a = configTOTAL_HEAP_SIZE;

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

// Allocate a buffer for the values we'll read from the gas sensor
CircularBuffer<float, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE> buffer;

uint64_t next_sampling_tick = micros();

#define INITIAL_FAN_STATE LOW

static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal

void draw_chart();

enum class ButtonId
{
  A = 0,
  B,
  C,
  PRESS
};
static const int ButtonNumber = 4;
static AceButton Buttons[ButtonNumber];

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

static void ButtonInit()
{
  Buttons[static_cast<int>(ButtonId::A)].init(WIO_KEY_A, HIGH, static_cast<uint8_t>(ButtonId::A));
  Buttons[static_cast<int>(ButtonId::B)].init(WIO_KEY_B, HIGH, static_cast<uint8_t>(ButtonId::B));
  Buttons[static_cast<int>(ButtonId::C)].init(WIO_KEY_C, HIGH, static_cast<uint8_t>(ButtonId::C));
  Buttons[static_cast<int>(ButtonId::PRESS)].init(WIO_5S_PRESS, HIGH, static_cast<uint8_t>(ButtonId::PRESS));

  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(ButtonEventHandler);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
}

static void ButtonDoWork()
{
  for (int i = 0; static_cast<size_t>(i) < std::extent<decltype(Buttons)>::value; ++i)
  {
      Buttons[i].check();
  }
}

/**
* @brief      Arduino setup function
*/
void setup()
{
  Storage::Load();

  Serial.begin(115200);

  pinMode(D0, OUTPUT);
  digitalWrite(D0, INITIAL_FAN_STATE);

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
      CliMode();
  }


  ButtonInit();

  gas->begin(Wire, 0x08); // use the hardware I2C

  // Connect to wi-fi
  Serial.println("bla");
  ei_printf("Connecting to SSID: %s. Free mem: %d\r\n", IOT_CONFIG_WIFI_SSID, freeMemory());
  do
  {
      ei_printf(".");
      WiFi.begin(IOT_CONFIG_WIFI_SSID, IOT_CONFIG_WIFI_PASSWORD);
      delay(500);
  }
  while (WiFi.status() != WL_CONNECTED);
  ei_printf("Connected\r\n");

  ////////////////////
  // Sync time server

  ntp.begin();

  ei_printf("Current year: %d\r\n", ntp.year());
  
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

int fan = 0;

/**
* @brief      Get data and run inferencing
*
* @param[in]  debug  Get debug info if true
*/
void loop()
{
  ButtonDoWork();

  if (mode == TRAINING)
  {
    strcpy(title_text, "Training mode");
  }

  uint64_t new_sampling_tick = -1;
  if (micros() > next_sampling_tick)
  {
    new_sampling_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);
    next_sampling_tick = new_sampling_tick;
  }
  for (int i = NB_SENSORS - 1; i >= 0; i--)
  {
    uint32_t sensorVal = sensors[i].readFn();
    if (sensorVal > 999)
    {
      sensorVal = 999;
    }
    sensors[i].last_val = sensorVal;

    if (new_sampling_tick != -1)
    {
      buffer.unshift(sensorVal);
    }
  }

  if (mode == TRAINING)
  {
    ei_printf("%d,%d,%d,%d\n", sensors[0].last_val, sensors[1].last_val, sensors[2].last_val, sensors[3].last_val);
  }
  else
  { // INFERENCE

    if (!buffer.isFull())
    {
      ei_printf("Need more samples to start infering.\n");
    }
    else
    {
      // Turn the raw buffer into a signal which we can then classify
      float buffer2[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

      for (int i = 0; i < buffer.size(); i++)
      {
        buffer2[i] = buffer[i];
        ei_printf("%f, ", buffer[i]);
      }
      ei_printf("\n");

      signal_t signal;
      int err = numpy::signal_from_buffer(buffer2, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
      if (err != 0)
      {
        ei_printf("Failed to create signal from buffer (%d)\n", err);
        return;
      }

      // Run the classifier
      ei_impulse_result_t result = {0};

      err = run_classifier(&signal, &result, debug_nn);
      if (err != EI_IMPULSE_OK)
      {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        return;
      }

      // print the predictions
      size_t best_prediction = 0;
      ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
                result.timing.dsp, result.timing.classification, result.timing.anomaly);

      int lineNumber = 60;
      char lineBuffer[30] = "";

      for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
      {
        if (result.classification[ix].value >= result.classification[best_prediction].value)
        {
          best_prediction = ix;
        }

        sprintf(lineBuffer, "    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
        ei_printf(lineBuffer);

      }

#if EI_CLASSIFIER_HAS_ANOMALY == 1
      ei_printf(lineBuffer);
#endif

      sprintf(title_text, "%s (%d%%)", result.classification[best_prediction].label, (int)(result.classification[best_prediction].value * 100));
      ei_printf("Best prediction: %s\n", title_text);
    }
  }

}
