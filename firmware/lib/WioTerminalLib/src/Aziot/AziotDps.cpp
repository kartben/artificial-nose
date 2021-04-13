#include "Aziot/AziotDps.h"
#include <rpcWiFiClientSecure.h>
#include <PubSubClient.h>
#include <Network/Certificates.h>
#include <Network/Signature.h>

static WiFiClientSecure Tcp_;	// TODO

EasyAziotDpsClient AziotDps::DpsClient_;
unsigned long AziotDps::DpsPublishTimeOfQueryStatus_ = 0;

AziotDps::AziotDps() :
    MqttPacketSize_(256)
{
}

void AziotDps::SetMqttPacketSize(int size)
{
    MqttPacketSize_ = size;
}

int AziotDps::RegisterDevice(const std::string& endpointHost, const std::string& idScope, const std::string& registrationId, const std::string& symmetricKey, const std::string& modelId, const uint64_t& expirationEpochTime, std::string* hubHost, std::string* deviceId)
{
    std::string endpointAndPort = endpointHost;
    endpointAndPort += ":";
    endpointAndPort += std::to_string(8883);

    if (DpsClient_.Init(endpointAndPort.c_str(), idScope.c_str(), registrationId.c_str()) != 0) return -1;
    if (DpsClient_.SetSAS(symmetricKey.c_str(), expirationEpochTime, GenerateEncryptedSignature) != 0) return -2;

	PubSubClient mqtt(Tcp_);
    Tcp_.setCACert(CERT_BALTIMORE_CYBERTRUST_ROOT_CA);
    mqtt.setBufferSize(MqttPacketSize_);
    mqtt.setServer(endpointHost.c_str(), 8883);
    mqtt.setCallback(AziotDps::MqttSubscribeCallback);
    if (!mqtt.connect(DpsClient_.GetMqttClientId().c_str(), DpsClient_.GetMqttUsername().c_str(), DpsClient_.GetMqttPassword().c_str())) return -3;

    mqtt.subscribe(DpsClient_.GetRegisterSubscribeTopic().c_str());
    
    mqtt.publish(DpsClient_.GetRegisterPublishTopic().c_str(), String::format("{payload:{\"modelId\":\"%s\"}}", modelId.c_str()).c_str());

    while (!DpsClient_.IsRegisterOperationCompleted())
    {
        mqtt.loop();
        if (DpsPublishTimeOfQueryStatus_ > 0 && millis() >= DpsPublishTimeOfQueryStatus_)
        {
            mqtt.publish(DpsClient_.GetQueryStatusPublishTopic().c_str(), "");
            DpsPublishTimeOfQueryStatus_ = 0;
        }
    }

    if (!DpsClient_.IsAssigned()) return -4;

    mqtt.disconnect();

    *hubHost = DpsClient_.GetHubHost();
    *deviceId = DpsClient_.GetDeviceId();

    return 0;
}

void AziotDps::MqttSubscribeCallback(char* topic, uint8_t* payload, unsigned int length)
{
    if (DpsClient_.RegisterSubscribeWork(topic, std::vector<uint8_t>(payload, payload + length)) != 0)
    {
        Serial.printf("Failed to parse topic and/or payload\n");
        return;
    }

    if (!DpsClient_.IsRegisterOperationCompleted())
    {
        const int waitSeconds = DpsClient_.GetWaitBeforeQueryStatusSeconds();
        Serial.printf("Querying after %u  seconds...\n", waitSeconds);

        DpsPublishTimeOfQueryStatus_ = millis() + waitSeconds * 1000;
    }
}
