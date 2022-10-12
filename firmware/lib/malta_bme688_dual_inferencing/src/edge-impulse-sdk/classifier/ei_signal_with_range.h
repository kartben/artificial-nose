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

#ifndef _EI_CLASSIFIER_SIGNAL_WITH_RANGE_H_
#define _EI_CLASSIFIER_SIGNAL_WITH_RANGE_H_

#include "edge-impulse-sdk/dsp/numpy_types.h"
#include "edge-impulse-sdk/dsp/returntypes.hpp"

#if !EIDSP_SIGNAL_C_FN_POINTER

using namespace ei;

class SignalWithRange {
public:
    SignalWithRange(signal_t *original_signal, uint32_t range_start, uint32_t range_end):
        _original_signal(original_signal), _range_start(range_start), _range_end(range_end)
    {

    }

    signal_t * get_signal() {
        if (this->_range_start == 0 && this->_range_end == this->_original_signal->total_length) {
            return this->_original_signal;
        }

        wrapped_signal.total_length = _range_end - _range_start;
#ifdef __MBED__
        wrapped_signal.get_data = mbed::callback(this, &SignalWithRange::get_data);
#else
        wrapped_signal.get_data = [this](size_t offset, size_t length, float *out_ptr) {
            return this->get_data(offset, length, out_ptr);
        };
#endif
        return &wrapped_signal;
    }

    int get_data(size_t offset, size_t length, float *out_ptr) {
        return _original_signal->get_data(offset + _range_start, length, out_ptr);
    }

private:
    signal_t *_original_signal;
    uint32_t _range_start;
    uint32_t _range_end;
    signal_t wrapped_signal;
};

#endif // #if !EIDSP_SIGNAL_C_FN_POINTER

#endif // _EI_CLASSIFIER_SIGNAL_WITH_RANGE_H_
