/*
 * Fast discrete cosine transform algorithms (C)
 *
 * Copyright (c) 2017 Project Nayuki. (MIT License)
 * https://www.nayuki.io/page/fast-discrete-cosine-transform-algorithms
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "fast-dct-fft.h"
#include "../returntypes.hpp"
#include "../numpy.hpp"
#include "../memory.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif // M_PI

// DCT type II, unscaled
int ei::dct::transform(float vector[], size_t len) {
    const size_t fft_data_out_size = (len / 2 + 1) * sizeof(ei::fft_complex_t);
    const size_t fft_data_in_size = len * sizeof(float);

    // Allocate KissFFT input / output buffer
    fft_complex_t *fft_data_out =
        (ei::fft_complex_t*)ei_dsp_calloc(fft_data_out_size, 1);
    if (!fft_data_out) {
        return ei::EIDSP_OUT_OF_MEM;
    }

    float *fft_data_in = (float*)ei_dsp_calloc(fft_data_in_size, 1);
    if (!fft_data_in) {
        ei_dsp_free(fft_data_out, fft_data_out_size);
        return ei::EIDSP_OUT_OF_MEM;
    }

    // Preprocess the input buffer with the data from the vector
    size_t halfLen = len / 2;
    for (size_t i = 0; i < halfLen; i++) {
        fft_data_in[i] = vector[i * 2];
        fft_data_in[len - 1 - i] = vector[i * 2 + 1];
    }
    if (len % 2 == 1) {
        fft_data_in[halfLen] = vector[len - 1];
    }

    int r = ei::numpy::rfft(fft_data_in, len, fft_data_out, (len / 2 + 1), len);
    if (r != 0) {
        ei_dsp_free(fft_data_in, fft_data_in_size);
        ei_dsp_free(fft_data_out, fft_data_out_size);
        return r;
    }

    size_t i = 0;
    for (; i < len / 2 + 1; i++) {
        float temp = i * M_PI / (len * 2);
        vector[i] = fft_data_out[i].r * cos(temp) + fft_data_out[i].i * sin(temp);
    }
    //take advantage of hermetian symmetry to calculate remainder of signal
    for (; i < len; i++) {
        float temp = i * M_PI / (len * 2);
        int conj_idx = len-i;
        // second half bins not calculated would have just been the conjugate of the first half (note minus of imag)
        vector[i] = fft_data_out[conj_idx].r * cos(temp) - fft_data_out[conj_idx].i * sin(temp);
    }
    ei_dsp_free(fft_data_in, fft_data_in_size);
    ei_dsp_free(fft_data_out, fft_data_out_size);

    return 0;
}