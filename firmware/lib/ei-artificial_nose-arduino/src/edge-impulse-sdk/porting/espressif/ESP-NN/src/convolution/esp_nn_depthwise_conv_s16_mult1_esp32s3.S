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

    # Program Unit: esp_nn_depthwise_conv_s16_mult1_esp32s3
    .type   esp_nn_depthwise_conv_s16_mult1_esp32s3, @function
    .align   4
    .global esp_nn_depthwise_conv_s16_mult1_esp32s3

esp_nn_depthwise_conv_s16_mult1_esp32s3:    # 0x4c8
    # scratch_buf = 0
    # gra_spill_temp_2 = 48
    # gra_spill_temp_22 = 52
    # gra_spill_temp_4 = 56
    # gra_spill_temp_23 = 60
    # gra_spill_temp_24 = 64
    # gra_spill_temp_7 = 68
    # gra_spill_temp_26 = 72
    # gra_spill_temp_27 = 76
    # gra_spill_temp_28 = 80
    # gra_spill_temp_29 = 84
    # gra_spill_temp_12 = 88
    # gra_spill_temp_13 = 92
    # gra_spill_temp_14 = 96
    # gra_spill_temp_15 = 100
    # gra_spill_temp_21 = 104
    # gra_spill_temp_17 = 108
    # gra_spill_temp_18 = 112
    # gra_spill_temp_20 = 116
    # gra_spill_temp_30 = 0
    # gra_spill_temp_34 = 16

 // in registers:
 // a2: *input_data
 // a3: input_wd
 // a4: input_ht
 // a5: channels
 // a6: pad_wd
 // a7: pad_ht

 // on stack:
 // stride_wd
 // stride_ht
 // *filter_data
 // filter_wd
 // filter_ht
 // *bias
 // *out_data
 // out_wd
 // out_ht
 // out_offset
 // *out_shift
 // *out_mult
 // activation_min
 // activation_max

    entry   a1,160                      #
    l32i    a9,a1,184                   # [7]  id:237 out_data+0x0
    l16ui   a8,a1,192                   # [8]  id:238 out_ht+0x0
    s32i    a2,a1,52                    # [0]  gra_spill_temp_22
    s32i.n  a4,a1,56                # [1]  gra_spill_temp_4
    s32i    a5,a1,60                    # [2]  gra_spill_temp_23
    s32i    a9,a1,112                   # [10]  gra_spill_temp_18
    beqz.n  a8,.Lt_4_7170           # [20]

.LBB3_esp_nn_depthwise_conv_s16_mult1:  # 0x508
    l16ui   a4,a1,172                   # [0]  id:240 filter_wd+0x0
    neg     a13,a7                      # [2]
    neg     a12,a6                      # [3]
    sext    a12,a12,15                  # [16]
    sext    a13,a13,15                  # [17]
    s32i    a13,a1,92                   # [18]  gra_spill_temp_13
    s32i.n  a12,a1,48               # [19]  gra_spill_temp_2
    movi.n  a8,0                    # [20]
    slli    a9,a5,1                     # [21]
    addi    a10,a5,-7                   # [22]
    s32i    a10,a1,100                  # [23]  gra_spill_temp_15
    s32i    a9,a1,64                    # [24]  gra_spill_temp_24
    s32i    a8,a1,68                    # [25]  gra_spill_temp_7
    j   .Lt_4_7682                      # [30]

.Lt_4_7938: # 0x561
    l32i    a15,a1,192                  # [0]  out_ht
    l32i.n  a9,a1,164                   # [1]  stride_ht
    l32i    a14,a1,68                   # [2]  gra_spill_temp_7
    l32i    a8,a1,92                    # [3]  gra_spill_temp_13
    addi.n  a14,a14,1               # [4]
    s32i    a14,a1,68                   # [5]  gra_spill_temp_7
    add.n   a9,a8,a9                    # [6]
    sub     a14,a14,a15                 # [7]
    sext    a8,a9,15                    # [8]
    s32i    a8,a1,92                    # [9]  gra_spill_temp_13
    beqz    a14,.Lt_4_7170              # [10]

.Lt_4_7682: # 0x57f
#<loop> Loop body line 59, nesting depth: 1, estimated iterations: 100
 #  60          const int16_t base_y = (out_y * stride_ht) - pad_ht;
 #  61          for (int out_x = 0; out_x < out_wd; out_x++) { //width_loop
    l32i    a10,a1,188                  # [0]  out_width
    beqz.n  a10,.Lt_4_7938          # [2]

.LBB6_esp_nn_depthwise_conv_s16_mult1:  # 0x584
#<loop> Part of loop body line 59, head labeled .Lt_4_7682
    movi.n  a14,0                   # [0]
    l32i.n  a7,a1,176                   # [1]  filter_ht
    l32i    a13,a1,92                   # [2]  gra_spill_temp_13
    l32i.n  a8,a1,56                # [3]  gra_spill_temp_4
    movi.n  a11,0                   # [4]
    l32i.n  a12,a1,48               # [5]  gra_spill_temp_2
    s32i    a12,a1,84                   # [6]  gra_spill_temp_29
    s32i    a11,a1,88                   # [7]  gra_spill_temp_12
    sub     a8,a8,a13                   # [8]
    min     a7,a7,a8                    # [9]
    neg     a13,a13                     # [10]
    max     a13,a13,a14                 # [11]
    s32i    a13,a1,96                   # [12]  gra_spill_temp_14
    j   .Lt_4_8450                      # [13]

.Lt_4_8706: # 0x5a9
#<loop> Part of loop body line 61, head labeled .Lt_4_8450
    l32i    a10,a1,188                  # [0]  out_width
    l32i    a12,a1,160                  # [1]  stride_wd
    l32i    a9,a1,88                    # [2]  gra_spill_temp_12
    l32i    a11,a1,84                   # [3]  gra_spill_temp_29
    addi.n  a9,a9,1                 # [4]
    s32i    a9,a1,88                    # [5]  gra_spill_temp_12
    add.n   a12,a11,a12                 # [6]
    sext    a11,a12,15                  # [7]
    s32i    a11,a1,84                   # [8]  gra_spill_temp_29
    beq     a9,a10,.Lt_4_7938           # [9]

.Lt_4_8450: # 0x5c5
#<loop> Loop body line 61, nesting depth: 2, estimated iterations: 100
 #  69              uint32_t bias_ptr = (uint32_t) bias;
 #  70              const int32_t *out_mult_ptr = out_mult;
 #  71              const int32_t *out_shift_ptr = out_shift;
 #  72
 #  73              for (int ch_idx = 0; ch_idx < channels - 7; ch_idx += 8) {//channel_loop
    l32i    a13,a1,100                  # [0]  gra_spill_temp_15
    l32i    a14,a1,180                  # [1]  bias
    l32i    a15,a1,204                  # [2]  out_mult
    l32i    a8,a1,200                   # [3]  out_shift
    s32i    a8,a1,104                   # [4]  gra_spill_temp_21
    s32i    a15,a1,116                  # [5]  gra_spill_temp_20
    s32i    a14,a1,108                  # [6]  gra_spill_temp_17
    blti    a13,1,.Lt_4_8706            # [7]

.LBB9_esp_nn_depthwise_conv_s16_mult1:  # 0x5dd
#<loop> Part of loop body line 61, head labeled .Lt_4_8450
    movi.n  a2,0                    # [0]
    l32i    a5,a1,84                    # [1]  gra_spill_temp_29
    movi.n  a8,0                    # [2]
    neg     a6,a5                       # [3]
    max     a6,a6,a8                    # [4]
    sub     a5,a3,a5                    # [5]
    min     a5,a4,a5                    # [6]
    sub     a9,a5,a6                    # [7]
    s32i    a9,a1,72                    # [8]  gra_spill_temp_26
    j   .Lt_4_9218                      # [9]

.Lt_4_9474: # 0x5f9

// extract data
    mov     a11,a1
    ee.st.qacc_l.l.128.ip   a11,16      # [2]  id:252
    ee.st.qacc_l.h.32.ip    a11,0       # [3]  id:253
    l8ui    a12,a1,15                   # [4]  scratch_buf+15
    l16ui   a10,a1,10                   # [5]  scratch_buf+10
    l8ui    a13,a1,5                    # [6]  scratch_buf+5
    l8ui    a14,a1,6                    # [7]  scratch_buf+6
    l8ui    a15,a1,16                   # [8]  scratch_buf+16
    s8i     a13,a1,2                    # [11]  scratch_buf+2
    s8i     a14,a1,3                    # [10]  scratch_buf+3
    s8i     a15,a1,7                    # [9]  scratch_buf+7
    s16i    a10,a1,4                    # [12]  scratch_buf+4
    s8i     a12,a1,6                    # [13]  scratch_buf+6

    movi.n  a10,16                  # [14]
    ee.st.qacc_h.l.128.ip   a11,16      # [15]  id:263
    ee.st.qacc_h.h.32.ip    a11,-32     # [16]  id:264
    ee.srcmb.s16.qacc       q1,a10,0        # [17]
    l8ui    a8,a1,31                    # [18]  scratch_buf+31
    l8ui    a9,a1,32                    # [19]  scratch_buf+32
    l16ui   a12,a1,16                   # [20]  scratch_buf+16
    l8ui    a13,a1,21                   # [21]  scratch_buf+21
    l8ui    a14,a1,22                   # [22]  scratch_buf+22
    l16ui   a15,a1,26                   # [23]  scratch_buf+26
    s8i     a13,a1,10                   # [26]  scratch_buf+10
    s8i     a14,a1,11                   # [25]  scratch_buf+11
    s16i    a15,a1,12                   # [24]  scratch_buf+12
    s16i    a12,a1,8                    # [27]  scratch_buf+8
    s8i     a9,a1,15                    # [28]  scratch_buf+15
    s8i     a8,a1,14                    # [29]  scratch_buf+14

    l32i            a9,a1,180                   # [30]  bias
    ee.vld.128.ip   q0,a11,0            # [31]  id:164
    ee.vzip.16      q0,q1               # [33]
    beqz.n          a9,.Lt_4_11522          # [34] // skip bias

// add bias
    l32i    a9,a1,108                   # [0]  gra_spill_temp_17
    addi    a8,a1,112                   # [1]
    extui   a10,a9,0,4                  # [2]
    wur.sar_byte    a10                 # [3]
    ee.vld.128.ip   q4,a9,16            # [4]  id:279
    ee.vld.128.ip   q7,a9,16            # [5]  id:168
    ee.vld.128.ip   q5,a9,0             # [6]  id:281
    s32i    a9,a1,108                   # [7]  gra_spill_temp_17
    ee.src.q    q4,q4,q7            # [8]
    ee.src.q    q7,q7,q5            # [10]
    ee.vadds.s32    q0,q0,q4            # [9]
    ee.vadds.s32    q1,q1,q7            # [11]
    st.qr   q1,a1,0                 # [12]  gra_spill_temp_30-112

.Lt_4_11522:    # 0x684

// apply quantisation: esp_nn_multiply_by_quantized_mult_ver1_esp32s3(q0, out_mult_ptr, out_shift_ptr);

    l32i    a10,a1,116                  # [1]  gra_spill_temp_20
    l32i    a11,a1,104                  # [3]  gra_spill_temp_21
    st.qr   q1,a1,0                 # [2]  gra_spill_temp_30-112
    call8   esp_nn_multiply_by_quantized_mult_ver1_esp32s3     # [4]  esp_nn_multiply_by_quantized_mult_ver1_esp32s3

    l32i    a10,a1,116                  # [2]  gra_spill_temp_20
    l32i    a11,a1,104                  # [0]  gra_spill_temp_21
    st.qr   q0,a1,16                # [3]  gra_spill_temp_34-112
    ld.qr   q0,a1,0                 # [4]  gra_spill_temp_30-112
    addi    a10,a10,16                  # [5] // out_mult_ptr += 4
    addi    a11,a11,16                  # [6] // out_shift_ptr += 4
    call8   esp_nn_multiply_by_quantized_mult_ver1_esp32s3     # [7]  esp_nn_multiply_by_quantized_mult_ver1_esp32s3

// add offset, apply activation and store
    l32i    a13,a1,100                  # [0]  gra_spill_temp_15
    addi.n  a2,a2,8                 # [1]
    l32i    a8,a1,112                   # [2]  gra_spill_temp_18
    l32i    a15,a1,116                  # [3]  gra_spill_temp_20
    l32i    a14,a1,104                  # [4]  gra_spill_temp_21

    addi        a12,a1,212
    ee.vldbc.32 q3,a12              # [14]  id:236 activation_max
    addi        a12,a1,196
    ee.vldbc.32 q1,a12              # [16]  id:234 out_offset
    addi    a12,a1,208

    ld.qr   q2,a1,16                # [8]  gra_spill_temp_34-112

    addi    a14,a14,32                  # [9]
    addi    a15,a15,32                  # [10]
    s32i    a15,a1,116                  # [11]  gra_spill_temp_20
    ee.vadds.s32    q2,q2,q1            # [12]
    s32i            a14,a1,104                  # [13]  gra_spill_temp_21
    ee.vadds.s32    q1,q0,q1            # [14]
    ee.vmin.s32     q0,q2,q3            # [15]
    ee.vldbc.32     q2,a12              # [16]  id:234 out_offset
    ee.vmin.s32     q1,q1,q3            # [17]
    ee.vmax.s32     q1,q1,q2            # [18]
    ee.vmax.s32     q0,q0,q2            # [19]
    ee.vunzip.16    q0,q1               # [20]
    ee.vunzip.8     q0,q1               # [21]
    ee.vst.l.64.ip  q0,a8,8         # [22]  id:172
    s32i    a8,a1,112                   # [23]  gra_spill_temp_18
    bge     a2,a13,.Lt_4_8706           # [24]

.Lt_4_9218: # 0x6f5
    ee.zero.qacc                    # [0]
    l32i    a13,a1,96                   # [1]  gra_spill_temp_14
    s32i    a13,a1,80                   # [2]  gra_spill_temp_28
    bge     a13,a7,.Lt_4_9474           # [3]

.LBB12_esp_nn_depthwise_conv_s16_mult1: # 0x701 // channel_loop
    mull    a15,a13,a4                  # [0]
    l32i    a14,a1,92                   # [1]  gra_spill_temp_13
    add.n   a8,a15,a5                   # [2]
    add.n   a14,a14,a13                 # [3]
    mull    a14,a3,a14                  # [4]
    s32i    a8,a1,76                    # [5]  gra_spill_temp_27
    bge     a6,a5,.Lt_4_10242           # [6]

.LBB15_esp_nn_depthwise_conv_s16_mult1: # 0x714
    l32i    a12,a1,64                   # [0]  gra_spill_temp_24
    l32i    a9,a1,168                   # [1]  filter_data
    l32i    a10,a1,60                   # [2]  gra_spill_temp_23
    l32i    a11,a1,84                   # [3]  gra_spill_temp_29
    add.n   a8,a15,a6                   # [4]
    add.n   a11,a11,a6                  # [5]
    mull    a8,a8,a10                   # [6]
    add.n   a11,a14,a11                 # [7]
    mull    a10,a10,a11                 # [8]
    add.n   a8,a2,a8                    # [9]
    l32i    a11,a1,52                   # [10]  gra_spill_temp_22
    addx2   a8,a8,a9                    # [11]
    add.n   a10,a2,a10                  # [12]
    l32i    a9,a1,72                    # [13]  gra_spill_temp_26
    addx2   a10,a10,a11                 # [14]
    loopgtz a9,.LBB41_esp_nn_depthwise_conv_s16_mult1   # [15]
// innermost loop
    ee.vld.128.xp   q0,a10,a12          # [0*II+3]  id:249
    ee.vld.128.xp   q1,a8,a12           # [0*II+4]  id:250
    ee.vmulas.s16.qacc  q0,q1       # [0*II+6]
.LBB41_esp_nn_depthwise_conv_s16_mult1: # 0x750

.Lt_4_10242:    # 0x750
    add.n   a14,a14,a3                  # [0]
    add.n   a15,a15,a4                  # [1]
    l32i    a9,a1,80                    # [2]  gra_spill_temp_28
    l32i    a10,a1,76                   # [3]  gra_spill_temp_27
    addi.n  a9,a9,1                 # [4]
    add.n   a10,a10,a4                  # [5]
    s32i    a10,a1,76                   # [6]  gra_spill_temp_27
    s32i    a9,a1,80                    # [7]  gra_spill_temp_28
    sub     a9,a7,a9                    # [8]
    beqz    a9,.Lt_4_9474               # [9]

    blt a6,a5,.LBB15_esp_nn_depthwise_conv_s16_mult1    # [0]

    j   .Lt_4_10242                     # [0]

.Lt_4_7170: # 0x770
    retw.n                          # [0]

    .size   esp_nn_depthwise_conv_s16_mult1_esp32s3, . - esp_nn_depthwise_conv_s16_mult1_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
