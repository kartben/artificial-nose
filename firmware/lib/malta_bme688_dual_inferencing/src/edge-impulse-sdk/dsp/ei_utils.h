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
#ifndef __EI_UTILS__H__
#define __EI_UTILS__H__

#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

// Stringify
#define ei_xstr(a) ei_str(a)
#define ei_str(a) #a

// Bit manipulation

//Set bit y (0-indexed) of x to '1' by generating a a mask with a '1' in the proper bit location and ORing x with the mask.
#define SET_BIT_POS(x,y) (x |= (1 << y))

//Set bit y (0-indexed) of x to '0' by generating a mask with a '0' in the y position and 1's elsewhere then ANDing the mask with x.
#define CLEAR_BIT_POS(x,y) (x &= ~(1<< y))

//Return '1' if the bit value at position y within x is '1' and '0' if it's 0 by ANDing x with a bit mask where the bit in y's position is '1' and '0' elsewhere and comparing it to all 0's.  Returns '1' in least significant bit position if the value of the bit is '1', '0' if it was '0'.
#define TEST_BIT_POS(x,y) ((0u == (x & (1<<y)))?0u:1u)

//Toggle bit y (0-index) of x to the inverse: '0' becomes '1', '1' becomes '0' by XORing x with a bitmask where the bit in position y is '1' and all others are '0'.
#define TOGGLE_BIT_POS(x,y) (x ^= (1<<y))

// Set the flag bits in word.
#define 	SET_BIT_MASK(y, flag)   ( y |= (flag) )

// Clear the flag bits in word.
#define 	CLEAR_BIT_MASK(y, flag)   ( y &= ~(flag) )

// Flip the flag bits in word.
#define 	TOGGLE_BIT_MASK(y, flag)   ( y ^= (flag) )

// Test whether all the flag bits in word are set.
#define 	TEST_BIT_MASK(y, flag)   ( ((y)&(flag)) == (flag) )

#define EI_TRY(x) { auto res = (x); if(res != EIDSP_OK) { return res; } }

#endif  //!__EI_UTILS__H__