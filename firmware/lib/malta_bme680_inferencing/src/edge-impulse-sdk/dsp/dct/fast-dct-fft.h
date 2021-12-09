/*
 * Fast discrete cosine transform algorithms (C)
 *
 * Copyright (c) 2018 Project Nayuki. (MIT License)
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

#ifndef __FAST_DCT_FFT__H__
#define __FAST_DCT_FFT__H__


#include <stdbool.h>
#include <stddef.h>
#include "../kissfft/kiss_fft.h"

namespace ei {
namespace dct {

int transform(float vector[], size_t len);
int inverse_transform(float vector[], size_t len);

} // namespace dct
} // namespace ei

#endif  //!__FAST-DCT-FFT__H__