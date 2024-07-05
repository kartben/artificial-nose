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

    # Program Unit: esp_nn_depthwise_conv_s16_mult4_esp32s3
    .type   esp_nn_depthwise_conv_s16_mult4_esp32s3, @function
    .align   4
    .global esp_nn_depthwise_conv_s16_mult4_esp32s3

esp_nn_depthwise_conv_s16_mult4_esp32s3:    # 0x17c8
    # qacc_scratch = 0
    # gra_spill_temp_220 = 32
    # gra_spill_temp_221 = 36
    # gra_spill_temp_222 = 40
    # gra_spill_temp_223 = 44
    # gra_spill_temp_224 = 48
    # gra_spill_temp_225 = 52
    # gra_spill_temp_226 = 56
    # gra_spill_temp_227 = 60
    # gra_spill_temp_228 = 64
    # gra_spill_temp_229 = 68
    # gra_spill_temp_230 = 72
    # gra_spill_temp_231 = 76
    # gra_spill_temp_232 = 80
    # gra_spill_temp_233 = 84
    # gra_spill_temp_234 = 88
    # gra_spill_temp_235 = 92
    # gra_spill_temp_236 = 96
    # gra_spill_temp_237 = 100
    # gra_spill_temp_238 = 104
    # gra_spill_temp_239 = 108
    # gra_spill_temp_240 = 112
    # gra_spill_temp_241 = 116
    # gra_spill_temp_242 = 120
    # gra_spill_temp_243 = 124
    # gra_spill_temp_244 = 128
    # gra_spill_temp_245 = 132
    # gra_spill_temp_246 = 136
    # gra_spill_temp_247 = 140
    # gra_spill_temp_248 = 144
    # gra_spill_temp_249 = 148
    # gra_spill_temp_250 = 152
    # gra_spill_temp_251 = 156
    # gra_spill_temp_252 = 160
    # gra_spill_temp_253 = 164
    # gra_spill_temp_254 = 168
    # gra_spill_temp_255 = 172
    # gra_spill_temp_256 = 176
    # gra_spill_temp_257 = 192
    # gra_spill_temp_258 = 208
    # gra_spill_temp_259 = 224
    # gra_spill_temp_260 = 240

 // registers:
 // a2: const int16_t *input_data
 // a3: const uint16_t input_wd
 // a4: const uint16_t input_ht
 // a5: const uint16_t channels
 // a6: const uint16_t pad_wd
 // a7: const uint16_t pad_ht

 // on stack:
 // const uint16_t stride_wd
 // const uint16_t stride_ht
 // const uint16_t ch_mult
 // const int16_t *filter_data
 // const uint16_t filter_wd
 // const uint16_t filter_ht
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
    s32i    a2,a1,136                   # [0]  gra_spill_temp_246
    s32i.n  a4,a1,40                # [1]  gra_spill_temp_222
    s32i    a5,a1,164                   # [2]  gra_spill_temp_253
    addi    a12,a1,112                  # [3]
    addmi   a10,a1,256                  # [4]
    addmi   a11,a1,256                  # [5]
    addmi   a13,a1,256                  # [6]
    l16ui   a8,a1,324                   # [7]  id:216 out_ht+0x0
    s32i.n  a8,a1,48                # [8]  gra_spill_temp_224
    addi    a13,a13,72                  # [9]
    addi    a11,a11,88                  # [10]
    addi    a10,a10,84                  # [11]
    ee.vldbc.32 q0,a10              # [12]  id:215 activation_min
    ee.vldbc.32 q1,a11              # [13]  id:214 activation_max
    ee.vldbc.32 q2,a13              # [14]  id:213 out_offset
    st.qr       q2,a12,80                   # [15]  gra_spill_temp_257-112
    st.qr       q1,a12,96                   # [16]  gra_spill_temp_258-112
    st.qr       q0,a12,112                  # [17]  gra_spill_temp_259-112
    beqz.n  a8,.Lt_10_8450          # [18]

    s32i    a1,a1,112                   # [0]  gra_spill_temp_240
    neg     a15,a6                      # [1]
    neg     a4,a7                       # [2]
    addmi   a8,a1,256                   # [3]
    movi.n  a9,0                    # [4]
    movi.n  a11,0                   # [5]
    slli    a14,a5,1                    # [6]
    l16ui   a13,a1,296                  # [7]  id:217 ch_mult+0x0
    l16ui   a10,a1,308                  # [8]  id:227 filter_ht+0x0
    s32i.n  a10,a1,36               # [9]  gra_spill_temp_221
    s32i    a13,a1,76                   # [10]  gra_spill_temp_231
    s32i    a14,a1,148                  # [11]  gra_spill_temp_249
    s32i.n  a11,a1,52               # [12]  gra_spill_temp_225
    s32i    a9,a1,116                   # [13]  gra_spill_temp_241
    st.qr   q4,a8,-16                   # [14]  gra_spill_temp_260-256
    sext    a4,a4,15                    # [15]
    sext    a15,a15,15                  # [16]
    s32i.n  a15,a1,32               # [17]  gra_spill_temp_220
    mul16u  a12,a5,a13              # [18]
    s32i    a4,a1,92                    # [19]  gra_spill_temp_235
    l16ui   a8,a1,320                   # [20]  id:229 out_wd+0x0
    l16ui   a9,a1,292                   # [21]  id:228 stride_ht+0x0
    l32i    a11,a1,336                  # [22]  id:226 out_mult+0x0
    s32i    a11,a1,64                   # [23]  gra_spill_temp_228
    s32i.n  a9,a1,44                # [24]  gra_spill_temp_223
    s32i    a8,a1,68                    # [25]  gra_spill_temp_229
    l32i    a4,a1,300                   # [26]  id:218 filter_data+0x0
    s32i    a12,a1,140                  # [27]  gra_spill_temp_247
    l32i    a15,a1,316                  # [28]  id:219 out_data+0x0
    s32i    a15,a1,96                   # [29]  gra_spill_temp_236
    slli    a12,a12,1                   # [30]
    s32i    a4,a1,152                   # [31]  gra_spill_temp_250
    addi    a14,a13,-3                  # [32]
    l16ui   a4,a1,304                   # [33]  id:223 filter_wd+0x0
    s32i    a14,a1,108                  # [34]  gra_spill_temp_239
    s32i    a12,a1,144                  # [35]  gra_spill_temp_248
    slli    a13,a13,2                   # [36]
    s32i    a13,a1,80                   # [37]  gra_spill_temp_232
    l32i    a12,a1,332                  # [38]  id:225 out_shift+0x0
    l32i    a14,a1,312                  # [39]  id:222 bias+0x0
    s32i    a14,a1,104                  # [40]  gra_spill_temp_238
    s32i.n  a12,a1,60               # [41]  gra_spill_temp_227
    l16ui   a13,a1,288                  # [42]  id:224 stride_wd+0x0
    s32i.n  a13,a1,56               # [43]  gra_spill_temp_226
    j   .Lt_10_8962                     # [44]

.Lt_10_9218:    # 0x1880
    l32i.n  a9,a1,48                # [0]  gra_spill_temp_224
    l32i.n  a11,a1,44               # [1]  gra_spill_temp_223
    l32i.n  a8,a1,52                # [2]  gra_spill_temp_225
    l32i    a10,a1,92                   # [3]  gra_spill_temp_235
    addi.n  a8,a8,1                 # [4]
    s32i.n  a8,a1,52                # [5]  gra_spill_temp_225
    add.n   a11,a10,a11                 # [6]
    sub     a8,a8,a9                    # [7]
    sext    a10,a11,15                  # [8]
    s32i    a10,a1,92                   # [9]  gra_spill_temp_235
    beqz    a8,.Lt_10_8450              # [10]

.Lt_10_8962:    # 0x189b
#<loop> Loop body line 1223, nesting depth: 1, estimated iterations: 100
 #1224          const int16_t base_y = (out_y * stride_ht) - pad_ht;
 #1225          for (int out_x = 0; out_x < out_wd; out_x++) { //width_loop
    l32i    a12,a1,68                   # [0]  gra_spill_temp_229
    beqz.n  a12,.Lt_10_9218         # [2]

.LBB6_esp_nn_depthwise_conv_s16_mult4:  # 0x18a0
    l32i.n  a7,a1,36                # [0]  gra_spill_temp_221
    movi.n  a11,0                   # [1]
    l32i.n  a8,a1,40                # [2]  gra_spill_temp_222
    l32i    a9,a1,92                    # [3]  gra_spill_temp_235
    movi.n  a13,0                   # [4]
    l32i.n  a14,a1,32               # [5]  gra_spill_temp_220
    s32i    a14,a1,160                  # [6]  gra_spill_temp_252
    s32i    a13,a1,72                   # [7]  gra_spill_temp_230
    neg     a10,a9                      # [8]
    sub     a8,a8,a9                    # [9]
    max     a10,a10,a11                 # [10]
    s32i    a10,a1,100                  # [11]  gra_spill_temp_237
    min     a7,a7,a8                    # [12]
    j   .Lt_10_9730                     # [13]

.Lt_10_9986:    # 0x18c5
    l32i    a13,a1,68                   # [0]  gra_spill_temp_229
    l32i.n  a15,a1,56               # [1]  gra_spill_temp_226
    l32i    a12,a1,72                   # [2]  gra_spill_temp_230
    l32i    a14,a1,160                  # [3]  gra_spill_temp_252
    addi.n  a12,a12,1               # [4]
    s32i    a12,a1,72                   # [5]  gra_spill_temp_230
    add.n   a15,a14,a15                 # [6]
    sext    a14,a15,15                  # [7]
    s32i    a14,a1,160                  # [8]  gra_spill_temp_252
    beq a12,a13,.Lt_10_9218         # [9]

.Lt_10_9730:    # 0x18e0
    l32i    a8,a1,164                   # [0]  gra_spill_temp_253
    l32i    a9,a1,64                    # [1]  gra_spill_temp_228
    l32i.n  a10,a1,60               # [2]  gra_spill_temp_227
    s32i    a10,a1,132                  # [3]  gra_spill_temp_245
    s32i    a9,a1,128                   # [4]  gra_spill_temp_244
    beqz.n  a8,.Lt_10_9986          # [5]

    movi.n  a8,0                    # [0]
    l32i    a5,a1,160                   # [1]  gra_spill_temp_252
    movi.n  a12,0                   # [2]
    movi.n  a13,0                   # [3]
    movi.n  a14,0                   # [4]
    s32i    a14,a1,84                   # [5]  gra_spill_temp_233
    s32i    a13,a1,88                   # [6]  gra_spill_temp_234
    s32i    a12,a1,176                  # [7]  gra_spill_temp_256
    neg     a6,a5                       # [8]
    max     a6,a6,a8                    # [9]
    sub     a5,a3,a5                    # [10]
    min     a5,a4,a5                    # [11]
    sub     a11,a5,a6                   # [12]
    s32i    a11,a1,156                  # [13]  gra_spill_temp_251
    j   .Lt_10_10498                    # [14]

.Lt_10_10754:   # 0x1919
    l32i    a10,a1,164                  # [0]  gra_spill_temp_253
    l32i    a14,a1,76                   # [1]  gra_spill_temp_231
    l32i    a13,a1,84                   # [2]  gra_spill_temp_233
    l32i    a12,a1,80                   # [3]  gra_spill_temp_232
    l32i    a9,a1,176                   # [4]  gra_spill_temp_256
    l32i    a11,a1,88                   # [5]  gra_spill_temp_234
    addi.n  a9,a9,1                 # [6]
    s32i    a9,a1,176                   # [7]  gra_spill_temp_256
    add.n   a11,a11,a12                 # [8]
    add.n   a13,a13,a14                 # [9]
    s32i    a13,a1,84                   # [10]  gra_spill_temp_233
    s32i    a11,a1,88                   # [11]  gra_spill_temp_234
    beq     a9,a10,.Lt_10_9986          # [12]

.Lt_10_10498:   # 0x193d
    l32i    a15,a1,108                  # [0]  gra_spill_temp_239
    blti    a15,1,.Lt_10_10754          # [2]

    l32i    a2,a1,84                    # [0]  gra_spill_temp_233
    l32i    a10,a1,104                  # [1]  gra_spill_temp_238
    l32i    a9,a1,88                    # [2]  gra_spill_temp_234
    movi.n  a8,0                    # [3]
    s32i    a8,a1,120                   # [4]  gra_spill_temp_242
    add.n   a9,a9,a10                   # [5]
    s32i    a9,a1,124                   # [6]  gra_spill_temp_243
    j   .Lt_10_11266                    # [7]

.Lt_10_11522:   # 0x1959
    addmi   a12,a1,256                  # [0]
    l32i    a14,a1,112                  # [1]  gra_spill_temp_240
    movi.n  a13,16                  # [2]
    ee.st.qacc_l.l.128.ip   a14,16      # [3]  id:234
    ee.st.qacc_l.h.32.ip    a14,-16     # [4]  id:235
    ee.srcmb.s16.qacc   q5,a13,0        # [5]
    l16ui   a15,a1,10                   # [6]  qacc_scratch+10
    l8ui    a8,a1,15                    # [7]  qacc_scratch+15
    l8ui    a9,a1,5                     # [8]  qacc_scratch+5
    l8ui    a11,a1,16                   # [9]  qacc_scratch+16
    l8ui    a10,a1,6                    # [10]  qacc_scratch+6
    s8i     a10,a1,3                    # [11]  qacc_scratch+3
    s8i     a11,a1,7                    # [12]  qacc_scratch+7
    s8i     a9,a1,2                     # [13]  qacc_scratch+2

    l32i    a11,a1,104                  # [14]  gra_spill_temp_238
    s8i     a8,a1,6                     # [15]  qacc_scratch+6
    s16i    a15,a1,4                    # [16]  qacc_scratch+4
    ee.vld.l.64.ip  q0,a14,0        # [17]  id:245
    s32i    a14,a1,112                  # [18]  gra_spill_temp_240
    ee.vzip.16  q0,q5               # [19]
    st.qr   q5,a12,-16                  # [20]  gra_spill_temp_260-256

    beqz.n  a11,.Lt_10_13570        # [21] // skip_bias

 // add bias
    l32i    a13,a1,124                  # [0]  gra_spill_temp_243
    extui   a12,a13,0,4                 # [2]
    ee.vld.128.ip   q7,a13,16           # [3]  id:248
    ee.vld.128.ip   q1,a13,0            # [4]  id:249
    wur.sar_byte    a12                 # [5]
    ee.src.q.qup    q6,q7,q1            # [6]
    ee.vadds.s32    q0,q0,q6            # [7]

.Lt_10_13570:   # 0x19ae
 #1287                      q0 = esp_nn_multiply_by_quantized_mult_ver1_esp32s3(q0, out_mult_ptr, out_shift_ptr);
    l32i    a10,a1,128                  # [0]  gra_spill_temp_244
    l32i    a11,a1,132                  # [1]  gra_spill_temp_245
    call8   esp_nn_multiply_by_quantized_mult_ver1_esp32s3     # [2]  esp_nn_multiply_by_quantized_mult_ver1_esp32s3

    addi.n  a2,a2,4                 # [0]
    l32i    a13,a1,96                   # [1]  gra_spill_temp_236
    l32i    a11,a1,128                  # [2]  gra_spill_temp_244
    l32i    a10,a1,132                  # [3]  gra_spill_temp_245
    addi    a8,a1,112                   # [4]
    ld.qr   q1,a8,96                    # [5]  gra_spill_temp_258-112
    ld.qr   q2,a8,80                    # [6]  gra_spill_temp_257-112
    addi    a10,a10,16                  # [7]
    addi    a11,a11,16                  # [8]
    s32i    a11,a1,128                  # [9]  gra_spill_temp_244
    ee.vadds.s32    q0,q0,q2            # [10]
    s32i    a10,a1,132                  # [11]  gra_spill_temp_245
    ee.vmin.s32 	q0,q0,q1            # [12]
    ld.qr   		q1,a8,112                   # [13]  gra_spill_temp_259-112
    l32i    		a8,a1,116                   # [14]  gra_spill_temp_241
    ee.vmax.s32 	q0,q0,q1            # [15]
    ee.movi.32.a    q0,a14,2            # [16]
    ee.movi.32.a    q0,a15,1            # [17]
    ee.movi.32.a    q0,a9,0             # [18]
    add.n   		a13,a8,a13                  # [19]
    ee.movi.32.a    q0,a12,3            # [20]
    addi.n  a8,a8,4                 # [21]
    s8i 	a12,a13,3                   # [22]  id:254
    s32i    a8,a1,116                   # [23]  gra_spill_temp_241
    s8i 	a9,a13,0                    # [24]  id:251
    s8i 	a15,a13,1                   # [25]  id:252
    s8i 	a14,a13,2                   # [26]  id:253
    l32i    a15,a1,108                  # [27]  gra_spill_temp_239
    l32i    a14,a1,120                  # [28]  gra_spill_temp_242
    l32i    a9,a1,124                   # [29]  gra_spill_temp_243
    addi.n  a14,a14,4               # [30]
    addi    a9,a9,16                    # [31]
    s32i    a9,a1,124                   # [32]  gra_spill_temp_243
    s32i    a14,a1,120                  # [33]  gra_spill_temp_242
    bge a14,a15,.Lt_10_10754        # [34]

.Lt_10_11266:   # 0x1a1c
#<loop> Loop body line 1230, nesting depth: 4, estimated iterations: 100
    ee.zero.qacc                    # [0]
    l32i    a9,a1,100                   # [1]  gra_spill_temp_237
    s32i    a9,a1,172                   # [2]  gra_spill_temp_255
    bge     a9,a7,.Lt_10_11522          # [3]

    mull    a15,a9,a4                   # [0]
    l32i    a14,a1,92                   # [1]  gra_spill_temp_235
    add.n   a11,a15,a5                  # [2]
    add.n   a14,a14,a9                  # [3]
    mull    a14,a3,a14                  # [4]
    s32i    a11,a1,168                  # [5]  gra_spill_temp_254
    bge     a6,a5,.Lt_10_12290          # [6]

.LBB18_esp_nn_depthwise_conv_s16_mult4: # 0x1a3b
    l32i    a10,a1,176                  # [0]  gra_spill_temp_256
    l32i    a11,a1,164                  # [1]  gra_spill_temp_253
    l32i    a12,a1,160                  # [2]  gra_spill_temp_252
    add.n   a9,a15,a6                   # [3]
    l32i    a8,a1,140                   # [4]  gra_spill_temp_247
    addmi   a13,a1,256                  # [5]
    ld.qr   q1,a13,-16                  # [6]  gra_spill_temp_260-256
    mull    a8,a8,a9                    # [7]
    add.n   a12,a12,a6                  # [8]
    l32i    a9,a1,152                   # [9]  gra_spill_temp_250
    add.n   a12,a14,a12                 # [10]
    mull    a11,a11,a12                 # [11]
    add.n   a8,a2,a8                    # [12]
    l32i    a12,a1,148                  # [13]  gra_spill_temp_249
    addx2   a8,a8,a9                    # [14]
    add.n   a10,a10,a11                 # [15]
    l32i    a11,a1,136                  # [16]  gra_spill_temp_246
    l32i    a9,a1,156                   # [17]  gra_spill_temp_251
    addx2   a10,a10,a11                 # [18]
    l32i    a11,a1,144                  # [19]  gra_spill_temp_248
    loopgtz a9,.LBB45_esp_nn_depthwise_conv_s16_mult4   # [20]

    mov.n   a9,a8                       # [0*II+0]
    ee.vldbc.16 q0,a10              # [0*II+1]  id:232
    add.n   a10,a10,a12                 # [0*II+2]
    ee.vld.l.64.ip  q1,a9,0         # [0*II+3]  id:231
    add.n   a8,a8,a11                   # [0*II+4]
    ee.vmulas.s16.qacc  q0,q1       # [0*II+5]
.LBB45_esp_nn_depthwise_conv_s16_mult4: # 0x1a84

    addmi   a10,a1,256                  # [0]
    st.qr   q1,a10,-16                  # [1]  gra_spill_temp_260-256

.Lt_10_12290:   # 0x1a8a
    add.n   a14,a14,a3                  # [0]
    add.n   a15,a15,a4                  # [1]
    l32i    a11,a1,172                  # [2]  gra_spill_temp_255
    l32i    a12,a1,168                  # [3]  gra_spill_temp_254
    addi.n  a11,a11,1               # [4]
    add.n   a12,a12,a4                  # [5]
    s32i    a12,a1,168                  # [6]  gra_spill_temp_254
    s32i    a11,a1,172                  # [7]  gra_spill_temp_255
    sub     a11,a7,a11                  # [8]
    beqz    a11,.Lt_10_11522            # [9]

    blt     a6,a5,.LBB18_esp_nn_depthwise_conv_s16_mult4    # [0]

    j   .Lt_10_12290                    # [0]

.Lt_10_8450:    # 0x1aaa
    retw.n                          # [0]

    .size   esp_nn_depthwise_conv_s16_mult4_esp32s3, . - esp_nn_depthwise_conv_s16_mult4_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
