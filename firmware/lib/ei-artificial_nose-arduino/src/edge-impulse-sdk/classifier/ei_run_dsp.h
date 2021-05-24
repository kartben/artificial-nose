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

/* extract mfcc slice features variables */
static uint32_t last_read_sample = 0;
static float *cache_sample_buffer;
static uint32_t cache_sample_size = 0;

#if defined(EI_DSP_IMAGE_BUFFER_STATIC_SIZE)
float ei_dsp_image_buffer[EI_DSP_IMAGE_BUFFER_STATIC_SIZE];
#endif

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

    const uint32_t frequency = static_cast<uint32_t>(sampling_frequency);

    // preemphasis class to preprocess the audio...
    class speechpy::processing::preemphasis pre(signal, config.pre_shift, config.pre_cof);
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
        ei_printf("out_matrix = %hux%hu\n", output_matrix->rows, output_matrix->cols);
        ei_printf("calculated size = %hux%hu\n", out_matrix_size.rows, out_matrix_size.cols);
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

/**
 * @brief Preemphasize audio from sample and collect data from cached buffer
 *        Cached buffer data is already preemphasized
 */
static int preemphasized_audio_signal_get_and_align_data(size_t offset, size_t length, float *out_ptr)
{
    size_t ix;

    for(ix = 0; (ix + offset) < cache_sample_size; ix++) {
        *(out_ptr++) = cache_sample_buffer[ix + offset];
    }
    offset += ix;
    length -= ix;

    /* Determine last read sample in signal buffer, so subtract cache buffer */
    last_read_sample = offset + length - cache_sample_size;
    return preemphasis->get_data(offset - cache_sample_size, length, out_ptr);
}

__attribute__((unused)) int extract_mfcc_per_slice_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float sampling_frequency) {

    ei_dsp_config_mfcc_t config = *((ei_dsp_config_mfcc_t*)config_ptr);

    if (config.axes != 1) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    if(config.implementation_version != 1 && config.implementation_version != 2) {
        EIDSP_ERR(EIDSP_BLOCK_VERSION_INCORRECT);
    }

    const uint32_t frequency = static_cast<uint32_t>(sampling_frequency);

    // preemphasis class to preprocess the audio...
    class speechpy::processing::preemphasis pre(signal, config.pre_shift, config.pre_cof);
    preemphasis = &pre;

    /* Increase the buffer length with the cached sample data */
    signal->total_length += cache_sample_size;

    /* Fake an extra frame_length for stack frames calculations. There, 1 frame_length is always
    subtracted and there for never used. But skip the first slice to fit the feature_matrix
    buffer */
    static bool first_run = false;

    if(config.implementation_version < 2) {

        if (first_run == true) {
            signal->total_length += (size_t)(config.frame_length * (float)frequency);
        }

        first_run = true;
    }

    signal_t preemphasized_audio_signal;
    preemphasized_audio_signal.total_length = signal->total_length;
    preemphasized_audio_signal.get_data = &preemphasized_audio_signal_get_and_align_data;
    last_read_sample = 0;

    // calculate the size of the MFCC matrix
    matrix_size_t out_matrix_size =
        speechpy::feature::calculate_mfcc_buffer_size(
            signal->total_length, frequency, config.frame_length, config.frame_stride, config.num_cepstral, config.implementation_version);
    /* Only throw size mismatch error calculated buffer doesn't fit for continuous inferencing */
    if (out_matrix_size.rows * out_matrix_size.cols > output_matrix->rows * output_matrix->cols) {
        ei_printf("out_matrix = %hux%hu\n", output_matrix->rows, output_matrix->cols);
        ei_printf("calculated size = %hux%hu\n", out_matrix_size.rows, out_matrix_size.cols);
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

    output_matrix->cols = out_matrix_size.rows * out_matrix_size.cols;
    output_matrix->rows = 1;

    if(config.implementation_version < 2) {
        if (first_run == true) {
            signal->total_length -= (size_t)(config.frame_length * (float)frequency);
        }
    }

    /* Get back to original sample length */
    signal->total_length -= cache_sample_size;

    if(cache_sample_buffer) {
        ei_free(cache_sample_buffer);
        cache_sample_size = 0;
        cache_sample_buffer = 0;
    }

    /* Cache data if not complete sample buffer is read */
    if(last_read_sample < (uint32_t)signal->total_length) {

        uint32_t missing_samples =  signal->total_length - last_read_sample;

        cache_sample_buffer = (float *)ei_malloc(missing_samples * sizeof(float));
        if(cache_sample_buffer == NULL) {
            EIDSP_ERR(EIDSP_OUT_OF_MEM);
        }
        preemphasis->get_data(last_read_sample, missing_samples, cache_sample_buffer);
        cache_sample_size = missing_samples;
    }

    return EIDSP_OK;
}


__attribute__((unused)) int extract_spectrogram_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float sampling_frequency) {
    ei_dsp_config_spectrogram_t config = *((ei_dsp_config_spectrogram_t*)config_ptr);

    if (config.axes != 1) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    const uint32_t frequency = static_cast<uint32_t>(sampling_frequency);

    // calculate the size of the MFE matrix
    matrix_size_t out_matrix_size =
        speechpy::feature::calculate_mfe_buffer_size(
            signal->total_length, frequency, config.frame_length, config.frame_stride, config.fft_length / 2 + 1,
            config.implementation_version);
    /* Only throw size mismatch error calculated buffer doesn't fit for continuous inferencing */
    if (out_matrix_size.rows * out_matrix_size.cols > output_matrix->rows * output_matrix->cols) {
        ei_printf("out_matrix = %hux%hu\n", output_matrix->rows, output_matrix->cols);
        ei_printf("calculated size = %hux%hu\n", out_matrix_size.rows, out_matrix_size.cols);
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

    ret = numpy::normalize(output_matrix);
    if (ret != EIDSP_OK) {
        EIDSP_ERR(ret);
    }

    output_matrix->cols = out_matrix_size.rows * out_matrix_size.cols;
    output_matrix->rows = 1;

    return EIDSP_OK;
}

__attribute__((unused)) int extract_spectrogram_per_slice_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float sampling_frequency) {
    ei_dsp_config_spectrogram_t config = *((ei_dsp_config_spectrogram_t*)config_ptr);

    static bool first_run = false;

    if (config.axes != 1) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
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

    // calculate the size of the MFE matrix
    matrix_size_t out_matrix_size =
        speechpy::feature::calculate_mfe_buffer_size(
            signal->total_length, frequency, config.frame_length, config.frame_stride, config.fft_length / 2 + 1,
            config.implementation_version);
    /* Only throw size mismatch error calculated buffer doesn't fit for continuous inferencing */
    if (out_matrix_size.rows * out_matrix_size.cols > output_matrix->rows * output_matrix->cols) {
        ei_printf("out_matrix = %hux%hu\n", output_matrix->rows, output_matrix->cols);
        ei_printf("calculated size = %hux%hu\n", out_matrix_size.rows, out_matrix_size.cols);
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    output_matrix->rows = out_matrix_size.rows;
    output_matrix->cols = out_matrix_size.cols;

    EI_DSP_MATRIX(energy_matrix, output_matrix->rows, 1);
    if (!energy_matrix.buffer) {
        EIDSP_ERR(EIDSP_OUT_OF_MEM);
    }

    // and calculate the spectrogram
    int ret = speechpy::feature::spectrogram(output_matrix, signal,
        frequency, config.frame_length, config.frame_stride, config.fft_length, config.implementation_version);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: Spectrogram failed (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    if(config.implementation_version < 2) {
        if (first_run == true) {
            signal->total_length -= (size_t)(config.frame_length * (float)frequency);
        }
    }

    output_matrix->cols = out_matrix_size.rows * out_matrix_size.cols;
    output_matrix->rows = 1;

    return EIDSP_OK;
}

__attribute__((unused)) int extract_mfe_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float sampling_frequency) {
    ei_dsp_config_mfe_t config = *((ei_dsp_config_mfe_t*)config_ptr);

    if (config.axes != 1) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    const uint32_t frequency = static_cast<uint32_t>(sampling_frequency);

    // calculate the size of the MFE matrix
    matrix_size_t out_matrix_size =
        speechpy::feature::calculate_mfe_buffer_size(
            signal->total_length, frequency, config.frame_length, config.frame_stride, config.num_filters,
            config.implementation_version);
    /* Only throw size mismatch error calculated buffer doesn't fit for continuous inferencing */
    if (out_matrix_size.rows * out_matrix_size.cols > output_matrix->rows * output_matrix->cols) {
        ei_printf("out_matrix = %hux%hu\n", output_matrix->rows, output_matrix->cols);
        ei_printf("calculated size = %hux%hu\n", out_matrix_size.rows, out_matrix_size.cols);
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    output_matrix->rows = out_matrix_size.rows;
    output_matrix->cols = out_matrix_size.cols;

    // and run the MFE extraction
    EI_DSP_MATRIX(energy_matrix, output_matrix->rows, 1);
    if (!energy_matrix.buffer) {
        EIDSP_ERR(EIDSP_OUT_OF_MEM);
    }

    int ret = speechpy::feature::mfe(output_matrix, &energy_matrix, signal,
        frequency, config.frame_length, config.frame_stride, config.num_filters, config.fft_length,
        config.low_frequency, config.high_frequency, config.implementation_version);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: MFE failed (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    // cepstral mean and variance normalization
    ret = speechpy::processing::cmvnw(output_matrix, config.win_size, false, true);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: cmvnw failed (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    output_matrix->cols = out_matrix_size.rows * out_matrix_size.cols;
    output_matrix->rows = 1;

    return EIDSP_OK;
}

__attribute__((unused)) int extract_mfe_per_slice_features(signal_t *signal, matrix_t *output_matrix, void *config_ptr, const float sampling_frequency) {
    ei_dsp_config_mfe_t config = *((ei_dsp_config_mfe_t*)config_ptr);

    static bool first_run = false;

    if (config.axes != 1) {
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
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

    // calculate the size of the MFE matrix
    matrix_size_t out_matrix_size =
        speechpy::feature::calculate_mfe_buffer_size(
            signal->total_length, frequency, config.frame_length, config.frame_stride, config.num_filters,
            config.implementation_version);
    /* Only throw size mismatch error calculated buffer doesn't fit for continuous inferencing */
    if (out_matrix_size.rows * out_matrix_size.cols > output_matrix->rows * output_matrix->cols) {
        ei_printf("out_matrix = %hux%hu\n", output_matrix->rows, output_matrix->cols);
        ei_printf("calculated size = %hux%hu\n", out_matrix_size.rows, out_matrix_size.cols);
        EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
    }

    output_matrix->rows = out_matrix_size.rows;
    output_matrix->cols = out_matrix_size.cols;

    EI_DSP_MATRIX(energy_matrix, output_matrix->rows, 1);
    if (!energy_matrix.buffer) {
        EIDSP_ERR(EIDSP_OUT_OF_MEM);
    }

    // and run the MFE extraction
    int ret = speechpy::feature::mfe(output_matrix, &energy_matrix, signal,
        frequency, config.frame_length, config.frame_stride, config.num_filters, config.fft_length,
        config.low_frequency, config.high_frequency, config.implementation_version);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: MFCC failed (%d)\n", ret);
        EIDSP_ERR(ret);
    }

    if(config.implementation_version < 2) {
        if (first_run == true) {
            signal->total_length -= (size_t)(config.frame_length * (float)frequency);
        }
    }

    output_matrix->cols = out_matrix_size.rows * out_matrix_size.cols;
    output_matrix->rows = 1;

    return EIDSP_OK;
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

            float r = static_cast<float>(pixel >> 16 & 0xff) / 255.0f;
            float g = static_cast<float>(pixel >> 8 & 0xff) / 255.0f;
            float b = static_cast<float>(pixel & 0xff) / 255.0f;

            if (channel_count == 3) {
                output_matrix->buffer[output_ix++] = static_cast<int8_t>(round(r / EI_CLASSIFIER_TFLITE_INPUT_SCALE) + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
                output_matrix->buffer[output_ix++] = static_cast<int8_t>(round(g / EI_CLASSIFIER_TFLITE_INPUT_SCALE) + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
                output_matrix->buffer[output_ix++] = static_cast<int8_t>(round(b / EI_CLASSIFIER_TFLITE_INPUT_SCALE) + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
            }
            else {
                // ITU-R 601-2 luma transform
                // see: https://pillow.readthedocs.io/en/stable/reference/Image.html#PIL.Image.Image.convert
                float v = (0.299f * r) + (0.587f * g) + (0.114f * b);
                output_matrix->buffer[output_ix++] = static_cast<int8_t>(round(v / EI_CLASSIFIER_TFLITE_INPUT_SCALE) + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
            }
        }

        bytes_left -= elements_to_read;
    }

    return EIDSP_OK;
}
#endif // EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _EDGE_IMPULSE_RUN_DSP_H_
