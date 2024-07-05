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

    # Program Unit: esp_nn_depthwise_conv_s16_mult1_3x3_esp32s3
    .type   esp_nn_depthwise_conv_s16_mult1_3x3_esp32s3, @function
    .align   4
    .global esp_nn_depthwise_conv_s16_mult1_3x3_esp32s3

esp_nn_depthwise_conv_s16_mult1_3x3_esp32s3:    # 0x776
    # qacc_scratch = 0
    # gra_spill_temp_35 = 48
    # gra_spill_temp_36 = 52
    # gra_spill_temp_37 = 56
    # gra_spill_temp_38 = 60
    # gra_spill_temp_39 = 64
    # gra_spill_temp_40 = 68
    # gra_spill_temp_41 = 72
    # gra_spill_temp_42 = 76
    # gra_spill_temp_43 = 80
    # gra_spill_temp_44 = 84
    # gra_spill_temp_45 = 88
    # gra_spill_temp_46 = 92
    # gra_spill_temp_47 = 96
    # gra_spill_temp_48 = 100
    # gra_spill_temp_49 = 104
    # gra_spill_temp_50 = 108
    # gra_spill_temp_51 = 112
    # gra_spill_temp_52 = 116
    # gra_spill_temp_53 = 120
    # gra_spill_temp_54 = 124
    # gra_spill_temp_55 = 128
    # gra_spill_temp_56 = 132
    # gra_spill_temp_57 = 136
    # gra_spill_temp_58 = 140
    # gra_spill_temp_59 = 144
    # gra_spill_temp_60 = 148
    # gra_spill_temp_61 = 152
    # gra_spill_temp_62 = 156
    # gra_spill_temp_63 = 160
    # gra_spill_temp_64 = 164
    # gra_spill_temp_65 = 168
    # gra_spill_temp_66 = 176
    # gra_spill_temp_67 = 192
    # gra_spill_temp_68 = 208
    # gra_spill_temp_69 = 224
    # gra_spill_temp_70 = 240

 // registers:
 // a2: const int16_t *input_data
 // a3: const uint16_t input_wd
 // a4: const uint16_t input_ht
 // a5: const uint16_t channels
 // a6: const uint16_t pad_wd
 // a7: const uint16_t pad_ht

 // on stack
 // const uint16_t stride_wd
 // const uint16_t stride_ht
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
    s32i    a2,a1,104                   # [0]  gra_spill_temp_49
    s32i    a3,a1,112                   # [1]  gra_spill_temp_51
    s32i    a5,a1,116                   # [2]  gra_spill_temp_52
    s32i.n  a6,a1,56                # [3]  gra_spill_temp_37
    addi    a14,a1,112                  # [4]
    addmi   a11,a1,256                  # [5]
    addmi   a13,a1,256                  # [6]
    addmi   a15,a1,256                  # [7]
    l32i    a9,a1,304                   # [8]  id:251 out_data+0x0
    l16ui   a8,a1,312                   # [9]  id:252 out_ht+0x0
    s32i    a8,a1,64                    # [10]  gra_spill_temp_39
    s32i    a9,a1,156                   # [11]  gra_spill_temp_62
    addi    a15,a15,60                  # [12]
    addi    a13,a13,72                  # [13]
    addi    a11,a11,76                  # [14]
    ee.vldbc.32 q0,a11              # [15]  id:250 activation_max
    ee.vldbc.32 q1,a13              # [16]  id:249 activation_min
    ee.vldbc.32 q2,a15              # [17]  id:248 out_offset
    st.qr   q2,a14,80                   # [18]  gra_spill_temp_67-112
    st.qr   q1,a14,96                   # [19]  gra_spill_temp_68-112
    st.qr   q0,a14,112                  # [20]  gra_spill_temp_69-112
    beqz.n  a8,.Lt_5_7426           # [21]

.LBB3_esp_nn_depthwise_conv_s16_mult1_3x3:  # 0x7b9
    s32i    a1,a1,160                   # [0]  gra_spill_temp_63
    s32i    a7,a1,72                    # [1]  gra_spill_temp_41
    mul16u  a6,a3,a5                # [2]
    l32i    a14,a1,296                  # [3]  id:254 filter_data+0x0
    l32i    a15,a1,300                  # [4]  id:253 bias+0x0
    l16ui   a9,a1,308                   # [5]  id:259 out_wd+0x0
    l16ui   a13,a1,288                  # [6]  id:255 stride_wd+0x0
    neg     a8,a7                       # [7]
    l16ui   a10,a1,292                  # [8]  id:258 stride_ht+0x0
    l32i    a11,a1,324                  # [9]  id:257 out_mult+0x0
    l32i    a12,a1,320                  # [10]  id:256 out_shift+0x0
    s32i    a12,a1,84                   # [11]  gra_spill_temp_44
    s32i    a11,a1,88                   # [12]  gra_spill_temp_45
    s32i.n  a10,a1,60               # [13]  gra_spill_temp_38
    s32i    a8,a1,124                   # [14]  gra_spill_temp_54
    s32i    a13,a1,80                   # [15]  gra_spill_temp_43
    s32i    a9,a1,92                    # [16]  gra_spill_temp_46
    s32i    a15,a1,140                  # [17]  gra_spill_temp_58
    s32i    a14,a1,108                  # [18]  gra_spill_temp_50
    slli    a6,a6,1                     # [19]
    movi.n  a14,16                  # [20]
    extui   a15,a15,0,4                 # [21]
    addi    a9,a5,-7                    # [22]
    movi.n  a13,0                   # [23]
    sub     a8,a4,a8                    # [24]
    addx2   a7,a5,a5                    # [25]
    slli    a7,a7,1                     # [26]
    slli    a4,a5,1                     # [27]
    s32i    a13,a1,68                   # [28]  gra_spill_temp_40
    s32i    a9,a1,144                   # [29]  gra_spill_temp_59
    s32i    a15,a1,132                  # [30]  gra_spill_temp_56
    l32i.n  a9,a1,56                # [31]  gra_spill_temp_37
    s32i    a8,a1,76                    # [32]  gra_spill_temp_42
    neg     a9,a9                       # [33]
    s32i.n  a9,a1,48                # [34]  gra_spill_temp_35
    sub     a8,a3,a9                    # [35]
    s32i.n  a8,a1,52                # [36]  gra_spill_temp_36

.Lt_5_7938: # 0x822
    l32i    a10,a1,92                   # [0]  gra_spill_temp_46
    beqz.n  a10,.Lt_5_8194          # [2]

.LBB6_esp_nn_depthwise_conv_s16_mult1_3x3:  # 0x827
    l32i.n  a5,a1,52                # [0]  gra_spill_temp_36
    l32i    a11,a1,76                   # [1]  gra_spill_temp_42
    movi.n  a13,0                   # [2]
    l32i    a12,a1,72                   # [3]  gra_spill_temp_41
    movi.n  a15,0                   # [4]
    l32i.n  a8,a1,48                # [5]  gra_spill_temp_35
    l32i.n  a9,a1,56                # [6]  gra_spill_temp_37
    s32i    a9,a1,100                   # [7]  gra_spill_temp_48
    s32i    a8,a1,128                   # [8]  gra_spill_temp_55
    s32i    a15,a1,96                   # [9]  gra_spill_temp_47
    max     a12,a12,a13                 # [10]
    s32i    a12,a1,152                  # [11]  gra_spill_temp_61
    movi.n  a13,3                   # [12]
    min     a11,a11,a13                 # [13]
    s32i    a11,a1,136                  # [14]  gra_spill_temp_57
    sub     a11,a11,a12                 # [15]
    s32i    a11,a1,120                  # [16]  gra_spill_temp_53

.Lt_5_8706: # 0x854
    l32i    a2,a1,84                    # [0]  gra_spill_temp_44
    l32i    a10,a1,144                  # [1]  gra_spill_temp_59
    l32i    a11,a1,140                  # [2]  gra_spill_temp_58
    l32i    a12,a1,88                   # [3]  gra_spill_temp_45
    s32i    a12,a1,168                  # [4]  gra_spill_temp_65
    s32i    a11,a1,148                  # [5]  gra_spill_temp_60
    blti    a10,1,.Lt_5_8962            # [6]

    movi.n  a8,0                    # [0]
    movi.n  a13,0                   # [1]
    l32i    a3,a1,100                   # [2]  gra_spill_temp_48
    s32i    a13,a1,164                  # [3]  gra_spill_temp_64
    max     a3,a3,a8                    # [4]

.Lt_5_9474: # 0x876
    l32i    a10,a1,136                  # [0]  gra_spill_temp_57
    l32i    a9,a1,152                   # [1]  gra_spill_temp_61
    ee.zero.qacc                    # [2]
    bge     a9,a10,.Lt_5_9730           # [3]

.LBB12_esp_nn_depthwise_conv_s16_mult1_3x3: # 0x882
    l32i    a12,a1,128                  # [0]  gra_spill_temp_55
    l32i    a15,a1,112                  # [1]  gra_spill_temp_51
    l32i    a10,a1,116                  # [2]  gra_spill_temp_52
    l32i    a13,a1,124                  # [3]  gra_spill_temp_54
    mull    a11,a9,a10                  # [4]
    add.n   a13,a13,a9                  # [5]
    mull    a13,a13,a15                 # [6]
    addx2   a11,a11,a11                 # [7]
    l32i    a9,a1,164                   # [8]  gra_spill_temp_64
    add.n   a12,a12,a13                 # [9]
    mull    a10,a10,a12                 # [10]
    add.n   a11,a9,a11                  # [11]
    l32i    a12,a1,108                  # [12]  gra_spill_temp_50
    add.n   a9,a9,a10                   # [13]
    l32i    a10,a1,104                  # [14]  gra_spill_temp_49
    addx2   a11,a11,a12                 # [15]
    l32i    a12,a1,120                  # [16]  gra_spill_temp_53
    addx2   a9,a9,a10                   # [17]
    loopgtz a12,.LBB32_esp_nn_depthwise_conv_s16_mult1_3x3  # [18]

    mov.n   a13,a9                      # [0]
    mov.n   a12,a11                     # [1]
    mov.n   a9,a11                      # [2]
    mov.n   a11,a13                     # [3]

    beqz.n  a3,.Lt_5_10498          # [4] if (filter_x_start)

    add.n   a11,a4,a13                  # [0]
    add.n   a9,a4,a12                   # [1]
.Lt_5_10498:    # 0x8c5

    ee.vld.128.xp   q0,a11,a4           # [0]  id:261
    ee.vld.128.xp   q1,a9,a4            # [1]  id:262

    bnez.n  a3,.Lt_5_11010          # [2] if (filter_x_start)

    ee.vmulas.s16.qacc  q0,q1       # [0]
    ee.vld.128.xp   q0,a11,a4           # [1]  id:264
    ee.vld.128.xp   q1,a9,a4            # [2]  id:265
.Lt_5_11010:    # 0x8d6

    ee.vmulas.s16.qacc  q0,q1       # [0]
    ee.vld.128.xp   q0,a11,a4           # [1]  id:267
    ee.vld.128.xp   q1,a9,a4            # [2]  id:268
    add.n   a9,a6,a13                   # [3]

    blti    a5,3,.Lt_5_11522            # [4] if (filter_x_end)
    ee.vmulas.s16.qacc  q0,q1       # [0]
.Lt_5_11522:    # 0x8e7

    add.n   a11,a7,a12                  # [0]

.LBB32_esp_nn_depthwise_conv_s16_mult1_3x3: # 0x8eb

.Lt_5_9730: # 0x8eb
 // extract data
    l32i    a9,a1,160                   # [0]  gra_spill_temp_63
    ee.st.qacc_l.l.128.ip   a9,16       # [2]  id:270
    ee.st.qacc_l.h.32.ip    a9,0        # [3]  id:271
    l8ui    a11,a1,15                   # [4]  qacc_scratch+15
    l16ui   a10,a1,10                   # [5]  qacc_scratch+10
    l8ui    a15,a1,16                   # [6]  qacc_scratch+16
    l8ui    a13,a1,6                    # [7]  qacc_scratch+6
    l8ui    a12,a1,5                    # [8]  qacc_scratch+5
    s8i     a12,a1,2                    # [9]  qacc_scratch+2
    s8i     a13,a1,3                    # [10]  qacc_scratch+3
    s8i     a15,a1,7                    # [11]  qacc_scratch+7
    s16i    a10,a1,4                    # [12]  qacc_scratch+4
    s8i     a11,a1,6                    # [13]  qacc_scratch+6

    ee.st.qacc_h.l.128.ip   a9,16       # [14]  id:281
    ee.st.qacc_h.h.32.ip    a9,-32      # [15]  id:282
    ee.srcmb.s16.qacc   q1,a14,0        # [16]
    l8ui    a15,a1,31                   # [17]  qacc_scratch+31
    l8ui    a8,a1,32                    # [18]  qacc_scratch+32
    l16ui   a13,a1,26                   # [19]  qacc_scratch+26
    l8ui    a12,a1,22                   # [20]  qacc_scratch+22
    l8ui    a11,a1,21                   # [21]  qacc_scratch+21
    l16ui   a10,a1,16                   # [22]  qacc_scratch+16
    s16i    a10,a1,8                    # [23]  qacc_scratch+8
    s8i     a11,a1,10                   # [24]  qacc_scratch+10
    s8i     a12,a1,11                   # [25]  qacc_scratch+11
    s16i    a13,a1,12                   # [26]  qacc_scratch+12
    s8i     a8,a1,15                    # [27]  qacc_scratch+15
    s8i     a15,a1,14                   # [28]  qacc_scratch+14


    l32i    a8,a1,140                   # [29]  gra_spill_temp_58 , bias
    ee.vld.128.ip   q0,a9,0             # [30]  id:294
    s32i    a9,a1,160                   # [31]  gra_spill_temp_63
    ee.vzip.16  q0,q1               # [32]
    beqz.n  a8,.Lt_5_12290          # [33] // skip bias

    addi    a8,a1,112                   # [0]
    l32i    a10,a1,132                  # [1]  gra_spill_temp_56
    l32i    a9,a1,148                   # [2]  gra_spill_temp_60
    wur.sar_byte    a10                 # [3]
    ee.vld.128.ip   q4,a9,16            # [4]  id:297
    ee.vld.128.ip   q7,a9,16            # [5]  id:298
    ee.vld.128.ip   q5,a9,0             # [6]  id:299
    s32i    a9,a1,148                   # [7]  gra_spill_temp_60
    ee.src.q.qup    q6,q4,q7            # [8]
    ee.vadds.s32    q0,q0,q6            # [9]
    ee.src.q.qup    q3,q4,q5            # [10]
    ee.vadds.s32    q1,q1,q3            # [11]
    st.qr   q1,a8,64                    # [12]  gra_spill_temp_66-112

.Lt_5_12290:    # 0x974
    addi    a11,a1,112                  # [0]

 # 287                  q0 = esp_nn_multiply_by_quantized_mult_ver1_esp32s3(q0, out_mult_ptr, out_shift_ptr);
    l32i    a10,a1,168                  # [1]  gra_spill_temp_65
    st.qr   q1,a11,64                   # [2]  gra_spill_temp_66-112
    mov.n   a11,a2                      # [3]
    call8   esp_nn_multiply_by_quantized_mult_ver1_esp32s3     # [4]  esp_nn_multiply_by_quantized_mult_ver1_esp32s3

 # 288                  out_mult_ptr += 4;
 # 289                  out_shift_ptr += 4;
 # 290
 # 291                  q1 = esp_nn_multiply_by_quantized_mult_ver1_esp32s3(q1, out_mult_ptr, out_shift_ptr);
    l32i    a10,a1,168                  # [0]  gra_spill_temp_65
    addmi   a12,a1,256                  # [1]
    addi    a11,a1,112                  # [2]
    st.qr   q0,a12,-16                  # [3]  gra_spill_temp_70-256
    ld.qr   q0,a11,64                   # [4]  gra_spill_temp_66-112
    addi    a10,a10,16                  # [5]
    addi    a11,a2,16                   # [6]
    call8   esp_nn_multiply_by_quantized_mult_ver1_esp32s3     # [7]  esp_nn_multiply_by_quantized_mult_ver1_esp32s3

.LBB25_esp_nn_depthwise_conv_s16_mult1_3x3: # 0x99a
#<loop> Part of loop body line 216, head labeled .Lt_5_9474
    movi.n  a14,16                  # [0]
 # 292                  out_mult_ptr += 4;
 # 293                  out_shift_ptr += 4;
    addi    a2,a2,32                    # [1]
    l32i    a15,a1,144                  # [2]  gra_spill_temp_59
    l32i    a9,a1,156                   # [3]  gra_spill_temp_62
    l32i    a8,a1,168                   # [4]  gra_spill_temp_65
    addmi   a12,a1,256                  # [5]
    addi    a13,a1,112                  # [6]
    ld.qr   q3,a13,112                  # [7]  gra_spill_temp_69-112
    ld.qr   q1,a13,80                   # [8]  gra_spill_temp_67-112
    ld.qr   q2,a12,-16                  # [9]  gra_spill_temp_70-256
    addi    a8,a8,32                    # [10]
    s32i    a8,a1,168                   # [11]  gra_spill_temp_65
    ee.vadds.s32    q2,q2,q1            # [12]
    ee.vadds.s32    q1,q0,q1            # [13]
    ee.vmin.s32 q0,q2,q3            # [14]
    ee.vmin.s32 q1,q1,q3            # [15]
    ld.qr   	q2,a13,96                   # [16]  gra_spill_temp_68-112
    l32i    	a13,a1,164                  # [17]  gra_spill_temp_64
    ee.vmax.s32 q1,q1,q2            # [18]
    ee.vmax.s32 q0,q0,q2            # [19]
    addi.n  	a13,a13,8               # [20]
    s32i    	a13,a1,164                  # [21]  gra_spill_temp_64
    ee.vunzip.16    q0,q1               # [22]
    ee.vunzip.8 	q0,q1               # [23]
    ee.vst.l.64.ip  q0,a9,8         # [24]  id:302
    s32i    	a9,a1,156                   # [25]  gra_spill_temp_62
    blt     	a13,a15,.Lt_5_9474          # [26]

.Lt_5_8962: # 0x9e9
#<loop> Part of loop body line 203, head labeled .Lt_5_8706
    l32i    a8,a1,92                    # [0]  gra_spill_temp_46
    l32i    a11,a1,100                  # [1]  gra_spill_temp_48
    l32i    a10,a1,128                  # [2]  gra_spill_temp_55
    l32i    a9,a1,80                    # [3]  gra_spill_temp_43
    l32i    a15,a1,96                   # [4]  gra_spill_temp_47
    sub     a5,a5,a9                    # [5]
    addi.n  a15,a15,1               # [6]
    s32i    a15,a1,96                   # [7]  gra_spill_temp_47
    add.n   a10,a10,a9                  # [8]
    sub     a11,a11,a9                  # [9]
    s32i    a11,a1,100                  # [10]  gra_spill_temp_48
    s32i    a10,a1,128                  # [11]  gra_spill_temp_55
    sub     a15,a15,a8                  # [12]
    bnez    a15,.Lt_5_8706              # [13]

.Lt_5_8194: # 0xa11
#<loop> Part of loop body line 201, head labeled .Lt_5_7938
    l32i    a13,a1,64                   # [0]  gra_spill_temp_39
    l32i    a10,a1,72                   # [1]  gra_spill_temp_41
    l32i    a9,a1,124                   # [2]  gra_spill_temp_54
    l32i.n  a8,a1,60                # [3]  gra_spill_temp_38
    l32i    a12,a1,68                   # [4]  gra_spill_temp_40
    l32i    a15,a1,76                   # [5]  gra_spill_temp_42
    addi.n  a12,a12,1               # [6]
    s32i    a12,a1,68                   # [7]  gra_spill_temp_40
    sub     a15,a15,a8                  # [8]
    add.n   a9,a9,a8                    # [9]
    sub     a10,a10,a8                  # [10]
    s32i    a10,a1,72                   # [11]  gra_spill_temp_41
    s32i    a9,a1,124                   # [12]  gra_spill_temp_54
    s32i    a15,a1,76                   # [13]  gra_spill_temp_42
    sub     a12,a12,a13                 # [14]
    bnez    a12,.Lt_5_7938              # [15]

.Lt_5_7426: # 0xa3e
    retw.n                          # [0]

    .size   esp_nn_depthwise_conv_s16_mult1_3x3_esp32s3, . - esp_nn_depthwise_conv_s16_mult1_3x3_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
