#include "edge-impulse-sdk/classifier/ei_classifier_config.h"
#if EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN
/*
 * SPDX-FileCopyrightText: 2020-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * Optimizations strategies used:
 * Below optimizations are capable of any size of input/filter:
 *
 * 1. For filter wdxht = 1x1 (Refer esp_nn_conv_s8_mult8_1x1_esp32s3 function)
 *      - For this specific version, the strategy we employ:
 *          > This particular filter has only the channel
 *              dimension and we have `out_ch` number of such filters.
 *          > We take 8 input lines at a time and transpose those.
 *          > Keep loading and multiplying filter values one by one,
 *              to produce 8 outputs in parallel
 *
 * 2. General version: (Refer esp_nn_conv_s8_filter_aligned_input_padded_esp32s3)
 *      - For all other cases:
 *          > Consider `filter_wd * in_ch` as a single row. These many values can
 *              be continuosly loaded from inputs as well.
 *          > multiply accumulate into a single filter output.
 *          > To speed things up further, we pre-calculate
 *              (filter * in_offset + bias term) earlier and add it at the end of filter
 *
 *      About ((filter * in_offset + bias term)) accumulate term:
 *          > The conv operation before requantization is as follows:
 *              for i in filter_size:
 *                  conv_out += (input + input_offset) * filter;
 *               conv_out += bias
 *
 *          > where input_offset is constant term hence, we can see that
 *              this term can be precalculated as:
 *                  for i in filter_size:
 *                      acc_term += input_offset * filter[i];
 *                  acc_term += bias
 *              OR
 *                   for i in filter_size:
 *                      acc_term += filter[i]; // accumulate filter values
 *                  acc_term = acc_term * input_offset + bias
 *
 *
 * In both the above versions we align the filter if needed, pad the input with
 *       -input_offset if needed and extend the channels to make those multiple
 *       of 8/16 as per function needs
 */

#include <stdio.h>
#include <edge-impulse-sdk/porting/espressif/ESP-NN/include/esp_nn_defs.h>

#include <edge-impulse-sdk/porting/espressif/ESP-NN/src/common/common_functions.h>

static int16_t *scratch_buffer = NULL;

extern void esp_nn_conv_s8_mult8_1x1_esp32s3(
                const int8_t *input_data,
                const uint16_t input_wd,
                const uint16_t input_ht,
                const uint16_t in_channels,
                const int32_t input_offset,
                const int8_t *filter_aligned,
                const int32_t *bias,
                int8_t *out_data,
                const uint16_t out_wd,
                const uint16_t out_ht,
                const uint16_t out_channels,
                const int32_t out_offset,
                const int32_t *out_shift,
                const int32_t *out_mult,
                const int32_t activation_min,
                const int32_t activation_max,
                void *buffer /* scratch buffer */);

extern void esp_nn_conv_s8_filter_aligned_input_padded_esp32s3(
                const int8_t *input_data,
                const uint16_t input_wd,
                const uint16_t input_ht,
                const uint16_t in_channels,
                const int32_t input_offset,
                const uint16_t stride_wd,
                const uint16_t stride_ht,
                const int8_t *filter_data,
                const uint16_t filter_wd,
                const uint16_t filter_ht,
                const int32_t *bias,
                int8_t *out_data,
                const uint16_t out_wd,
                const uint16_t out_ht,
                const uint16_t out_channels,
                const int32_t out_offset,
                const int32_t *out_shift,
                const int32_t *out_mult,
                const int32_t activation_min,
                const int32_t activation_max,
                void *scratch_buffer);

int esp_nn_get_conv_scratch_size_esp32s3(const data_dims_t *input_dims,
                                         const data_dims_t *filter_dims,
                                         const data_dims_t *output_dims,
                                         const conv_params_t *conv_params)
{
    const uint16_t input_wd = input_dims->width;
    const uint16_t input_ht = input_dims->height;
    const uint16_t in_ch = input_dims->channels;
    const uint16_t filter_wd = filter_dims->width;
    const uint16_t filter_ht = filter_dims->height;
    const uint16_t out_ch = output_dims->channels;
    const uint16_t pad_wd = conv_params->padding.width;
    const uint16_t pad_ht = conv_params->padding.height;
    const uint16_t stride_wd = conv_params->stride.width;
    const uint16_t stride_ht = conv_params->stride.height;

    int new_channels = (in_ch + 7) & ~7;

    int input_scratch = input_wd * input_ht * in_ch;
    int filter_scratch = filter_wd * filter_ht * in_ch * out_ch;

    int align_buf_size = 32; /* extra buffer for alignment */
    if ((filter_wd == 1 && filter_ht == 1 && pad_wd == 0 && pad_ht == 0) &&
            (stride_wd == 1 && stride_ht == 1)) {
        int transpose_buf_size = 2 * (8 * new_channels); /* to store intermediate data */
        if (input_wd * input_ht < 8) {
            transpose_buf_size = 0; // not using this for leftover
        }
        if (in_ch % 8) {
            input_scratch = input_wd * input_ht * new_channels;
        } else {
            input_scratch = 0;
        }
        filter_scratch = new_channels * out_ch;
        return input_scratch + filter_scratch + transpose_buf_size + align_buf_size;
    } else {
        new_channels = (in_ch + 15) & ~15;
        if (pad_wd == 0 && pad_ht == 0) {
            input_scratch = 0;
        } else {
            input_scratch = (input_wd + 2 * pad_wd) * (input_ht + 2 * pad_ht) * in_ch;
        }
        filter_scratch = filter_wd * filter_ht * new_channels * out_ch;
        int offset_acc_scratch = out_ch * 4;
        return input_scratch + filter_scratch + align_buf_size + offset_acc_scratch;
    }
    return align_buf_size;
}

void esp_nn_set_conv_scratch_buf_esp32s3(void *buf)
{
    scratch_buffer = (int16_t *) buf;
}

void esp_nn_conv_s8_esp32s3(const data_dims_t *input_dims,
                            const int8_t *input,
                            const data_dims_t *filter_dims,
                            const int8_t *filter_data,
                            const int32_t *bias,
                            const data_dims_t *output_dims,
                            int8_t *out_data,
                            const conv_params_t *conv_params,
                            const quant_data_t *quant_data)
{
    if (scratch_buffer == NULL) {
        printf("esp_nn_conv error! scratch_buffer not set!\n");
        return;
    }
    const uint16_t input_wd = input_dims->width;
    const uint16_t input_ht = input_dims->height;
    const uint16_t channels = input_dims->channels;
    const int32_t input_offset = conv_params->in_offset;
    const int32_t out_offset = conv_params->out_offset;
    const uint16_t pad_wd = conv_params->padding.width;
    const uint16_t pad_ht = conv_params->padding.height;
    const uint16_t stride_wd = conv_params->stride.width;
    const uint16_t stride_ht = conv_params->stride.height;
    const uint16_t filter_wd = filter_dims->width;
    const uint16_t filter_ht = filter_dims->height;
    const uint16_t out_wd = output_dims->width;
    const uint16_t out_ht = output_dims->height;
    const uint16_t out_channels = output_dims->channels;
    const int32_t *out_shift = quant_data->shift;
    const int32_t *out_mult = quant_data->mult;
    const int32_t activation_min = conv_params->activation.min;
    const int32_t activation_max = conv_params->activation.max;

    int filter_size = filter_wd * filter_ht * channels * out_channels;

    if (filter_wd == 1 && filter_ht == 1 && pad_wd == 0 && pad_ht == 0 &&
            stride_wd == 1 && stride_ht == 1) {

        int8_t *input_aligned = (int8_t *) input;
        int8_t *scratch_buf = (int8_t *) scratch_buffer;
        int8_t *filter_aligned = (int8_t *) scratch_buffer;
        int new_channels = channels;
        if (channels % 8 == 0) {
            if ((int) filter_data & 7) { // if the filter_data is not aligned to 8 bytes
                int scratch_offset = (int) (filter_aligned + filter_size);
                scratch_buf = (int8_t *) (scratch_offset + 16 - (scratch_offset & 15));
                memcpy(filter_aligned, filter_data, filter_size); // copy to aligned address
            } else {
                filter_aligned = (int8_t *) filter_data;
            }
        } else {
            // pad extra channel to make it multiple of 8. Both input and filter
            new_channels = (channels + 7) & ~7;
            for (int out_ch_idx = 0; out_ch_idx < out_channels; out_ch_idx++) {
                memcpy(filter_aligned, filter_data, channels);
                memset(filter_aligned + channels, 0, new_channels - channels);
                filter_aligned += new_channels;
                filter_data += channels;
            }
            filter_aligned = (int8_t *) scratch_buffer;
            int filter_data_size = new_channels * out_channels;
            input_aligned = filter_aligned + filter_data_size;
            for (int input_idx = 0; input_idx < input_ht * input_wd; input_idx++) {
                memcpy(input_aligned, input, channels);
                memset(input_aligned + channels, 0, new_channels - channels);
                input_aligned += new_channels;
                input += channels;
            }
            input_aligned = filter_aligned + filter_data_size;
            scratch_buf = input_aligned +  input_ht * input_wd * new_channels;
        }
        esp_nn_conv_s8_mult8_1x1_esp32s3(
            input_aligned, input_wd, input_ht, new_channels, input_offset,
            filter_aligned, bias, out_data, out_wd, out_ht, out_channels, out_offset,
            out_shift, out_mult, activation_min, activation_max, scratch_buf);
    } else {
        // align the `filter width * channels` to 16 bytes. Do zero padding for the same
        int32_t filter_row_size = filter_wd * channels;
        int32_t filter_alignment_padding = 16 - (filter_row_size & 15);
        int8_t *filter_data_aligned = (int8_t *) filter_data;
        int8_t *input_padded = (int8_t *) input;
        int8_t *scratch_data = (int8_t *) scratch_buffer;
        int new_input_wd = input_wd, new_input_ht = input_ht;
        if (filter_alignment_padding != 16) {
            // pad filter_data
            int32_t new_row_size = filter_wd * channels + filter_alignment_padding;
            filter_data_aligned = scratch_data;
            int8_t *row_ptr = filter_data_aligned;
            for (int32_t ch_idx = 0; ch_idx < out_channels; ch_idx++) {
                for (int32_t row_idx = 0; row_idx < filter_ht; row_idx++) {
                    memcpy(row_ptr, filter_data, filter_row_size);
                    memset(row_ptr + filter_row_size, 0, new_row_size - filter_row_size);
                    filter_data += filter_row_size;
                    row_ptr += new_row_size;
                }
            }
            scratch_data += new_row_size * filter_ht * out_channels;
            filter_row_size = new_row_size;
        } else if ( (int) filter_data & 15) {
            filter_data_aligned = scratch_data;
            memcpy(filter_data_aligned, filter_data, filter_size);
            scratch_data += filter_size;
        }
        if (pad_wd != 0 || pad_ht != 0) { // need padding
            input_padded = (int8_t *) scratch_data;
            esp_nn_aligned_s8_pad_with_value(input, input_padded, input_wd, input_ht, channels,
                                            -input_offset, pad_wd, pad_ht);
            new_input_wd = input_wd + 2 * pad_wd;
            new_input_ht = input_ht + 2 * pad_ht;
            scratch_data += new_input_wd * new_input_ht * channels;
        }
        esp_nn_conv_s8_filter_aligned_input_padded_esp32s3(
            input_padded, new_input_wd, new_input_ht, channels, input_offset,
            stride_wd, stride_ht, filter_data_aligned, filter_wd, filter_ht,
            bias, out_data, out_wd, out_ht, out_channels, out_offset,
            out_shift, out_mult, activation_min, activation_max, scratch_data);
    }
}

#endif // EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN
