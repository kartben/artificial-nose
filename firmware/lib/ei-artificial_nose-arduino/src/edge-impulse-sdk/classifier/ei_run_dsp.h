/* Edge Impulse inferencing library
 * Copyright (c) 2021 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _EDGE_IMPULSE_RUN_DSP_H_
#define _EDGE_IMPULSE_RUN_DSP_H_

#include "model-parameters/model_metadata.h"
#include "edge-impulse-sdk/dsp/spectral/spectral.hpp"
#include "edge-impulse-sdk/dsp/speechpy/speechpy.hpp"
#include "edge-impulse-sdk/classifier/ei_signal_with_range.h"

#if defined(__cplusplus) && EI_C_LINKAGE == 1
extern "C" {
    extern void ei_printf(const char *format, ...);
}
#else
extern void ei_printf(const char *format, ...);
#endif

#ifdef __cplusplus
namespace {
#endif // __cplusplus

using namespace ei;

#if defined(EI_DSP_IMAGE_BUFFER_STATIC_SIZE)
float ei_dsp_image_buffer[EI_DSP_IMAGE_BUFFER_STATIC_SIZE];
#endif

// this is the frame we work on... allocate it statically so we share between invocations
static float *ei_dsp_cont_current_frame = nullptr;
static size_t ei_dsp_cont_current_frame_size = 0;
static int ei_dsp_cont_current_frame_ix = 0;

__attribute__((unused)) int extract_spectral_analysis_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float frequency) {
    ei_dsp_config_spectral_analysis_t config = *((ei_dsp_config_spectral_analysis_t*)config_ptr);

    int ret;

    const float sampling_freq = frequency;

    // input matrix from the raw signal
    matrix_t input_matrix(signal->total_length / config.axes, config.axes);
    if (!input_matrix.buffer) {
        EIDSP_ERR(EIDSP_OUT_OF_MEM);
    }

    signal->get_data(0, signal->total_length, input_matrix.buffer);

    // scale the signal
    ret = numpy::scale(&input_matrix, config.scale_axes);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: Failed to scale signal (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    // transpose the matrix so we have one row per axis (nifty!)
    ret = numpy::transpose(&input_matrix);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: Failed to transpose matrix (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    // the spectral edges that we want to calculate
    matrix_t edges_matrix_in(64, 1);
    size_t edge_matrix_ix = 0;

    char spectral_str[128] = { 0 };
    if (strlen(config.spectral_power_edges) > sizeof(spectral_str) - 1) {
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }
    memcpy(spectral_str, config.spectral_power_edges, strlen(config.spectral_power_edges));

    // convert spectral_power_edges (string) into float array
    char *spectral_ptr = spectral_str;
    while (spectral_ptr != NULL) {
        while((*spectral_ptr) == ' ') {
            spectral_ptr++;
        }

        edges_matrix_in.buffer[edge_matrix_ix++] = atof(spectral_ptr);

        // find next (spectral) delimiter (or '\0' character)
        while((*spectral_ptr != ',')) {
            spectral_ptr++;
            if (*spectral_ptr == '\0') break;
        }

        if (*spectral_ptr == '\0') {
            spectral_ptr = NULL;
        }
        else  {
            spectral_ptr++;
        }
    }
    edges_matrix_in.rows = edge_matrix_ix;

    // calculate how much room we need for the output matrix
    size_t output_matrix_cols = spectral::feature::calculate_spectral_buffer_size(
        true, config.spectral_peaks_count, edges_matrix_in.rows
    );
    // ei_printf("output_matrix_size %hux%zu\n", input_matrix.rows, output_matrix_cols);
    if (output_matrix->cols * output_matrix->rows != static_cast<uint32_t>(output_matrix_cols * config.axes)) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    output_matrix->cols = output_matrix_cols;
    output_matrix->rows = config.axes;

    spectral::filter_t filter_type;
    if (strcmp(config.filter_type, "low") == 0) {
        filter_type = spectral::filter_lowpass;
    }
    else if (strcmp(config.filter_type, "high") == 0) {
        filter_type = spectral::filter_highpass;
    }
    else {
        filter_type = spectral::filter_none;
    }

    ret = spectral::feature::spectral_analysis(output_matrix, &input_matrix,
        sampling_freq, filter_type, config.filter_cutoff, config.filter_order,
        config.fft_length, config.spectral_peaks_count, config.spectral_peaks_threshold, &edges_matrix_in);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: Failed to calculate spectral features (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    // flatten again
    output_matrix->cols = config.axes * output_matrix_cols;
    output_matrix->rows = 1;

    return EIDSP_OK;
}

matrix_i16_t *create_edges_matrix(ei_dsp_config_spectral_analysis_t config, const float sampling_freq)
{
    // the spectral edges that we want to calculate
    static matrix_i16_t edges_matrix_in(64, 1);
    static bool matrix_created = false;
    size_t edge_matrix_ix = 0;

    if(matrix_created == false) {

        char spectral_str[128] = { 0 };
        if (strlen(config.spectral_power_edges) > sizeof(spectral_str) - 1) {
            return NULL;
        }
        memcpy(spectral_str, config.spectral_power_edges, strlen(config.spectral_power_edges));

        // convert spectral_power_edges (string) into float array
        char *spectral_ptr = spectral_str;
        while (spectral_ptr != NULL) {
            float edge = (atof(spectral_ptr) / (float)(sampling_freq/2.f));
            numpy::float_to_int16(&edge, &edges_matrix_in.buffer[edge_matrix_ix++], 1);

            // find next (spectral) delimiter (or '\0' character)
            while((*spectral_ptr != ',')) {
                spectral_ptr++;
                if (*spectral_ptr == '\0') break;
            }

            if (*spectral_ptr == '\0') {
                spectral_ptr = NULL;
            }
            else  {
                spectral_ptr++;
            }
        }
        edges_matrix_in.rows = edge_matrix_ix;
        matrix_created = true;
    }

    return &edges_matrix_in;
}

__attribute__((unused)) int extract_spectral_analysis_features(signal_i16_t *signal, matrix_i32_t *output_matrix, void *config_ptr, const float frequency) {
    ei_dsp_config_spectral_analysis_t config = *((ei_dsp_config_spectral_analysis_t*)config_ptr);

    int ret;

    const float sampling_freq = frequency;

    // input matrix from the raw signal
    matrix_i16_t input_matrix(signal->total_length / config.axes, config.axes);
    if (!input_matrix.buffer) {
        EIDSP_ERR(EIDSP_OUT_OF_MEM);
    }

    signal->get_data(0, signal->total_length, (EIDSP_i16 *)&input_matrix.buffer[0]);

    // scale the signal
    ret = numpy::scale(&input_matrix, config.scale_axes);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: Failed to scale signal (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    // transpose the matrix so we have one row per axis (nifty!)
    ret = numpy::transpose(&input_matrix);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: Failed to transpose matrix (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    matrix_i16_t *edges_matrix_in = create_edges_matrix(config, sampling_freq);

    if(edges_matrix_in == NULL) {
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    // calculate how much room we need for the output matrix
    size_t output_matrix_cols = spectral::feature::calculate_spectral_buffer_size(
        true, config.spectral_peaks_count, edges_matrix_in->rows
    );
    // ei_printf("output_matrix_size %hux%zu\n", input_matrix.rows, output_matrix_cols);
    if (output_matrix->cols * output_matrix->rows != static_cast<uint32_t>(output_matrix_cols * config.axes)) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    output_matrix->cols = output_matrix_cols;
    output_matrix->rows = config.axes;

    spectral::filter_t filter_type;
    if (strcmp(config.filter_type, "low") == 0) {
        filter_type = spectral::filter_lowpass;
    }
    else if (strcmp(config.filter_type, "high") == 0) {
        filter_type = spectral::filter_highpass;
    }
    else {
        filter_type = spectral::filter_none;
    }

    ret = spectral::feature::spectral_analysis(output_matrix, &input_matrix,
        sampling_freq, filter_type, config.filter_cutoff, config.filter_order,
        config.fft_length, config.spectral_peaks_count, config.spectral_peaks_threshold, edges_matrix_in);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: Failed to calculate spectral features (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    // flatten again
    output_matrix->cols = config.axes * output_matrix_cols;
    output_matrix->rows = 1;

    return EIDSP_OK;
}

__attribute__((unused)) int extract_raw_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float frequency) {
    ei_dsp_config_raw_t config = *((ei_dsp_config_raw_t*)config_ptr);

    // input matrix from the raw signal
    matrix_t input_matrix(signal->total_length / config.axes, config.axes);
    if (!input_matrix.buffer) {
        EIDSP_ERR(EIDSP_OUT_OF_MEM);
    }
    signal->get_data(0, signal->total_length, input_matrix.buffer);

    // scale the signal
    int ret = numpy::scale(&input_matrix, config.scale_axes);
    if (ret != EIDSP_OK) {
        EIDSP_ERR(ret);
    }

    memcpy(output_matrix->buffer, input_matrix.buffer, signal->total_length * sizeof(float));

    return EIDSP_OK;
}

__attribute__((unused)) int extract_flatten_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float frequency) {
    ei_dsp_config_flatten_t config = *((ei_dsp_config_flatten_t*)config_ptr);

    uint32_t expected_matrix_size = 0;
    if (config.average) expected_matrix_size += config.axes;
    if (config.minimum) expected_matrix_size += config.axes;
    if (config.maximum) expected_matrix_size += config.axes;
    if (config.rms) expected_matrix_size += config.axes;
    if (config.stdev) expected_matrix_size += config.axes;
    if (config.skewness) expected_matrix_size += config.axes;
    if (config.kurtosis) expected_matrix_size += config.axes;

    if (output_matrix->rows * output_matrix->cols != expected_matrix_size) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    int ret;

    // input matrix from the raw signal
    matrix_t input_matrix(signal->total_length / config.axes, config.axes);
    if (!input_matrix.buffer) {
        EIDSP_ERR(EIDSP_OUT_OF_MEM);
    }
    signal->get_data(0, signal->total_length, input_matrix.buffer);

    // scale the signal
    ret = numpy::scale(&input_matrix, config.scale_axes);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: Failed to scale signal (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    // transpose the matrix so we have one row per axis (nifty!)
    ret = numpy::transpose(&input_matrix);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: Failed to transpose matrix (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    size_t out_matrix_ix = 0;

    for (size_t row = 0; row < input_matrix.rows; row++) {
        matrix_t row_matrix(1, input_matrix.cols, input_matrix.buffer + (row * input_matrix.cols));

        if (config.average) {
            float fbuffer;
            matrix_t out_matrix(1, 1, &fbuffer);
            numpy::mean(&row_matrix, &out_matrix);
            output_matrix->buffer[out_matrix_ix++] = out_matrix.buffer[0];
        }

        if (config.minimum) {
            float fbuffer;
            matrix_t out_matrix(1, 1, &fbuffer);
            numpy::min(&row_matrix, &out_matrix);
            output_matrix->buffer[out_matrix_ix++] = out_matrix.buffer[0];
        }

        if (config.maximum) {
            float fbuffer;
            matrix_t out_matrix(1, 1, &fbuffer);
            numpy::max(&row_matrix, &out_matrix);
            output_matrix->buffer[out_matrix_ix++] = out_matrix.buffer[0];
        }

        if (config.rms) {
            float fbuffer;
            matrix_t out_matrix(1, 1, &fbuffer);
            numpy::rms(&row_matrix, &out_matrix);
            output_matrix->buffer[out_matrix_ix++] = out_matrix.buffer[0];
        }

        if (config.stdev) {
            float fbuffer;
            matrix_t out_matrix(1, 1, &fbuffer);
            numpy::stdev(&row_matrix, &out_matrix);
            output_matrix->buffer[out_matrix_ix++] = out_matrix.buffer[0];
        }

        if (config.skewness) {
            float fbuffer;
            matrix_t out_matrix(1, 1, &fbuffer);
            numpy::skew(&row_matrix, &out_matrix);
            output_matrix->buffer[out_matrix_ix++] = out_matrix.buffer[0];
        }

        if (config.kurtosis) {
            float fbuffer;
            matrix_t out_matrix(1, 1, &fbuffer);
            numpy::kurtosis(&row_matrix, &out_matrix);
            output_matrix->buffer[out_matrix_ix++] = out_matrix.buffer[0];
        }
    }

    // flatten again
    output_matrix->cols = output_matrix->rows * output_matrix->cols;
    output_matrix->rows = 1;

    return EIDSP_OK;
}

static class speechpy::processing::preemphasis *preemphasis;
static int preemphasized_audio_signal_get_data(size_t offset, size_t length, float *out_ptr) {
    return preemphasis->get_data(offset, length, out_ptr);
}

__attribute__((unused)) int extract_mfcc_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float sampling_frequency) {
    ei_dsp_config_mfcc_t config = *((ei_dsp_config_mfcc_t*)config_ptr);

    if (config.axes != 1) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    if(config.implementation_version != 1 && config.implementation_version != 2) {
        EIDSP_ERR(EIDSP_BLOCK_VERSION_INCORRECT);
    }

    if (signal->total_length == 0) {
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    const uint32_t frequency = static_cast<uint32_t>(sampling_frequency);

    // preemphasis class to preprocess the audio...
    class speechpy::processing::preemphasis pre(signal, config.pre_shift, config.pre_cof, false);
    preemphasis = &pre;

    signal_t preemphasized_audio_signal;
    preemphasized_audio_signal.total_length = signal->total_length;
    preemphasized_audio_signal.get_data = &preemphasized_audio_signal_get_data;

    // calculate the size of the MFCC matrix
    matrix_size_t out_matrix_size =
        speechpy::feature::calculate_mfcc_buffer_size(
            signal->total_length, frequency, config.frame_length, config.frame_stride, config.num_cepstral, config.implementation_version);
    /* Only throw size mismatch error calculated buffer doesn't fit for continuous inferencing */
    if (out_matrix_size.rows * out_matrix_size.cols > output_matrix->rows * output_matrix->cols) {
        ei_printf("out_matrix = %dx%d\n", (int)output_matrix->rows, (int)output_matrix->cols);
        ei_printf("calculated size = %dx%d\n", (int)out_matrix_size.rows, (int)out_matrix_size.cols);
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    output_matrix->rows = out_matrix_size.rows;
    output_matrix->cols = out_matrix_size.cols;

    // and run the MFCC extraction (using 32 rather than 40 filters here to optimize speed on embedded)
    int ret = speechpy::feature::mfcc(output_matrix, &preemphasized_audio_signal,
        frequency, config.frame_length, config.frame_stride, config.num_cepstral, config.num_filters, config.fft_length,
        config.low_frequency, config.high_frequency, true, config.implementation_version);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: MFCC failed (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    // cepstral mean and variance normalization
    ret = speechpy::processing::cmvnw(output_matrix, config.win_size, true, false);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: cmvnw failed (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    output_matrix->cols = out_matrix_size.rows * out_matrix_size.cols;
    output_matrix->rows = 1;

    return EIDSP_OK;
}


static int extract_mfcc_run_slice(signal_t *signal, matrix_t *output_matrix, ei_dsp_config_mfcc_t *config, const float sampling_frequency, matrix_size_t *matrix_size_out, int implementation_version) {
    uint32_t frequency = (uint32_t)sampling_frequency;

    int x;

    // calculate the size of the spectrogram matrix
    matrix_size_t out_matrix_size =
        speechpy::feature::calculate_mfcc_buffer_size(
            signal->total_length, frequency, config->frame_length, config->frame_stride, config->num_cepstral,
            implementation_version);

    // we roll the output matrix back so we have room at the end...
    x = numpy::roll(output_matrix->buffer, output_matrix->rows * output_matrix->cols,
        -(out_matrix_size.rows * out_matrix_size.cols));
    if (x != EIDSP_OK) {
        EIDSP_ERR(x);
    }

    // slice in the output matrix to write to
    // the offset in the classification matrix here is always at the end
    size_t output_matrix_offset = (output_matrix->rows * output_matrix->cols) -
        (out_matrix_size.rows * out_matrix_size.cols);

    matrix_t output_matrix_slice(out_matrix_size.rows, out_matrix_size.cols, output_matrix->buffer + output_matrix_offset);

    // and run the MFCC extraction
    x = speechpy::feature::mfcc(&output_matrix_slice, signal,
        frequency, config->frame_length, config->frame_stride, config->num_cepstral, config->num_filters, config->fft_length,
        config->low_frequency, config->high_frequency, true, implementation_version);
    if (x != EIDSP_OK) {
        ei_printf("ERR: MFCC failed (%d)\n", x);
        EIDSP_ERR(x);
    }

    matrix_size_out->rows += out_matrix_size.rows;
    if (out_matrix_size.cols > 0) {
        matrix_size_out->cols = out_matrix_size.cols;
    }

    return EIDSP_OK;
}

__attribute__((unused)) int extract_mfcc_per_slice_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float sampling_frequency, matrix_size_t *matrix_size_out) {
#if defined(__cplusplus) && EI_C_LINKAGE == 1
    ei_printf("ERR: Continuous audio is not supported when EI_C_LINKAGE is defined\n");
    EIDSP_ERR(EIDSP_NOT_SUPPORTED);
#else

    ei_dsp_config_mfcc_t config = *((ei_dsp_config_mfcc_t*)config_ptr);

    if (config.axes != 1) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    if(config.implementation_version != 1 && config.implementation_version != 2) {
        EIDSP_ERR(EIDSP_BLOCK_VERSION_INCORRECT);
    }

    if (signal->total_length == 0) {
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    const uint32_t frequency = static_cast<uint32_t>(sampling_frequency);

    // preemphasis class to preprocess the audio...
    class speechpy::processing::preemphasis pre(signal, config.pre_shift, config.pre_cof, false);
    preemphasis = &pre;

    signal_t preemphasized_audio_signal;
    preemphasized_audio_signal.total_length = signal->total_length;
    preemphasized_audio_signal.get_data = &preemphasized_audio_signal_get_data;

    // Go from the time (e.g. 0.25 seconds to number of frames based on freq)
    const size_t frame_length_values = frequency * config.frame_length;
    const size_t frame_stride_values = frequency * config.frame_stride;
    const int frame_overlap_values = static_cast<int>(frame_length_values) - static_cast<int>(frame_stride_values);

    if (frame_overlap_values < 0) {
        ei_printf("ERR: frame_length (%f) cannot be lower than frame_stride (%f) for continuous classification\n",
            config.frame_length, config.frame_stride);
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    int x;

    // have current frame, but wrong size? then free
    if (ei_dsp_cont_current_frame && ei_dsp_cont_current_frame_size != frame_length_values) {
        ei_free(ei_dsp_cont_current_frame);
        ei_dsp_cont_current_frame = nullptr;
    }

    int implementation_version = config.implementation_version;

    // this is the offset in the signal from which we'll work
    size_t offset_in_signal = 0;

    if (!ei_dsp_cont_current_frame) {
        ei_dsp_cont_current_frame = (float*)ei_calloc(frame_length_values * sizeof(float), 1);
        if (!ei_dsp_cont_current_frame) {
            EIDSP_ERR(EIDSP_OUT_OF_MEM);
        }
        ei_dsp_cont_current_frame_size = frame_length_values;
        ei_dsp_cont_current_frame_ix = 0;
    }


    if ((frame_length_values) > preemphasized_audio_signal.total_length  + ei_dsp_cont_current_frame_ix) {
        ei_printf("ERR: frame_length (%d) cannot be larger than signal's total length (%d) for continuous classification\n",
            (int)frame_length_values, (int)preemphasized_audio_signal.total_length  + ei_dsp_cont_current_frame_ix);
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    matrix_size_out->rows = 0;
    matrix_size_out->cols = 0;

    // for continuous use v2 stack frame calculations
    if (implementation_version == 1) {
        implementation_version = 2;
    }

    if (ei_dsp_cont_current_frame_ix > (int)ei_dsp_cont_current_frame_size) {
        ei_printf("ERR: ei_dsp_cont_current_frame_ix is larger than frame size (ix=%d size=%d)\n",
            ei_dsp_cont_current_frame_ix, (int)ei_dsp_cont_current_frame_size);
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    // if we still have some code from previous run
    while (ei_dsp_cont_current_frame_ix > 0) {
        // then from the current frame we need to read `frame_length_values - ei_dsp_cont_current_frame_ix`
        // starting at offset 0
        x = preemphasized_audio_signal.get_data(0, frame_length_values - ei_dsp_cont_current_frame_ix, ei_dsp_cont_current_frame + ei_dsp_cont_current_frame_ix);
        if (x != EIDSP_OK) {
            EIDSP_ERR(x);
        }

        // now ei_dsp_cont_current_frame is complete
        signal_t frame_signal;
        x = numpy::signal_from_buffer(ei_dsp_cont_current_frame, frame_length_values, &frame_signal);
        if (x != EIDSP_OK) {
            EIDSP_ERR(x);
        }

        x = extract_mfcc_run_slice(&frame_signal, output_matrix, &config, sampling_frequency, matrix_size_out, implementation_version);
        if (x != EIDSP_OK) {
            EIDSP_ERR(x);
        }

        // if there's overlap between frames we roll through
        if (frame_stride_values > 0) {
            numpy::roll(ei_dsp_cont_current_frame, frame_length_values, -frame_stride_values);
        }

        ei_dsp_cont_current_frame_ix -= frame_stride_values;
    }

    if (ei_dsp_cont_current_frame_ix < 0) {
        offset_in_signal = -ei_dsp_cont_current_frame_ix;
        ei_dsp_cont_current_frame_ix = 0;
    }

    if (offset_in_signal >= signal->total_length) {
        offset_in_signal -= signal->total_length;
        return EIDSP_OK;
    }

    // now... we need to discard part of the signal...
    SignalWithRange signal_with_range(&preemphasized_audio_signal, offset_in_signal, signal->total_length);

    signal_t *range_signal = signal_with_range.get_signal();
    size_t range_signal_orig_length = range_signal->total_length;

    // then we'll just go through normal processing of the signal:
    x = extract_mfcc_run_slice(range_signal, output_matrix, &config, sampling_frequency, matrix_size_out, implementation_version);
    if (x != EIDSP_OK) {
        EIDSP_ERR(x);
    }

    // Make sure v1 model are reset to the original length;
    range_signal->total_length = range_signal_orig_length;

    // update offset
    int length_of_signal_used = speechpy::processing::calculate_signal_used(range_signal->total_length, sampling_frequency,
        config.frame_length, config.frame_stride, false, implementation_version);
    offset_in_signal += length_of_signal_used;

    // see what's left?
    int bytes_left_end_of_frame = signal->total_length - offset_in_signal;
    bytes_left_end_of_frame += frame_overlap_values;

    if (bytes_left_end_of_frame > 0) {
        // then read that into the ei_dsp_cont_current_frame buffer
        x = preemphasized_audio_signal.get_data(
            (preemphasized_audio_signal.total_length - bytes_left_end_of_frame),
            bytes_left_end_of_frame,
            ei_dsp_cont_current_frame);
        if (x != EIDSP_OK) {
            EIDSP_ERR(x);
        }
    }

    ei_dsp_cont_current_frame_ix = bytes_left_end_of_frame;

    preemphasis = nullptr;

    return EIDSP_OK;
#endif
}

__attribute__((unused)) int extract_spectrogram_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float sampling_frequency) {
    ei_dsp_config_spectrogram_t config = *((ei_dsp_config_spectrogram_t*)config_ptr);

    if (config.axes != 1) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    if (signal->total_length == 0) {
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    const uint32_t frequency = static_cast<uint32_t>(sampling_frequency);

    // calculate the size of the MFE matrix
    matrix_size_t out_matrix_size =
        speechpy::feature::calculate_mfe_buffer_size(
            signal->total_length, frequency, config.frame_length, config.frame_stride, config.fft_length / 2 + 1,
            config.implementation_version);
    /* Only throw size mismatch error calculated buffer doesn't fit for continuous inferencing */
    if (out_matrix_size.rows * out_matrix_size.cols > output_matrix->rows * output_matrix->cols) {
        ei_printf("out_matrix = %dx%d\n", (int)output_matrix->rows, (int)output_matrix->cols);
        ei_printf("calculated size = %dx%d\n", (int)out_matrix_size.rows, (int)out_matrix_size.cols);
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    output_matrix->rows = out_matrix_size.rows;
    output_matrix->cols = out_matrix_size.cols;

    // and run the MFE extraction
    EI_DSP_MATRIX(energy_matrix, output_matrix->rows, 1);
    if (!energy_matrix.buffer) {
        EIDSP_ERR(EIDSP_OUT_OF_MEM);
    }

    int ret = speechpy::feature::spectrogram(output_matrix, signal,
        frequency, config.frame_length, config.frame_stride, config.fft_length, config.implementation_version);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: Spectrogram failed (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    if (config.implementation_version < 3) {
        ret = numpy::normalize(output_matrix);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(ret);
        }
    }
    else {
        // normalization
        ret = speechpy::processing::spectrogram_normalization(output_matrix, config.noise_floor_db);
        if (ret != EIDSP_OK) {
            ei_printf("ERR: normalization failed (%d)\n", ret);
            EIDSP_ERR(ret);
        }
    }

    output_matrix->cols = out_matrix_size.rows * out_matrix_size.cols;
    output_matrix->rows = 1;

    return EIDSP_OK;
}


static int extract_spectrogram_run_slice(signal_t *signal, matrix_t *output_matrix, ei_dsp_config_spectrogram_t *config, const float sampling_frequency, matrix_size_t *matrix_size_out) {
    uint32_t frequency = (uint32_t)sampling_frequency;

    int x;

    // calculate the size of the spectrogram matrix
    matrix_size_t out_matrix_size =
        speechpy::feature::calculate_mfe_buffer_size(
            signal->total_length, frequency, config->frame_length, config->frame_stride, config->fft_length / 2 + 1,
            config->implementation_version);

    // we roll the output matrix back so we have room at the end...
    x = numpy::roll(output_matrix->buffer, output_matrix->rows * output_matrix->cols,
        -(out_matrix_size.rows * out_matrix_size.cols));
    if (x != EIDSP_OK) {
        if (preemphasis) {
            delete preemphasis;
        }
        EIDSP_ERR(x);
    }

    // slice in the output matrix to write to
    // the offset in the classification matrix here is always at the end
    size_t output_matrix_offset = (output_matrix->rows * output_matrix->cols) -
        (out_matrix_size.rows * out_matrix_size.cols);

    matrix_t output_matrix_slice(out_matrix_size.rows, out_matrix_size.cols, output_matrix->buffer + output_matrix_offset);

    // and run the spectrogram extraction
    int ret = speechpy::feature::spectrogram(&output_matrix_slice, signal,
        frequency, config->frame_length, config->frame_stride, config->fft_length, config->implementation_version);

    if (ret != EIDSP_OK) {
        ei_printf("ERR: Spectrogram failed (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    matrix_size_out->rows += out_matrix_size.rows;
    if (out_matrix_size.cols > 0) {
        matrix_size_out->cols = out_matrix_size.cols;
    }

    return EIDSP_OK;
}

__attribute__((unused)) int extract_spectrogram_per_slice_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float sampling_frequency, matrix_size_t *matrix_size_out) {
#if defined(__cplusplus) && EI_C_LINKAGE == 1
    ei_printf("ERR: Continuous audio is not supported when EI_C_LINKAGE is defined\n");
    EIDSP_ERR(EIDSP_NOT_SUPPORTED);
#else

    ei_dsp_config_spectrogram_t config = *((ei_dsp_config_spectrogram_t*)config_ptr);

    static bool first_run = false;

    if (config.axes != 1) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    if (signal->total_length == 0) {
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    const uint32_t frequency = static_cast<uint32_t>(sampling_frequency);

    /* Fake an extra frame_length for stack frames calculations. There, 1 frame_length is always
    subtracted and there for never used. But skip the first slice to fit the feature_matrix
    buffer */
    if(config.implementation_version < 2) {

        if (first_run == true) {
            signal->total_length += (size_t)(config.frame_length * (float)frequency);
        }

        first_run = true;
    }

    // Go from the time (e.g. 0.25 seconds to number of frames based on freq)
    const size_t frame_length_values = frequency * config.frame_length;
    const size_t frame_stride_values = frequency * config.frame_stride;
    const int frame_overlap_values = static_cast<int>(frame_length_values) - static_cast<int>(frame_stride_values);

    if (frame_overlap_values < 0) {
        ei_printf("ERR: frame_length (%f) cannot be lower than frame_stride (%f) for continuous classification\n",
            config.frame_length, config.frame_stride);
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    if (frame_length_values > signal->total_length) {
        ei_printf("ERR: frame_length (%d) cannot be larger than signal's total length (%d) for continuous classification\n",
            (int)frame_length_values, (int)signal->total_length);
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    int x;

    // have current frame, but wrong size? then free
    if (ei_dsp_cont_current_frame && ei_dsp_cont_current_frame_size != frame_length_values) {
        ei_free(ei_dsp_cont_current_frame);
        ei_dsp_cont_current_frame = nullptr;
    }

    if (!ei_dsp_cont_current_frame) {
        ei_dsp_cont_current_frame = (float*)ei_calloc(frame_length_values * sizeof(float), 1);
        if (!ei_dsp_cont_current_frame) {
            EIDSP_ERR(EIDSP_OUT_OF_MEM);
        }
        ei_dsp_cont_current_frame_size = frame_length_values;
        ei_dsp_cont_current_frame_ix = 0;
    }

    matrix_size_out->rows = 0;
    matrix_size_out->cols = 0;

    // this is the offset in the signal from which we'll work
    size_t offset_in_signal = 0;

    if (ei_dsp_cont_current_frame_ix > (int)ei_dsp_cont_current_frame_size) {
        ei_printf("ERR: ei_dsp_cont_current_frame_ix is larger than frame size\n");
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    // if we still have some code from previous run
    while (ei_dsp_cont_current_frame_ix > 0) {
        // then from the current frame we need to read `frame_length_values - ei_dsp_cont_current_frame_ix`
        // starting at offset 0
        x = signal->get_data(0, frame_length_values - ei_dsp_cont_current_frame_ix, ei_dsp_cont_current_frame + ei_dsp_cont_current_frame_ix);
        if (x != EIDSP_OK) {
            EIDSP_ERR(x);
        }

        // now ei_dsp_cont_current_frame is complete
        signal_t frame_signal;
        x = numpy::signal_from_buffer(ei_dsp_cont_current_frame, frame_length_values, &frame_signal);
        if (x != EIDSP_OK) {
            EIDSP_ERR(x);
        }

        x = extract_spectrogram_run_slice(&frame_signal, output_matrix, &config, sampling_frequency, matrix_size_out);
        if (x != EIDSP_OK) {
            EIDSP_ERR(x);
        }

        // if there's overlap between frames we roll through
        if (frame_stride_values > 0) {
            numpy::roll(ei_dsp_cont_current_frame, frame_length_values, -frame_stride_values);
        }

        ei_dsp_cont_current_frame_ix -= frame_stride_values;
    }

    if (ei_dsp_cont_current_frame_ix < 0) {
        offset_in_signal = -ei_dsp_cont_current_frame_ix;
        ei_dsp_cont_current_frame_ix = 0;
    }

    if (offset_in_signal >= signal->total_length) {
        offset_in_signal -= signal->total_length;
        return EIDSP_OK;
    }

    // now... we need to discard part of the signal...
    SignalWithRange signal_with_range(signal, offset_in_signal, signal->total_length);

    signal_t *range_signal = signal_with_range.get_signal();
    size_t range_signal_orig_length = range_signal->total_length;

    // then we'll just go through normal processing of the signal:
    x = extract_spectrogram_run_slice(range_signal, output_matrix, &config, sampling_frequency, matrix_size_out);
    if (x != EIDSP_OK) {
        EIDSP_ERR(x);
    }

    // update offset
    int length_of_signal_used = speechpy::processing::calculate_signal_used(range_signal->total_length, sampling_frequency,
        config.frame_length, config.frame_stride, false, config.implementation_version);
    offset_in_signal += length_of_signal_used;

    // not sure why this is being manipulated...
    range_signal->total_length = range_signal_orig_length;

    // see what's left?
    int bytes_left_end_of_frame = signal->total_length - offset_in_signal;
    bytes_left_end_of_frame += frame_overlap_values;

    if (bytes_left_end_of_frame > 0) {
        // then read that into the ei_dsp_cont_current_frame buffer
        x = signal->get_data(
            (signal->total_length - bytes_left_end_of_frame),
            bytes_left_end_of_frame,
            ei_dsp_cont_current_frame);
        if (x != EIDSP_OK) {
            EIDSP_ERR(x);
        }
    }

    ei_dsp_cont_current_frame_ix = bytes_left_end_of_frame;

    if (config.implementation_version < 2) {
        if (first_run == true) {
            signal->total_length -= (size_t)(config.frame_length * (float)frequency);
        }
    }

    return EIDSP_OK;
#endif
}


__attribute__((unused)) int extract_mfe_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float sampling_frequency) {
    ei_dsp_config_mfe_t config = *((ei_dsp_config_mfe_t*)config_ptr);

    if (config.axes != 1) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    if (signal->total_length == 0) {
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    const uint32_t frequency = static_cast<uint32_t>(sampling_frequency);

    signal_t preemphasized_audio_signal;

    // before version 3 we did not have preemphasis
    if (config.implementation_version < 3) {
        preemphasis = nullptr;

        preemphasized_audio_signal.total_length = signal->total_length;
        preemphasized_audio_signal.get_data = signal->get_data;
    }
    else {
        // preemphasis class to preprocess the audio...
        class speechpy::processing::preemphasis *pre = new class speechpy::processing::preemphasis(signal, 1, 0.98f, true);
        preemphasis = pre;

        preemphasized_audio_signal.total_length = signal->total_length;
        preemphasized_audio_signal.get_data = &preemphasized_audio_signal_get_data;
    }

    // calculate the size of the MFE matrix
    matrix_size_t out_matrix_size =
        speechpy::feature::calculate_mfe_buffer_size(
            preemphasized_audio_signal.total_length, frequency, config.frame_length, config.frame_stride, config.num_filters,
            config.implementation_version);
    /* Only throw size mismatch error calculated buffer doesn't fit for continuous inferencing */
    if (out_matrix_size.rows * out_matrix_size.cols > output_matrix->rows * output_matrix->cols) {
        ei_printf("out_matrix = %dx%d\n", (int)output_matrix->rows, (int)output_matrix->cols);
        ei_printf("calculated size = %dx%d\n", (int)out_matrix_size.rows, (int)out_matrix_size.cols);
        if (preemphasis) {
            delete preemphasis;
        }
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    output_matrix->rows = out_matrix_size.rows;
    output_matrix->cols = out_matrix_size.cols;

    // and run the MFE extraction
    EI_DSP_MATRIX(energy_matrix, output_matrix->rows, 1);
    if (!energy_matrix.buffer) {
        if (preemphasis) {
            delete preemphasis;
        }
        EIDSP_ERR(EIDSP_OUT_OF_MEM);
    }

    int ret = speechpy::feature::mfe(output_matrix, &energy_matrix, &preemphasized_audio_signal,
        frequency, config.frame_length, config.frame_stride, config.num_filters, config.fft_length,
        config.low_frequency, config.high_frequency, config.implementation_version);
    if (preemphasis) {
        delete preemphasis;
    }
    if (ret != EIDSP_OK) {
        ei_printf("ERR: MFE failed (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    if (config.implementation_version < 3) {
        // cepstral mean and variance normalization
        ret = speechpy::processing::cmvnw(output_matrix, config.win_size, false, true);
        if (ret != EIDSP_OK) {
            ei_printf("ERR: cmvnw failed (%d)\n", ret);
            EIDSP_ERR(ret);
        }
    }
    else {
        // normalization
        ret = speechpy::processing::mfe_normalization(output_matrix, config.noise_floor_db);
        if (ret != EIDSP_OK) {
            ei_printf("ERR: normalization failed (%d)\n", ret);
            EIDSP_ERR(ret);
        }
    }

    output_matrix->cols = out_matrix_size.rows * out_matrix_size.cols;
    output_matrix->rows = 1;

    return EIDSP_OK;
}

static int extract_mfe_run_slice(signal_t *signal, matrix_t *output_matrix, ei_dsp_config_mfe_t *config, const float sampling_frequency, matrix_size_t *matrix_size_out) {
    uint32_t frequency = (uint32_t)sampling_frequency;

    int x;

    // calculate the size of the spectrogram matrix
    matrix_size_t out_matrix_size =
        speechpy::feature::calculate_mfe_buffer_size(
            signal->total_length, frequency, config->frame_length, config->frame_stride, config->num_filters,
            config->implementation_version);

    // we roll the output matrix back so we have room at the end...
    x = numpy::roll(output_matrix->buffer, output_matrix->rows * output_matrix->cols,
        -(out_matrix_size.rows * out_matrix_size.cols));
    if (x != EIDSP_OK) {
        EIDSP_ERR(x);
    }

    // slice in the output matrix to write to
    // the offset in the classification matrix here is always at the end
    size_t output_matrix_offset = (output_matrix->rows * output_matrix->cols) -
        (out_matrix_size.rows * out_matrix_size.cols);

    matrix_t output_matrix_slice(out_matrix_size.rows, out_matrix_size.cols, output_matrix->buffer + output_matrix_offset);

    // energy matrix
    EI_DSP_MATRIX(energy_matrix, out_matrix_size.rows, 1);
    if (!energy_matrix.buffer) {
        EIDSP_ERR(EIDSP_OUT_OF_MEM);
    }

    // and run the MFE extraction
    x = speechpy::feature::mfe(&output_matrix_slice, &energy_matrix, signal,
        frequency, config->frame_length, config->frame_stride, config->num_filters, config->fft_length,
        config->low_frequency, config->high_frequency, config->implementation_version);
    if (x != EIDSP_OK) {
        ei_printf("ERR: MFE failed (%d)\n", x);
        EIDSP_ERR(x);
    }

    matrix_size_out->rows += out_matrix_size.rows;
    if (out_matrix_size.cols > 0) {
        matrix_size_out->cols = out_matrix_size.cols;
    }

    return EIDSP_OK;
}

__attribute__((unused)) int extract_mfe_per_slice_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float sampling_frequency, matrix_size_t *matrix_size_out) {
#if defined(__cplusplus) && EI_C_LINKAGE == 1
    ei_printf("ERR: Continuous audio is not supported when EI_C_LINKAGE is defined\n");
    EIDSP_ERR(EIDSP_NOT_SUPPORTED);
#else

    ei_dsp_config_mfe_t config = *((ei_dsp_config_mfe_t*)config_ptr);

    // signal is already the right size,
    // output matrix is not the right size, but we can start writing at offset 0 and then it's OK too

    static bool first_run = false;

    if (config.axes != 1) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    if (signal->total_length == 0) {
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    const uint32_t frequency = static_cast<uint32_t>(sampling_frequency);

    // Fake an extra frame_length for stack frames calculations. There, 1 frame_length is always
    // subtracted and there for never used. But skip the first slice to fit the feature_matrix
    // buffer
    if (config.implementation_version == 1) {
        if (first_run == true) {
            signal->total_length += (size_t)(config.frame_length * (float)frequency);
        }

        first_run = true;
    }

    // ok all setup, let's construct the signal (with preemphasis for impl version >3)
    signal_t preemphasized_audio_signal;

   // before version 3 we did not have preemphasis
    if (config.implementation_version < 3) {
        preemphasis = nullptr;
        preemphasized_audio_signal.total_length = signal->total_length;
        preemphasized_audio_signal.get_data = signal->get_data;
    }
    else {
        // preemphasis class to preprocess the audio...
        class speechpy::processing::preemphasis *pre = new class speechpy::processing::preemphasis(signal, 1, 0.98f, true);
        preemphasis = pre;
        preemphasized_audio_signal.total_length = signal->total_length;
        preemphasized_audio_signal.get_data = &preemphasized_audio_signal_get_data;
    }

    // Go from the time (e.g. 0.25 seconds to number of frames based on freq)
    const size_t frame_length_values = frequency * config.frame_length;
    const size_t frame_stride_values = frequency * config.frame_stride;
    const int frame_overlap_values = static_cast<int>(frame_length_values) - static_cast<int>(frame_stride_values);

    if (frame_overlap_values < 0) {
        ei_printf("ERR: frame_length (%f) cannot be lower than frame_stride (%f) for continuous classification\n",
            config.frame_length, config.frame_stride);
        if (preemphasis) {
            delete preemphasis;
        }
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    if (frame_length_values > preemphasized_audio_signal.total_length) {
        ei_printf("ERR: frame_length (%d) cannot be larger than signal's total length (%d) for continuous classification\n",
            (int)frame_length_values, (int)preemphasized_audio_signal.total_length);
        if (preemphasis) {
            delete preemphasis;
        }
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    int x;

    // have current frame, but wrong size? then free
    if (ei_dsp_cont_current_frame && ei_dsp_cont_current_frame_size != frame_length_values) {
        ei_free(ei_dsp_cont_current_frame);
        ei_dsp_cont_current_frame = nullptr;
    }

    if (!ei_dsp_cont_current_frame) {
        ei_dsp_cont_current_frame = (float*)ei_calloc(frame_length_values * sizeof(float), 1);
        if (!ei_dsp_cont_current_frame) {
            if (preemphasis) {
                delete preemphasis;
            }
            EIDSP_ERR(EIDSP_OUT_OF_MEM);
        }
        ei_dsp_cont_current_frame_size = frame_length_values;
        ei_dsp_cont_current_frame_ix = 0;
    }

    matrix_size_out->rows = 0;
    matrix_size_out->cols = 0;

    // this is the offset in the signal from which we'll work
    size_t offset_in_signal = 0;

    if (ei_dsp_cont_current_frame_ix > (int)ei_dsp_cont_current_frame_size) {
        ei_printf("ERR: ei_dsp_cont_current_frame_ix is larger than frame size\n");
        if (preemphasis) {
            delete preemphasis;
        }
        EIDSP_ERR(EIDSP_PARAMETER_INVALID);
    }

    // if we still have some code from previous run
    while (ei_dsp_cont_current_frame_ix > 0) {
        // then from the current frame we need to read `frame_length_values - ei_dsp_cont_current_frame_ix`
        // starting at offset 0
        x = preemphasized_audio_signal.get_data(0, frame_length_values - ei_dsp_cont_current_frame_ix, ei_dsp_cont_current_frame + ei_dsp_cont_current_frame_ix);
        if (x != EIDSP_OK) {
            if (preemphasis) {
                delete preemphasis;
            }
            EIDSP_ERR(x);
        }

        // now ei_dsp_cont_current_frame is complete
        signal_t frame_signal;
        x = numpy::signal_from_buffer(ei_dsp_cont_current_frame, frame_length_values, &frame_signal);
        if (x != EIDSP_OK) {
            if (preemphasis) {
                delete preemphasis;
            }
            EIDSP_ERR(x);
        }

        x = extract_mfe_run_slice(&frame_signal, output_matrix, &config, sampling_frequency, matrix_size_out);
        if (x != EIDSP_OK) {
            if (preemphasis) {
                delete preemphasis;
            }
            EIDSP_ERR(x);
        }

        // if there's overlap between frames we roll through
        if (frame_stride_values > 0) {
            numpy::roll(ei_dsp_cont_current_frame, frame_length_values, -frame_stride_values);
        }

        ei_dsp_cont_current_frame_ix -= frame_stride_values;
    }

    if (ei_dsp_cont_current_frame_ix < 0) {
        offset_in_signal = -ei_dsp_cont_current_frame_ix;
        ei_dsp_cont_current_frame_ix = 0;
    }

    if (offset_in_signal >= signal->total_length) {
        if (preemphasis) {
            delete preemphasis;
        }
        offset_in_signal -= signal->total_length;
        return EIDSP_OK;
    }

    // now... we need to discard part of the signal...
    SignalWithRange signal_with_range(&preemphasized_audio_signal, offset_in_signal, signal->total_length);

    signal_t *range_signal = signal_with_range.get_signal();
    size_t range_signal_orig_length = range_signal->total_length;

    // then we'll just go through normal processing of the signal:
    x = extract_mfe_run_slice(range_signal, output_matrix, &config, sampling_frequency, matrix_size_out);
    if (x != EIDSP_OK) {
        if (preemphasis) {
            delete preemphasis;
        }
        EIDSP_ERR(x);
    }

    // update offset
    int length_of_signal_used = speechpy::processing::calculate_signal_used(range_signal->total_length, sampling_frequency,
        config.frame_length, config.frame_stride, false, config.implementation_version);
    offset_in_signal += length_of_signal_used;

    // not sure why this is being manipulated...
    range_signal->total_length = range_signal_orig_length;

    // see what's left?
    int bytes_left_end_of_frame = signal->total_length - offset_in_signal;
    bytes_left_end_of_frame += frame_overlap_values;

    if (bytes_left_end_of_frame > 0) {
        // then read that into the ei_dsp_cont_current_frame buffer
        x = preemphasized_audio_signal.get_data(
            (preemphasized_audio_signal.total_length - bytes_left_end_of_frame),
            bytes_left_end_of_frame,
            ei_dsp_cont_current_frame);
        if (x != EIDSP_OK) {
            if (preemphasis) {
                delete preemphasis;
            }
            EIDSP_ERR(x);
        }
    }

    ei_dsp_cont_current_frame_ix = bytes_left_end_of_frame;


    if (config.implementation_version == 1) {
        if (first_run == true) {
            signal->total_length -= (size_t)(config.frame_length * (float)frequency);
        }
    }

    if (preemphasis) {
        delete preemphasis;
    }

    return EIDSP_OK;
#endif
}

__attribute__((unused)) int extract_image_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float frequency) {
    ei_dsp_config_image_t config = *((ei_dsp_config_image_t*)config_ptr);

    int16_t channel_count = strcmp(config.channels, "Grayscale") == 0 ? 1 : 3;

    if (output_matrix->rows * output_matrix->cols != static_cast<uint32_t>(EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT * channel_count)) {
        ei_printf("out_matrix = %hu items\n", output_matrix->rows, output_matrix->cols);
        ei_printf("calculated size = %hu items\n", static_cast<uint32_t>(EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT * channel_count));
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    size_t output_ix = 0;

#if defined(EI_DSP_IMAGE_BUFFER_STATIC_SIZE)
    const size_t page_size = EI_DSP_IMAGE_BUFFER_STATIC_SIZE;
#else
    const size_t page_size = 1024;
#endif

    // buffered read from the signal
    size_t bytes_left = signal->total_length;
    for (size_t ix = 0; ix < signal->total_length; ix += page_size) {
        size_t elements_to_read = bytes_left > page_size ? page_size : bytes_left;

#if defined(EI_DSP_IMAGE_BUFFER_STATIC_SIZE)
        matrix_t input_matrix(elements_to_read, config.axes, ei_dsp_image_buffer);
#else
        matrix_t input_matrix(elements_to_read, config.axes);
#endif
        if (!input_matrix.buffer) {
            EIDSP_ERR(EIDSP_OUT_OF_MEM);
        }
        signal->get_data(ix, elements_to_read, input_matrix.buffer);

        for (size_t jx = 0; jx < elements_to_read; jx++) {
            uint32_t pixel = static_cast<uint32_t>(input_matrix.buffer[jx]);

            // rgb to 0..1
            float r = static_cast<float>(pixel >> 16 & 0xff) / 255.0f;
            float g = static_cast<float>(pixel >> 8 & 0xff) / 255.0f;
            float b = static_cast<float>(pixel & 0xff) / 255.0f;

            if (channel_count == 3) {
                output_matrix->buffer[output_ix++] = r;
                output_matrix->buffer[output_ix++] = g;
                output_matrix->buffer[output_ix++] = b;
            }
            else {
                // ITU-R 601-2 luma transform
                // see: https://pillow.readthedocs.io/en/stable/reference/Image.html#PIL.Image.Image.convert
                float v = (0.299f * r) + (0.587f * g) + (0.114f * b);
                output_matrix->buffer[output_ix++] = v;
            }
        }

        bytes_left -= elements_to_read;
    }

    return EIDSP_OK;
}

#if EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1

__attribute__((unused)) int extract_image_features_quantized(signal_t *signal, matrix_i8_t *output_matrix, void *config_ptr, const float frequency) {
    ei_dsp_config_image_t config = *((ei_dsp_config_image_t*)config_ptr);

    int16_t channel_count = strcmp(config.channels, "Grayscale") == 0 ? 1 : 3;

    if (output_matrix->rows * output_matrix->cols != static_cast<uint32_t>(EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT * channel_count)) {
        ei_printf("out_matrix = %hu items\n", output_matrix->rows, output_matrix->cols);
        ei_printf("calculated size = %hu items\n", static_cast<uint32_t>(EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT * channel_count));
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    size_t output_ix = 0;

    const int32_t iRedToGray = (int32_t)(0.299f * 65536.0f);
    const int32_t iGreenToGray = (int32_t)(0.587f * 65536.0f);
    const int32_t iBlueToGray = (int32_t)(0.114f * 65536.0f);

#if defined(EI_DSP_IMAGE_BUFFER_STATIC_SIZE)
    const size_t page_size = EI_DSP_IMAGE_BUFFER_STATIC_SIZE;
#else
    const size_t page_size = 1024;
#endif

    // buffered read from the signal
    size_t bytes_left = signal->total_length;
    for (size_t ix = 0; ix < signal->total_length; ix += page_size) {
        size_t elements_to_read = bytes_left > page_size ? page_size : bytes_left;

#if defined(EI_DSP_IMAGE_BUFFER_STATIC_SIZE)
        matrix_t input_matrix(elements_to_read, config.axes, ei_dsp_image_buffer);
#else
        matrix_t input_matrix(elements_to_read, config.axes);
#endif
        if (!input_matrix.buffer) {
            EIDSP_ERR(EIDSP_OUT_OF_MEM);
        }
        signal->get_data(ix, elements_to_read, input_matrix.buffer);

        for (size_t jx = 0; jx < elements_to_read; jx++) {
            uint32_t pixel = static_cast<uint32_t>(input_matrix.buffer[jx]);

            if (channel_count == 3) {
                // fast code path
                if (EI_CLASSIFIER_TFLITE_INPUT_SCALE == 0.003921568859368563f && EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT == -128) {
                    int32_t r = static_cast<int32_t>(pixel >> 16 & 0xff);
                    int32_t g = static_cast<int32_t>(pixel >> 8 & 0xff);
                    int32_t b = static_cast<int32_t>(pixel & 0xff);

                    output_matrix->buffer[output_ix++] = static_cast<int8_t>(r + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
                    output_matrix->buffer[output_ix++] = static_cast<int8_t>(g + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
                    output_matrix->buffer[output_ix++] = static_cast<int8_t>(b + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
                }
                // slow code path
                else {
                    float r = static_cast<float>(pixel >> 16 & 0xff) / 255.0f;
                    float g = static_cast<float>(pixel >> 8 & 0xff) / 255.0f;
                    float b = static_cast<float>(pixel & 0xff) / 255.0f;

                    output_matrix->buffer[output_ix++] = static_cast<int8_t>(round(r / EI_CLASSIFIER_TFLITE_INPUT_SCALE) + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
                    output_matrix->buffer[output_ix++] = static_cast<int8_t>(round(g / EI_CLASSIFIER_TFLITE_INPUT_SCALE) + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
                    output_matrix->buffer[output_ix++] = static_cast<int8_t>(round(b / EI_CLASSIFIER_TFLITE_INPUT_SCALE) + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
                }
            }
            else {
                // fast code path
                if (EI_CLASSIFIER_TFLITE_INPUT_SCALE == 0.003921568859368563f && EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT == -128) {
                    int32_t r = static_cast<int32_t>(pixel >> 16 & 0xff);
                    int32_t g = static_cast<int32_t>(pixel >> 8 & 0xff);
                    int32_t b = static_cast<int32_t>(pixel & 0xff);

                    // ITU-R 601-2 luma transform
                    // see: https://pillow.readthedocs.io/en/stable/reference/Image.html#PIL.Image.Image.convert
                    int32_t gray = (iRedToGray * r) + (iGreenToGray * g) + (iBlueToGray * b);
                    gray >>= 16; // scale down to int8_t
                    gray += EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT;
                    if (gray < - 128) gray = -128;
                    else if (gray > 127) gray = 127;
                    output_matrix->buffer[output_ix++] = static_cast<int8_t>(gray);
                }
                // slow code path
                else {
                    float r = static_cast<float>(pixel >> 16 & 0xff) / 255.0f;
                    float g = static_cast<float>(pixel >> 8 & 0xff) / 255.0f;
                    float b = static_cast<float>(pixel & 0xff) / 255.0f;

                    // ITU-R 601-2 luma transform
                    // see: https://pillow.readthedocs.io/en/stable/reference/Image.html#PIL.Image.Image.convert
                    float v = (0.299f * r) + (0.587f * g) + (0.114f * b);
                    output_matrix->buffer[output_ix++] = static_cast<int8_t>(round(v / EI_CLASSIFIER_TFLITE_INPUT_SCALE) + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
                }
            }
        }

        bytes_left -= elements_to_read;
    }

    return EIDSP_OK;
}
#endif // EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1

/**
 * Clear all state regarding continuous audio. Invoke this function after continuous audio loop ends.
 */
__attribute__((unused)) int ei_dsp_clear_continuous_audio_state() {
    if (ei_dsp_cont_current_frame) {
        ei_free(ei_dsp_cont_current_frame);
    }

    ei_dsp_cont_current_frame = nullptr;
    ei_dsp_cont_current_frame_size = 0;
    ei_dsp_cont_current_frame_ix = 0;

    return EIDSP_OK;
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _EDGE_IMPULSE_RUN_DSP_H_
