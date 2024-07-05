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

	# Program Unit: esp_nn_aligned_s8_to_s16_with_offset_esp32s3
	.type	esp_nn_aligned_s8_to_s16_with_offset_esp32s3, @function
	.align	 4
	.global esp_nn_aligned_s8_to_s16_with_offset_esp32s3

esp_nn_aligned_s8_to_s16_with_offset_esp32s3:	# 0x30d

	entry	a1,48                   	#
	mov.n	a10,a2                  	# // src
	mov.n	a9,a3                   	# // dst
	mov.n	a8,a4                   	# // size
	s32i.n	a5,a1,12               	# [3] // offset
	addi.n	a2,a1,12               	# [4]

	blti	a4,32,.Lt_2_6402         	# [5] if (size < 32) goto unopt

	addi.n	a6,a8,-1               	# [0]
	ee.zero.q	q5                  	# [1]
	ee.vldbc.16	q4,a2             	# [2]  id:136 offset
	mov.n	a3,a10                  	# [3]
	mov.n	a2,a9                   	# [4]
	ee.vld.128.ip	q0,a3,16        	# [5]  id:137
	ee.vld.128.ip	q1,a3,16        	# [6]  id:138
	ee.vcmp.lt.s8	q2,q0,q5        	# [7]
	ee.vzip.8	q0,q2               	# [8]
	ee.vadds.s16	q0,q0,q4         	# [9]
	ee.vadds.s16.st.incp	q0,a2,q0,q2,q4 	# [10]  id:139
	blti	a4,64,.Lt_2_7170         	# [11]

	addi	a5,a4,-32                	# [0]
	srai	a5,a5,5                  	# [1]
	slli	a4,a5,5                  	# [2]
	loopgtz	a5,.LBB37_esp_nn_aligned_s8_to_s16_with_offset_esp32s3 	# [3]

	ee.vst.128.ip	q0,a2,16        	# [0*II+0]  id:140
	ee.vcmp.lt.s8	q0,q1,q5        	# [0*II+1]
	ee.vzip.8	q1,q0               	# [0*II+2]
	ee.vadds.s16.ld.incp	q2,a3,q3,q1,q4 	# [0*II+3]  id:141
	ee.vadds.s16.st.incp	q3,a2,q0,q0,q4 	# [0*II+4]  id:142
	ee.vcmp.lt.s8	q3,q2,q5        	# [0*II+5]
	ee.vst.128.ip	q0,a2,16        	# [0*II+6]  id:143
	ee.vzip.8	q2,q3               	# [0*II+7]
	ee.vadds.s16.ld.incp	q1,a3,q0,q2,q4 	# [0*II+8]  id:144
	ee.vadds.s16.st.incp	q0,a2,q0,q3,q4 	# [0*II+9]  id:145

.LBB37_esp_nn_aligned_s8_to_s16_with_offset_esp32s3:	# 0x36d
	addi	a4,a4,32                 	# [0]

.Lt_2_3842:	# 0x370
	ee.vst.128.ip	q0,a2,16        	# [0]  id:146
	ee.vcmp.lt.s8	q2,q1,q5        	# [1]
	ee.vzip.8	q1,q2               	# [2]
	ee.vadds.s16	q2,q2,q4         	# [3]
	ee.vadds.s16	q3,q1,q4         	# [4]
	ee.vst.128.ip	q3,a2,16        	# [5]  id:147
	ee.vst.128.ip	q2,a2,16        	# [6]  id:148
	bge	a4,a6,.Lt_2_4866          	# [7]

	l32i.n	a5,a1,12               	# [0]  id:135 offset+0x0

.Lt_2_5122:	# 0x38a
	mov.n	a11,a4                  	# [0]
	add.n	a2,a4,a10               	# [1]
 # 576          dst[i + 0] = src[i + 0] + offset;
	l8ui	a7,a2,0                  	# [2]  id:149
	addx2	a6,a4,a9                	# [3]
	sext	a7,a7,7                  	# [4]
	add.n	a7,a7,a5                	# [5]
	s16i	a7,a6,0                  	# [6]  id:150
 # 577          dst[i + 1] = src[i + 1] + offset;
	l8ui	a3,a2,1                  	# [7]  id:151
	sub	a7,a8,a4                  	# [8]
	addi.n	a2,a2,2                	# [9]
	srai	a7,a7,1                  	# [10]
	sext	a3,a3,7                  	# [11]
	add.n	a3,a3,a5                	# [12]
	s16i	a3,a6,2                  	# [13]  id:152
	addi.n	a3,a7,-1               	# [14]
	loopgtz	a3,.LBB52_esp_nn_aligned_s8_to_s16_with_offset_esp32s3 	# [15]

	l8ui	a3,a2,0                  	# [0*II+0]  id:149
	addi.n	a6,a6,4                	# [1*II+1]
	sext	a3,a3,7                  	# [0*II+2]
	add.n	a3,a3,a5                	# [0*II+3]
	s16i	a3,a6,0                  	# [0*II+4]  id:150
	l8ui	a3,a2,1                  	# [0*II+5]  id:151
	addi.n	a2,a2,2                	# [0*II+6]
	sext	a3,a3,7                  	# [0*II+7]
	add.n	a3,a3,a5                	# [0*II+8]
	s16i	a3,a6,2                  	# [0*II+9]  id:152

.LBB52_esp_nn_aligned_s8_to_s16_with_offset_esp32s3:	# 0x3ce
	addx2	a4,a7,a11               	# [0]

.Lt_2_4866:	# 0x3d1
	bge	a4,a8,.Lt_2_7682          	# [0]

 # 580          dst[i] = src[i] + offset;
	addx2	a11,a4,a9               	# [0]
	add.n	a8,a4,a10               	# [1]
	l8ui	a8,a8,0                  	# [2]  id:153
	l32i.n	a12,a1,12              	# [3]  id:135 offset+0x0
	sext	a8,a8,7                  	# [4]
	add.n	a8,a8,a12               	# [5]
	s16i	a8,a11,0                 	# [6]  id:154
	retw.n                        	# [7]

.Lt_2_6402:	# 0x3e8
	blti	a4,2,.Lt_2_6658          	# [0]

	movi.n	a4,0                   	# [0]
	j	.Lt_2_5122                  	# [1]

.Lt_2_7682:	# 0x3f0
	retw.n                        	# [0]

.Lt_2_6658:	# 0x3f2
	blti	a4,1,.Lt_2_7682          	# [0]

	l8ui	a11,a10,0                	# [0]  id:153
	sext	a11,a11,7                	# [2]
	add.n	a11,a11,a5              	# [3]
	s16i	a11,a3,0                 	# [4]  id:154
	retw.n                        	# [5]

.Lt_2_7170:	# 0x402
	movi.n	a4,32                  	# [0]
	j	.Lt_2_3842                  	# [1]

	.size	esp_nn_aligned_s8_to_s16_with_offset_esp32s3, . - esp_nn_aligned_s8_to_s16_with_offset_esp32s3


	.literal_position

	# Program Unit: esp_nn_s8_to_s16_esp32s3
	.type	esp_nn_s8_to_s16_esp32s3, @function
	.align	 4
	.global esp_nn_s8_to_s16_esp32s3

esp_nn_s8_to_s16_esp32s3:	# 0x40b
	entry	a1,32                   	#
	mov.n	a9,a2 // src
	mov.n	a8,a3 // dst
	mov.n	a7,a4 // size
    blti	a4,1,.Lt_3_4866  // size == 0
	blti	a4,16,.Lt_3_4610 // if (size < 16) jump to unopt path

 // load align_len to sar_byte
	extui	a2,a2,0,4               	# [0]
	wur.sar_byte	a2               	# [1]
	mov.n	a2,a9                   	# [2]

 // preload
	ee.vld.128.ip	q0,a2,16
	ee.vld.128.ip	q1,a2,16
    ee.zero.q	    q4
 # 672
 # 673      for (i = 16; i < size - 15; i += 16) {
	blti	a4,32,.Lt_3_5378         	# [5]
	addi	a6,a4,-16                	# [1]
	srai	a6,a6,4                  	# [2]
	slli	a4,a6,4                  	# [3]
	loopgtz	a6,.LBB35_esp_nn_s8_to_s16_esp32s3 	# [4]

	ee.src.q.qup	q2,q0,q1         	# [0*II+0]
	ee.vcmp.lt.s8	q3,q2,q4        	# [0*II+1] // sign
	ee.vld.128.ip	q1,a2,16        	# [0*II+2] // for next iteration
	ee.vzip.8	q2,q3               	# [0*II+3]
	ee.vst.128.ip	q2,a3,16        	# [0*II+4]  id:93
	ee.vst.128.ip	q3,a3,16        	# [0*II+5]  id:94

.LBB35_esp_nn_s8_to_s16_esp32s3:	# 0x449
	addi	a4,a4,16                 	# [0]

.Lt_3_2050:	# 0x44c
	ee.src.q.qup	q5,q0,q1         	# [0]
	ee.vcmp.lt.s8	q3,q5,q4        	# [1]
	ee.vzip.8	q5,q3               	# [2]
	ee.vst.128.ip	q5,a3,16        	# [3]  id:96
	ee.vst.128.ip	q3,a3,16        	# [4]  id:97
 # 687
 # 688  skip_to_remains_s8_to_s16:
 # 689      for (; i < size; i += 2) {
	bge	a4,a7,.Lt_3_4866          	# [5]

.Lt_3_3330:	# 0x45e
	mov.n	a11,a4                  	# [0]
	add.n	a2,a4,a9                	# [1]
 # 690          dst[i + 0] = src[i + 0];
	l8ui	a10,a2,0                 	# [2]  id:98
	addx2	a5,a4,a8                	# [3]
	sext	a10,a10,7                	# [4]
	s16i	a10,a5,0                 	# [5]  id:99
 # 691          dst[i + 1] = src[i + 1];
	l8ui	a3,a2,1                  	# [6]  id:100
	sub	a10,a7,a4                 	# [7]
	addi.n	a2,a2,2                	# [8]
	addi.n	a10,a10,1              	# [9]
	srai	a10,a10,1                	# [10]
	sext	a3,a3,7                  	# [11]
	s16i	a3,a5,2                  	# [12]  id:101
	addi.n	a3,a10,-1              	# [13]
	loopgtz	a3,.LBB50_esp_nn_s8_to_s16_esp32s3 	# [14]

	l8ui	a3,a2,0                  	# [0*II+0]  id:98
	addi.n	a5,a5,4                	# [1*II+1]
	sext	a3,a3,7                  	# [0*II+2]
	s16i	a3,a5,0                  	# [0*II+3]  id:99
	l8ui	a3,a2,1                  	# [0*II+4]  id:100
	addi.n	a2,a2,2                	# [0*II+5]
	sext	a3,a3,7                  	# [0*II+6]
	s16i	a3,a5,2                  	# [0*II+7]  id:101

.LBB50_esp_nn_s8_to_s16_esp32s3:	# 0x49c
	addx2	a4,a10,a11              	# [0]
 # 692      }
 # 693      if(i < size) {
	bge	a4,a7,.Lt_3_4866          	# [1]

 # 694          dst[i] = src[i];
	add.n	a11,a4,a9               	# [0]
	l8ui	a11,a11,0                	# [1]  id:102
	addx2	a12,a4,a8               	# [2]
	sext	a11,a11,7                	# [3]
	s16i	a11,a12,0                	# [4]  id:103
	retw.n                        	# [5]

.Lt_3_4610:	# 0x4b2
	movi.n	a4,0                   	# [0]
	j	.Lt_3_3330                  	# [1]

.Lt_3_4866:	# 0x4ba
	retw.n                        	# [0]

.Lt_3_5378:	# 0x4bc
	movi.n	a4,16                  	# [1]
	j	.Lt_3_2050                  	# [2]

	.size	esp_nn_s8_to_s16_esp32s3, . - esp_nn_s8_to_s16_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
