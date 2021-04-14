#pragma once

constexpr int DISPLAY_BRIGHTNESS = 127;         // 0-255

constexpr int TELEMETRY_INTERVAL = 60;   // [sec.]

extern const char DPS_GLOBAL_DEVICE_ENDPOINT_HOST[];
extern const char MODEL_ID[];

constexpr int MQTT_PACKET_SIZE = 1024;
constexpr int TOKEN_LIFESPAN = 1 * 60 * 60;     // [sec.]
constexpr float RECONNECT_RATE = 0.85;
constexpr int JSON_MAX_SIZE = 1024;
