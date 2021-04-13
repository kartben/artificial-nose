#pragma once

#include <string>
#include <vector>
#include <az_iot_provisioning_client.h>

class AzureDpsClient
{
public:
    AzureDpsClient();
    AzureDpsClient(const AzureDpsClient&) = delete;
    AzureDpsClient& operator=(const AzureDpsClient&) = delete;

    std::string GetEndpoint() const { return Endpoint; }
    void SetEndpoint(const std::string& endpoint) { Endpoint = endpoint; }
    std::string GetIdScope() const { return IdScope; }
    void SetIdScope(const std::string& idScope) { IdScope = idScope; }
    std::string GetRegistrationId() const { return RegistrationId; }
    void SetRegistrationId(const std::string& registrationId) { RegistrationId = registrationId; }

    int Init(const std::string& endpoint, const std::string& idScope, const std::string& registrationId);

    std::vector<uint8_t> GetSignature(const uint64_t& expirationEpochTime);

    std::string GetMqttClientId();
    std::string GetMqttUsername();
    std::string GetMqttPassword(const std::string& encryptedSignature, const uint64_t& expirationEpochTime);

    std::string GetRegisterPublishTopic();
    std::string GetRegisterSubscribeTopic() const;
    int RegisterSubscribeWork(const std::string& topic, const std::vector<uint8_t>& payload);
    bool IsRegisterOperationCompleted();
    int GetWaitBeforeQueryStatusSeconds() const;
    std::string GetQueryStatusPublishTopic();

    bool IsAssigned();
    std::string GetHubHost();
    std::string GetDeviceId();

private:
    std::string Endpoint;
    std::string IdScope;
    std::string RegistrationId;

    az_iot_provisioning_client ProvClient;

    bool ResponseValid;
    std::string ResponseTopic;
    std::vector<uint8_t> ResponsePayload;
    az_iot_provisioning_client_register_response Response;

private:
    static az_iot_provisioning_client_operation_status GetOperationStatus(az_iot_provisioning_client_register_response& response);

};
