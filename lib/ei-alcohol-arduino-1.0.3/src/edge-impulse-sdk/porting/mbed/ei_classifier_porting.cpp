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

#ifdef __MBED__

#include "mbed.h"
#include "us_ticker_api.h"
#include "../ei_classifier_porting.h"

using namespace rtos;

#ifdef ARDUINO
#if !defined(DEVICE_USTICKER) && !defined(DEVICE_LPTICKER)
#include <Arduino.h>
#endif
#define EI_WEAK_FN __attribute__((weak))
#else
#define EI_WEAK_FN __weak
#endif

EI_WEAK_FN EI_IMPULSE_ERROR ei_run_impulse_check_canceled() {
    return EI_IMPULSE_OK;
}

/**
 * Cancelable sleep, can be triggered with signal from other thread
 */
EI_WEAK_FN EI_IMPULSE_ERROR ei_sleep(int32_t time_ms) {
#if MBED_VERSION >= MBED_ENCODE_VERSION((5), (11), (0))
    ThisThread::sleep_for(time_ms);
#else
    wait_ms(time_ms);
#endif // MBED_VERSION >= MBED_ENCODE_VERSION((5), (11), (0))
    return EI_IMPULSE_OK;
}

uint64_t ei_read_timer_ms() {
#if DEVICE_USTICKER
    return us_ticker_read() / 1000L;
#elif DEVICE_LPTICKER
    return ei_read_timer_us() / 1000L;
#elif defined(ARDUINO)
    return millis();
#else
    #error "Target does not have DEVICE_LPTICKER nor DEVICE_USTICKER"
#endif
}

uint64_t ei_read_timer_us() {
#if DEVICE_USTICKER
    return us_ticker_read();
#elif DEVICE_LPTICKER
	const ticker_info_t *info = lp_ticker_get_info();
	uint32_t n_ticks = lp_ticker_read();
    return (uint64_t)n_ticks * (1000000UL / info->frequency);
#elif defined(ARDUINO)
    return micros();
#else
    #error "Target does not have DEVICE_LPTICKER nor DEVICE_USTICKER"
#endif
}

#endif // __MBED__

// Non-Mbed Arduino targets (should probably move to it's own file)
#if !defined(__MBED__) && defined(ARDUINO)

#include <Arduino.h>
#include "../ei_classifier_porting.h"

#define EI_WEAK_FN __attribute__((weak))

EI_WEAK_FN EI_IMPULSE_ERROR ei_run_impulse_check_canceled() {
    return EI_IMPULSE_OK;
}

EI_WEAK_FN EI_IMPULSE_ERROR ei_sleep(int32_t time_ms) {
    delay(time_ms);
    return EI_IMPULSE_OK;
}

uint64_t ei_read_timer_ms() {
    return millis();
}

uint64_t ei_read_timer_us() {
    return micros();
}

#endif // !defined(MBED) && defined(ARDUINO)
