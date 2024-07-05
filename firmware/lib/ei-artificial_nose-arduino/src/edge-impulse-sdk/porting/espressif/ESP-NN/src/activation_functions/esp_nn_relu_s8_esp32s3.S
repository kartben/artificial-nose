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


    .text
    .align  4
    .literal_position

# in place relu6 function. a2: data, a3: size
    # Program Unit: esp_nn_relu6_s8_esp32s3
    .type   esp_nn_relu6_s8_esp32s3, @function
    .align   4
    .global esp_nn_relu6_s8_esp32s3

esp_nn_relu6_s8_esp32s3:
    entry   a1,48                       #
    mov.n   a9,a2                       # [0], data
    mov.n   a7,a3                       # [1], size

 // process multiple of 16
    movi.n      a4,6                    # [4]
    s8i         a4,a1,0                     # [5]  six
    addi        a10,a3,-7                   # [2]
    ee.vldbc.8  q1,a1               # [6]  id:72 six+0x0
    blti        a3,16,.Lt_0_5634            # [7]

    srai        a8,a3,4                     # [0]
    ee.zero.q   q2                      # [1]
    loopgtz     a8,.LBB37_esp_nn_relu6_s8_esp32s3   # [3]

    ee.vld.128.ip   q0,a2,0             # [0*II+0]  id:73
    ee.vmax.s8      q0,q0,q2            # [0*II+2]
    ee.vmin.s8      q0,q0,q1            # [0*II+3]
    ee.vst.128.ip   q0,a2,16            # [0*II+4]  id:74
.LBB37_esp_nn_relu6_s8_esp32s3: # 0x34

    slli    a8,a8,4                     # [0]

 // remaining multiple of 8 data
    bge     a8,a10,.Lt_0_3586           # [1]

.Lt_0_3842: # 0x3a
    sub     a6,a7,a8                    # [0]
    srai    a6,a6,3                     # [1]
    loopgtz a6,.LBB52_esp_nn_relu6_s8_esp32s3   # [2]

    ee.vld.l.64.ip  q0,a2,0         # [0*II+0]  id:75
    ee.vmax.s8      q0,q0,q2            # [0*II+2]
    ee.vmin.s8      q0,q0,q1            # [0*II+3]
    ee.vst.l.64.ip  q0,a2,8         # [0*II+4]  id:76

.LBB52_esp_nn_relu6_s8_esp32s3: # 0x4f
    addx8   a8,a6,a8                    # [0]

.Lt_0_3586: # 0x52
 // process leftover
    bge     a8,a7,.Lt_0_6402            # [0]

.Lt_0_4866: # 0x55
    movi.n  a5,0                    # [0]
    sub     a3,a7,a8                    # [1]
    add.n   a2,a8,a9                    # [2]
    l8ui    a6,a2,0                     # [3]  id:78
    addi.n  a3,a3,-1                # [4]
    sext    a6,a6,7
    max     a6,a5,a6                    # [6]
    min     a6,a4,a6                    # [7]
    s8i     a6,a2,0                     # [8]  id:79

    loopgtz a3,.LBB67_esp_nn_relu6_s8_esp32s3   # [9]

    l8ui    a3,a2,1                     # [0*II+0]  id:78
    addi.n  a2,a2,1                 # [1*II+1]
    sext    a3,a3,7
    max     a3,a5,a3                    # [0*II+3]
    min     a3,a4,a3                    # [0*II+4]
    s8i     a3,a2,0                     # [0*II+5]  id:79
.LBB67_esp_nn_relu6_s8_esp32s3: # 0x81

.Lt_0_6402: # 0x83
    retw.n                          # [0]

.Lt_0_5634: # 0x85
    blti    a10,1,.Lt_0_5890            # [0]

    movi.n  a8,0                    # [0]
    ee.zero.q   q2                      # [1]
    j   .Lt_0_3842                      # [2]

.Lt_0_5890: # 0x90
    beqz.n  a3,.Lt_0_6402           # [0]

    movi.n  a8,0                    # [0]
    j   .Lt_0_4866                      # [1]

    .size   esp_nn_relu6_s8_esp32s3, . - esp_nn_relu6_s8_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
