/* Edge Impulse inferencing library
 * Copyright (c) 2022 EdgeImpulse Inc.
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

#ifndef HR_PPG_HPP
#define HR_PPG_HPP

#include "edge-impulse-sdk/dsp/numpy.hpp"
#include "edge-impulse-sdk/dsp/ei_dsp_handle.h"
#include "edge-impulse-enterprise/hr/hr_ppg.hpp"

class hr_class : public DspHandle {
public:
    int print() override {
        ei_printf("Last HR: %f\n", ppg._res.hr);
        return ei::EIDSP_OK;
    }

    int extract(ei::signal_t *signal, ei::matrix_t *output_matrix, void *config_ptr, const float frequency) override {
        using namespace ei;

        // Don't need just yet
        // ei_dsp_config_hr_t config = *((ei_dsp_config_hr_t*)config_ptr);


        // TODO fix for axes / accel
        size_t samples_per_inc = ppg.win_inc_samples;
        // TODO go in a loop for the full window size, once I can actually test this vs studio
        if(signal->total_length != samples_per_inc) {
            return EIDSP_BUFFER_SIZE_MISMATCH;
        }

        // TODO ask for smaller increments and bp them into place
        // Copy into the end of the buffer
        matrix_t temp(ppg.axes, samples_per_inc);
        signal->get_data(0, samples_per_inc, temp.buffer);


        output_matrix->buffer[0] = ppg.stream(&temp);

        output_matrix->rows = 1;
        output_matrix->cols = 1;
        return EIDSP_OK;
    }

    // TODO: actually read in config: axes too!
    hr_class(float frequency) : ppg(frequency, 1, 8*50, 2*50, true) {
    }

    // Boilerplate below here
    static DspHandle* create(void* config, float frequency);

    void* operator new(size_t size) {
        // Custom memory allocation logic here
        return ei_malloc(size);
    }

    void operator delete(void* ptr) {
        // Custom memory deallocation logic here
        ei_free(ptr);
    }
    // end boilerplate
private:
    ei::hr_ppg ppg;
};

DspHandle* hr_class::create(void* config_in, float frequency) { // NOLINT def in header is OK at EI
    // Don't need just yet
    // auto config = reinterpret_cast<ei_dsp_config_hr_t*>(config_in);
    // TODO: actually read in config
    return new hr_class(frequency);
};

/*
NOTE, contact EI sales for license and source to use EI heart rate and heart rate variance functions in deployment
*/

#endif