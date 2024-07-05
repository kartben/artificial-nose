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
    .literal    .nudge_val, 1073741824

    # Program Unit: esp_nn_add_elementwise_s8_esp32s3
    .type   esp_nn_add_elementwise_s8_esp32s3, @function
    .align   4
    .global esp_nn_add_elementwise_s8_esp32s3

esp_nn_add_elementwise_s8_esp32s3:  # 0x4
    # temp_neg_out_shift = 0
    # temp_neg_input2_shift = 4
    # temp_neg_input1_shift = 8
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

 // a2 : *input1_data
 // a3 : *input2_data
 // a4 : input1_offset
 // a5 : input2_offset
 // a6 : input1_mult
 // a7 : input2_mult
 // On stack:
 // 80: input1_shift
 // 84: input2_shift
 // 88: left_shift
 // 92: *output
 // 96: out_offset
 // 100: out_mult, loaded in `a8`
 // 104: out_shift
 // 108: activation_min
 // 112: activation_max
 // 116: size

    entry       a1,80                      #
    s32i.n      a4,a1,48                    # [10]  gra_spill_temp_11, input1_offset
    s32i.n      a5,a1,52                    # [0]  gra_spill_temp_12, input2_offset
    s32i.n      a2,a1,32                 # [5]  gra_spill_temp_7, input1_data
    s32i.n      a3,a1,12                    # [3]  gra_spill_temp_2, input2_data

    l32i        a12,a1,116                  # [11]  id:720 size+0x0
    mov.n       a14,a2                      # [6]
    mov.n       a10,a3                      # [8]
    blti        a12,1,.exit           # [1] // exit

    l32i        a3,a1,80                   # [0]  id:721 input1_shift+0x0
    l32i        a13,a1,84                  # [1]  id:722 input2_shift+0x0
    l32i        a2,a1,104                   # [8]  id:723 out_shift+0x0
    l32i        a8,a1,100                   # [1]  out_mult

    neg         a3,a3                       # [12]
    neg         a13,a13                     # [7]
    neg         a2,a2                       # [11]

    s32i.n      a3,a1,8                    # [12]  temp_neg_input1_shift, -input1_shift
    s32i.n      a13,a1,4                   # [7]  temp_neg_input2_shift, -input2_shift
    s32i.n      a2,a1,0                    # [16]  temp_neg_out_shift, -out_shift

    movi.n      a5,1
    addi        a9,a3,-1
    ssl         a9
    sll         a15,a5
    s32i.n      a15,a1,16               # gra_spill_temp_3, 1 << (exponent - 1) for input1

    addi        a9,a13,-1
    ssl         a9
    sll         a15,a5
    s32i.n      a15,a1,20               # gra_spill_temp_4, 1 << (exponent - 1) for input2

    addi        a9,a2,-1
    ssl         a9
    sll         a15,a5
    s32i.n      a15,a1,24               # gra_spill_temp_5, 1 << (exponent - 1) for out

    movi.n      a2,0
    blti        a12,12,.process_leftover          # [23]

    // skip to leftover routine if inputs are unaligned
    or          a9,a14,a10
    extui       a9,a9,0,4
    bnez        a9,.process_leftover

    l32i        a9,a1,92                   # [17]  id:1279 output+0x0

    l32i        a13,a1,116                  # [20]
    srai        a13,a13,3                   # [21]
    s32i.n      a13,a1,56                   # [22]  gra_spill_temp_13

    movi.n      a13,8
    s32i.n      a13,a1,28               # gra_spill_temp_6, mult_of8 counter

    ee.zero.q       q6                      # [8]

.vector_loop: // process 8 values in one go
    l32i            a15,a1,88                  # [6]  left_shift
    ee.vld.l.64.ip  q0,a14,8        # [9]  id:729
    s32i.n          a9,a1,44                    # [10]  gra_spill_temp_10, out_ptr
    s32i.n          a14,a1,40                   # [20]  gra_spill_temp_9
    wsr.sar         a15                     # [21] load left shift

    addi.n          a15,a1,48                   # [14]
    ee.vldbc.16     q7,a15              # [21]  id:1277 input1_offset
    ee.vcmp.lt.s8   q5,q0,q6            # [29]
    ee.vzip.8       q0,q5                   # [31], 20 bits
    ee.vadds.s16    q0,q0,q7            # [34], add offset
    ee.vcmp.lt.s16  q2,q0,q6        # [36]
    ee.vzip.16      q0,q2               # [39], 32 bits
    ee.vsl.32       q0,q0                   # [41] left_shift
    ee.vsl.32       q2,q2                   # [42] left_shift

    l32r            a9,.nudge_val              # [15], nudge

// mulhi32 for q0
    ee.movi.32.a    q0,a3,2             # [44]
    ee.movi.32.a    q0,a4,3             # [45]
    ee.movi.32.a    q0,a14,1            # [46]
    ee.movi.32.a    q0,a5,0             # [62]

    mulsh           a13,a6,a3                   # [51]
    mull            a3,a6,a3                    # [53]

    mulsh           a12,a6,a4                   # [50]
    mull            a4,a6,a4                    # [55]

    mulsh           a15,a6,a14                  # [48]
    mull            a14,a6,a14                  # [49]

    ssai            31                          # [47]

    add             a3,a3,a9
    saltu           a2,a3,a9
    add.n           a13,a13,a2
    src             a13,a13,a3

    add             a4,a4,a9
    saltu           a2,a4,a9
    add.n           a12,a12,a2
    src             a12,a12,a4
    ee.movi.32.q    q0,a13,2            # [62]

    add             a14,a14,a9
    saltu           a2,a14,a9
    add.n           a15,a15,a2
    src             a15,a15,a14
    ee.movi.32.q    q0,a12,3            # [62]

    mulsh           a13,a6,a5                   # [51]
    mull            a5,a6,a5                    # [53]
    ee.movi.32.q    q0,a15,1            # [62]

    add             a5,a5,a9
    saltu           a2,a5,a9
    add.n           a13,a13,a2
    src             a13,a13,a5
    ee.movi.32.q    q0,a13,0            # [62]


// mulhi32 for q2
    ee.movi.32.a    q2,a3,2             # [44]
    ee.movi.32.a    q2,a4,3             # [45]
    ee.movi.32.a    q2,a14,1            # [46]
    ee.movi.32.a    q2,a5,0             # [62]

    mulsh           a13,a6,a3                   # [51]
    mull            a3,a6,a3                    # [53]

    mulsh           a12,a6,a4                   # [50]
    mull            a4,a6,a4                    # [55]

    mulsh           a15,a6,a14                  # [48]
    mull            a14,a6,a14                  # [49]

    ssai            31                          # [47]

    add             a3,a3,a9
    saltu           a2,a3,a9
    add.n           a13,a13,a2
    src             a13,a13,a3

    add             a4,a4,a9
    saltu           a2,a4,a9
    add.n           a12,a12,a2
    src             a12,a12,a4
    ee.movi.32.q    q2,a13,2            # [62]

    add             a14,a14,a9
    saltu           a2,a14,a9
    add.n           a15,a15,a2
    src             a15,a15,a14
    ee.movi.32.q    q2,a12,3            # [62]

    mulsh           a13,a6,a5                   # [51]
    mull            a5,a6,a5                    # [53]
    ee.movi.32.q    q2,a15,1            # [62]

    l32i            a3,a1,8                    # [12]  temp_neg_input1_shift, -input1_shift
    add             a5,a5,a9
    saltu           a2,a5,a9
    add.n           a13,a13,a2
    src             a13,a13,a5
    ee.movi.32.q    q2,a13,0            # [62]


    blti            a3,1, .skip_div_by2_in0

    addi.n          a13,a1,16
    ee.vcmp.lt.s32  q1,q0,q6
    ee.vcmp.lt.s32  q3,q2,q6
    ee.vldbc.32     q5,a13      // 1 << (exponent - 1)
    wsr.sar         a3          // load right_shift
    ee.vadds.s32    q0,q0,q1    // subtract 1 `if (val < 0)`
    ee.vadds.s32    q2,q2,q3    // subtract 1 `if (val < 0)`
    ee.vadds.s32    q0,q0,q5
    ee.vadds.s32    q2,q2,q5
    ee.vsr.32       q0,q0
    ee.vsr.32       q2,q2

.skip_div_by2_in0:


    ee.vld.l.64.ip  q1,a10,8        # [11]  id:1290
    addi.n          a15,a1,52                   # [12]
    ee.vldbc.16     q7,a15              # [19]  id:1278 input2_offset
    l32i            a15,a1,88                  # [6]  left_shift
    s32i            a10,a1,36                   # [14]  gra_spill_temp_8
    ee.vcmp.lt.s8   q3,q1,q6            # [271]
    wsr.sar         a15                     # [21], load shift for left shift
    ee.vzip.8       q1,q3                   # [274], 20 bits
    ee.vadds.s16    q1,q1,q7            # [281]
    ee.vcmp.lt.s16  q3,q1,q6        # [282]
    ee.vzip.16      q1,q3               # [283], 32 bits
    ee.vsl.32       q1,q1                   # [284]
    ee.vsl.32       q3,q3                   # [285]


// mulhi32 for q1
    ee.movi.32.a    q1,a3,2             # [44]
    ee.movi.32.a    q1,a4,3             # [45]
    ee.movi.32.a    q1,a14,1            # [46]
    ee.movi.32.a    q1,a5,0             # [62]

    mulsh           a13,a7,a3                   # [51]
    mull            a3,a7,a3                    # [53]

    mulsh           a12,a7,a4                   # [50]
    mull            a4,a7,a4                    # [55]

    mulsh           a15,a7,a14                  # [48]
    mull            a14,a7,a14                  # [49]

    ssai            31                          # [47]

    add             a3,a3,a9
    saltu           a2,a3,a9
    add.n           a13,a13,a2
    src             a13,a13,a3

    add             a4,a4,a9
    saltu           a2,a4,a9
    add.n           a12,a12,a2
    src             a12,a12,a4
    ee.movi.32.q    q1,a13,2            # [62]

    add             a14,a14,a9
    saltu           a2,a14,a9
    add.n           a15,a15,a2
    src             a15,a15,a14
    ee.movi.32.q    q1,a12,3            # [62]

    mulsh           a13,a7,a5                   # [51]
    mull            a5,a7,a5                    # [53]
    ee.movi.32.q    q1,a15,1            # [62]

    add             a5,a5,a9
    saltu           a2,a5,a9
    add.n           a13,a13,a2
    src             a13,a13,a5
    ee.movi.32.q    q1,a13,0            # [62]


// mulhi32 for q3
    ee.movi.32.a    q3,a3,2             # [44]
    ee.movi.32.a    q3,a4,3             # [45]
    ee.movi.32.a    q3,a14,1            # [46]
    ee.movi.32.a    q3,a5,0             # [62]

    mulsh           a13,a7,a3                   # [51]
    mull            a3,a7,a3                    # [53]

    mulsh           a12,a7,a4                   # [50]
    mull            a4,a7,a4                    # [55]

    mulsh           a15,a7,a14                  # [48]
    mull            a14,a7,a14                  # [49]

    ssai            31                          # [47]

    add             a3,a3,a9
    saltu           a2,a3,a9
    add.n           a13,a13,a2
    src             a13,a13,a3

    add             a4,a4,a9
    saltu           a2,a4,a9
    add.n           a12,a12,a2
    src             a12,a12,a4
    ee.movi.32.q    q3,a13,2            # [62]

    add             a14,a14,a9
    saltu           a2,a14,a9
    add.n           a15,a15,a2
    src             a15,a15,a14
    ee.movi.32.q    q3,a12,3            # [62]

    mulsh           a13,a7,a5                   # [51]
    mull            a5,a7,a5                    # [53]
    ee.movi.32.q    q3,a15,1            # [62]
    l32i            a14,a1,4                   # [7]  temp_neg_input2_shift, -input2_shift

    add             a5,a5,a9
    saltu           a2,a5,a9
    add.n           a13,a13,a2
    src             a13,a13,a5
    ee.movi.32.q    q3,a13,0            # [62]

    // multiplication results: q0-q2 & q1-q3


    blti            a14,1, .skip_div_by2_in1

    addi.n          a5,a1,20
    ee.vcmp.lt.s32  q4,q1,q6
    ee.vcmp.lt.s32  q5,q3,q6
    ee.vldbc.32     q7,a5       // 1 << (exponent - 1)
    wsr.sar         a14         // load right_shift
    ee.vadds.s32    q4,q4,q7    // subtract 1 `if (val < 0)`
    ee.vadds.s32    q5,q5,q7    // subtract 1 `if (val < 0)`
    ee.vadds.s32    q1,q1,q4
    ee.vadds.s32    q3,q3,q5
    ee.vsr.32       q1,q1
    ee.vsr.32       q3,q3

.skip_div_by2_in1:

    ee.vadds.s32        q0,q0,q1
    ee.vadds.s32        q1,q2,q3

// mulhi32 for q0
    ee.movi.32.a    q0,a3,2             # [44]
    ee.movi.32.a    q0,a4,3             # [45]
    ee.movi.32.a    q0,a14,1            # [46]
    ee.movi.32.a    q0,a5,0             # [62]

    mulsh           a13,a8,a3                   # [51]
    mull            a3,a8,a3                    # [53]

    mulsh           a12,a8,a4                   # [50]
    mull            a4,a8,a4                    # [55]

    mulsh           a15,a8,a14                  # [48]
    mull            a14,a8,a14                  # [49]

    ssai            31                          # [47]

    add             a3,a3,a9
    saltu           a2,a3,a9
    add.n           a13,a13,a2
    src             a13,a13,a3

    add             a4,a4,a9
    saltu           a2,a4,a9
    add.n           a12,a12,a2
    src             a12,a12,a4
    ee.movi.32.q    q0,a13,2            # [62]

    add             a14,a14,a9
    saltu           a2,a14,a9
    add.n           a15,a15,a2
    src             a15,a15,a14
    ee.movi.32.q    q0,a12,3            # [62]

    mulsh           a13,a8,a5                   # [51]
    mull            a5,a8,a5                    # [53]
    ee.movi.32.q    q0,a15,1            # [62]

    add             a5,a5,a9
    saltu           a2,a5,a9
    add.n           a13,a13,a2
    src             a13,a13,a5
    ee.movi.32.q    q0,a13,0            # [62]


// mulhi32 for q1
    ee.movi.32.a    q1,a3,2             # [44]
    ee.movi.32.a    q1,a4,3             # [45]
    ee.movi.32.a    q1,a14,1            # [46]
    ee.movi.32.a    q1,a5,0             # [62]

    mulsh           a13,a8,a3                   # [51]
    mull            a3,a8,a3                    # [53]

    mulsh           a12,a8,a4                   # [50]
    mull            a4,a8,a4                    # [55]

    mulsh           a15,a8,a14                  # [48]
    mull            a14,a8,a14                  # [49]

    ssai            31                          # [47]

    add             a3,a3,a9
    saltu           a2,a3,a9
    add.n           a13,a13,a2
    src             a13,a13,a3

    add             a4,a4,a9
    saltu           a2,a4,a9
    add.n           a12,a12,a2
    src             a12,a12,a4
    ee.movi.32.q    q1,a13,2            # [62]

    add             a14,a14,a9
    saltu           a2,a14,a9
    add.n           a15,a15,a2
    src             a15,a15,a14
    ee.movi.32.q    q1,a12,3            # [62]

    mulsh           a13,a8,a5                   # [51]
    mull            a5,a8,a5                    # [53]
    ee.movi.32.q    q1,a15,1            # [62]
    l32i            a14,a1,0                   # [738]  temp_neg_out_shift, -out_shift

    add             a5,a5,a9
    saltu           a2,a5,a9
    add.n           a13,a13,a2
    src             a13,a13,a5
    ee.movi.32.q    q1,a13,0            # [62]


    //q0-q1 has output

    blti            a14,1,.skip_div_by2_out
    addi.n          a5,a1,24
    ee.vcmp.lt.s32  q2,q0,q6
    ee.vcmp.lt.s32  q3,q1,q6
    ee.vldbc.32     q5,a5       // 1 << (exponent - 1)
    wsr.sar         a14         // load right shift
    ee.vadds.s32    q0,q0,q2    // subtract 1 `if (val < 0)`
    ee.vadds.s32    q1,q1,q3    // subtract 1 `if (val < 0)`
    ee.vadds.s32    q0,q0,q5
    ee.vadds.s32    q1,q1,q5
    ee.vsr.32       q0,q0
    ee.vsr.32       q1,q1

.skip_div_by2_out:

// add offset and apply activation
    addi            a15,a1,96
    ee.vldbc.32     q3,a15              # [809]  id:802 out_offset
    ee.vadds.s32    q0,q0,q3            # [811]
    ee.vadds.s32    q1,q1,q3            # [812]
    addi            a13,a1,108
    addi            a14,a1,112
    ee.vldbc.32     q3,a14              # [813]  id:803 activation_max
    ee.vmin.s32     q0,q0,q3            # [815]
    ee.vmin.s32     q1,q1,q3            # [816]
    ee.vldbc.32     q3,a13              # [817]  id:804 activation_min
    l32i            a13,a1,4                   # [818]  temp_neg_input2_shift
    ee.vmax.s32     q1,q1,q3            # [819]
    ee.vmax.s32     q0,q0,q3            # [820]

//pack the data and store
    l32i.n          a9,a1,44                    # [784]  gra_spill_temp_10
    ee.vunzip.16    q0,q1               # [821]
    ee.vunzip.8     q0,q1               # [822]
    l32i.n          a13,a1,28           # gra_spill_temp_6, multiple of 12 index
    ee.vst.l.64.ip  q0,a9,8             # [823]  id:805
    l32i            a15,a1,116                  # [1], size
    l32i.n          a14,a1,40                   # [20]  gra_spill_temp_9
    l32i.n          a10,a1,36                   # [14]  gra_spill_temp_8
    addi            a13,a13,8
    s32i.n          a13,a1,28           # gra_spill_temp_6
    bge             a15,a13,.vector_loop

    l32i.n  a2,a1,56                # [0]  gra_spill_temp_13

// check for leftover
    l32i    a10,a1,116                  # [1]
    slli    a2,a2,3                     # [2]
    bge     a2,a10,.exit          # [3] // done, exit

.process_leftover:
    l32i.n  a3,a1,48                    # [1]  gra_spill_temp_11
    l32i.n  a12,a1,52                   # [2]  gra_spill_temp_12

    l32i.n  a10,a1,12                   # [3]  gra_spill_temp_2
    l32i.n  a14,a1,32                # [8]  gra_spill_temp_7
    add.n   a10,a2,a10                  # [5]
    add.n   a14,a2,a14                  # [6]
    l8ui    a14,a14,0                   # [7]  id:809, input1
    l8ui    a10,a10,0                   # [12]  id:1370, input2

    sext    a14,a14,7                   # [9]
    sext    a10,a10,7                   # [10]
    add.n   a10,a10,a12                 # [11] // add offset2
    add.n   a14,a14,a3                  # [16] // add offset1
    l32i    a12,a1,88                  # [13]  left_shift

    // sat_round_doubling_high_mul step for input1 and input2
    ssl     a12                         # [15]
    sll     a10,a10                     # [20]
    sll     a14,a14                     # [17]

    l32r            a12,.nudge_val             # [0], nudge

    // a13,a3 are free, a12: nudge, a6:mult1
    mulsh           a13,a14,a6
    mull            a9,a14,a6
    ssai            31

    add             a9,a9,a12
    saltu           a3,a9,a12
    add.n           a13,a13,a3
    src             a14,a13,a9 //result in a14

    mulsh           a13,a10,a7
    mull            a9,a10,a7
    ssai            31

    add             a9,a9,a12
    saltu           a3,a9,a12
    add.n           a13,a13,a3
    src             a10,a13,a9 //result in a10

// divide_by_power_of2_step for input1 (a14), input2 (a10)
// free registers: a13, a12, a9, a3

    l32i.n          a12,a1,8   // -input1_shift
    l32i.n          a13,a1,4   // -input2_shift

    blti            a12,1,.skip_div_by2_in0_remain
    l32i.n          a3,a1,16    // 1 << (exponent - 1)
    extui           a9,a14,31,1
    ssr             a12         // load right_shift
    sub             a3,a3,a9    // 1 << (exponent - 1) - (val < 0)
    add             a14,a14,a3
    sra             a14,a14
.skip_div_by2_in0_remain:

    blti            a13,1,.skip_div_by2_in1_remain
    l32i.n          a3,a1,20    // 1 << (exponent - 1)
    extui           a9,a10,31,1
    ssr             a13         // load right_shift
    sub             a3,a3,a9    // 1 << (exponent - 1) - (val < 0)
    add             a10,a10,a3
    sra             a10,a10
.skip_div_by2_in1_remain:

// process output
    l32r            a12,.nudge_val             # [0], nudge
    l32i            a13,a1,0                   // -out_shift
    add.n           a10,a10,a14                 # [45]

// multiply and pick high32
    mulsh           a3,a10,a8
    mull            a10,a10,a8
    ssai            31                          # [0]
    add             a10,a10,a12
    saltu           a9,a10,a12
    add             a12,a3,a9
    src             a12,a12,a10

// div by power of 2 for output

    l32i            a9,a1,96                   # [31]  out_offset
    blti            a13,1,.skip_div_by2_out_remain
    l32i.n          a3,a1,24    // 1 << (exponent - 1)
    extui           a14,a12,31,1
    ssr             a13         // load right_shift
    sub             a3,a3,a14   // 1 << (exponent - 1) - (val < 0)
    add             a12,a12,a3
    sra             a12,a12
.skip_div_by2_out_remain:

// add offset
    add.n   a9,a9,a12                   # [33]

// apply activation
    l32i    a13,a1,112                  # [34]  activation_max
    l32i    a12,a1,108                  # [35]  activation_min
    min     a13,a13,a9                      # [36]
    l32i    a9,a1,92                   # [37]  output
    max     a13,a13,a12                     # [38]
    add.n   a9,a2,a9                    # [39]
    s8i     a13,a9,0                    # [40]  id:1371
    l32i    a12,a1,116
    addi.n  a2,a2,1                 # [41]
    blt     a2,a12,.process_leftover

.exit:
    retw.n                          # [0]

    .size   esp_nn_add_elementwise_s8_esp32s3, . - esp_nn_add_elementwise_s8_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
