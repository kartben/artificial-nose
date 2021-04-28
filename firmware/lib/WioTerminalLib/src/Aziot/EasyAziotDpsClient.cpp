#include "Aziot/EasyAziotConfig.h"
#include "Aziot/EasyAziotDpsClient.h"
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>

static inline const az_span az_span_create_from_string(const std::string& str)
{
    return az_span_create(reinterpret_cast<uint8_t*>(const_cast<char*>(str.c_str())), str.size());
}

EasyAziotDpsClient::EasyAziotDpsClient() :
    ResponseValid_(false)
{
}

int EasyAziotDpsClient::Init(const char* endpoint, const char* idScope, const char* registrationId)
{
    ResponseValid_ = false;

    Endpoint_ = endpoint;
    IdScope_ = idScope;
    RegistrationId_ = registrationId;

    {
        const az_span endpointSpan = az_span_create_from_string(Endpoint_);
        const az_span idScopeSpan = az_span_create_from_string(IdScope_);
        const az_span registrationIdSpan = az_span_create_from_string(RegistrationId_);
        if (az_result_failed(az_iot_provisioning_client_init(&ProvClient_, endpointSpan, idScopeSpan, registrationIdSpan, nullptr))) return -1; // SDK_API
    }

    {
        char mqttUsername[MQTT_USERNAME_MAX_SIZE];
        if (az_result_failed(az_iot_provisioning_client_get_user_name(&ProvClient_, mqttUsername, sizeof(mqttUsername), nullptr))) return -2;   // SDK_API
        MqttUsername_ = mqttUsername;
    }

    {
        char mqttClientId[MQTT_CLIENT_ID_MAX_SIZE];
        if (az_result_failed(az_iot_provisioning_client_get_client_id(&ProvClient_, mqttClientId, sizeof(mqttClientId), nullptr))) return -3;   // SDK_API
        MqttClientId_ = mqttClientId;
    }

    MqttPassword_.clear();

    return 0;
}

int EasyAziotDpsClient::SetSAS(const char* symmetricKey, const uint64_t& expirationEpochTime, std::function<std::string(const std::string& symmetricKey, const std::vector<uint8_t>& signature)> generateEncryptedSignature)
{
    ////////////////////
    // SAS auth

    std::string encryptedSignature;
    {
        std::vector<uint8_t> signature;
        {
            uint8_t signatureBuf[SIGNATURE_MAX_SIZE];
            const az_span signatureSpan = AZ_SPAN_FROM_BUFFER(signatureBuf);
            az_span signatureValidSpan;
            if (az_result_failed(az_iot_provisioning_client_sas_get_signature(&ProvClient_, expirationEpochTime, signatureSpan, &signatureValidSpan))) return -4;   // SDK_API
            signature.assign(az_span_ptr(signatureValidSpan), az_span_ptr(signatureValidSpan) + az_span_size(signatureValidSpan));
        }
        encryptedSignature = generateEncryptedSignature(symmetricKey, signature);
    }

    {
        char mqttPassword[MQTT_PASSWORD_MAX_SIZE];
        const az_span encryptedSignatureSpan = az_span_create_from_string(encryptedSignature);
        if (az_result_failed(az_iot_provisioning_client_sas_get_password(&ProvClient_, encryptedSignatureSpan, expirationEpochTime, AZ_SPAN_EMPTY, mqttPassword, sizeof(mqttPassword), nullptr))) return -5;    // SDK_API
        MqttPassword_ = mqttPassword;
    }

    return 0;
}

const std::string& EasyAziotDpsClient::GetMqttUsername() const
{
    return MqttUsername_;
}

const std::string& EasyAziotDpsClient::GetMqttClientId() const
{
    return MqttClientId_;
}

const std::string& EasyAziotDpsClient::GetMqttPassword() const
{
    return MqttPassword_;
}

std::string EasyAziotDpsClient::GetRegisterPublishTopic()
{
    char registerPublishTopic[REGISTER_PUBLISH_TOPIC_MAX_SIZE];
    if (az_result_failed(az_iot_provisioning_client_register_get_publish_topic(&ProvClient_, registerPublishTopic, sizeof(registerPublishTopic), nullptr))) return std::string();   // SDK_API

    return registerPublishTopic;
}

std::string EasyAziotDpsClient::GetRegisterSubscribeTopic() const
{
    return AZ_IOT_PROVISIONING_CLIENT_REGISTER_SUBSCRIBE_TOPIC;
}

int EasyAziotDpsClient::RegisterSubscribeWork(const char* topic, const std::vector<uint8_t>& payload)
{
    ResponseValid_ = false;

    ResponseTopic_ = topic;         // DON'T REMOVE THIS CODE
    ResponsePayload_ = payload;     // DON'T REMOVE THIS CODE

    if (az_result_failed(az_iot_provisioning_client_parse_received_topic_and_payload(&ProvClient_, az_span_create_from_string(ResponseTopic_), az_span_create(const_cast<uint8_t*>(&ResponsePayload_[0]), ResponsePayload_.size()), &Response_))) return -6;    // SDK_API

    ResponseValid_ = true;

    return 0;
}

bool EasyAziotDpsClient::IsRegisterOperationCompleted()
{
    if (!ResponseValid_) return false;

    // TODO az_iot_provisioning_client_parse_operation_status?
    return az_iot_provisioning_client_operation_complete(GetOperationStatus(Response_));    // SDK_API
}

int EasyAziotDpsClient::GetWaitBeforeQueryStatusSeconds() const
{
    if (!ResponseValid_) return 0;

    return Response_.retry_after_seconds;
}

std::string EasyAziotDpsClient::GetQueryStatusPublishTopic()
{
    if (!ResponseValid_) return std::string();

    char queryStatusPublishTopic[QUERY_STATUS_PUBLISH_TOPIC_MAX_SIZE];
    if (az_result_failed(az_iot_provisioning_client_query_status_get_publish_topic(&ProvClient_, Response_.operation_id, queryStatusPublishTopic, sizeof(queryStatusPublishTopic), nullptr))) return std::string(); // SDK_API

    return queryStatusPublishTopic;
}

bool EasyAziotDpsClient::IsAssigned() const
{
    if (!ResponseValid_) return false;

    return GetOperationStatus(Response_) == AZ_IOT_PROVISIONING_STATUS_ASSIGNED;
}

std::string EasyAziotDpsClient::GetHubHost()
{
    if (!IsAssigned()) return std::string();

    const az_span& span = Response_.registration_state.assigned_hub_hostname;
    return std::string(az_span_ptr(span), az_span_ptr(span) + az_span_size(span));
}

std::string EasyAziotDpsClient::GetDeviceId()
{
    if (!IsAssigned()) return std::string();

    const az_span& span = Response_.registration_state.device_id;
    return std::string(az_span_ptr(span), az_span_ptr(span) + az_span_size(span));
}

az_iot_provisioning_client_operation_status EasyAziotDpsClient::GetOperationStatus(const az_iot_provisioning_client_register_response& response)
{
    return response.operation_status;
}
