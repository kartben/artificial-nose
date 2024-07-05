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

#ifndef _EDGE_IMPULSE_RUN_CLASSIFIER_H_
#define _EDGE_IMPULSE_RUN_CLASSIFIER_H_

#include "ei_model_types.h"
#include "model-parameters/model_metadata.h"

#include "ei_run_dsp.h"
#include "ei_classifier_types.h"
#include "ei_signal_with_axes.h"
#include "ei_performance_calibration.h"

#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "edge-impulse-sdk/porting/ei_logging.h"
#include <memory>

#if EI_CLASSIFIER_HAS_ANOMALY
#include "inferencing_engines/anomaly.h"
#endif

#if defined(EI_CLASSIFIER_HAS_SAMPLER) && EI_CLASSIFIER_HAS_SAMPLER == 1
#include "ei_sampler.h"
#endif

#if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE) && (EI_CLASSIFIER_COMPILED != 1)
#include "edge-impulse-sdk/classifier/inferencing_engines/tflite_micro.h"
#elif EI_CLASSIFIER_COMPILED == 1
#include "edge-impulse-sdk/classifier/inferencing_engines/tflite_eon.h"
#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE_FULL
#include "edge-impulse-sdk/classifier/inferencing_engines/tflite_full.h"
#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE_TIDL
#include "edge-impulse-sdk/classifier/inferencing_engines/tflite_tidl.h"
#elif (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSORRT)
#include "edge-impulse-sdk/classifier/inferencing_engines/tensorrt.h"
#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSAIFLOW
#include "edge-impulse-sdk/classifier/inferencing_engines/tensaiflow.h"
#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_DRPAI
#include "edge-impulse-sdk/classifier/inferencing_engines/drpai.h"
#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_AKIDA
#include "edge-impulse-sdk/classifier/inferencing_engines/akida.h"
#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_ONNX_TIDL
#include "edge-impulse-sdk/classifier/inferencing_engines/onnx_tidl.h"
#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_MEMRYX
#include "edge-impulse-sdk/classifier/inferencing_engines/memryx.h"
#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_NONE
// noop
#else
#error "Unknown inferencing engine"
#endif

// This file has an implicit dependency on ei_run_dsp.h, so must come after that include!
#include "model-parameters/model_variables.h"

#ifdef __cplusplus
namespace {
#endif // __cplusplus

/* Function prototypes ----------------------------------------------------- */
extern "C" EI_IMPULSE_ERROR run_inference(ei_impulse_handle_t *handle, ei_feature_t *fmatrix, ei_impulse_result_t *result, bool debug);
extern "C" EI_IMPULSE_ERROR run_classifier_image_quantized(const ei_impulse_t *impulse, signal_t *signal, ei_impulse_result_t *result, bool debug);
static EI_IMPULSE_ERROR can_run_classifier_image_quantized(const ei_impulse_t *impulse, ei_learning_block_t block_ptr);

#if EI_CLASSIFIER_LOAD_IMAGE_SCALING
EI_IMPULSE_ERROR ei_scale_fmatrix(ei_learning_block_t *block, ei::matrix_t *fmatrix);
EI_IMPULSE_ERROR ei_unscale_fmatrix(ei_learning_block_t *block, ei::matrix_t *fmatrix);
#endif // EI_CLASSIFIER_LOAD_IMAGE_SCALING

/* Private variables ------------------------------------------------------- */

static uint64_t classifier_continuous_features_written = 0;
static RecognizeEvents *avg_scores = NULL;

/* Private functions ------------------------------------------------------- */

/* These functions (up to Public functions section) are not exposed to end-user,
therefore changes are allowed. */


/**
 * @brief      Display the results of the inference
 *
 * @param      result  The result
 */
__attribute__((unused)) void display_results(ei_impulse_result_t* result)
{
    // print the predictions
    ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
                result->timing.dsp, result->timing.classification, result->timing.anomaly);
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    ei_printf("#Object detection results:\r\n");
    bool bb_found = result->bounding_boxes[0].value > 0;
    for (size_t ix = 0; ix < result->bounding_boxes_count; ix++) {
        auto bb = result->bounding_boxes[ix];
        if (bb.value == 0) {
            continue;
        }
        ei_printf("    %s (", bb.label);
        ei_printf_float(bb.value);
        ei_printf(") [ x: %u, y: %u, width: %u, height: %u ]\n", bb.x, bb.y, bb.width, bb.height);
    }

    if (!bb_found) {
        ei_printf("    No objects found\n");
    }

#elif (EI_CLASSIFIER_LABEL_COUNT == 1) && (!EI_CLASSIFIER_HAS_ANOMALY)// regression
    ei_printf("#Regression results:\r\n");
    ei_printf("    %s: ", result->classification[0].label);
    ei_printf_float(result->classification[0].value);
    ei_printf("\n");

#elif EI_CLASSIFIER_LABEL_COUNT > 1 // if there is only one label, this is an anomaly only
    ei_printf("#Classification results:\r\n");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("    %s: ", result->classification[ix].label);
        ei_printf_float(result->classification[ix].value);
        ei_printf("\n");
    }
#endif
#if EI_CLASSIFIER_HAS_ANOMALY == 3 // visual AD
    ei_printf("#Visual anomaly grid results:\r\n");
    for (uint32_t i = 0; i < result->visual_ad_count; i++) {
        ei_impulse_result_bounding_box_t bb = result->visual_ad_grid_cells[i];
        if (bb.value == 0) {
            continue;
        }
        ei_printf("    %s (", bb.label);
        ei_printf_float(bb.value);
        ei_printf(") [ x: %u, y: %u, width: %u, height: %u ]\n", bb.x, bb.y, bb.width, bb.height);
    }
    ei_printf("Visual anomaly values: Mean ");
    ei_printf_float(result->visual_ad_result.mean_value);
    ei_printf(" Max ");
    ei_printf_float(result->visual_ad_result.max_value);
    ei_printf("\r\n");
#elif (EI_CLASSIFIER_HAS_ANOMALY > 0) // except for visual AD
    ei_printf("Anomaly prediction: ");
    ei_printf_float(result->anomaly);
    ei_printf("\r\n");
#endif
}

/**
 * @brief      Do inferencing over the processed feature matrix
 *
 * @param      impulse  struct with information about model and DSP
 * @param      fmatrix  Processed matrix
 * @param      result   Output classifier results
 * @param[in]  debug    Debug output enable
 *
 * @return     The ei impulse error.
 */
extern "C" EI_IMPULSE_ERROR run_inference(
    ei_impulse_handle_t *handle,
    ei_feature_t *fmatrix,
    ei_impulse_result_t *result,
    bool debug = false)
{
    auto& impulse = handle->impulse;
    for (size_t ix = 0; ix < impulse->learning_blocks_size; ix++) {

        ei_learning_block_t block = impulse->learning_blocks[ix];

#if EI_CLASSIFIER_LOAD_IMAGE_SCALING
        // we do not plan to have multiple dsp blocks with image
        // so just apply scaling to the first one
        EI_IMPULSE_ERROR scale_res = ei_scale_fmatrix(&block, fmatrix[0].matrix);
        if (scale_res != EI_IMPULSE_OK) {
            return scale_res;
        }
#endif

        result->copy_output = block.keep_output;

        EI_IMPULSE_ERROR res = block.infer_fn(impulse, fmatrix, ix, (uint32_t*)block.input_block_ids, block.input_block_ids_size, result, block.config, debug);
        if (res != EI_IMPULSE_OK) {
            return res;
        }

#if EI_CLASSIFIER_LOAD_IMAGE_SCALING
        // undo scaling
        scale_res = ei_unscale_fmatrix(&block, fmatrix[0].matrix);
        if (scale_res != EI_IMPULSE_OK) {
            return scale_res;
        }
#endif
    }

    if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
        return EI_IMPULSE_CANCELED;
    }

    return EI_IMPULSE_OK;
}

/**
 * @brief      Process a complete impulse
 *
 * @param      impulse  struct with information about model and DSP
 * @param      signal   Sample data
 * @param      result   Output classifier results
 * @param      handle   Handle from open_impulse. nullptr for backward compatibility
 * @param[in]  debug    Debug output enable
 *
 * @return     The ei impulse error.
 */
extern "C" EI_IMPULSE_ERROR process_impulse(ei_impulse_handle_t *handle,
                                            signal_t *signal,
                                            ei_impulse_result_t *result,
                                            bool debug = false)
{
    if(!handle) {
        return EI_IMPULSE_INFERENCE_ERROR;
    }

#if (EI_CLASSIFIER_QUANTIZATION_ENABLED == 1 && (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE || EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSAIFLOW || EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_ONNX_TIDL)) || EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_DRPAI
    // Shortcut for quantized image models
    ei_learning_block_t block = handle->impulse->learning_blocks[0];
    if (can_run_classifier_image_quantized(handle->impulse, block) == EI_IMPULSE_OK) {
        return run_classifier_image_quantized(handle->impulse, signal, result, debug);
    }
#endif

    memset(result, 0, sizeof(ei_impulse_result_t));
    uint32_t block_num = handle->impulse->dsp_blocks_size + handle->impulse->learning_blocks_size;

    // smart pointer to features array
    std::unique_ptr<ei_feature_t[]> features_ptr(new ei_feature_t[block_num]);
    ei_feature_t* features = features_ptr.get();
    memset(features, 0, sizeof(ei_feature_t) * block_num);

    // have it outside of the loop to avoid going out of scope
    std::unique_ptr<ei::matrix_t> *matrix_ptrs = new std::unique_ptr<ei::matrix_t>[block_num];

    uint64_t dsp_start_us = ei_read_timer_us();

    size_t out_features_index = 0;

    for (size_t ix = 0; ix < handle->impulse->dsp_blocks_size; ix++) {
        ei_model_dsp_t block = handle->impulse->dsp_blocks[ix];
        matrix_ptrs[ix] = std::unique_ptr<ei::matrix_t>(new ei::matrix_t(1, block.n_output_features));
        features[ix].matrix = matrix_ptrs[ix].get();
        features[ix].blockId = block.blockId;

        if (out_features_index + block.n_output_features > handle->impulse->nn_input_frame_size) {
            ei_printf("ERR: Would write outside feature buffer\n");
            delete[] matrix_ptrs;
            return EI_IMPULSE_DSP_ERROR;
        }

#if EIDSP_SIGNAL_C_FN_POINTER
        if (block.axes_size != handle->impulse->raw_samples_per_frame) {
            ei_printf("ERR: EIDSP_SIGNAL_C_FN_POINTER can only be used when all axes are selected for DSP blocks\n");
            delete[] matrix_ptrs;
            return EI_IMPULSE_DSP_ERROR;
        }
        auto internal_signal = signal;
#else
        SignalWithAxes swa(signal, block.axes, block.axes_size, handle->impulse);
        auto internal_signal = swa.get_signal();
#endif

        int ret;
        if (block.factory) { // ie, if we're using state
            // Msg user
            static bool has_printed = false;
            if (!has_printed) {
                EI_LOGI("Impulse maintains state. Call run_classifier_init() to reset state (e.g. if data stream is interrupted.)\n");
                has_printed = true;
            }

            // getter has a lazy init, so we can just call it
            auto dsp_handle = handle->state.get_dsp_handle(ix);
            if(dsp_handle) {
                ret = dsp_handle->extract(internal_signal, features[ix].matrix, block.config, handle->impulse->frequency);
            } else {
                return EI_IMPULSE_OUT_OF_MEMORY;
            }
        } else {
            ret = block.extract_fn(internal_signal, features[ix].matrix, block.config, handle->impulse->frequency);
        }

        if (ret != EIDSP_OK) {
            ei_printf("ERR: Failed to run DSP process (%d)\n", ret);
            delete[] matrix_ptrs;
            return EI_IMPULSE_DSP_ERROR;
        }

        if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
            delete[] matrix_ptrs;
            return EI_IMPULSE_CANCELED;
        }

        out_features_index += block.n_output_features;
    }

#if EI_CLASSIFIER_SINGLE_FEATURE_INPUT == 0
    for (size_t ix = 0; ix < handle->impulse->learning_blocks_size; ix++) {
        ei_learning_block_t block = handle->impulse->learning_blocks[ix];

        if (block.keep_output) {
            matrix_ptrs[handle->impulse->dsp_blocks_size + ix] = std::unique_ptr<ei::matrix_t>(new ei::matrix_t(1, block.output_features_count));
            features[handle->impulse->dsp_blocks_size + ix].matrix = matrix_ptrs[handle->impulse->dsp_blocks_size + ix].get();
            features[handle->impulse->dsp_blocks_size + ix].blockId = block.blockId;
        }
    }
#endif // EI_CLASSIFIER_SINGLE_FEATURE_INPUT

    result->timing.dsp_us = ei_read_timer_us() - dsp_start_us;
    result->timing.dsp = (int)(result->timing.dsp_us / 1000);

    if (debug) {
        ei_printf("Features (%d ms.): ", result->timing.dsp);
        for (size_t ix = 0; ix < block_num; ix++) {
            if (features[ix].matrix == nullptr) {
                continue;
            }
            for (size_t jx = 0; jx < features[ix].matrix->cols; jx++) {
                ei_printf_float(features[ix].matrix->buffer[jx]);
                ei_printf(" ");
            }
            ei_printf("\n");
        }
    }

    if (debug) {
        ei_printf("Running impulse...\n");
    }

    EI_IMPULSE_ERROR res = run_inference(handle, features, result, debug);
    delete[] matrix_ptrs;
    return res;
}

/**
 * @brief      Opens an impulse
 *
 * @param      impulse  struct with information about model and DSP
 *
 * @return     A pointer to the impulse handle, or nullptr if memory allocation failed.
 */
extern "C" EI_IMPULSE_ERROR init_impulse(ei_impulse_handle_t *handle) {
    if (!handle) {
        return EI_IMPULSE_OUT_OF_MEMORY;
    }
    handle->state.reset();
    return EI_IMPULSE_OK;
}

/**
 * @brief      Process a complete impulse for continuous inference
 *
 * @param      impulse  struct with information about model and DSP
 * @param      signal   Sample data
 * @param      result   Output classifier results
 * @param[in]  debug    Debug output enable
 *
 * @return     The ei impulse error.
 */
extern "C" EI_IMPULSE_ERROR process_impulse_continuous(ei_impulse_handle_t *handle,
                                            signal_t *signal,
                                            ei_impulse_result_t *result,
                                            bool debug,
                                            bool enable_maf)
{
    auto impulse = handle->impulse;
    static ei::matrix_t static_features_matrix(1, impulse->nn_input_frame_size);
    if (!static_features_matrix.buffer) {
        return EI_IMPULSE_ALLOC_FAILED;
    }

    memset(result, 0, sizeof(ei_impulse_result_t));

    EI_IMPULSE_ERROR ei_impulse_error = EI_IMPULSE_OK;

    uint64_t dsp_start_us = ei_read_timer_us();

    size_t out_features_index = 0;

    for (size_t ix = 0; ix < impulse->dsp_blocks_size; ix++) {
        ei_model_dsp_t block = impulse->dsp_blocks[ix];

        if (out_features_index + block.n_output_features > impulse->nn_input_frame_size) {
            ei_printf("ERR: Would write outside feature buffer\n");
            return EI_IMPULSE_DSP_ERROR;
        }

        ei::matrix_t fm(1, block.n_output_features,
                        static_features_matrix.buffer + out_features_index);

        int (*extract_fn_slice)(ei::signal_t *signal, ei::matrix_t *output_matrix, void *config, const float frequency, matrix_size_t *out_matrix_size);

        /* Switch to the slice version of the mfcc feature extract function */
        if (block.extract_fn == extract_mfcc_features) {
            extract_fn_slice = &extract_mfcc_per_slice_features;
        }
        else if (block.extract_fn == extract_spectrogram_features) {
            extract_fn_slice = &extract_spectrogram_per_slice_features;
        }
        else if (block.extract_fn == extract_mfe_features) {
            extract_fn_slice = &extract_mfe_per_slice_features;
        }
        else {
            ei_printf("ERR: Unknown extract function, only MFCC, MFE and spectrogram supported\n");
            return EI_IMPULSE_DSP_ERROR;
        }

        matrix_size_t features_written;

#if EIDSP_SIGNAL_C_FN_POINTER
        if (block.axes_size != impulse->raw_samples_per_frame) {
            ei_printf("ERR: EIDSP_SIGNAL_C_FN_POINTER can only be used when all axes are selected for DSP blocks\n");
            return EI_IMPULSE_DSP_ERROR;
        }
        int ret = extract_fn_slice(signal, &fm, block.config, impulse->frequency, &features_written);
#else
        SignalWithAxes swa(signal, block.axes, block.axes_size, impulse);
        int ret = extract_fn_slice(swa.get_signal(), &fm, block.config, impulse->frequency, &features_written);
#endif

        if (ret != EIDSP_OK) {
            ei_printf("ERR: Failed to run DSP process (%d)\n", ret);
            return EI_IMPULSE_DSP_ERROR;
        }

        if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
            return EI_IMPULSE_CANCELED;
        }

        classifier_continuous_features_written += (features_written.rows * features_written.cols);

        out_features_index += block.n_output_features;
    }

    result->timing.dsp_us = ei_read_timer_us() - dsp_start_us;
    result->timing.dsp = (int)(result->timing.dsp_us / 1000);

    if (classifier_continuous_features_written >= impulse->nn_input_frame_size) {
        dsp_start_us = ei_read_timer_us();

        uint32_t block_num = impulse->dsp_blocks_size + impulse->learning_blocks_size;

        // smart pointer to features array
        std::unique_ptr<ei_feature_t[]> features_ptr(new ei_feature_t[block_num]);
        ei_feature_t* features = features_ptr.get();
        memset(features, 0, sizeof(ei_feature_t) * block_num);

        // have it outside of the loop to avoid going out of scope
        std::unique_ptr<ei::matrix_t> *matrix_ptrs = new std::unique_ptr<ei::matrix_t>[block_num];

        out_features_index = 0;
        // iterate over every dsp block and run normalization
        for (size_t ix = 0; ix < impulse->dsp_blocks_size; ix++) {
            ei_model_dsp_t block = impulse->dsp_blocks[ix];
            matrix_ptrs[ix] = std::unique_ptr<ei::matrix_t>(new ei::matrix_t(1, block.n_output_features));
            features[ix].matrix = matrix_ptrs[ix].get();
            features[ix].blockId = block.blockId;

            /* Create a copy of the matrix for normalization */
            for (size_t m_ix = 0; m_ix < block.n_output_features; m_ix++) {
                features[ix].matrix->buffer[m_ix] = static_features_matrix.buffer[out_features_index + m_ix];
            }

            if (block.extract_fn == extract_mfcc_features) {
                calc_cepstral_mean_and_var_normalization_mfcc(features[ix].matrix, block.config);
            }
            else if (block.extract_fn == extract_spectrogram_features) {
                calc_cepstral_mean_and_var_normalization_spectrogram(features[ix].matrix, block.config);
            }
            else if (block.extract_fn == extract_mfe_features) {
                calc_cepstral_mean_and_var_normalization_mfe(features[ix].matrix, block.config);
            }
            out_features_index += block.n_output_features;
        }

        result->timing.dsp_us += ei_read_timer_us() - dsp_start_us;
        result->timing.dsp = (int)(result->timing.dsp_us / 1000);

        if (debug) {
            ei_printf("Feature Matrix: \n");
            for (size_t ix = 0; ix < features->matrix->cols; ix++) {
                ei_printf_float(features->matrix->buffer[ix]);
                ei_printf(" ");
            }
            ei_printf("\n");
            ei_printf("Running impulse...\n");
        }

        ei_impulse_error = run_inference(handle, features, result, debug);

#if EI_CLASSIFIER_CALIBRATION_ENABLED
        if (impulse->sensor == EI_CLASSIFIER_SENSOR_MICROPHONE) {
            if((void *)avg_scores != NULL && enable_maf == true) {
                if (enable_maf && !impulse->calibration.is_configured) {
                    // perfcal is not configured, print msg first time
                    static bool has_printed_msg = false;

                    if (!has_printed_msg) {
                        ei_printf("WARN: run_classifier_continuous, enable_maf is true, but performance calibration is not configured.\n");
                        ei_printf("       Previously we'd run a moving-average filter over your outputs in this case, but this is now disabled.\n");
                        ei_printf("       Go to 'Performance calibration' in your Edge Impulse project to configure post-processing parameters.\n");
                        ei_printf("       (You can enable this from 'Dashboard' if it's not visible in your project)\n");
                        ei_printf("\n");

                        has_printed_msg = true;
                    }
                }
                else {
                    // perfcal is configured
                    static bool has_printed_msg = false;

                    if (!has_printed_msg) {
                        ei_printf("\nPerformance calibration is configured for your project. If no event is detected, all values are 0.\r\n\n");
                        has_printed_msg = true;
                    }

                    int label_detected = avg_scores->trigger(result->classification);

                    if (avg_scores->should_boost()) {
                        for (int i = 0; i < impulse->label_count; i++) {
                            if (i == label_detected) {
                                result->classification[i].value = 1.0f;
                            }
                            else {
                                result->classification[i].value = 0.0f;
                            }
                        }
                    }
                }
            }
        }
#endif
        delete[] matrix_ptrs;
    }
    else {
        for (int i = 0; i < impulse->label_count; i++) {
            // set label correctly in the result struct if we have no results (otherwise is nullptr)
            result->classification[i].label = impulse->categories[(uint32_t)i];
        }
    }

    return ei_impulse_error;
}

/**
 * Check if the current impulse could be used by 'run_classifier_image_quantized'
 */
__attribute__((unused)) static EI_IMPULSE_ERROR can_run_classifier_image_quantized(const ei_impulse_t *impulse, ei_learning_block_t block_ptr) {

    if (impulse->inferencing_engine != EI_CLASSIFIER_TFLITE
        && impulse->inferencing_engine != EI_CLASSIFIER_TENSAIFLOW
        && impulse->inferencing_engine != EI_CLASSIFIER_DRPAI
        && impulse->inferencing_engine != EI_CLASSIFIER_ONNX_TIDL) // check later
    {
        return EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE;
    }

    // visual anomaly also needs to go through the normal path
    if (impulse->has_anomaly){
        return EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES;
    }

        // Check if we have tflite graph
    if (block_ptr.infer_fn != run_nn_inference) {
        return EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES;
    }

    // Check if we have a quantized NN Input layer (input is always quantized for DRP-AI)
    ei_learning_block_config_tflite_graph_t *block_config = (ei_learning_block_config_tflite_graph_t*)block_ptr.config;
    if (block_config->quantized != 1) {
        return EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES;
    }

    // And if we have one DSP block which operates on images...
    if (impulse->dsp_blocks_size != 1 || impulse->dsp_blocks[0].extract_fn != extract_image_features) {
        return EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES;
    }

    return EI_IMPULSE_OK;
}

#if EI_CLASSIFIER_QUANTIZATION_ENABLED == 1 && (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE || EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSAIFLOW || EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_DRPAI || EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_ONNX_TIDL)

/**
 * Special function to run the classifier on images, only works on TFLite models (either interpreter, EON, tensaiflow, drpai, tidl, memryx)
 * that allocates a lot less memory by quantizing in place. This only works if 'can_run_classifier_image_quantized'
 * returns EI_IMPULSE_OK.
 */
extern "C" EI_IMPULSE_ERROR run_classifier_image_quantized(
    const ei_impulse_t *impulse,
    signal_t *signal,
    ei_impulse_result_t *result,
    bool debug = false)
{
    memset(result, 0, sizeof(ei_impulse_result_t));

    return run_nn_inference_image_quantized(impulse, signal, result, impulse->learning_blocks[0].config, debug);
}

#endif // #if EI_CLASSIFIER_QUANTIZATION_ENABLED == 1 && (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE || EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSAIFLOW || EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_DRPAI)

#if EI_CLASSIFIER_LOAD_IMAGE_SCALING
static const float torch_mean[] = { 0.485, 0.456, 0.406 };
static const float torch_std[] = { 0.229, 0.224, 0.225 };
// This is ordered BGR
static const float tao_mean[] = { 103.939, 116.779, 123.68 };

EI_IMPULSE_ERROR ei_scale_fmatrix(ei_learning_block_t *block, ei::matrix_t *fmatrix) {
    if (block->image_scaling == EI_CLASSIFIER_IMAGE_SCALING_TORCH) {
        // @todo; could we write some faster vector math here?
        for (size_t ix = 0; ix < fmatrix->rows * fmatrix->cols; ix += 3) {
            fmatrix->buffer[ix + 0] = (fmatrix->buffer[ix + 0] - torch_mean[0]) / torch_std[0];
            fmatrix->buffer[ix + 1] = (fmatrix->buffer[ix + 1] - torch_mean[1]) / torch_std[1];
            fmatrix->buffer[ix + 2] = (fmatrix->buffer[ix + 2] - torch_mean[2]) / torch_std[2];
        }
    }
    else if (block->image_scaling == EI_CLASSIFIER_IMAGE_SCALING_0_255) {
        int scale_res = numpy::scale(fmatrix, 255.0f);
        if (scale_res != EIDSP_OK) {
            ei_printf("ERR: Failed to scale matrix (%d)\n", scale_res);
            return EI_IMPULSE_DSP_ERROR;
        }
    }
    else if (block->image_scaling == EI_CLASSIFIER_IMAGE_SCALING_MIN128_127) {
        int scale_res = numpy::scale(fmatrix, 255.0f);
        if (scale_res != EIDSP_OK) {
            ei_printf("ERR: Failed to scale matrix (%d)\n", scale_res);
            return EI_IMPULSE_DSP_ERROR;
        }
        scale_res = numpy::subtract(fmatrix, 128.0f);
        if (scale_res != EIDSP_OK) {
            ei_printf("ERR: Failed to scale matrix (%d)\n", scale_res);
            return EI_IMPULSE_DSP_ERROR;
        }
    }
    else if (block->image_scaling == EI_CLASSIFIER_IMAGE_SCALING_MIN1_1) {
        int scale_res = numpy::scale(fmatrix, 2.0f);
        if (scale_res != EIDSP_OK) {
            ei_printf("ERR: Failed to scale matrix (%d)\n", scale_res);
            return EI_IMPULSE_DSP_ERROR;
        }
        scale_res = numpy::subtract(fmatrix, 1.0f);
        if (scale_res != EIDSP_OK) {
            ei_printf("ERR: Failed to scale matrix (%d)\n", scale_res);
            return EI_IMPULSE_DSP_ERROR;
        }
    }
    else if (block->image_scaling == EI_CLASSIFIER_IMAGE_SCALING_BGR_SUBTRACT_IMAGENET_MEAN) {
        int scale_res = numpy::scale(fmatrix, 255.0f);
        if (scale_res != EIDSP_OK) {
            ei_printf("ERR: Failed to scale matrix (%d)\n", scale_res);
            return EI_IMPULSE_DSP_ERROR;
        }
        // Transpose RGB to BGR and subtract mean
        for (size_t ix = 0; ix < fmatrix->rows * fmatrix->cols; ix += 3) {
            float r = fmatrix->buffer[ix + 0];
            fmatrix->buffer[ix + 0] = fmatrix->buffer[ix + 2] - tao_mean[0];
            fmatrix->buffer[ix + 1] -= tao_mean[1];
            fmatrix->buffer[ix + 2] = r - tao_mean[2];
        }
    }

    return EI_IMPULSE_OK;
}

EI_IMPULSE_ERROR ei_unscale_fmatrix(ei_learning_block_t *block, ei::matrix_t *fmatrix) {
    if (block->image_scaling == EI_CLASSIFIER_IMAGE_SCALING_TORCH) {
        // @todo; could we write some faster vector math here?
        for (size_t ix = 0; ix < fmatrix->rows * fmatrix->cols; ix += 3) {
            fmatrix->buffer[ix + 0] = (fmatrix->buffer[ix + 0] * torch_std[0]) + torch_mean[0];
            fmatrix->buffer[ix + 1] = (fmatrix->buffer[ix + 1] * torch_std[1]) + torch_mean[1];
            fmatrix->buffer[ix + 2] = (fmatrix->buffer[ix + 2] * torch_std[2]) + torch_mean[2];
        }
    }
    else if (block->image_scaling == EI_CLASSIFIER_IMAGE_SCALING_MIN128_127) {
        int scale_res = numpy::add(fmatrix, 128.0f);
        if (scale_res != EIDSP_OK) {
            ei_printf("ERR: Failed to scale matrix (%d)\n", scale_res);
            return EI_IMPULSE_DSP_ERROR;
        }
        scale_res = numpy::scale(fmatrix, 1 / 255.0f);
        if (scale_res != EIDSP_OK) {
            ei_printf("ERR: Failed to scale matrix (%d)\n", scale_res);
            return EI_IMPULSE_DSP_ERROR;
        }
    }
    else if (block->image_scaling == EI_CLASSIFIER_IMAGE_SCALING_MIN1_1) {
        int scale_res = numpy::add(fmatrix, 1.0f);
        if (scale_res != EIDSP_OK) {
            ei_printf("ERR: Failed to scale matrix (%d)\n", scale_res);
            return EI_IMPULSE_DSP_ERROR;
        }
        scale_res = numpy::scale(fmatrix, 1 / 2.0f);
        if (scale_res != EIDSP_OK) {
            ei_printf("ERR: Failed to scale matrix (%d)\n", scale_res);
            return EI_IMPULSE_DSP_ERROR;
        }
    }
    else if (block->image_scaling == EI_CLASSIFIER_IMAGE_SCALING_0_255) {
        int scale_res = numpy::scale(fmatrix, 1 / 255.0f);
        if (scale_res != EIDSP_OK) {
            ei_printf("ERR: Failed to scale matrix (%d)\n", scale_res);
            return EI_IMPULSE_DSP_ERROR;
        }
    }
    else if (block->image_scaling == EI_CLASSIFIER_IMAGE_SCALING_BGR_SUBTRACT_IMAGENET_MEAN) {
        // Transpose BGR to RGB and add mean
        for (size_t ix = 0; ix < fmatrix->rows * fmatrix->cols; ix += 3) {
            float b = fmatrix->buffer[ix + 0];
            fmatrix->buffer[ix + 0] = fmatrix->buffer[ix + 2] + tao_mean[2];
            fmatrix->buffer[ix + 1] += tao_mean[1];
            fmatrix->buffer[ix + 2] = b + tao_mean[0];
        }
        int scale_res = numpy::scale(fmatrix, 1 / 255.0f);
        if (scale_res != EIDSP_OK) {
            ei_printf("ERR: Failed to scale matrix (%d)\n", scale_res);
            return EI_IMPULSE_DSP_ERROR;
        }
    }
    return EI_IMPULSE_OK;
}
#endif

/* Public functions ------------------------------------------------------- */

/* Tread carefully: public functions are not to be changed
to preserve backwards compatibility. Anything in this public section
will be documented by Doxygen. */

/**
 * @defgroup ei_functions Functions
 * 
 * Public-facing functions for running inference using the Edge Impulse C++ library. 
 * 
 * **Source**: [classifier/ei_run_classifier.h](https://github.com/edgeimpulse/inferencing-sdk-cpp/blob/master/classifier/ei_run_classifier.h)
 * 
 * @addtogroup ei_functions
 * @{
 */

/**
 * @brief Initialize static variables for running preprocessing and inference 
 *  continuously.
 * 
 * Initializes and clears any internal static variables needed by `run_classifier_continuous()`.
 * This includes the moving average filter (MAF). This function should be called prior to
 * calling `run_classifier_continuous()`.
 * 
 * **Blocking**: yes
 * 
 * **Example**: [nano_ble33_sense_microphone_continuous.ino](https://github.com/edgeimpulse/example-lacuna-ls200/blob/main/nano_ble33_sense_microphone_continous/nano_ble33_sense_microphone_continuous.ino)
 */
extern "C" void run_classifier_init(void)
{

    classifier_continuous_features_written = 0;
    ei_dsp_clear_continuous_audio_state();
    init_impulse(&ei_default_impulse);

#if EI_CLASSIFIER_CALIBRATION_ENABLED

    const auto impulse = ei_default_impulse.impulse;
    const ei_model_performance_calibration_t *calibration = &impulse->calibration;

    if(calibration != NULL) {
        avg_scores = new RecognizeEvents(calibration,
            impulse->label_count, impulse->slice_size, impulse->interval_ms);
    }
#endif
}

/**
 * @brief Initialize static variables for running preprocessing and inference 
 *  continuously.
 * 
 * Initializes and clears any internal static variables needed by `run_classifier_continuous()`.
 * This includes the moving average filter (MAF). This function should be called prior to
 * calling `run_classifier_continuous()`.
 * 
 * **Blocking**: yes
 * 
 * **Example**: [nano_ble33_sense_microphone_continuous.ino](https://github.com/edgeimpulse/example-lacuna-ls200/blob/main/nano_ble33_sense_microphone_continous/nano_ble33_sense_microphone_continuous.ino)
 * 
 * @param[in]   handle struct with information about model and DSP
 */
__attribute__((unused)) void run_classifier_init(ei_impulse_handle_t *handle)
{
    classifier_continuous_features_written = 0;
    ei_dsp_clear_continuous_audio_state();
    init_impulse(handle);

#if EI_CLASSIFIER_CALIBRATION_ENABLED
    auto impulse = handle->impulse;
    const ei_model_performance_calibration_t *calibration = &impulse->calibration;

    if(calibration != NULL) {
        avg_scores = new RecognizeEvents(calibration,
            impulse->label_count, impulse->slice_size, impulse->interval_ms);
    }
#endif
}

/**
 * @brief Deletes static variables when running preprocessing and inference continuously.
 * 
 * Deletes internal static variables used by `run_classifier_continuous()`, which
 * includes the moving average filter (MAF). This function should be called when you
 * are done running continuous classification.
 * 
 * **Blocking**: yes
 * 
 * **Example**: [ei_run_audio_impulse.cpp](https://github.com/edgeimpulse/firmware-nordic-thingy53/blob/main/src/inference/ei_run_audio_impulse.cpp)
 */
extern "C" void run_classifier_deinit(void)
{
    if((void *)avg_scores != NULL) {
        delete avg_scores;
    }
}

/**
 * @brief Run preprocessing (DSP) on new slice of raw features. Add output features 
 *  to rolling matrix and run inference on full sample.
 *
 * Accepts a new slice of features give by the callback defined in the `signal` parameter. 
 * It performs preprocessing (DSP) on this new slice of features and appends the output to 
 * a sliding window of pre-processed features (stored in a static features matrix). The matrix
 * stores the new slice and as many old slices as necessary to make up one full sample for 
 * performing inference.
 * 
 * `run_classifier_init()` must be called before making any calls to 
 * `run_classifier_continuous().`
 * 
 * For example, if you are doing keyword spotting on 1-second slices of audio and you want to
 * perform inference 4 times per second (given by `EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW`), you
 * would collect 0.25 seconds of audio and call run_classifier_continuous(). The function would
 * compute the Mel-Frequency Cepstral Coefficients (MFCCs) for that 0.25 second slice of audio,
 * drop the oldest 0.25 seconds' worth of MFCCs from its internal matrix, and append the newest
 * slice of MFCCs. This process allows the library to keep track of the pre-processed features
 * (e.g. MFCCs) in the window instead of the entire set of raw features (e.g. raw audio data),
 * which can potentially save a lot of space in RAM. After updating the static matrix, 
 * inference is performed using the whole matrix, which acts as a sliding window of 
 * pre-processed features.
 * 
 * Additionally, a moving average filter (MAF) can be enabled for `run_classifier_continuous()`, 
 * which averages (arithmetic mean) the last *n* inference results for each class. *n* is 
 * `EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW / 2`. In our example above, if we enabled the MAF, the 
 * values in `result` would contain predictions averaged from the previous 2 inferences.
 * 
 * To learn more about `run_classifier_continuous()`, see 
 * [this guide](https://docs.edgeimpulse.com/docs/tutorials/advanced-inferencing/continuous-audio-sampling) 
 * on continuous audio sampling. While the guide is written for audio signals, the concepts of continuous sampling and inference can be extrapolated to any time-series data.
 * 
 * **Blocking**: yes
 * 
 * **Example**: [nano_ble33_sense_microphone_continuous.ino](https://github.com/edgeimpulse/example-lacuna-ls200/blob/main/nano_ble33_sense_microphone_continous/nano_ble33_sense_microphone_continuous.ino)
 * 
 * @param[in] signal  Pointer to a signal_t struct that contains the number of elements in the 
 *  slice of raw features (e.g. `EI_CLASSIFIER_SLICE_SIZE`) and a pointer to a callback that reads 
 *  in the slice of raw features.
 * @param[out] result Pointer to an `ei_impulse_result_t` struct that contains the various output 
 *  results from inference after run_classifier() returns.
 * @param[in]  debug Print internal preprocessing and inference debugging information via 
 *  `ei_printf()`.
 * @param[in]  enable_maf Enable the moving average filter (MAF) for the classifier.
 *
 * @return Error code as defined by `EI_IMPULSE_ERROR` enum. Will be `EI_IMPULSE_OK` if inference 
 *  completed successfully.
 */
extern "C" EI_IMPULSE_ERROR run_classifier_continuous(
    signal_t *signal,
    ei_impulse_result_t *result,
    bool debug = false,
    bool enable_maf = true)
{
    auto& impulse = ei_default_impulse;
    return process_impulse_continuous(&impulse, signal, result, debug, enable_maf);
}

/**
 * @brief Run preprocessing (DSP) on new slice of raw features. Add output features 
 *  to rolling matrix and run inference on full sample.
 *
 * Accepts a new slice of features give by the callback defined in the `signal` parameter. 
 * It performs preprocessing (DSP) on this new slice of features and appends the output to 
 * a sliding window of pre-processed features (stored in a static features matrix). The matrix
 * stores the new slice and as many old slices as necessary to make up one full sample for 
 * performing inference.
 * 
 * `run_classifier_init()` must be called before making any calls to 
 * `run_classifier_continuous().`
 * 
 * For example, if you are doing keyword spotting on 1-second slices of audio and you want to
 * perform inference 4 times per second (given by `EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW`), you
 * would collect 0.25 seconds of audio and call run_classifier_continuous(). The function would
 * compute the Mel-Frequency Cepstral Coefficients (MFCCs) for that 0.25 second slice of audio,
 * drop the oldest 0.25 seconds' worth of MFCCs from its internal matrix, and append the newest
 * slice of MFCCs. This process allows the library to keep track of the pre-processed features
 * (e.g. MFCCs) in the window instead of the entire set of raw features (e.g. raw audio data),
 * which can potentially save a lot of space in RAM. After updating the static matrix, 
 * inference is performed using the whole matrix, which acts as a sliding window of 
 * pre-processed features.
 * 
 * Additionally, a moving average filter (MAF) can be enabled for `run_classifier_continuous()`, 
 * which averages (arithmetic mean) the last *n* inference results for each class. *n* is 
 * `EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW / 2`. In our example above, if we enabled the MAF, the 
 * values in `result` would contain predictions averaged from the previous 2 inferences.
 * 
 * To learn more about `run_classifier_continuous()`, see 
 * [this guide](https://docs.edgeimpulse.com/docs/tutorials/advanced-inferencing/continuous-audio-sampling) 
 * on continuous audio sampling. While the guide is written for audio signals, the concepts of continuous sampling and inference can be extrapolated to any time-series data.
 * 
 * **Blocking**: yes
 * 
 * **Example**: [nano_ble33_sense_microphone_continuous.ino](https://github.com/edgeimpulse/example-lacuna-ls200/blob/main/nano_ble33_sense_microphone_continous/nano_ble33_sense_microphone_continuous.ino)
 * 
 * @param[in] impulse `ei_impulse_handle_t` struct with information about preprocessing and model.
 * @param[in] signal  Pointer to a signal_t struct that contains the number of elements in the 
 *  slice of raw features (e.g. `EI_CLASSIFIER_SLICE_SIZE`) and a pointer to a callback that reads 
 *  in the slice of raw features.
 * @param[out] result Pointer to an `ei_impulse_result_t` struct that contains the various output 
 *  results from inference after run_classifier() returns.
 * @param[in] debug Print internal preprocessing and inference debugging information via 
 *  `ei_printf()`.
 * @param[in] enable_maf Enable the moving average filter (MAF) for the classifier.
 *
 * @return Error code as defined by `EI_IMPULSE_ERROR` enum. Will be `EI_IMPULSE_OK` if inference 
 *  completed successfully.
 */
__attribute__((unused)) EI_IMPULSE_ERROR run_classifier_continuous(
    ei_impulse_handle_t *impulse,
    signal_t *signal,
    ei_impulse_result_t *result,
    bool debug = false,
    bool enable_maf = true)
{
    return process_impulse_continuous(impulse, signal, result, debug, enable_maf);
}

/**
 * @brief Run the classifier over a raw features array.
 * 
 * 
 * Overloaded function [run_classifier()](#run_classifier-1) that defaults to the single impulse.
 * 
 * **Blocking**: yes
 * 
 * @param[in] signal Pointer to a `signal_t` struct that contains the total length of the raw 
 *  feature array, which must match EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, and a pointer to a callback
 *  that reads in the raw features.
 * @param[out] result  Pointer to an ei_impulse_result_t struct that will contain the various output 
 *  results from inference after `run_classifier()` returns.
 * @param[in] debug Print internal preprocessing and inference debugging information via `ei_printf()`.
 * 
 * @return Error code as defined by `EI_IMPULSE_ERROR` enum. Will be `EI_IMPULSE_OK` if inference
 *  completed successfully.
 */
extern "C" EI_IMPULSE_ERROR run_classifier(
    signal_t *signal,
    ei_impulse_result_t *result,
    bool debug = false)
{
    return process_impulse(&ei_default_impulse, signal, result, debug);
}

/**
 * @brief Run the classifier over a raw features array.
 * 
 * 
 * Accepts a `signal_t` input struct pointing to a callback that reads in pages of raw features. 
 * `run_classifier()` performs any necessary preprocessing on the raw features (e.g. DSP, cropping 
 * of images, etc.) before performing inference. Results from inference are stored in an 
 * `ei_impulse_result_t` struct.
 * 
 * **Blocking**: yes
 * 
 * **Example**: [standalone inferencing main.cpp](https://github.com/edgeimpulse/example-standalone-inferencing/blob/master/source/main.cpp)
 * 
 * @param[in] impulse Pointer to an `ei_impulse_handle_t` struct that contains the model and
 *  preprocessing information.
 * @param[in] signal Pointer to a `signal_t` struct that contains the total length of the raw 
 *  feature array, which must match EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, and a pointer to a callback
 *  that reads in the raw features.
 * @param[out] result  Pointer to an ei_impulse_result_t struct that will contain the various output 
 *  results from inference after `run_classifier()` returns.
 * @param[in] debug Print internal preprocessing and inference debugging information via `ei_printf()`.
 * 
 * @return Error code as defined by `EI_IMPULSE_ERROR` enum. Will be `EI_IMPULSE_OK` if inference
 *  completed successfully.
 */
__attribute__((unused)) EI_IMPULSE_ERROR run_classifier(
    ei_impulse_handle_t *impulse,
    signal_t *signal,
    ei_impulse_result_t *result,
    bool debug = false)
{
    return process_impulse(impulse, signal, result, debug);
}

/** @} */ // end of ei_functions Doxygen group

/* Deprecated functions ------------------------------------------------------- */

/* These functions are being deprecated and possibly will be removed or moved in future.
Do not use these - if possible, change your code to reflect the upcoming changes. */

#if EIDSP_SIGNAL_C_FN_POINTER == 0

/**
 * @brief Run the impulse, if you provide an instance of sampler it will also persist 
 *  the data for you.
 * 
 * @deprecated This function is deprecated and will be removed in future versions. Use 
 *  `run_classifier()` instead.
 * 
 * @param[in] sampler Instance to an **initialized** sampler
 * @param[out] result Object to store the results in
 * @param[in] data_fn Callback function to retrieve data from sensors
 * @param[in] debug Whether to log debug messages (default false)
 * 
 * @return Error code as defined by `EI_IMPULSE_ERROR` enum. Will be `EI_IMPULSE_OK` if inference
 *  completed successfully.
 */
__attribute__((unused)) EI_IMPULSE_ERROR run_impulse(
#if (defined(EI_CLASSIFIER_HAS_SAMPLER) && EI_CLASSIFIER_HAS_SAMPLER == 1) || defined(__DOXYGEN__)
        EdgeSampler *sampler,
#endif
        ei_impulse_result_t *result,
#ifdef __MBED__
        mbed::Callback<void(float*, size_t)> data_fn,
#else
        std::function<void(float*, size_t)> data_fn,
#endif
        bool debug = false) {

    auto& impulse = *(ei_default_impulse.impulse);

    float *x = (float*)calloc(impulse.dsp_input_frame_size, sizeof(float));
    if (!x) {
        return EI_IMPULSE_OUT_OF_MEMORY;
    }

    uint64_t next_tick = 0;

    uint64_t sampling_us_start = ei_read_timer_us();

    // grab some data
    for (int i = 0; i < (int)impulse.dsp_input_frame_size; i += impulse.raw_samples_per_frame) {
        uint64_t curr_us = ei_read_timer_us() - sampling_us_start;

        next_tick = curr_us + (impulse.interval_ms * 1000);

        data_fn(x + i, impulse.raw_samples_per_frame);
#if defined(EI_CLASSIFIER_HAS_SAMPLER) && EI_CLASSIFIER_HAS_SAMPLER == 1
        if (sampler != NULL) {
            sampler->write_sensor_data(x + i, impulse.raw_samples_per_frame);
        }
#endif

        if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
            free(x);
            return EI_IMPULSE_CANCELED;
        }

        while (next_tick > ei_read_timer_us() - sampling_us_start);
    }

    result->timing.sampling = (ei_read_timer_us() - sampling_us_start) / 1000;

    signal_t signal;
    int err = numpy::signal_from_buffer(x, impulse.dsp_input_frame_size, &signal);
    if (err != 0) {
        free(x);
        ei_printf("ERR: signal_from_buffer failed (%d)\n", err);
        return EI_IMPULSE_DSP_ERROR;
    }

    EI_IMPULSE_ERROR r = run_classifier(&signal, result, debug);
    free(x);
    return r;
}

#if (defined(EI_CLASSIFIER_HAS_SAMPLER) && EI_CLASSIFIER_HAS_SAMPLER == 1) || defined(__DOXYGEN__)
/**
 * @brief Run the impulse, does not persist data.
 * 
 * @deprecated This function is deprecated and will be removed in future versions. Use 
 *  `run_classifier()` instead.
 * 
 * @param[out] result Object to store the results in
 * @param[in] data_fn Callback function to retrieve data from sensors
 * @param[out] debug Whether to log debug messages (default false)
 * 
 * @return Error code as defined by `EI_IMPULSE_ERROR` enum. Will be `EI_IMPULSE_OK` if inference
 *  completed successfully.
 */
__attribute__((unused)) EI_IMPULSE_ERROR run_impulse(
        ei_impulse_result_t *result,
#ifdef __MBED__
        mbed::Callback<void(float*, size_t)> data_fn,
#else
        std::function<void(float*, size_t)> data_fn,
#endif
        bool debug = false) {
    return run_impulse(NULL, result, data_fn, debug);
}
#endif

#endif // #if EIDSP_SIGNAL_C_FN_POINTER == 0

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _EDGE_IMPULSE_RUN_CLASSIFIER_H_
