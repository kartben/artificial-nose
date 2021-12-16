/* Edge Impulse inferencing library
 * Copyright (c) 2021 EdgeImpulse Inc.
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

#ifndef _EI_CLASSIFIER_PORTING_H_
#define _EI_CLASSIFIER_PORTING_H_

#include <stdint.h>
#include <stdlib.h>
#include "edge-impulse-sdk/tensorflow/lite/micro/debug_log.h"

#if defined(__cplusplus) && EI_C_LINKAGE == 1
extern "C" {
#endif // defined(__cplusplus)

typedef enum {
    EI_IMPULSE_OK = 0,
    EI_IMPULSE_ERROR_SHAPES_DONT_MATCH = -1,
    EI_IMPULSE_CANCELED = -2,
    EI_IMPULSE_TFLITE_ERROR = -3,
    EI_IMPULSE_DSP_ERROR = -5,
    EI_IMPULSE_TFLITE_ARENA_ALLOC_FAILED = -6,
    EI_IMPULSE_CUBEAI_ERROR = -7,
    EI_IMPULSE_ALLOC_FAILED = -8,
    EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES = -9,
    EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE = -10,
    EI_IMPULSE_OUT_OF_MEMORY = -11,
    EI_IMPULSE_NOT_SUPPORTED_WITH_I16 = -12,
    EI_IMPULSE_INPUT_TENSOR_WAS_NULL = -13,
    EI_IMPULSE_OUTPUT_TENSOR_WAS_NULL = -14,
    EI_IMPULSE_SCORE_TENSOR_WAS_NULL = -15,
    EI_IMPULSE_LABEL_TENSOR_WAS_NULL = -16,
    EI_IMPULSE_TENSORRT_INIT_FAILED = -17
} EI_IMPULSE_ERROR;

/**
 * Cancelable sleep, can be triggered with signal from other thread
 */
EI_IMPULSE_ERROR ei_sleep(int32_t time_ms);

/**
 * Check if the sampler thread was canceled, use this in conjunction with
 * the same signaling mechanism as ei_sleep
 */
EI_IMPULSE_ERROR ei_run_impulse_check_canceled();

/**
 * Read the millisecond timer
 */
uint64_t ei_read_timer_ms();

/**
 * Read the microsecond timer
 */
uint64_t ei_read_timer_us();

/**
 * Set Serial baudrate
 */
void ei_serial_set_baudrate(int baudrate);

/**
 * @brief      Connect to putchar of target
 *
 * @param[in]  c The chararater
 */
void ei_putchar(char c);

/**
 * Print wrapper around printf()
 * This is used internally to print debug information.
 */
void ei_printf(const char *format, ...);

/**
 * Override this function if your target cannot properly print floating points
 * If not overriden, this will be sent through `ei_printf()`.
 */
void ei_printf_float(float f);

/**
 * Wrapper around malloc
 */
void *ei_malloc(size_t size);

/**
 * Wrapper around calloc
 */
void *ei_calloc(size_t nitems, size_t size);

/**
 * Wrapper around free
 */
void ei_free(void *ptr);

#if defined(__cplusplus) && EI_C_LINKAGE == 1
}
#endif // defined(__cplusplus) && EI_C_LINKAGE == 1

// Load porting layer depending on target
#ifndef EI_PORTING_ARDUINO
#ifdef ARDUINO
#define EI_PORTING_ARDUINO      1
#else
#define EI_PORTING_ARDUINO      0
#endif
#endif

#ifndef EI_PORTING_ECM3532
#ifdef ECM3532
#define EI_PORTING_ECM3532      1
#else
#define EI_PORTING_ECM3532      0
#endif
#endif

#ifndef EI_PORTING_MBED
#ifdef __MBED__
#define EI_PORTING_MBED      1
#else
#define EI_PORTING_MBED      0
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

#ifndef EI_PORTING_ZEPHYR
#if defined(__ZEPHYR__)
#define EI_PORTING_ZEPHYR      1
#else
#define EI_PORTING_ZEPHYR      0
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

#endif // _EI_CLASSIFIER_PORTING_H_
