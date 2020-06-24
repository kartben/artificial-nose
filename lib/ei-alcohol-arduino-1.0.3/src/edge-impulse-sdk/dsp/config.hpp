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

#ifndef _EIDSP_CPP_CONFIG_H_
#define _EIDSP_CPP_CONFIG_H_

#ifndef EIDSP_USE_CMSIS_DSP
#if defined(__MBED__) || defined(__TARGET_CPU_CORTEX_M0) || defined(__TARGET_CPU_CORTEX_M0PLUS) || defined(__TARGET_CPU_CORTEX_M3) || defined(__TARGET_CPU_CORTEX_M4) || defined(__TARGET_CPU_CORTEX_M7) || defined(USE_HAL_DRIVER)
    // Mbed OS versions before 5.7 are not based on CMSIS5, disable CMSIS-DSP and CMSIS-NN instructions
    #if defined(__MBED__) && (MBED_VERSION < MBED_ENCODE_VERSION((5), (7), (0)))
        #define EIDSP_USE_CMSIS_DSP      0
    #else
        #define EIDSP_USE_CMSIS_DSP      1
    #endif // Mbed OS 5.7 version check
#else
    #define EIDSP_USE_CMSIS_DSP     0
#endif // Mbed / ARM Core check
#endif // ifndef EIDSP_USE_CMSIS_DSP

#if EIDSP_USE_CMSIS_DSP == 1
#define EIDSP_i16                q15_t
#define EIDSP_i8                 q7_t
#define ARM_MATH_ROUNDING        1
#else
#define EIDSP_i16                int16_t
#define EIDSP_i8                 int8_t
#endif // EIDSP_USE_CMSIS_DSP

#ifndef EIDSP_USE_ASSERTS
#define EIDSP_USE_ASSERTS        1
#endif // EIDSP_USE_ASSERTS

#if EIDSP_USE_ASSERTS == 1
#include <assert.h>
#define EIDSP_ERR(err_code) printf("ERR: %d (%s)\n", err_code, #err_code); assert(false)
#else // EIDSP_USE_ASSERTS == 0
#define EIDSP_ERR(err_code) return(err_code)
#endif

// To save memory you can quantize the filterbanks,
// this has an effect on runtime speed as CMSIS-DSP does not have optimized instructions
// for q7 matrix multiplication and matrix transformation...
#ifndef EIDSP_QUANTIZE_FILTERBANK
#define EIDSP_QUANTIZE_FILTERBANK    1
#endif // EIDSP_QUANTIZE_FILTERBANK

// prints buffer allocations to stdout, useful when debugging
#ifndef EIDSP_TRACK_ALLOCATIONS
#define EIDSP_TRACK_ALLOCATIONS      0
#endif // EIDSP_TRACK_ALLOCATIONS

#ifndef EIDSP_SIGNAL_C_FN_POINTER
#define EIDSP_SIGNAL_C_FN_POINTER    0
#endif // EIDSP_SIGNAL_C_FN_POINTER

#endif // _EIDSP_CPP_CONFIG_H_
