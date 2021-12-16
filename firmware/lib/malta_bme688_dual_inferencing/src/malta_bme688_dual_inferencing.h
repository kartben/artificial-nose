/* Edge Impulse ingestion SDK
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

#ifndef _INFERENCE_H
#define _INFERENCE_H

// Undefine min/max macros as these conflict with C++ std min/max functions
// these are often included by Arduino cores
#include <Arduino.h>
#include <stdarg.h>
#ifdef min
#undef min
#endif // min
#ifdef max
#undef max
#endif // max
#ifdef round
#undef round
#endif // round
// Similar the ESP32 seems to define this, which is also used as an enum value in TFLite
#ifdef DEFAULT
#undef DEFAULT
#endif // DEFAULT
// Infineon core defines this, conflicts with CMSIS/DSP/Include/dsp/controller_functions.h
#ifdef A0
#undef A0
#endif // A0
#ifdef A1
#undef A1
#endif // A1
#ifdef A2
#undef A2
#endif // A2

/* Includes ---------------------------------------------------------------- */
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"
#include "model-parameters/model_metadata.h"

extern void ei_printf(const char *format, ...);

#endif // _INFERENCE_H
