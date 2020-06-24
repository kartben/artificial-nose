/* Edge Impulse inferencing library
 * Copyright (c) 2020 EdgeImpulse Inc.
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

extern void ei_printf(const char *format, ...);

#ifdef __cplusplus
namespace {
#endif // __cplusplus

using namespace ei;

__attribute__((unused)) int extract_spectral_analysis_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr) {
    ei_dsp_config_spectral_analysis_t config = *((ei_dsp_config_spectral_analysis_t*)config_ptr);

    int ret;

    const float sampling_freq = EI_CLASSIFIER_FREQUENCY;

    // input matrix from the raw signal
    matrix_t input_matrix(signal->total_length / config.axes, config.axes);
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

    // the spectral edges that we want to calculate (@todo, take this from the config)
    float edges[] = { 0.1, 0.5, 1.0, 2.0, 5.0 };
    matrix_t edges_matrix_in(sizeof(edges) / sizeof(edges[0]), 1, edges);

    // calculate how much room we need for the output matrix
    size_t output_matrix_cols = spectral::feature::calculate_spectral_buffer_size(
        true, config.spectral_peaks_count, edges_matrix_in.rows
    );
    // ei_printf("output_matrix_size %hux%zu\n", input_matrix.rows, output_matrix_cols);
    if (output_matrix->cols * output_matrix->rows != output_matrix_cols * config.axes) {
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

__attribute__((unused)) int extract_raw_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr) {
    ei_dsp_config_raw_t config = *((ei_dsp_config_raw_t*)config_ptr);

    // input matrix from the raw signal
    matrix_t input_matrix(signal->total_length / config.axes, config.axes);
    signal->get_data(0, signal->total_length, input_matrix.buffer);

    // scale the signal
    int ret = numpy::scale(&input_matrix, config.scale_axes);
    if (ret != EIDSP_OK) {
        EIDSP_ERR(ret);
    }

    memcpy(output_matrix->buffer, input_matrix.buffer, signal->total_length);

    return EIDSP_OK;
}

__attribute__((unused)) int extract_flatten_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr) {
    ei_dsp_config_flatten_t config = *((ei_dsp_config_flatten_t*)config_ptr);

    size_t expected_matrix_size = 0;
    if (config.average) expected_matrix_size += config.axes;
    if (config.minimum) expected_matrix_size += config.axes;
    if (config.maximum) expected_matrix_size += config.axes;
    if (config.rms) expected_matrix_size += config.axes;

    if (output_matrix->rows * output_matrix->cols != expected_matrix_size) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    int ret;

    // input matrix from the raw signal
    matrix_t input_matrix(signal->total_length / config.axes, config.axes);
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
            matrix_t out_matrix(1, 1);
            numpy::mean(&row_matrix, &out_matrix);
            output_matrix->buffer[out_matrix_ix++] = out_matrix.buffer[0];
        }

        if (config.minimum) {
            matrix_t out_matrix(1, 1);
            numpy::min(&row_matrix, &out_matrix);
            output_matrix->buffer[out_matrix_ix++] = out_matrix.buffer[0];
        }

        if (config.maximum) {
            matrix_t out_matrix(1, 1);
            numpy::max(&row_matrix, &out_matrix);
            output_matrix->buffer[out_matrix_ix++] = out_matrix.buffer[0];
        }

        if (config.rms) {
            matrix_t out_matrix(1, 1);
            numpy::rms(&row_matrix, &out_matrix);
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

__attribute__((unused)) int extract_mfcc_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr) {
    ei_dsp_config_mfcc_t config = *((ei_dsp_config_mfcc_t*)config_ptr);

    if (config.axes != 1) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    // @todo: move this to config
    const uint32_t frequency = static_cast<uint32_t>(EI_CLASSIFIER_FREQUENCY);

    // preemphasis class to preprocess the audio...
    class speechpy::processing::preemphasis pre(signal, config.pre_shift, config.pre_cof);
    preemphasis = &pre;

    signal_t preemphasized_audio_signal;
    preemphasized_audio_signal.total_length = signal->total_length;
    preemphasized_audio_signal.get_data = &preemphasized_audio_signal_get_data;

    // calculate the size of the MFCC matrix
    matrix_size_t out_matrix_size =
        speechpy::feature::calculate_mfcc_buffer_size(
            signal->total_length, frequency, config.frame_length, config.frame_stride, config.num_cepstral);
    if (out_matrix_size.rows * out_matrix_size.cols != output_matrix->rows * output_matrix->cols) {
        ei_printf("out_matrix = %hux%hu\n", output_matrix->rows, output_matrix->cols);
        ei_printf("calculated size = %hux%hu\n", out_matrix_size.rows, out_matrix_size.cols);
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    output_matrix->rows = out_matrix_size.rows;
    output_matrix->cols = out_matrix_size.cols;

#ifdef __MBED__
    Timer t;
    t.start();
#endif

    // and run the MFCC extraction (using 32 rather than 40 filters here to optimize speed on embedded)
    int ret = speechpy::feature::mfcc(output_matrix, &preemphasized_audio_signal,
        frequency, config.frame_length, config.frame_stride, config.num_cepstral, config.num_filters, config.fft_length,
        config.low_frequency, config.high_frequency);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: MFCC failed (%d)\n", ret);
        EIDSP_ERR(ret);
    }

#ifdef __MBED__
    t.stop();

    // ei_printf("mfcc done in %d ms.\n", t.read_ms());

    t.reset();
    t.start();
#endif

    // cepstral mean and variance normalization
    ret = speechpy::processing::cmvnw(output_matrix, config.win_size, true);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: cmvnw failed (%d)\n", ret);
        EIDSP_ERR(ret);
    }

#ifdef __MBED__
    t.stop();

    // ei_printf("cmvnw done in %d ms.\n", t.read_ms());
#endif

    output_matrix->cols = out_matrix_size.rows * out_matrix_size.cols;
    output_matrix->rows = 1;

    return EIDSP_OK;
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _EDGE_IMPULSE_RUN_DSP_H_
