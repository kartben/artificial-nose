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

    # Program Unit: esp_nn_depthwise_conv_s16_mult8_3x3_esp32s3
    .type   esp_nn_depthwise_conv_s16_mult8_3x3_esp32s3, @function
    .align   4
    .global esp_nn_depthwise_conv_s16_mult8_3x3_esp32s3

esp_nn_depthwise_conv_s16_mult8_3x3_esp32s3:    # 0x11b3
    # qacc_scratch = 0
    # gra_spill_temp_142 = 48
    # gra_spill_temp_143 = 52
    # gra_spill_temp_144 = 56
    # gra_spill_temp_145 = 60
    # gra_spill_temp_146 = 64
    # gra_spill_temp_147 = 68
    # gra_spill_temp_148 = 72
    # gra_spill_temp_149 = 76
    # gra_spill_temp_150 = 80
    # gra_spill_temp_151 = 84
    # gra_spill_temp_152 = 88
    # gra_spill_temp_153 = 92
    # gra_spill_temp_154 = 96
    # gra_spill_temp_155 = 100
    # gra_spill_temp_156 = 104
    # gra_spill_temp_157 = 108
    # gra_spill_temp_158 = 112
    # gra_spill_temp_159 = 116
    # gra_spill_temp_160 = 120
    # gra_spill_temp_161 = 124
    # gra_spill_temp_162 = 128
    # gra_spill_temp_163 = 132
    # gra_spill_temp_164 = 136
    # gra_spill_temp_165 = 140
    # gra_spill_temp_166 = 144
    # gra_spill_temp_167 = 148
    # gra_spill_temp_168 = 152
    # gra_spill_temp_169 = 156
    # gra_spill_temp_170 = 160
    # gra_spill_temp_171 = 164
    # gra_spill_temp_172 = 168
    # gra_spill_temp_173 = 172
    # gra_spill_temp_174 = 176
    # gra_spill_temp_175 = 180
    # gra_spill_temp_176 = 184
    # gra_spill_temp_177 = 188
    # gra_spill_temp_178 = 192
    # gra_spill_temp_179 = 208
    # gra_spill_temp_180 = 224
    # gra_spill_temp_181 = 240
    # gra_spill_temp_182 = 256

 // registers:
 // a2: const int16_t *input_data
 // a3: const uint16_t input_wd
 // a4: const uint16_t input_ht
 // a5: const uint16_t channels
 // a6: const uint16_t pad_wd
 // a7: const uint16_t pad_ht

 // const uint16_t stride_wd
 // const uint16_t stride_ht
 // const uint16_t ch_mult
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

    entry   a1,304                      #
    s32i    a2,a1,116                   # [0]  gra_spill_temp_159
    s32i    a3,a1,120                   # [1]  gra_spill_temp_160
    s32i    a5,a1,144                   # [2]  gra_spill_temp_166
    s32i.n  a6,a1,60                # [3]  gra_spill_temp_145

    addmi   a9,a1,256                   # [4]
    addi    a12,a1,112                  # [5]
    addmi   a10,a1,256                  # [6]
    addmi   a11,a1,256                  # [7]
    addmi   a13,a1,256                  # [8]

 // height loop
    l16ui   a8,a1,332                   # [9]  id:261 out_ht+0x0
    l32i    a14,a1,324                  # [10]  id:257 out_data+0x0
    s32i    a14,a1,176                  # [11]  gra_spill_temp_174
    s32i    a8,a1,68                    # [12]  gra_spill_temp_147
    addi    a13,a13,80                  # [13]
    addi    a11,a11,96                  # [14]
    addi    a10,a10,92                  # [15]
    ee.vldbc.32 q0,a10              # [16]  id:260 activation_min
    ee.vldbc.32 q1,a11              # [17]  id:259 activation_max
    ee.vldbc.32 q2,a13              # [18]  id:258 out_offset
    st.qr   	q2,a12,96                   # [19]  gra_spill_temp_179-112
    st.qr   	q1,a12,112                  # [20]  gra_spill_temp_180-112
    st.qr   	q0,a9,-16                   # [21]  gra_spill_temp_181-256
    beqz.n  a8,.Lt_8_8194           # [22]

.LBB3_esp_nn_depthwise_conv_s16_mult8_3x3:  # 0x11f9
    s32i    a1,a1,180                   # [0]  gra_spill_temp_175
    mul16u  a6,a3,a5                # [1]
    s32i    a7,a1,76                    # [2]  gra_spill_temp_149
    l32i    a9,a1,316                   # [3]  id:264 filter_data+0x0
    l32i    a15,a1,320                  # [4]  id:262 bias+0x0
    l16ui   a10,a1,312                  # [5]  id:263 ch_mult+0x0
    slli    a11,a5,1                    # [6]
    l16ui   a12,a1,308                  # [7]  id:268 stride_ht+0x0
    l32i    a13,a1,344                  # [8]  id:267 out_mult+0x0
    l32i    a14,a1,340                  # [9]  id:266 out_shift+0x0
    s32i    a14,a1,88                   # [10]  gra_spill_temp_152
    s32i    a13,a1,92                   # [11]  gra_spill_temp_153
    s32i    a12,a1,64                   # [12]  gra_spill_temp_146
    s32i    a11,a1,124                  # [13]  gra_spill_temp_161
    s32i    a10,a1,108                  # [14]  gra_spill_temp_157
    s32i    a15,a1,160                  # [15]  gra_spill_temp_170
    s32i    a9,a1,128                   # [16]  gra_spill_temp_162
    neg     a7,a7                       # [17]
    slli    a6,a6,1                     # [18]
    s32i    a7,a1,136                   # [19]  gra_spill_temp_164
    movi.n  a9,0                    # [20]
    extui   a15,a15,0,4                 # [21]
    s32i    a15,a1,152                  # [22]  gra_spill_temp_168
    s32i    a9,a1,72                    # [23]  gra_spill_temp_148
    sub     a7,a4,a7                    # [24]
    l32i.n  a9,a1,60                # [25]  gra_spill_temp_145
    s32i    a7,a1,80                    # [26]  gra_spill_temp_150
    l16ui   a4,a1,328                   # [27]  id:269 out_wd+0x0
    s32i    a4,a1,96                    # [28]  gra_spill_temp_154
    l16ui   a7,a1,304                   # [29]  id:265 stride_wd+0x0
    s32i    a7,a1,84                    # [30]  gra_spill_temp_151
    mul16u  a4,a5,a10               # [31]
    neg     a9,a9                       # [32]
    s32i.n  a9,a1,52                # [33]  gra_spill_temp_143
    sub     a8,a3,a9                    # [34]
    addi    a10,a10,-7                  # [35]
    s32i    a10,a1,164                  # [36]  gra_spill_temp_171
    s32i.n  a8,a1,56                # [37]  gra_spill_temp_144
    addx2   a7,a4,a4                    # [38]
    slli    a7,a7,1                     # [39]
    j       .Lt_8_8706                      # [40]

.Lt_8_8962: # 0x1270
#<loop> Part of loop body line 933, head labeled .Lt_8_8706
    l32i    a10,a1,68                   # [0]  gra_spill_temp_147
    l32i    a14,a1,76                   # [1]  gra_spill_temp_149
    l32i    a13,a1,136                  # [2]  gra_spill_temp_164
    l32i    a12,a1,64                   # [3]  gra_spill_temp_146
    l32i    a9,a1,72                    # [4]  gra_spill_temp_148
    l32i    a11,a1,80                   # [5]  gra_spill_temp_150
    addi.n  a9,a9,1                 # [6]
    s32i    a9,a1,72                    # [7]  gra_spill_temp_148
    sub     a11,a11,a12                 # [8]
    add.n   a13,a13,a12                 # [9]
    sub     a14,a14,a12                 # [10]
    s32i    a14,a1,76                   # [11]  gra_spill_temp_149
    s32i    a13,a1,136                  # [12]  gra_spill_temp_164
    s32i    a11,a1,80                   # [13]  gra_spill_temp_150
    sub     a9,a9,a10                   # [14]
    beqz    a9,.Lt_8_8194               # [15]

.Lt_8_8706: # 0x129e
#<loop> Loop body line 933, nesting depth: 1, estimated iterations: 100
 # 934          const int32_t base_y = (out_y * stride_ht) - pad_ht;
 # 935          for (int out_x = 0; out_x < out_wd; out_x++) { //width_loop
    l32i    a15,a1,96                   # [0]  gra_spill_temp_154
    beqz.n  a15,.Lt_8_8962          # [2]

.LBB6_esp_nn_depthwise_conv_s16_mult8_3x3:  # 0x12a3
#<loop> Part of loop body line 933, head labeled .Lt_8_8706
    l32i.n  a3,a1,56                # [0]  gra_spill_temp_144
    l32i    a8,a1,80                    # [1]  gra_spill_temp_150
    movi.n  a10,0                   # [2]
    l32i    a9,a1,76                    # [3]  gra_spill_temp_149
    movi.n  a11,0                   # [4]
    l32i.n  a12,a1,52               # [5]  gra_spill_temp_143
    l32i.n  a13,a1,60               # [6]  gra_spill_temp_145
    s32i    a13,a1,104                  # [7]  gra_spill_temp_156
    s32i    a12,a1,140                  # [8]  gra_spill_temp_165
    s32i    a11,a1,100                  # [9]  gra_spill_temp_155
    max     a9,a9,a10                   # [10]
    movi.n  a10,3                   # [11]
    s32i    a9,a1,172                   # [12]  gra_spill_temp_173
    min     a8,a8,a10                   # [13]
    s32i    a8,a1,156                   # [14]  gra_spill_temp_169
    sub     a8,a8,a9                    # [15]
    s32i    a8,a1,132                   # [16]  gra_spill_temp_163
    j       .Lt_8_9474                      # [17]

.Lt_8_9730: # 0x12d3
#<loop> Part of loop body line 935, head labeled .Lt_8_9474
    l32i    a15,a1,96                   # [0]  gra_spill_temp_154
    l32i    a10,a1,104                  # [1]  gra_spill_temp_156
    l32i    a9,a1,140                   # [2]  gra_spill_temp_165
    l32i    a8,a1,84                    # [3]  gra_spill_temp_151
    l32i    a14,a1,100                  # [4]  gra_spill_temp_155
    sub     a3,a3,a8                    # [5]
    addi.n  a14,a14,1               # [6]
    s32i    a14,a1,100                  # [7]  gra_spill_temp_155
    add.n   a9,a9,a8                    # [8]
    sub     a10,a10,a8                  # [9]
    s32i    a10,a1,104                  # [10]  gra_spill_temp_156
    s32i    a9,a1,140                   # [11]  gra_spill_temp_165
    beq     a14,a15,.Lt_8_8962          # [12]

.Lt_8_9474: # 0x12f8
 # 936              const int32_t base_x = (out_x * stride_wd) - pad_wd;
 # 937              const int32_t *out_mult_ptr = out_mult;
 # 938              const int32_t *out_shift_ptr = out_shift;
    l32i    a2,a1,88                    # [0]  gra_spill_temp_152
    l32i    a10,a1,92                   # [1]  gra_spill_temp_153
 # 939              uint32_t bias_ptr = (uint32_t) (bias);
    l32i    a12,a1,160                  # [2]  gra_spill_temp_170
 # 940
 # 941              for (int ch_idx = 0; ch_idx < channels; ch_idx++) {//channel_loop
    l32i    a11,a1,144                  # [3]  gra_spill_temp_166
    s32i    a12,a1,168                  # [4]  gra_spill_temp_172
    beqz.n  a11,.Lt_8_9730          # [5]

.LBB9_esp_nn_depthwise_conv_s16_mult8_3x3:  # 0x1309
#<loop> Part of loop body line 935, head labeled .Lt_8_9474
    movi.n  a8,0                    # [0]
    l32i    a5,a1,104                   # [1]  gra_spill_temp_156
    movi.n  a13,0                   # [2]
    movi.n  a9,0                    # [3]
    s32i    a9,a1,112                   # [4]  gra_spill_temp_158
    s32i    a13,a1,148                  # [5]  gra_spill_temp_167
    max     a5,a5,a8                    # [6]
    j       .Lt_8_10242                     # [7]

.Lt_8_10498:    # 0x131e
#<loop> Part of loop body line 941, head labeled .Lt_8_10242
    l32i    a12,a1,144                  # [0]  gra_spill_temp_166
    l32i    a14,a1,108                  # [1]  gra_spill_temp_157
    l32i    a11,a1,148                  # [2]  gra_spill_temp_167
    l32i    a13,a1,112                  # [3]  gra_spill_temp_158
    addi.n  a11,a11,1               # [4]
    s32i    a11,a1,148                  # [5]  gra_spill_temp_167
    add.n   a13,a13,a14                 # [6]
    s32i    a13,a1,112                  # [7]  gra_spill_temp_158
    beq     a11,a12,.Lt_8_9730          # [8]

.Lt_8_10242:    # 0x1337
 # 942                  for (int ch_mult_idx = 0; ch_mult_idx < ch_mult - 7; ch_mult_idx += 8) {
    l32i    a15,a1,164                  # [0]  gra_spill_temp_171
    blti    a15,1,.Lt_8_10498           # [2]

    movi.n  a8,0                    # [0]
    l32i    a9,a1,112                   # [1]  gra_spill_temp_158
    s32i    a9,a1,188                   # [2]  gra_spill_temp_177
    s32i    a8,a1,184                   # [3]  gra_spill_temp_176
    j   .Lt_8_11010                     # [4]

.LBB23_esp_nn_depthwise_conv_s16_mult8_3x3: # 0x134b
    s32i.n  a10,a1,48               # [0]  gra_spill_temp_142
    addi    a11,a1,112                  # [1]
    l32i    a13,a1,152                  # [2]  gra_spill_temp_168
    l32i    a12,a1,168                  # [3]  gra_spill_temp_172
    wur.sar_byte    a13                 # [4]
    ee.vld.128.ip   q4,a12,16           # [5]  id:307
    ee.vld.128.ip   q7,a12,16           # [6]  id:308
    ee.vld.128.ip   q5,a12,0            # [7]  id:309
    s32i    a12,a1,168                  # [8]  gra_spill_temp_172
    ee.src.q.qup    q6,q4,q7            # [9]
    ee.vadds.s32    q0,q0,q6            # [10]
    ee.src.q.qup    q3,q4,q5            # [11]
    ee.vadds.s32    q1,q1,q3            # [12]
    st.qr   q1,a11,80                   # [13]  gra_spill_temp_178-112

.Lt_8_13314:    # 0x1374
 #1025  q0 = esp_nn_multiply_by_quantized_mult_ver1_esp32s3(q0, out_mult_ptr, out_shift_ptr);
    l32i.n  a10,a1,48               # [0]  gra_spill_temp_142
    mov.n   a11,a2                      # [1]
    call8   esp_nn_multiply_by_quantized_mult_ver1_esp32s3

 #1026                      out_mult_ptr += 4;
 #1027                      out_shift_ptr += 4;
 #1028
 #1029   q1 = esp_nn_multiply_by_quantized_mult_ver1_esp32s3(q1, out_mult_ptr, out_shift_ptr);
    l32i.n  a10,a1,48               # [0]  gra_spill_temp_142
    addmi   a12,a1,256                  # [1]
    addi    a11,a1,112                  # [2]
    st.qr   q0,a12,0                    # [3]  gra_spill_temp_182-256
    ld.qr   q0,a11,80                   # [4]  gra_spill_temp_178-112
    addi    a10,a10,16                  # [5]
    addi    a11,a2,16                   # [6]
    call8   esp_nn_multiply_by_quantized_mult_ver1_esp32s3

#<loop> Part of loop body line 942, head labeled .Lt_8_11010
 #1030                      out_mult_ptr += 4;
 #1031                      out_shift_ptr += 4;
    addi    a2,a2,32                    # [0]
    l32i    a14,a1,164                  # [1]  gra_spill_temp_171

    l32i    a8,a1,176                   # [2]  gra_spill_temp_174
    l32i    a15,a1,188                  # [3]  gra_spill_temp_177
    l32i    a13,a1,184                  # [4]  gra_spill_temp_176
    l32i.n  a10,a1,48               # [5]  gra_spill_temp_142
    addmi   a11,a1,256                  # [6]
    addi    a12,a1,112                  # [7]
    ld.qr   q3,a12,112                  # [8]  gra_spill_temp_180-112
    ld.qr   q1,a12,96                   # [9]  gra_spill_temp_179-112
    ld.qr   q2,a11,0                    # [10]  gra_spill_temp_182-256
    addi    a10,a10,32                  # [11]
    addi.n  a13,a13,8               # [12]
    addi.n  a15,a15,8               # [13]
    s32i    a15,a1,188                  # [14]  gra_spill_temp_177
    ee.vadds.s32    q2,q2,q1            # [15]
    s32i    a13,a1,184                  # [16]  gra_spill_temp_176
    ee.vadds.s32    q1,q0,q1            # [17]
    ee.vmin.s32     q0,q2,q3            # [18]
    ld.qr           q2,a11,-16                  # [19]  gra_spill_temp_181-256
    ee.vmin.s32     q1,q1,q3            # [20]
    ee.vmax.s32     q1,q1,q2            # [21]
    ee.vmax.s32     q0,q0,q2            # [22]
    ee.vunzip.16    q0,q1               # [23]
    ee.vunzip.8     q0,q1               # [24]
    ee.vst.l.64.ip  q0,a8,8         # [25]  id:312
    s32i    a8,a1,176                   # [26]  gra_spill_temp_174
    bge     a13,a14,.Lt_8_10498         # [27]

.Lt_8_11010:    # 0x13e3
#<loop> Loop body line 942, nesting depth: 4, estimated iterations: 100
    l32i    a14,a1,156                  # [0]  gra_spill_temp_169
    l32i    a13,a1,172                  # [1]  gra_spill_temp_173
    ee.zero.qacc                    # [2]
    bge     a13,a14,.Lt_8_11266         # [3]

.LBB15_esp_nn_depthwise_conv_s16_mult8_3x3: # 0x13ef
#<loop> Part of loop body line 942, head labeled .Lt_8_11010
    l32i    a12,a1,124                  # [0]  gra_spill_temp_161
    l32i    a8,a1,140                   # [1]  gra_spill_temp_165
    l32i    a11,a1,120                  # [2]  gra_spill_temp_160
    l32i    a14,a1,188                  # [3]  gra_spill_temp_177
    l32i    a9,a1,136                   # [4]  gra_spill_temp_164
    mull    a15,a4,a13                  # [5]
    add.n   a9,a9,a13                   # [6]
    addx2   a15,a15,a15                 # [7]
    l32i    a13,a1,148                  # [8]  gra_spill_temp_167
    add.n   a14,a14,a15                 # [9]
    mull    a9,a9,a11                   # [10]
    l32i    a15,a1,144                  # [11]  gra_spill_temp_166
    add.n   a8,a8,a9                    # [12]
    mull    a15,a15,a8                  # [13]
    l32i    a8,a1,128                   # [14]  gra_spill_temp_162
    add.n   a13,a13,a15                 # [15]
    l32i    a15,a1,116                  # [16]  gra_spill_temp_159
    addx2   a14,a14,a8                  # [17]
    addx2   a13,a13,a15                 # [18]
    add.n   a11,a12,a13                 # [19]
    l32i    a15,a1,132                  # [20]  gra_spill_temp_163
    add.n   a12,a12,a11                 # [21]
    loopgtz a15,.LBB34_esp_nn_depthwise_conv_s16_mult8_3x3  # [22]

.Lt_8_11778:    # 0x142e
    mov.n   a15,a14                     # [0]
    mov.n   a9,a14                      # [1]
    bnez.n  a5,.Lt_8_12034          # [2]

    ee.vldbc.16     q3,a13              # [0]  id:271
    mov.n           a9,a14                      # [1]
    ee.vld.128.ip       q4,a9,0             # [2]  id:272
    ee.vmulas.s16.qacc  q3,q4       # [4]

.Lt_8_12034:    # 0x143f
    ee.vldbc.16     q5,a11              # [0]  id:274
    addx2           a9,a4,a9                    # [1]
    ee.vld.128.ip   q6,a9,0             # [2]  id:275
    add.n           a13,a13,a6                  # [3]
    ee.vmulas.s16.qacc  q5,q6       # [4]
    blti    a3,3,.Lt_8_12546            # [5]

    ee.vldbc.16     q7,a12              # [0]  id:277
    addx2           a14,a4,a9                   # [1]
    ee.vld.128.ip   q0,a14,0            # [2]  id:278
    ee.vmulas.s16.qacc  q7,q0       # [4]

.Lt_8_12546:    # 0x145c
#<loop> Part of loop body line 953, head labeled .Lt_8_11778
    add.n   a11,a11,a6                  # [0]
    add.n   a12,a12,a6                  # [1]
    add.n   a14,a7,a15                  # [2]

.LBB34_esp_nn_depthwise_conv_s16_mult8_3x3: # 0x1464
.Lt_8_11266:    # 0x1464

    l32i    a8,a1,180                   # [0]  gra_spill_temp_175
    ee.st.qacc_l.l.128.ip   a8,16       # [2]  id:280
    ee.st.qacc_l.h.32.ip    a8,0        # [3]  id:281
    l16ui   a9,a1,10                    # [4]  qacc_scratch+10
    l8ui    a11,a1,15                   # [5]  qacc_scratch+15
    l8ui    a12,a1,5                    # [6]  qacc_scratch+5
    l8ui    a13,a1,6                    # [7]  qacc_scratch+6
    l8ui    a14,a1,16                   # [8]  qacc_scratch+16
    s8i     a14,a1,7                    # [9]  qacc_scratch+7
    s8i     a13,a1,3                    # [10]  qacc_scratch+3
    s8i     a12,a1,2                    # [11]  qacc_scratch+2
    s8i     a11,a1,6                    # [12]  qacc_scratch+6
    s16i    a9,a1,4                     # [13]  qacc_scratch+4
    ee.st.qacc_h.l.128.ip   a8,16       # [14]  id:291
    ee.st.qacc_h.h.32.ip    a8,-32      # [15]  id:292
    l16ui   a9,a1,16                    # [16]  qacc_scratch+16
    l8ui    a15,a1,32                   # [17]  qacc_scratch+32
    l8ui    a12,a1,22                   # [18]  qacc_scratch+22
    l8ui    a11,a1,21                   # [19]  qacc_scratch+21
    l8ui    a14,a1,31                   # [20]  qacc_scratch+31
    l16ui   a13,a1,26                   # [21]  qacc_scratch+26
    s16i    a13,a1,12                   # [22]  qacc_scratch+12
    s8i 	a14,a1,14                   # [23]  qacc_scratch+14
    s8i 	a11,a1,10                   # [24]  qacc_scratch+10
    s8i 	a12,a1,11                   # [25]  qacc_scratch+11
    s8i 	a15,a1,15                   # [26]  qacc_scratch+15
    s16i    a9,a1,8                     # [27]  qacc_scratch+8
    l32i    a15,a1,160                  # [28]  gra_spill_temp_170
    movi.n  a9,16                   # [29]
    ee.srcmb.s16.qacc   q1,a9,0         # [30]
    ee.vld.128.ip   q0,a8,0             # [31]  id:304
    s32i    a8,a1,180                   # [32]  gra_spill_temp_175
    ee.vzip.16  q0,q1               # [33]
    bnez.n  a15,.LBB23_esp_nn_depthwise_conv_s16_mult8_3x3  # [34]

    s32i.n  a10,a1,48               # [0]  gra_spill_temp_142
    addi    a15,a1,112                  # [1]
    st.qr   q1,a15,80                   # [2]  gra_spill_temp_178-112
    j   .Lt_8_13314                     # [3]

.Lt_8_8194: # 0x14d3
    retw.n                          # [0]

    .size   esp_nn_depthwise_conv_s16_mult8_3x3_esp32s3, . - esp_nn_depthwise_conv_s16_mult8_3x3_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
