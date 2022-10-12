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

#include "../ei_classifier_porting.h"
#if EI_PORTING_SEEED_VISION_AI == 1

/* Include ----------------------------------------------------------------- */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
// #include "hx_drv_tflm.h"
#include "hx_drv_timer.h"
#include <math.h>

#include "embARC_debug.h"


/* Constants ---------------------------------------------------------------- */
#define HIMAX_TIMER_CLK_FREQ_HZ  400000000
#define HIMAX_TIMER_TICK_1SEC    (HIMAX_TIMER_CLK_FREQ_HZ/1)
#define HIMAX_TIMER_TICK_1MSEC   (HIMAX_TIMER_TICK_1SEC/1000)

/* Private variables -------------------------------------------------------- */

__attribute__((weak)) EI_IMPULSE_ERROR ei_run_impulse_check_canceled() {
    return EI_IMPULSE_OK;
}

/**
 * Cancelable sleep, can be triggered with signal from other thread
 */
__attribute__((weak)) EI_IMPULSE_ERROR ei_sleep(int32_t time_ms) {

    board_delay_ms(time_ms);

    return EI_IMPULSE_OK;
}

// Should be called at least once every ~10.7 seconds
uint64_t ei_read_timer_ms()
{
    static uint64_t system_time_ms = 0;
    static uint64_t prev_tick_us = 0;
    uint64_t tick_us;
    int64_t diff_tick_us, elapsed_time_ms;

    tick_us = board_get_cur_us();
    diff_tick_us = tick_us - prev_tick_us;
    elapsed_time_ms = diff_tick_us / 1000;

    // update system time and previous tick reference
    if (elapsed_time_ms > 0) {
        system_time_ms += elapsed_time_ms;
        prev_tick_us = tick_us;
    }
    else {
        ei_printf("TMR: tmr overflow!\n");
    }

    return system_time_ms;
}

uint64_t ei_read_timer_us()
{
    return board_get_cur_us();
}

void ei_serial_set_baudrate(int baudrate)
{
    // hx_drv_uart_initial((HX_DRV_UART_BAUDRATE_E)baudrate);
}

void ei_putchar(char c)
{
    /* Send char to serial output */
    ei_printf("%c", c);
}

__attribute__((weak)) void ei_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    // print_out(format, args);
    xvprintf(format, args);
    va_end(args);
}

__attribute__((weak)) void ei_printf_float(float f) {
    float n = f;

    static double PRECISION = 0.00001;
    static int MAX_NUMBER_STRING_SIZE = 32;

    char s[MAX_NUMBER_STRING_SIZE];

    if (n == 0.0) {
        ei_printf("0.00000");
    } else {
        int digit, m;  //, m1;
        char *c = s;
        int neg = (n < 0);
        if (neg) {
            n = -n;
        }
        // calculate magnitude
        m = log10(n);
        if (neg) {
            *(c++) = '-';
        }
        if (m < 1.0) {
            m = 0;
        }
        // convert the number
        while (n > PRECISION || m >= 0) {
            double weight = pow(10.0, m);
            if (weight > 0 && !isinf(weight)) {
                digit = floor(n / weight);
                n -= (digit * weight);
                *(c++) = '0' + digit;
            }
            if (m == 0 && n > 0) {
                *(c++) = '.';
            }
            m--;
        }
        *(c) = '\0';
        ei_printf("%s", s);
    }
}

__attribute__((weak)) void *ei_malloc(size_t size) {
    return malloc(size);
}

__attribute__((weak)) void *ei_calloc(size_t nitems, size_t size) {
    return calloc(nitems, size);
}

__attribute__((weak)) void ei_free(void *ptr) {
    free(ptr);
}

#if defined(__cplusplus) && EI_C_LINKAGE == 1
extern "C"
#endif
__attribute__((weak)) void DebugLog(const char* s) {
    ei_printf("%s", s);
}

#endif // #if EI_PORTING_SEEED_VISION_AI == 1
