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

namespace ei {
namespace spectral {

typedef enum {
    filter_none = 0,
    filter_lowpass = 1,
    filter_highpass = 2
} filter_t;

class feature {
public:
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

        // calculate the mean
        EI_DSP_MATRIX(mean_matrix, axes, 1);
        ret = numpy::mean(input_matrix, &mean_matrix);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        // scale by the mean
        ret = numpy::subtract(input_matrix, &mean_matrix);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

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

    static int spectral_analysis(
        matrix_i32_t *out_features,
        matrix_i16_t *input_matrix,
        float sampling_freq,
        filter_t filter_type,
        float filter_cutoff,
        uint8_t filter_order,
        uint16_t fft_length,
        uint8_t fft_peaks,
        float fft_peaks_threshold,
        matrix_i16_t *edges_matrix_in
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

        // calculate the mean
        EI_DSP_i16_MATRIX(mean_matrix, axes, 1);
        ret = numpy::mean(input_matrix, &mean_matrix);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        // scale by the mean
        ret = numpy::subtract(input_matrix, &mean_matrix);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        // apply filter
        if (filter_type == filter_lowpass) {
            ret = spectral::processing::i16_filter(
                input_matrix, sampling_freq, filter_order, filter_cutoff, 0);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
            }
        } 
        else if (filter_type == filter_highpass) {
            ret = spectral::processing::i16_filter(
                input_matrix, sampling_freq, filter_order, 0, filter_cutoff);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
            }
        }

        // calculate RMS
        EI_DSP_i16_MATRIX(rms_matrix, axes, 1);
        ret = numpy::rms(input_matrix, &rms_matrix);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        // calculate FFT
        EI_DSP_i32_MATRIX(fft_matrix, 1, fft_length / 2 + 1);
        ei_matrix_i32 axis_matrix_i32(1, input_matrix->cols);

        // find peaks in FFT
        EI_DSP_i16_MATRIX(peaks_matrix, axes, fft_peaks * 2);

        // EIDSP_i16 fft_scaled = fft_length / 10;

        EI_DSP_i16_MATRIX(period_fft_matrix, 1, fft_length / 2 + 1);
        EI_DSP_i16_MATRIX(period_freq_matrix, 1, fft_length / 2 + 1);
        EI_DSP_i16_MATRIX(edges_matrix_out, edges_matrix_in->rows - 1, 1);

        for (size_t row = 0; row < input_matrix->rows; row++) {
            // per axis code

            // get a slice of the current axis
            EI_DSP_i16_MATRIX_B(axis_matrix, 1, input_matrix->cols, input_matrix->buffer + (row * input_matrix->cols));

            // Convert to i32 for accuracy
            for(uint32_t i = 0; i < input_matrix->cols; i++) {
                axis_matrix_i32.buffer[i] = ((EIDSP_i32)axis_matrix.buffer[i]) << 16;
            }

            ret = numpy::rfft(axis_matrix_i32.buffer, axis_matrix.cols, fft_matrix.buffer, fft_matrix.cols, fft_length);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
            }

            // multiply by 2/N
            numpy::scale(&fft_matrix, (2.0f / static_cast<float>(fft_length)));

            // we're now using the FFT matrix to calculate peaks etc.
            EI_DSP_i32_MATRIX(peaks_matrix, fft_peaks, 2);
            ret = spectral::processing::find_fft_peaks(&fft_matrix, &peaks_matrix, sampling_freq, fft_peaks_threshold, fft_length);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
            }

            // calculate periodogram for spectral power buckets
            ret = spectral::processing::periodogram(&axis_matrix,
                &period_fft_matrix, &period_freq_matrix, sampling_freq, fft_length);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(ret);
            }

            // EI_DSP_i16_MATRIX(edges_matrix_out, edges_matrix_in->rows - 1, 1);
            ret = spectral::processing::spectral_power_edges(
                &period_fft_matrix,
                &period_freq_matrix,
                edges_matrix_in,
                &edges_matrix_out,
                sampling_freq);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(ret);
            }

            EIDSP_i32 *features_row = out_features->buffer + (row * out_features->cols);

            size_t fx = 0;
            
            features_row[fx++] = rms_matrix.buffer[row];

            for (size_t peak_row = 0; peak_row < peaks_matrix.rows; peak_row++) {

                features_row[fx++] = (EIDSP_i16)(peaks_matrix.buffer[peak_row * peaks_matrix.cols + 0] >> 16) * fft_length;
                features_row[fx++] = (EIDSP_i16)(peaks_matrix.buffer[peak_row * peaks_matrix.cols + 1] >> 16) * fft_length;
            }

            for (size_t edge_row = 0; edge_row < edges_matrix_out.rows; edge_row++) {
                features_row[fx] = (EIDSP_i16)(edges_matrix_out.buffer[edge_row * edges_matrix_out.cols] >> 16);
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
};

} // namespace spectral
} // namespace ei



#endif // _EIDSP_SPECTRAL_FEATURE_H_
