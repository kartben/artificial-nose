// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief This header defines the types and functions your application uses to be notified of Azure
 * SDK client library log messages.
 *
 * @details If you define the `AZ_NO_LOGGING` symbol when compiling the SDK code (or adding option
 * `-DLOGGING=OFF` with cmake), all of the Azure SDK logging functionality will be excluded, making
 * the resulting compiled code smaller and faster.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_LOG_H
#define _az_LOG_H

#include <azure/core/az_result.h>
#include <azure/core/az_span.h>

#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

/**
 * @brief Identifies the classifications of log messages produced by the SDK.
 *
 * @note See the following `az_log_classification` values from various headers:
 * - #az_log_classification_core
 * - #az_log_classification_iot
 */
typedef int32_t az_log_classification;

// az_log_classification Bits:
//   - 31 Always 0.
//   - 16..30 Facility.
//   - 0..15 Code.

#define _az_LOG_MAKE_CLASSIFICATION(facility, code) \
  ((az_log_classification)(((uint32_t)(facility) << 16U) | (uint32_t)(code)))

/**
 * @brief Identifies the #az_log_classification produced by the SDK Core.
 */
enum az_log_classification_core
{
  AZ_LOG_HTTP_REQUEST
  = _az_LOG_MAKE_CLASSIFICATION(_az_FACILITY_HTTP, 1), ///< HTTP request is about to be sent.

  AZ_LOG_HTTP_RESPONSE
  = _az_LOG_MAKE_CLASSIFICATION(_az_FACILITY_HTTP, 2), ///< HTTP response was received.

  AZ_LOG_HTTP_RETRY = _az_LOG_MAKE_CLASSIFICATION(
      _az_FACILITY_HTTP,
      3), ///< First HTTP request did not succeed and will be retried.
};

/**
 * @brief Defines the signature of the callback function that application developers must write in
 * order to receive Azure SDK log messages.
 *
 * @param[in] classification The log message's #az_log_classification.
 * @param[in] message The log message.
 */
typedef void (*az_log_message_fn)(az_log_classification classification, az_span message);

/**
 * @brief Sets the function that will be invoked to report an SDK log message.
 *
 * @param[in] log_message_callback __[nullable]__ A pointer to the function that will be invoked
 * when the SDK reports a log message. If `NULL`, no function will be invoked.
 */
#ifndef AZ_NO_LOGGING
void az_log_set_message_callback(az_log_message_fn log_message_callback);
#else
AZ_INLINE void az_log_set_message_callback(az_log_message_fn log_message_callback)
{
  (void)log_message_callback;
}
#endif // AZ_NO_LOGGING

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_LOG_H
