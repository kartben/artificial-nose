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

#ifndef _EI_CLASSIFIER_INFERENCING_ENGINE_TENSORRT_H_
#define _EI_CLASSIFIER_INFERENCING_ENGINE_TENSORRT_H_

#if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSORRT)

#include "model-parameters/model_metadata.h"

#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "edge-impulse-sdk/classifier/ei_fill_result_struct.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <filesystem>
#include <stdlib.h>
#include "tflite/linux-jetson-nano/libeitrt.h"

#if __APPLE__
#include <mach-o/dyld.h>
#else
#include <linux/limits.h>
#endif

EiTrt *ei_trt_handle = NULL;

inline bool file_exists(char *model_file_name)
{
    if (FILE *file = fopen(model_file_name, "r")) {
        fclose(file);
        return true;
    }
    else {
        return false;
    }
}

/**
 * @brief      Do neural network inferencing over the processed feature matrix
 *
 * @param      fmatrix  Processed matrix
 * @param      result   Output classifier results
 * @param[in]  debug    Debug output enable
 *
 * @return     The ei impulse error.
 */
EI_IMPULSE_ERROR run_nn_inference(
    const ei_impulse_t *impulse,
    ei_feature_t *fmatrix,
    uint32_t learn_block_index,
    uint32_t* input_block_ids,
    uint32_t input_block_ids_size,
    ei_impulse_result_t *result,
    void *config_ptr,
    bool debug = false)
{
    ei_learning_block_config_tflite_graph_t *block_config = (ei_learning_block_config_tflite_graph_t*)config_ptr;
    ei_config_tflite_graph_t *graph_config = (ei_config_tflite_graph_t*)block_config->graph_config;

    #if EI_CLASSIFIER_QUANTIZATION_ENABLED == 1
    #error "TensorRT requires an unquantized network"
    #endif

    static char current_exe_path[PATH_MAX] = { 0 };

#if __APPLE__
    uint32_t len = PATH_MAX;
    if (_NSGetExecutablePath(current_exe_path, &len) != 0) {
        current_exe_path[0] = '\0'; // buffer too small
    }
    else {
        // resolve symlinks, ., .. if possible
        char *canonical_path = realpath(current_exe_path, NULL);
        if (canonical_path != NULL)
        {
            strncpy(current_exe_path, canonical_path, len);
            free(canonical_path);
        }
    }
#else
    int readlink_res = readlink("/proc/self/exe", current_exe_path, PATH_MAX);
    if (readlink_res < 0) {
        printf("readlink_res = %d\n", readlink_res);
        current_exe_path[0] = '\0'; // failed to find location
    }
#endif

    static char model_file_name[PATH_MAX];

    if (strlen(current_exe_path) == 0) {
        // could not determine current exe path, use /tmp for the engine file
        snprintf(
            model_file_name,
            PATH_MAX,
            "/tmp/ei-%d-%d.engine",
            impulse->project_id,
            impulse->deploy_version);
    }
    else {
        std::filesystem::path p(current_exe_path);
        snprintf(
            model_file_name,
            PATH_MAX,
            "%s/%s-project%d-v%d.engine",
            p.parent_path().c_str(),
            p.stem().c_str(),
            impulse->project_id,
            impulse->deploy_version);
    }

    static bool first_run = true;

    if (first_run) {

        bool fexists = file_exists(model_file_name);
        if (!fexists) {
            ei_printf("INFO: Model file '%s' does not exist, creating...\n", model_file_name);

            FILE *file = fopen(model_file_name, "w");
            if (!file) {
                ei_printf("ERR: TensorRT init failed to open '%s'\n", model_file_name);
                return EI_IMPULSE_TENSORRT_INIT_FAILED;
            }

            if (fwrite(graph_config->model, graph_config->model_size, 1, file) != 1) {
                ei_printf("ERR: TensorRT init fwrite failed.\n");
                return EI_IMPULSE_TENSORRT_INIT_FAILED;
            }

            if (fclose(file) != 0) {
                ei_printf("ERR: TensorRT init fclose failed.\n");
                return EI_IMPULSE_TENSORRT_INIT_FAILED;
            }
        }

        first_run = false;
    }

    uint32_t out_data_size = 0;

    if (block_config->object_detection) {
        switch (block_config->object_detection_last_layer) {
            case EI_CLASSIFIER_LAST_LAYER_TAO_SSD:
            case EI_CLASSIFIER_LAST_LAYER_TAO_RETINANET:
            case EI_CLASSIFIER_LAST_LAYER_TAO_YOLOV3:
            case EI_CLASSIFIER_LAST_LAYER_TAO_YOLOV4:
            case EI_CLASSIFIER_LAST_LAYER_FOMO:
            case EI_CLASSIFIER_LAST_LAYER_YOLOV5:
            case EI_CLASSIFIER_LAST_LAYER_YOLOV5_V5_DRPAI: {
                out_data_size = impulse->tflite_output_features_count;
                break;
            }
            default: {
                ei_printf(
                    "ERR: Unsupported object detection last layer (%d)\n",
                    block_config->object_detection_last_layer);
                return EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE;
            }
        }
    }
    else {
        out_data_size = impulse->label_count;
    }

    float *out_data = (float*)ei_malloc(out_data_size * sizeof(float));
    if (out_data == nullptr) {
        ei_printf("ERR: Cannot allocate memory for output data \n");
    }

    // lazy initialize tensorRT context
    if (ei_trt_handle == nullptr) {
        ei_trt_handle = libeitrt::create_EiTrt(model_file_name, debug);
    }

#if EI_CLASSIFIER_SINGLE_FEATURE_INPUT == 0
    size_t mtx_size = impulse->dsp_blocks_size + impulse->learning_blocks_size;
    ei::matrix_t* matrix = NULL;

    ei::matrix_t combined_matrix(1, impulse->nn_input_frame_size);
    uint32_t buf_pos = 0;

    for (size_t i = 0; i < input_block_ids_size; i++) {
        size_t cur_mtx = input_block_ids[i];

        if (!find_mtx_by_idx(fmatrix, &matrix, cur_mtx, mtx_size)) {
            ei_printf("ERR: Cannot find matrix with id %zu\n", cur_mtx);
            return EI_IMPULSE_INVALID_SIZE;
        }

        for (size_t ix = 0; ix < matrix->rows * matrix->cols; ix++) {
            combined_matrix.buffer[buf_pos++] = matrix->buffer[ix];
        }
    }
    matrix = &combined_matrix;
#else
    ei::matrix_t* matrix = fmatrix[0].matrix;
#endif

    uint64_t ctx_start_us = ei_read_timer_us();

    libeitrt::infer(ei_trt_handle, matrix->buffer, out_data, out_data_size);

    uint64_t ctx_end_us = ei_read_timer_us();

    result->timing.classification_us = ctx_end_us - ctx_start_us;
    result->timing.classification = (int)(result->timing.classification_us / 1000);

    EI_IMPULSE_ERROR fill_res = EI_IMPULSE_OK;

    if (block_config->object_detection) {
        switch (block_config->object_detection_last_layer) {
            case EI_CLASSIFIER_LAST_LAYER_FOMO: {
                fill_res = fill_result_struct_f32_fomo(
                    impulse,
                    block_config,
                    result,
                    out_data,
                    impulse->fomo_output_size,
                    impulse->fomo_output_size);
                break;
            }
            case EI_CLASSIFIER_LAST_LAYER_YOLOV5:
            case EI_CLASSIFIER_LAST_LAYER_YOLOV5_V5_DRPAI: {
                int version = block_config->object_detection_last_layer == EI_CLASSIFIER_LAST_LAYER_YOLOV5_V5_DRPAI ?
                    5 : 6;
                fill_res = fill_result_struct_f32_yolov5(
                    impulse,
                    block_config,
                    result,
                    version,
                    out_data,
                    impulse->tflite_output_features_count);
                break;
            }
            case EI_CLASSIFIER_LAST_LAYER_TAO_SSD:
            case EI_CLASSIFIER_LAST_LAYER_TAO_RETINANET: {
                fill_res = fill_result_struct_f32_tao_decode_detections(
                    impulse,
                    block_config,
                    result,
                    out_data,
                    impulse->tflite_output_features_count,
                    debug);
                break;
            }
            case EI_CLASSIFIER_LAST_LAYER_TAO_YOLOV3:
                fill_res = fill_result_struct_f32_tao_yolov3(
                    impulse,
                    block_config,
                    result,
                    out_data,
                    impulse->tflite_output_features_count,
                    debug);
                break;
            case EI_CLASSIFIER_LAST_LAYER_TAO_YOLOV4: {
                fill_res = fill_result_struct_f32_tao_yolov4(
                    impulse,
                    block_config,
                    result,
                    out_data,
                    impulse->tflite_output_features_count,
                    debug);
                break;
            }
            default: {
                ei_printf(
                    "ERR: Unsupported object detection last layer (%d)\n",
                    block_config->object_detection_last_layer);
                return EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE;
            }
        }
    }
    else {
        fill_res = fill_result_struct_f32(impulse, result, out_data, debug);
    }

    ei_free(out_data);

    if (fill_res != EI_IMPULSE_OK) {
        return fill_res;
    }

    return EI_IMPULSE_OK;
}

/**
 * Special function to run the classifier on images for quantized models
 * that allocates a lot less memory by quantizing in place. This only works if 'can_run_classifier_image_quantized'
 * returns EI_IMPULSE_OK.
 */
EI_IMPULSE_ERROR run_nn_inference_image_quantized(
    const ei_impulse_t *impulse,
    signal_t *signal,
    ei_impulse_result_t *result,
    void *config_ptr,
    bool debug = false)
{
    return EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE;
}

#endif // #if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSORRT)
#endif // _EI_CLASSIFIER_INFERENCING_ENGINE_TENSORRT_H_
