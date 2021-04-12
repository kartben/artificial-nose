// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "az_http_private.h"
#include <azure/core/az_credentials.h>
#include <azure/core/az_http.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_http_internal.h>
#include <azure/core/internal/az_result_internal.h>

#include <azure/core/_az_cfg.h>

static const az_span AZ_HTTP_HEADER_USER_AGENT = AZ_SPAN_LITERAL_FROM_STR("User-Agent");

AZ_NODISCARD az_result az_http_pipeline_policy_apiversion(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response)
{

  _az_http_policy_apiversion_options const* const options
      = (_az_http_policy_apiversion_options const*)ref_options;

  switch (options->_internal.option_location)
  {
    case _az_http_policy_apiversion_option_location_header:
      // Add the version as a header
      _az_RETURN_IF_FAILED(az_http_request_append_header(
          ref_request, options->_internal.name, options->_internal.version));
      break;
    case _az_http_policy_apiversion_option_location_queryparameter:
      // Add the version as a query parameter. This value doesn't need url-encoding. Use `true` for
      // url-encode to avoid encoding.
      _az_RETURN_IF_FAILED(az_http_request_set_query_parameter(
          ref_request, options->_internal.name, options->_internal.version, true));
      break;
    default:
      return AZ_ERROR_ARG;
  }

  return _az_http_pipeline_nextpolicy(ref_policies, ref_request, ref_response);
}

AZ_NODISCARD az_result az_http_pipeline_policy_telemetry(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response)
{

  _az_http_policy_telemetry_options* options = (_az_http_policy_telemetry_options*)(ref_options);

  _az_RETURN_IF_FAILED(
      az_http_request_append_header(ref_request, AZ_HTTP_HEADER_USER_AGENT, options->os));

  return _az_http_pipeline_nextpolicy(ref_policies, ref_request, ref_response);
}

AZ_NODISCARD az_result az_http_pipeline_policy_credential(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response)
{
  _az_credential* const credential = (_az_credential*)ref_options;
  _az_http_policy_process_fn const policy_credential_apply
      = credential == NULL ? NULL : credential->_internal.apply_credential_policy;

  if (credential == AZ_CREDENTIAL_ANONYMOUS || policy_credential_apply == NULL)
  {
    return _az_http_pipeline_nextpolicy(ref_policies, ref_request, ref_response);
  }

  return policy_credential_apply(ref_policies, credential, ref_request, ref_response);
}

AZ_NODISCARD az_result az_http_pipeline_policy_transport(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response)
{
  (void)ref_policies; // this is the last policy in the pipeline, we just void it
  (void)ref_options;

  // make sure the response is resetted
  _az_http_response_reset(ref_response);

  return az_http_client_send_request(ref_request, ref_response);
}
