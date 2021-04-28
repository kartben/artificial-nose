#pragma once

#include <string>
#include <vector>
#include <functional>
#include <azure/iot/az_iot_provisioning_client.h>

class EasyAziotDpsClient
{
public:
    EasyAziotDpsClient();
    EasyAziotDpsClient(const EasyAziotDpsClient&) = delete;
    EasyAziotDpsClient& operator=(const EasyAziotDpsClient&) = delete;

    int Init(const char* endpoint, const char* idScope, const char* registrationId);
    int SetSAS(const char* symmetricKey, const uint64_t& expirationEpochTime, std::function<std::string(const std::string& symmetricKey, const std::vector<uint8_t>& signature)> generateEncryptedSignature);

    const std::string& GetMqttUsername() const;
    const std::string& GetMqttClientId() const;
    const std::string& GetMqttPassword() const;

    std::string GetRegisterPublishTopic();
    std::string GetRegisterSubscribeTopic() const;
    int RegisterSubscribeWork(const char* topic, const std::vector<uint8_t>& payload);
    bool IsRegisterOperationCompleted();
    int GetWaitBeforeQueryStatusSeconds() const;
    std::string GetQueryStatusPublishTopic();

    bool IsAssigned() const;
    std::string GetHubHost();
    std::string GetDeviceId();

private:
    std::string Endpoint_;
    std::string IdScope_;
    std::string RegistrationId_;

    az_iot_provisioning_client ProvClient_;

    std::string MqttUsername_;
    std::string MqttClientId_;
    std::string MqttPassword_;

    bool ResponseValid_;
    std::string ResponseTopic_;             // DON'T REMOVE THIS CODE
    std::vector<uint8_t> ResponsePayload_;  // DON'T REMOVE THIS CODE
    az_iot_provisioning_client_register_response Response_;

    static az_iot_provisioning_client_operation_status GetOperationStatus(const az_iot_provisioning_client_register_response& response);

};
