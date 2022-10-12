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

#ifndef _EI_CLASSIFIER_CONFIG_H_
#define _EI_CLASSIFIER_CONFIG_H_

// clang-format off
#if EI_CLASSIFIER_TFLITE_ENABLE_SILABS_MVP == 1
    #define EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN      0
#endif

#ifndef EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN
#if defined(__MBED__)
    #include "mbed.h"
    #if (MBED_VERSION < MBED_ENCODE_VERSION(5, 7, 0))
        #define EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN      0
    #else
        #define EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN      1
    #endif // Mbed OS 5.7 version check

// __ARM_ARCH_PROFILE is a predefine of arm-gcc.  __TARGET_* is armcc
#elif __ARM_ARCH_PROFILE == 'M' || defined(__TARGET_CPU_CORTEX_M0) || defined(__TARGET_CPU_CORTEX_M0PLUS) || defined(__TARGET_CPU_CORTEX_M3) || defined(__TARGET_CPU_CORTEX_M4) || defined(__TARGET_CPU_CORTEX_M7) || defined(ARDUINO_NRF52_ADAFRUIT)
    #define EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN      1
#else
    #define EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN      0
#endif
#endif // EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN

// CMSIS-NN falls back to reference kernels when __ARM_FEATURE_DSP and __ARM_FEATURE_MVE are not defined
// we should never use those... So disable CMSIS-NN in that case and throw a warning
#if EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN == 1
    #if !defined(__ARM_FEATURE_DSP) && !defined(__ARM_FEATURE_MVE)
        #pragma message( \
            "CMSIS-NN enabled, but neither __ARM_FEATURE_DSP nor __ARM_FEATURE_MVE defined. Falling back.")
        #undef EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN
        #define EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN 0
    #endif
#endif // EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN == 1

#if EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN == 1
#define CMSIS_NN                    1
#endif

#ifndef EI_CLASSIFIER_TFLITE_ENABLE_ARC
#ifdef CPU_ARC
#define EI_CLASSIFIER_TFLITE_ENABLE_ARC             1
#else
#define EI_CLASSIFIER_TFLITE_ENABLE_ARC             0
#endif // CPU_ARC
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ARC

#ifndef EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN
    #if defined(ESP32)
        #define EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN      1
    #endif // ESP32 check
#endif

// no include checks in the compiler? then just include metadata and then ops_define (optional if on EON model)
#ifndef __has_include
    #include "model-parameters/model_metadata.h"
    #if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE) && (EI_CLASSIFIER_COMPILED == 1)
        #include "tflite-model/trained_model_ops_define.h"
    #endif
#else
    #if __has_include("tflite-model/trained_model_ops_define.h")
    #include "tflite-model/trained_model_ops_define.h"
    #endif
#endif // __has_include

// clang-format on
#endif // _EI_CLASSIFIER_CONFIG_H_
