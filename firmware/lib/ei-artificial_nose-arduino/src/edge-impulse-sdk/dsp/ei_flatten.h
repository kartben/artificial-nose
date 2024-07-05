/* Edge Impulse inferencing library
 * Copyright (c) 2023 EdgeImpulse Inc.
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

#ifndef __EI_FLATTEN__H__
#define __EI_FLATTEN__H__

#include "edge-impulse-sdk/dsp/ei_vector.h"
#include "edge-impulse-sdk/dsp/returntypes.hpp"
#include "edge-impulse-sdk/dsp/ei_dsp_handle.h"
#include "model-parameters/model_metadata.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"
#include "edge-impulse-sdk/dsp/config.hpp"

class flatten_class : public DspHandle {
public:
    int print() override {
        ei_printf("means: ");
        for(int axis = 0; (size_t)axis < this->means.size(); axis++) {
        ei_printf("axis: %i\n", axis);
            for (size_t i = 0; i < this->means.size(); i++) {
                ei_printf("%f ", this->means[axis][i]);
            }
        }
        ei_printf("\n");
        return ei::EIDSP_OK;
    }

    int extract(ei::signal_t *signal, ei::matrix_t *output_matrix, void *config_ptr, const float frequency) override {
        using namespace ei;

        ei_dsp_config_flatten_t config = *((ei_dsp_config_flatten_t*)config_ptr);

        uint32_t expected_matrix_size = 0;
        if (config.average) expected_matrix_size += config.axes;
        if (config.minimum) expected_matrix_size += config.axes;
        if (config.maximum) expected_matrix_size += config.axes;
        if (config.rms) expected_matrix_size += config.axes;
        if (config.stdev) expected_matrix_size += config.axes;
        if (config.skewness) expected_matrix_size += config.axes;
        if (config.kurtosis) expected_matrix_size += config.axes;
        if (config.moving_avg_num_windows) expected_matrix_size += config.axes;

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

        // transpose the matrix so we have one row per axis
        numpy::transpose_in_place(&input_matrix);

        size_t out_matrix_ix = 0;

        for (size_t row = 0; row < input_matrix.rows; row++) {
            matrix_t row_matrix(1, input_matrix.cols, input_matrix.buffer + (row * input_matrix.cols));

            float mean; // to use with moving average

            if (config.average || config.moving_avg_num_windows) {
                float fbuffer;
                matrix_t out_matrix(1, 1, &fbuffer);
                numpy::mean(&row_matrix, &out_matrix);
                mean = out_matrix.buffer[0];
                if (config.average) {
                    output_matrix->buffer[out_matrix_ix++] = mean;
                }
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

            if (config.moving_avg_num_windows) {
                push_mean(row, mean);
                output_matrix->buffer[out_matrix_ix++] = numpy::mean(means[row].data(), means[row].size());
            }
        }

        // flatten again
        output_matrix->cols = output_matrix->rows * output_matrix->cols;
        output_matrix->rows = 1;

        return EIDSP_OK;
    }

    static DspHandle* create(void* config, float _sampling_frequency);

    void* operator new(size_t size) {
        // Custom memory allocation logic here
        return ei_malloc(size);
    }

    void operator delete(void* ptr) {
        // Custom memory deallocation logic here
        ei_free(ptr);
    }

private:
    ei_vector<ei_vector<float>> means;
    ei_vector<size_t> head_indexes;
    size_t moving_avg_num_windows;

    flatten_class(int moving_avg_num_windows, int axes_count) : means(axes_count), head_indexes(axes_count, 0) {
        this->moving_avg_num_windows = moving_avg_num_windows;
    }

    void push_mean(int axis, float mean) {
        auto& head = head_indexes[axis];
        if (head_indexes[axis] >= means[axis].size()) {
            means[axis].push_back(mean);
        } else {
            means[axis][head] = mean;
        }
        head = head + 1;
        // This is a lot cheaper than mod (%)
        if (head >= moving_avg_num_windows) {
            head = 0;
        }
    }
};

DspHandle* flatten_class::create(void* config_in, float _sampling_frequency) { // NOLINT def in header is OK at EI
    auto config = reinterpret_cast<ei_dsp_config_flatten_t*>(config_in);
    return new flatten_class(config->moving_avg_num_windows, config->axes);
};

#endif  //!__EI_FLATTEN__H__