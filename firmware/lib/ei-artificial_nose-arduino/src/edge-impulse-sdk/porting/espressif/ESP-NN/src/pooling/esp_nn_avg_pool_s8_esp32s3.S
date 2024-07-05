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

    # Program Unit: esp_nn_avg_pool_s8_esp32s3
    .type   esp_nn_avg_pool_s8_esp32s3, @function
    .align   4
    .global esp_nn_avg_pool_s8_esp32s3

// no of channels must be multiple of 4.

// a2: input
// a3: input_wd
// a4: input_ht
// a5: output
// a6: output_wd
// a7: output_ht
// on stack: stride_wd
// on stack: stride_ht
// on stack: filter_wd
// on stack: filter_ht
// on stack: pad_wd
// on stack: pad_ht
// on stack: activation_min
// on stack: activation_max
// on stack: channels

esp_nn_avg_pool_s8_esp32s3: # 0x4
    # activation_min = 0
    # activation_max = 4
    # gra_spill_temp_0 = 8
    # gra_spill_temp_1 = 12
    # gra_spill_temp_2 = 16
    # gra_spill_temp_3 = 20
    # gra_spill_temp_4 = 24
    # gra_spill_temp_5 = 28
    # gra_spill_temp_6 = 32
    # gra_spill_temp_7 = 36
    # gra_spill_temp_8 = 40
    # gra_spill_temp_9 = 44
    # gra_spill_temp_10 = 48
    # gra_spill_temp_11 = 52
    # gra_spill_temp_12 = 56
    # gra_spill_temp_13 = 60
    # gra_spill_temp_14 = 64
    # gra_spill_temp_15 = 68
    # gra_spill_temp_16 = 72
    # gra_spill_temp_17 = 76
    # gra_spill_temp_18 = 80
    # gra_spill_temp_19 = 84
    # gra_spill_temp_20 = 88
    # gra_spill_temp_21 = 92
    # gra_spill_temp_22 = 96
    # gra_spill_temp_23 = 100
    # gra_spill_temp_24 = 104
    # gra_spill_temp_25 = 108
    # gra_spill_temp_26 = 112
    # gra_spill_temp_27 = 116
    # gra_spill_temp_28 = 120
    # gra_spill_temp_29 = 124
    # gra_spill_temp_30 = 128
    # gra_spill_temp_31 = 132
    # gra_spill_temp_32 = 136
    # gra_spill_temp_33 = 140
    # gra_spill_temp_34 = 144
    # gra_spill_temp_35 = 148
    # gra_spill_temp_36 = 152
    # gra_spill_temp_37 = 156
    # gra_spill_temp_38 = 160
    # gra_spill_temp_39 = 164
    # gra_spill_temp_40 = 168
    # gra_spill_temp_41 = 172
    # gra_spill_temp_43 = 180

    entry   a1,240                      #
    mov.n   a11,a3                      # [0]
    mov.n   a12,a2                      # [1]
    s32i    a5,a1,136                   # [4]  gra_spill_temp_30
    s32i    a6,a1,128                   # [3]  gra_spill_temp_32

    l16ui   a5,a1,272                   # [5]  id:663 channels+0x0
    s32i    a7,a1,72                    # [6]  gra_spill_temp_16

    l32i        a9,a1,264                   # [1]  id:664 activation_min+0x0
    l32i        a10,a1,268                  # [2]  id:666 activation_max+0x0
    s32i.n      a9,a1,0                 # [4]  activation_min
    s32i.n      a10,a1,4                # [3]  activation_max
    addi.n      a8,a1,4                 # [0]  activation_max
    ee.vldbc.32 q7,a1               # [5]  id:668 activation_min
    ee.vldbc.32 q6,a8               # [6]  id:669 activation_max
    ee.zero.q   q4                      # [0]

    extui   a10,a5,0,3                  # [7]
    beqz.n  a10,.LBB3_esp_nn_avg_pool_s8_esp32s3    # [8], if (channels % 8 == 0)

    extui   a13,a5,0,2                  # [0]
    beqz.n  a13,.LBB52_esp_nn_avg_pool_s8_esp32s3   # [1], if (channels % 4 == 0)

// exit
.Lt_0_44546:    # 0x1e9
    retw.n                          # [0]

.LBB3_esp_nn_avg_pool_s8_esp32s3:   # 0x1eb // if (channels % 8 == 0)

    l16ui   a7,a1,256                   # [1]  id:671 pad_wd+0x0
    l16ui   a10,a1,260                  # [5]  id:670 pad_ht+0x0
    l32i    a15,a1,72                   # [12]  gra_spill_temp_16
    movi.n  a14,0                   # [13]
    movi.n  a8,0                    # [14]
    neg     a10,a10                     # [15]
    s32i    a10,a1,56                   # [16]  gra_spill_temp_12
    s32i    a8,a1,44                    # [17]  gra_spill_temp_9
    s32i.n  a14,a1,20               # [18]  gra_spill_temp_3
    sub     a9,a4,a10                   # [19]
    s32i    a9,a1,40                    # [20]  gra_spill_temp_8
    mul16u  a15,a15,a5              # [21]
    neg     a13,a7                      # [22]
    s32i    a13,a1,104                  # [23]  gra_spill_temp_24
    s32i.n  a15,a1,16               # [24]  gra_spill_temp_2
    sub     a13,a3,a13                  # [25]
    s32i.n  a13,a1,12               # [26]  gra_spill_temp_1
    j       .Lt_0_28162                     # [27]

.Lt_0_28418:    # 0x24e
#<loop> Part of loop body line 44, head labeled .Lt_0_28162
    l32i    a15,a1,260                  # [0]  pad_ht
    l32i    a14,a1,56                   # [1]  gra_spill_temp_12
    l32i.n  a9,a1,16                # [2]  gra_spill_temp_2
    l32i    a13,a1,244                  # [3]  stride_ht
    l32i    a10,a1,40                   # [4]  gra_spill_temp_8
    l32i    a8,a1,44                    # [5]  gra_spill_temp_9
    sub     a10,a10,a13                 # [6]
    add.n   a8,a8,a9                    # [7]
    add.n   a14,a14,a13                 # [8]
    sub     a15,a15,a13                 # [9]
    s32i    a15,a1,260                  # [10]  pad_ht
    s32i    a14,a1,56                   # [11]  gra_spill_temp_12
    s32i    a8,a1,44                    # [12]  gra_spill_temp_9
    s32i    a10,a1,40                   # [13]  gra_spill_temp_8
    l32i.n  a8,a1,20                # [14]  gra_spill_temp_3
    l32i    a9,a1,72                    # [15]  gra_spill_temp_16
    addi.n  a8,a8,1                 # [16]
    s32i.n  a8,a1,20                # [17]  gra_spill_temp_3
    beq a8,a9,.Lt_0_44546           # [18]

.Lt_0_28162:    # 0x281
    l32i    a10,a1,128                  # [0]  gra_spill_temp_32
    beqz.n  a10,.Lt_0_28418         # [2]

.LBB7_esp_nn_avg_pool_s8_esp32s3:   # 0x286
#<loop> Part of loop body line 44, head labeled .Lt_0_28162
    s32i    a7,a1,112                   # [0]  gra_spill_temp_26
    movi.n  a10,0                   # [1]
    l32i    a9,a1,260                   # [2]  pad_ht
    l32i.n  a6,a1,12                # [3]  gra_spill_temp_1
    l32i    a8,a1,44                    # [4]  gra_spill_temp_9
    movi.n  a13,0                   # [5]
    l32i    a15,a1,104                  # [6]  gra_spill_temp_24
    s32i    a15,a1,116                  # [7]  gra_spill_temp_27
    s32i    a13,a1,48                   # [8]  gra_spill_temp_10
    s32i    a8,a1,124                   # [9]  gra_spill_temp_29
    s32i    a6,a1,120                   # [10]  gra_spill_temp_28
    l32i    a8,a1,40                    # [11]  gra_spill_temp_8
    l32i    a6,a1,252                   # [12]  filter_ht
    movi.n  a13,0                   # [13]
    max     a9,a9,a10                   # [14]
    s32i    a9,a1,160                   # [15]  gra_spill_temp_38
    s32i    a13,a1,92                   # [16]  gra_spill_temp_21
    min     a6,a6,a8                    # [17]
    bnez.n  a5,.LBB10_esp_nn_avg_pool_s8_esp32s3    # [18]

.Lt_0_29186:    # 0x2ba
    l32i    a8,a1,116                   # [0]  gra_spill_temp_27
    l32i    a15,a1,120                  # [1]  gra_spill_temp_28
    l32i    a9,a1,48                    # [2]  gra_spill_temp_10
    l32i    a14,a1,240                  # [3]  stride_wd
    l32i    a10,a1,124                  # [4]  gra_spill_temp_29
    l32i    a13,a1,112                  # [5]  gra_spill_temp_26
    add.n   a10,a10,a5                  # [6]
    s32i    a10,a1,124                  # [7]  gra_spill_temp_29
    sub     a13,a13,a14                 # [8]
    add.n   a9,a9,a14                   # [9]
    sub     a15,a15,a14                 # [10]
    add.n   a8,a8,a14                   # [11]
    s32i    a8,a1,116                   # [12]  gra_spill_temp_27
    s32i    a15,a1,120                  # [13]  gra_spill_temp_28
    s32i    a9,a1,48                    # [14]  gra_spill_temp_10
    s32i    a13,a1,112                  # [15]  gra_spill_temp_26
    l32i    a9,a1,92                    # [16]  gra_spill_temp_21
    l32i    a10,a1,128                  # [17]  gra_spill_temp_32
    addi.n  a9,a9,1                 # [18]
    s32i    a9,a1,92                    # [19]  gra_spill_temp_21
    beq     a9,a10,.Lt_0_28418          # [20]

.Lt_0_28930:    # 0x2f5
#<loop> Part of loop body line 46, head labeled .Lt_0_29186
    beqz.n  a5,.Lt_0_29186          # [0]

.LBB10_esp_nn_avg_pool_s8_esp32s3:  # 0x2f7
#<loop> Part of loop body line 44, head labeled .Lt_0_28162
    l32i    a14,a1,120                  # [0]  gra_spill_temp_28
    l32i    a13,a1,248                  # [1]  filter_wd
    l32i    a9,a1,136                   # [2]  gra_spill_temp_30
    l32i    a8,a1,124                   # [3]  gra_spill_temp_29
    movi.n  a15,0                   # [4]
    s32i    a15,a1,24                   # [5]  gra_spill_temp_60
    add.n   a10,a8,a5                   # [6]
    movi.n  a15,0                   # [7]
    add.n   a8,a8,a9                    # [8]
    min     a13,a13,a14                 # [9]
    add.n   a10,a9,a10                  # [10]
    s32i    a10,a1,180                  # [11]  gra_spill_temp_43
    s32i    a13,a1,76                   # [12]  gra_spill_temp_17
    l32i    a14,a1,112                  # [13]  gra_spill_temp_26
    s32i    a8,a1,148                   # [14]  gra_spill_temp_45
    max     a14,a14,a15                 # [15]
    l32i    a15,a1,116                  # [16]  gra_spill_temp_27
    s32i    a14,a1,152                  # [17]  gra_spill_temp_63
    add.n   a8,a15,a14                  # [18]
    s32i    a8,a1,36                    # [19]  gra_spill_temp_7
    add.n   a15,a15,a13                 # [20]
    s32i    a15,a1,204                  # [21]  gra_spill_temp_39
    sub     a13,a13,a14                 # [22]
    s32i    a13,a1,280                  # [23]  gra_spill_temp_58
    j   .Lt_0_29698                     # [24]

.LBB13_esp_nn_avg_pool_s8_esp32s3:  # 0x33b
#<loop> Part of loop body line 16, head labeled .Lt_0_29698
    l32i    a10,a1,56                   # [0]  gra_spill_temp_12
    l32i    a14,a1,204                  # [1]  gra_spill_temp_39
    add.n   a10,a10,a15                 # [2]
    mull    a10,a11,a10                 # [3]
    movi.n  a15,0                   # [4]
    add.n   a14,a10,a14                 # [5]

.Lt_0_30466:    # 0x34a
#<loop> Loop body line 61, nesting depth: 4, estimated iterations: 252
    l32i    a9,a1,76                    # [0]  gra_spill_temp_17
    l32i    a8,a1,152                   # [1]  gra_spill_temp_63
    add.n   a14,a14,a11                 # [2]
    bge     a8,a9,.Lt_0_30722           # [3]

.LBB16_esp_nn_avg_pool_s8_esp32s3:  # 0x355
#<loop> Part of loop body line 61, head labeled .Lt_0_30466
    l32i    a3,a1,36                    # [0]  gra_spill_temp_7
    l32i    a2,a1,24                    # [1]  gra_spill_temp_4
    add.n   a3,a3,a10                   # [2]
    mull    a3,a3,a5                    # [3]
    movi.n  a8,0                    # [4]
    add.n   a2,a2,a3                    # [5]
    l32i    a3,a1,280                   # [6]  gra_spill_temp_58
    add.n   a2,a12,a2                   # [7]
    loopgtz a3,.LBB140_esp_nn_avg_pool_s8_esp32s3   # [8]

    ee.vld.l.64.xp  q0,a2,a5            # [0*II+1]  id:677
    ee.vcmp.lt.s8   q1,q0,q4            # [0*II+3]
    ee.vzip.8       q0,q1                   # [0*II+4]
    ee.vcmp.lt.s16  q1,q0,q4        # [0*II+5]
    ee.vzip.16      q0,q1               # [0*II+6]
    ee.vadds.s32    q2,q2,q1            # [0*II+7]
    ee.vadds.s32    q3,q3,q0            # [0*II+8]


.LBB140_esp_nn_avg_pool_s8_esp32s3: # 0x385
#<loop> Part of loop body line 61, head labeled .Lt_0_30466
    l32i    a2,a1,48                    # [0]  gra_spill_temp_10
    sub     a9,a7,a2                    # [2]
    sub     a2,a2,a7                    # [3]
    max     a9,a9,a8                    # [4]
    l32i    a8,a1,248                   # [5]  filter_wd
    sub     a2,a11,a2                   # [6]
    min     a8,a8,a2                    # [7]
    sub     a8,a8,a9                    # [8]
    add.n   a15,a15,a8                  # [9]

.Lt_0_30722:    # 0x39f
#<loop> Part of loop body line 61, head labeled .Lt_0_30466
    add.n   a10,a10,a11                 # [0]
    addi.n  a13,a13,1               # [1]
    bne     a6,a13,.Lt_0_30466          # [2]

.Lt_0_29954:    # 0x3a6
    srai            a2,a15,1                    # [3]

// move data to general purpose registers and average
    ee.movi.32.a    q3,a9,0             # [0]
    ee.movi.32.a    q3,a4,1             # [0]

    blti            a9,1,.Lt_0_32258            # [4]
    add.n           a9,a9,a2                    # [0]
    j               .Lt_0_32002                     # [2]
.Lt_0_32258:    # 0x45e
    sub             a9,a9,a2                    # [0]
.Lt_0_32002:    # 0x3b9

    blti            a4,1,.Lt_0_32770            # [1]
    add.n           a4,a2,a4                    # [0]
    j               .Lt_0_32514                     # [2]
.Lt_0_32770:
    sub             a4,a4,a2                    # [0]
.Lt_0_32514:    # 0x3c4

    quos            a9,a9,a15                   # [1]
    quos            a4,a4,a15                   # [1]
    ee.movi.32.q    q3,a9,0             # [0]
    ee.movi.32.q    q3,a4,1             # [1]

    ee.movi.32.a    q3,a9,2             # [2]
    ee.movi.32.a    q3,a14,3            # [0]

    blti            a9,1,.Lt_0_33282            # [3]
    add.n           a9,a9,a2                    # [0]
    j               .Lt_0_33026                     # [2]
.Lt_0_33282:    # 0x470
    sub             a9,a9,a2                    # [0]
.Lt_0_33026:    # 0x3d5

    blti            a14,1,.Lt_0_33794           # [1]
    add.n           a14,a2,a14                  # [0]
    j               .Lt_0_33538                     # [2]
.Lt_0_33794:    # 0x479
    sub             a14,a14,a2                      # [0]
.Lt_0_33538:    # 0x3e0

    quos            a9,a9,a15                   # [1]
    quos            a14,a14,a15                 # [1]
    ee.movi.32.q    q3,a9,2             # [0]
    ee.movi.32.q    q3,a14,3            # [1]


    ee.movi.32.a    q2,a9,0             # [0]
    ee.movi.32.a    q2,a4,1             # [0]

    blti            a9,1,.Lt_0_34306            # [3]
    add.n           a9,a9,a2                    # [0]
    j               .Lt_0_34050                     # [2]
.Lt_0_34306:    # 0x482
    sub             a9,a9,a2                    # [0]
.Lt_0_34050:    # 0x3f1

    blti            a4,1,.Lt_0_34818            # [1]
    add.n           a4,a2,a4                    # [0]
    j               .Lt_0_34562                     # [2]
.Lt_0_34818:    # 0x48b
    sub             a4,a4,a2                    # [0]
.Lt_0_34562:    # 0x3fc

    quos            a9,a9,a15                   # [1]
    quos            a4,a4,a15                   # [1]
    ee.movi.32.q    q2,a9,0             # [0]
    ee.movi.32.q    q2,a4,1             # [1]

    ee.movi.32.a    q2,a9,2             # [2]
    ee.movi.32.a    q2,a14,3            # [0]

    blti            a9,1,.Lt_0_35330            # [3]
    add.n           a9,a9,a2                    # [0]
    j               .Lt_0_35074                     # [2]
.Lt_0_35330:    # 0x494
    sub             a9,a9,a2                    # [0]
.Lt_0_35074:    # 0x40d

    blti            a14,1,.Lt_0_35842           # [1]
    add.n           a14,a2,a14                  # [0]
    j               .Lt_0_35586                     # [2]
.Lt_0_35842:    # 0x49d
    sub             a14,a14,a2                      # [0]
.Lt_0_35586:    # 0x418

    quos            a9,a9,a15                   # [1]
    quos            a14,a14,a15                 # [1]
    ee.movi.32.q    q2,a9,2             # [0]
    ee.movi.32.q    q2,a14,3            # [1]


    l32i            a9,a1,180                   # [0]  gra_spill_temp_43
    l32i            a14,a1,24                   # [1]  gra_spill_temp_4
    l32i            a13,a1,148                  # [2]  gra_spill_temp_45
    ee.vmin.s32     q1,q3,q6            # [4]
    ee.vmax.s32     q1,q1,q7            # [5]
    ee.vmin.s32     q5,q2,q6            # [8]
    addi.n          a14,a14,8               # [9]
    s32i            a14,a1,24                   # [10]  gra_spill_temp_4
    ee.vmax.s32     q5,q5,q7            # [11]
    addi.n          a8,a13,8                    # [12]
    s32i            a8,a1,148                   # [13]  gra_spill_temp_45
    ee.vunzip.16    q1,q5               # [14]
    ee.vunzip.8     q1,q5               # [15]
    ee.vst.l.64.ip  q1,a13,0        # [16]  id:678
    bge             a8,a9,.Lt_0_29186           # [17]

.Lt_0_29698:    # 0x44b
#<loop> Loop body line 16, nesting depth: 3, estimated iterations: 252
    mv.qr   q3,q4                       # [0]
    l32i    a15,a1,160                  # [1]  gra_spill_temp_38
    mv.qr   q2,q4                       # [2]
    mov.n   a13,a15                     # [3]
    blt a15,a6,.LBB13_esp_nn_avg_pool_s8_esp32s3    # [4]

.Lt_0_51458:    # 0x459
#<loop> Part of loop body line 16, head labeled .Lt_0_29698
    movi.n  a15,0                   # [0]
    j   .Lt_0_29954                     # [1]


.LBB52_esp_nn_avg_pool_s8_esp32s3:  # 0x4a6 // if (channels % 4 == 0)

    l16ui   a7,a1,256                   # [1]  id:671 pad_wd+0x0
    l16ui   a13,a1,260                  # [5]  id:670 pad_ht+0x0
    s32i    a13,a1,64                   # [8]  gra_spill_temp_4
    l32i    a8,a1,72                    # [12]  gra_spill_temp_16
    movi.n  a15,0                   # [13]
    movi.n  a9,0                    # [14]
    neg     a13,a13                     # [15]
    s32i    a13,a1,192                  # [16]  gra_spill_temp_36
    s32i    a9,a1,32                    # [17]  gra_spill_temp_6
    s32i.n  a15,a1,8                # [18]  gra_spill_temp_0
    sub     a10,a4,a13                  # [19]
    s32i    a10,a1,28                   # [20]  gra_spill_temp_5
    mul16u  a8,a8,a5                # [21]
    neg     a14,a7                      # [22]
    s32i    a14,a1,104                  # [23]  gra_spill_temp_24
    s32i.n  a8,a1,16                # [24]  gra_spill_temp_2
    sub     a14,a3,a14                  # [25]
    s32i.n  a14,a1,12               # [26]  gra_spill_temp_1
    j   .Lt_0_37890                     # [27]

.Lt_0_38146:    # 0x50b
#<loop> Part of loop body line 161, head labeled .Lt_0_37890
    l32i    a15,a1,64                   # [0]  gra_spill_temp_4
    l32i    a14,a1,192                  # [1]  gra_spill_temp_36
    l32i.n  a9,a1,16                # [2]  gra_spill_temp_2
    l32i    a13,a1,244                  # [3]  stride_ht
    l32i    a10,a1,28                   # [4]  gra_spill_temp_5
    l32i    a8,a1,32                    # [5]  gra_spill_temp_6
    sub     a10,a10,a13                 # [6]
    add.n   a8,a8,a9                    # [7]
    add.n   a14,a14,a13                 # [8]
    sub     a15,a15,a13                 # [9]
    s32i    a15,a1,64                   # [10]  gra_spill_temp_4
    s32i    a14,a1,192                  # [11]  gra_spill_temp_36
    s32i    a8,a1,32                    # [12]  gra_spill_temp_6
    s32i    a10,a1,28                   # [13]  gra_spill_temp_5
    l32i.n  a8,a1,8                 # [14]  gra_spill_temp_0
    l32i    a9,a1,72                    # [15]  gra_spill_temp_16
    addi.n  a8,a8,1                 # [16]
    s32i.n  a8,a1,8                 # [17]  gra_spill_temp_0
    sub     a8,a8,a9                    # [18]
    beqz    a8,.Lt_0_44546              # [19]

.Lt_0_37890:    # 0x541
#<loop> Loop body line 161, nesting depth: 1, estimated iterations: 252
    l32i    a10,a1,128                  # [0]  gra_spill_temp_32
    beqz.n  a10,.Lt_0_38146         # [2]

#<loop> Part of loop body line 161, head labeled .Lt_0_37890
    s32i    a7,a1,96                    # [0]  gra_spill_temp_22
    movi.n  a10,0                   # [1]
    l32i    a9,a1,64                    # [2]  gra_spill_temp_4
    l32i.n  a6,a1,12                # [3]  gra_spill_temp_1
    l32i    a8,a1,32                    # [4]  gra_spill_temp_6
    movi.n  a13,0                   # [5]
    l32i    a15,a1,104                  # [6]  gra_spill_temp_24
    s32i    a15,a1,100                  # [7]  gra_spill_temp_23
    s32i    a13,a1,148                  # [8]  gra_spill_temp_35
    s32i    a8,a1,108                   # [9]  gra_spill_temp_25
    s32i    a6,a1,144                   # [10]  gra_spill_temp_24
    l32i    a8,a1,28                    # [11]  gra_spill_temp_5
    l32i    a6,a1,252                   # [12]  filter_ht
    max     a9,a9,a10                   # [14]
    s32i    a9,a1,168                   # [15]  gra_spill_temp_40
    s32i    a13,a1,88                   # [16]  gra_spill_temp_20
    min     a6,a6,a8                    # [17]
    bnez.n  a5,.LBB59_esp_nn_avg_pool_s8_esp32s3    # [18]

.Lt_0_38914:    # 0x57a
#<loop> Loop body line 163
    l32i    a8,a1,100                   # [0]  gra_spill_temp_23
    l32i    a15,a1,144                  # [1]  gra_spill_temp_24
    l32i    a9,a1,148                   # [2]  gra_spill_temp_35
    l32i    a14,a1,240                  # [3]  stride_wd
    l32i    a10,a1,108                  # [4]  gra_spill_temp_25
    l32i    a13,a1,96                   # [5]  gra_spill_temp_22
    add.n   a10,a10,a5                  # [6]
    s32i    a10,a1,108                  # [7]  gra_spill_temp_25
    sub     a13,a13,a14                 # [8]
    add.n   a9,a9,a14                   # [9]
    sub     a15,a15,a14                 # [10]
    add.n   a8,a8,a14                   # [11]
    s32i    a8,a1,100                   # [12]  gra_spill_temp_23
    s32i    a15,a1,144                  # [13]  gra_spill_temp_24
    s32i    a9,a1,148                   # [14]  gra_spill_temp_35
    s32i    a13,a1,96                   # [15]  gra_spill_temp_22
    l32i    a9,a1,88                    # [16]  gra_spill_temp_20
    l32i    a10,a1,128                  # [17]  gra_spill_temp_32
    addi.n  a9,a9,1                 # [18]
    s32i    a9,a1,88                    # [19]  gra_spill_temp_20
    beq     a9,a10,.Lt_0_38146          # [20]

    beqz.n  a5,.Lt_0_38914          # [0]

.LBB59_esp_nn_avg_pool_s8_esp32s3:  # 0x5b7
#<loop> Part of loop body line 161, head labeled .Lt_0_37890
    l32i    a14,a1,144                  # [0]  gra_spill_temp_24
    l32i    a13,a1,248                  # [1]  filter_wd
    l32i    a9,a1,136                   # [2]  gra_spill_temp_30
    l32i    a8,a1,108                   # [3]  gra_spill_temp_25
    movi.n  a15,0                   # [4]
    s32i    a15,a1,216                  # [5]  gra_spill_temp_52
    add.n   a10,a8,a5                   # [6]
    add.n   a8,a8,a9                    # [8]
    min     a13,a13,a14                 # [9]
    add.n   a10,a9,a10                  # [10]
    s32i    a10,a1,172                  # [11]  gra_spill_temp_41
    s32i    a13,a1,132                  # [12]  gra_spill_temp_31
    l32i    a14,a1,96                   # [13]  gra_spill_temp_22
    s32i    a8,a1,164                   # [14]  gra_spill_temp_39
    max     a14,a14,a15                 # [15]
    l32i    a15,a1,100                  # [16]  gra_spill_temp_23
    s32i    a14,a1,208                  # [17]  gra_spill_temp_50
    add.n   a8,a15,a14                  # [18]
    s32i    a8,a1,60                    # [19]  gra_spill_temp_13
    add.n   a15,a15,a13                 # [20]
    s32i    a15,a1,196                  # [21]  gra_spill_temp_37
    sub     a13,a13,a14                 # [22]
    s32i    a13,a1,52                   # [23]  gra_spill_temp_11
    j       .Lt_0_39426                     # [24]

.LBB62_esp_nn_avg_pool_s8_esp32s3:  # 0x5fb
#<loop> Part of loop body line 173, head labeled .Lt_0_39426
    l32i    a10,a1,192                  # [0]  gra_spill_temp_36
    l32i    a14,a1,196                  # [1]  gra_spill_temp_37
    add.n   a10,a10,a15                 # [2]
    mull    a10,a11,a10                 # [3]
    movi.n  a15,0                   # [4]
    add.n   a14,a10,a14                 # [5]

.Lt_0_40194:    # 0x60a
#<loop> Loop body line 178, nesting depth: 4, estimated iterations: 252
    l32i    a9,a1,132                   # [0]  gra_spill_temp_31
    l32i    a8,a1,208                   # [1]  gra_spill_temp_50
    add.n   a14,a14,a11                 # [2]
    bge a8,a9,.Lt_0_40450           # [3]

.LBB65_esp_nn_avg_pool_s8_esp32s3:  # 0x615
#<loop> Part of loop body line 178, head labeled .Lt_0_40194
    l32i    a3,a1,60                    # [0]  gra_spill_temp_13
    l32i    a2,a1,216                   # [1]  gra_spill_temp_52
    add.n   a3,a3,a10                   # [2]
    mull    a3,a3,a5                    # [3]
    l32i    a4,a1,52                    # [4]  gra_spill_temp_11
    add.n   a2,a2,a3                    # [5]
    add.n   a2,a12,a2                   # [6]
    loopgtz a4,.LBB155_esp_nn_avg_pool_s8_esp32s3   # [7]

    ee.vldbc.32.xp  q0,a2,a5                # [0*II+0]  id:684
    ee.vcmp.lt.s8   q1,q0,q4            # [0*II+2]
    ee.vzip.8       q0,q1                   # [0*II+3]
    ee.vcmp.lt.s16  q1,q0,q4        # [0*II+4]
    ee.vzip.16      q0,q1               # [0*II+5]
    ee.vadds.s32    q2,q2,q0            # [0*II+6]

.LBB155_esp_nn_avg_pool_s8_esp32s3: # 0x63e
#<loop> Part of loop body line 178, head labeled .Lt_0_40194
    l32i    a2,a1,148                   # [0]  gra_spill_temp_35
    movi.n  a8,0                    # [1]
    sub     a9,a7,a2                    # [2]
    sub     a2,a2,a7                    # [3]
    max     a9,a9,a8                    # [4]
    l32i    a8,a1,248                   # [5]  filter_wd
    sub     a2,a11,a2                   # [6]
    min     a8,a8,a2                    # [7]
    sub     a8,a8,a9                    # [8]
    add.n   a15,a15,a8                  # [9]

.Lt_0_40450:    # 0x65a
#<loop> Part of loop body line 178, head labeled .Lt_0_40194
    add.n   a10,a10,a11                 # [0]
    addi.n  a13,a13,1               # [1]
    bne     a6,a13,.Lt_0_40194          # [2]

.Lt_0_39682:    # 0x661
#<loop> Part of loop body line 173, head labeled .Lt_0_39426
    srai            a2,a15,1                    # [5]

// move to gp registers and average

    ee.movi.32.a    q2,a9,0             # [0]
    ee.movi.32.a    q2,a4,1             # [0]

    blti            a9,1,.Lt_0_41986            # [3]
    add.n           a9,a9,a2                    # [0]
    j               .Lt_0_41730                     # [2]
.Lt_0_41986:    # 0x482
    sub             a9,a9,a2                    # [0]
.Lt_0_41730:    # 0x3f1

    blti            a4,1,.Lt_0_42498            # [1]
    add.n           a4,a2,a4                    # [0]
    j               .Lt_0_42242                     # [2]
.Lt_0_42498:    # 0x48b
    sub             a4,a4,a2                    # [0]
.Lt_0_42242:    # 0x3fc


    quos            a9,a9,a15                   # [1]
    quos            a4,a4,a15                   # [1]
    ee.movi.32.q    q2,a9,0             # [0]
    ee.movi.32.q    q2,a4,1             # [1]

    ee.movi.32.a    q2,a9,2             # [2]
    ee.movi.32.a    q2,a14,3            # [0]

    blti            a9,1,.Lt_0_43010            # [3]
    add.n           a9,a9,a2                    # [0]
    j               .Lt_0_42754                     # [2]
.Lt_0_43010:    # 0x494
    sub             a9,a9,a2                    # [0]
.Lt_0_42754:    # 0x40d


    blti            a14,1,.Lt_0_43522           # [1]
    add.n           a14,a2,a14                  # [0]
    j               .Lt_0_43266                     # [2]
.Lt_0_43522:    # 0x49d
    sub             a14,a14,a2                      # [0]
.Lt_0_43266:    # 0x418

    quos            a9,a9,a15                   # [1]
    quos            a14,a14,a15                 # [1]
    ee.movi.32.q    q2,a9,2             # [0]
    ee.movi.32.q    q2,a14,3            # [1]


    l32i            a9,a1,172                   # [0]  gra_spill_temp_41
    l32i            a8,a1,164                   # [1]  gra_spill_temp_39
    l32i            a14,a1,216                  # [2]  gra_spill_temp_52
    addi.n          a14,a14,4               # [5]
    ee.vmin.s32     q2,q2,q6            # [6]
    s32i            a14,a1,216                  # [7]  gra_spill_temp_52
    ee.vmax.s32     q2,q2,q7            # [8]
    ee.vunzip.16    q2,q1               # [9]
    ee.vunzip.8     q2,q1               # [10]
    ee.vst.l.64.ip  q2,a1,0         # [11]  id:691
    l32i.n          a13,a1,0                # [12]  id:692
    s32i.n          a13,a8,0                # [13]  id:693
    addi.n          a8,a8,4                 # [14]
    s32i            a8,a1,164                   # [15]  gra_spill_temp_39
    bge             a8,a9,.Lt_0_38914           # [16]

.Lt_0_39426:    # 0x6cb
    l32i    a15,a1,168                  # [0]  gra_spill_temp_40
    mv.qr   q2,q4                       # [1]
    mov.n   a13,a15                     # [2]
    blt     a15,a6,.LBB62_esp_nn_avg_pool_s8_esp32s3    # [3]

.Lt_0_52738:    # 0x6d6
    movi.n  a15,0                   # [0]
    j       .Lt_0_39682                     # [1]

    .size   esp_nn_avg_pool_s8_esp32s3, . - esp_nn_avg_pool_s8_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
