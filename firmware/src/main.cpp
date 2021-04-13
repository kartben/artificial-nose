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

#include "config.h"
#include "Storage.h"
#include "Signature.h"
#include "AzureDpsClient.h"
#include "CliMode.h"
#include <rpcWiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTP.h>
#include <az_json.h>
#include <az_result.h>
#include <az_span.h>
#include <az_iot_hub_client.h>
#define MQTT_PACKET_SIZE 1024

WiFiClientSecure wifi_client;
PubSubClient mqtt_client(wifi_client);
WiFiUDP wifi_udp;
NTP ntp(wifi_udp);

const char* ROOT_CA_BALTIMORE =
"-----BEGIN CERTIFICATE-----\n"
"MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n"
"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n"
"VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n"
"DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n"
"ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n"
"VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n"
"mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n"
"IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n"
"mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n"
"XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n"
"dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n"
"jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n"
"BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n"
"DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n"
"9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n"
"jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n"
"Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n"
"ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n"
"R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n"
"-----END CERTIFICATE-----";

#define AZ_RETURN_IF_FAILED(exp) \
  do \
  { \
    az_result const _result = (exp); \
    if (az_result_failed(_result)) \
    { \
      return _result; \
    } \
  } while (0)

std::string HubHost;
std::string DeviceId;

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


////////////////////////////////////////////////////////////////////////////////
// Azure IoT DPS

static AzureDpsClient DpsClient;
static unsigned long DpsPublishTimeOfQueryStatus = 0;

static void MqttSubscribeCallbackDPS(char* topic, byte* payload, unsigned int length);

static int RegisterDeviceToDPS(const std::string& endpoint, const std::string& idScope, const std::string& registrationId, const std::string& symmetricKey, const uint64_t& expirationEpochTime, std::string* hubHost, std::string* deviceId)
{
    std::string endpointAndPort{ endpoint };
    endpointAndPort += ":";
    endpointAndPort += std::to_string(8883);

    if (DpsClient.Init(endpointAndPort, idScope, registrationId) != 0) return -1;

    const std::string mqttClientId = DpsClient.GetMqttClientId();
    const std::string mqttUsername = DpsClient.GetMqttUsername();

    const std::vector<uint8_t> signature = DpsClient.GetSignature(expirationEpochTime);
    const std::string encryptedSignature = GenerateEncryptedSignature(symmetricKey, signature);
    const std::string mqttPassword = DpsClient.GetMqttPassword(encryptedSignature, expirationEpochTime);

    const std::string registerPublishTopic = DpsClient.GetRegisterPublishTopic();
    const std::string registerSubscribeTopic = DpsClient.GetRegisterSubscribeTopic();

    ei_printf("DPS:\r\n");
    ei_printf(" Endpoint = %s\r\n", endpoint.c_str());
    ei_printf(" Id scope = %s\r\n", idScope.c_str());
    ei_printf(" Registration id = %s\r\n", registrationId.c_str());
    ei_printf(" MQTT client id = %s\r\n", mqttClientId.c_str());
    ei_printf(" MQTT username = %s\r\n", mqttUsername.c_str());
    //Log(" MQTT password = %s" DLM, mqttPassword.c_str());

    wifi_client.setCACert(ROOT_CA_BALTIMORE);
    mqtt_client.setBufferSize(MQTT_PACKET_SIZE);
    mqtt_client.setServer(endpoint.c_str(), 8883);
    mqtt_client.setCallback(MqttSubscribeCallbackDPS);
    ei_printf("Connecting to Azure IoT Hub DPS...\r\n");
    if (!mqtt_client.connect(mqttClientId.c_str(), mqttUsername.c_str(), mqttPassword.c_str())) return -2;

    mqtt_client.subscribe(registerSubscribeTopic.c_str());
    mqtt_client.publish(registerPublishTopic.c_str(), "{payload:{\"modelId\":\"" IOT_CONFIG_MODEL_ID "\"}}");

    while (!DpsClient.IsRegisterOperationCompleted())
    {
        mqtt_client.loop();
        if (DpsPublishTimeOfQueryStatus > 0 && millis() >= DpsPublishTimeOfQueryStatus)
        {
            mqtt_client.publish(DpsClient.GetQueryStatusPublishTopic().c_str(), "");
            ei_printf("Client sent operation query message\r\n");
            DpsPublishTimeOfQueryStatus = 0;
        }
    }

    if (!DpsClient.IsAssigned()) return -3;

    mqtt_client.disconnect();

    *hubHost = DpsClient.GetHubHost();
    *deviceId = DpsClient.GetDeviceId();

    ei_printf("Device provisioned:\r\n");
    ei_printf(" Hub host = %s\r\n", hubHost->c_str());
    ei_printf(" Device id = %s\r\n", deviceId->c_str());

    return 0;
}

static void MqttSubscribeCallbackDPS(char* topic, byte* payload, unsigned int length)
{
    ei_printf("Subscribe:\r\n %s\r\n %.*s\r\n", topic, length, (const char*)payload);

    if (DpsClient.RegisterSubscribeWork(topic, std::vector<uint8_t>(payload, payload + length)) != 0)
    {
        ei_printf("Failed to parse topic and/or payload\r\n");
        return;
    }

    if (!DpsClient.IsRegisterOperationCompleted())
    {
        const int waitSeconds = DpsClient.GetWaitBeforeQueryStatusSeconds();
        ei_printf("Querying after %u  seconds...\r\n", waitSeconds);

        DpsPublishTimeOfQueryStatus = millis() + waitSeconds * 1000;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Azure IoT Hub

static az_iot_hub_client HubClient;

static int SendCommandResponse(az_iot_hub_client_method_request* request, uint16_t status, az_span response);
static void MqttSubscribeCallbackHub(char* topic, byte* payload, unsigned int length);

static int ConnectToHub(az_iot_hub_client* iot_hub_client, const std::string& host, const std::string& deviceId, const std::string& symmetricKey, const uint64_t& expirationEpochTime)
{
    static std::string deviceIdCache;
    deviceIdCache = deviceId;

    const az_span hostSpan{ az_span_create((uint8_t*)&host[0], host.size()) };
    const az_span deviceIdSpan{ az_span_create((uint8_t*)&deviceIdCache[0], deviceIdCache.size()) };
    az_iot_hub_client_options options = az_iot_hub_client_options_default();
    options.model_id = AZ_SPAN_LITERAL_FROM_STR(IOT_CONFIG_MODEL_ID);
    if (az_result_failed(az_iot_hub_client_init(iot_hub_client, hostSpan, deviceIdSpan, &options))) return -1;

    char mqttClientId[128];
    size_t client_id_length;
    if (az_result_failed(az_iot_hub_client_get_client_id(iot_hub_client, mqttClientId, sizeof(mqttClientId), &client_id_length))) return -4;

    char mqttUsername[256];
    if (az_result_failed(az_iot_hub_client_get_user_name(iot_hub_client, mqttUsername, sizeof(mqttUsername), NULL))) return -5;

    char mqttPassword[300];
    uint8_t signatureBuf[256];
    az_span signatureSpan = az_span_create(signatureBuf, sizeof(signatureBuf));
    az_span signatureValidSpan;
    if (az_result_failed(az_iot_hub_client_sas_get_signature(iot_hub_client, expirationEpochTime, signatureSpan, &signatureValidSpan))) return -2;
    const std::vector<uint8_t> signature(az_span_ptr(signatureValidSpan), az_span_ptr(signatureValidSpan) + az_span_size(signatureValidSpan));
    const std::string encryptedSignature = GenerateEncryptedSignature(symmetricKey, signature);
    az_span encryptedSignatureSpan = az_span_create((uint8_t*)&encryptedSignature[0], encryptedSignature.size());
    if (az_result_failed(az_iot_hub_client_sas_get_password(iot_hub_client, expirationEpochTime, encryptedSignatureSpan, AZ_SPAN_EMPTY, mqttPassword, sizeof(mqttPassword), NULL))) return -3;

    ei_printf("Hub:\r\n");
    ei_printf(" Host = %s\r\n", host.c_str());
    ei_printf(" Device id = %s\r\n", deviceIdCache.c_str());
    ei_printf(" MQTT client id = %s\r\n", mqttClientId);
    ei_printf(" MQTT username = %s\r\n", mqttUsername);
    //Log(" MQTT password = %s" DLM, mqttPassword);

    wifi_client.setCACert(ROOT_CA_BALTIMORE);
    mqtt_client.setBufferSize(MQTT_PACKET_SIZE);
    mqtt_client.setServer(host.c_str(), 8883);
    mqtt_client.setCallback(MqttSubscribeCallbackHub);

    if (!mqtt_client.connect(mqttClientId, mqttUsername, mqttPassword)) return -6;

    mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_METHODS_SUBSCRIBE_TOPIC);
    mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC);

    return 0;
}


static az_result SendTelemetry()
{
    char telemetry_topic[128];
    if (az_result_failed(az_iot_hub_client_telemetry_get_publish_topic(&HubClient, NULL, telemetry_topic, sizeof(telemetry_topic), NULL)))
    {
        ei_printf("Failed az_iot_hub_client_telemetry_get_publish_topic\r\n");
        return AZ_ERROR_NOT_SUPPORTED;
    }

    az_json_writer json_builder;
    char telemetry_payload[200];
    AZ_RETURN_IF_FAILED(az_json_writer_init(&json_builder, AZ_SPAN_FROM_BUFFER(telemetry_payload), NULL));
    AZ_RETURN_IF_FAILED(az_json_writer_append_begin_object(&json_builder));
    AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR(TELEMETRY_NO2)));
    AZ_RETURN_IF_FAILED(az_json_writer_append_double(&json_builder, sensors[0].last_val, 3));
    AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR(TELEMETRY_C2H5NH)));
    AZ_RETURN_IF_FAILED(az_json_writer_append_double(&json_builder, sensors[1].last_val, 3));
    AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR(TELEMETRY_VOC)));
    AZ_RETURN_IF_FAILED(az_json_writer_append_double(&json_builder, sensors[2].last_val, 3));
    AZ_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_builder, AZ_SPAN_LITERAL_FROM_STR(TELEMETRY_CO)));
    AZ_RETURN_IF_FAILED(az_json_writer_append_int32(&json_builder, sensors[3].last_val));
    AZ_RETURN_IF_FAILED(az_json_writer_append_end_object(&json_builder));
    const az_span out_payload{ az_json_writer_get_bytes_used_in_destination(&json_builder) };

    ei_printf("Free mem: %d\r\n", freeMemory());


    static int sendCount = 0;
    if (!mqtt_client.publish(telemetry_topic, az_span_ptr(out_payload), az_span_size(out_payload), false))
    {
        ei_printf("ERROR: Send telemetry %d\r\n", sendCount);
    }
    else
    {
        ++sendCount;
        ei_printf("Sent telemetry %d\r\n", sendCount);
    }

    return AZ_OK;
}

static void MqttSubscribeCallbackHub(char* topic, byte* payload, unsigned int length)
{
    az_span topic_span = az_span_create((uint8_t *)topic, strlen(topic));
    az_iot_hub_client_method_request command_request;

    if (az_result_succeeded(az_iot_hub_client_methods_parse_received_topic(&HubClient, topic_span, &command_request)))
    {
        ei_printf("Command arrived!\r\n");
        // Determine if the command is supported and take appropriate actions
        
        // TODO re-enable
        // HandleCommandMessage(az_span_create(payload, length), &command_request);
    }

    ei_printf("\r\n");
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

    ////////////////////
    // Provisioning

#if defined(USE_CLI) || defined(USE_DPS)

    if (RegisterDeviceToDPS(IOT_CONFIG_GLOBAL_DEVICE_ENDPOINT, IOT_CONFIG_ID_SCOPE, IOT_CONFIG_REGISTRATION_ID, IOT_CONFIG_SYMMETRIC_KEY, ntp.epoch() + TOKEN_LIFESPAN, &HubHost, &DeviceId) != 0)
    {
        ei_printf("ERROR PROVISIONING\r\n");
        while(true) {}
    }

#else

    HubHost = IOT_CONFIG_IOTHUB;
    DeviceId = IOT_CONFIG_DEVICE_ID;

#endif // USE_CLI || USE_DPS

  
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


    static uint64_t reconnectTime;
    if (!mqtt_client.connected())
    {
        ei_printf("Connecting to Azure IoT Hub...\r\n");
        const uint64_t now = ntp.epoch();
        if (ConnectToHub(&HubClient, HubHost, DeviceId, IOT_CONFIG_SYMMETRIC_KEY, now + TOKEN_LIFESPAN) != 0)
        {
            ei_printf("> ERROR.\r\n");
            ei_printf("> ERROR. Status code =%d. Try again in 5 seconds.\r\n", mqtt_client.state());
            delay(5000);
            return;
        }

        ei_printf("> SUCCESS.\r\n");
        reconnectTime = now + TOKEN_LIFESPAN * 0.85;
    }
    else
    {
        if ((uint64_t)ntp.epoch() >= reconnectTime)
        {
            ei_printf("Disconnect\r\n");
            mqtt_client.disconnect();
            return;
        }

        mqtt_client.loop();

        static unsigned long nextTelemetrySendTime = 0;
        if (millis() > nextTelemetrySendTime)
        {
            SendTelemetry();
            nextTelemetrySendTime = millis() + TELEMETRY_FREQUENCY_MILLISECS;
        }

    }

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
