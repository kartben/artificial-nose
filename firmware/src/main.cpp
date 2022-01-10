/**
 * Left nostril: 0x66
 * Right nostril: 0x67
 *
 */

#include <Arduino.h>
// #include "Config.h"
// #include "ConfigurationMode.h"

#include <malta_bme688_dual_inferencing.h>

////////////////////////////////////////////////////////////////////////////////
// BME688
#include "bme688_helper.h"

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

#include <AceButton.h>
using namespace ace_button;

#include <CircularBuffer.h>
#include <Wire.h>

#include "seeed_line_chart.h"
#include <TFT_eSPI.h>
TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft); // main sprite

#define DARK_BACKGROUND 0
#define TEXT_COLOR (DARK_BACKGROUND ? TFT_WHITE : TFT_BLACK)
#define BG_COLOR (DARK_BACKGROUND ? TFT_BLACK : TFT_WHITE)

#include "fonts/roboto_bold_28.h"

#include "images/icon_wifi.h"

#define USE_ICONS 0

#if USE_ICONS
const unsigned short *ICONS_MAP[] = {icon_ambient, icon_coffee, icon_whiskey};

#include "images/icon_ambient.h"
#include "images/icon_anomaly.h"
#include "images/icon_coffee.h"
#include "images/icon_no_anomaly.h"
#include "images/icon_whiskey.h"

#endif

typedef struct SENSOR_INFO
{
  const char *name;
  const char *unit;
  uint16_t color;
  float val;
} SENSOR_INFO;

SENSOR_INFO sensors[4] = {
    {"Temp 1", "C", TFT_RED, 0.f},
    {"Hum 1", "%", TFT_GREEN, 0.f},
    {"Temp 2", "C", TFT_BLUE, 0.f},
    {"Hum2", "%", TFT_PURPLE, 0.f}};
#define NB_SENSORS 4

enum MODE
{
  TRAINING,
  //  INFERENCE
};
enum MODE mode = TRAINING;

enum SCREEN_MODE
{
  SENSORS,
  INFERENCE_RESULTS
};
enum SCREEN_MODE screen_mode = SENSORS;

int latest_inference_idx = -1;
float latest_inference_confidence_level = -1.;

#define MAX_CHART_SIZE 50
std::vector<doubles> chart_series = std::vector<doubles>(NB_SENSORS, doubles());

#define INITIAL_FAN_STATE LOW
static int fan_state = INITIAL_FAN_STATE;

static float featuresLeftSensor[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
static float featuresRightSensor[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

int nb_sensors_have_completed_cycle = 0;

// XXX HACK
int raw_features_get_data(size_t offset, size_t length, float *out_ptr)
{
  Serial.printf("offset: %d, length: %d\n", offset, length);

  memcpy(out_ptr, featuresLeftSensor, 10 * sizeof(float));
  memcpy(out_ptr + 10, featuresRightSensor, 10 * sizeof(float));

  return 0;
}

// int raw_features_left_get_data(size_t offset, size_t length, float *out_ptr)
// {
//   memcpy(out_ptr, featuresLeftSensor + offset, length * sizeof(float));

//   return 0;
// }

// int raw_features_right_get_data(size_t offset, size_t length, float *out_ptr)
// {
//   memcpy(out_ptr, featuresRightSensor + offset, length * sizeof(float));

//   return 0;
// }

int ts = 0; // "fake" timestamp

char inference_result[32] = "";

void runInference()
{
  ei_impulse_result_t result = {0};

  // the features are stored into flash, and we don't want to load everything into RAM
  signal_t features_signal;
  features_signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
  features_signal.get_data = raw_features_get_data;

  // invoke the impulse
  EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, true);

  if (res != 0)
    return;

  // print the predictions
  ei_printf("\n********************************************************\n");
  ei_printf("Prediction:\n");

  size_t best_prediction = 0;

  // human-readable predictions
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
  {
    if (result.classification[ix].value >=
        result.classification[best_prediction].value)
    {
      best_prediction = ix;
    }

    ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
  }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
  ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif

  sprintf(inference_result,
          "%s (%d%%)",
          result.classification[best_prediction].label,
          (int)(result.classification[best_prediction].value * 100));

  ei_printf("\nBest prediction: %s\n", inference_result);
  ei_printf("\n********************************************************\n");
}

void bsecGenericCallback(const bme68x_data &input, const BsecOutput &outputs, float features[10], float &temperature, float &humidity)
{
  if (!outputs.len)
    return;

  Serial.print(".");

  int gas_index = -1;
  float gas_resistance = -1;

  for (uint8_t i = 0; i < outputs.len; i++)
  {
    const bsec_output_t &output = outputs.outputs[i];

    switch (output.sensor_id)
    {
    case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE:
      temperature = output.signal;
      break;
    case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY:
      humidity = output.signal;
      break;
    case BSEC_OUTPUT_RAW_GAS:
      // Serial.println("\tgas resistance = " + String(output.signal));
      gas_resistance = output.signal;
      break;
    case BSEC_OUTPUT_RAW_GAS_INDEX:
      //      Serial.println("\tgas index = " + String(output.signal));
      gas_index = output.signal;
      break;
    case BSEC_OUTPUT_STABILIZATION_STATUS:
      // Serial.println("\tstabilization status = " + String(output.signal));
      break;
    case BSEC_OUTPUT_RUN_IN_STATUS:
      // Serial.println("\trun in status = " + String(output.signal));
      break;
    default:
      break;
    }
  }

  features[gas_index] = gas_resistance; //( numpy::log10(gas_resistance) - 4) / 4;

  if (/* true || */ gas_index == 9)
  {
    nb_sensors_have_completed_cycle++;

    if (nb_sensors_have_completed_cycle == 2)
    {
      Serial.print("\n[CSV] 0,");
      Serial.print(ts);
      ts += 1000;
      for (int i = 0; i < 10; i++)
      {
        Serial.print(",");
        Serial.print(featuresLeftSensor[i]);
      }
      for (int i = 0; i < 10; i++)
      {
        Serial.print(",");
        Serial.print(featuresRightSensor[i]);
      }
      Serial.println();

      runInference();

      nb_sensors_have_completed_cycle = 0;
    }
  }
}

void bsecCallbackSensor1(const bme68x_data &input, const BsecOutput &outputs)
{
  bsecGenericCallback(input, outputs, featuresLeftSensor, sensors[0].val, sensors[1].val);
}

Bsec sensor1(bsecCallbackSensor1);

void bsecCallbackSensor2(const bme68x_data &input, const BsecOutput &outputs)
{
  bsecGenericCallback(input, outputs, featuresRightSensor, sensors[2].val, sensors[3].val);
}

Bsec sensor2(bsecCallbackSensor2);

enum class ButtonId
{
  C,
  LEFT,
  RIGHT,
  UP,
  DOWN,
  PRESS
};
static const int ButtonNumber = 6;
static AceButton Buttons[ButtonNumber];

static void ButtonEventHandler(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
  const uint8_t id = button->getId();
  if (ButtonNumber <= id)
    return;

  switch (eventType) {
    case AceButton::kEventReleased:
      switch (static_cast<ButtonId>(id)) {
        case ButtonId::C:
          // Toggle Fan
          fan_state = (fan_state == HIGH) ? LOW : HIGH ; 
          digitalWrite(D0, fan_state); // Turn fan ON
          break;
        case ButtonId::PRESS:
          // Formerly used to toggle modes.
          // Does nothing at the moment.
          break;
        case ButtonId::LEFT:
          switch (screen_mode) {
            case INFERENCE_RESULTS:
              screen_mode = SENSORS;
              break;
          }
          break;
        case ButtonId::RIGHT:
          switch (screen_mode) {
            case SENSORS:
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
  Serial.begin(115200);

  Wire.begin();
  Wire1.begin();

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

  ButtonInit();

  pinMode(D0, OUTPUT);
  digitalWrite(D0, INITIAL_FAN_STATE);

  Serial.println("Setting up sensor 0x76 @ I2C1 (Wire)");
  setupBsec(sensor1, 0x76, Wire);
  //  delay(5500);
  Serial.println("Setting up sensor 0x77 @ I2C1 (Wire)");
  setupBsec(sensor2, 0x77, Wire);
  delay(100);

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
}

int fan = 0;

void loop()
{
  if (!sensor1.run())
  {
    checkBsecStatus(sensor1);
  }

  if (!sensor2.run())
  {
    checkBsecStatus(sensor2);
  }

  spr.fillSprite(BG_COLOR);

  ButtonDoWork();

  spr.setFreeFont(&FreeSansBoldOblique9pt7b); // Select the font
  spr.setTextColor(TEXT_COLOR);

  for (int i = NB_SENSORS - 1; i >= 0; i--) {
    float sensorVal = sensors[i].val;
    if (sensorVal > 999) {
      sensorVal = 999;
    }
    // sensors[i].last_val = sensorVal;

    if (chart_series[i].size() == MAX_CHART_SIZE) {
      chart_series[i].pop();
    }
    chart_series[i].push(sensorVal);
  }

  switch (screen_mode) {
    case SENSORS: {
      for (int i = 0; i < NB_SENSORS; i++) {
        int x_ref = 60 + (i % 2 * 170);
        int y_ref = 50 + (i / 2 * 80);

        spr.setTextColor(TEXT_COLOR);
        spr.drawString(sensors[i].name, x_ref - 24, y_ref - 24, 1);
        spr.drawRoundRect(x_ref - 24, y_ref, 80, 40, 5, TEXT_COLOR);
        spr.drawFloat(chart_series[i].back(), 2, x_ref - 20, y_ref + 10, 1);
        spr.setTextColor(TFT_BLUE);
        spr.drawString(sensors[i].unit, x_ref + 36, y_ref + 10, 1);
      }

      spr.setTextColor(TFT_RED);
      spr.drawString(inference_result, 60 - 24, 180);
      break;
    }

    default: {
      break; // nothing
    }
  }

  spr.pushSprite(0, 0, TFT_MAGENTA);
}