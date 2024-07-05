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

    # Program Unit: esp_nn_max_pool_s8_esp32s3
    .type   esp_nn_max_pool_s8_esp32s3, @function
    .align   4
    .global esp_nn_max_pool_s8_esp32s3

// no of channels must be multiple of 4

esp_nn_max_pool_s8_esp32s3: # 0x4
    # int8_min = 0
    # gra_spill_temp_0 = 4
    # gra_spill_temp_1 = 8
    # gra_spill_temp_2 = 12
    # gra_spill_temp_3 = 16
    # gra_spill_temp_4 = 20
    # gra_spill_temp_5 = 24
    # gra_spill_temp_6 = 28
    # gra_spill_temp_7 = 32
    # gra_spill_temp_8 = 36
    # gra_spill_temp_9 = 40
    # gra_spill_temp_10 = 44
    # gra_spill_temp_11 = 48
    # gra_spill_temp_12 = 52
    # gra_spill_temp_13 = 56
    # gra_spill_temp_14 = 60
    # gra_spill_temp_15 = 64
    # gra_spill_temp_16 = 68
    # gra_spill_temp_17 = 72
    # gra_spill_temp_18 = 76
    # gra_spill_temp_19 = 80
    # gra_spill_temp_20 = 84
    # gra_spill_temp_21 = 88
    # gra_spill_temp_22 = 92
    # gra_spill_temp_23 = 96

// a2: input
// a3: input_wd
// a4: input_ht
// a5: output
// a6: output_wd
// a7: output_ht
// on stack: stride_wd = 120
// on stack: stride_ht = 124
// on stack: filter_wd = 128
// on stack: filter_ht = 132
// on stack: pad_wd = 136
// on stack: pad_ht = 140
// on stack: activation_min
// on stack: activation_max
// on stack: channels


    entry   a1,120                      #
    mov.n   a12,a2                      # [0]
    s32i    a6,a1,4                 # [2]  gra_spill_temp_0
    s32i    a7,a1,68                    # [3]  gra_spill_temp_16
    mov.n   a11,a3                      # [4]
    s32i    a5,a1,96                    # [5]  gra_spill_temp_23

    l16ui   a5,a1,152                   # [6]  id:465 channels+0x0
    movi    a3,-128                     # [7]
    s32i.n  a3,a1,0                 # [1]  int8_min

    addi.n      a9,a1,148                   # [0]  activation_max
    addi.n      a15,a1,144                  # [1]  activation_min
    ee.vldbc.8  q3,a1               # [7]  id:473 int8_min+0x0
    ee.vldbc.8  q5,a15                  # [8]  id:470 activation_min+0x0
    ee.vldbc.8  q4,a9               # [9]  id:471 activation_max+0x0

    extui   a8,a5,0,3                   # [8]
    beqz.n  a8,.LBB3_esp_nn_max_pool_s8_esp32s3     # [9] // if (channels % 8 == 0)

    extui   a14,a5,0,2                  # [0]
    beqz.n  a14,.LBB25_esp_nn_max_pool_s8_esp32s3   # [1] // if (channels % 4 == 0)

    retw.n                          # [0]   // exit

.LBB3_esp_nn_max_pool_s8_esp32s3:   # 0x1c5 // if (channels % 8 == 0)

    l16ui   a15,a1,136                  # [1]  id:475 pad_wd+0x0
    l16ui   a14,a1,140                  # [4]  id:474 pad_ht+0x0
    movi.n  a8,0                    # [13]
    movi.n  a10,0                   # [15]
    s32i    a14,a1,44                   # [7]  gra_spill_temp_10
    neg     a15,a15                     # [12]
    mul16u  a9,a6,a5                # [14]
    neg     a14,a14                     # [16]
    s32i    a14,a1,92                   # [17]  gra_spill_temp_22
    s32i    a10,a1,52                   # [18]  gra_spill_temp_12
    s32i    a9,a1,60                    # [19]  gra_spill_temp_14
    s32i.n  a8,a1,36                # [16]  gra_spill_temp_8
    s32i    a15,a1,56                   # [21]  gra_spill_temp_13
    sub     a13,a4,a14                  # [22]
    s32i    a13,a1,48                   # [23]  gra_spill_temp_11
    sub     a15,a11,a15                 # [24]
    s32i.n  a15,a1,40               # [25]  gra_spill_temp_9

.Lt_0_21506:    # 0x229
    l32i    a8,a1,4                 # [0]  gra_spill_temp_0
    beqz.n  a8,.Lt_0_21762          # [2]

    movi.n  a10,0                   # [0]
    l32i    a9,a1,44                    # [1]  gra_spill_temp_10
    l32i.n  a15,a1,40               # [2]  gra_spill_temp_9
    l32i    a8,a1,52                    # [3]  gra_spill_temp_12
    l32i.n  a13,a1,136                  # [4]  ,pad_wd
    l32i    a14,a1,56                   # [5]  gra_spill_temp_13
    s32i    a14,a1,80                   # [6]  gra_spill_temp_19
    s32i    a13,a1,76                   # [7]  gra_spill_temp_18
    s32i    a8,a1,88                    # [8]  gra_spill_temp_21
    s32i    a15,a1,84                   # [9]  gra_spill_temp_20
    l32i    a8,a1,48                    # [10]  gra_spill_temp_11
    max     a9,a9,a10                   # [11]
    l32i    a15,a1,132                  # [12]  filter_ht
    s32i    a9,a1,8                 # [13]  gra_spill_temp_1
    movi.n  a9,0                    # [14]
    min     a15,a15,a8                  # [15]
    s32i    a9,a1,64                    # [16]  gra_spill_temp_15

.Lt_0_22274:    # 0x25d
    beqz.n  a5,.Lt_0_22530          # [0]

.LBB10_esp_nn_max_pool_s8_esp32s3:  # 0x25f
#<loop> Part of loop body line 46, head labeled .Lt_0_22274
    l32i    a6,a1,76                    # [0]  gra_spill_temp_18
    l32i    a13,a1,96                   # [1]  gra_spill_temp_23
    l32i    a8,a1,84                    # [2]  gra_spill_temp_20
    l32i    a7,a1,128                   # [3]  filter_wd
    l32i    a10,a1,88                   # [4]  gra_spill_temp_21
    movi.n  a9,0                    # [5]
    s32i    a9,a1,20                    # [6]  gra_spill_temp_4
    add.n   a14,a10,a5                  # [7]
    min     a7,a7,a8                    # [8]
    add.n   a10,a10,a13                 # [9]
    add.n   a14,a13,a14                 # [10]
    s32i    a14,a1,12                   # [11]  gra_spill_temp_2
    s32i    a10,a1,16                   # [12]  gra_spill_temp_3
    movi.n  a8,0                    # [13]
    l32i    a10,a1,80                   # [14]  gra_spill_temp_19
    max     a6,a6,a8                    # [15]
    sub     a9,a7,a6                    # [16]
    s32i    a9,a1,28                    # [17]  gra_spill_temp_6
    add.n   a13,a10,a6                  # [18]
    s32i    a13,a1,24                   # [19]  gra_spill_temp_5
    add.n   a10,a10,a7                  # [16]
    s32i    a10,a1,72                   # [21]  gra_spill_temp_17

.Lt_0_23042:    # 0x29a
    l32i    a8,a1,8                 # [0]  gra_spill_temp_1
    mv.qr   q1,q3                       # [1]
    mov.n   a13,a8                      # [2]
    bge     a8,a15,.Lt_0_23298          # [3]

.LBB13_esp_nn_max_pool_s8_esp32s3:  # 0x2a5
#<loop> Part of loop body line 40, head labeled .Lt_0_23042
    l32i    a10,a1,92                   # [0]  gra_spill_temp_22
    l32i    a14,a1,72                   # [1]  gra_spill_temp_17
    add.n   a10,a10,a8                  # [2]
    mull    a10,a11,a10                 # [3]
    add.n   a14,a10,a14                 # [5]

.Lt_0_23810:    # 0x2b2
    add.n   a14,a14,a11                 # [0]
    addi.n  a13,a13,1               # [1]
    bge     a6,a7,.Lt_0_24066           # [2]

.LBB16_esp_nn_max_pool_s8_esp32s3:  # 0x2b9
    l32i    a3,a1,24                    # [0]  gra_spill_temp_5
    l32i    a2,a1,20                    # [1]  gra_spill_temp_4
    add.n   a3,a3,a10                   # [2]
    mull    a3,a3,a5                    # [3]
    add.n   a2,a2,a3                    # [5]
    l32i    a3,a1,28                    # [6]  gra_spill_temp_6
    add.n   a2,a12,a2                   # [7]
    loopgtz a3,.LBB93_esp_nn_max_pool_s8_esp32s3    # [8]

    ee.vld.l.64.ip  q0,a2,0         # [0*II+1]  id:481
    add.n           a2,a2,a5                    # [0*II+2]
    ee.vmax.s8      q1,q1,q0            # [0*II+3]
.LBB93_esp_nn_max_pool_s8_esp32s3:  # 0x2d8

.Lt_0_24066:    # 0x2d8
    add.n   a10,a10,a11                 # [0]
    bne     a15,a13,.Lt_0_23810         # [1]

.Lt_0_23298:    # 0x2dd
    l32i    a9,a1,12                    # [0]  gra_spill_temp_2
    l32i    a13,a1,20                   # [1]  gra_spill_temp_4
    l32i    a8,a1,16                    # [2]  gra_spill_temp_3
    ee.vmin.s8  q2,q1,q4            # [3]
    ee.vmax.s8  q2,q2,q5            # [4]
    mov.n   a10,a8                      # [5]
    addi.n  a13,a13,8               # [6]
    s32i    a13,a1,20                   # [7]  gra_spill_temp_4
    ee.vst.l.64.ip  q2,a10,0        # [8]  id:482
    addi.n  a8,a8,8                 # [9]
    s32i    a8,a1,16                    # [10]  gra_spill_temp_3
    blt     a8,a9,.Lt_0_23042           # [11]

.Lt_0_22530:    # 0x2fe
    l32i    a13,a1,84                   # [0]  gra_spill_temp_20
    l32i    a14,a1,80                   # [1]  gra_spill_temp_19
    l32i    a10,a1,120                  # [2]  stride_wd
    l32i    a8,a1,88                    # [3]  gra_spill_temp_21
    l32i    a9,a1,76                    # [4]  gra_spill_temp_18
    add.n   a8,a8,a5                    # [5]
    s32i    a8,a1,88                    # [6]  gra_spill_temp_21
    sub     a9,a9,a10                   # [7]
    add.n   a14,a14,a10                 # [8]
    sub     a13,a13,a10                 # [9]
    s32i    a13,a1,84                   # [10]  gra_spill_temp_20
    s32i    a14,a1,80                   # [11]  gra_spill_temp_19
    s32i    a9,a1,76                    # [12]  gra_spill_temp_18
    l32i    a14,a1,64                   # [13]  gra_spill_temp_15
    l32i    a8,a1,4                 # [14]  gra_spill_temp_0
    addi.n  a14,a14,1               # [15]
    s32i    a14,a1,64                   # [16]  gra_spill_temp_15
    sub     a14,a14,a8                  # [17]
    bnez    a14,.Lt_0_22274             # [18]

.Lt_0_21762:    # 0x334
#<loop> Part of loop body line 20, head labeled .Lt_0_21506
    l32i    a8,a1,44                    # [0]  gra_spill_temp_10
    l32i    a15,a1,92                   # [1]  gra_spill_temp_22
    l32i    a10,a1,60                   # [2]  gra_spill_temp_14
    l32i    a14,a1,124                  # [3]  stride_ht
    l32i    a13,a1,48                   # [4]  gra_spill_temp_11
    l32i    a9,a1,52                    # [5]  gra_spill_temp_12
    sub     a13,a13,a14                 # [6]
    add.n   a9,a9,a10                   # [7]
    add.n   a15,a15,a14                 # [8]
    sub     a8,a8,a14                   # [9]
    s32i    a8,a1,44                    # [10]  gra_spill_temp_10
    s32i    a15,a1,92                   # [11]  gra_spill_temp_22
    s32i    a9,a1,52                    # [12]  gra_spill_temp_12
    s32i    a13,a1,48                   # [13]  gra_spill_temp_11
    l32i.n  a9,a1,36                # [14]  gra_spill_temp_8
    l32i    a10,a1,68                   # [15]  gra_spill_temp_16
    addi.n  a9,a9,1                 # [16]
    s32i.n  a9,a1,36                # [17]  gra_spill_temp_8
    sub     a9,a9,a10                   # [18]
    bnez    a9,.Lt_0_21506              # [19]

    retw.n                          # [0] // exit

.LBB25_esp_nn_max_pool_s8_esp32s3:  # 0x36d // if (channels % 4 == 0)

    l16ui   a10,a1,136                  # [1]  id:475 pad_wd+0x0
    l16ui   a9,a1,140                   # [4]  id:474 pad_ht+0x0
    movi.n  a13,0                   # [13]
    movi.n  a15,0                   # [15]
    neg     a10,a10                     # [12]
    s32i    a9,a1,44                    # [7]  gra_spill_temp_10
    mul16u  a14,a6,a5               # [14]
    neg     a9,a9                       # [16]
    s32i    a9,a1,92                    # [17]  gra_spill_temp_22
    s32i    a15,a1,52                   # [18]  gra_spill_temp_12
    s32i    a14,a1,60                   # [19]  gra_spill_temp_14
    s32i.n  a13,a1,36               # [16]  gra_spill_temp_8
    s32i    a10,a1,56                   # [21]  gra_spill_temp_13
    sub     a8,a4,a9                    # [22]
    s32i    a8,a1,48                    # [23]  gra_spill_temp_11
    sub     a10,a11,a10                 # [24]
    s32i.n  a10,a1,40               # [25]  gra_spill_temp_9

.Lt_0_27138:    # 0x3d5
    l32i    a13,a1,4                # [0]  gra_spill_temp_0
    beqz.n  a13,.Lt_0_27394         # [2]

.LBB29_esp_nn_max_pool_s8_esp32s3:  # 0x3da
#<loop> Part of loop body line 107, head labeled .Lt_0_27138
    movi.n  a10,0                   # [0]
    l32i    a9,a1,44                    # [1]  gra_spill_temp_10
    l32i.n  a15,a1,40               # [2]  gra_spill_temp_9
    l32i    a8,a1,52                    # [3]  gra_spill_temp_12
    l32i    a14,a1,56                   # [4]  gra_spill_temp_13
    l32i.n  a13,a1,136                  # [5]  pad_wd
    s32i    a13,a1,76                   # [6]  gra_spill_temp_18
    s32i    a14,a1,80                   # [7]  gra_spill_temp_19
    s32i    a8,a1,88                    # [8]  gra_spill_temp_21
    s32i    a15,a1,84                   # [9]  gra_spill_temp_20
    l32i    a8,a1,48                    # [10]  gra_spill_temp_11
    l32i    a15,a1,132                  # [11]  filter_ht
    movi.n  a14,0                   # [12]
    max     a9,a9,a10                   # [13]
    s32i    a9,a1,8                 # [14]  gra_spill_temp_1
    s32i    a14,a1,64                   # [15]  gra_spill_temp_15
    min     a15,a15,a8                  # [16]

.Lt_0_27906:    # 0x409
#<loop> Loop body line 109, nesting depth: 2, estimated iterations: 56
    beqz.n  a5,.Lt_0_28162          # [0]

.LBB32_esp_nn_max_pool_s8_esp32s3:  # 0x40b
#<loop> Part of loop body line 109, head labeled .Lt_0_27906
    l32i    a6,a1,76                    # [0]  gra_spill_temp_18
    l32i    a13,a1,96                   # [1]  gra_spill_temp_23
    l32i    a8,a1,84                    # [2]  gra_spill_temp_20
    l32i    a7,a1,128                   # [3]  filter_wd
    l32i    a10,a1,88                   # [4]  gra_spill_temp_21
    movi.n  a9,0                    # [5]
    s32i    a9,a1,32                    # [6]  gra_spill_temp_7
    add.n   a14,a10,a5                  # [7]
    min     a7,a7,a8                    # [8]
    add.n   a10,a10,a13                 # [9]
    add.n   a14,a13,a14                 # [10]
    s32i    a14,a1,12                   # [11]  gra_spill_temp_2
    s32i    a10,a1,16                   # [12]  gra_spill_temp_3
    movi.n  a8,0                    # [13]
    l32i    a10,a1,80                   # [14]  gra_spill_temp_19
    max     a6,a6,a8                    # [15]
    sub     a9,a7,a6                    # [16]
    s32i    a9,a1,28                    # [17]  gra_spill_temp_6
    add.n   a13,a10,a6                  # [18]
    s32i    a13,a1,24                   # [19]  gra_spill_temp_5
    add.n   a10,a10,a7                  # [16]
    s32i    a10,a1,72                   # [21]  gra_spill_temp_17

.Lt_0_28674:    # 0x446
#<loop> Loop body line 8, nesting depth: 3, estimated iterations: 56
    l32i    a8,a1,8                 # [0]  gra_spill_temp_1
    mv.qr   q1,q3                       # [1]
    mov.n   a13,a8                      # [2]
    bge     a8,a15,.Lt_0_28930          # [3]

.LBB35_esp_nn_max_pool_s8_esp32s3:  # 0x451
#<loop> Part of loop body line 8, head labeled .Lt_0_28674
    l32i    a10,a1,92                   # [0]  gra_spill_temp_22
    l32i    a14,a1,72                   # [1]  gra_spill_temp_17
    add.n   a10,a10,a8                  # [2]
    mull    a10,a11,a10                 # [3]
    add.n   a14,a10,a14                 # [5]

.Lt_0_29442:    # 0x45e
    add.n   a14,a14,a11                 # [0]
    addi.n  a13,a13,1               # [1]
    bge     a6,a7,.Lt_0_29698           # [2]

.LBB38_esp_nn_max_pool_s8_esp32s3:  # 0x465
    l32i    a3,a1,24                    # [0]  gra_spill_temp_5
    l32i    a2,a1,32                    # [1]  gra_spill_temp_7
    add.n   a3,a3,a10                   # [2]
    mull    a3,a3,a5                    # [3]
    l32i    a4,a1,28                    # [4]  gra_spill_temp_6
    add.n   a2,a2,a3                    # [5]
    add.n   a2,a12,a2                   # [6]
    loopgtz a4,.LBB108_esp_nn_max_pool_s8_esp32s3   # [7]

    ee.vldbc.32 q0,a2               # [0*II+0]  id:489
    add.n       a2,a2,a5                    # [0*II+1]
    ee.vmax.s8  q1,q1,q0            # [0*II+2]
.LBB108_esp_nn_max_pool_s8_esp32s3: # 0x482

.Lt_0_29698:    # 0x482
    add.n   a10,a10,a11                 # [0]
    bne     a15,a13,.Lt_0_29442         # [1]

.Lt_0_28930:    # 0x487
#<loop> Part of loop body line 8, head labeled .Lt_0_28674
    l32i            a9,a1,12                    # [0]  gra_spill_temp_2
    l32i            a8,a1,16                    # [1]  gra_spill_temp_3
    l32i            a10,a1,32                   # [3]  gra_spill_temp_7

    ee.vmin.s8      q5,q1,q4            # [4]
    ee.vmax.s8      q5,q5,q5            # [5]
    addi.n          a10,a10,4               # [6]
    ee.movi.32.a    q5,a13,0
    s32i            a10,a1,32                   # [9]  gra_spill_temp_7
    s32i.n          a13,a8,0                # [10]  id:492
    addi.n          a8,a8,4                 # [11]
    s32i            a8,a1,16                    # [12]  gra_spill_temp_3
    blt             a8,a9,.Lt_0_28674           # [13]

.Lt_0_28162:    # 0x4ad
#<loop> Part of loop body line 109, head labeled .Lt_0_27906
    l32i    a13,a1,84                   # [0]  gra_spill_temp_20
    l32i    a14,a1,80                   # [1]  gra_spill_temp_19
    l32i    a10,a1,120                  # [2]  stride_wd
    l32i    a8,a1,88                    # [3]  gra_spill_temp_21
    l32i    a9,a1,76                    # [4]  gra_spill_temp_18
    add.n   a8,a8,a5                    # [5]
    s32i    a8,a1,88                    # [6]  gra_spill_temp_21
    sub     a9,a9,a10                   # [7]
    add.n   a14,a14,a10                 # [8]
    sub     a13,a13,a10                 # [9]
    s32i    a13,a1,84                   # [10]  gra_spill_temp_20
    s32i    a14,a1,80                   # [11]  gra_spill_temp_19
    s32i    a9,a1,76                    # [12]  gra_spill_temp_18
    l32i    a14,a1,64                   # [13]  gra_spill_temp_15
    l32i    a8,a1,4                 # [14]  gra_spill_temp_0
    addi.n  a14,a14,1               # [15]
    s32i    a14,a1,64                   # [16]  gra_spill_temp_15
    sub     a14,a14,a8                  # [17]
    bnez    a14,.Lt_0_27906             # [18]

.Lt_0_27394:    # 0x4e3
#<loop> Part of loop body line 107, head labeled .Lt_0_27138
    l32i    a8,a1,44                    # [0]  gra_spill_temp_10
    l32i    a15,a1,92                   # [1]  gra_spill_temp_22
    l32i    a10,a1,60                   # [2]  gra_spill_temp_14
    l32i    a14,a1,124                  # [3]  stride_ht
    l32i    a13,a1,48                   # [4]  gra_spill_temp_11
    l32i    a9,a1,52                    # [5]  gra_spill_temp_12
    sub     a13,a13,a14                 # [6]
    add.n   a9,a9,a10                   # [7]
    add.n   a15,a15,a14                 # [8]
    sub     a8,a8,a14                   # [9]
    s32i    a8,a1,44                    # [10]  gra_spill_temp_10
    s32i    a15,a1,92                   # [11]  gra_spill_temp_22
    s32i    a9,a1,52                    # [12]  gra_spill_temp_12
    s32i    a13,a1,48                   # [13]  gra_spill_temp_11
    l32i.n  a9,a1,36                # [14]  gra_spill_temp_8
    l32i    a10,a1,68                   # [15]  gra_spill_temp_16
    addi.n  a9,a9,1                 # [16]
    s32i.n  a9,a1,36                # [17]  gra_spill_temp_8
    sub     a9,a9,a10                   # [18]
    bnez    a9,.Lt_0_27138              # [19]

    retw.n                          # [0] // exit

    .size   esp_nn_max_pool_s8_esp32s3, . - esp_nn_max_pool_s8_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
