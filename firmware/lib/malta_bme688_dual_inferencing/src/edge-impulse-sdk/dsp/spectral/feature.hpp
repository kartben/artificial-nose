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

#ifndef _EIDSP_SPECTRAL_FEATURE_H_
#define _EIDSP_SPECTRAL_FEATURE_H_

#include <vector>
#include <stdint.h>
#include "processing.hpp"
#include "edge-impulse-sdk/dsp/ei_utils.h"
#include "model-parameters/model_metadata.h"

namespace ei {
namespace spectral {

typedef enum {
    filter_none = 0,
    filter_lowpass = 1,
    filter_highpass = 2
} filter_t;

class feature {
public:

    static int subtract_mean(matrix_t* input_matrix) {
        // calculate the mean
        EI_DSP_MATRIX(mean_matrix, input_matrix->rows, 1);
        int ret = numpy::mean(input_matrix, &mean_matrix);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        // scale by the mean
        ret = numpy::subtract(input_matrix, &mean_matrix);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        return EIDSP_OK;
    }

    /**
     * Calculate the spectral features over a signal.
     * @param out_features Output matrix. Use `calculate_spectral_buffer_size` to calculate
     *  the size required. Needs as many rows as `raw_data`.
     * @param input_matrix Signal, with one row per axis
     * @param sampling_freq Sampling frequency of the signal
     * @param filter_type Filter type
     * @param filter_cutoff Filter cutoff frequency
     * @param filter_order Filter order
     * @param fft_length Length of the FFT signal
     * @param fft_peaks Number of FFT peaks to find
     * @param fft_peaks_threshold Minimum threshold
     * @param edges_matrix Spectral power edges
     * @returns 0 if OK
     */
    static int spectral_analysis(
        matrix_t *out_features,
        matrix_t *input_matrix,
        float sampling_freq,
        filter_t filter_type,
        float filter_cutoff,
        uint8_t filter_order,
        uint16_t fft_length,
        uint8_t fft_peaks,
        float fft_peaks_threshold,
        matrix_t *edges_matrix_in
    ) {
        if (out_features->rows != input_matrix->rows) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        if (out_features->cols != calculate_spectral_buffer_size(true, fft_peaks, edges_matrix_in->rows)) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        if (edges_matrix_in->cols != 1) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        int ret;

        size_t axes = input_matrix->rows;

        EI_TRY( subtract_mean(input_matrix) );

        // apply filter
        if (filter_type == filter_lowpass) {
            ret = spectral::processing::butterworth_lowpass_filter(
                input_matrix, sampling_freq, filter_cutoff, filter_order);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
            }
        }
        else if (filter_type == filter_highpass) {
            ret = spectral::processing::butterworth_highpass_filter(
                input_matrix, sampling_freq, filter_cutoff, filter_order);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
            }
        }

        // calculate RMS
        EI_DSP_MATRIX(rms_matrix, axes, 1);
        ret = numpy::rms(input_matrix, &rms_matrix);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        // find peaks in FFT
        EI_DSP_MATRIX(peaks_matrix, axes, fft_peaks * 2);

        for (size_t row = 0; row < input_matrix->rows; row++) {
            // per axis code

            // get a slice of the current axis
            EI_DSP_MATRIX_B(axis_matrix, 1, input_matrix->cols, input_matrix->buffer + (row * input_matrix->cols));

            // calculate FFT
            EI_DSP_MATRIX(fft_matrix, 1, fft_length / 2 + 1);
            ret = numpy::rfft(axis_matrix.buffer, axis_matrix.cols, fft_matrix.buffer, fft_matrix.cols, fft_length);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
            }

            // multiply by 2/N
            numpy::scale(&fft_matrix, (2.0f / static_cast<float>(fft_length)));

            // we're now using the FFT matrix to calculate peaks etc.
            EI_DSP_MATRIX(peaks_matrix, fft_peaks, 2);
            ret = spectral::processing::find_fft_peaks(&fft_matrix, &peaks_matrix,
                sampling_freq, fft_peaks_threshold, fft_length);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
            }

            // calculate periodogram for spectral power buckets
            EI_DSP_MATRIX(period_fft_matrix, 1, fft_length / 2 + 1);
            EI_DSP_MATRIX(period_freq_matrix, 1, fft_length / 2 + 1);
            ret = spectral::processing::periodogram(&axis_matrix,
                &period_fft_matrix, &period_freq_matrix, sampling_freq, fft_length);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(ret);
            }

            EI_DSP_MATRIX(edges_matrix_out, edges_matrix_in->rows - 1, 1);
            ret = spectral::processing::spectral_power_edges(
                &period_fft_matrix,
                &period_freq_matrix,
                edges_matrix_in,
                &edges_matrix_out,
                sampling_freq);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(ret);
            }

            float *features_row = out_features->buffer + (row * out_features->cols);

            size_t fx = 0;

            features_row[fx++] = rms_matrix.buffer[row];
            for (size_t peak_row = 0; peak_row < peaks_matrix.rows; peak_row++) {
                features_row[fx++] = peaks_matrix.buffer[peak_row * peaks_matrix.cols + 0];
                features_row[fx++] = peaks_matrix.buffer[peak_row * peaks_matrix.cols + 1];
            }
            for (size_t edge_row = 0; edge_row < edges_matrix_out.rows; edge_row++) {
                features_row[fx++] = edges_matrix_out.buffer[edge_row * edges_matrix_out.cols] / 10.0f;
            }
        }

        return EIDSP_OK;
    }


    /**
     * Calculate the buffer size for Spectral Analysis
     * @param rms: Whether to calculate the RMS as part of the features
     * @param peaks_count: Number of FFT peaks
     * @param spectral_edges_count: Number of spectral edges
     */
    static size_t calculate_spectral_buffer_size(
        bool rms, size_t peaks_count, size_t spectral_edges_count)
    {
        size_t count = 0;
        if (rms) count++;
        count += (peaks_count * 2);
        if (spectral_edges_count > 0) {
            count += (spectral_edges_count - 1);
        }
        return count;
    }

    static int extract_spectral_analysis_features_v1(
        matrix_t *input_matrix,
        matrix_t *output_matrix,
        ei_dsp_config_spectral_analysis_t *config_ptr,
        const float sampling_freq)
    {
        // scale the signal
        int ret = numpy::scale(input_matrix, config_ptr->scale_axes);
        if (ret != EIDSP_OK) {
            ei_printf("ERR: Failed to scale signal (%d)\n", ret);
            EIDSP_ERR(ret);
        }

        // transpose the matrix so we have one row per axis (nifty!)
        ret = numpy::transpose(input_matrix);
        if (ret != EIDSP_OK) {
            ei_printf("ERR: Failed to transpose matrix (%d)\n", ret);
            EIDSP_ERR(ret);
        }

        // the spectral edges that we want to calculate
        matrix_t edges_matrix_in(64, 1);
        size_t edge_matrix_ix = 0;

        char spectral_str[128] = { 0 };
        if (strlen(config_ptr->spectral_power_edges) > sizeof(spectral_str) - 1) {
            EIDSP_ERR(EIDSP_PARAMETER_INVALID);
        }
        memcpy(
            spectral_str,
            config_ptr->spectral_power_edges,
            strlen(config_ptr->spectral_power_edges));

        // convert spectral_power_edges (string) into float array
        char *spectral_ptr = spectral_str;
        while (spectral_ptr != NULL) {
            while ((*spectral_ptr) == ' ') {
                spectral_ptr++;
            }

            edges_matrix_in.buffer[edge_matrix_ix++] = atof(spectral_ptr);

            // find next (spectral) delimiter (or '\0' character)
            while ((*spectral_ptr != ',')) {
                spectral_ptr++;
                if (*spectral_ptr == '\0')
                    break;
            }

            if (*spectral_ptr == '\0') {
                spectral_ptr = NULL;
            }
            else {
                spectral_ptr++;
            }
        }
        edges_matrix_in.rows = edge_matrix_ix;

        // calculate how much room we need for the output matrix
        size_t output_matrix_cols = spectral::feature::calculate_spectral_buffer_size(
            true,
            config_ptr->spectral_peaks_count,
            edges_matrix_in.rows);
        // ei_printf("output_matrix_size %hux%zu\n", input_matrix.rows, output_matrix_cols);
        if (output_matrix->cols * output_matrix->rows !=
            static_cast<uint32_t>(output_matrix_cols * config_ptr->axes)) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        output_matrix->cols = output_matrix_cols;
        output_matrix->rows = config_ptr->axes;

        spectral::filter_t filter_type;
        if (strcmp(config_ptr->filter_type, "low") == 0) {
            filter_type = spectral::filter_lowpass;
        }
        else if (strcmp(config_ptr->filter_type, "high") == 0) {
            filter_type = spectral::filter_highpass;
        }
        else {
            filter_type = spectral::filter_none;
        }

        ret = spectral::feature::spectral_analysis(
            output_matrix,
            input_matrix,
            sampling_freq,
            filter_type,
            config_ptr->filter_cutoff,
            config_ptr->filter_order,
            config_ptr->fft_length,
            config_ptr->spectral_peaks_count,
            config_ptr->spectral_peaks_threshold,
            &edges_matrix_in);
        if (ret != EIDSP_OK) {
            ei_printf("ERR: Failed to calculate spectral features (%d)\n", ret);
            EIDSP_ERR(ret);
        }

        // flatten again
        output_matrix->cols = config_ptr->axes * output_matrix_cols;
        output_matrix->rows = 1;

        return EIDSP_OK;
    }

    static void get_start_stop_bin(
        float sampling_freq,
        size_t fft_length,
        float filter_cutoff,
        size_t *start_bin,
        size_t *stop_bin,
        bool is_high_pass)
    {
        // we want to find n such that fcutoff < sample_f / fft * n ( or > for high pass )
        // also, + - half bin width (sample_f/(fft*2)) for high / low pass
        float bin = filter_cutoff * fft_length / sampling_freq;
        if (is_high_pass) {
            *start_bin = static_cast<size_t>(bin - 0.5) + 1; // add one b/c we want to always round up
            // don't use the DC bin b/c it's zero
            *start_bin = *start_bin == 0 ? 1 : *start_bin;
            *stop_bin = fft_length / 2 + 1; // go one past
        }
        else {
            *start_bin = 1;
            *stop_bin = static_cast<size_t>(bin + 0.5) + 1; // go one past
        }
    }

    static int extract_spectral_analysis_features_v2(
        matrix_t *input_matrix,
        matrix_t *output_matrix,
        ei_dsp_config_spectral_analysis_t *config,
        const float sampling_freq)
    {
        // transpose the matrix so we have one row per axis
        numpy::transpose_in_place(input_matrix);
        
        // func tests for scale of 1 and does a no op in that case
        EI_TRY(numpy::scale(input_matrix, config->scale_axes));

        bool do_filter = false;
        bool is_high_pass;

        // apply filter, if enabled
        // "zero" order filter allowed.  will still remove unwanted fft bins later
        if (strcmp(config->filter_type, "low") == 0) {
            if( config->filter_order ) {
                EI_TRY(spectral::processing::butterworth_lowpass_filter(
                    input_matrix,
                    sampling_freq,
                    config->filter_cutoff,
                    config->filter_order));
            }
            do_filter = true;
            is_high_pass = false;
        }
        else if (strcmp(config->filter_type, "high") == 0) {
            if( config->filter_order ) {
                EI_TRY(spectral::processing::butterworth_highpass_filter(
                    input_matrix,
                    sampling_freq,
                    config->filter_cutoff,
                    config->filter_order));
            }
            do_filter = true;
            is_high_pass = true;
        }

        EI_TRY(subtract_mean(input_matrix));

        // Figure bins we remove based on filter cutoff
        size_t start_bin, stop_bin;
        if (do_filter) {
            get_start_stop_bin(
                sampling_freq,
                config->fft_length,
                config->filter_cutoff,
                &start_bin,
                &stop_bin,
                is_high_pass);
        }
        else {
            start_bin = 1;
            stop_bin = config->fft_length / 2 + 1;
        }
        size_t num_bins = stop_bin - start_bin;

        float *feature_out = output_matrix->buffer;
        for (size_t row = 0; row < input_matrix->rows; row++) {
            float *data_window = input_matrix->get_row_ptr(row);
            size_t data_size = input_matrix->cols;

            matrix_t rms_in_matrix(1, data_size, data_window);
            matrix_t rms_out_matrix(1, 1, feature_out);
            EI_TRY(numpy::rms(&rms_in_matrix, &rms_out_matrix));

            feature_out++;

            // Standard Deviation
            float stddev = *(feature_out-1); //= sqrt(numpy::variance(data_window, data_size));
            // Don't add std dev as a feature b/c it's the same as RMS
            // Skew and Kurtosis w/ shortcut:
            // See definition at https://en.wikipedia.org/wiki/Skewness
            // See definition at https://en.wikipedia.org/wiki/Kurtosis
            // Substitute 0 for mean (b/c it is subtracted out above)
            // Skew becomes: mean(X^3) / stddev^3
            // Kurtosis becomes: mean(X^4) / stddev^4
            // Note, this is the Fisher definition of Kurtosis, so subtract 3
            // (see https://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.kurtosis.html)
            float s_sum = 0;
            float k_sum = 0;
            float temp;
            for (int i = 0; i < data_size; i++) {
                temp = data_window[i] * data_window[i] * data_window[i];
                s_sum += temp;
                k_sum += temp * data_window[i];
            }
            // Skewness out
            temp = stddev * stddev * stddev;
            *feature_out++ = (s_sum / data_size) / temp;
            // Kurtosis out
            *feature_out++ = ((k_sum / data_size) / (temp * stddev)) - 3;

            EI_TRY(numpy::welch_max_hold(
                data_window,
                data_size,
                feature_out,
                start_bin,
                stop_bin,
                config->fft_length,
                config->do_fft_overlap));
            if (config->do_log) {
                numpy::zero_handling(feature_out, num_bins);
                ei_matrix temp(num_bins, 1, feature_out);
                numpy::log10(&temp);
            }
            feature_out += num_bins;
        }
        return EIDSP_OK;
    }
};

} // namespace spectral
} // namespace ei



#endif // _EIDSP_SPECTRAL_FEATURE_H_
