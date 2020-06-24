/* Edge Impulse inferencing library
 * Copyright (c) 2020 EdgeImpulse Inc.
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

typedef enum {
    EI_IMPULSE_OK = 0,
    EI_IMPULSE_ERROR_SHAPES_DONT_MATCH = -1,
    EI_IMPULSE_CANCELED = -2,
    EI_IMPULSE_TFLITE_ERROR = -3,
    EI_IMPULSE_DSP_ERROR = -5,
    EI_IMPULSE_TFLITE_ARENA_ALLOC_FAILED = -6,
    EI_IMPULSE_CUBEAI_ERROR = -7
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

#endif // _EI_CLASSIFIER_PORTING_H_
