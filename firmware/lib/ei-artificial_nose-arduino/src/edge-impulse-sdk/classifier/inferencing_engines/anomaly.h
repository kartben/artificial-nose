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

#ifndef _EDGE_IMPULSE_INFERENCING_ANOMALY_H_
#define _EDGE_IMPULSE_INFERENCING_ANOMALY_H_

#if (EI_CLASSIFIER_HAS_ANOMALY)

#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "edge-impulse-sdk/classifier/ei_classifier_types.h"
#include "edge-impulse-sdk/classifier/ei_aligned_malloc.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "edge-impulse-sdk/classifier/inferencing_engines/engines.h"
#include "edge-impulse-sdk/classifier/ei_fill_result_struct.h"

#ifdef __cplusplus
namespace {
#endif // __cplusplus

/**
 * Standard scaler, scales all values in the input vector
 * Note that this *modifies* the array in place!
 * @param input Array of input values
 * @param scale Array of scale values (obtain from StandardScaler in Python)
 * @param mean Array of mean values (obtain from StandardScaler in Python)
 * @param input_size Size of input, scale and mean arrays
 */
void standard_scaler(float *input, const float *scale, const float *mean, size_t input_size) {
    for (size_t ix = 0; ix < input_size; ix++) {
        input[ix] = (input[ix] - mean[ix]) / scale[ix];
    }
}

/**
 * Calculate the distance between input vector and the cluster
 * @param input Array of input values (already scaled by standard_scaler)
 * @param input_size Size of the input array
 * @param cluster A cluster (number of centroids should match input_size)
 */
float calculate_cluster_distance(float *input, size_t input_size, const ei_classifier_anom_cluster_t *cluster) {
    // todo: check input_size and centroid size?

    float dist = 0.0f;
    for (size_t ix = 0; ix < input_size; ix++) {
        dist += pow(input[ix] - cluster->centroid[ix], 2);
    }
    return sqrt(dist) - cluster->max_error;
}

/**
 * Get minimum distance to a cluster
 * @param input Array of input values (already scaled by standard_scaler)
 * @param input_size Size of the input array
 * @param clusters Array of clusters
 * @param cluster_size Size of cluster array
 */
float get_min_distance_to_cluster(float *input, size_t input_size, const ei_classifier_anom_cluster_t *clusters, size_t cluster_size) {
    float min = 1000.0f;
    for (size_t ix = 0; ix < cluster_size; ix++) {
        float dist = calculate_cluster_distance(input, input_size, &clusters[ix]);
        if (dist < min) {
            min = dist;
        }
    }
    return min;
}

#ifdef __cplusplus
}
#endif // __cplusplus


/**
 * Extracts the input values from the feature matrix based on the anomaly axes.
 * @param fmatrix Feature matrix
 * @param input_block_ids Array of block IDs to extract from the feature matrix
 * @param input_block_ids_size Size of input_block_ids array
 * @param block_config Anomaly block configuration
 * @param input Array to store the extracted input values
 * @return EI_IMPULSE_OK if successful, otherwise an error code
 */
EI_IMPULSE_ERROR extract_anomaly_input_values(
    ei_feature_t *fmatrix,
    uint32_t* input_block_ids,
    uint32_t input_block_ids_size,
    uint32_t anom_axes_size,
    const uint16_t *anom_axis,
    float *input)
{
    if (input_block_ids_size == 1) {
        for (size_t ix = 0; ix < anom_axes_size; ix++) {
            input[ix] = fmatrix[0].matrix->buffer[anom_axis[ix]];
        }
    }
    else {
#if EI_CLASSIFIER_SINGLE_FEATURE_INPUT == 0
        ei::matrix_t* matrix = NULL;
#endif
        // tracks where we are now in the combined feature matrix
        uint32_t global_buf_pos = 0;
        // we add the size of passed matrix to it
        uint32_t buf_offset = 0;
        // current index of input feature
        uint32_t input_pos = 0;

        for (size_t i = 0; i < input_block_ids_size; i++) {
#if EI_CLASSIFIER_SINGLE_FEATURE_INPUT == 0
            size_t cur_mtx = input_block_ids[i];
            if (!find_mtx_by_idx(fmatrix, &matrix, cur_mtx, anom_axes_size)) {
                ei_printf("ERR: Cannot find matrix with id %zu\n", cur_mtx);
                return EI_IMPULSE_INVALID_SIZE;
            }
#else
            ei::matrix_t* matrix = fmatrix[0].matrix;
#endif
            for (size_t ix = 0; ix < anom_axes_size; ix++) {
                global_buf_pos = anom_axis[input_pos];
                if (global_buf_pos <= buf_offset + (matrix->rows * matrix->cols)) {
                    input[input_pos] = matrix->buffer[anom_axis[input_pos] - buf_offset];
                    input_pos++;
                if (input_pos >= anom_axes_size) { goto end; }
                }
                else {
                    break;
                }
            }
            buf_offset += matrix->rows * matrix->cols;
        }
        end:;
    }
    return EI_IMPULSE_OK;
}


EI_IMPULSE_ERROR run_kmeans_anomaly(
    const ei_impulse_t *impulse,
    ei_feature_t *fmatrix,
    uint32_t learn_block_index,
    uint32_t* input_block_ids,
    uint32_t input_block_ids_size,
    ei_impulse_result_t *result,
    void *config_ptr,
    bool debug = false)
{
    ei_learning_block_config_anomaly_kmeans_t *block_config = (ei_learning_block_config_anomaly_kmeans_t*)config_ptr;

    uint64_t anomaly_start_ms = ei_read_timer_ms();

    float *input = (float*)ei_malloc(block_config->anom_axes_size * sizeof(float));
    if (!input) {
        ei_printf("Failed to allocate memory for anomaly input buffer");
        return EI_IMPULSE_OUT_OF_MEMORY;
    }

    extract_anomaly_input_values(fmatrix, input_block_ids, input_block_ids_size, block_config->anom_axes_size, block_config->anom_axis, input);

    standard_scaler(input, block_config->anom_scale, block_config->anom_mean, block_config->anom_axes_size);
    float anomaly = get_min_distance_to_cluster(
        input, block_config->anom_axes_size, block_config->anom_clusters, block_config->anom_cluster_count);

    uint64_t anomaly_end_ms = ei_read_timer_ms();

    if (debug) {
        ei_printf("Anomaly score (time: %d ms.): ", static_cast<int>(anomaly_end_ms - anomaly_start_ms));
        ei_printf_float(anomaly);
        ei_printf("\n");
    }

    result->timing.anomaly = anomaly_end_ms - anomaly_start_ms;
    result->anomaly = anomaly;
    ei_free(input);

    return EI_IMPULSE_OK;
}

#if (EI_CLASSIFIER_INFERENCING_ENGINE != EI_CLASSIFIER_NONE)
EI_IMPULSE_ERROR run_gmm_anomaly(
    const ei_impulse_t *impulse,
    ei_feature_t *fmatrix,
    uint32_t learn_block_index,
    uint32_t* input_block_ids,
    uint32_t input_block_ids_size,
    ei_impulse_result_t *result,
    void *config_ptr,
    bool debug = false)
{
    ei_learning_block_config_anomaly_gmm_t *block_config = (ei_learning_block_config_anomaly_gmm_t*)config_ptr;

    ei_learning_block_config_tflite_graph_t ei_learning_block_config_gmm = {
        .implementation_version = 1,
        .classification_mode = block_config->classification_mode,
        .block_id = 0,
        .object_detection = 0,
        .object_detection_last_layer = EI_CLASSIFIER_LAST_LAYER_UNKNOWN,
        .output_data_tensor = 0,
        .output_labels_tensor = 0,
        .output_score_tensor = 0,
        .threshold = block_config->anomaly_threshold,
        .quantized = 0,
        .compiled = 0,
        .graph_config = block_config->graph_config
    };

    ei_impulse_result_t anomaly_result = { 0 };

    std::unique_ptr<ei_feature_t[]> input_ptr(new ei_feature_t[1]);
    ei_feature_t* input = input_ptr.get();

    memset(&anomaly_result, 0, sizeof(ei_impulse_result_t));

    std::unique_ptr<ei::matrix_t> matrix_ptr(new ei::matrix_t(1, block_config->anom_axes_size));

    if (block_config->classification_mode == EI_CLASSIFIER_CLASSIFICATION_MODE_VISUAL_ANOMALY) {
        // [JJ] Here we assume that the feature extractor block is always directly before the GMM block
        // if that changes (which I assume it will at some point, e.g. if we have a shared backbone)
        // this will break. Would it be better if `run_nn_inference` would get pointers to the input/output
        // matrices instead?
        input[0].matrix = fmatrix[impulse->dsp_blocks_size + (learn_block_index - 1)].matrix;
        input[0].blockId = fmatrix[impulse->dsp_blocks_size + (learn_block_index - 1)].blockId;

        input_block_ids_size = 1;
    }
    else {
        input[0].matrix = matrix_ptr.get();
        input[0].blockId = 0;

        extract_anomaly_input_values(fmatrix, input_block_ids, input_block_ids_size, block_config->anom_axes_size, block_config->anom_axis, input[0].matrix->buffer);
        input_block_ids_size = 1;
    }

    EI_IMPULSE_ERROR res = run_nn_inference(impulse, input, learn_block_index, input_block_ids, input_block_ids_size, &anomaly_result, (void*)&ei_learning_block_config_gmm, debug);
    if (res != EI_IMPULSE_OK) {
        return res;
    }

    if (debug) {
        ei_printf("Anomaly score (time: %d ms.): ", anomaly_result.timing.classification);
        ei_printf_float(anomaly_result.classification[0].value);
        ei_printf("\n");
    }

    result->timing.anomaly = anomaly_result.timing.classification;

    if (block_config->classification_mode == EI_CLASSIFIER_CLASSIFICATION_MODE_VISUAL_ANOMALY) {
#if EI_CLASSIFIER_HAS_VISUAL_ANOMALY
        result->visual_ad_grid_cells = anomaly_result.visual_ad_grid_cells;
        result->visual_ad_count = anomaly_result.visual_ad_count;
        result->visual_ad_result.mean_value = anomaly_result.visual_ad_result.mean_value;
        result->visual_ad_result.max_value = anomaly_result.visual_ad_result.max_value;
#endif // EI_CLASSIFIER_HAS_VISUAL_ANOMALY
    }
    else {
        result->anomaly = anomaly_result.classification[0].value;
    }

    return EI_IMPULSE_OK;
}
#endif // (EI_CLASSIFIER_INFERENCING_ENGINE != EI_CLASSIFIER_NONE)

#endif //#if (EI_CLASSIFIER_HAS_ANOMALY == 1)
#endif // _EDGE_IMPULSE_INFERENCING_ANOMALY_H_
