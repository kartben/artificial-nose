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

// quantisation version where we deal with different shifts and mults.

    .set use_nudge, 1

    .text
    .literal_position
    .literal    .LC3_19_48, 1073741824

    # Program Unit: esp_nn_multiply_by_quantized_mult_ver1_esp32s3
    .type   esp_nn_multiply_by_quantized_mult_ver1_esp32s3, @function
    .align   4
    .global esp_nn_multiply_by_quantized_mult_ver1_esp32s3

esp_nn_multiply_by_quantized_mult_ver1_esp32s3:    # 0x1ee
    entry       a1,32                       #
    ee.zero.q   q3                      # [0]
    l32i.n      a8,a3,0                 # [5]  id:200 // shift0
    l32i.n      a7,a3,4                 # [2]  id:201 // shift1
    l32i.n      a12,a2,0                # [3]  id:204 // mult0
    l32i.n      a15,a2,4                # [1]  id:205 // mult1
    movi.n      a10,0                   # [7]

    max             a6,a10,a8                   # [1] // left_shift0
    max             a5,a10,a7                   # [7] // left_shift1
    sub             a8,a6,a8                    # [2] // right_shift0
    sub             a7,a5,a7                    # [8] // right_shift1

    ee.movi.32.a    q0,a9,0             # [4]
    ee.movi.32.a    q0,a11,1            # [11]
    ssl             a6                          # [3]
    sll             a9,a9                       # [4]
    mulsh           a4,a12,a9                   # [6]
    mull            a12,a12,a9                  # [9]
    ssl             a5                          # [10]
    sll             a11,a11                         # [12]
    mulsh           a14,a15,a11                 # [14]
    mull            a15,a15,a11                 # [16]
    l32r            a13,.LC3_19_48              # [23]

    ee.movi.32.q    q0,a9,0             # [5]
    ee.movi.32.q    q0,a11,1            # [15]


    l32i.n          a6,a3,8                 # [6]  id:202 // shift2
    l32i.n          a9,a2,8                 # [19]  id:206 // mult2
    max             a5,a10,a6                   # [0] // left_shift2
    sub             a6,a5,a6                    # [24] // right_shift2


    ee.movi.32.a    q0,a11,2            # [17]
    ssl             a5                          # [13]
    sll             a11,a11                     # [18]
    ee.movi.32.q    q0,a11,2            # [20]
    mulsh           a5,a9,a11                  # [21]
    mull            a9,a9,a11                   # [22]
    mov             a11, a5

// add nudge to result0 & result1
    add.n           a12,a13,a12                 # [25]
    saltu           a5,a12,a13                  # [26]
    add.n           a15,a13,a15                 # [27]
    add.n           a5,a5,a4                    # [28]
    saltu           a4,a15,a13                  # [29]
    add.n           a4,a4,a14                   # [30]


    l32i.n          a14,a3,12               # [31]  id:203 // shift3
    add.n           a9,a13,a9                   # [32] // add nudge low2
    max             a10,a10,a14                 # [33]  // left_shift3
    sub             a14,a10,a14                 # [34]  // right_shift3
    ssl             a10                         # [35]
    ee.movi.32.a    q0,a10,3            # [36]
    sll             a10,a10                     # [37]

// select high32 from result0 and resul1
    ssai            31                          # [39]
    src             a5,a5,a12                   # [40]
    src             a4,a4,a15                   # [41]
    movi.n          a12,1                   # [42]
    ee.movi.32.q    q0,a5,0             # [43]
    saltu           a15,a9,a13                  # [44]
    add.n           a15,a15,a11                 # [45]
    ee.movi.32.q    q0,a4,1             # [46]
    l32i.n          a11,a2,12               # [47]  id:207 // mult3
    src             a15,a15,a9                  # [48]
    ee.movi.32.q    q0,a15,2            # [49]
    mull            a9,a11,a10                  # [50]
    mulsh           a11,a11,a10                 # [51]
    add.n           a9,a13,a9                   # [52]
    saltu           a13,a9,a13                  # [53]
    add.n           a13,a13,a11                 # [54]
    src             a13,a13,a9                  # [55]
    ee.movi.32.q    q0,a13,3            # [57]

// divide_by_power_of2_step
    ssl             a8                          # [56]
    sll             a9,a12                      # [58]
    ssl             a7                          # [59]
    addi.n          a9,a9,-1                # [60]
    ee.movi.32.q    q2,a9,0             # [61]
    sll             a11,a12                     # [62]
    addi.n          a11,a11,-1              # [63]
    ssl             a6                          # [64]
    sll             a10,a12                     # [65]
    ee.movi.32.q    q2,a11,1            # [66]
    ssl             a14                         # [67]
    addi.n          a10,a10,-1              # [68]
    ee.movi.32.q    q2,a10,2            # [69]
    sll             a9,a12                      # [70]
    addi.n          a9,a9,-1                # [71]
    ee.movi.32.q    q2,a9,3             # [74]
    ee.andq         q1,q0,q2                # [75]

    ssr             a8                          # [72]
    sra             a5,a5                       # [73]
    ssr             a7                          # [76]
    sra             a4,a4                       # [78]
    ssr             a6                          # [79]
    sra             a15,a15                     # [81]
    ssr             a14                         # [82]
    sra             a13,a13                     # [84]
    wsr.sar         a12                     # [85]

    ee.movi.32.q    q7,a5,0             # [77]
    ee.movi.32.q    q7,a4,1             # [80]
    ee.movi.32.q    q7,a15,2            # [83]
    ee.movi.32.q    q7,a13,3            # [86]

    ee.vcmp.lt.s32  q3,q7,q3        # [87]
    ee.vsr.32       q2,q2                   # [88]
    ee.vsubs.s32    q2,q2,q3            # [89]
    ee.vcmp.gt.s32  q1,q1,q2        # [90]
    ee.vsubs.s32    q0,q7,q1            # [91]

// return
    retw.n                          # [92]

    .size   esp_nn_multiply_by_quantized_mult_ver1_esp32s3, . - esp_nn_multiply_by_quantized_mult_ver1_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
