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

#ifndef _EIDSP_SPECTRAL_FILTERS_H_
#define _EIDSP_SPECTRAL_FILTERS_H_

#include <math.h>
#include "../numpy.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif // M_PI

namespace ei {
namespace spectral {
namespace filters {
    /**
     * The Butterworth filter has maximally flat frequency response in the passband.
     * @param filter_order Even filter order (between 2..8)
     * @param sampling_freq Sample frequency of the signal
     * @param cutoff_freq Cut-off frequency of the signal
     * @param src Source array
     * @param dest Destination array
     * @param size Size of both source and destination arrays
     */
    static void butterworth_lowpass(
        int filter_order,
        float sampling_freq,
        float cutoff_freq,
        const float *src,
        float *dest,
        size_t size)
    {
        int n_steps = filter_order / 2;
        float a = tan(M_PI * cutoff_freq / sampling_freq);
        float a2 = pow(a, 2);
        float *A = (float*)ei_calloc(n_steps, sizeof(float));
        float *d1 = (float*)ei_calloc(n_steps, sizeof(float));
        float *d2 = (float*)ei_calloc(n_steps, sizeof(float));
        float *w0 = (float*)ei_calloc(n_steps, sizeof(float));
        float *w1 = (float*)ei_calloc(n_steps, sizeof(float));
        float *w2 = (float*)ei_calloc(n_steps, sizeof(float));

        // Calculate the filter parameters
        for(int ix = 0; ix < n_steps; ix++) {
            float r = sin(M_PI * ((2.0 * ix) + 1.0) / (2.0 * filter_order));
            sampling_freq = a2 + (2.0 * a * r) + 1.0;
            A[ix] = a2 / sampling_freq;
            d1[ix] = 2.0 * (1 - a2) / sampling_freq;
            d2[ix] = -(a2 - (2.0 * a * r) + 1.0) / sampling_freq;
        }

        // Apply the filter
        for (size_t sx = 0; sx < size; sx++) {
            dest[sx] = src[sx];

            for (int i = 0; i < n_steps; i++) {
                w0[i] = d1[i] * w1[i] + d2[i] * w2[i] + dest[sx];
                dest[sx] = A[i] * (w0[i] + (2.0 * w1[i]) + w2[i]);
                w2[i] = w1[i];
                w1[i] = w0[i];
            }
        }

        ei_free(A);
        ei_free(d1);
        ei_free(d2);
        ei_free(w0);
        ei_free(w1);
        ei_free(w2);
    }

    /**
     * The Butterworth filter has maximally flat frequency response in the passband.
     * @param filter_order Even filter order (between 2..8)
     * @param sampling_freq Sample frequency of the signal
     * @param cutoff_freq Cut-off frequency of the signal
     * @param src Source array
     * @param dest Destination array
     * @param size Size of both source and destination arrays
     */
    static void butterworth_highpass(
        int filter_order,
        float sampling_freq,
        float cutoff_freq,
        const float *src,
        float *dest,
        size_t size)
    {
        int n_steps = filter_order / 2;
        float a = tan(M_PI * cutoff_freq / sampling_freq);
        float a2 = pow(a, 2);
        float *A = (float*)ei_calloc(n_steps, sizeof(float));
        float *d1 = (float*)ei_calloc(n_steps, sizeof(float));
        float *d2 = (float*)ei_calloc(n_steps, sizeof(float));
        float *w0 = (float*)ei_calloc(n_steps, sizeof(float));
        float *w1 = (float*)ei_calloc(n_steps, sizeof(float));
        float *w2 = (float*)ei_calloc(n_steps, sizeof(float));

        // Calculate the filter parameters
        for (int ix = 0; ix < n_steps; ix++) {
            float r = sin(M_PI * ((2.0 * ix) + 1.0) / (2.0 * filter_order));
            sampling_freq = a2 + (2.0 * a * r) + 1.0;
            A[ix] = 1.0f / sampling_freq;
            d1[ix] = 2.0 * (1 - a2) / sampling_freq;
            d2[ix] = -(a2 - (2.0 * a * r) + 1.0) / sampling_freq;
        }

        // Apply the filter
        for (size_t sx = 0; sx < size; sx++) {
            dest[sx] = src[sx];

            for (int i = 0; i < n_steps; i++) {
                w0[i] = d1[i] * w1[i] + d2[i] * w2[i] + dest[sx];
                dest[sx] = A[i] * (w0[i] - (2.0 * w1[i]) + w2[i]);
                w2[i] = w1[i];
                w1[i] = w0[i];
            }
        }

        ei_free(A);
        ei_free(d1);
        ei_free(d2);
        ei_free(w0);
        ei_free(w1);
        ei_free(w2);
    }

} // namespace filters
} // namespace spectral
} // namespace ei

#endif // _EIDSP_SPECTRAL_FILTERS_H_
