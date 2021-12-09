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

#ifndef _EI_CLASSIFIER_SIGNAL_WITH_AXES_H_
#define _EI_CLASSIFIER_SIGNAL_WITH_AXES_H_

#include "edge-impulse-sdk/dsp/numpy_types.h"
#include "edge-impulse-sdk/dsp/returntypes.hpp"

#if !EIDSP_SIGNAL_C_FN_POINTER

using namespace ei;

class SignalWithAxes {
public:
    SignalWithAxes(signal_t *original_signal, uint8_t *axes, size_t axes_count):
        _original_signal(original_signal), _axes(axes), _axes_count(axes_count)
    {

    }

    signal_t * get_signal() {
        if (this->_axes_count == EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME) {
            return this->_original_signal;
        }

        wrapped_signal.total_length = _original_signal->total_length / EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME * _axes_count;
#ifdef __MBED__
        wrapped_signal.get_data = mbed::callback(this, &SignalWithAxes::get_data);
#else
        wrapped_signal.get_data = [this](size_t offset, size_t length, float *out_ptr) {
            return this->get_data(offset, length, out_ptr);
        };
#endif
        return &wrapped_signal;
    }

    int get_data(size_t offset, size_t length, float *out_ptr) {
        size_t offset_on_original_signal = offset / _axes_count * EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME;
        size_t length_on_original_signal = length / _axes_count * EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME;

        size_t out_ptr_ix = 0;

        for (size_t ix = offset_on_original_signal; ix < offset_on_original_signal + length_on_original_signal; ix += EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME) {
            for (size_t axis_ix = 0; axis_ix < this->_axes_count; axis_ix++) {
                int r = _original_signal->get_data(ix + _axes[axis_ix], 1, &out_ptr[out_ptr_ix++]);
                if (r != 0) {
                    return r;
                }
            }
        }

        return 0;
    }

private:
    signal_t *_original_signal;
    uint8_t *_axes;
    size_t _axes_count;
    signal_t wrapped_signal;
};

class SignalWithAxesI16 {
public:
    SignalWithAxesI16(signal_i16_t *original_signal, uint8_t *axes, size_t axes_count):
        _original_signal(original_signal), _axes(axes), _axes_count(axes_count)
    {

    }

    signal_i16_t * get_signal() {
        if (this->_axes_count == EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME) {
            return this->_original_signal;
        }

        wrapped_signal.total_length = _original_signal->total_length / EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME * _axes_count;
#ifdef __MBED__
        wrapped_signal.get_data = mbed::callback(this, &SignalWithAxesI16::get_data);
#else
        wrapped_signal.get_data = [this](size_t offset, size_t length, int16_t *out_ptr) {
            return this->get_data(offset, length, out_ptr);
        };
#endif
        return &wrapped_signal;
    }

    int get_data(size_t offset, size_t length, int16_t *out_ptr) {
        size_t offset_on_original_signal = offset / _axes_count * EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME;
        size_t length_on_original_signal = length / _axes_count * EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME;

        size_t out_ptr_ix = 0;

        for (size_t ix = offset_on_original_signal; ix < offset_on_original_signal + length_on_original_signal; ix += EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME) {
            for (size_t axis_ix = 0; axis_ix < this->_axes_count; axis_ix++) {
                int r = _original_signal->get_data(ix + _axes[axis_ix], 1, &out_ptr[out_ptr_ix++]);
                if (r != 0) {
                    return r;
                }
            }
        }

        return 0;
    }

private:
    signal_i16_t *_original_signal;
    uint8_t *_axes;
    size_t _axes_count;
    signal_i16_t wrapped_signal;
};

#endif // #if !EIDSP_SIGNAL_C_FN_POINTER

#endif // _EI_CLASSIFIER_SIGNAL_WITH_AXES_H_
