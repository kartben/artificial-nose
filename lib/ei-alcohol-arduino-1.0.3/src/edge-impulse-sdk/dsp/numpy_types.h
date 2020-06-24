/* Edge Impulse inferencing library
 * Copyright (c) 2020 EdgeImpulse Inc.
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

#ifndef _EIDSP_NUMPY_TYPES_H_
#define _EIDSP_NUMPY_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#ifdef __cplusplus
#include <functional>
#ifdef __MBED__
#include "mbed.h"
using namespace mbed;
#endif // __MBED__
#endif // __cplusplus

#ifdef __cplusplus
namespace ei {
#endif // __cplusplus

typedef struct {
    float r;
    float i;
} fft_complex_t;

/**
 * A matrix structure that allocates a matrix on the **heap**.
 * Freeing happens by calling `delete` on the object or letting the object go out of scope.
 */
typedef struct ei_matrix {
    float *buffer;
    uint16_t rows;
    uint16_t cols;
    bool buffer_managed_by_me;

#if EIDSP_TRACK_ALLOCATIONS
    const char *_fn;
    const char *_file;
    int _line;
#endif

#ifdef __cplusplus
    /**
     * Create a new matrix
     * @param n_rows Number of rows
     * @param n_cols Number of columns
     * @param a_buffer Buffer, if not provided we'll alloc on the heap
     */
    ei_matrix(
        uint16_t n_rows,
        uint16_t n_cols,
        float *a_buffer = NULL
#if EIDSP_TRACK_ALLOCATIONS
        ,
        const char *fn = NULL,
        const char *file = NULL,
        int line = 0
#endif
        )
    {
        if (a_buffer) {
            buffer = a_buffer;
            buffer_managed_by_me = false;
        }
        else {
            buffer = (float*)calloc(n_rows * n_cols * sizeof(float), 1);
            buffer_managed_by_me = true;
        }
        rows = n_rows;
        cols = n_cols;

        if (!a_buffer) {
#if EIDSP_TRACK_ALLOCATIONS
            _fn = fn;
            _file = file;
            _line = line;
            if (_fn) {
                ei_dsp_register_matrix_alloc_internal(fn, file, line, rows, cols, sizeof(float));
            }
            else {
                ei_dsp_register_matrix_alloc(rows, cols, sizeof(float));
            }
#endif
        }
    }

    ~ei_matrix() {
        if (buffer && buffer_managed_by_me) {
            free(buffer);

#if EIDSP_TRACK_ALLOCATIONS
            if (_fn) {
                ei_dsp_register_matrix_free_internal(_fn, _file, _line, rows, cols, sizeof(float));
            }
            else {
                ei_dsp_register_matrix_free(rows, cols, sizeof(float));
            }
#endif
        }
    }
#endif // #ifdef __cplusplus
} matrix_t;

/**
 * Another matrix structure that allocates a matrix on the **heap**.
 * Freeing happens by calling `delete` on the object or letting the object go out of scope.
 * We use this for the filterbanks, as we quantize these operations to save memory.
 */
typedef struct ei_quantized_matrix {
    uint8_t *buffer;
    uint16_t rows;
    uint16_t cols;
    bool buffer_managed_by_me;
#ifdef __MBED__
    Callback<float(uint8_t)> dequantization_fn;
#else
    float (*dequantization_fn)(uint8_t);
#endif

#if EIDSP_TRACK_ALLOCATIONS
    const char *_fn;
    const char *_file;
    int _line;
#endif

#ifdef __cplusplus
    /**
     * Create a quantized matrix
     * @param n_rows Number of rows
     * @param n_cols Number of columns
     * @param a_dequantization_fn How to dequantize the values in this matrix
     * @param a_buffer Optional: a buffer, if set we won't allocate memory ourselves
     */
    ei_quantized_matrix(uint16_t n_rows,
                        uint16_t n_cols,
#ifdef __MBED__
                        Callback<float(uint8_t)> a_dequantization_fn,
#else
                        float (*a_dequantization_fn)(uint8_t),
#endif
                        uint8_t *a_buffer = NULL
#if EIDSP_TRACK_ALLOCATIONS
                        ,
                        const char *fn = NULL,
                        const char *file = NULL,
                        int line = 0
#endif
                        )
    {
        if (a_buffer) {
            buffer = a_buffer;
            buffer_managed_by_me = false;
        }
        else {
            buffer = (uint8_t*)calloc(n_rows * n_cols * sizeof(uint8_t), 1);
            buffer_managed_by_me = true;
        }
        rows = n_rows;
        cols = n_cols;
        dequantization_fn = a_dequantization_fn;
        if (!a_buffer) {
#if EIDSP_TRACK_ALLOCATIONS
            _fn = fn;
            _file = file;
            _line = line;
            if (_fn) {
                ei_dsp_register_matrix_alloc_internal(fn, file, line, rows, cols, sizeof(uint8_t));
            }
            else {
                ei_dsp_register_matrix_alloc(rows, cols, sizeof(uint8_t));
            }
#endif
        }
    }

    ~ei_quantized_matrix() {
        if (buffer && buffer_managed_by_me) {
            free(buffer);

#if EIDSP_TRACK_ALLOCATIONS
            if (_fn) {
                ei_dsp_register_matrix_free_internal(_fn, _file, _line, rows, cols, sizeof(uint8_t));
            }
            else {
                ei_dsp_register_matrix_free(rows, cols, sizeof(uint8_t));
            }
#endif
        }
    }
#endif // #ifdef __cplusplus
} quantized_matrix_t;

/**
 * Size of a matrix
 */
typedef struct {
    uint16_t rows;
    uint16_t cols;
} matrix_size_t;

typedef enum {
    DCT_NORMALIZATION_NONE,
    DCT_NORMALIZATION_ORTHO
} DCT_NORMALIZATION_MODE;

/**
 * Sensor signal structure
 */
typedef struct ei_signal_t {
    /**
     * A function to retrieve part of the sensor signal
     * No bytes will be requested outside of the `total_length`.
     * @param offset The offset in the signal
     * @param length The total length of the signal
     * @param out_ptr An out buffer to set the signal data
     */
#if EIDSP_SIGNAL_C_FN_POINTER == 1
    int (*get_data)(size_t, size_t, float *);
#else
#ifdef __MBED__
    Callback<int(size_t offset, size_t length, float *out_ptr)> get_data;
#else
    std::function<int(size_t offset, size_t length, float *out_ptr)> get_data;
#endif // __MBED__
#endif // EIDSP_SIGNAL_C_FN_POINTER == 1

    size_t total_length;
} signal_t;

#ifdef __cplusplus
} // namespace ei {
#endif // __cplusplus

#endif // _EIDSP_NUMPY_TYPES_H_
