#pragma once

#include <cstddef>

constexpr size_t SIGNATURE_MAX_SIZE = 256;
constexpr size_t MQTT_USERNAME_MAX_SIZE = 256;
constexpr size_t MQTT_CLIENT_ID_MAX_SIZE = 128;
constexpr size_t MQTT_PASSWORD_MAX_SIZE = 300;

constexpr size_t REGISTER_PUBLISH_TOPIC_MAX_SIZE = 128;
constexpr size_t QUERY_STATUS_PUBLISH_TOPIC_MAX_SIZE = 256;

constexpr size_t TELEMETRY_PUBLISH_TOPIC_MAX_SIZE = 128;
constexpr size_t TWIN_DOCUMENT_PUBLISH_TOPIC_MAX_SIZE = 128;
constexpr size_t TWIN_PATCH_PUBLISH_TOPIC_MAX_SIZE = 128;
