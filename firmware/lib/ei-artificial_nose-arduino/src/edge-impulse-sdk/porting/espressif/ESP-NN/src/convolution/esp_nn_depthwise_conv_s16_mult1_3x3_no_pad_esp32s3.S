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
    .literal_position

    # Program Unit: esp_nn_depthwise_conv_s16_mult1_3x3_no_pad_esp32s3
    .type   esp_nn_depthwise_conv_s16_mult1_3x3_no_pad_esp32s3, @function
    .align   4
    .global esp_nn_depthwise_conv_s16_mult1_3x3_no_pad_esp32s3

esp_nn_depthwise_conv_s16_mult1_3x3_no_pad_esp32s3: # 0xa42
    # qacc_scratch = 0
    # gra_spill_temp_71 = 48
    # gra_spill_temp_72 = 52
    # gra_spill_temp_73 = 56
    # gra_spill_temp_74 = 60
    # gra_spill_temp_75 = 64
    # gra_spill_temp_76 = 68
    # gra_spill_temp_77 = 72
    # gra_spill_temp_78 = 76
    # gra_spill_temp_79 = 80
    # gra_spill_temp_80 = 84
    # gra_spill_temp_81 = 88
    # gra_spill_temp_82 = 92
    # gra_spill_temp_83 = 96
    # gra_spill_temp_84 = 100
    # gra_spill_temp_85 = 104
    # gra_spill_temp_86 = 108
    # gra_spill_temp_87 = 112
    # gra_spill_temp_88 = 116
    # gra_spill_temp_89 = 120
    # gra_spill_temp_90 = 124
    # gra_spill_temp_91 = 128
    # gra_spill_temp_92 = 132
    # gra_spill_temp_93 = 136
    # gra_spill_temp_94 = 140
    # gra_spill_temp_95 = 144
    # gra_spill_temp_96 = 160
    # gra_spill_temp_97 = 176
    # gra_spill_temp_98 = 192
    # gra_spill_temp_99 = 208
    # gra_spill_temp_100 = 224
    # gra_spill_temp_101 = 240
    # gra_spill_temp_102 = 244
    # gra_spill_temp_103 = 248

 // registers:
 // a2: const int16_t *input_data
 // a3: const uint16_t input_wd
 // a4: const uint16_t input_ht
 // a5: const uint16_t channels
 // a6: const uint16_t stride_wd
 // a7: const uint16_t stride_ht

 // on stack:
 // const int16_t *filter_data
 // const int32_t *bias
 // int8_t *out_data
 // const uint16_t out_wd
 // const uint16_t out_ht
 // const int32_t out_offset
 // const int32_t *out_shift
 // const int32_t *out_mult
 // const int32_t activation_min
 // const int32_t activation_max

    entry   a1,288                      #
    s32i    a2,a1,120                   # [0]  gra_spill_temp_89
    s32i.n  a3,a1,48                # [1]  gra_spill_temp_71
    s32i    a5,a1,76                    # [2]  gra_spill_temp_78
    s32i    a6,a1,84                    # [3]  gra_spill_temp_80
    s32i.n  a7,a1,60                # [4]  gra_spill_temp_74
    l32i    a12,a1,296                  # [5]  id:241 out_data+0x0
    addi    a14,a1,112                  # [6]
    addmi   a10,a1,256                  # [7]
    addmi   a13,a1,256                  # [8]
    addmi   a15,a1,256                  # [9]

 // height loop
    l16ui   a8,a1,304                   # [10]  id:242 out_ht+0x0
    s32i.n  a8,a1,56                # [11]  gra_spill_temp_73
    addi    a15,a15,52                  # [12]
    addi    a13,a13,64                  # [13]
    addi    a10,a10,68                  # [14]
    ee.vldbc.32 q0,a10              # [15]  id:240 activation_max
    ee.vldbc.32 q1,a13              # [16]  id:239 activation_min
    ee.vldbc.32 q2,a15              # [17]  id:238 out_offset
    st.qr   q2,a14,64                   # [18]  gra_spill_temp_97-112
    st.qr   q1,a14,80                   # [19]  gra_spill_temp_98-112
    st.qr   q0,a14,96                   # [20]  gra_spill_temp_99-112
    beqz.n  a8,.Lt_6_6914           # [21]

.LBB3_esp_nn_depthwise_conv_s16_mult1_3x3_no_pad:   # 0xa83
    s32i    a1,a1,144                   # [0]  gra_spill_temp_95
    mul16u  a7,a3,a5                # [1]
    s32i    a4,a1,72                    # [2]  gra_spill_temp_77
    addi    a9,a5,-7                    # [3]
    l16ui   a11,a1,300                  # [4]  id:247 out_wd+0x0
    l32i    a10,a1,292                  # [5]  id:243 bias+0x0
    l32i    a15,a1,288                  # [6]  id:244 filter_data+0x0
    l32i    a13,a1,316                  # [7]  id:246 out_mult+0x0
    l32i    a14,a1,312                  # [8]  id:245 out_shift+0x0
    s32i    a14,a1,88                   # [9]  gra_spill_temp_81
    s32i    a13,a1,92                   # [10]  gra_spill_temp_82
    s32i    a15,a1,124                  # [11]  gra_spill_temp_90
    s32i    a10,a1,116                  # [12]  gra_spill_temp_88
    s32i    a11,a1,96                   # [13]  gra_spill_temp_83
    s32i    a9,a1,136                   # [14]  gra_spill_temp_93
    addx2   a4,a5,a5                    # [15]
    slli    a4,a4,1                     # [16]
    slli    a7,a7,1                     # [17]
    l32i.n  a9,a1,60                # [18]  gra_spill_temp_74
    movi.n  a11,0                   # [19]
    extui   a10,a10,0,4                 # [20]
    movi.n  a15,0                   # [21]
    slli    a5,a5,1                     # [22]
    s32i    a15,a1,68                   # [23]  gra_spill_temp_76
    s32i    a10,a1,112                  # [24]  gra_spill_temp_87
    s32i    a11,a1,64                   # [25]  gra_spill_temp_75
    mul16u  a8,a3,a9                # [26]
    movi.n  a11,0                   # [27]
    s32i    a11,a1,80                   # [28]  gra_spill_temp_79
    s32i.n  a8,a1,52                # [29]  gra_spill_temp_72

.Lt_6_7426: # 0xad8 // width_loop
    l32i    a8,a1,96                    # [0]  gra_spill_temp_83
    beqz.n  a8,.Lt_6_7682           # [2]

    movi.n  a11,3                   # [0]
    l32i    a10,a1,72                   # [1]  gra_spill_temp_77
    movi.n  a9,0                    # [2]
    movi.n  a13,0                   # [3]
    l32i.n  a14,a1,48               # [4]  gra_spill_temp_71
    s32i    a14,a1,108                  # [5]  gra_spill_temp_86
    s32i    a13,a1,104                  # [6]  gra_spill_temp_85
    s32i    a9,a1,100                   # [7]  gra_spill_temp_84
    min a10,a10,a11                 # [8]
    s32i    a10,a1,128                  # [9]  gra_spill_temp_91

.Lt_6_8194: # 0xaf7
    l32i    a2,a1,88                    # [0]  gra_spill_temp_81
    l32i    a6,a1,92                    # [1]  gra_spill_temp_82
    l32i    a8,a1,116                   # [2]  gra_spill_temp_88

// channel loop
    l32i    a15,a1,136                  # [3]  gra_spill_temp_93
    s32i    a8,a1,140                   # [4]  gra_spill_temp_94
    blti    a15,1,.Lt_6_8450            # [5]

    movi.n  a11,0                   # [0]
    movi.n  a10,0                   # [1]
    l32i    a9,a1,76                    # [2]  gra_spill_temp_78
    l32i    a14,a1,80                   # [3]  gra_spill_temp_79
    movi.n  a8,3                    # [4]
    l32i    a3,a1,108                   # [5]  gra_spill_temp_86
    l32i    a13,a1,104                  # [6]  gra_spill_temp_85
    min a3,a3,a8                    # [7]
    add.n   a13,a13,a14                 # [8]
    mull    a9,a9,a13                   # [9]
    s32i    a9,a1,132                   # [10]  gra_spill_temp_92

.Lt_6_8962: # 0xb26
    ee.zero.qacc                    # [0]
    l32i    a9,a1,132                   # [1]  gra_spill_temp_92
    l32i    a13,a1,120                  # [2]  gra_spill_temp_89
    add.n   a9,a9,a10                   # [3]
    addx2   a9,a9,a13                   # [4]
    l32i    a13,a1,124                  # [5]  gra_spill_temp_90
    l32i    a14,a1,128                  # [6]  gra_spill_temp_91
    add.n   a13,a11,a13                 # [7]
    loopgtz a14,.LBB30_esp_nn_depthwise_conv_s16_mult1_3x3_no_pad   # [8]

.Lt_6_9730: # 0xb3f
#<loop> Loop body line 360, nesting depth: 4, estimated iterations: 100
    mov.n   a14,a13                     # [0]
    mov.n   a15,a9                      # [1]
    ee.vld.128.xp   q0,a15,a5           # [2]  id:249
    ee.vld.128.xp   q1,a14,a5           # [3]  id:250
    add.n   a9,a9,a7                    # [4]
    beqi    a3,2,.LBB15_esp_nn_depthwise_conv_s16_mult1_3x3_no_pad  # [5]

.Lt_6_9986: # 0xb4e
    beqi    a3,3,.LBB17_esp_nn_depthwise_conv_s16_mult1_3x3_no_pad  # [0]

.Lt_6_10498:    # 0xb51
    add.n   a13,a13,a4                  # [0]
    ee.vmulas.s16.qacc  q0,q1       # [1]

.LBB30_esp_nn_depthwise_conv_s16_mult1_3x3_no_pad:  # 0xb58

 // extract data
    l32i    a15,a1,144                  # [0]  gra_spill_temp_95
    ee.st.qacc_l.l.128.ip   a15,16      # [2]  id:258
    ee.st.qacc_l.h.32.ip    a15,0       # [3]  id:259
    l8ui    a14,a1,15                   # [4]  qacc_scratch+15
    l8ui    a13,a1,16                   # [5]  qacc_scratch+16
    l8ui    a8,a1,5                     # [6]  qacc_scratch+5
    l8ui    a9,a1,6                     # [7]  qacc_scratch+6
    s8i     a9,a1,3                     # [8]  qacc_scratch+3
    s8i     a8,a1,2                     # [9]  qacc_scratch+2
    s8i     a13,a1,7                    # [10]  qacc_scratch+7
    s8i     a14,a1,6                    # [11]  qacc_scratch+6
    l16ui   a13,a1,10                   # [12]  qacc_scratch+10
    s16i    a13,a1,4                    # [13]  qacc_scratch+4
    ee.st.qacc_h.l.128.ip   a15,16      # [14]  id:269
    ee.st.qacc_h.h.32.ip    a15,-32     # [15]  id:270
    l8ui    a9,a1,32                    # [16]  qacc_scratch+32
    l8ui    a13,a1,22                   # [17]  qacc_scratch+22
    l8ui    a8,a1,31                    # [18]  qacc_scratch+31
    l16ui   a14,a1,26                   # [19]  qacc_scratch+26
    s16i    a14,a1,12                   # [20]  qacc_scratch+12
    s8i     a8,a1,14                    # [21]  qacc_scratch+14
    s8i     a13,a1,11                   # [22]  qacc_scratch+11
    s8i     a9,a1,15                    # [23]  qacc_scratch+15

    l32i    a13,a1,116                  # [24]  gra_spill_temp_88
    l8ui    a9,a1,21                    # [25]  qacc_scratch+21
    l16ui   a8,a1,16                    # [26]  qacc_scratch+16
    movi.n  a14,16                  # [27]
    ee.srcmb.s16.qacc   q1,a14,0        # [28]
    s16i    a8,a1,8                     # [29]  qacc_scratch+8
    s8i     a9,a1,10                    # [30]  qacc_scratch+10
    ee.vld.128.ip   q0,a15,0            # [31]  id:282
    s32i    a15,a1,144                  # [32]  gra_spill_temp_95
    ee.vzip.16  q0,q1               # [33]

    bnez.n  a13,.LBB20_esp_nn_depthwise_conv_s16_mult1_3x3_no_pad   # [34]

    s32i    a12,a1,240                  # [0]  gra_spill_temp_101
    s32i    a11,a1,244                  # [1]  gra_spill_temp_102
    s32i    a10,a1,248                  # [2]  gra_spill_temp_103
    addi    a14,a1,112                  # [3]
    st.qr   q1,a14,48                   # [4]  gra_spill_temp_96-112
    j   .Lt_6_11266                     # [5]

.LBB15_esp_nn_depthwise_conv_s16_mult1_3x3_no_pad:  # 0xbce
#<loop> Part of loop body line 360, head labeled .Lt_6_9730
    ee.vmulas.s16.qacc.ld.xp    q0,a15,a5,q0,q1     # [0]  id:251
    ee.vld.128.xp   q1,a14,a5           # [1]  id:252
    bnei    a3,3,.Lt_6_10498            # [2]

.LBB17_esp_nn_depthwise_conv_s16_mult1_3x3_no_pad:  # 0xbd8
    ee.vmulas.s16.qacc.ld.xp    q3,a15,a5,q0,q1     # [0]  id:253
    ee.vld.128.xp   q4,a14,a5           # [1]  id:254
    ee.vld.128.xp   q1,a14,a5           # [2]  id:256
    ee.vmulas.s16.qacc.ld.xp    q0,a15,a5,q3,q4     # [3]  id:255
    j   .Lt_6_10498                     # [4]

.LBB20_esp_nn_depthwise_conv_s16_mult1_3x3_no_pad:  # 0xbe9
#<loop> Part of loop body line 358, head labeled .Lt_6_8962
    s32i    a12,a1,240                  # [0]  gra_spill_temp_101
    s32i    a11,a1,244                  # [1]  gra_spill_temp_102
    s32i    a10,a1,248                  # [2]  gra_spill_temp_103
    addi    a15,a1,112                  # [3]
    l32i    a9,a1,112                   # [4]  gra_spill_temp_87
    l32i    a8,a1,140                   # [5]  gra_spill_temp_94
    wur.sar_byte    a9                  # [6]
    ee.vld.128.ip   q6,a8,16            # [7]  id:285
    ee.vld.128.ip   q3,a8,16            # [8]  id:286
    ee.vld.128.ip   q7,a8,0             # [9]  id:287
    s32i    a8,a1,140                   # [10]  gra_spill_temp_94
    ee.src.q.qup    q2,q6,q3            # [11]
    ee.vadds.s32    q0,q0,q2            # [12]
    ee.src.q.qup    q5,q6,q7            # [13]
    ee.vadds.s32    q1,q1,q5            # [14]
    st.qr           q1,a15,48                   # [15]  gra_spill_temp_96-112

.Lt_6_11266:    # 0xc19
 # 423                  q0 = esp_nn_multiply_by_quantized_mult_ver1_esp32s3(q0, out_mult_ptr, out_shift_ptr);
    mov.n   a10,a6                      # [0]
    mov.n   a11,a2                      # [1]
    call8   esp_nn_multiply_by_quantized_mult_ver1_esp32s3     # [2]  esp_nn_multiply_by_quantized_mult_ver1_esp32s3

    addi    a11,a1,112                  # [0]
    addi    a10,a6,16                   # [1]
    st.qr   q0,a11,112                  # [2]  gra_spill_temp_100-112
    ld.qr   q0,a11,48                   # [3]  gra_spill_temp_96-112
    addi    a11,a2,16                   # [4]
    call8   esp_nn_multiply_by_quantized_mult_ver1_esp32s3     # [5]  esp_nn_multiply_by_quantized_mult_ver1_esp32s3

    addi    a6,a6,32                    # [0]
    addi    a2,a2,32                    # [1]

    l32i    a13,a1,136                  # [2]  gra_spill_temp_93
    l32i    a12,a1,240                  # [3]  gra_spill_temp_101
    l32i    a10,a1,248                  # [4]  gra_spill_temp_103
    l32i    a11,a1,244                  # [5]  gra_spill_temp_102
    addi    a9,a1,112                   # [6]
    ld.qr   q6,a9,80                    # [7]  gra_spill_temp_98-112
    ld.qr   q7,a9,96                    # [8]  gra_spill_temp_99-112
    ld.qr   q5,a9,64                    # [9]  gra_spill_temp_97-112
    ld.qr   q4,a9,112                   # [10]  gra_spill_temp_100-112
    addi    a11,a11,16                  # [11]
    addi.n  a10,a10,8               # [12]
    ee.vadds.s32    q4,q4,q5            # [13]
    ee.vadds.s32    q5,q0,q5            # [14]
    ee.vmin.s32     q4,q4,q7            # [15]
    ee.vmax.s32     q4,q4,q6            # [16]
    ee.vmin.s32     q5,q5,q7            # [17]
    ee.vmax.s32     q5,q5,q6            # [18]
    ee.vunzip.16    q4,q5               # [19]
    ee.vunzip.8     q4,q5               # [20]
    ee.vst.l.64.ip  q4,a12,8        # [21]  id:290
    blt         a10,a13,.Lt_6_8962          # [22]

.Lt_6_8450: # 0xc76
#<loop> Part of loop body line 348, head labeled .Lt_6_8194
    l32i    a11,a1,96                   # [0]  gra_spill_temp_83
    l32i    a15,a1,104                  # [1]  gra_spill_temp_85
    l32i    a14,a1,84                   # [2]  gra_spill_temp_80
    l32i    a10,a1,100                  # [3]  gra_spill_temp_84
    l32i    a13,a1,108                  # [4]  gra_spill_temp_86
    addi.n  a10,a10,1               # [5]
    s32i    a10,a1,100                  # [6]  gra_spill_temp_84
    sub     a13,a13,a14                 # [7]
    add.n   a15,a15,a14                 # [8]
    s32i    a15,a1,104                  # [9]  gra_spill_temp_85
    s32i    a13,a1,108                  # [10]  gra_spill_temp_86
    sub     a10,a10,a11                 # [11]
    bnez    a10,.Lt_6_8194              # [12]

.Lt_6_7682: # 0xc9b
    l32i.n  a9,a1,56                # [0]  gra_spill_temp_73
    l32i    a15,a1,64                   # [1]  gra_spill_temp_75
    l32i.n  a14,a1,52               # [2]  gra_spill_temp_72
    l32i    a13,a1,80                   # [3]  gra_spill_temp_79
    l32i.n  a11,a1,60               # [4]  gra_spill_temp_74
    l32i    a8,a1,68                    # [5]  gra_spill_temp_76
    l32i    a10,a1,72                   # [6]  gra_spill_temp_77
    addi.n  a8,a8,1                 # [7]
    s32i    a8,a1,68                    # [8]  gra_spill_temp_76
    sub     a10,a10,a11                 # [9]
    add.n   a13,a13,a14                 # [10]
    add.n   a15,a15,a11                 # [11]
    s32i    a15,a1,64                   # [12]  gra_spill_temp_75
    s32i    a13,a1,80                   # [13]  gra_spill_temp_79
    s32i    a10,a1,72                   # [14]  gra_spill_temp_77
    sub     a8,a8,a9                    # [15]
    bnez    a8,.Lt_6_7426               # [16]

.Lt_6_6914: # 0xcc8
    retw.n                          # [0]

    .size   esp_nn_depthwise_conv_s16_mult1_3x3_no_pad_esp32s3, . - esp_nn_depthwise_conv_s16_mult1_3x3_no_pad_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
