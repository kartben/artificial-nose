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

#ifndef _EDGE_IMPULSE_INFERENCING_ANOMALY_H_
#define _EDGE_IMPULSE_INFERENCING_ANOMALY_H_

#if (EI_CLASSIFIER_HAS_ANOMALY == 1)

#include <cmath>
#include <stdlib.h>

#include "edge-impulse-sdk/classifier/ei_classifier_types.h"
#include "edge-impulse-sdk/classifier/ei_aligned_malloc.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"

EI_IMPULSE_ERROR inference_anomaly_invoke(const ei_impulse_t *impulse,
                                          ei::matrix_t *fmatrix,
                                          ei_impulse_result_t *result,
                                          bool debug = false)
{

    uint64_t anomaly_start_ms = ei_read_timer_ms();

    float input[EI_CLASSIFIER_ANOM_AXIS_SIZE];
    for (size_t ix = 0; ix < EI_CLASSIFIER_ANOM_AXIS_SIZE; ix++) {
        input[ix] = fmatrix->buffer[EI_CLASSIFIER_ANOM_AXIS[ix]];
    }
    standard_scaler(input, ei_classifier_anom_scale, ei_classifier_anom_mean, EI_CLASSIFIER_ANOM_AXIS_SIZE);
    float anomaly = get_min_distance_to_cluster(
        input, EI_CLASSIFIER_ANOM_AXIS_SIZE, ei_classifier_anom_clusters, EI_CLASSIFIER_ANOM_CLUSTER_COUNT);

    uint64_t anomaly_end_ms = ei_read_timer_ms();

    if (debug) {
        ei_printf("Anomaly score (time: %d ms.): ", static_cast<int>(anomaly_end_ms - anomaly_start_ms));
        ei_printf_float(anomaly);
        ei_printf("\n");
    }

    result->timing.anomaly = anomaly_end_ms - anomaly_start_ms;

    result->anomaly = anomaly;

    return EI_IMPULSE_OK;
}

#endif //#if (EI_CLASSIFIER_HAS_ANOMALY == 1)
#endif // _EDGE_IMPULSE_INFERENCING_ANOMALY_H_
