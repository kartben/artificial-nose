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

#ifndef _EIDSP_SPEECHPY_FUNCTIONS_H_
#define _EIDSP_SPEECHPY_FUNCTIONS_H_

#include <math.h>
#include "../numpy.hpp"
#include "../returntypes.hpp"

namespace ei {
namespace speechpy {

class functions {
public:
    /**
     * Converting from frequency to Mel scale
     *
     * @param f The frequency values(or a single frequency) in Hz.
     * @returns The mel scale values(or a single mel).
     */
    static float frequency_to_mel(float f) {
        return 1127.0 * numpy::log(1 + f / 700.0f);
    }

    /**
     * Converting from Mel scale to frequency.
     *
     * @param mel The mel scale values(or a single mel).
     * @returns The frequency values(or a single frequency) in Hz.
     */
    static float mel_to_frequency(float mel) {
        return 700.0f * (exp(mel / 1127.0f) - 1.0f);
    }

    /**
     * This function handle the issue with zero values if the are exposed
     * to become an argument for any log function.
     * @param input Array
     * @param input_size Size of array
     * @returns void
     */
    static void zero_handling(float *input, size_t input_size) {
        for (size_t ix = 0; ix < input_size; ix++) {
            if (input[ix] == 0) {
                input[ix] = 1e-10;
            }
        }
    }

    /**
     * This function handle the issue with zero values if the are exposed
     * to become an argument for any log function.
     * @param input Matrix
     * @returns void
     */
    static void zero_handling(matrix_t *input) {
        zero_handling(input->buffer, input->rows * input->cols);
    }


    /**
     * Triangle, I'm not really sure what this does
     * @param x Linspace output, will be overwritten!
     * @param x_size Size of the linspace output
     * @param left
     * @param middle
     * @param right
     */
    static int triangle(float *x, size_t x_size, int left, int middle, int right) {
        EI_DSP_MATRIX(out, 1, x_size);

        for (size_t ix = 0; ix < x_size; ix++) {
            if (x[ix] > left && x[ix] <= middle) {
                out.buffer[ix] = (x[ix] - left) / (middle - left);
            }

            if (x[ix] < right && middle <= x[ix]) {
                out.buffer[ix] = (right - x[ix]) / (right - middle);
            }
        }

        memcpy(x, out.buffer, x_size * sizeof(float));

        return EIDSP_OK;
    }
};

} // namespace speechpy
} // namespace ei

#endif // _EIDSP_SPEECHPY_FUNCTIONS_H_
