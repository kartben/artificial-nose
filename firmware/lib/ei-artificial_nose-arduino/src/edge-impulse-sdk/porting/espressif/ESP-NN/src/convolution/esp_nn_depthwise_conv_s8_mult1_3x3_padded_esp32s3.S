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

// processes multiple of 16 channels
// already padded version. no additional padding needed
// simply keep sliding filter window by stride_size

    # Program Unit: esp_nn_depthwise_conv_s8_mult1_3x3_padded_esp32s3
    .type   esp_nn_depthwise_conv_s8_mult1_3x3_padded_esp32s3, @function
    .align   4
    .global esp_nn_depthwise_conv_s8_mult1_3x3_padded_esp32s3

esp_nn_depthwise_conv_s8_mult1_3x3_padded_esp32s3:  # 0xccc
    # qacc_scratch = 0
    # gra_spill_temp_103 = 40 // stride_wd*channels
    # gra_spill_temp_104 = 44 // bias_align
    # gra_spill_temp_107 = 48 // input_offset
    # gra_spill_temp_105 = 52 // out_mult_ptr
    # gra_spill_temp_106 = 56 // out_shift_ptr
    # gra_spill_temp_108 = 60 // ch_idx
    # gra_spill_temp_109 = 64 // out_ch
    # gra_spill_temp_110 = 68 // bias_ptr
    # gra_spill_temp_111 = 72 // 2 * (input_wd * channels)
    # gra_spill_temp_112 = 76 // input_data
    # gra_spill_temp_118 = 96
    # gra_spill_temp_119 = 100
    # gra_spill_temp_120 = 104
    # gra_spill_temp_121 = 108
    # gra_spill_temp_113 = 112 // input_wd * channels
    # gra_spill_temp_114 = 116 // input_wd
    # gra_spill_temp_130 = 120

    # gra_spill_temp_141 = 0
    # gra_spill_temp_120 = 16
    # gra_spill_temp_137 = 80

// offset+bias factor
    # gra_spill_temp_134 = 128 //256-128
    # gra_spill_temp_135 = 144 //256-112
    # gra_spill_temp_133 = 160 //256-96
    # gra_spill_temp_132 = 176 //256-80


 // registers:
 // a2: input_data
 // a3: input_wd
 // a4: input_ht
 // a5: channels
 // a6: input_offset
 // a7: stride_wd

 // on stack:

 // 320: stride_ht
 // 324: filter_data
 // 328: *bias
 // 332: *out_data
 // 336: out_wd
 // 340: out_ht
 // 344: out_offset
 // 348: *out_shift
 // 352: *out_mult
 // 356: activation_min
 // 360: activation_max

    entry   a1,320                      #
    mul16u  a7,a7,a5
    s32i    a3,a1,116                   # [0]  gra_spill_temp_114, input_wd
    s32i    a6,a1,48                    # [1]  gra_spill_temp_107, input_offset
    s32i    a7,a1,40                    # gra_spill_temp_103, stride_wd*channels

    addi    a8,a5,-15                   # [2]
    s32i    a2,a1,76                    # [3]  gra_spill_temp_112, input_data
    l32i    a9,a1,328                   # [4]  id:664 bias+0x0
    mov.n   a2,a5                       # [5]
    s32i    a8,a1,64                    # [7]  gra_spill_temp_109
    s32i    a9,a1,68                    # [8]  gra_spill_temp_110, bias_ptr
    blti    a8,1,.Lt_7_4610             # [9]

    l32i    a12,a1,348                  # [4]  id:666 out_shift+0x0
    mul16u  a15,a3,a5               # [1]
    movi.n  a9,0                    # [13]
    s32i    a12,a1,56                   # [9]  gra_spill_temp_106 // out_shift_ptr
    s32i    a9,a1,60                    # [14]  gra_spill_temp_108, ch_idx
    s32i    a15,a1,112                  # [12]  gra_spill_temp_113, input_wd*channels
    l32i    a9,a1,352                   # [24]  id:665 out_mult+0x0
    slli    a15,a15,1                   # [15]
    s32i    a15,a1,72                   # [23]  gra_spill_temp_111, 2 * (input_wd * channels)
    s32i    a9,a1,52                    # [25]  gra_spill_temp_105, out_mult_ptr

// outer most out_ch loop
.Lt_7_5122: # 0xd57
    l32i            a13,a1,324                  # [1]  filter_data
    l32i            a6,a1,60                    # [2]  gra_spill_temp_108, ch_idx
    l32i            a9,a1,48                    # [0]  gra_spill_temp_107, input_offset
    ee.zero.q       q2                      # [3]
    add.n           a13,a6,a13                  # [4]
    s32i            a13,a1,108                  # [5]  gra_spill_temp_121

// multiply accumulate filter points
    ee.vld.128.xp   q1,a13,a2           # [6]  id:673
    ee.vld.128.xp   q3,a13,a2           # [7]  id:674
    ee.vcmp.lt.s8   q0,q1,q2            # [8]
    ee.vcmp.lt.s8   q4,q3,q2            # [9]
    ee.vzip.8       q1,q0                   # [10]
    ee.vzip.8       q3,q4                   # [11]
    ee.vadds.s16    q0,q0,q4            # [12]
    ee.vld.128.xp   q4,a13,a2           # [13]  id:675
    ee.vadds.s16    q1,q1,q3            # [14]
    ee.vcmp.lt.s8   q3,q4,q2            # [15]
    ee.vzip.8       q4,q3                   # [16]
    ee.vadds.s16    q1,q1,q4            # [17]
    ee.vld.128.xp   q4,a13,a2           # [18]  id:676
    ee.vadds.s16    q0,q0,q3            # [19]
    ee.vcmp.lt.s8   q3,q4,q2            # [20]
    ee.vzip.8       q4,q3                   # [21]
    ee.vadds.s16    q0,q0,q3            # [22]
    ee.vld.128.xp   q3,a13,a2           # [23]  id:677
    ee.vadds.s16    q1,q1,q4            # [24]
    ee.vcmp.lt.s8   q4,q3,q2            # [25]
    ee.vzip.8       q3,q4                   # [26]
    ee.vadds.s16    q1,q1,q3            # [27]
    ee.vld.128.xp   q3,a13,a2           # [28]  id:678
    ee.vadds.s16    q0,q0,q4            # [29]
    ee.vcmp.lt.s8   q4,q3,q2            # [30]
    ee.vzip.8       q3,q4                   # [31]
    ee.vadds.s16    q0,q0,q4            # [32]
    ee.vld.128.xp   q4,a13,a2           # [33]  id:679
    ee.vadds.s16    q1,q1,q3            # [34]
    ee.vcmp.lt.s8   q3,q4,q2            # [35]
    ee.vzip.8       q4,q3                   # [36]
    ee.vadds.s16    q1,q1,q4            # [37]
    ee.vld.128.xp   q4,a13,a2           # [38]  id:680
    ee.vadds.s16    q0,q0,q3            # [39]
    ee.vcmp.lt.s8   q3,q4,q2            # [40]
    ee.vzip.8       q4,q3                   # [41]
    ee.vadds.s16    q0,q0,q3            # [42]
    ee.vld.128.xp   q3,a13,a2           # [44]  id:681
    ee.vadds.s16    q1,q1,q4            # [43]
    ee.vcmp.lt.s8   q2,q3,q2            # [47]
    ee.vzip.8       q3,q2                   # [48]
    ee.vadds.s16    q0,q0,q2            # [49]
    ee.vadds.s16    q1,q1,q3            # [50]

    ee.movi.32.a    q1,a15,1            # [51]
    ee.movi.32.a    q1,a8,3             # [52]
    ee.movi.32.a    q0,a10,3            # [54]
    ee.movi.32.a    q0,a13,1            # [55]
    srai            a11,a10,16                  # [56]
    srai            a12,a8,16                   # [57]
    mull            a12,a9,a12                  # [58]
    mull            a11,a9,a11                  # [59]
    sext            a8,a8,15                    # [328]
    sext            a10,a10,15                  # [61]
    srai            a14,a13,16                  # [62]
    mull            a14,a9,a14                  # [63]
    mull            a10,a9,a10                  # [64]
    mull            a8,a9,a8                    # [65]
    sext            a13,a13,15                  # [66]
    mull            a13,a9,a13                  # [67]
    ee.movi.32.q    q3,a11,3            # [68]
    ee.movi.32.q    q4,a12,3            # [69]
    ee.movi.32.q    q4,a8,2             # [70]
    ee.movi.32.q    q3,a10,2            # [71]
    ee.movi.32.a    q1,a11,2            # [72]
    srai            a12,a11,16                  # [74]
    srai            a8,a15,16                   # [75]
    mull            a8,a9,a8                    # [76]
    mull            a12,a9,a12                  # [77]
    sext            a15,a15,15                  # [78]
    sext            a11,a11,15                  # [79]
    mull            a11,a9,a11                  # [80]
    mull            a15,a9,a15                  # [81]
    ee.movi.32.q    q4,a12,1            # [82]
    ee.movi.32.q    q1,a8,3             # [83]
    ee.movi.32.q    q1,a15,2            # [84]
    ee.movi.32.q    q4,a11,0            # [85]
    ee.movi.32.a    q0,a15,2            # [86]
    ee.movi.32.q    q0,a14,3            # [88]
    ee.movi.32.q    q0,a13,2            # [91]
    srai            a8,a15,16                   # [89]
    mull            a8,a9,a8                    # [90]
    sext            a15,a15,15                  # [92]
    mull            a15,a9,a15                  # [93]
 # 526  MUL_IN_OFFSET_EXPAND(q_sum2, 0, q_sum2, 0);
    ee.movi.32.a    q0,a11,0            # [94]
    srai            a13,a11,16                  # [95]
    ee.movi.32.q    q3,a8,1             # [96]
    ee.movi.32.q    q3,a15,0            # [100]
    sext            a11,a11,15                  # [97]
    mull            a13,a9,a13                  # [98]
    l32i            a8,a1,332                   # [99]
    ee.movi.32.a    q1,a10,0            # [103]
    ee.movi.32.q    q0,a13,1            # [100]
    srai            a12,a10,16                  # [105]
    sext            a10,a10,15                  # [106]
    mull            a12,a9,a12                  # [107]
    mull            a10,a9,a10                  # [108]
    mull            a9,a9,a11                   # [109]
    ee.movi.32.q    q1,a12,1            # [110]
    ee.movi.32.q    q1,a10,0            # [111]

    l32i            a11,a1,328      // load bias
    add.n           a6,a6,a8                    # [102]
    ee.movi.32.q    q0,a9,0             # [113]
    beqz.n          a11,.Lt_7_5378          # [114]

// add bias
    l32i            a8,a1,68                    # [0]  gra_spill_temp_110, bias_ptr
    extui           a11,a11,0,4                 # [2] // bias_align
    wur.sar_byte    a11                 # [4]
    ee.vld.128.ip   q5,a8,16            # [5]  id:683
    ee.vld.128.ip   q6,a8,16            # [6]  id:684
    ee.vld.128.ip   q7,a8,16            # [7]  id:685
    addmi           a10,a1,256                  # [2]
    ee.src.q.ld.ip  q2,a8,16,q5,q6              # [9]
    ee.vadds.s32    q1,q1,q5            # [12]
    ee.src.q.ld.ip  q5,a8,0,q6,q7               # [13]
    s32i            a8,a1,68                    # [11]  gra_spill_temp_110, bias_ptr
    ee.vadds.s32    q4,q4,q6            # [18]
    ee.src.q        q7,q7,q2                # [9]
    ee.src.q        q2,q2,q5                # [13]
    ee.vadds.s32    q0,q0,q7            # [12]
    ee.vadds.s32    q3,q3,q2            # [12]
.Lt_7_5378: # 0xeef

// store offset+bias factor (q1,q4,q0,q3)
    st.qr           q4,a10,-112                  # [17]  gra_spill_temp_135-256
    st.qr           q3,a10,-128                  # [21]  gra_spill_temp_134-256
    st.qr           q1,a10,-96                  # [7]  gra_spill_temp_133-256
    st.qr           q0,a10,-80                  # [8]  gra_spill_temp_132-256

// prepare height loop
    movi.n  a15,0                   # [1]
    movi.n  a8,0                    # [2]
    movi.n  a9,0                    # [3]
    s32i    a9,a1,100                   # [4]  gra_spill_temp_119
    s32i    a8,a1,104                   # [5]  gra_spill_temp_120
    s32i    a15,a1,96                  # [6]  gra_spill_temp_118

// height loop
.Lt_7_6402: # 0xf0c
    l32i    a4,a1,104                   # [2]  gra_spill_temp_120 // out_y * (input_wd * stride_ht) * channels)
    l32i    a8,a1,100                   # [3]  gra_spill_temp_119 // initialised to 0 before height loop
    l32i    a5,a1,76                    # [1]  gra_spill_temp_112, input_data
    l32i    a3,a1,60                    # [0]  gra_spill_temp_108, ch_idx
    l32i    a7,a1,112                   # [1]  gra_spill_temp_113, input_wd*channels
    l32i    a10,a1,336                  # [0]  out_wd
    add.n   a4,a4,a5                    # [4] // input_data + (out_y * stride_ht) * input_wd * channels
    mov.n   a5,a8                       # [5] // index
    add.n   a3,a3,a4                    # [6] // input_row0
    l32i    a4,a1,72                    # [9]  gra_spill_temp_111, 2 * (input_wd * channels)
    add.n   a7,a7,a3                    # [7] // input_row1 = (input_wd * channels)
    add.n   a8,a8,a10                   # [8]
    s32i    a8,a1,120                   # [10]  gra_spill_temp_130
    add.n   a4,a4,a3                    # [11] // input_row2

// width loop
.Lt_7_7170: # 0xf32
    l32i                    a9,a1,108                   # [3]  gra_spill_temp_121, filter_ptr
    ee.zero.qacc                    # [2]
    mov.n                   a12,a3                      # [4]
    mov.n                   a11,a7                      # [1]
    mov.n                   a10,a4                      # [0]
    ee.vld.128.xp           q0,a12,a2           # [5]  id:693
    ee.vld.128.xp           q6,a12,a2           # [6]  id:695
    ee.vld.128.xp           q1,a9,a2            # [7]  id:694
    ee.vld.128.xp           q7,a9,a2            # [8]  id:696
    ee.vld.128.xp           q5,a9,a2            # [9]  id:698
    ee.vld.128.xp           q3,a9,a2            # [10]  id:700
    ee.vmulas.s8.qacc.ld.xp q4,a12,a2,q0,q1     # [11]  id:697
    ee.vmulas.s8.qacc.ld.xp q2,a11,a2,q6,q7     # [13]  id:699
    ee.vld.128.xp           q1,a9,a2            # [14]  id:702
    ee.vmulas.s8.qacc.ld.xp q0,a11,a2,q4,q5     # [15]  id:701
    ee.vmulas.s8.qacc.ld.xp q6,a11,a2,q2,q3     # [16]  id:703
    ee.vld.128.xp           q7,a9,a2            # [17]  id:704
    ee.vld.128.xp           q3,a9,a2            # [18]  id:706
    ee.vmulas.s8.qacc.ld.xp q0,a10,a2,q0,q1     # [19]  id:705
    ee.vmulas.s8.qacc.ld.xp q1,a10,a2,q6,q7     # [20]  id:707
    ee.vmulas.s8.qacc.ld.xp q4,a10,a2,q0,q3     # [21]  id:709
    ee.vld.128.xp           q6,a9,a2            # [22]  id:708
    ee.vld.128.xp           q5,a9,a2            # [23]  id:710
    ee.vmulas.s8.qacc       q1,q6           # [24]
    ee.vmulas.s8.qacc       q4,q5           # [25]

 // extract data
    mov     a12,a1      //// scratch
    ee.st.qacc_l.l.128.ip   a12,16      # [27]  id:713
    ee.st.qacc_l.h.32.ip    a12,-16     # [28]  id:714

    l32i.n  a9,a1,8                 # [29]  qacc_scratch+8
    l32i.n  a11,a1,4                # [30]  qacc_scratch+4
    l32i.n  a15,a1,0                # [31]  qacc_scratch
    slli    a14,a11,24                  # [32]
    sext    a8,a15,19                   # [33]
    slli    a10,a9,16                   # [34]
    slli    a13,a11,4                   # [35]
    extui   a9,a9,16,16                 # [36]
    srai    a13,a13,12                  # [37]
    extui   a15,a15,20,12               # [39]
    srai    a14,a14,12                  # [40]
    srai    a10,a10,12                  # [41]
    extui   a11,a11,28,4                # [42]
    or      a10,a10,a11                 # [43]
    or      a14,a14,a15                 # [44]

// insert to q0
    ee.movi.32.q    q0,a8,0             # [38]
    ee.movi.32.q    q0,a14,1            # [45]
    ee.movi.32.q    q0,a13,2            # [48]
    ee.movi.32.q    q0,a10,3            # [49]

    l32i.n  a11,a1,16               # [46]  qacc_scratch+16
    l32i.n  a14,a1,12               # [47]  qacc_scratch+12
    slli    a13,a11,20                  # [50]

    ee.st.qacc_h.l.128.ip   a12,16      # [51]  id:720
    ee.st.qacc_h.h.32.ip    a12,-16     # [55]  id:721
    srai    a11,a11,12                  # [52]
    srai    a13,a13,12                  # [53]
    slli    a8,a14,28                   # [54]
    slli    a15,a14,8                   # [56]
    srai    a15,a15,12                  # [57]
    srai    a8,a8,12                    # [59]

    l32i.n          a12,a1,8                # [328]  qacc_scratch+8
    or              a8,a8,a9                    # [61]
    extui           a14,a14,24,8                # [62]
    l32i.n          a9,a1,0                 # [63]  qacc_scratch
    or              a13,a13,a14                 # [64]
//insert to q3
    ee.movi.32.q    q3,a8,0             # [65]
    ee.movi.32.q    q3,a15,1            # [67]
    ee.movi.32.q    q3,a13,2            # [69]
    ee.movi.32.q    q3,a11,3            # [70]

    l32i.n          a14,a1,4                # [66]  qacc_scratch+4
    sext            a10,a9,19                   # [68]
    extui           a9,a9,20,12                 # [72]
    slli            a13,a12,16                  # [73]
    slli            a8,a14,24                   # [74]
    extui           a12,a12,16,16               # [75]
    srai            a13,a13,12                  # [76]
    srai            a8,a8,12                    # [77]
    slli            a15,a14,4                   # [78]
    srai            a15,a15,12                  # [79]
    or              a8,a8,a9                    # [80]
    extui           a14,a14,28,4                # [81]
    l32i.n          a9,a1,12                # [82]  qacc_scratch+12
    or              a13,a13,a14                 # [83]
// insert to q1
    ee.movi.32.q    q1,a10,0            # [71]
    ee.movi.32.q    q1,a8,1             # [84]
    ee.movi.32.q    q1,a15,2            # [85]
    ee.movi.32.q    q1,a13,3            # [88]

// load in_offset+bias factor
    addmi           a14,a1,256                  # [86]
    ld.qr           q7,a14,-128                  # [87]  gra_spill_temp_134-256
    ld.qr           q4,a14,-112                  # [89]  gra_spill_temp_135-256
    l32i.n          a15,a1,16               # [90]  qacc_scratch+16
    ld.qr           q2,a14,-96                  # [91]  gra_spill_temp_133-256
    slli            a11,a9,28                   # [92]
    slli            a10,a9,8                    # [93]
    srai            a10,a10,12                  # [94]
    srai            a11,a11,12                  # [95]
    extui           a9,a9,24,8                  # [96]
    or              a11,a11,a12                 # [97]
    ee.vadds.s32    q0,q0,q2            # [98]
    slli            a8,a15,20                   # [99]
    ee.vadds.s32    q3,q3,q4            # [100]
    st.qr           q3,a1,80                # [101]  gra_spill_temp_137-256
    srai            a15,a15,12                  # [102]
    ld.qr           q2,a14,-80                  # [103]  gra_spill_temp_132-256
    srai            a8,a8,12                    # [105]
    or              a8,a8,a9                    # [108]

// insert to q6
    ee.movi.32.q    q6,a11,0            # [100]
    ee.movi.32.q    q6,a10,1            # [107]
    ee.movi.32.q    q6,a8,2             # [112]
    ee.movi.32.q    q6,a15,3            # [113]

    ee.vadds.s32    q1,q1,q2            # [110]
    ee.vadds.s32    q6,q6,q7            # [114]
    st.qr           q1,a1,16                   # [111]  gra_spill_temp_120
    s32i.n          a7,a1,32                # [0] // tmp
    s32i.n          a6,a1,36                # [106] // tmp
    l32i            a7,a1,52                # [109]  gra_spill_temp_105, out_mult_ptr
    l32i            a6,a1,56                # [106]  gra_spill_temp_106, out_shift_ptr
    addi.n          a10,a7,0
    addi.n          a11,a6,0
    call8   esp_nn_multiply_by_quantized_mult_ver1_esp32s3     # [116]  esp_nn_multiply_by_quantized_mult_ver1_esp32s3

    mv.qr       q5,q0
    ld.qr       q0,a1,80                # [4]  gra_spill_temp_137-256
    addi.n      a10,a7,16
    addi.n      a11,a6,16
    call8   esp_nn_multiply_by_quantized_mult_ver1_esp32s3     # [5]  esp_nn_multiply_by_quantized_mult_ver1_esp32s3

    mv.qr       q4,q0
    ld.qr       q0,a1,16                   # [5]  gra_spill_temp_120
    addi.n      a10,a7,32
    addi.n      a11,a6,32
    call8   esp_nn_multiply_by_quantized_mult_ver1_esp32s3     # [6]  esp_nn_multiply_by_quantized_mult_ver1_esp32s3

    st.qr       q0,a1,0                 # [3]  gra_spill_temp_141
    mv.qr       q0,q6
    addi.n      a10,a7,48
    addi.n      a11,a6,48
    call8   esp_nn_multiply_by_quantized_mult_ver1_esp32s3     # [6]  esp_nn_multiply_by_quantized_mult_ver1_esp32s3


    l32i.n  a6,a1,36                # [106]  // tmp
    l32i.n  a7,a1,32                # [0]  // tmp
    l32i    a15,a1,40                   # gra_spill_temp_103, stride_wd * channels
    l32i    a11,a1,120                  # [3]  gra_spill_temp_130

    add.n   a3,a3,a15                   # [0]
    add.n   a4,a4,a15                   # [1]
    add.n   a7,a7,a15                   # [2]
    addi.n  a5,a5,1                 # [4]

 // add offset, apply activation and store
    addmi   a13,a1,256                  # [8]
    ld.qr   q3,a1,0                 # [10]  gra_spill_temp_141
    mv.qr   q2,q5
    addi    a8,a13,88                   # [14]
    addi    a9,a13,100                  # [15]
    addi    a15,a13,104                 # [13]
    ee.vldbc.32     q6,a9               # [17]  id:723 activation_min
    ee.vldbc.32     q1,a8               # [18]  id:722 out_offset
    ee.vldbc.32     q7,a15              # [19]  id:724 activation_max
    ee.vadds.s32    q4,q4,q1            # [20]
    ee.vadds.s32    q2,q2,q1            # [21]
    ee.vadds.s32    q5,q0,q1            # [22]
    ee.vadds.s32    q3,q3,q1            # [23]
    ee.vmin.s32     q3,q3,q7            # [24]
    ee.vmin.s32     q5,q5,q7            # [25]
    ee.vmin.s32     q2,q2,q7            # [26]
    ee.vmin.s32     q4,q4,q7            # [27]
    ee.vmax.s32     q4,q4,q6            # [28]
    ee.vmax.s32     q2,q2,q6            # [29]
    ee.vmax.s32     q5,q5,q6            # [30]
    ee.vmax.s32     q3,q3,q6            # [31]
    ee.vunzip.16    q3,q5               # [32]
    ee.vunzip.16    q2,q4               # [33]
    ee.vunzip.8     q2,q3               # [34]
    ee.vst.128.xp   q2,a6,a2            # [35]  id:725
    bne             a5,a11,.Lt_7_7170               # [36]

.Lt_7_6658: # 0x112f
#<loop> Part of loop body line 548, head labeled .Lt_7_6402
    l32i    a15,a1,112                  # [3]  gra_spill_temp_113, input_wd*channels
    l32i    a10,a1,320                  # gra_spill_temp_103
    l32i    a13,a1,340                  # [0]  // out_ht
    l32i    a9,a1,116                   # [1]  gra_spill_temp_114, input_wd
    l32i    a12,a1,96                  # [4]  gra_spill_temp_118
    mull    a15,a10,a15                 # // (input_wd * stride_ht) * channels
    l32i    a14,a1,104                  # [5]  gra_spill_temp_120
    l32i    a8,a1,100                   # [2]  gra_spill_temp_119

    addi.n  a12,a12,1               # [6]
    s32i    a12,a1,96                  # [7]  gra_spill_temp_118
    add.n   a14,a14,a15                 # [8]
    add.n   a8,a8,a9                    # [9]
    s32i    a8,a1,100                   # [10]  gra_spill_temp_119
    s32i    a14,a1,104                  # [11]  gra_spill_temp_120, (input_wd * stride_wd) * channels
    bne     a12,a13,.Lt_7_6402              # [13] // iterate over height loop

#<loop> Part of loop body line 348, head labeled .Lt_7_5122
    l32i    a11,a1,56                   # [6]  gra_spill_temp_106 // out_shift_ptr
    l32i    a15,a1,52                   # [2]  gra_spill_temp_105, out_mult_ptr
    l32i    a10,a1,60                   # [24]  gra_spill_temp_108, ch_idx
    addi    a11,a11,64                  # [8]
    addi    a15,a15,64                  # [13]
    s32i    a11,a1,56                   # [23]  gra_spill_temp_106
    s32i    a15,a1,52                   # [18]  gra_spill_temp_105, out_mult_ptr
    l32i    a11,a1,64                   # [25]  gra_spill_temp_109
    addi    a10,a10,16                  # [26]
    s32i    a10,a1,60                   # [27]  gra_spill_temp_108, ch_idx
    blt     a10,a11,.Lt_7_5122          # [28] // iterate over outer most out_ch loop

.Lt_7_4610: # 0x11ad
    retw.n                          # [0]

    .size   esp_nn_depthwise_conv_s8_mult1_3x3_padded_esp32s3, . - esp_nn_depthwise_conv_s8_mult1_3x3_padded_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
