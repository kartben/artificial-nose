// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "az_span_private.h"
#include <azure/core/az_config.h>
#include <azure/core/az_http.h>
#include <azure/core/az_http_transport.h>
#include <azure/core/az_log.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_http_internal.h>
#include <azure/core/internal/az_log_internal.h>

#include <stddef.h>

#include <azure/core/_az_cfg.h>

#ifndef AZ_NO_LOGGING

static az_log_classification const* volatile _az_log_classifications = NULL;
static az_log_message_fn volatile _az_log_message_callback = NULL;

void _az_log_set_classifications(az_log_classification const classifications[])
{
  _az_log_classifications = classifications;
}

void az_log_set_message_callback(az_log_message_fn log_message_callback)
{
  _az_log_message_callback = log_message_callback;
}

// _az_LOG_WRITE_engine is a function private to this .c file; it contains the code to handle
// _az_LOG_SHOULD_WRITE & _az_LOG_WRITE.
//
// If log_it is false, then the function returns true or false indicating whether the message
// should be logged (without actually logging it).
//
// If log_it is true, then the function logs the message (if it should) and returns true or
// false indicating whether it was logged.
static bool _az_log_write_engine(bool log_it, az_log_classification classification, az_span message)
{
  // Copy the volatile fields to local variables so that they don't change within this function
  az_log_message_fn const callback = _az_log_message_callback;
  az_log_classification const* classifications = _az_log_classifications;

  if (callback == NULL)
  {
    // If no one is listening, don't attempt to log.
    return false;
  }

  if (classifications == NULL)
  {
    // If the user hasn't registered any classifications, then we log everything.
    classifications
        = &classification; // We don't need _az_LOG_END_OF_LIST to be there, as very first
                           // comparison is going to succeed and return from the function.
  }

  for (az_log_classification const* cls = classifications; *cls != _az_LOG_END_OF_LIST; ++cls)
  {
    // If this message's classification is in the customer-provided list, we should log it.
    if (*cls == classification)
    {
      if (log_it)
      {
        callback(classification, message);
      }

      return true;
    }
  }

  // This message's classification is not in the customer-provided list; we should not log it.
  return false;
}

// This function returns whether or not the passed-in message should be logged.
bool _az_log_should_write(az_log_classification classification)
{
  return _az_log_write_engine(false, classification, AZ_SPAN_EMPTY);
}

// This function attempts to log the passed-in message.
void _az_log_write(az_log_classification classification, az_span message)
{
  (void)_az_log_write_engine(true, classification, message);
}

#endif // AZ_NO_LOGGING
