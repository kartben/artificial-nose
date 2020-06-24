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

#ifndef _EIDSP_MEMORY_H_
#define _EIDSP_MEMORY_H_

#include <stdlib.h>

extern size_t ei_memory_in_use;
extern size_t ei_memory_peak_use;

namespace ei {

/**
 * These are macros used to track allocations when running DSP processes.
 * Enable memory tracking through the EIDSP_TRACK_ALLOCATIONS macro.
 */

#if EIDSP_TRACK_ALLOCATIONS
    /**
     * Register a manual allocation (malloc or calloc).
     * Typically you want to use ei::matrix_t types, as they keep track automatically.
     * @param bytes Number of bytes allocated
     */
    #define ei_dsp_register_alloc_internal(fn, file, line, bytes) \
        ei_memory_in_use += bytes; \
        if (ei_memory_in_use > ei_memory_peak_use) { \
            ei_memory_peak_use = ei_memory_in_use; \
        } \
        printf("alloc %lu bytes (in_use=%lu, peak=%lu) (%s@%s:%d)\n", \
            bytes, ei_memory_in_use, ei_memory_peak_use, fn, file, line);

    /**
     * Register a matrix allocation. Don't call this function yourself,
     * matrices already track this automatically.
     * @param rows Number of rows
     * @param cols Number of columns
     * @param type_size Size of the data type
     */
    #define ei_dsp_register_matrix_alloc_internal(fn, file, line, rows, cols, type_size) \
        ei_memory_in_use += (rows * cols * type_size); \
        if (ei_memory_in_use > ei_memory_peak_use) { \
            ei_memory_peak_use = ei_memory_in_use; \
        } \
        printf("alloc matrix %hu x %hu = %lu bytes (in_use=%lu, peak=%lu) (%s@%s:%d)\n", \
            rows, cols, rows * cols * type_size, ei_memory_in_use, ei_memory_peak_use, fn, file, line);

    /**
     * Register free'ing manually allocated memory (allocated through malloc/calloc)
     * @param bytes Number of bytes free'd
     */
    #define ei_dsp_register_free_internal(fn, file, line, bytes) \
        ei_memory_in_use -= bytes; \
        printf("free %lu bytes (in_use=%lu, peak=%lu) (%s@%s:%d)\n", \
            bytes, ei_memory_in_use, ei_memory_peak_use, fn, file, line);

    /**
     * Register a matrix free. Don't call this function yourself,
     * matrices already track this automatically.
     * @param rows Number of rows
     * @param cols Number of columns
     * @param type_size Size of the data type
     */
    #define ei_dsp_register_matrix_free_internal(fn, file, line, rows, cols, type_size) \
        ei_memory_in_use -= (rows * cols * type_size); \
        printf("free matrix %hu x %hu = %lu bytes (in_use=%lu, peak=%lu) (%s@%s:%d)\n", \
            rows, cols, rows * cols * type_size, ei_memory_in_use, ei_memory_peak_use, fn, file, line);

    #define ei_dsp_register_alloc(...) ei_dsp_register_alloc_internal(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define ei_dsp_register_matrix_alloc(...) ei_dsp_register_matrix_alloc_internal(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define ei_dsp_register_free(...) ei_dsp_register_free_internal(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define ei_dsp_register_matrix_free(...) ei_dsp_register_matrix_free_internal(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define ei_dsp_malloc(...) memory::ei_malloc(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define ei_dsp_calloc(...) memory::ei_calloc(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define ei_dsp_realloc(...) memory::ei_realloc(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define ei_dsp_free(...) memory::ei_free(__func__, __FILE__, __LINE__, __VA_ARGS__)
    #define EI_DSP_MATRIX(name, rows, cols) matrix_t name(rows, cols, NULL, __func__, __FILE__, __LINE__)
    #define EI_DSP_MATRIX_B(name, rows, cols, buffer) matrix_t name(rows, cols, buffer, __func__, __FILE__, __LINE__)
    #define EI_DSP_QUANTIZED_MATRIX(name, rows, cols, fn) quantized_matrix_t name(rows, cols, fn, NULL, __func__, __FILE__, __LINE__)
    #define EI_DSP_QUANTIZED_MATRIX_B(name, rows, cols, fn, buffer) quantized_matrix_t name(rows, cols, fn, buffer, __func__, __FILE__, __LINE__)
#else
    #define ei_dsp_register_alloc(...) (void)0
    #define ei_dsp_register_matrix_alloc(...) (void)0
    #define ei_dsp_register_free(...) (void)0
    #define ei_dsp_register_matrix_free(...) (void)0
    #define ei_dsp_malloc malloc
    #define ei_dsp_calloc calloc
    #define ei_dsp_realloc realloc
    #define ei_dsp_free(ptr, size) free(ptr)
    #define EI_DSP_MATRIX(name, ...) matrix_t name(__VA_ARGS__)
    #define EI_DSP_MATRIX_B(name, ...) matrix_t name(__VA_ARGS__)
    #define EI_DSP_QUANTIZED_MATRIX(name, ...) quantized_matrix_t name(__VA_ARGS__)
    #define EI_DSP_QUANTIZED_MATRIX_B(name, ...) quantized_matrix_t name(__VA_ARGS__)
#endif

#if EIDSP_TRACK_ALLOCATIONS
class memory {


public:
    /**
     * Allocate a new block of memory
     * @param size The size of the memory block, in bytes.
     */
    static void *ei_malloc(const char *fn, const char *file, int line, size_t size) {
        void *ptr = malloc(size);
        if (ptr) {
            ei_dsp_register_alloc_internal(fn, file, line, size);
        }
        return ptr;
    }

    /**
     * Allocates a block of memory for an array of num elements, each of them size bytes long,
     * and initializes all its bits to zero.
     * @param num Number of elements to allocate
     * @param size Size of each element
     */
    static void *ei_calloc(const char *fn, const char *file, int line, size_t num, size_t size) {
        void *ptr = calloc(num, size);
        if (ptr) {
            ei_dsp_register_alloc_internal(fn, file, line, num * size);
        }
        return ptr;
    }

    /**
     * Deallocate memory previously allocated by a call to calloc, malloc, or realloc.
     * @param ptr Pointer to a memory block previously allocated with malloc, calloc or realloc.
     * @param size Size of the block of memory previously allocated.
     */
    static void ei_free(const char *fn, const char *file, int line, void *ptr, size_t size) {
        free(ptr);
        ei_dsp_register_free_internal(fn, file, line, size);
    }

    /**
     * Reallocates the given area of memory.
     * @param ptr Pointer to the memory area to be reallocated.
     * @param old_size Size of the block of memory previously allocated.
     * @param new_size Size of the new block.
     */
    static void *ei_realloc(const char *fn, const char *file, int line, void *ptr, size_t old_size, size_t new_size) {
        void *new_ptr = realloc(ptr, new_size);
        if (new_ptr) {
            ei_dsp_register_free_internal(fn, file, line, old_size);
            ei_dsp_register_alloc_internal(fn, file, line, new_size);
        }
        return new_ptr;
    }
};
#endif // #if EIDSP_TRACK_ALLOCATIONS

} // namespace ei

#endif // _EIDSP_MEMORY_H_
