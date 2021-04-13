#define USE_CLI
#define USE_DPS

#if defined(USE_CLI)

// Wi-Fi
#define IOT_CONFIG_WIFI_SSID				Storage::WiFiSSID.c_str()
#define IOT_CONFIG_WIFI_PASSWORD			Storage::WiFiPassword.c_str()

// Azure IoT Hub DPS
#define IOT_CONFIG_GLOBAL_DEVICE_ENDPOINT	"global.azure-devices-provisioning.net"
#define IOT_CONFIG_ID_SCOPE					Storage::IdScope
#define IOT_CONFIG_REGISTRATION_ID			Storage::RegistrationId
#define IOT_CONFIG_SYMMETRIC_KEY			Storage::SymmetricKey

#else // USE_CLI

// Wi-Fi
#define IOT_CONFIG_WIFI_SSID				"[wifi ssid]"
#define IOT_CONFIG_WIFI_PASSWORD			"[wifi password]"

#if !defined(USE_DPS)
// Azure IoT Hub
#define IOT_CONFIG_IOTHUB					"[Azure IoT Hub host name].azure-devices.net"
#define IOT_CONFIG_DEVICE_ID				"[device id]"
#define IOT_CONFIG_SYMMETRIC_KEY			"[symmetric key]"
#else // USE_DPS
// Azure IoT Hub DPS
#define IOT_CONFIG_GLOBAL_DEVICE_ENDPOINT	"global.azure-devices-provisioning.net"
#define IOT_CONFIG_ID_SCOPE					"[id scope]"
#define IOT_CONFIG_REGISTRATION_ID			"[registration id]"
#define IOT_CONFIG_SYMMETRIC_KEY			"[symmetric key]"
#endif // USE_DPS

#endif // USE_CLI

#define IOT_CONFIG_MODEL_ID					"dtmi:kartben:artificial_nose;1"

#define TOKEN_LIFESPAN                      3600

#define TELEMETRY_FREQUENCY_MILLISECS		10000
#define TELEMETRY_CO	    				"co"
#define TELEMETRY_VOC		    			"voc"
#define TELEMETRY_C2H5NH					"c2h5nh"
#define TELEMETRY_NO2    					"no2"
#define TELEMETRY_LIGHT                     "light"
#define TELEMETRY_RIGHT_BUTTON              "rightButton"
#define TELEMETRY_CENTER_BUTTON             "centerButton"
#define TELEMETRY_LEFT_BUTTON               "leftButton"
#define COMMAND_RING_BUZZER					"ringBuzzer"
