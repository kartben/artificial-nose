/* Edge Impulse inferencing library
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _EI_LOGGING_H_
#define _EI_LOGGING_H_

#include <stdint.h>
#include <stdarg.h>

#include "ei_classifier_porting.h"

#define EI_LOG_LEVEL_NONE 0 /*!< No log output */
#define EI_LOG_LEVEL_ERROR 1 /*!< Critical errors, software module can not recover on its own */
#define EI_LOG_LEVEL_WARNING 2 /*!< Error conditions from which recovery measures have been taken */
#define EI_LOG_LEVEL_INFO 3 /*!< Information messages which describe normal flow of events */
#define EI_LOG_LEVEL_DEBUG 4 /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */

// if we do not want ANY logging, setting EI_LOG_LEVEL to EI_LOG_LEVEL_NONE
// will not generate any code according to 
// https://stackoverflow.com/a/25021889

#define EI_LOGE(format, ...) (void)0
#define EI_LOGW(format, ...) (void)0
#define EI_LOGI(format, ...) (void)0
#define EI_LOGD(format, ...) (void)0

#ifndef EI_LOG_LEVEL
    #define EI_LOG_LEVEL EI_LOG_LEVEL_NONE
#endif

#if defined(__cplusplus) && EI_C_LINKAGE == 1
extern "C"
#endif // defined(__cplusplus) && EI_C_LINKAGE == 1

const char *debug_msgs[] =
{
    "NONE", // this one will never show
    "ERR",
    "WARNING",
    "INFO",
    "DEBUG"
};

#if EI_LOG_LEVEL >= EI_LOG_LEVEL_ERROR
    #define EI_LOGE(format, ...) ei_printf("%s: ",debug_msgs[EI_LOG_LEVEL_ERROR]); ei_printf(format, ##__VA_ARGS__);
#endif

#if EI_LOG_LEVEL >= EI_LOG_LEVEL_WARNING
    #define EI_LOGW(format, ...) ei_printf("%s: ",debug_msgs[EI_LOG_LEVEL_WARNING]); ei_printf(format, ##__VA_ARGS__);
#endif

#if EI_LOG_LEVEL >= EI_LOG_LEVEL_INFO
    #define EI_LOGI(format, ...) ei_printf("%s: ",debug_msgs[EI_LOG_LEVEL_INFO]); ei_printf(format, ##__VA_ARGS__);
#endif

#if EI_LOG_LEVEL >= EI_LOG_LEVEL_DEBUG
    #define EI_LOGD(format, ...) ei_printf("%s: ",debug_msgs[EI_LOG_LEVEL_DEBUG]); ei_printf(format, ##__VA_ARGS__);
#endif

#endif // _EI_LOGGING_H_