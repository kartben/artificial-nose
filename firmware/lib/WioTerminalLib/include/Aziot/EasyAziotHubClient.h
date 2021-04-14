#pragma once

#include <string>
#include <vector>
#include <functional>
#include <azure/iot/az_iot_hub_client.h>

class EasyAziotHubClient
{
public:
    struct TwinResponse
    {
        std::string RequestId;
        az_iot_status Status;
        az_iot_hub_client_twin_response_type ResponseType;
        std::string Version;
    };

public:
    EasyAziotHubClient();
    EasyAziotHubClient(const EasyAziotHubClient&) = delete;
    EasyAziotHubClient& operator=(const EasyAziotHubClient&) = delete;

    int Init(const char* host, const char* deviceId, const char* modelId);
    int SetSAS(const char* symmetricKey, const uint64_t& expirationEpochTime, std::function<std::string(const std::string& symmetricKey, const std::vector<uint8_t>& signature)> generateEncryptedSignature);

    const std::string& GetMqttUsername() const;
    const std::string& GetMqttClientId() const;
    const std::string& GetMqttPassword() const;

    std::string GetTelemetryPublishTopic(char * componentName);
    std::string GetTwinDocumentPublishTopic(const char* requestId);
    std::string GetTwinPatchPublishTopic(const char* requestId);

    int ParseTwinTopic(const char* topic, TwinResponse& twinResponse);

private:
    std::string Host_;
    std::string DeviceId_;
    std::string ModelId_;
    
    az_iot_hub_client HubClient_;

    std::string MqttUsername_;
    std::string MqttClientId_;
    std::string MqttPassword_;

};
