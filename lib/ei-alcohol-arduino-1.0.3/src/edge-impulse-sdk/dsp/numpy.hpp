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

#ifndef _EIDSP_NUMPY_H_
#define _EIDSP_NUMPY_H_

#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <cfloat>
#include "numpy_types.h"
#include "config.hpp"
#include "returntypes.hpp"
#include "memory.hpp"
#include "dct/fast-dct-fft.h"
#include "kissfft/kiss_fftr.h"
#if EIDSP_USE_CMSIS_DSP
#include "edge-impulse-sdk/CMSIS/DSP/Include/arm_math.h"
#endif

#ifdef __MBED__
#include "mbed.h"
#else
#include <functional>
#endif // __MBED__

namespace ei {

// lookup table for quantized values between 0.0f and 1.0f
static const float quantized_values_one_zero[] = { (0.0f / 1.0f), (1.0f / 100.0f), (2.0f / 100.0f), (3.0f / 100.0f), (4.0f / 100.0f), (1.0f / 22.0f), (1.0f / 21.0f), (1.0f / 20.0f), (1.0f / 19.0f), (1.0f / 18.0f), (1.0f / 17.0f), (6.0f / 100.0f), (1.0f / 16.0f), (1.0f / 15.0f), (7.0f / 100.0f), (1.0f / 14.0f), (1.0f / 13.0f), (8.0f / 100.0f), (1.0f / 12.0f), (9.0f / 100.0f), (1.0f / 11.0f), (2.0f / 21.0f), (1.0f / 10.0f), (2.0f / 19.0f), (11.0f / 100.0f), (1.0f / 9.0f), (2.0f / 17.0f), (12.0f / 100.0f), (1.0f / 8.0f), (13.0f / 100.0f), (2.0f / 15.0f), (3.0f / 22.0f), (14.0f / 100.0f), (1.0f / 7.0f), (3.0f / 20.0f), (2.0f / 13.0f), (3.0f / 19.0f), (16.0f / 100.0f), (1.0f / 6.0f), (17.0f / 100.0f), (3.0f / 17.0f), (18.0f / 100.0f), (2.0f / 11.0f), (3.0f / 16.0f), (19.0f / 100.0f), (4.0f / 21.0f), (1.0f / 5.0f), (21.0f / 100.0f), (4.0f / 19.0f), (3.0f / 14.0f), (22.0f / 100.0f), (2.0f / 9.0f), (5.0f / 22.0f), (23.0f / 100.0f), (3.0f / 13.0f), (4.0f / 17.0f), (5.0f / 21.0f), (24.0f / 100.0f), (1.0f / 4.0f), (26.0f / 100.0f), (5.0f / 19.0f), (4.0f / 15.0f), (27.0f / 100.0f), (3.0f / 11.0f), (5.0f / 18.0f), (28.0f / 100.0f), (2.0f / 7.0f), (29.0f / 100.0f), (5.0f / 17.0f), (3.0f / 10.0f), (4.0f / 13.0f), (31.0f / 100.0f), (5.0f / 16.0f), (6.0f / 19.0f), (7.0f / 22.0f), (32.0f / 100.0f), (33.0f / 100.0f), (1.0f / 3.0f), (34.0f / 100.0f), (7.0f / 20.0f), (6.0f / 17.0f), (5.0f / 14.0f), (36.0f / 100.0f), (4.0f / 11.0f), (7.0f / 19.0f), (37.0f / 100.0f), (3.0f / 8.0f), (38.0f / 100.0f), (8.0f / 21.0f), (5.0f / 13.0f), (7.0f / 18.0f), (39.0f / 100.0f), (2.0f / 5.0f), (9.0f / 22.0f), (41.0f / 100.0f), (7.0f / 17.0f), (5.0f / 12.0f), (42.0f / 100.0f), (8.0f / 19.0f), (3.0f / 7.0f), (43.0f / 100.0f), (7.0f / 16.0f), (44.0f / 100.0f), (4.0f / 9.0f), (9.0f / 20.0f), (5.0f / 11.0f), (46.0f / 100.0f), (6.0f / 13.0f), (7.0f / 15.0f), (47.0f / 100.0f), (8.0f / 17.0f), (9.0f / 19.0f), (10.0f / 21.0f), (48.0f / 100.0f), (49.0f / 100.0f), (1.0f / 2.0f), (51.0f / 100.0f), (52.0f / 100.0f), (11.0f / 21.0f), (10.0f / 19.0f), (9.0f / 17.0f), (53.0f / 100.0f), (8.0f / 15.0f), (7.0f / 13.0f), (54.0f / 100.0f), (6.0f / 11.0f), (11.0f / 20.0f), (5.0f / 9.0f), (56.0f / 100.0f), (9.0f / 16.0f), (57.0f / 100.0f), (4.0f / 7.0f), (11.0f / 19.0f), (58.0f / 100.0f), (7.0f / 12.0f), (10.0f / 17.0f), (59.0f / 100.0f), (13.0f / 22.0f), (3.0f / 5.0f), (61.0f / 100.0f), (11.0f / 18.0f), (8.0f / 13.0f), (13.0f / 21.0f), (62.0f / 100.0f), (5.0f / 8.0f), (63.0f / 100.0f), (12.0f / 19.0f), (7.0f / 11.0f), (64.0f / 100.0f), (9.0f / 14.0f), (11.0f / 17.0f), (13.0f / 20.0f), (66.0f / 100.0f), (2.0f / 3.0f), (67.0f / 100.0f), (68.0f / 100.0f), (15.0f / 22.0f), (13.0f / 19.0f), (11.0f / 16.0f), (69.0f / 100.0f), (9.0f / 13.0f), (7.0f / 10.0f), (12.0f / 17.0f), (71.0f / 100.0f), (5.0f / 7.0f), (72.0f / 100.0f), (13.0f / 18.0f), (8.0f / 11.0f), (73.0f / 100.0f), (11.0f / 15.0f), (14.0f / 19.0f), (74.0f / 100.0f), (3.0f / 4.0f), (76.0f / 100.0f), (16.0f / 21.0f), (13.0f / 17.0f), (10.0f / 13.0f), (77.0f / 100.0f), (17.0f / 22.0f), (7.0f / 9.0f), (78.0f / 100.0f), (11.0f / 14.0f), (15.0f / 19.0f), (79.0f / 100.0f), (4.0f / 5.0f), (17.0f / 21.0f), (81.0f / 100.0f), (13.0f / 16.0f), (9.0f / 11.0f), (82.0f / 100.0f), (14.0f / 17.0f), (83.0f / 100.0f), (5.0f / 6.0f), (84.0f / 100.0f), (16.0f / 19.0f), (11.0f / 13.0f), (17.0f / 20.0f), (6.0f / 7.0f), (86.0f / 100.0f), (19.0f / 22.0f), (13.0f / 15.0f), (87.0f / 100.0f), (7.0f / 8.0f), (88.0f / 100.0f), (15.0f / 17.0f), (8.0f / 9.0f), (89.0f / 100.0f), (17.0f / 19.0f), (9.0f / 10.0f), (19.0f / 21.0f), (10.0f / 11.0f), (91.0f / 100.0f), (11.0f / 12.0f), (92.0f / 100.0f), (12.0f / 13.0f), (13.0f / 14.0f), (93.0f / 100.0f), (14.0f / 15.0f), (15.0f / 16.0f), (94.0f / 100.0f), (16.0f / 17.0f), (17.0f / 18.0f), (18.0f / 19.0f), (19.0f / 20.0f), (20.0f / 21.0f), (21.0f / 22.0f), (96.0f / 100.0f), (97.0f / 100.0f), (98.0f / 100.0f), (99.0f / 100.0f), (1.0f / 1.0f) };

class numpy {
public:
    /**
     * Roll array elements along a given axis.
     * Elements that roll beyond the last position are re-introduced at the first.
     * @param input_array
     * @param input_array_size
     * @param shift The number of places by which elements are shifted.
     * @returns EIDSP_OK if OK
     */
    static int roll(float *input_array, size_t input_array_size, int shift) {
        if (shift < 0) {
            shift = input_array_size + shift;
        }

        if (shift == 0) {
            return EIDSP_OK;
        }

        // so we need to allocate a buffer of the size of shift...
        EI_DSP_MATRIX(shift_matrix, 1, shift);

        // we copy from the end of the buffer into the shift buffer
        memcpy(shift_matrix.buffer, input_array + input_array_size - shift, shift * sizeof(float));

        // now we do a memmove to shift the array
        memmove(input_array + shift, input_array, (input_array_size - shift) * sizeof(float));

        // and copy the shift buffer back to the beginning of the array
        memcpy(input_array, shift_matrix.buffer, shift * sizeof(float));

        return EIDSP_OK;
    }

    static float sum(float *input_array, size_t input_array_size) {
        float res = 0.0f;
        for (size_t ix = 0; ix < input_array_size; ix++) {
            res += input_array[ix];
        }
        return res;
    }

    /**
     * Multiply two matrices (MxN * NxK matrix)
     * @param matrix1 Pointer to matrix1 (MxN)
     * @param matrix2 Pointer to matrix2 (NxK)
     * @param out_matrix Pointer to out matrix (MxK)
     * @returns EIDSP_OK if OK
     */
    static int dot(matrix_t *matrix1, matrix_t *matrix2, matrix_t *out_matrix) {
        if (matrix1->cols != matrix2->rows) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        // no. of rows in matrix1 determines the
        if (matrix1->rows != out_matrix->rows || matrix2->cols != out_matrix->cols) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

#if EIDSP_USE_CMSIS_DSP
        const arm_matrix_instance_f32 m1 = { matrix1->rows, matrix1->cols, matrix1->buffer };
        const arm_matrix_instance_f32 m2 = { matrix2->rows, matrix2->cols, matrix2->buffer };
        arm_matrix_instance_f32 mo = { out_matrix->rows, out_matrix->cols, out_matrix->buffer };
        int status = arm_mat_mult_f32(&m1, &m2, &mo);
        if (status != ARM_MATH_SUCCESS) {
            EIDSP_ERR(status);
        }
#else
        memset(out_matrix->buffer, 0, out_matrix->rows * out_matrix->cols * sizeof(float));

        for (size_t i = 0; i < matrix1->rows; i++) {
            dot_by_row(i,
                matrix1->buffer + (i * matrix1->cols),
                matrix1->cols,
                matrix2,
                out_matrix);
        }
#endif

        return EIDSP_OK;
    }

    /**
     * Multiply two matrices (MxN * NxK matrix)
     * @param matrix1 Pointer to matrix1 (MxN)
     * @param matrix2 Pointer to quantized matrix2 (NxK)
     * @param out_matrix Pointer to out matrix (MxK)
     * @returns EIDSP_OK if OK
     */
    static int dot(matrix_t *matrix1,
                    quantized_matrix_t *matrix2,
                    matrix_t *out_matrix)
    {
        if (matrix1->cols != matrix2->rows) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        // no. of rows in matrix1 determines the
        if (matrix1->rows != out_matrix->rows || matrix2->cols != out_matrix->cols) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        memset(out_matrix->buffer, 0, out_matrix->rows * out_matrix->cols * sizeof(float));

        for (size_t i = 0; i < matrix1->rows; i++) {
            dot_by_row(i,
                matrix1->buffer + (i * matrix1->cols),
                matrix1->cols,
                matrix2,
                out_matrix);
        }

        return EIDSP_OK;
    }

    /**
     * Multiply two matrices lazily per row in matrix 1 (MxN * NxK matrix)
     * @param i matrix1 row index
     * @param row matrix1 row
     * @param matrix1_cols matrix1 row size (1xN)
     * @param matrix2 Pointer to matrix2 (NxK)
     * @param out_matrix Pointer to out matrix (MxK)
     * @returns EIDSP_OK if OK
     */
    static inline int dot_by_row(int i, float *row, size_t matrix1_cols, matrix_t *matrix2, matrix_t *out_matrix) {
        if (matrix1_cols != matrix2->rows) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

#if EIDSP_USE_CMSIS_DSP
        const arm_matrix_instance_f32 m1 = { 1, static_cast<uint16_t>(matrix1_cols), row };
        const arm_matrix_instance_f32 m2 = { matrix2->rows, matrix2->cols, matrix2->buffer };
        arm_matrix_instance_f32 mo = { 1, out_matrix->cols, out_matrix->buffer + (i * out_matrix->cols) };
        int status = arm_mat_mult_f32(&m1, &m2, &mo);
        if (status != ARM_MATH_SUCCESS) {
            EIDSP_ERR(status);
        }
#else
        for (size_t j = 0; j < matrix2->cols; j++) {
            for (size_t k = 0; k < matrix1_cols; k++) {
                out_matrix->buffer[i * matrix2->cols + j] +=
                    row[k] * matrix2->buffer[k * matrix2->cols + j];
            }
        }
#endif

        return EIDSP_OK;
    }

    /**
     * Multiply two matrices lazily per row in matrix 1 (MxN * NxK matrix)
     * @param i matrix1 row index
     * @param row matrix1 row
     * @param matrix1_cols matrix1 row size
     * @param matrix2 Pointer to matrix2 (NxK)
     * @param out_matrix Pointer to out matrix (MxK)
     * @returns EIDSP_OK if OK
     */
    static inline int dot_by_row(int i, float *row, size_t matrix1_cols,
        quantized_matrix_t *matrix2, matrix_t *out_matrix)
    {
        if (matrix1_cols != matrix2->rows) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

#if EIDSP_USE_CMSIS_DSP
        EI_DSP_MATRIX(dequantized_matrix, 1, matrix1_cols);
        if (!dequantized_matrix.buffer) {
            EIDSP_ERR(EIDSP_OUT_OF_MEM);
        }
#endif

        for (uint16_t j = 0; j < matrix2->cols; j++) {
#if EIDSP_USE_CMSIS_DSP
            for (uint16_t k = 0; k < matrix1_cols; k++) {
                dequantized_matrix.buffer[k] = matrix2->dequantization_fn(matrix2->buffer[k * matrix2->cols + j]);
            }

            float result;
            arm_dot_prod_f32(row, dequantized_matrix.buffer, matrix1_cols, &result);
            out_matrix->buffer[i * matrix2->cols + j] = result;
#else
            for (uint16_t k = 0; k < matrix1_cols; k++) {
                out_matrix->buffer[i * matrix2->cols + j] +=
                    row[k] * matrix2->dequantization_fn(matrix2->buffer[k * matrix2->cols + j]);
            }
#endif
        }

        return EIDSP_OK;
    }

    /**
     * Transpose an array in place (from MxN to NxM)
     * Note: this temporary allocates a copy of the matrix on the heap.
     * @param matrix
     * @param rows
     * @param columns
     * @returns EIDSP_OK if OK
     */
    static int transpose(matrix_t *matrix) {
        int r = transpose(matrix->buffer, matrix->cols, matrix->rows);
        if (r != 0) {
            return r;
        }

        uint16_t old_rows = matrix->rows;
        uint16_t old_cols = matrix->cols;

        matrix->rows = old_cols;
        matrix->cols = old_rows;

        return EIDSP_OK;
    }

    /**
     * Transpose an array in place (from MxN to NxM)
     * @param matrix
     * @param rows
     * @param columns
     * @returns EIDSP_OK if OK
     */
    static int transpose(float *matrix, int rows, int columns) {
        EI_DSP_MATRIX(temp_matrix, rows, columns);
        if (!temp_matrix.buffer) {
            EIDSP_ERR(EIDSP_OUT_OF_MEM);
        }

#if EIDSP_USE_CMSIS_DSP
        const arm_matrix_instance_f32 i_m = {
            static_cast<uint16_t>(columns),
            static_cast<uint16_t>(rows),
            matrix
        };
        arm_matrix_instance_f32 o_m = {
            static_cast<uint16_t>(rows),
            static_cast<uint16_t>(columns),
            temp_matrix.buffer
        };
        arm_status status = arm_mat_trans_f32(&i_m, &o_m);
        if (status != ARM_MATH_SUCCESS) {
            return status;
        }
#else
        for (int j = 0; j < rows; j++){
            for (int i = 0; i < columns; i++){
                temp_matrix.buffer[j * columns + i] = matrix[i * rows + j];
            }
        }
#endif

        memcpy(matrix, temp_matrix.buffer, rows * columns * sizeof(float));

        return EIDSP_OK;
    }

    /**
     * Transpose an array in place (from MxN to NxM)
     * Note: this temporary allocates a copy of the matrix on the heap.
     * @param matrix
     * @param rows
     * @param columns
     * @returns EIDSP_OK if OK
     */
    static int transpose(quantized_matrix_t *matrix) {
        int r = transpose(matrix->buffer, matrix->cols, matrix->rows);
        if (r != 0) {
            return r;
        }

        uint16_t old_rows = matrix->rows;
        uint16_t old_cols = matrix->cols;

        matrix->rows = old_cols;
        matrix->cols = old_rows;

        return EIDSP_OK;
    }

    /**
     * Transpose an array in place (from MxN to NxM)
     * @param matrix
     * @param rows
     * @param columns
     * @returns EIDSP_OK if OK
     */
    static int transpose(uint8_t *matrix, int rows, int columns) {
        // dequantization function is not used actually...
        EI_DSP_QUANTIZED_MATRIX(temp_matrix, rows, columns, &dequantize_zero_one);
        if (!temp_matrix.buffer) {
            EIDSP_ERR(EIDSP_OUT_OF_MEM);
        }

        for (int j = 0; j < rows; j++){
            for (int i = 0; i < columns; i++){
                temp_matrix.buffer[j * columns + i] = matrix[i * rows + j];
            }
        }

        memcpy(matrix, temp_matrix.buffer, rows * columns * sizeof(uint8_t));

        return EIDSP_OK;
    }

    /**
     * Return the Discrete Cosine Transform of arbitrary type sequence 2.
     * @param input Input array (of size N)
     * @param N number of items in input and output array
     * @returns EIDSP_OK if OK
     */
    static int dct2(float *input, size_t N, DCT_NORMALIZATION_MODE normalization = DCT_NORMALIZATION_NONE) {
        if (N == 0) {
            return EIDSP_OK;
        }

        int ret = ei::dct::transform(input, N);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(ret);
        }

        // for some reason the output is 2x too low...
        for (size_t ix = 0; ix < N; ix++) {
            input[ix] *= 2;
        }

        if (normalization == DCT_NORMALIZATION_ORTHO) {
            input[0] = input[0] * sqrt(1.0f / static_cast<float>(4 * N));
            for (size_t ix = 1; ix < N; ix++) {
                input[ix] = input[ix] * sqrt(1.0f / static_cast<float>(2 * N));
            }
        }

        return EIDSP_OK;
    }

    /**
     * Discrete Cosine Transform of arbitrary type sequence 2 on a matrix.
     * @param matrix
     * @returns EIDSP_OK if OK
     */
    static int dct2(matrix_t *matrix, DCT_NORMALIZATION_MODE normalization = DCT_NORMALIZATION_NONE) {
        for (size_t row = 0; row < matrix->rows; row++) {
            int r = dct2(matrix->buffer + (row * matrix->cols), matrix->cols, normalization);
            if (r != EIDSP_OK) {
                return r;
            }
        }

        return EIDSP_OK;
    }

    /**
     * Quantize a float value between zero and one
     * @param value Float value
     */
    static uint8_t quantize_zero_one(float value) {
        const size_t length = sizeof(quantized_values_one_zero) / sizeof(float);

        // look in the table
        for (size_t ix = 0; ix < length; ix++) {
            if (quantized_values_one_zero[ix] == value) return ix;
        }

        // no match?

        if (value < quantized_values_one_zero[0]) {
            return quantized_values_one_zero[0];
        }
        if (value > quantized_values_one_zero[length - 1]) {
            return quantized_values_one_zero[length - 1];
        }

        int lo = 0;
        int hi = length - 1;

        while (lo <= hi) {
            int mid = (hi + lo) / 2;

            if (value < quantized_values_one_zero[mid]) {
                hi = mid - 1;
            } else if (value > quantized_values_one_zero[mid]) {
                lo = mid + 1;
            } else {
                return quantized_values_one_zero[mid];
            }
        }

        // lo == hi + 1
        return (quantized_values_one_zero[lo] - value) < (value - quantized_values_one_zero[hi]) ?
            lo :
            hi;
    }

    /**
     * Dequantize a float value between zero and one
     * @param value
     */
    static float dequantize_zero_one(uint8_t value) {
        if (value > 247) value = 247;
        return quantized_values_one_zero[value];
    }

    /**
     * Pad an array.
     * Pads with the reflection of the vector mirrored along the edge of the array.
     * @param input Input matrix (MxN)
     * @param output Output matrix of size (M+pad_before+pad_after x N)
     * @param pad_before Number of items to pad before
     * @param pad_after Number of items to pad after
     * @returns 0 if OK
     */
    static int pad_1d_symmetric(matrix_t *input, matrix_t *output, uint16_t pad_before, uint16_t pad_after) {
        if (output->cols != input->cols) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        if (output->rows != input->rows + pad_before + pad_after) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        if (input->rows == 0) {
            EIDSP_ERR(EIDSP_INPUT_MATRIX_EMPTY);
        }

        int32_t pad_before_index = 0;
        bool pad_before_direction_up = true;

        for (int32_t ix = pad_before - 1; ix >= 0; ix--) {
            memcpy(output->buffer + (input->cols * ix),
                input->buffer + (pad_before_index * input->cols),
                input->cols * sizeof(float));

            if (pad_before_index == 0 && !pad_before_direction_up) {
                pad_before_direction_up = true;
            }
            else if (pad_before_index == input->rows - 1 && pad_before_direction_up) {
                pad_before_direction_up = false;
            }
            else if (pad_before_direction_up) {
                pad_before_index++;
            }
            else {
                pad_before_index--;
            }
        }

        memcpy(output->buffer + (input->cols * pad_before),
            input->buffer,
            input->rows * input->cols * sizeof(float));

        int32_t pad_after_index = input->rows - 1;
        bool pad_after_direction_up = false;

        for (int32_t ix = 0; ix < pad_after; ix++) {
            memcpy(output->buffer + (input->cols * (ix + pad_before + input->rows)),
                input->buffer + (pad_after_index * input->cols),
                input->cols * sizeof(float));

            if (pad_after_index == 0 && !pad_after_direction_up) {
                pad_after_direction_up = true;
            }
            else if (pad_after_index == input->rows - 1 && pad_after_direction_up) {
                pad_after_direction_up = false;
            }
            else if (pad_after_direction_up) {
                pad_after_index++;
            }
            else {
                pad_after_index--;
            }
        }

        return EIDSP_OK;
    }

    /**
     * Scale a matrix in place
     * @param matrix
     * @param scale
     * @returns 0 if OK
     */
    static int scale(matrix_t *matrix, float scale) {
        if (scale == 1.0f) return EIDSP_OK;

#if EIDSP_USE_CMSIS_DSP
        const arm_matrix_instance_f32 mi = { matrix->rows, matrix->cols, matrix->buffer };
        arm_matrix_instance_f32 mo = { matrix->rows, matrix->cols, matrix->buffer };
        int status = arm_mat_scale_f32(&mi, scale, &mo);
        if (status != ARM_MATH_SUCCESS) {
            return status;
        }
#else
        for (size_t ix = 0; ix < matrix->rows * matrix->cols; ix++) {
            matrix->buffer[ix] *= scale;
        }
#endif
        return EIDSP_OK;
    }

    /**
     * Scale a matrix in place, per row
     * @param matrix Input matrix (MxN)
     * @param scale_matrix Scale matrix (Mx1)
     * @returns 0 if OK
     */
    static int scale(matrix_t *matrix, matrix_t *scale_matrix) {
        if (matrix->rows != scale_matrix->rows) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        if (scale_matrix->cols != 1) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        for (size_t row = 0; row < matrix->rows; row++) {
            EI_DSP_MATRIX_B(temp, 1, matrix->cols, matrix->buffer + (row * matrix->cols));
            int ret = scale(&temp, scale_matrix->buffer[row]);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(ret);
            }
        }

        return EIDSP_OK;
    }

    /**
     * Add on matrix in place
     * @param matrix
     * @param addition
     * @returns 0 if OK
     */
    static int add(matrix_t *matrix, float addition) {
        for (size_t ix = 0; ix < matrix->rows * matrix->cols; ix++) {
            matrix->buffer[ix] += addition;
        }
        return EIDSP_OK;
    }

    /**
     * Add on a matrix in place, per row
     * @param matrix Input matrix (MxN)
     * @param add Scale matrix (Mx1)
     * @returns 0 if OK
     */
    static int add(matrix_t *matrix, matrix_t *add_matrix) {
        if (matrix->rows != add_matrix->rows) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        if (add_matrix->cols != 1) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        for (size_t row = 0; row < matrix->rows; row++) {
            EI_DSP_MATRIX_B(temp, 1, matrix->cols, matrix->buffer + (row * matrix->cols));
            int ret = add(&temp, add_matrix->buffer[row]);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(ret);
            }
        }

        return EIDSP_OK;
    }

    /**
     * Subtract from matrix in place
     * @param matrix
     * @param subtraction
     * @returns 0 if OK
     */
    static int subtract(matrix_t *matrix, float subtraction) {
        for (size_t ix = 0; ix < matrix->rows * matrix->cols; ix++) {
            matrix->buffer[ix] -= subtraction;
        }
        return EIDSP_OK;
    }

    /**
     * Add on a matrix in place, per row
     * @param matrix Input matrix (MxN)
     * @param add Scale matrix (Mx1)
     * @returns 0 if OK
     */
    static int subtract(matrix_t *matrix, matrix_t *subtract_matrix) {
        if (matrix->rows != subtract_matrix->rows) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        if (subtract_matrix->cols != 1) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        for (size_t row = 0; row < matrix->rows; row++) {
            EI_DSP_MATRIX_B(temp, 1, matrix->cols, matrix->buffer + (row * matrix->cols));
            int ret = subtract(&temp, subtract_matrix->buffer[row]);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(ret);
            }
        }

        return EIDSP_OK;
    }

    /**
     * Calculate the root mean square of a matrix, one per row
     * @param matrix Matrix of size (MxN)
     * @param output_matrix Matrix of size (Mx1)
     * @returns 0 if OK
     */
    static int rms(matrix_t *matrix, matrix_t *output_matrix) {
        if (matrix->rows != output_matrix->rows) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        if (output_matrix->cols != 1) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        for (size_t row = 0; row < matrix->rows; row++) {
#if EIDSP_USE_CMSIS_DSP
            float rms_result;
            arm_rms_f32(matrix->buffer + (row * matrix->cols), matrix->cols, &rms_result);
            output_matrix->buffer[row] = rms_result;
#else
            float sum = 0.0;
            for(size_t ix = 0; ix < matrix->cols; ix++) {
                float v = matrix->buffer[(row * matrix->cols) + ix];
                sum += v * v;
            }
            output_matrix->buffer[row] = sqrt(sum / static_cast<float>(matrix->cols));
#endif
        }

        return EIDSP_OK;
    }

    /**
     * Calculate the mean over a matrix per row
     * @param input_matrix Input matrix (MxN)
     * @param output_matrix Output matrix (Mx1)
     */
    static int mean(matrix_t *input_matrix, matrix_t *output_matrix) {
        if (input_matrix->rows != output_matrix->rows) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }
        if (output_matrix->cols != 1) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        for (size_t row = 0; row < input_matrix->rows; row++) {
#if EIDSP_USE_CMSIS_DSP
            float mean;
            arm_mean_f32(input_matrix->buffer + (row * input_matrix->cols), input_matrix->cols, &mean);
            output_matrix->buffer[row] = mean;
#else
            float sum = 0.0f;

            for (size_t col = 0; col < input_matrix->cols; col++) {
                sum += input_matrix->buffer[( row * input_matrix->cols ) + col];
            }

            output_matrix->buffer[row] = sum / input_matrix->cols;
#endif
        }

        return EIDSP_OK;
    }

    /**
     * Calculate the mean over a matrix on axis 0
     * @param input_matrix Input matrix (MxN)
     * @param output_matrix Output matrix (Nx1)
     * @returns 0 if OK
     */
    static int mean_axis0(matrix_t *input_matrix, matrix_t *output_matrix) {
        if (input_matrix->cols != output_matrix->rows) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        if (output_matrix->cols != 1) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        int ret = transpose(input_matrix);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(ret);
        }

        for (size_t row = 0; row < input_matrix->rows; row++) {
#if EIDSP_USE_CMSIS_DSP
            float mean;
            arm_mean_f32(input_matrix->buffer + (row * input_matrix->cols), input_matrix->cols, &mean);
            output_matrix->buffer[row] = mean;
#else

            float sum = 0.0f;

            for (size_t col = 0; col < input_matrix->cols; col++) {
                sum += input_matrix->buffer[( row * input_matrix->cols ) + col];
            }

            output_matrix->buffer[row] = sum / input_matrix->cols;
#endif
        }

        // retranspose
        ret = transpose(input_matrix);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(ret);
        }

        return EIDSP_OK;
    }

    /**
     * Calculate the standard deviation over a matrix on axis 0
     * @param input_matrix Input matrix (MxN)
     * @param output_matrix Output matrix (Nx1)
     * @returns 0 if OK
     */
    static int std_axis0(matrix_t *input_matrix, matrix_t *output_matrix) {
        if (input_matrix->cols != output_matrix->rows) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        if (output_matrix->cols != 1) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        int ret = transpose(input_matrix);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(ret);
        }


        for (size_t row = 0; row < input_matrix->rows; row++) {
#if EIDSP_USE_CMSIS_DSP
            float std;
            arm_std_f32(input_matrix->buffer + (row * input_matrix->cols), input_matrix->cols, &std);
            output_matrix->buffer[row] = std;
#else
            float sum = 0.0f;

            for (size_t col = 0; col < input_matrix->cols; col++) {
                sum += input_matrix->buffer[(row * input_matrix->cols) + col];
            }

            float mean = sum / input_matrix->cols;

            float std = 0.0f;

            for (size_t col = 0; col < input_matrix->cols; col++) {
                std += pow(input_matrix->buffer[(row * input_matrix->cols) + col] - mean, 2);
            }

            output_matrix->buffer[row] = sqrt(std / input_matrix->cols);
#endif
        }

        // retranspose
        ret = transpose(input_matrix);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(ret);
        }

        return EIDSP_OK;
    }

    /**
     * Get the minimum value in a matrix per row
     * @param input_matrix Input matrix (MxN)
     * @param output_matrix Output matrix (Mx1)
     */
    static int min(matrix_t *input_matrix, matrix_t *output_matrix) {
        if (input_matrix->rows != output_matrix->rows) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }
        if (output_matrix->cols != 1) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        for (size_t row = 0; row < input_matrix->rows; row++) {
#if EIDSP_USE_CMSIS_DSP
            float min;
            uint32_t ix;
            arm_min_f32(input_matrix->buffer + (row * input_matrix->cols), input_matrix->cols, &min, &ix);
            output_matrix->buffer[row] = min;
#else
            float min = FLT_MAX;

            for (size_t col = 0; col < input_matrix->cols; col++) {
                float v = input_matrix->buffer[( row * input_matrix->cols ) + col];
                if (v < min) {
                    min = v;
                }
            }

            output_matrix->buffer[row] = min;
#endif
        }

        return EIDSP_OK;
    }

    /**
     * Get the maximum value in a matrix per row
     * @param input_matrix Input matrix (MxN)
     * @param output_matrix Output matrix (Mx1)
     */
    static int max(matrix_t *input_matrix, matrix_t *output_matrix) {
        if (input_matrix->rows != output_matrix->rows) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }
        if (output_matrix->cols != 1) {
            EIDSP_ERR(EIDSP_MATRIX_SIZE_MISMATCH);
        }

        for (size_t row = 0; row < input_matrix->rows; row++) {
#if EIDSP_USE_CMSIS_DSP
            float max;
            uint32_t ix;
            arm_max_f32(input_matrix->buffer + (row * input_matrix->cols), input_matrix->cols, &max, &ix);
            output_matrix->buffer[row] = max;
#else
            float max = FLT_MIN;

            for (size_t col = 0; col < input_matrix->cols; col++) {
                float v = input_matrix->buffer[( row * input_matrix->cols ) + col];
                if (v > max) {
                    max = v;
                }
            }

            output_matrix->buffer[row] = max;
#endif
        }

        return EIDSP_OK;
    }

    /**
     * Compute the one-dimensional discrete Fourier Transform for real input.
     * This function computes the one-dimensional n-point discrete Fourier Transform (DFT) of
     * a real-valued array by means of an efficient algorithm called the Fast Fourier Transform (FFT).
     * @param src Source buffer
     * @param src_size Size of the source buffer
     * @param output Output buffer
     * @param output_size Size of the output buffer, should be n_fft / 2 + 1
     * @returns 0 if OK
     */
    static int rfft(const float *src, size_t src_size, float *output, size_t output_size, size_t n_fft) {
        size_t n_fft_out_features = (n_fft / 2) + 1;
        if (output_size != n_fft_out_features) {
            EIDSP_ERR(EIDSP_BUFFER_SIZE_MISMATCH);
        }

        // truncate if needed
        if (src_size > n_fft) {
            src_size = n_fft;
        }

        // declare input and output arrays
        EI_DSP_MATRIX(fft_input, 1, n_fft);
        if (!fft_input.buffer) {
            EIDSP_ERR(EIDSP_OUT_OF_MEM);
        }

        // copy from src to fft_input
        memcpy(fft_input.buffer, src, src_size * sizeof(float));
        // pad to the rigth with zeros
        memset(fft_input.buffer + src_size, 0, (n_fft - src_size) * sizeof(kiss_fft_scalar));

#if EIDSP_USE_CMSIS_DSP
        if (n_fft != 32 && n_fft != 64 && n_fft != 128 && n_fft != 256 &&
            n_fft != 512 && n_fft != 1024 && n_fft != 2048 && n_fft != 4096) {
            int ret = software_rfft(fft_input.buffer, output, n_fft, n_fft_out_features);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(ret);
            }
        }
        else {
            // hardware acceleration only works for the powers above...
            arm_rfft_fast_instance_f32 rfft_instance;
            arm_status status = arm_rfft_fast_init_f32(&rfft_instance, n_fft);
            if (status != ARM_MATH_SUCCESS) {
                return status;
            }

            EI_DSP_MATRIX(fft_output, 1, n_fft);
            if (!fft_output.buffer) {
                EIDSP_ERR(EIDSP_OUT_OF_MEM);
            }

            arm_rfft_fast_f32(&rfft_instance, fft_input.buffer, fft_output.buffer, 0);

            output[0] = fft_output.buffer[0];
            output[n_fft_out_features - 1] = fft_output.buffer[1];

            size_t fft_output_buffer_ix = 2;
            for (size_t ix = 1; ix < n_fft_out_features - 1; ix += 1) {
                float rms_result;
                arm_rms_f32(fft_output.buffer + fft_output_buffer_ix, 2, &rms_result);
                output[ix] = rms_result * sqrt(2);

                fft_output_buffer_ix += 2;
            }
        }
#else
        int ret = software_rfft(fft_input.buffer, output, n_fft, n_fft_out_features);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(ret);
        }
#endif

        return EIDSP_OK;
    }

    /**
     * Compute the one-dimensional discrete Fourier Transform for real input.
     * This function computes the one-dimensional n-point discrete Fourier Transform (DFT) of
     * a real-valued array by means of an efficient algorithm called the Fast Fourier Transform (FFT).
     * @param src Source buffer
     * @param src_size Size of the source buffer
     * @param output Output buffer
     * @param output_size Size of the output buffer, should be n_fft / 2 + 1
     * @returns 0 if OK
     */
    static int rfft(const float *src, size_t src_size, fft_complex_t *output, size_t output_size, size_t n_fft) {
        size_t n_fft_out_features = (n_fft / 2) + 1;
        if (output_size != n_fft_out_features) {
            EIDSP_ERR(EIDSP_BUFFER_SIZE_MISMATCH);
        }

        // truncate if needed
        if (src_size > n_fft) {
            src_size = n_fft;
        }

        // declare input and output arrays
        float *fft_input_buffer = NULL;
        if (src_size == n_fft) {
            fft_input_buffer = (float*)src;
        }

        EI_DSP_MATRIX_B(fft_input, 1, n_fft, fft_input_buffer);
        if (!fft_input.buffer) {
            EIDSP_ERR(EIDSP_OUT_OF_MEM);
        }

        if (!fft_input_buffer) {
            // copy from src to fft_input
            memcpy(fft_input.buffer, src, src_size * sizeof(float));
            // pad to the rigth with zeros
            memset(fft_input.buffer + src_size, 0, (n_fft - src_size) * sizeof(float));
        }

#if EIDSP_USE_CMSIS_DSP
        if (n_fft != 32 && n_fft != 64 && n_fft != 128 && n_fft != 256 &&
            n_fft != 512 && n_fft != 1024 && n_fft != 2048 && n_fft != 4096) {
            int ret = software_rfft(fft_input.buffer, output, n_fft, n_fft_out_features);
            if (ret != EIDSP_OK) {
                EIDSP_ERR(ret);
            }
        }
        else {
            // hardware acceleration only works for the powers above...
            arm_rfft_fast_instance_f32 rfft_instance;
            arm_status status = arm_rfft_fast_init_f32(&rfft_instance, n_fft);
            if (status != ARM_MATH_SUCCESS) {
                return status;
            }

            EI_DSP_MATRIX(fft_output, 1, n_fft);
            if (!fft_output.buffer) {
                EIDSP_ERR(EIDSP_OUT_OF_MEM);
            }

            arm_rfft_fast_f32(&rfft_instance, fft_input.buffer, fft_output.buffer, 0);

            output[0].r = fft_output.buffer[0];
            output[0].i = 0.0f;
            output[n_fft_out_features - 1].r = fft_output.buffer[1];
            output[n_fft_out_features - 1].i = 0.0f;

            size_t fft_output_buffer_ix = 2;
            for (size_t ix = 1; ix < n_fft_out_features - 1; ix += 1) {
                output[ix].r = fft_output.buffer[fft_output_buffer_ix];
                output[ix].i = fft_output.buffer[fft_output_buffer_ix + 1];

                fft_output_buffer_ix += 2;
            }
        }
#else
        int ret = software_rfft(fft_input.buffer, output, n_fft, n_fft_out_features);
        if (ret != EIDSP_OK) {
            EIDSP_ERR(ret);
        }
#endif

        return EIDSP_OK;
    }

    /**
     * Return evenly spaced numbers over a specified interval.
     * Returns num evenly spaced samples, calculated over the interval [start, stop].
     * The endpoint of the interval can optionally be excluded.
     *
     * Based on https://github.com/ntessore/algo/blob/master/linspace.c
     * Licensed in public domain (see LICENSE in repository above)
     *
     * @param start The starting value of the sequence.
     * @param stop The end value of the sequence.
     * @param number Number of samples to generate.
     * @param out Out array, with size `number`
     * @returns 0 if OK
     */
    static int linspace(float start, float stop, uint32_t number, float *out)
    {
        if (number < 1 || !out) {
            EIDSP_ERR(EIDSP_PARAMETER_INVALID);
        }

        if (number == 1) {
            out[0] = start;
            return EIDSP_OK;
        }

        // step size
        float step = (stop - start) / (number - 1);

        // do steps
        for (uint32_t ix = 0; ix < number - 1; ix++) {
            out[ix] = start + ix * step;
        }

        // last entry always stop
        out[number - 1] = stop;

        return EIDSP_OK;
    }

    /**
     * Convert an int16_t buffer into a float buffer
     * @param input
     * @param output
     * @param length
     * @returns 0 if OK
     */
    static int int16_to_float(const EIDSP_i16 *input, float *output, size_t length) {
#if EIDSP_USE_CMSIS_DSP
        arm_q15_to_float(input, output, length);
#else
        for (size_t ix = 0; ix < length; ix++) {
            output[ix] = (float)(input[ix]);
        }
#endif
        return EIDSP_OK;
    }

    /**
     * Convert an int8_t buffer into a float buffer
     * @param input
     * @param output
     * @param length
     * @returns 0 if OK
     */
    static int int8_to_float(const EIDSP_i8 *input, float *output, size_t length) {
#if EIDSP_USE_CMSIS_DSP
        arm_q7_to_float(input, output, length);
#else
        for (size_t ix = 0; ix < length; ix++) {
            output[ix] = (float)(input[ix]);
        }
#endif
        return EIDSP_OK;
    }

#if EIDSP_SIGNAL_C_FN_POINTER == 0
    /**
     * Create a signal structure from a buffer.
     * This is useful for data that you keep in memory anyway. If you need to load from
     * flash, then create the structure yourself.
     * @param data Buffer, make sure to keep this pointer alive
     * @param data_size Size of the buffer
     * @param signal Output signal
     * @returns EIDSP_OK if ok
     */
    static int signal_from_buffer(float *data, size_t data_size, signal_t *signal)
    {
        signal->total_length = data_size;
#ifdef __MBED__
        signal->get_data = callback(&numpy::signal_get_data, data);
#else
        signal->get_data = [data](size_t offset, size_t length, float *out_ptr) {
            return numpy::signal_get_data(data, offset, length, out_ptr);
        };
#endif
        return EIDSP_OK;
    }
#endif

    /**
     * > 50% faster then the math.h log() function
     * in return for a small loss in accuracy (0.00001 average diff with log())
     * From: https://stackoverflow.com/questions/39821367/very-fast-approximate-logarithm-natural-log-function-in-c/39822314#39822314
     * Licensed under the CC BY-SA 3.0
     * @param a Input number
     * @returns Natural log value of a
     */
    __attribute__((always_inline)) static inline float log(float a)
    {
        float m, r, s, t, i, f;
        int32_t e, g;

        g = (int32_t) * ((int32_t *)&a);
        e = (g - 0x3f2aaaab) & 0xff800000;
        g = g - e;
        m = (float) * ((float *)&g);
        i = (float)e * 1.19209290e-7f; // 0x1.0p-23
        /* m in [2/3, 4/3] */
        f = m - 1.0f;
        s = f * f;
        /* Compute log1p(f) for f in [-1/3, 1/3] */
        r = fmaf(0.230836749f, f, -0.279208571f); // 0x1.d8c0f0p-3, -0x1.1de8dap-2
        t = fmaf(0.331826031f, f, -0.498910338f); // 0x1.53ca34p-2, -0x1.fee25ap-2
        r = fmaf(r, s, t);
        r = fmaf(r, s, f);
        r = fmaf(i, 0.693147182f, r); // 0x1.62e430p-1 // log(2)

        return r;
    }

    /**
     * Calculate the natural log value of a matrix. Does an in-place replacement.
     * @param matrix Matrix (MxN)
     * @returns 0 if OK
     */
    static int log(matrix_t *matrix)
    {
        for (size_t ix = 0; ix < matrix->rows * matrix->cols; ix++) {
            matrix->buffer[ix] = numpy::log(matrix->buffer[ix]);
        }

        return EIDSP_OK;
    }

private:
    static int software_rfft(float *fft_input, float *output, size_t n_fft, size_t n_fft_out_features) {
        kiss_fft_cpx *fft_output = (kiss_fft_cpx*)ei_dsp_malloc(n_fft_out_features * sizeof(kiss_fft_cpx));
        if (!fft_output) {
            EIDSP_ERR(EIDSP_OUT_OF_MEM);
        }

        size_t kiss_fftr_mem_length;

        // create fftr context
        kiss_fftr_cfg cfg = kiss_fftr_alloc(n_fft, 0, NULL, NULL, &kiss_fftr_mem_length);
        if (!cfg) {
            ei_dsp_free(fft_output, n_fft_out_features * sizeof(kiss_fft_cpx));
            EIDSP_ERR(EIDSP_OUT_OF_MEM);
        }

        ei_dsp_register_alloc(kiss_fftr_mem_length);

        // execute the rfft operation
        kiss_fftr(cfg, fft_input, fft_output);

        // and write back to the output
        for (size_t ix = 0; ix < n_fft_out_features; ix++) {
            output[ix] = sqrt(pow(fft_output[ix].r, 2) + pow(fft_output[ix].i, 2));
        }

        ei_dsp_free(cfg, kiss_fftr_mem_length);
        ei_dsp_free(fft_output, n_fft_out_features * sizeof(kiss_fft_cpx));

        return EIDSP_OK;
    }

    static int software_rfft(float *fft_input, fft_complex_t *output, size_t n_fft, size_t n_fft_out_features)
    {
        // create fftr context
        size_t kiss_fftr_mem_length;

        kiss_fftr_cfg cfg = kiss_fftr_alloc(n_fft, 0, NULL, NULL, &kiss_fftr_mem_length);
        if (!cfg) {
            EIDSP_ERR(EIDSP_OUT_OF_MEM);
        }

        ei_dsp_register_alloc(kiss_fftr_mem_length);

        // execute the rfft operation
        kiss_fftr(cfg, fft_input, (kiss_fft_cpx*)output);

        ei_dsp_free(cfg, kiss_fftr_mem_length);

        return EIDSP_OK;
    }

    static int signal_get_data(float *in_buffer, size_t offset, size_t length, float *out_ptr)
    {
        memcpy(out_ptr, in_buffer + offset, length * sizeof(float));
        return 0;
    }
};

} // namespace ei

#endif // _EIDSP_NUMPY_H_
