/*
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an "AS
 * IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _EIDSP_NUMPY_TYPES_H_
#define _EIDSP_NUMPY_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#ifdef __cplusplus
#include <functional>
#include "edge-impulse-sdk/dsp/ei_vector.h"
#ifdef __MBED__
#include "mbed.h"
#endif // __MBED__
#endif // __cplusplus
#include "config.hpp"
#include "edge-impulse-sdk/dsp/returntypes.h"

#if EIDSP_TRACK_ALLOCATIONS
#include "memory.hpp"
#endif

#ifdef __cplusplus
namespace ei {
#endif // __cplusplus

typedef struct {
    float r;
    float i;
} fft_complex_t;

typedef struct {
    int32_t r;
    int32_t i;
} fft_complex_i32_t;
/**
 * A matrix structure that allocates a matrix on the **heap**.
 * Freeing happens by calling `delete` on the object or letting the object go out of scope.
 */
typedef struct ei_matrix {
    float *buffer;
    uint32_t rows;
    uint32_t cols;
    bool buffer_managed_by_me;

#if EIDSP_TRACK_ALLOCATIONS
    const char *_fn;
    const char *_file;
    int _line;
    uint32_t _originally_allocated_rows;
    uint32_t _originally_allocated_cols;
#endif

#ifdef __cplusplus
    /**
     * Create a new matrix
     * @param n_rows Number of rows
     * @param n_cols Number of columns
     * @param a_buffer Buffer, if not provided we'll alloc on the heap
     */
    ei_matrix(
        uint32_t n_rows,
        uint32_t n_cols,
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
            buffer = (float*)ei_calloc(n_rows * n_cols * sizeof(float), 1);
            buffer_managed_by_me = true;
        }
        rows = n_rows;
        cols = n_cols;

        if (!a_buffer) {
#if EIDSP_TRACK_ALLOCATIONS
            _fn = fn;
            _file = file;
            _line = line;
            _originally_allocated_rows = rows;
            _originally_allocated_cols = cols;
            if (_fn) {
                ei_dsp_register_matrix_alloc_internal(fn, file, line, rows, cols, sizeof(float), buffer);
            }
            else {
                ei_dsp_register_matrix_alloc(rows, cols, sizeof(float), buffer);
            }
#endif
        }
    }

    ~ei_matrix() {
        if (buffer && buffer_managed_by_me) {
            ei_free(buffer);

#if EIDSP_TRACK_ALLOCATIONS
            if (_fn) {
                ei_dsp_register_matrix_free_internal(_fn, _file, _line, _originally_allocated_rows,
                    _originally_allocated_cols, sizeof(float), buffer);
            }
            else {
                ei_dsp_register_matrix_free(_originally_allocated_rows, _originally_allocated_cols,
                    sizeof(float), buffer);
            }
#endif
        }
    }

    /**
     * @brief Get a pointer to the buffer advanced by n rows
     *
     * @param row Numer of rows to advance the returned buffer pointer
     * @return float* Pointer to the buffer at the start of row n
     */
    float *get_row_ptr(size_t row)
    {
        return buffer + row * cols;
    }

    ei_matrix(ei_vector<float> &in) : ei_matrix(1, in.size(), in.data()) {
    }
#endif // #ifdef __cplusplus
} matrix_t;


/**
 * A matrix structure that allocates a matrix on the **heap**.
 * Freeing happens by calling `delete` on the object or letting the object go out of scope.
 */
typedef struct ei_matrix_i8 {
    int8_t *buffer;
    uint32_t rows;
    uint32_t cols;
    bool buffer_managed_by_me;

#if EIDSP_TRACK_ALLOCATIONS
    const char *_fn;
    const char *_file;
    int _line;
    uint32_t _originally_allocated_rows;
    uint32_t _originally_allocated_cols;
#endif

#ifdef __cplusplus
    /**
     * Create a new matrix
     * @param n_rows Number of rows
     * @param n_cols Number of columns
     * @param a_buffer Buffer, if not provided we'll alloc on the heap
     */
    ei_matrix_i8(
        uint32_t n_rows,
        uint32_t n_cols,
        int8_t *a_buffer = NULL
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
            buffer = (int8_t*)ei_calloc(n_rows * n_cols * sizeof(int8_t), 1);
            buffer_managed_by_me = true;
        }
        rows = n_rows;
        cols = n_cols;

        if (!a_buffer) {
#if EIDSP_TRACK_ALLOCATIONS
            _fn = fn;
            _file = file;
            _line = line;
            _originally_allocated_rows = rows;
            _originally_allocated_cols = cols;
            if (_fn) {
                ei_dsp_register_matrix_alloc_internal(fn, file, line, rows, cols, sizeof(int8_t), buffer);
            }
            else {
                ei_dsp_register_matrix_alloc(rows, cols, sizeof(int8_t), buffer);
            }
#endif
        }
    }

    ~ei_matrix_i8() {
        if (buffer && buffer_managed_by_me) {
            ei_free(buffer);

#if EIDSP_TRACK_ALLOCATIONS
            if (_fn) {
                ei_dsp_register_matrix_free_internal(_fn, _file, _line, _originally_allocated_rows,
                    _originally_allocated_cols, sizeof(int8_t), buffer);
            }
            else {
                ei_dsp_register_matrix_free(_originally_allocated_rows, _originally_allocated_cols,
                    sizeof(int8_t), buffer);
            }
#endif
        }
    }

    /**
     * @brief Get a pointer to the buffer advanced by n rows
     *
     * @param row Numer of rows to advance the returned buffer pointer
     * @return float* Pointer to the buffer at the start of row n
     */
    int8_t *get_row_ptr(size_t row)
    {
        return buffer + row * cols;
    }

#endif // #ifdef __cplusplus
} matrix_i8_t;

/**
 * A matrix structure that allocates a matrix on the **heap**.
 * Freeing happens by calling `delete` on the object or letting the object go out of scope.
 */
typedef struct ei_matrix_i32 {
    int32_t *buffer;
    uint32_t rows;
    uint32_t cols;
    bool buffer_managed_by_me;

#if EIDSP_TRACK_ALLOCATIONS
    const char *_fn;
    const char *_file;
    int _line;
    uint32_t _originally_allocated_rows;
    uint32_t _originally_allocated_cols;
#endif

#ifdef __cplusplus
    /**
     * Create a new matrix
     * @param n_rows Number of rows
     * @param n_cols Number of columns
     * @param a_buffer Buffer, if not provided we'll alloc on the heap
     */
    ei_matrix_i32(
        uint32_t n_rows,
        uint32_t n_cols,
        int32_t *a_buffer = NULL
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
            buffer = (int32_t*)ei_calloc(n_rows * n_cols * sizeof(int32_t), 1);
            buffer_managed_by_me = true;
        }
        rows = n_rows;
        cols = n_cols;

        if (!a_buffer) {
#if EIDSP_TRACK_ALLOCATIONS
            _fn = fn;
            _file = file;
            _line = line;
            _originally_allocated_rows = rows;
            _originally_allocated_cols = cols;
            if (_fn) {
                ei_dsp_register_matrix_alloc_internal(fn, file, line, rows, cols, sizeof(int32_t), buffer);
            }
            else {
                ei_dsp_register_matrix_alloc(rows, cols, sizeof(int32_t), buffer);
            }
#endif
        }
    }

    ~ei_matrix_i32() {
        if (buffer && buffer_managed_by_me) {
            ei_free(buffer);

#if EIDSP_TRACK_ALLOCATIONS
            if (_fn) {
                ei_dsp_register_matrix_free_internal(_fn, _file, _line, _originally_allocated_rows,
                    _originally_allocated_cols, sizeof(int32_t), buffer);
            }
            else {
                ei_dsp_register_matrix_free(_originally_allocated_rows, _originally_allocated_cols,
                    sizeof(int32_t), buffer);
            }
#endif
        }
    }

    /**
     * @brief Get a pointer to the buffer advanced by n rows
     *
     * @param row Numer of rows to advance the returned buffer pointer
     * @return float* Pointer to the buffer at the start of row n
     */
    int32_t *get_row_ptr(size_t row)
    {
        return buffer + row * cols;
    }

#endif // #ifdef __cplusplus
} matrix_i32_t;

/**
 * Another matrix structure that allocates a matrix on the **heap**.
 * Freeing happens by calling `delete` on the object or letting the object go out of scope.
 * We use this for the filterbanks, as we quantize these operations to save memory.
 */
typedef struct ei_quantized_matrix {
    uint8_t *buffer;
    uint32_t rows;
    uint32_t cols;
    bool buffer_managed_by_me;
#ifdef __MBED__
    mbed::Callback<float(uint8_t)> dequantization_fn;
#else
    float (*dequantization_fn)(uint8_t);
#endif

#if EIDSP_TRACK_ALLOCATIONS
    const char *_fn;
    const char *_file;
    int _line;
    uint32_t _originally_allocated_rows;
    uint32_t _originally_allocated_cols;
#endif

#ifdef __cplusplus
    /**
     * Create a quantized matrix
     * @param n_rows Number of rows
     * @param n_cols Number of columns
     * @param a_dequantization_fn How to dequantize the values in this matrix
     * @param a_buffer Optional: a buffer, if set we won't allocate memory ourselves
     */
    ei_quantized_matrix(uint32_t n_rows,
                        uint32_t n_cols,
#ifdef __MBED__
                        mbed::Callback<float(uint8_t)> a_dequantization_fn,
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
            buffer = (uint8_t*)ei_calloc(n_rows * n_cols * sizeof(uint8_t), 1);
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
            _originally_allocated_rows = rows;
            _originally_allocated_cols = cols;
            if (_fn) {
                ei_dsp_register_matrix_alloc_internal(fn, file, line, rows, cols, sizeof(uint8_t), buffer);
            }
            else {
                ei_dsp_register_matrix_alloc(rows, cols, sizeof(uint8_t), buffer);
            }
#endif
        }
    }

    ~ei_quantized_matrix() {
        if (buffer && buffer_managed_by_me) {
            ei_free(buffer);

#if EIDSP_TRACK_ALLOCATIONS
            if (_fn) {
                ei_dsp_register_matrix_free_internal(_fn, _file, _line, _originally_allocated_rows,
                    _originally_allocated_cols, sizeof(uint8_t), buffer);
            }
            else {
                ei_dsp_register_matrix_free(_originally_allocated_rows, _originally_allocated_cols,
                    sizeof(uint8_t), buffer);
            }
#endif
        }
    }

    /**
     * @brief Get a pointer to the buffer advanced by n rows
     *
     * @param row Numer of rows to advance the returned buffer pointer
     * @return float* Pointer to the buffer at the start of row n
     */
    uint8_t *get_row_ptr(size_t row)
    {
        return buffer + row * cols;
    }

#endif // #ifdef __cplusplus
} quantized_matrix_t;

/**
 * A matrix structure that allocates a matrix on the **heap**.
 * Freeing happens by calling `delete` on the object or letting the object go out of scope.
 */
typedef struct ei_matrix_u8 {
    uint8_t *buffer;
    uint32_t rows;
    uint32_t cols;
    bool buffer_managed_by_me;

#if EIDSP_TRACK_ALLOCATIONS
    const char *_fn;
    const char *_file;
    int _line;
    uint32_t _originally_allocated_rows;
    uint32_t _originally_allocated_cols;
#endif

#ifdef __cplusplus
    /**
     * Create a new matrix
     * @param n_rows Number of rows
     * @param n_cols Number of columns
     * @param a_buffer Buffer, if not provided we'll alloc on the heap
     */
    ei_matrix_u8(
        uint32_t n_rows,
        uint32_t n_cols,
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
            buffer = (uint8_t*)ei_calloc(n_rows * n_cols * sizeof(uint8_t), 1);
            buffer_managed_by_me = true;
        }
        rows = n_rows;
        cols = n_cols;

        if (!a_buffer) {
#if EIDSP_TRACK_ALLOCATIONS
            _fn = fn;
            _file = file;
            _line = line;
            _originally_allocated_rows = rows;
            _originally_allocated_cols = cols;
            if (_fn) {
                ei_dsp_register_matrix_alloc_internal(fn, file, line, rows, cols, sizeof(uint8_t), buffer);
            }
            else {
                ei_dsp_register_matrix_alloc(rows, cols, sizeof(uint8_t), buffer);
            }
#endif
        }
    }

    ~ei_matrix_u8() {
        if (buffer && buffer_managed_by_me) {
            ei_free(buffer);

#if EIDSP_TRACK_ALLOCATIONS
            if (_fn) {
                ei_dsp_register_matrix_free_internal(_fn, _file, _line, _originally_allocated_rows,
                    _originally_allocated_cols, sizeof(uint8_t), buffer);
            }
            else {
                ei_dsp_register_matrix_free(_originally_allocated_rows, _originally_allocated_cols,
                    sizeof(uint8_t), buffer);
            }
#endif
        }
    }

    /**
     * @brief Get a pointer to the buffer advanced by n rows
     *
     * @param row Numer of rows to advance the returned buffer pointer
     * @return float* Pointer to the buffer at the start of row n
     */
    uint8_t *get_row_ptr(size_t row)
    {
        return buffer + row * cols;
    }

#endif // #ifdef __cplusplus
} matrix_u8_t;

/**
 * Size of a matrix
 */
typedef struct {
    uint32_t rows;
    uint32_t cols;
} matrix_size_t;

typedef enum {
    DCT_NORMALIZATION_NONE,
    DCT_NORMALIZATION_ORTHO
} DCT_NORMALIZATION_MODE;

/**
 * @addtogroup ei_structs
 * @{
 */

/**
 * @brief Holds the callback pointer for retrieving raw data and the length 
 *  of data to be retrieved.
 * 
 *  Holds the callback function, `get_data(size_t offset, size_t length, float 
 *  *out_ptr)`. This callback should be implemented by the user and fills the memory
 *  location given by `*out_ptr` with raw features. Features must be flattened to a
 *  1-dimensional vector, as described in 
 *  [this guide](https://docs.edgeimpulse.com/docs/deploy-your-model-as-a-c-library#signal-structure).
 * 
 *  `get_data()` may be called multiple times during preprocessing or inference (e.g.
 *  during execution of 
 *  [run_classifier()](https://docs.edgeimpulse.com/reference/run_classifier) or
 *  [run_classifier_continuous()](https://docs.edgeimpulse.com/reference/run_classifier_continuous)). 
 *  The `offset` argument will update to point to new data, and `length` data must 
 *  be copied into the location specified by `out_ptr`. This scheme allows raw features
 *  to be stored in RAM or flash memory and paged in as necessary.
 * 
 *  Note that `get_data()` (even after multiple calls during a single execution of
 *  `run_classifier()` or `run_classifier_continuous()`) will never request more than a
 *  total number of features as given by `total_length`.
 * 
 * **Source**: [dsp/numpy_types.h](https://github.com/edgeimpulse/inferencing-sdk-cpp/blob/master/dsp/numpy_types.h)
 * 
 * **Example**: [standalone inferencing main.cpp](https://github.com/edgeimpulse/example-standalone-inferencing/blob/master/source/main.cpp)
 */
typedef struct ei_signal_t {
    /**
     * Callback function to be implemented by the user. Parameters are given as 
     * `get_data(size_t offset, size_t length, float *out_ptr)` and should return an 
     * int (e.g. `EIDSP_OK` if copying completed successfully). No bytes will be
     * requested outside of the `total_length`.
     * Callback parameters:  
     * `offset`: The offset in the signal  
     * `length`: The number of samples to write into `out_ptr`  
     * `out_ptr`: An out buffer to set the signal data  
    */
#if EIDSP_SIGNAL_C_FN_POINTER == 1
    int (*get_data)(size_t, size_t, float *);
#else
#ifdef __MBED__
    mbed::Callback<int(size_t offset, size_t length, float *out_ptr)> get_data;
#else
    std::function<int(size_t offset, size_t length, float *out_ptr)> get_data;
#endif // __MBED__
#endif // EIDSP_SIGNAL_C_FN_POINTER == 1

    /**
     * Total number of samples the user will provide (via get_data).  This value should match either the total number of raw features required for a full window (ie, the window size in Studio, but in samples), OR, if using run_classifier_continuous(), the number of samples in a single slice)
     *  for a new slice (`run_classifier_continuous()`) in order to perform 
     *  preprocessing and inference.
    */
    size_t total_length;
} signal_t;

/** @} */

#ifdef __cplusplus
} // namespace ei {
#endif // __cplusplus

// required on Adafruit nRF52
#if defined(__cplusplus) && defined(ARDUINO_NRF52_ADAFRUIT)
namespace std {
    __attribute__((weak)) void __throw_bad_function_call() { while(1); };
    __attribute__((weak)) void __throw_length_error(char const*) { while(1); };
}
#endif // __cplusplus

#endif // _EIDSP_NUMPY_TYPES_H_
