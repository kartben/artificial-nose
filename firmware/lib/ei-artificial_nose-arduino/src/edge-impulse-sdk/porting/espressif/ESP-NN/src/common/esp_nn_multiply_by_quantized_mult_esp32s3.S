#include "edge-impulse-sdk/classifier/ei_classifier_config.h"
#if EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
// Copyright 2021-2022 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// the macro `use_nudge` enables adding rounding factor similar to tflite implementation
// this barely changes any accuracy
// keep this disabled for better performance

#ifndef SKIP_NUDGE
    # set SKIP_NUDGE flag for ~20% faster (but not bit-exact) quantisation
    .set use_nudge, 1
#endif

    .text
    .literal_position
    .literal    .nudge_val, 1073741824          # 1 << 30

    .type   esp_nn_multiply_by_quantized_mult_asm_esp32s3, @function
    .align   4
    .global esp_nn_multiply_by_quantized_mult_asm_esp32s3

esp_nn_multiply_by_quantized_mult_asm_esp32s3:  # 0x4
    # to_add = 4

    entry       a1,32
    wsr.sar     a3
    ee.zero.q   q2

    bltz        a3,     .skip_left_shift
    ee.vsl.32   q0,q0                   # [13]
.skip_left_shift:

    ssai    31                      # [15]

# move data to general purpose registers
    ee.movi.32.a    q0,a12,0            # [17]
    ee.movi.32.a    q0,a13,1            # [16]
    ee.movi.32.a    q0,a14,2            # [18]
    ee.movi.32.a    q0,a15,3            # [19]

.ifdef use_nudge
    l32r            a6,.nudge_val
.endif

# perform 64 bit mult
    mulsh   a4,a2,a12                   # [22]
    mulsh   a11,a2,a13                  # [23]
    mulsh   a10,a2,a14                  # [21]
    mulsh   a8,a2,a15                   # [20]
    mull    a12,a2,a12                  # [24]
    mull    a13,a2,a13                  # [25]
    mull    a14,a2,a14                  # [26]
    mull    a15,a2,a15                  # [27]

# add nudge_val and discard low31

.ifdef use_nudge
    add.n           a14,a6,a14                  # [41]
    saltu           a2,a14,a6                   # [44]
    add.n           a10,a10,a2                  # [45]

    add.n           a13,a6,a13                  # [47]
    saltu           a9,a13,a6                   # [50]
    add.n           a11,a11,a9                  # [51]
.endif

    src             a10,a10,a14                     # [88]
    src             a11,a11,a13                 # [78]
    ee.movi.32.q    q0,a10,2
    ee.movi.32.q    q0,a11,1

.ifdef use_nudge
    add.n           a15,a6,a15                  # [36]
    saltu           a2,a15,a6                   # [39]
    add.n           a8,a8,a2                    # [40]

    add.n           a12,a6,a12                  # [54]
    saltu           a10,a12,a6                  # [57]
    add.n           a4,a4,a10                   # [58]
.endif

    src             a8,a8,a15                  # [95]
    src             a4,a4,a12                  # [69] # discard lower 31 bits
    ee.movi.32.q    q0,a8,3
    ee.movi.32.q    q0,a4,0

    bgez    a3, .skip_div_by_power_of_2

    neg     a5,a3                       # [0]  right_shift/exponent = -shift
    ee.vcmp.lt.s32  q2,q0,q2        # [97]
    addi.n          a7,a5,-1                # [0]  exponent - 1
    ssl             a7                      # [1]
    movi.n          a6,1                    # [92]
    sll             a6,a6                   # [2]
    s32i.n          a6,a1,4                 # [3]  to_add
    addi.n          a4,a1,4                 # [94]  to_add_addr
    ee.vldbc.32     q1,a4           # [4]  id:148 to_add
    wsr.sar         a5
    ee.vadds.s32    q1,q1,q2
    ee.vadds.s32    q0,q0,q1
    ee.vsr.32       q0,q0

.skip_div_by_power_of_2:
    retw.n                          # [9]

    .size   esp_nn_multiply_by_quantized_mult_asm_esp32s3, . - esp_nn_multiply_by_quantized_mult_asm_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
