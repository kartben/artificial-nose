#include "edge-impulse-sdk/classifier/ei_classifier_config.h"
#if EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
//
// SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
//
// SPDX-License-Identifier: Apache-2.0
//


//
// Contraints used by this function are:
//     1. pad_wd and pad_ht is 0. For versions needing padding we do this
//        explicitly
//     2. All the filter rows are aligned to 16 bytes boundary. To make sure
//        this is indeed the case, for filter rows (filter_wd * channels) not
//        multiple of 16, we add zeros to fill it till 16 bondary.
//
//     The optimized kernel assumes this and skips filter row with following
//     size: ((filter_wd * input_ch) + 15) & ~15.

	.text

.literal_position
	.literal .LC1, 1073741824

    # Program Unit: esp_nn_conv_s8_filter_aligned_input_padded_esp32s3
	.type	esp_nn_conv_s8_filter_aligned_input_padded_esp32s3, @function
	.align	4
	.global	esp_nn_conv_s8_filter_aligned_input_padded_esp32s3
 // registers:
 // a2: const int16_t *input_data
 // a3: const uint16_t input_wd
 // a4: const uint16_t input_ht
 // a5: const uint16_t in_ch
 // a6: const uint16_t input_offset
 // a7: const uint16_t stride_wd

 // on stack:
 // const uint16_t stride_ht	: 80
 // const int8_t *filter_data	: 84
 // const uint16_t filter_wd	: 88
 // const uint16_t filter_ht	: 92
 // const int32_t *bias			: 96
 // int8_t *out_data			: 100
 // const uint16_t out_wd		: 104
 // const uint16_t out_ht		: 108
 // const uint16_t out_channels	: 112
 // const int32_t out_offset	: 116
 // const int32_t *out_shift	: 120
 // const int32_t *out_mult		: 124
 // const int32_t activation_min: 128
 // const int32_t activation_max: 132
 // void *scratch_buffer: 136

esp_nn_conv_s8_filter_aligned_input_padded_esp32s3:
	entry	sp, 80
	s32i.n  a2, sp, 40  	# input_data
	mov		a11, a6			# input_offset
	l16ui	a2, sp, 88  	# filter_wd
	l32i	a8, sp, 100		# out_data
	l16ui	a6, sp, 80		# stride_ht
	mov.n	a15, a5

	mull	a4, a2, a15		# filter_row_sz
	s32i.n	a8, sp, 24		# out_data_ptr
	movi.n	a9, 0
	s32i.n	a9, sp, 36      # out_y

	addi.n	a4, a4, 15		# to round the size up
	srli	a2, a4, 4		# (filter_row_sz) >> 4
	slli	a12, a2, 4		# ((filter_row_sz) >> 4) << 4

	mull	a4, a6, a3		# stride_ht * input_wd
	mull	a5, a3, a15		# input_wd * in_ch
	l32i.n	a10, sp, 112     # out_ch

	mull 	a9, a7, a15		# stride_wd * in_ch
	mull 	a4, a4, a15		# (stride_ht * input_wd) * in_ch

	slli	a3, a10, 2		# out_ch * 4

	s32i.n	a3, sp, 32		# out_ch * 4
	s32i.n	a5, sp, 12		# input_wd * in_ch
	s32i.n	a9, sp, 52		# stride_wd * in_ch
	s32i	a4, sp, 56		# (stride_ht * input_wd) * in_ch

	l32i.n	a3, sp, 92   	# filter_ht
	l32i	a13, sp, 136	# scratch_buf
	l32i	a5, sp, 84		# filter_data
	mull    a4, a12, a3		# (filter_wd * filter_ht * in_ch)
	srai	a4, a4, 1
	addx4	a10, a10, a13   # scratch_buf + 4 * out_ch
	l32i	a3, sp, 96
	// accumulate filter values per channel into scratch buffer
.L_acc_out_channel_loop:
	movi.n	a9, 0	// acc
	loop	a4, .L_acc_filter_size_loop
	l8ui	a14, a5, 0
	l8ui	a7, a5, 1
	addi.n	a5, a5, 2
	sext	a14, a14, 7
	sext	a7, a7, 7
	add		a9, a9, a14
	add		a9, a9, a7
	.L_acc_filter_size_loop:

	// multiply by offset, add bias and store the acc value per channel
	mull 	a9, a9, a11
	beqz.n 	a3, .L_skip_bias
	l32i	a8, a3, 0
	addi	a3, a3, 4	// this will remain 0 if bias not present
	add 	a9, a9, a8
.L_skip_bias:
	s32i	a9, a13, 0
	addi.n 	a13, a13, 4
	blt    	a13, a10, .L_acc_out_channel_loop

	movi.n	a4, 0			# 0

.L_height_loop:
	l32i.n	a8, sp, 40  	# in_row_ptr
	movi.n	a9, 0
	l32i.n	a10, sp, 104	# out_wd
	s32i.n	a8, sp, 28  	# input_ptr
	s32i.n	a9, sp, 44      # out_x

.L_width_loop:
	movi.n	a9, 0
	l32i	a5, sp, 84		# filter_data
	s32i.n	a9, sp, 20
	l32i	a3, sp, 136		# scratch_buf

.L_out_ch_loop:
	movi.n	a6, 0
	l32i.n	a9, sp, 28  	# input_ptr
	mov.n	a10, a6

.L_filter_ht_loop:
	add.n	a8, a5, a12
	mov.n	a13, a9

	ee.zero.accx
	ee.ld.128.usar.ip 	q0, a13, 16
	ee.vld.128.ip 		q4, a13, 16
	ee.vld.128.ip 		q1, a5, 16

	sub             a15, a8, a5         // row_len - 16
	extui           a14, a15, 4, 1      // if multiple of 16 and not 32
	srai            a15, a15, 5         // multiples of 32
	ee.src.q.qup 	q2, q0, q4
	beqz	a15, .L_vector_32_loop_end

	loop	a15, .L_vector_32_loop_end

	ee.vld.128.ip 					q4, a13, 16
	ee.vmulas.s8.accx.ld.ip.qup 	q3, a5, 16, q2, q1, q0, q4
	ee.vld.128.ip 					q2, a13, 16
	ee.vmulas.s8.accx.ld.ip.qup 	q1, a5, 16, q0, q3, q4, q2
	ee.orq 							q0, q2, q2
	ee.orq 							q2, q4, q4

.L_vector_32_loop_end:
	beqz	a14, .L_vector_loop_end
	ee.vmulas.s8.accx.ld.ip 		q4, a13, 16, q2, q1
	ee.src.q.ld.ip					q1, a5, 16, q0, q4
	ee.orq 							q2, q0, q0

.L_vector_loop_end:
	ee.vmulas.s8.accx 	q2, q1
	addi	a13, a13, -16	// since we incremented by 16 too much
	movi 	a15, 0
	ee.srs.accx  	a14, a15, 0

	mov.n	a5, a8
	add.n 			a6, a6, a14
.L7:
	l32i.n	a8, sp, 12		# input_wd * in_ch
	l32i.n	a2, sp, 92   	# filter_ht
	addi.n	a10, a10, 1		# filter_y_idx
	add.n	a9, a9, a8
	blt		a10, a2, .L_filter_ht_loop
.L9:
	l32i    a7, a3, 0		# load input_offset acc
	addi    a3, a3, 4		# increment offset acc ptr
	l32i.n	a8, sp, 20
	add.n	a6, a6, a7		# add input_offset accumulation

.L_multiply_by_quant_mult:
	l32i	a10, sp, 120
	l32i	a9, sp, 124
	add.n	a2, a10, a8
	l32i.n	a2, a2, 0
	add.n	a7, a9, a8
	l32i.n	a7, a7, 0
	max		a8, a2, a4
	ssl		a8
	sll		a6, a6
	mull	a9, a6, a7
	l32r	a10, .LC1
	sub		a2, a8, a2
	add.n	a8, a9, a10
	mulsh	a6, a6, a7
	movi.n	a7, 1
	bltu	a8, a9, .L13
	movi.n	a7, 0

.L13:
	add.n	a6, a7, a6
	slli	a6, a6, 1
	extui	a8, a8, 31, 1
	or		a6, a6, a8
	beqz.n	a2, .L_skip_div_by_pow_of_2
	addi.n	a7, a2, -1
	movi.n	a9, 1
	extui	a8, a6, 31, 1
	ssl		a7
	sll		a7, a9
	sub		a7, a7, a8
	add.n	a6, a7, a6
	ssr		a2
	sra		a6, a6
.L_skip_div_by_pow_of_2:
	l32i	a10, sp, 116
	l32i	a8, sp, 128
	add.n	a2, a10, a6
	l32i	a9, sp, 132
	l32i.n	a10, sp, 24		# out_data_ptr
	max		a2, a2, a8
	min		a2, a2, a9
	s8i		a2, a10, 0
	l32i.n	a2, sp, 20
	addi.n	a10, a10, 1
	addi.n	a2, a2, 4
	l32i.n	a6, sp, 32
	s32i.n	a2, sp, 20
	s32i.n	a10, sp, 24		# out_data_ptr
	bne		a6, a2, .L_out_ch_loop

.L4:
	l32i.n	a5, sp, 44      # out_x
	l32i.n	a6, sp, 28  	# input_ptr (was stored by height loop)
	l32i.n	a8, sp, 52		# stride_wd * in_ch
	addi.n	a5, a5, 1
	add.n	a6, a6, a8		# input_ptr + stride_wd * in_ch
	l32i.n	a9, sp, 104 	# out_wd
	s32i.n	a5, sp, 44      # out_x
	s32i.n	a6, sp, 28  	# input_ptr
	bne		a9, a5, .L_width_loop

	l32i.n	a10, sp, 36     # out_y
	l32i.n	a2, sp, 40  	# in_row_ptr
	l32i	a5, sp, 56		# (stride_ht * input_wd) * in_ch
	l32i.n	a6, sp, 108		# out_ht
	addi.n	a10, a10, 1
	add.n	a2, a2, a5		# in_row_ptr
	s32i.n	a10, sp, 36     # out_y
	s32i.n	a2, sp, 40  	# in_row_ptr
	blt		a10, a6, .L_height_loop
	// end outer (height) loop
	retw.n

	.size	esp_nn_conv_s8_filter_aligned_input_padded_esp32s3, .-esp_nn_conv_s8_filter_aligned_input_padded_esp32s3

#elif defined(WIO_TERMINAL)
// dummy code, added for old ARM toolchain
.syntax unified
.thumb
.cpu cortex-m0

.section .text
#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN && EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN_S3
