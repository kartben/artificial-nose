#include <Arduino.h>
#include "Config.h"
#include "ConfigurationMode.h"

#include <malta_bme680_inferencing.h>

////////////////////////////////////////////////////////////////////////////////
// Storage

#include <ExtFlashLoader.h>
#include "Storage.h"

static ExtFlashLoader::QSPIFlash Flash_;
static Storage Storage_(Flash_);

////////////////////////////////////////////////////////////////////////////////
// OTA
#include <OTA.h>

////////////////////////////////////////////////////////////////////////////////
// BME688
#include "bme688_helper.h"

static float featuresSensor1[10];
/**
 * @brief      Copy raw feature data in out_ptr
 *             Function called by inference library
 *
 * @param[in]  offset   The offset
 * @param[in]  length   The length
 * @param      out_ptr  The out pointer
 *
 * @return     0
 */
int raw_feature1_get_data(size_t offset, size_t length, float *out_ptr)
{
  memcpy(out_ptr, featuresSensor1 + offset, length * sizeof(float));

  return 0;
}

void bsecCallbackSensor1(const bme68x_data &input, const BsecOutput &outputs)
{
  if (!outputs.len)
    return;

  int gas_index = -1;
  float gas_resistance = -1;

  for (uint8_t i = 0; i < outputs.len; i++)
  {
    const bsec_output_t &output = outputs.outputs[i];

    switch (output.sensor_id)
    {
    case BSEC_OUTPUT_IAQ:
      // Serial.println("\tiaq = " + String(output.signal));
      // Serial.println("\tiaq accuracy = " + String((int)output.accuracy));
      break;
    case BSEC_OUTPUT_RAW_TEMPERATURE:
      // Serial.println("\ttemperature = " + String(output.signal));
      break;
    case BSEC_OUTPUT_RAW_PRESSURE:
      // Serial.println("\tpressure = " + String(output.signal));
      break;
    case BSEC_OUTPUT_RAW_HUMIDITY:
      // Serial.println("\thumidity = " + String(output.signal));
      break;
    case BSEC_OUTPUT_RAW_GAS:
      // Serial.println("\tgas resistance = " + String(output.signal));
      gas_resistance = output.signal;
      break;
    case BSEC_OUTPUT_RAW_GAS_INDEX:
      // Serial.println("\tgas index = " + String(output.signal));
      gas_index = output.signal;
      break;
    case BSEC_OUTPUT_STABILIZATION_STATUS:
      // Serial.println("\tstabilization status = " + String(output.signal));
      break;
    case BSEC_OUTPUT_RUN_IN_STATUS:
      // Serial.println("\trun in status = " + String(output.signal));
      break;
    case BSEC_OUTPUT_GAS_ESTIMATE_1:
    case BSEC_OUTPUT_GAS_ESTIMATE_2:
    case BSEC_OUTPUT_GAS_ESTIMATE_3:
    case BSEC_OUTPUT_GAS_ESTIMATE_4:
      // Serial.println("\tgas estimate " + String((int)(output.sensor_id + 1 - BSEC_OUTPUT_GAS_ESTIMATE_1)) + String(" = ") + String(output.signal) + " - accuracy = " + String((int)output.accuracy));
      break;
    default:
      break;
    }
  }

  featuresSensor1[gas_index] = gas_resistance; //( numpy::log10(gas_resistance) - 4) / 4;

  if (gas_index == 9)
  {

    if (sizeof(featuresSensor1) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE)
    {
      ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
                EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(featuresSensor1) / sizeof(float));
      delay(1000);
      return;
    }

    ei_impulse_result_t result = {0};

    // the features are stored into flash, and we don't want to load everything into RAM
    signal_t features_signal;
    features_signal.total_length = sizeof(featuresSensor1) / sizeof(featuresSensor1[0]);
    features_signal.get_data = &raw_feature1_get_data;

    // invoke the impulse
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
    ei_printf("run_classifier returned: %d\n", res);

    if (res != 0)
      return;

    // print the predictions
    ei_printf("Predictions ");
    ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
              result.timing.dsp, result.timing.classification, result.timing.anomaly);
    ei_printf(": \n");
    ei_printf("[");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
    {
      ei_printf("%.5f", result.classification[ix].value);
#if EI_CLASSIFIER_HAS_ANOMALY == 1
      ei_printf(", ");
#else
      if (ix != EI_CLASSIFIER_LABEL_COUNT - 1)
      {
        ei_printf(", ");
      }
#endif
    }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("%.3f", result.anomaly);
#endif
    ei_printf("]\n");

    // human-readable predictions
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
    {
      ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
    }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif
  }
}

Bsec sensor1(bsecCallbackSensor1);

static float featuresSensor2[10];
/**
 * @brief      Copy raw feature data in out_ptr
 *             Function called by inference library
 *
 * @param[in]  offset   The offset
 * @param[in]  length   The length
 * @param      out_ptr  The out pointer
 *
 * @return     0
 */
int raw_feature2_get_data(size_t offset, size_t length, float *out_ptr)
{
  memcpy(out_ptr, featuresSensor2 + offset, length * sizeof(float));

  return 0;
}

void bsecCallbackSensor2(const bme68x_data &input, const BsecOutput &outputs)
{
  if (!outputs.len)
    return;

  int gas_index = -1;
  float gas_resistance = -1;

  for (uint8_t i = 0; i < outputs.len; i++)
  {
    const bsec_output_t &output = outputs.outputs[i];

    switch (output.sensor_id)
    {
    case BSEC_OUTPUT_IAQ:
      // Serial.println("\tiaq = " + String(output.signal));
      // Serial.println("\tiaq accuracy = " + String((int)output.accuracy));
      break;
    case BSEC_OUTPUT_RAW_TEMPERATURE:
      // Serial.println("\ttemperature = " + String(output.signal));
      break;
    case BSEC_OUTPUT_RAW_PRESSURE:
      // Serial.println("\tpressure = " + String(output.signal));
      break;
    case BSEC_OUTPUT_RAW_HUMIDITY:
      // Serial.println("\thumidity = " + String(output.signal));
      break;
    case BSEC_OUTPUT_RAW_GAS:
      // Serial.println("\tgas resistance = " + String(output.signal));
      gas_resistance = output.signal;
      break;
    case BSEC_OUTPUT_RAW_GAS_INDEX:
      // Serial.println("\tgas index = " + String(output.signal));
      gas_index = output.signal;
      break;
    case BSEC_OUTPUT_STABILIZATION_STATUS:
      // Serial.println("\tstabilization status = " + String(output.signal));
      break;
    case BSEC_OUTPUT_RUN_IN_STATUS:
      // Serial.println("\trun in status = " + String(output.signal));
      break;
    case BSEC_OUTPUT_GAS_ESTIMATE_1:
    case BSEC_OUTPUT_GAS_ESTIMATE_2:
    case BSEC_OUTPUT_GAS_ESTIMATE_3:
    case BSEC_OUTPUT_GAS_ESTIMATE_4:
      // Serial.println("\tgas estimate " + String((int)(output.sensor_id + 1 - BSEC_OUTPUT_GAS_ESTIMATE_1)) + String(" = ") + String(output.signal) + " - accuracy = " + String((int)output.accuracy));
      break;
    default:
      break;
    }
  }

  featuresSensor2[gas_index] = gas_resistance; //( numpy::log10(gas_resistance) - 4) / 4;

  if (gas_index == 9)
  {

    if (sizeof(featuresSensor2) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE)
    {
      ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
                EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(featuresSensor2) / sizeof(float));
      delay(1000);
      return;
    }

    ei_impulse_result_t result = {0};

    // the features are stored into flash, and we don't want to load everything into RAM
    signal_t features_signal;
    features_signal.total_length = sizeof(featuresSensor2) / sizeof(featuresSensor2[0]);
    features_signal.get_data = &raw_feature2_get_data;

    // invoke the impulse
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, true /* debug */);
    ei_printf("run_classifier returned: %d\n", res);

    if (res != 0)
      return;

    // print the predictions
    ei_printf("Predictions ");
    ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
              result.timing.dsp, result.timing.classification, result.timing.anomaly);
    ei_printf(": \n");
    ei_printf("[");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
    {
      ei_printf("%.5f", result.classification[ix].value);
#if EI_CLASSIFIER_HAS_ANOMALY == 1
      ei_printf(", ");
#else
      if (ix != EI_CLASSIFIER_LABEL_COUNT - 1)
      {
        ei_printf(", ");
      }
#endif
    }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("%.3f", result.anomaly);
#endif
    ei_printf("]\n");

    // human-readable predictions
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
    {
      ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
    }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif
  }
}

Bsec sensor2(bsecCallbackSensor2);

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

#include <AceButton.h>
using namespace ace_button;

#include <CircularBuffer.h>
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

#include "images/icon_wifi.h"

#define USE_ICONS 0

#if USE_ICONS
const unsigned short* ICONS_MAP[] = { icon_ambient, icon_coffee, icon_whiskey };

#include "images/icon_ambient.h"
#include "images/icon_anomaly.h"
#include "images/icon_coffee.h"
#include "images/icon_no_anomaly.h"
#include "images/icon_whiskey.h"

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
  INFERENCE_RESULTS
};
enum SCREEN_MODE screen_mode = SENSORS;

int latest_inference_idx = -1;
float latest_inference_confidence_level = -1.;

#define MAX_CHART_SIZE 50
std::vector<doubles> chart_series = std::vector<doubles>(NB_SENSORS, doubles());

// Allocate a buffer for the values we'll read from the gas sensor
CircularBuffer<float, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE> buffer;

uint64_t next_sampling_tick = micros();

#define INITIAL_FAN_STATE LOW
static int fan_state = INITIAL_FAN_STATE;

static bool debug_nn = false; // Set this to true to see e.g. features generated
                              // from the raw signal

void draw_chart();

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
          mode = (mode == INFERENCE) ? TRAINING : INFERENCE;
          spr.pushSprite(0, 0);
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
  Storage_.Load();

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

  Serial.println("Setting up sensor 0x76 @ I2C1 (Wire)");
  setupBsec(sensor1, 0x76, Wire);
  delay(5500);
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
    uint32_t sensorVal = 123; // sensors[i].readFn();
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

  if (mode == TRAINING) {
    // ei_printf("%d,%d,%d,%d\n", sensors[0].last_val, sensors[1].last_val, sensors[2].last_val, sensors[3].last_val);
  } else { // INFERENCE

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
#if USE_ICONS
      if (mode == INFERENCE && screen_mode == INFERENCE_RESULTS) {
        spr.pushImage(160,
                      35,
                      130,
                      130,
                      (result.anomaly > 0.15) ? icon_anomaly : icon_no_anomaly);
      }
#endif
#endif

      sprintf(title_text,
              "%s (%d%%)",
              result.classification[best_prediction].label,
              (int)(result.classification[best_prediction].value * 100));

      ei_printf("Best prediction: %s\n", title_text);

      latest_inference_idx = best_prediction;
      latest_inference_confidence_level =
        result.classification[best_prediction].value;
    }
  }

  spr.pushSprite(0, 0, TFT_MAGENTA);
}