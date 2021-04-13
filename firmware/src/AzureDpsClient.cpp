#include "AzureDpsClient.h"
#include <az_result.h>
#include <az_span.h>

static constexpr size_t SignatureMaxSize = 256;
static constexpr size_t MqttClientIdMaxSize = 128;
static constexpr size_t MqttUsernameMaxSize = 128;
static constexpr size_t MqttPasswordMaxSize = 300;
static constexpr size_t RegisterPublishTopicMaxSize = 128;
static constexpr size_t QueryStatusPublishTopicMaxSize = 256;

AzureDpsClient::AzureDpsClient() :
    ResponseValid{ false }
{
}

int AzureDpsClient::Init(const std::string& endpoint, const std::string& idScope, const std::string& registrationId)
{
    ResponseValid = false;

    Endpoint = endpoint;
    IdScope = idScope;
    RegistrationId = registrationId;

    const az_span endpointSpan{ az_span_create((uint8_t*)&Endpoint[0], Endpoint.size()) };
    const az_span idScopeSpan{ az_span_create((uint8_t*)&IdScope[0], IdScope.size()) };
    const az_span registrationIdSpan{ az_span_create((uint8_t*)&RegistrationId[0], RegistrationId.size()) };
    if (az_result_failed(az_iot_provisioning_client_init(&ProvClient, endpointSpan, idScopeSpan, registrationIdSpan, NULL))) return -1;

    return 0;
}

std::vector<uint8_t> AzureDpsClient::GetSignature(const uint64_t& expirationEpochTime)
{
    uint8_t signature[SignatureMaxSize];
    az_span signatureSpan = az_span_create(signature, sizeof(signature));
    az_span signatureValidSpan;
    if (az_result_failed(az_iot_provisioning_client_sas_get_signature(&ProvClient, expirationEpochTime, signatureSpan, &signatureValidSpan))) return std::vector<uint8_t>();

    return std::vector<uint8_t>(az_span_ptr(signatureValidSpan), az_span_ptr(signatureValidSpan) + az_span_size(signatureValidSpan));
}

std::string AzureDpsClient::GetMqttClientId()
{
    char mqttClientId[MqttClientIdMaxSize];
    if (az_result_failed(az_iot_provisioning_client_get_client_id(&ProvClient, mqttClientId, sizeof(mqttClientId), NULL))) return std::string();

    return mqttClientId;
}

std::string AzureDpsClient::GetMqttUsername()
{
    char mqttUsername[MqttUsernameMaxSize];
    if (az_result_failed(az_iot_provisioning_client_get_user_name(&ProvClient, mqttUsername, sizeof(mqttUsername), NULL))) return std::string();

    return mqttUsername;
}

std::string AzureDpsClient::GetMqttPassword(const std::string& encryptedSignature, const uint64_t& expirationEpochTime)
{
    char mqttPassword[MqttPasswordMaxSize];
    az_span encryptedSignatureSpan = az_span_create((uint8_t*)&encryptedSignature[0], encryptedSignature.size());
    if (az_result_failed(az_iot_provisioning_client_sas_get_password(&ProvClient, encryptedSignatureSpan, expirationEpochTime, AZ_SPAN_EMPTY, mqttPassword, sizeof(mqttPassword), NULL))) return std::string();

    return mqttPassword;
}

std::string AzureDpsClient::GetRegisterPublishTopic()
{
    char registerPublishTopic[RegisterPublishTopicMaxSize];
    if (az_result_failed(az_iot_provisioning_client_register_get_publish_topic(&ProvClient, registerPublishTopic, sizeof(registerPublishTopic), NULL))) return std::string();

    return registerPublishTopic;
}

std::string AzureDpsClient::GetRegisterSubscribeTopic() const
{
    return AZ_IOT_PROVISIONING_CLIENT_REGISTER_SUBSCRIBE_TOPIC;
}

int AzureDpsClient::RegisterSubscribeWork(const std::string& topic, const std::vector<uint8_t>& payload)
{
    ResponseValid = false;

    ResponseTopic = topic;
    ResponsePayload = payload;

    if (az_result_failed(az_iot_provisioning_client_parse_received_topic_and_payload(&ProvClient, az_span_create((uint8_t*)&ResponseTopic[0], ResponseTopic.size()), az_span_create((uint8_t*)&ResponsePayload[0], ResponsePayload.size()), &Response))) return -1;

    ResponseValid = true;

    return 0;
}

bool AzureDpsClient::IsRegisterOperationCompleted()
{
    if (!ResponseValid) return false;

    return az_iot_provisioning_client_operation_complete(GetOperationStatus(Response));
}

int AzureDpsClient::GetWaitBeforeQueryStatusSeconds() const
{
    if (!ResponseValid) return 0;

    return Response.retry_after_seconds;
}

std::string AzureDpsClient::GetQueryStatusPublishTopic()
{
    if (!ResponseValid) return std::string();

    char queryStatusPublishTopic[QueryStatusPublishTopicMaxSize];
    if (az_result_failed(az_iot_provisioning_client_query_status_get_publish_topic(&ProvClient, Response.operation_id, queryStatusPublishTopic, sizeof(queryStatusPublishTopic), NULL))) return std::string();

    return queryStatusPublishTopic;
}

bool AzureDpsClient::IsAssigned()
{
    if (!ResponseValid) return false;

    return GetOperationStatus(Response) == AZ_IOT_PROVISIONING_STATUS_ASSIGNED;
}

std::string AzureDpsClient::GetHubHost()
{
    if (!IsAssigned()) return std::string();

    const az_span& span{ Response.registration_state.assigned_hub_hostname };
    return std::string(az_span_ptr(span), az_span_ptr(span) + az_span_size(span));
}

std::string AzureDpsClient::GetDeviceId()
{
    if (!IsAssigned()) return std::string();

    const az_span& span{ Response.registration_state.device_id };
    return std::string(az_span_ptr(span), az_span_ptr(span) + az_span_size(span));
}

az_iot_provisioning_client_operation_status AzureDpsClient::GetOperationStatus(az_iot_provisioning_client_register_response& response)
{
    return response.operation_status;
}
