/*
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an "AS
 * IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _EI_CLASSIFIER_PORTING_H_
#define _EI_CLASSIFIER_PORTING_H_

#include <stdint.h>
#include <stdlib.h>
#include "edge-impulse-sdk/dsp/returntypes.h"

#if defined(__cplusplus) && EI_C_LINKAGE == 1
extern "C" {
#endif // defined(__cplusplus)

/* Private functions ------------------------------------------------------- */

EI_IMPULSE_ERROR ei_run_impulse_check_canceled();
void ei_serial_set_baudrate(int baudrate);

/* Public functions -------------------------------------------------------- */

/**
 * @defgroup ei_user_functions User-defined functions
 * 
 * These functions are required to be implemented by the user for the target platform.
 * See [this porting guide](https://docs.edgeimpulse.com/docs/edge-ai-hardware/porting-guide) for more information. They are declared internally in the Edge Impulse
 * C++ SDK library, and they must be defined by the user.
 * 
 * **Source**: [porting/ei_classifier_porting.h](https://github.com/edgeimpulse/inferencing-sdk-cpp/blob/master/porting/ei_classifier_porting.h)
 * 
 * **Examples**:
 * The following examples demonstrate possible implementations of this function for
 * various platforms. Note the `__attribute__((weak))` in most of the definitions, which
 * means that a user could override the implementation elsewhere in the program:
 * * [Arduino classifier porting](https://github.com/edgeimpulse/inferencing-sdk-cpp/blob/master/porting/arduino/ei_classifier_porting.cpp)
 * * [mbed classifier porting](https://github.com/edgeimpulse/inferencing-sdk-cpp/blob/master/porting/mbed/ei_classifier_porting.cpp)
 * * [POSIX classifier porting](https://github.com/edgeimpulse/inferencing-sdk-cpp/blob/master/porting/posix/ei_classifier_porting.cpp)
 * * [Silicon Labs classifier porting](https://github.com/edgeimpulse/inferencing-sdk-cpp/blob/master/porting/silabs/ei_classifier_porting.cpp)
 * * [STM32 classifier porting](https://github.com/edgeimpulse/inferencing-sdk-cpp/blob/master/porting/stm32-cubeai/ei_classifier_porting.cpp)
 * * [TI classifier porting](https://github.com/edgeimpulse/inferencing-sdk-cpp/blob/master/porting/ti/debug_log.cpp)
 * * [Zephyr classifier porting](https://github.com/edgeimpulse/inferencing-sdk-cpp/blob/master/porting/zephyr/ei_classifier_porting.cpp)
 * 
 * @addtogroup ei_user_functions
 * @{
 */

/**
 * Cancelable sleep, can be triggered with signal from other thread
 */
/**
 * @brief Cancellable sleep, can be triggered with signal from other thread
 * 
 * Allow the processor or thread to sleep or block for the given time.
 * 
 * @param[in] time_ms Time in milliseconds to sleep
 * 
 * @return `EI_IMPULSE_OK` if successful, error code otherwise
 */
EI_IMPULSE_ERROR ei_sleep(int32_t time_ms);

/**
 * Read the millisecond timer
 */
/**
 * @brief Read the millisecond timer
 * 
 * This function should return the number of milliseconds that have passed since the 
 * start of the program. If you do not need to determine the run times for DSP and 
 * inference blocks, you can simply return 0 from this function. Your impulse will still
 * work correctly without timing information.
 * 
 * @return The number of milliseconds that have passed since the start of the program
 */
uint64_t ei_read_timer_ms();

/**
 * @brief Read the microsecond timer
 * 
 * This function should return the number of milliseconds that have passed since the 
 * start of the program. If you do not need to determine the run times for DSP and 
 * inference blocks, you can simply return 0 from this function. Your impulse will still
 * work correctly without timing information.
 * 
 * @return The number of microseconds that have passed since the start of the program
 */
uint64_t ei_read_timer_us();

/**
 * @brief Send a single character to the serial port
 *
 * @param[in]  c The chararater to send
 */
void ei_putchar(char c);

/**
 * @brief Read a single character from the serial port
 * 
 * @return The character read from the serial port
 */
char ei_getchar(void);

/**
 * @brief Print wrapper around printf()
 * 
 * `ei_printf()` is declared internally to the Edge Impulse SDK library so that debugging
 * information (e.g. during inference) can be printed out. However, the function must be
 * defined by the user, as printing methods can change depending on the platform and use
 * case. For example, you may want to print debugging information to stdout in Linux or 
 * over a UART serial port on a microcontroller.
 * 
 * @param[in] format Pointer to a character array or string that should be printed
 * @param[in] ... Other optional arguments may be passed as necessary (e.g. handle to a 
 *  UART object). Note that any calls to `ei_printf()` from within the 
 *  *edge-impulse-sdk* library do not pass anything other than the `format` argument.
 */
__attribute__ ((format (printf, 1, 2)))
void ei_printf(const char *format, ...);

/**
 * @brief Used to print floating point numbers
 * 
 * Some platforms cannot handle directly printing floating point numbers (e.g. to a 
 * console or over a serial port). If your platform cannot directly print floats, 
 * provide an implementation of this function to print them as needed (for example,
 * construct a string containing scientific notation with integers and call 
 * `ei_printf()`).
 * 
 * If your platform can print floating point values, the easiest implementation of this
 * function is as follows:
 * 
 * ```
 * __attribute__((weak)) void ei_printf_float(float f) {
 *     printf("%f", f);
 * }
 * ```
 * 
 * @param[in] f The floating point number to print
 */
void ei_printf_float(float f);

/**
 * @brief Wrapper around malloc
 * 
 * This function should allocate `size` bytes and return a pointer to the allocated 
 * memory. In bare-metal implementations, it can simply be a wrapper for `malloc()`. For
 * example:
 * 
 * ```
 * __attribute__((weak)) void *ei_malloc(size_t size) {
 *     return malloc(size);
 * }
 * ```
 * 
 * If you intend to run your impulse in a multi-threaded environment, you will need to
 * ensure that your implementation of `ei_malloc()` is thread-safe. For example, if you
 * are using FreeRTOS, here is one possible implementation:
 * 
 * ```
 * __attribute__((weak)) void *ei_malloc(size_t size) {
 *     return pvPortMalloc(size);
 * }
 * ```
 * 
 * @param[in] size The number of bytes to allocate
 */
void *ei_malloc(size_t size);

/**
 * @brief Wrapper around calloc
 * 
 * This function should allocate `nitems * size` bytes and initialize all bytes in this
 * allocated memory to 0. It should return a pointer to the allocated memory. In 
 * bare-metal implementations, it can simply be a wrapper for `calloc()`. For example:
 * 
 * ```
 * __attribute__((weak)) void *ei_calloc(size_t nitems, size_t size) {
 *     return calloc(nitems, size);
 * }
 * ```
 * 
 * If you intend to run your impulse in a multi-threaded environment, you will need to
 * ensure that your implementation of `ei_calloc()` is thread-safe. For example, if you
 * are using FreeRTOS, here is one possible implementation:
 * 
 * ```
 * __attribute__((weak)) void *ei_calloc(size_t nitems, size_t size) {
 *     void *ptr = NULL;
 *     if (size > 0) {
 *         ptr = pvPortMalloc(nitems * size);
 *         if(ptr)
 *            memset(ptr, 0, (nitems * size));
 *     }
 *     return ptr;
 * }
 * ```
 * 
 * @param[in] nitems Number of blocks to allocate and clear
 * @param[in] size Size (in bytes) of each block
 */
void *ei_calloc(size_t nitems, size_t size);

/**
 * @brief Wrapper around free
 * 
 * This function should free the memory space pointed to by `ptr`. If `ptr` is `NULL`,
 * no operation should be performed. In bare-metal implementations, it can simply be a
 * wrapper for `free()`. For example:
 * 
 * ```
 * __attribute__((weak)) void ei_free(void *ptr) {
 *     free(ptr);
 * }
 * ```
 * 
 * If you intend to run your impulse in a multi-threaded environment, you will need to
 * ensure that your implementation of `ei_free()` is thread-safe. For example, if you 
 * are using FreeRTOS, here is one possible implementation:
 * 
 * ```
 * __attribute__((weak)) void ei_free(void *ptr) {
 *     pvPortFree(ptr);
 * }
 * ```
 * 
 * @param[in] ptr Pointer to the memory to free
 */
void ei_free(void *ptr);

/** @} */

#if defined(__cplusplus) && EI_C_LINKAGE == 1
}
#endif // defined(__cplusplus) && EI_C_LINKAGE == 1

// Load porting layer depending on target

// First check if any of the general frameworks or operating systems are supported/enabled
#ifndef EI_PORTING_ZEPHYR
#if defined(__ZEPHYR__)
#define EI_PORTING_ZEPHYR      1
#else
#define EI_PORTING_ZEPHYR      0
#endif
#endif

#ifndef EI_PORTING_ARDUINO
#ifdef ARDUINO
#define EI_PORTING_ARDUINO      1
#else
#define EI_PORTING_ARDUINO      0
#endif
#endif

#ifndef EI_PORTING_MBED
#ifdef __MBED__
#define EI_PORTING_MBED      1
#else
#define EI_PORTING_MBED      0
#endif
#endif

// Then check for target spcific build systems

#ifndef EI_PORTING_ESPRESSIF
#if ((defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3)) && EI_PORTING_ZEPHYR == 0)
#include "esp_idf_version.h"
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif
#define EI_PORTING_ESPRESSIF      1
#define EI_PORTING_ARDUINO        0
#else
#define EI_PORTING_ESPRESSIF     0
#endif
#endif

#ifndef EI_PORTING_POSIX
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define EI_PORTING_POSIX      1
#else
#define EI_PORTING_POSIX      0
#endif
#endif

#ifndef EI_PORTING_SILABS
#if defined(EFR32MG12P332F1024GL125)
#define EI_PORTING_SILABS      1
#else
#define EI_PORTING_SILABS      0
#endif
#endif

#ifndef EI_PORTING_RASPBERRY
#ifdef PICO_BOARD
#define EI_PORTING_RASPBERRY      1
#else
#define EI_PORTING_RASPBERRY      0
#endif
#endif


#ifndef EI_PORTING_STM32_CUBEAI
#if defined(USE_HAL_DRIVER) && !defined(__MBED__) && EI_PORTING_ZEPHYR == 0
#define EI_PORTING_STM32_CUBEAI      1
#else
#define EI_PORTING_STM32_CUBEAI      0
#endif
#endif

#ifndef EI_PORTING_HIMAX
#ifdef CPU_ARC
#define EI_PORTING_HIMAX        1
#else
#define EI_PORTING_HIMAX        0
#endif
#endif

#ifndef EI_PORTING_MINGW32
#ifdef __MINGW32__
#define EI_PORTING_MINGW32      1
#else
#define EI_PORTING_MINGW32      0
#endif
#endif
// End load porting layer depending on target

// Additional configuration for specific architecture
#if defined(__CORTEX_M)

#if (__CORTEX_M == 55U)
#define EI_MAX_OVERFLOW_BUFFER_COUNT	15
#endif

#if (__CORTEX_M == 85U)
#define EI_MAX_OVERFLOW_BUFFER_COUNT	50
#endif

#endif

#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define EI_MAX_OVERFLOW_BUFFER_COUNT	30
#endif

// End additional configuration

#endif // _EI_CLASSIFIER_PORTING_H_
