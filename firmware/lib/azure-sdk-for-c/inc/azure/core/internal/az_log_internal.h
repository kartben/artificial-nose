// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef _az_LOG_INTERNAL_H
#define _az_LOG_INTERNAL_H

#include <azure/core/az_log.h>
#include <azure/core/az_span.h>

#include <stdbool.h>

#include <azure/core/_az_cfg_prefix.h>

// If the user hasn't registered any classifications, then we log everything.

// Terminates the classification array passed to #_az_log_set_classifications().
#define _az_LOG_END_OF_LIST -1

/**
 * @brief Allows the application to specify which #az_log_classification types it is interested in
 * receiving.
 *
 * @param[in] classifications __[nullable]__ An array of #az_log_classification values, terminated
 * by #_az_LOG_END_OF_LIST.
 *
 * @details If no classifications are set (\p classifications is `NULL`), the application will
 * receive log messages for all #az_log_classification values.
 * @details If \p classifications is not `NULL`, it must point to an array of
 * #az_log_classification, terminated by #_az_LOG_END_OF_LIST.
 * @details In contrast to \p classifications being `NULL`, \p classifications pointing to an empty
 * array (which still should be terminated by #_az_LOG_END_OF_LIST), states that an application is
 * not interested in receiving any log messages.
 *
 * @warning Users must not change the \p classifications array elements, once it is passed to this
 * function. If \p classifications array is allocated on a stack, program behavior in multi-threaded
 * environment is undefined.
 */
#ifndef AZ_NO_LOGGING
void _az_log_set_classifications(az_log_classification const classifications[]);
#else
AZ_INLINE void _az_log_set_classifications(az_log_classification const classifications[])
{
  (void)classifications;
}
#endif // AZ_NO_LOGGING

#ifndef AZ_NO_LOGGING

bool _az_log_should_write(az_log_classification classification);
void _az_log_write(az_log_classification classification, az_span message);

#define _az_LOG_SHOULD_WRITE(classification) _az_log_should_write(classification)
#define _az_LOG_WRITE(classification, message) _az_log_write(classification, message)

#else

#define _az_LOG_SHOULD_WRITE(classification) false

#define _az_LOG_WRITE(classification, message)

#endif // AZ_NO_LOGGING

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_LOG_INTERNAL_H
