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

#ifndef _EIDSP_RETURN_TYPES_H_
#define _EIDSP_RETURN_TYPES_H_

#include <stdint.h>

namespace ei {

typedef enum {
    EIDSP_OK = 0,
    EIDSP_OUT_OF_MEM = -1002,
    EIDSP_SIGNAL_SIZE_MISMATCH = -1003,
    EIDSP_MATRIX_SIZE_MISMATCH = -1004,
    EIDSP_DCT_ERROR = -1005,
    EIDSP_INPUT_MATRIX_EMPTY = -1006,
    EIDSP_BUFFER_SIZE_MISMATCH = -1007,
    EIDSP_PARAMETER_INVALID = -1008,
    EIDSP_UNEXPECTED_NEXT_OFFSET = -1009,
    EIDSP_OUT_OF_BOUNDS = -1010,
    EIDSP_UNSUPPORTED_FILTER_CONFIG = -1011,
    EIDSP_NARROWING = -1012,
    EIDSP_BLOCK_VERSION_INCORRECT = -1013,
    EIDSP_NOT_SUPPORTED = -1014
} EIDSP_RETURN_T;

} // namespace ei

#endif // _EIDSP_RETURN_TYPES_H_
