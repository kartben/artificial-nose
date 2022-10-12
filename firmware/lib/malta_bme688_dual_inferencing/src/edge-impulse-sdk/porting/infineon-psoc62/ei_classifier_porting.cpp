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
#if EI_PORTING_INFINEONPSOC62 == 1

#include <stdarg.h>
#include <stdlib.h>
#include <cstdio>
#include "unistd.h"
#include "cyhal.h"
#include "cyhal_lptimer.h"

static bool timer_init = false;
static volatile uint64_t tick = 0;

static void systick_isr(void)
{
    tick++;
}

__attribute__((weak)) EI_IMPULSE_ERROR ei_run_impulse_check_canceled() {
    return EI_IMPULSE_OK;
}

/**
 * Cancelable sleep, can be triggered with signal from other thread
 */
__attribute__((weak)) EI_IMPULSE_ERROR ei_sleep(int32_t time_ms) {

    cyhal_system_delay_ms(time_ms);

    return EI_IMPULSE_OK;
}

uint64_t ei_read_timer_ms() {
    if(timer_init == false) {
        cyhal_clock_t clock;
        uint32_t freq;

        // get IMO clock frequency
        cyhal_clock_reserve(&clock, &CYHAL_CLOCK_IMO);
        freq = cyhal_clock_get_frequency(&clock);
        cyhal_clock_free(&clock);

        // set SysTick to 1 ms
        Cy_SysTick_Init(CY_SYSTICK_CLOCK_SOURCE_CLK_IMO, (freq / 1000) - 1);
        Cy_SysTick_SetCallback(0, systick_isr);
        timer_init = true;
        return 0;
    }

    return tick;
}

uint64_t ei_read_timer_us() {

    return ei_read_timer_ms() * 1000;
}

void ei_putchar(char c)
{
    putchar(c);
}

__attribute__((weak)) void ei_printf(const char *format, ...) {

    char buffer[256];
    va_list myargs;
    va_start(myargs, format);
    vsnprintf(buffer, 256, format, myargs);
    va_end(myargs);

    printf("%s", buffer);

    // Serial_Out(buffer, length);
}

__attribute__((weak)) void ei_printf_float(float f) {
    ei_printf("%f", f);
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

#endif // EI_PORTING_INFINEONPSOC62
