#pragma once

constexpr int DISPLAY_BRIGHTNESS = 127;         // 0-255

constexpr int TELEMETRY_INTERVAL = 60;   // [sec.]

extern const char DPS_GLOBAL_DEVICE_ENDPOINT_HOST[];
extern const char MODEL_ID[];

constexpr int MQTT_PACKET_SIZE = 1024;
constexpr int TOKEN_LIFESPAN = 1 * 60 * 60;     // [sec.]
constexpr float RECONNECT_RATE = 0.85;
constexpr int JSON_MAX_SIZE = 1024;

// Edge Impulse Ingestion Settings
#define EDGE_IMPULSE_API_KEY "oLQEPYfKqMqGM8r"
#define EDGE_IMPULSE_DEVICE_ID "13:51:F2:11:53:54:4C:32:57:20:20:20:FF:06:1F:08"
#define EDGE_IMPULSE_DEVICE_TYPE "DATA_FORWARDER"
#define EDGE_IMPULSE_LABEL "gas_batch_training"
#define EDGE_IMPULSE_SENSOR_INTERVAL_MS 100
#define EDGE_IMPULSE_BATCH_SIZE 100
#define EDGE_IMPULSE_INGESTION_HOST "ingestion.edgeimpulse.com"
#define EDGE_IMPULSE_INGESTION_PATH_TRAINING "/api/training/data"
