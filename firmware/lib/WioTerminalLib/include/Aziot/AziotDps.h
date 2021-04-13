#pragma once

#include <string>
#include <Aziot/EasyAziotDpsClient.h>

class AziotDps
{
public:
    AziotDps();
    AziotDps(const AziotDps&) = delete;
    AziotDps& operator=(const AziotDps&) = delete;

    void SetMqttPacketSize(int size);

    int RegisterDevice(const std::string& endpointHost, const std::string& idScope, const std::string& registrationId, const std::string& symmetricKey, const std::string& modelId, const uint64_t& expirationEpochTime, std::string* hubHost, std::string* deviceId);

private:
    uint16_t MqttPacketSize_;

    static EasyAziotDpsClient DpsClient_;
    static unsigned long DpsPublishTimeOfQueryStatus_;
    static void MqttSubscribeCallback(char* topic, uint8_t* payload, unsigned int length);

};
