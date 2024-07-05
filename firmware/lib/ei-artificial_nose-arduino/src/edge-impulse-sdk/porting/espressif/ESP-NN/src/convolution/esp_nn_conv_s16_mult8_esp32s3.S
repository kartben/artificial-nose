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
    .literal    .LC10_28_153, -2147483648
    .literal    .LC11_28_154, -1073741823
    .literal    .LC12_28_155, 2147483647
    .literal    .LC13_28_156, 1073741824

    # Program Unit: esp_nn_conv_s16_mult8_esp32s3
    .type   esp_nn_conv_s16_mult8_esp32s3, @function
    .align   4
    .global esp_nn_conv_s16_mult8_esp32s3
esp_nn_conv_s16_mult8_esp32s3:  # 0x6e2
    # qacc_scratch = 0
    # gra_spill_temp_96 = 48
    # gra_spill_temp_97 = 52
    # gra_spill_temp_98 = 56
    # gra_spill_temp_99 = 60
    # gra_spill_temp_100 = 64
    # gra_spill_temp_101 = 68
    # gra_spill_temp_102 = 72
    # gra_spill_temp_103 = 76
    # gra_spill_temp_104 = 80
    # gra_spill_temp_105 = 84
    # gra_spill_temp_106 = 88
    # gra_spill_temp_107 = 92
    # gra_spill_temp_108 = 96
    # gra_spill_temp_109 = 100
    # gra_spill_temp_110 = 104
    # gra_spill_temp_111 = 108
    # gra_spill_temp_112 = 112
    # gra_spill_temp_113 = 116
    # gra_spill_temp_114 = 120
    # gra_spill_temp_115 = 124
    # gra_spill_temp_116 = 128
    # gra_spill_temp_117 = 132
    # gra_spill_temp_118 = 136
    # gra_spill_temp_119 = 140
    # gra_spill_temp_120 = 144
    # gra_spill_temp_121 = 148
    # gra_spill_temp_122 = 152
    # gra_spill_temp_123 = 156
    # gra_spill_temp_124 = 160
    # gra_spill_temp_125 = 164
    # gra_spill_temp_126 = 168
    # gra_spill_temp_127 = 172
    # gra_spill_temp_128 = 176
    # gra_spill_temp_129 = 180
    # gra_spill_temp_130 = 184
    # gra_spill_temp_131 = 188
    # gra_spill_temp_132 = 192
    # gra_spill_temp_133 = 196
    # gra_spill_temp_134 = 200
    # gra_spill_temp_135 = 204
    # gra_spill_temp_136 = 208
    # gra_spill_temp_137 = 212

 // registers:
 // a2: const int16_t *input_data
 // a3: const uint16_t input_wd
 // a4: const uint16_t input_ht
 // a5: const uint16_t in_channels
 // a6: const uint16_t pad_wd
 // a7: const uint16_t pad_ht

 // on stack:
 // const uint16_t stride_wd
 // const uint16_t stride_ht
 // const int16_t *filter_data
 // const uint16_t filter_wd
 // const uint16_t filter_ht
 // const int32_t *bias
 // int8_t *out_data
 // const uint16_t out_wd
 // const uint16_t out_ht
 // const uint16_t out_channels
 // const int32_t out_offset
 // const int32_t *out_shift
 // const int32_t *out_mult
 // const int32_t activation_min
 // const int32_t activation_max


    entry   a1,256                      #
    s32i    a2,a1,176                   # [0]  gra_spill_temp_128
    s32i    a3,a1,192                   # [1]  gra_spill_temp_132
    s32i.n  a6,a1,60                # [2]  gra_spill_temp_99
    l16ui   a8,a1,288                   # [3]  id:282 out_ht+0x0
    s32i    a8,a1,68                    # [4]  gra_spill_temp_101
    beqz.n  a8,.Lt_2_11778          # [5]

    s32i    a7,a1,76                    # [0]  gra_spill_temp_103
    s32i    a1,a1,156                   # [1]  gra_spill_temp_123
    l16ui   a8,a1,272                   # [2]  id:285 filter_ht+0x0
    neg     a11,a7                      # [3]
    movi.n  a12,0                   # [4]
    neg     a14,a6                      # [5]
    l16ui   a15,a1,268                  # [6]  id:286 filter_wd+0x0
    l16ui   a9,a1,292                   # [7]  id:283 out_channels+0x0
    l32i    a10,a1,304                  # [8]  id:284 out_mult+0x0
    s32i    a10,a1,88                   # [9]  gra_spill_temp_106
    s32i    a9,a1,96                    # [10]  gra_spill_temp_108
    s32i    a15,a1,196                  # [11]  gra_spill_temp_133
    s32i.n  a14,a1,48               # [12]  gra_spill_temp_96
    s32i    a12,a1,72                   # [13]  gra_spill_temp_102
    s32i    a11,a1,80                   # [14]  gra_spill_temp_104
    s32i.n  a8,a1,52                # [15]  gra_spill_temp_97
    sub     a13,a3,a14                  # [16]
    mul16u  a8,a5,a8                # [17]
    s32i.n  a13,a1,56               # [18]  gra_spill_temp_98
    sub     a11,a4,a11                  # [19]
    l32i    a12,a1,276                  # [20]  id:292 bias+0x0
    s32i    a12,a1,152                  # [21]  gra_spill_temp_122
    s32i    a11,a1,84                   # [22]  gra_spill_temp_105
    l32i    a14,a1,308                  # [23]  id:290 activation_min+0x0
    l32i    a13,a1,312                  # [24]  id:291 activation_max+0x0
    s32i    a13,a1,144                  # [25]  gra_spill_temp_120
    mull    a15,a15,a8                  # [26]
    addx4   a9,a9,a10                   # [27]
    s32i    a14,a1,140                  # [28]  gra_spill_temp_119
    l32i    a11,a1,300                  # [29]  id:293 out_shift+0x0
    s32i    a11,a1,92                   # [30]  gra_spill_temp_107
    slli    a14,a5,1                    # [31]
    s32i    a9,a1,124                   # [32]  gra_spill_temp_115
    s32i    a15,a1,128                  # [33]  gra_spill_temp_116
    l32i    a8,a1,280                   # [34]  id:288 out_data+0x0
    movi.n  a10,0                   # [35]
    s32i    a10,a1,160                  # [36]  gra_spill_temp_124
    s32i    a8,a1,132                   # [37]  gra_spill_temp_117
    l32i    a15,a1,296                  # [38]  id:289 out_offset+0x0
    l32i    a9,a1,264                   # [39]  id:287 filter_data+0x0
    s32i    a9,a1,180                   # [40]  gra_spill_temp_129
    s32i    a15,a1,136                  # [41]  gra_spill_temp_118
    l16ui   a8,a1,284                   # [42]  id:296 out_wd+0x0
    l16ui   a10,a1,256                  # [43]  id:294 stride_wd+0x0
    s32i    a10,a1,100                  # [44]  gra_spill_temp_109
    s32i    a8,a1,104                   # [45]  gra_spill_temp_110
    addi.n  a15,a5,-1               # [46]
    l16ui   a9,a1,260                   # [47]  id:295 stride_ht+0x0
    s32i    a9,a1,64                    # [48]  gra_spill_temp_100
    srai    a15,a15,3                   # [49]
    j   .Lt_2_12290                     # [50]

.Lt_2_12546:    # 0x788
    l32i    a8,a1,68                    # [0]  gra_spill_temp_101
    l32i    a12,a1,80                   # [1]  gra_spill_temp_104
    l32i    a11,a1,84                   # [2]  gra_spill_temp_105
    l32i    a10,a1,64                   # [3]  gra_spill_temp_100
    l32i    a13,a1,72                   # [4]  gra_spill_temp_102
    l32i    a9,a1,76                    # [5]  gra_spill_temp_103
    addi.n  a13,a13,1               # [6]
    s32i    a13,a1,72                   # [7]  gra_spill_temp_102
    sub     a9,a9,a10                   # [8]
    sub     a11,a11,a10                 # [9]
    add.n   a12,a12,a10                 # [10]
    s32i    a12,a1,80                   # [11]  gra_spill_temp_104
    s32i    a11,a1,84                   # [12]  gra_spill_temp_105
    s32i    a9,a1,76                    # [13]  gra_spill_temp_103
    sub     a13,a13,a8                  # [14]
    beqz    a13,.Lt_2_11778             # [15]

.Lt_2_12290:    # 0x7b6 // width loop
    l32i    a13,a1,104                  # [0]  gra_spill_temp_110
    beqz.n  a13,.Lt_2_12546         # [2]

    l32i    a8,a1,192                   # [0]  gra_spill_temp_132
    l32i    a9,a1,80                    # [1]  gra_spill_temp_104
    movi.n  a11,0                   # [2]
    l32i    a10,a1,76                   # [3]  gra_spill_temp_103
    l32i.n  a12,a1,60               # [4]  gra_spill_temp_99
    l32i.n  a13,a1,56               # [5]  gra_spill_temp_98
    s32i    a13,a1,116                  # [6]  gra_spill_temp_113
    s32i    a12,a1,112                  # [7]  gra_spill_temp_112
    max     a10,a10,a11                 # [8]
    s32i    a10,a1,148                  # [9]  gra_spill_temp_121
    add.n   a9,a9,a10                   # [10]
    l32i.n  a11,a1,48               # [11]  gra_spill_temp_96
    s32i    a11,a1,184                  # [12]  gra_spill_temp_130
    mull    a8,a8,a9                    # [13]
    l32i    a10,a1,84                   # [14]  gra_spill_temp_105
    s32i    a8,a1,120                   # [15]  gra_spill_temp_114
    l32i.n  a9,a1,52                # [16]  gra_spill_temp_97
    movi.n  a8,0                    # [17]
    s32i    a8,a1,108                   # [18]  gra_spill_temp_111
    min     a9,a9,a10                   # [19]
    s32i    a9,a1,204                   # [20]  gra_spill_temp_135
    j   .Lt_2_13058                     # [21]

.Lt_2_13314:    # 0x7f6
#<loop> Part of loop body line 186, head labeled .Lt_2_13058
    l32i    a13,a1,104                  # [0]  gra_spill_temp_110
    l32i    a11,a1,112                  # [1]  gra_spill_temp_112
    l32i    a10,a1,184                  # [2]  gra_spill_temp_130
    l32i    a9,a1,100                   # [3]  gra_spill_temp_109
    l32i    a12,a1,108                  # [4]  gra_spill_temp_111
    l32i    a8,a1,116                   # [5]  gra_spill_temp_113
    addi.n  a12,a12,1               # [6]
    s32i    a12,a1,108                  # [7]  gra_spill_temp_111
    sub     a8,a8,a9                    # [8]
    add.n   a10,a10,a9                  # [9]
    sub     a11,a11,a9                  # [10]
    s32i    a11,a1,112                  # [11]  gra_spill_temp_112
    s32i    a10,a1,184                  # [12]  gra_spill_temp_130
    s32i    a8,a1,116                   # [13]  gra_spill_temp_113
    beq     a12,a13,.Lt_2_12546         # [14]

.Lt_2_13058:    # 0x821 // channel loop
    l32i    a12,a1,96                   # [0]  gra_spill_temp_108
    beqz.n  a12,.Lt_2_13314         # [2]

    movi.n  a11,0                   # [0]
    l32i    a10,a1,112                  # [1]  gra_spill_temp_112
    l32i    a13,a1,92                   # [2]  gra_spill_temp_107
    l32i    a8,a1,152                   # [3]  gra_spill_temp_122
    movi.n  a9,0                    # [4]
    l32i    a12,a1,88                   # [5]  gra_spill_temp_106
    s32i    a12,a1,168                  # [6]  gra_spill_temp_126
    s32i    a9,a1,188                   # [7]  gra_spill_temp_131
    s32i    a8,a1,164                   # [8]  gra_spill_temp_125
    s32i    a13,a1,172                  # [9]  gra_spill_temp_127
    l32i    a8,a1,116                   # [10]  gra_spill_temp_113
    l32i    a13,a1,196                  # [11]  gra_spill_temp_133
    max     a10,a10,a11                 # [12]
    s32i    a10,a1,208                  # [13]  gra_spill_temp_136
    min     a13,a13,a8                  # [14]
    s32i    a13,a1,200                  # [15]  gra_spill_temp_134
    j   .Lt_2_13826                     # [16]

.Lt_2_14082:    # 0x857

// extract data
    l32i    a4,a1,156                   # [0]  gra_spill_temp_123
    ee.st.qacc_l.l.128.ip   a4,16       # [2]  id:303
    ee.st.qacc_l.h.32.ip    a4,0        # [3]  id:304
    l8ui    a9,a1,15                    # [4]  qacc_scratch+15
    l16ui   a8,a1,10                    # [5]  qacc_scratch+10
    l8ui    a12,a1,16                   # [6]  qacc_scratch+16
    l8ui    a11,a1,6                    # [7]  qacc_scratch+6
    l8ui    a10,a1,5                    # [8]  qacc_scratch+5
    s8i     a10,a1,2                    # [9]  qacc_scratch+2
    s8i     a11,a1,3                    # [10]  qacc_scratch+3
    s8i     a12,a1,7                    # [11]  qacc_scratch+7
    s16i    a8,a1,4                     # [12]  qacc_scratch+4
    s8i     a9,a1,6                     # [13]  qacc_scratch+6

    ee.st.qacc_h.l.128.ip   a4,16       # [14]  id:314
    ee.st.qacc_h.h.32.ip    a4,-32      # [15]  id:315
    l8ui    a13,a1,32                   # [16]  qacc_scratch+32
    l8ui    a9,a1,21                    # [17]  qacc_scratch+21
    l8ui    a12,a1,31                   # [18]  qacc_scratch+31
    l16ui   a11,a1,26                   # [19]  qacc_scratch+26
    l8ui    a10,a1,22                   # [20]  qacc_scratch+22
    l16ui   a8,a1,16                    # [21]  qacc_scratch+16
    s16i    a8,a1,8                     # [22]  qacc_scratch+8
    s8i     a10,a1,11                   # [23]  qacc_scratch+11
    s16i    a11,a1,12                   # [24]  qacc_scratch+12
    s8i     a12,a1,14                   # [25]  qacc_scratch+14
    s8i     a9,a1,10                    # [26]  qacc_scratch+10
    s8i     a13,a1,15                   # [27]  qacc_scratch+15

    l32i    a9,a1,152                   # [28]  gra_spill_temp_122, bias
    movi.n  a13,16                  # [29]
    ee.srcmb.s16.qacc   q1,a13,0        # [30]
    ee.vld.128.ip   q0,a4,0             # [31]  id:327
    s32i            a4,a1,156                   # [32]  gra_spill_temp_123
    ee.vzip.16      q0,q1               # [33]
    ee.vadds.s32    q0,q0,q1            # [34]
    ee.movi.32.a    q0,a12,3            # [35]
    ee.movi.32.a    q0,a11,2            # [36]
    ee.movi.32.a    q0,a10,0            # [37]
    add.n           a11,a11,a12                 # [38]
    ee.movi.32.a    q0,a12,1            # [39]
    add.n           a10,a10,a12                 # [40]
    add.n           a10,a10,a11                 # [41]

    beqz.n  a9,.Lt_2_17154          # [42] // skip bias

    l32i    a13,a1,164                  # [0]  gra_spill_temp_125
    l32i.n  a13,a13,0               # [2]  id:329
    add.n   a10,a10,a13                 # [4]
.Lt_2_17154:    # 0x8d7

 # 259                  conv_out = esp_nn_multiply_by_quantized_mult(conv_out, out_mult[out_ch_idx], out_shift[out_ch_idx]);
    l32i    a11,a1,172                  # [0]  gra_spill_temp_127
    l32i    a4,a1,168                   # [1]  gra_spill_temp_126
    l32i.n  a11,a11,0               # [2]  id:331
    l32i.n  a4,a4,0                 # [3]  id:330

    blti    a11,1,.LBB26_esp_nn_conv_s16_mult8_esp32s3  # [4]
    movi.n  a13,0                   # [0]
    j       .Lt_2_17666                     # [1]
.LBB26_esp_nn_conv_s16_mult8_esp32s3:   # 0xa4e
    neg     a13,a11                     # [0]
.Lt_2_17666:    # 0x8e6

    movi.n  a12,0                   # [0]
    max     a12,a11,a12                 # [1]
    movi.n  a11,0                   # [2]
    ssl     a12                         # [3]
    sll     a10,a10                     # [4]
    bne     a10,a4,.Lt_2_20994          # [5]

    l32r    a9,.LC10_28_153             # [0]
    movi.n  a8,1                    # [1]
    sub     a9,a10,a9                   # [2]
    moveqz  a11,a8,a9               # [3]

.Lt_2_20994:    # 0x901
    extui   a8,a4,31,1                  # [0]
    extui   a12,a10,31,1                # [1]
    xor     a12,a12,a8                  # [2]
    extui   a12,a12,0,8                 # [3]

    beqz.n  a12,.Lt_2_18434         # [4]
    movi.n  a12,-1                  # [0]
    l32r    a9,.LC11_28_154             # [1]
    j       .Lt_2_18178                     # [2]

.Lt_2_18434:    # 0xa54
    movi.n  a12,0                   # [0]
    l32r    a9,.LC13_28_156             # [1]
.Lt_2_18178:    # 0x914

    ssai    31                          # [0]
    l32r    a8,.LC12_28_155             # [1]
    mulsh   a6,a4,a10                   # [2]
    mull    a4,a4,a10                   # [3]
    add.n   a6,a6,a12                   # [4]
    add.n   a7,a4,a9                    # [5]
    saltu   a4,a7,a4                    # [6]
    add.n   a4,a4,a6                    # [7]
    srai    a6,a4,31                    # [8]
    and     a6,a6,a8                    # [9]
    add.n   a7,a6,a7                    # [10]
    srai    a3,a6,31                    # [11]
    add.n   a3,a3,a4                    # [12]
    saltu   a6,a7,a6                    # [13]
    add.n   a6,a6,a3                    # [14]
    src     a6,a6,a7                    # [15]
    extui   a3,a11,0,8                  # [16]
    movi.n  a7,1                    # [17]
    ssr     a13                         # [18]
    movnez  a6,a8,a3                # [19]
    sra     a8,a6                       # [20]

    addi.n  a3,a8,1                 # [21]
    ssl     a13                         # [22]
    sll     a7,a7                       # [23]
    extui   a4,a8,31,1                  # [24]
    addi.n  a7,a7,-1                # [25]
    and     a6,a6,a7                    # [26]
    srai    a7,a7,1                     # [27]
    add.n   a4,a4,a7                    # [28]
    l32i    a7,a1,164                   # [29]  gra_spill_temp_125
    salt    a4,a4,a6                    # [30]
    movnez  a8,a3,a4                # [31]
    l32i    a6,a1,172                   # [32]  gra_spill_temp_127
    l32i    a4,a1,132                   # [33]  gra_spill_temp_117
    l32i    a3,a1,160                   # [34]  gra_spill_temp_124
    addi.n  a7,a7,4                 # [35]
    s32i    a7,a1,164                   # [36]  gra_spill_temp_125
    addi.n  a6,a6,4                 # [37]
    s32i    a6,a1,172                   # [38]  gra_spill_temp_127
    l32i    a7,a1,136                   # [39]  gra_spill_temp_118
    l32i    a6,a1,140                   # [40]  gra_spill_temp_119
    add.n   a4,a3,a4                    # [41]
    add.n   a7,a7,a8                    # [42]
    addi.n  a3,a3,1                 # [43]
    l32i    a8,a1,128                   # [44]  gra_spill_temp_116
    max     a6,a6,a7                    # [45]
    s32i    a3,a1,160                   # [46]  gra_spill_temp_124
    l32i    a7,a1,188                   # [47]  gra_spill_temp_131
    l32i    a3,a1,144                   # [48]  gra_spill_temp_120
    add.n   a7,a7,a8                    # [49]
    min     a3,a3,a6                    # [50]
    s8i     a3,a4,0                     # [51]  id:332
    s32i    a7,a1,188                   # [52]  gra_spill_temp_131
    l32i    a4,a1,168                   # [53]  gra_spill_temp_126
    l32i    a6,a1,124                   # [54]  gra_spill_temp_115
    addi.n  a4,a4,4                 # [55]
    s32i    a4,a1,168                   # [56]  gra_spill_temp_126
    sub     a4,a4,a6                    # [57]
    beqz    a4,.Lt_2_13314              # [58]

.Lt_2_13826:    # 0x9b4
    ee.zero.qacc                    # [0]
    l32i    a9,a1,204                   # [1]  gra_spill_temp_135
    l32i    a8,a1,148                   # [2]  gra_spill_temp_121
    s32i    a8,a1,212                   # [3]  gra_spill_temp_137
    bge     a8,a9,.Lt_2_14082           # [4]

.LBB12_esp_nn_conv_s16_mult8_esp32s3:   # 0x9c3
#<loop> Part of loop body line 187, head labeled .Lt_2_13826
    l32i    a8,a1,196                   # [0]  gra_spill_temp_133
    l32i    a7,a1,212                   # [1]  gra_spill_temp_137
    l32i    a13,a1,200                  # [2]  gra_spill_temp_134
    mull    a7,a7,a8                    # [3]
    l32i    a6,a1,120                   # [4]  gra_spill_temp_114
    add.n   a13,a7,a13                  # [5]
    j   .Lt_2_14594                     # [6]

.Lt_2_14850:    # 0x9d7
#<loop> Part of loop body line 201, head labeled .Lt_2_14594
    l32i    a9,a1,204                   # [0]  gra_spill_temp_135
    l32i    a10,a1,212                  # [1]  gra_spill_temp_137
    l32i    a12,a1,192                  # [2]  gra_spill_temp_132
    l32i    a11,a1,196                  # [3]  gra_spill_temp_133
    add.n   a6,a6,a12                   # [4]
    add.n   a7,a7,a11                   # [5]
    add.n   a13,a13,a11                 # [6]
    addi.n  a10,a10,1               # [7]
    s32i    a10,a1,212                  # [8]  gra_spill_temp_137
    sub     a9,a9,a10                   # [9]
    beqz    a9,.Lt_2_14082              # [10]

.Lt_2_14594:    # 0x9f4
    l32i    a9,a1,200                   # [0]  gra_spill_temp_134
    l32i    a8,a1,208                   # [1]  gra_spill_temp_136
    bge     a8,a9,.Lt_2_14850           # [3]

    l32i    a11,a1,176                  # [0]  gra_spill_temp_128
    l32i    a10,a1,184                  # [1]  gra_spill_temp_130
    add.n   a12,a7,a8                   # [2]
    add.n   a10,a10,a8                  # [3]
    add.n   a10,a6,a10                  # [4]
    mull    a10,a5,a10                  # [5]
    mull    a8,a12,a5                   # [6]
    addx2   a10,a10,a11                 # [7]
    l32i    a11,a1,188                  # [8]  gra_spill_temp_131
    add.n   a11,a11,a8                  # [10]
    l32i    a8,a1,180                   # [11]  gra_spill_temp_129
    mov.n   a2,a10                      # [12]
    addx2   a11,a11,a8                  # [13]
    movi.n  a8,8                    # [14]
    mov.n   a3,a11                      # [15]
    j   .Lt_2_15362                     # [16]

.LBB18_esp_nn_conv_s16_mult8_esp32s3:   # 0xa26
    loopgtz a15,.LBB54_esp_nn_conv_s16_mult8_esp32s3    # [0]

    ee.vmulas.s16.qacc.ld.ip    q0,a2,16,q0,q1  # [0*II+0]  id:300
    ee.vld.128.ip   q1,a3,16            # [0*II+1]  id:301
.LBB54_esp_nn_conv_s16_mult8_esp32s3:   # 0xa30

.Lt_2_15618:    # 0xa30
    ee.vmulas.s16.qacc  q0,q1       # [0]
    movi.n  a8,8                    # [1]
    add.n   a10,a10,a14                 # [2]
    add.n   a11,a11,a14                 # [3]
    mov.n   a3,a11                      # [4]
    mov.n   a2,a10                      # [5]
    beq     a12,a13,.Lt_2_14850         # [6]

.Lt_2_15362:    # 0xa40
    ee.vld.128.ip   q1,a3,16            # [0]  id:299
    ee.vld.128.ip   q0,a2,16            # [1]  id:298
    addi.n  a12,a12,1               # [2]
    bltu    a8,a5,.LBB18_esp_nn_conv_s16_mult8_esp32s3  # [3]

    j   .Lt_2_15618                     # [0]

.Lt_2_11778:    # 0xa5c
    retw.n                          # [0]

    .size   esp_nn_conv_s16_mult8_esp32s3, . - esp_nn_conv_s16_mult8_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
