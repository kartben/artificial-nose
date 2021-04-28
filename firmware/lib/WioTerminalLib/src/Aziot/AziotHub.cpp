#include "Aziot/AziotHub.h"
#include <rpcWiFiClientSecure.h>
#include <PubSubClient.h>
#include <Network/Certificates.h>
#include <Network/Signature.h>

static WiFiClientSecure Tcp_;	// TODO
static PubSubClient Mqtt_(Tcp_);

std::function<void(const char* json, const char* requestId)> AziotHub::ReceivedTwinDocumentCallback;
std::function<void(const char* json, const char* version)> AziotHub::ReceivedTwinDesiredPatchCallback;
EasyAziotHubClient AziotHub::HubClient_;

AziotHub::AziotHub() :
    MqttPacketSize_(256)
{
}

void AziotHub::SetMqttPacketSize(int size)
{
    MqttPacketSize_ = size;
}

void AziotHub::DoWork()
{
    Mqtt_.loop();
}

bool AziotHub::IsConnected()
{
    return Mqtt_.connected();
}

int AziotHub::Connect(const std::string& host, const std::string& deviceId, const std::string& symmetricKey, const std::string& modelId, const uint64_t& expirationEpochTime)
{
    if (HubClient_.Init(host.c_str(), deviceId.c_str(),  modelId.c_str()) != 0) return -1;
    if (HubClient_.SetSAS(symmetricKey.c_str(), expirationEpochTime, GenerateEncryptedSignature) != 0) return -2;

    Serial.println("Hub:");
    Serial.print(" Host = ");
    Serial.println(host.c_str());
    Serial.print(" Device id = ");
    Serial.println(deviceId.c_str());
    Serial.print(" MQTT client id = ");
    Serial.println(HubClient_.GetMqttClientId().c_str());
    Serial.print(" MQTT username = ");
    Serial.println(HubClient_.GetMqttUsername().c_str());

    Tcp_.setCACert(CERT_BALTIMORE_CYBERTRUST_ROOT_CA);
    Mqtt_.setBufferSize(MqttPacketSize_);
    Mqtt_.setServer(host.c_str(), 8883);
    Mqtt_.setCallback(MqttSubscribeCallback);
    if (!Mqtt_.connect(HubClient_.GetMqttClientId().c_str(), HubClient_.GetMqttUsername().c_str(), HubClient_.GetMqttPassword().c_str())) return -3;

    Mqtt_.subscribe(AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_SUBSCRIBE_TOPIC);
    Mqtt_.subscribe(AZ_IOT_HUB_CLIENT_TWIN_PATCH_SUBSCRIBE_TOPIC);

    return 0;
}

void AziotHub::Disconnect()
{
    Mqtt_.disconnect();
}

void AziotHub::SendTelemetry(const char* payload, char* componentName)
{
    std::string telemetryTopic = HubClient_.GetTelemetryPublishTopic(componentName);

    static int sendCount = 0;
    if (!Mqtt_.publish(telemetryTopic.c_str(), payload, false))
    {
        // Serial.printf("ERROR: Send telemetry %d\n", sendCount);
        return; // TODO
    }
    else
    {
        ++sendCount;
        // Serial.printf("Sent telemetry %d\n", sendCount);
    }
}

void AziotHub::RequestTwinDocument(const char* requestId)
{
    Mqtt_.publish(HubClient_.GetTwinDocumentPublishTopic(requestId).c_str(), nullptr);
}

void AziotHub::SendTwinPatch(const char* requestId, const char* payload)
{
    Mqtt_.publish(HubClient_.GetTwinPatchPublishTopic(requestId).c_str(), payload);
}

void AziotHub::MqttSubscribeCallback(char* topic, uint8_t* payload, unsigned int length)
{
    // Serial.printf("Received twin\n");
    // Serial.printf(" topic  :%s\n", topic);
    // Serial.print("payload:");
    // for (int i = 0; i < static_cast<int>(length); i++) Serial.print(static_cast<char>(payload[i]));
    // Serial.println();

    EasyAziotHubClient::TwinResponse response;
    if (HubClient_.ParseTwinTopic(topic, response) == 0)
    {
        std::string json(reinterpret_cast<char*>(payload), reinterpret_cast<char*>(payload) + length);

        switch (response.ResponseType)
        {
        case AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_GET:
            if (ReceivedTwinDocumentCallback != nullptr) ReceivedTwinDocumentCallback(json.c_str(), response.RequestId.c_str());
            break;
        case AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_DESIRED_PROPERTIES:
            if (ReceivedTwinDesiredPatchCallback != nullptr) ReceivedTwinDesiredPatchCallback(json.c_str(), response.Version.c_str());
            break;
        case AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_REPORTED_PROPERTIES:
            break;
        }
    }
}
