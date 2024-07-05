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

#include "../ei_classifier_porting.h"
#if EI_PORTING_HIMAX_WE2 == 1

/* Include ----------------------------------------------------------------- */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "xprintf.h"
extern "C" {
	#include "timer_interface.h"
};

__attribute__((weak)) EI_IMPULSE_ERROR ei_run_impulse_check_canceled() {
    return EI_IMPULSE_OK;
}

/**
 * Cancelable sleep, can be triggered with signal from other thread
 */
__attribute__((weak)) EI_IMPULSE_ERROR ei_sleep(int32_t time_ms) {
    hx_drv_timer_cm55x_delay_ms(time_ms, TIMER_STATE_DC);

    return EI_IMPULSE_OK;
}

// Should be called at least once every ~10.7 seconds
uint64_t ei_read_timer_ms()
{
    uint32_t tick, loop_cnt;
    SystemGetTick(&tick, &loop_cnt);

    // tick is counting down, so we need to add elapsed ticks to the total tick count
    uint64_t elapsed_ms = (uint64_t)loop_cnt * (uint64_t)(SysTick_LOAD_RELOAD_Msk+1) + (SysTick_LOAD_RELOAD_Msk + 1 - tick);
    // convert ticks to ms knowing the CPU frequency
    elapsed_ms = elapsed_ms / (SystemCoreClock / 1000);

    return elapsed_ms;
}

uint64_t ei_read_timer_us()
{
    uint32_t tick, loop_cnt;
    SystemGetTick(&tick, &loop_cnt);

    // tick is counting down, so we need to add elapsed ticks to the total tick count
    uint64_t elapsed_us = (uint64_t)loop_cnt * (uint64_t)(SysTick_LOAD_RELOAD_Msk+1) + (SysTick_LOAD_RELOAD_Msk + 1 - tick);
    // convert ticks to ms knowing the CPU frequency
    elapsed_us = elapsed_us / (SystemCoreClock / 1000000);

    return elapsed_us;
}

void ei_serial_set_baudrate(int baudrate)
{
    // hx_drv_uart_initial((HX_DRV_UART_BAUDRATE_E)baudrate);
}

void ei_putchar(char c)
{
    /* Send char to serial output */
    xputc(c);
}

__attribute__((weak)) void ei_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
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

#endif // #if EI_PORTING_HIMAX_WE2 == 1
