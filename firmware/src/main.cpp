#include <Arduino.h>

#include <AceButton.h>
using namespace ace_button;

#include <artificial_nose_inference.h>
#include <CircularBuffer.h>

#include <Wire.h>
#include <Multichannel_Gas_GMXXX.h>
GAS_GMXXX<TwoWire> *gas = new GAS_GMXXX<TwoWire>();

#include <TFT_eSPI.h>
#include "seeed_line_chart.h"
TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft); // Sprite

#define DARK_BACKGROUND 0
#define TEXT_COLOR (DARK_BACKGROUND ? TFT_WHITE : TFT_BLACK)
#define BG_COLOR (DARK_BACKGROUND ? TFT_BLACK : TFT_WHITE)

#include "fonts/roboto_bold_28.h"

typedef uint32_t (GAS_GMXXX<TwoWire>::*sensorGetFn)();

typedef struct SENSOR_INFO
{
  char *name;
  char *unit;
  std::function<uint32_t()> readFn;
  uint16_t color;
} SENSOR_INFO;

SENSOR_INFO sensors[4] = {
    {"NO2",     "ppm", std::bind(&GAS_GMXXX<TwoWire>::getGM102B, gas), TFT_RED},
    {"C2H5NH",  "ppm", std::bind(&GAS_GMXXX<TwoWire>::getGM302B, gas), TFT_BLUE},
    {"VOC",     "ppm", std::bind(&GAS_GMXXX<TwoWire>::getGM502B, gas), TFT_PURPLE},
    {"CO",      "ppm", std::bind(&GAS_GMXXX<TwoWire>::getGM702B, gas), TFT_GREEN}};
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

#define MAX_CHART_SIZE 50
std::vector<doubles> chart_series = std::vector<doubles>(NB_SENSORS, doubles());

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
  LEFT,
  RIGHT,
  UP,
  DOWN,
  PRESS
};
static const int ButtonNumber = 8;
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
    case ButtonId::LEFT:
      switch (screen_mode)
      {
      case INFERENCE_RESULTS:
        screen_mode = GRAPH;
        break;
      case GRAPH:
        screen_mode = SENSORS;
        break;
      }
      break;
    case ButtonId::RIGHT:
      switch (screen_mode)
      {
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
  Buttons[static_cast<int>(ButtonId::A)].init(WIO_KEY_A, HIGH, static_cast<uint8_t>(ButtonId::A));
  Buttons[static_cast<int>(ButtonId::B)].init(WIO_KEY_B, HIGH, static_cast<uint8_t>(ButtonId::B));
  Buttons[static_cast<int>(ButtonId::C)].init(WIO_KEY_C, HIGH, static_cast<uint8_t>(ButtonId::C));
  Buttons[static_cast<int>(ButtonId::LEFT)].init(WIO_5S_LEFT, HIGH, static_cast<uint8_t>(ButtonId::LEFT));
  Buttons[static_cast<int>(ButtonId::RIGHT)].init(WIO_5S_RIGHT, HIGH, static_cast<uint8_t>(ButtonId::RIGHT));
  Buttons[static_cast<int>(ButtonId::UP)].init(WIO_5S_UP, HIGH, static_cast<uint8_t>(ButtonId::UP));
  Buttons[static_cast<int>(ButtonId::DOWN)].init(WIO_5S_DOWN, HIGH, static_cast<uint8_t>(ButtonId::DOWN));
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
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(D0, OUTPUT);
  digitalWrite(D0, INITIAL_FAN_STATE);

  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);

  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);

  ButtonInit();

  gas->begin(Wire, 0x08); // use the hardware I2C

  // put your setup code here, to run once:
  tft.begin();
  tft.setRotation(3);
  spr.createSprite(tft.width(), tft.height());
}

/**
* @brief      Printf function uses vsnprintf and output using Arduino Serial
*
* @param[in]  format     Variable argument list
*/
void ei_printf(const char *format, ...)
{
  static char print_buf[512] = {0};

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

  spr.fillSprite(BG_COLOR);

  spr.setFreeFont(&Roboto_Bold_28);
  spr.setTextColor(TEXT_COLOR);
  spr.drawString(title_text, 15, 10, 1);
  for (int8_t line_index = 0; line_index <= 2; line_index++)
  {
    spr.drawLine(0, 50 + line_index, tft.width(), 50 + line_index, TEXT_COLOR);
  }

  spr.setFreeFont(&FreeSansBoldOblique9pt7b); // Select the font
  spr.setTextColor(TEXT_COLOR);

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

    if (chart_series[i].size() == MAX_CHART_SIZE)
    {
      chart_series[i].pop();
    }
    chart_series[i].push(sensorVal);
    if (new_sampling_tick != -1)
    {
      buffer.unshift(sensorVal);
    }
  }

  switch (screen_mode)
  {
  case SENSORS:
  {
    for (int i = 0; i < NB_SENSORS; i++)
    {
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
  case GRAPH:
  {
    auto content = line_chart(10, 60); //(x,y) where the line graph begins
    content
        .height(tft.height() - 70 * 1.2)
        .width(tft.width() - content.x() * 2)
        .based_on(0.0)
        .show_circle(false)
        .value(chart_series)
        .x_role_color(TEXT_COLOR)
        .y_role_color(TEXT_COLOR)
        .x_tick_color(TEXT_COLOR)
        .y_tick_color(TEXT_COLOR)
        .x_auxi_role(dash_line().color(TFT_DARKGREY))
        .color(TFT_RED, TFT_BLUE, TFT_PURPLE, TFT_GREEN)
        .draw();


    for (int i = 0 ; i < NB_SENSORS ; i++) {
      spr.setFreeFont(&FreeSans9pt7b);
      spr.setTextColor(sensors[i].color);
      spr.setTextDatum(BC_DATUM);
      spr.drawString(sensors[i].name, 70 + (i * 70), 236, 1);
    }

    spr.setTextDatum(TL_DATUM); // reset to default

    break;
  }

  default:
  {
    break; // nothing
  }
  }

  if (mode == TRAINING)
  {
    ei_printf("%d,%d,%d,%d\n", (uint32_t)chart_series[0].back(), (uint32_t)chart_series[1].back(), (uint32_t)chart_series[2].back(), (uint32_t)chart_series[3].back());
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

        if (mode == INFERENCE && screen_mode == INFERENCE_RESULTS)
        {
          spr.drawString(lineBuffer, 10, lineNumber, 1); // Print the test text in the custom font
          lineNumber += 20;
        }
      }

#if EI_CLASSIFIER_HAS_ANOMALY == 1
      sprintf(lineBuffer, "    anomaly score: %.3f\n", result.anomaly);
      ei_printf(lineBuffer);
      if (mode == INFERENCE && screen_mode == INFERENCE_RESULTS)
      {
        spr.drawString(lineBuffer, 10, lineNumber + 15, 1); // Print the test text in the custom font
      }
#endif

      sprintf(title_text, "%s (%d%%)", result.classification[best_prediction].label, (int)(result.classification[best_prediction].value * 100));

      ei_printf("Best prediction: %s\n", title_text);
    }
  }

  spr.pushSprite(0, 0);


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
