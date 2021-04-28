#pragma once

#include <string>
#include <functional>
#include <Aziot/EasyAziotHubClient.h>

class AziotHub
{
public:
    AziotHub();
    AziotHub(const AziotHub&) = delete;
    AziotHub& operator=(const AziotHub&) = delete;

    void SetMqttPacketSize(int size);

    void DoWork();
    bool IsConnected();
    int Connect(const std::string& host, const std::string& deviceId, const std::string& symmetricKey, const std::string& modelId, const uint64_t& expirationEpochTime);
    void Disconnect();
    void SendTelemetry(const char* payload, char* componentName);
    void RequestTwinDocument(const char* requestId);
    void SendTwinPatch(const char* requestId, const char* payload);

    static std::function<void(const char* json, const char* requestId)> ReceivedTwinDocumentCallback;
    static std::function<void(const char* json, const char* version)> ReceivedTwinDesiredPatchCallback;

private:
    uint16_t MqttPacketSize_;

    static EasyAziotHubClient HubClient_;
    static void MqttSubscribeCallback(char* topic, uint8_t* payload, unsigned int length);

};
