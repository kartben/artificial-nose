#include "edge-impulse-sdk/classifier/ei_classifier_config.h"
#if EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
// Copyright 2021-2023 Espressif Systems (Shanghai) PTE LTD
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
    .literal    .LC0_26_123, 1073741824 // `1 << 30`

    # Program Unit: esp_nn_mul_elementwise_s8_esp32s3
    .type   esp_nn_mul_elementwise_s8_esp32s3, @function
    .align   4
    .global esp_nn_mul_elementwise_s8_esp32s3

esp_nn_mul_elementwise_s8_esp32s3:  # 0x4
    # to_add = 0
    # gra_spill_temp_0 = 4
    # gra_spill_temp_1 = 8
    # gra_spill_temp_2 = 12
    # gra_spill_temp_3 = 16
    # gra_spill_temp_4 = 20
    # gra_spill_temp_5 = 24
    # gra_spill_temp_6 = 28
    # gra_spill_temp_7 = 32
    # gra_spill_temp_8 = 36
    # gra_spill_temp_<> = 40
    # gra_spill_temp_<> = 44
    # gra_spill_temp_<> = 48
    # gra_spill_temp_13 = 64

 // registers:
 // a2: const int8_t *input1_data
 // a3: const int8_t *input2_data
 // a4: const int32_t input1_offset
 // a5: const int32_t input2_offset
 // a6: int8_t *output
 // a7: const int32_t out_offset

 // on stack:
 // 120: const int32_t out_mult
 // 124: const int32_t out_shift
 // 128: const int32_t activation_min
 // 132: const int32_t activation_max
 // 136: const int32_t size

    entry   a1,120                      #
    s32i.n  a4,a1,24                # [0]  gra_spill_temp_5, input1_offset
    s32i.n  a5,a1,28                # [1]  gra_spill_temp_12, input2_offset

    s32i.n  a3,a1,4                     # [5]  gra_spill_temp_0, input2
    mov.n   a10,a3                      # [6]
    l32i    a3,a1,136                   # [18]  id:361 size+0x0
    mov.n   a9,a6                       # [2] // out_addr
    blti    a3,1,.exit            # [0] // exit

    s32i.n  a2,a1,16                    # [9]  gra_spill_temp_3, input1
    s32i    a7,a1,40                    # [4]  id:358 out_offset+0x0
    movi.n  a11,0                       # [3]
    mov.n   a12,a2                      # [10]
    s32i    a4,a1,44                # [13]  id:356 input1_offset+0x0
    s32i    a5,a1,48                # [14]  id:357 input2_offset+0x0
    movi.n  a2,1                    # [15]

    l32i    a15,a1,124                  # [3]  id:362 out_shift+0x0
    l32i    a13,a1,120                  # [4]  id:363 out_mult+0x0
    s32i.n  a6,a1,8                 # [1]  gra_spill_temp_1, out_addr
    max     a14,a15,a11                 # [11] left_shift
    sub     a4,a14,a15              # right_shift
    s32i.n  a4,a1,20                # [9]  gra_spill_temp_4

    blti    a3,8,.process_leftover             # [20]

    // skip to leftover routine if inputs are unaligned
    or          a6,a12,a10
    extui       a6,a6,0,4
    bnez        a6,.process_leftover

    // `size > 8`, s3 optimisation path...
    ee.zero.q   q1                      # [0]
    addi    a4,a1,44                # [7]
    addi    a8,a1,48                    # [8]
    ee.vldbc.16 q0,a4               # [17]  id:359 input1_offset
    ee.vldbc.16 q7,a8               # [16]  id:360 input2_offset
    l32r    a4,.LC0_26_123              # [12]
    movi    a8, 8
    st.qr   q0,a1,64                    # [19]  gra_spill_temp_13
    s32i.n  a8,a1,12                # [6]  gra_spill_temp_2

.Lt_0_7682: # 0x60
    s32i            a9,a1,36                    # [1]  gra_spill_temp_8, out_addr
    ld.qr           q4,a1,64                    # [2]  gra_spill_temp_13, input1_offset
    ee.vld.l.64.ip  q2,a12,8        # [4]  id:367, input1_ptr
    movi.n          a7,16                   # [3]
    ee.vld.h.64.ip  q2,a10,8        # [5]  id:368, input2_ptr
    wsr.sar         a7                      # [6]
    ee.vcmp.lt.s8   q5,q2,q1            # [7]
    ee.vzip.8       q2,q5               # [8]
    ee.vadds.s16    q5,q5,q7            # [9] input2_offset
    ee.vadds.s16    q4,q2,q4            # [10] input1_offset
    ee.vmul.s16     q3,q4,q5            # [11]
    wsr.sar         a11                         # [12]
    ee.vmul.s16     q2,q4,q5            # [13]

    wsr.sar         a14                     # [14] left_shift
    ee.vzip.16      q2,q3               # [15]
    ee.vsl.32       q6,q2                   # [16] left_shift
    ssai            31                          # [17]

    ee.movi.32.a    q6,a3,2             # [18]
    ee.movi.32.a    q6,a8,3             # [26]

    mulsh           a6,a13,a3                   # [19]
    mull            a3,a13,a3                   # [20]
    mulsh           a7,a13,a8                   # [27]
    add.n           a3,a4,a3                    # [22]
    saltu           a2,a3,a4                    # [23]
    add.n           a2,a2,a6                    # [24]
    src             a2,a2,a3                    # [25]

    mull            a6,a13,a8                   # [28]
    add.n           a6,a4,a6                    # [30]
    saltu           a9,a6,a4                    # [31]
    add.n           a9,a9,a7                    # [32]
    src             a9,a9,a6                    # [33]
    ee.movi.32.q    q2,a2,2             # [53]
    ee.movi.32.q    q2,a9,3             # [54]

    ee.movi.32.a    q6,a6,1             # [34]
    mulsh           a7,a13,a6                   # [35]
    mull            a6,a13,a6                   # [36]
    add.n           a6,a4,a6                    # [38]
    saltu           a3,a6,a4                    # [39]
    add.n           a3,a3,a7                    # [16]
    src             a3,a3,a6                    # [41]
    ee.movi.32.a    q6,a2,0             # [42]
    mulsh           a8,a13,a2                   # [43]
    mull            a7,a13,a2                   # [4]
    add.n           a7,a4,a7                    # [46]
    saltu           a6,a7,a4                    # [47]
    add.n           a6,a6,a8                    # [24]
    src             a6,a6,a7                    # [49]
    ee.movi.32.q    q2,a3,1             # [28]
    ee.movi.32.q    q2,a6,0             # [50]

    wsr.sar         a14                     # [10]
    ee.vsl.32       q4,q3                   # [11]
    ee.movi.32.a    q4,a2,2             # [13]
    mulsh           a3,a13,a2                   # [14]
    mull            a2,a13,a2                   # [15]
    ssai            31                          # [12]
    add.n           a2,a4,a2                    # [17]
    saltu           a5,a2,a4                # [18]
    add.n           a5,a5,a3                # [19]
    src             a5,a5,a2                    # [20]
    ee.movi.32.a    q4,a3,3             # [21]
    mulsh           a6,a13,a3                   # [22]
    mull            a3,a13,a3                   # [23]
    add.n           a3,a4,a3                    # [25]
    saltu           a8,a3,a4                    # [26]
    add.n           a8,a8,a6                    # [27]
    src             a8,a8,a3                    # [28]
    ee.movi.32.q    q0,a5,2             # [24]
    ee.movi.32.q    q0,a8,3             # [51]

    ee.movi.32.a    q4,a7,1             # [29]
    mulsh           a6,a13,a7                   # [30]
    mull            a3,a13,a7                   # [31]
    add.n           a3,a4,a3                    # [33]
    saltu           a2,a3,a4                    # [34]
    add.n           a2,a2,a6                    # [35]
    src             a2,a2,a3                    # [36]
    ee.movi.32.a    q4,a6,0             # [37]
    mulsh           a7,a13,a6                   # [38]
    mull            a6,a13,a6                   # [39]
    add.n           a6,a4,a6                    # [41]
    saltu           a3,a6,a4                    # [42]
    add.n           a3,a3,a7                    # [43]
    src             a3,a3,a6                    # [4]
    ee.movi.32.q    q0,a2,1             # [47]
    ee.movi.32.q    q0,a3,0             # [46]

    l32i.n          a5,a1,20                # [0]  gra_spill_temp_4, right_shift
    movi.n          a7,1                    # [51]

    blti            a5,1,.skip_div_by_pow_of_2
// divide by power of 2
    ee.vcmp.lt.s32  q5,q2,q1        # [56]
    ee.vcmp.lt.s32  q6,q0,q1        # [28]

    addi.n          a8,a5,-1                # [1]
    ssl             a8                          # [2]
    sll             a7,a7                       # [3]
    s32i.n          a7,a1,0                 # [4]  to_add
    ee.vldbc.32     q4,a1               # [5]  id:376 to_add

    wsr.sar         a5                      # [6]
    ee.vadds.s32    q5,q4,q5            # [7]
    ee.vadds.s32    q5,q2,q5            # [8]
    ee.vsr.32       q2,q5                   # [9]

    wsr.sar         a5                      # [5]
    ee.vadds.s32    q5,q4,q6            # [9]
    ee.vadds.s32    q5,q0,q5            # [11]
    ee.vsr.32       q0,q5                   # [12]
.skip_div_by_pow_of_2:

// add offset, apply activation
    addi            a8,a1,132                   # [54]
    ee.vldbc.32     q4,a8               # [55]  id:385 activation_max
    addi            a5,a1,40                    # [8]
    ee.vldbc.32     q6,a5               # [10]  id:384 out_offset
    addi            a7,a1,128                   # [4]
    ee.vadds.s32    q0,q0,q6            # [13] // add out_offset
    ee.vadds.s32    q2,q2,q6            # [14] // add out_offset
    ee.vldbc.32     q6,a7               # [16]  id:386 activation_min
    ee.vmin.s32     q0,q0,q4            # [17]
    ee.vmin.s32     q2,q2,q4            # [15]
    ee.vmax.s32     q0,q0,q6            # [18]
    ee.vmax.s32     q2,q2,q6            # [19]

// pack and store
    ee.vunzip.16    q2,q0               # [20]
    ee.vunzip.8     q2,q0               # [21]
    l32i.n          a7,a1,12 // count
    l32i            a9,a1,36                    # [55]  gra_spill_temp_8
    l32i.n          a3,a1,136               # [1] , size
    ee.vst.l.64.ip  q2,a9,8         # [22]  id:387
    addi            a7,a7,8
    s32i.n          a7,a1,12 // increment count
    bge             a3,a7,.Lt_0_7682

    addi            a11,a7,-8
    bge             a11,a3,.exit  # [3] // exit

.process_leftover:
    sub     a8,a3,a11                   # [1]
    loopgtz a8,.LBB33_esp_nn_mul_elementwise_s8_esp32s3     # [9]

    ssl     a14                         # [0] left_shift
    l32i.n  a8,a1,24                # [1]  gra_spill_temp_5, input1_offset
    l32i.n  a10,a1,4                # [2]  gra_spill_temp_0, input2
    l32i.n  a12,a1,16               # [3]  gra_spill_temp_3, input1
    add.n   a10,a11,a10                 # [4], input2
    add.n   a12,a11,a12                 # [5], input1
    l8ui    a12,a12,0                   # [6]  id:390
    l8ui    a10,a10,0                   # [7]  id:391
    sext    a12,a12,7                   # [8]
    add.n   a12,a12,a8                  # [9]
    l32i.n  a8,a1,28                # [10]  gra_spill_temp_12, input2_offset
    sext    a10,a10,7                   # [11]
    add.n   a10,a10,a8                  # [12]
    mull    a10,a12,a10                 # [13] // multiplication result

// multiply by quantised mult
    l32i.n  a9,a1,20                # [0]  gra_spill_temp_4, load right_shift

    sll     a10,a10                     # [15] // left shift

    mulsh   a3,a10,a13                  # [1]
    mull    a8,a10,a13                  # [6]
    ssai    31                          # [0]
    add.n   a6,a8,a4                    # [8]
    saltu   a8,a6,a8                    # [9]
    add.n   a8,a8,a3                    # [10]
    src     a3,a8,a6                    # [19] // result

    blti    a9, 1, .skip_div_by_pow_of_2_remains
// divide by power of 2
    // calculate to_add = `1 << (exponent - 1)`
    addi    a6,a9,-1
    ssl     a6                          # [23]
    movi    a7,1
    sll     a7,a7                       // to_add

    extui   a8,a3,31,1                  # [24], sign
    add     a3,a3,a8            // add sign
    add     a3,a3,a7            // add to_add

    ssr     a9                          # [20] load right_shift
    sra     a3,a3               // right shift
.skip_div_by_pow_of_2_remains:

    l32i.n  a6,a1,40                    # [32], out_offset
    l32i.n  a8,a1,132                   # [35], act_max
    l32i.n  a7,a1,128                   # [36], act_min

// add offset and apply activation
    add.n   a3,a3,a6                    # [34], offset added
    min     a8,a8,a3                    # [37]
    l32i.n  a3,a1,8                 # [38]  gra_spill_temp_1, load base out_addr
    max     a8,a8,a7                    # [39]

// store
    add.n   a3,a11,a3                   # [16], add index from `a11`
    s8i     a8,a3,0                     # [41]  id:392 // store
    addi.n  a11,a11,1               # [42]  // inc index

.LBB33_esp_nn_mul_elementwise_s8_esp32s3:   # 0x2ed
.exit:
    retw.n                          # [0]

    .size   esp_nn_mul_elementwise_s8_esp32s3, . - esp_nn_mul_elementwise_s8_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
