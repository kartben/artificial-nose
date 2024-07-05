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
#if EI_PORTING_ESPRESSIF == 1

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
// Include FreeRTOS for delay
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// for millis and micros
#include "esp_timer.h"
#include "esp_idf_version.h"

// memory handling
#include "esp_heap_caps.h"

#define EI_WEAK_FN __attribute__((weak))

EI_WEAK_FN EI_IMPULSE_ERROR ei_run_impulse_check_canceled() {
    return EI_IMPULSE_OK;
}

EI_WEAK_FN EI_IMPULSE_ERROR ei_sleep(int32_t time_ms) {
    vTaskDelay(time_ms / portTICK_RATE_MS);
    return EI_IMPULSE_OK;
}

uint64_t ei_read_timer_ms() {
    return esp_timer_get_time()/1000;
}

uint64_t ei_read_timer_us() {
    return esp_timer_get_time();
}

void ei_putchar(char c)
{
    /* Send char to serial output */
    putchar(c);
}

/**
 *  Printf function uses vsnprintf and output using USB Serial
 */
__attribute__((weak)) void ei_printf(const char *format, ...) {
    static char print_buf[1024] = { 0 };

    va_list args;
    va_start(args, format);
    int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
    va_end(args);

    if (r > 0) {
       printf(print_buf);
    }
}

__attribute__((weak)) void ei_printf_float(float f) {
    ei_printf("%f", f);
}

// we use alligned alloc instead of regular malloc
// due to https://github.com/espressif/esp-nn/issues/7
__attribute__((weak)) void *ei_malloc(size_t size) {
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 1)
    return heap_caps_aligned_alloc(16, size, MALLOC_CAP_DEFAULT);
#else
    return aligned_alloc(16, size);
#endif
#endif
    return malloc(size);
}

__attribute__((weak)) void *ei_calloc(size_t nitems, size_t size) {
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 1)
    return heap_caps_calloc(nitems, size, MALLOC_CAP_DEFAULT);
#else
    void *p;
    p = aligned_alloc(16, nitems * size);
    if (p == nullptr)
        return p;

    memset(p, '\0', nitems * size);
    return p;
#endif
#endif
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

#endif // EI_PORTING_ESPRESSIF == 1
