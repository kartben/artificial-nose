/* Edge Impulse inferencing library
 * Copyright (c) 2022 EdgeImpulse Inc.
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

#ifndef _EI_CLASSIFIER_INFERENCING_ENGINE_TENSORRT_H_
#define _EI_CLASSIFIER_INFERENCING_ENGINE_TENSORRT_H_

#if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSORRT)

#include "model-parameters/model_metadata.h"
#if EI_CLASSIFIER_HAS_MODEL_VARIABLES == 1
#include "model-parameters/model_variables.h"
#endif

#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "model-parameters/dsp_blocks.h"
#include "edge-impulse-sdk/classifier/ei_fill_result_struct.h"

#include <stdlib.h>
#include "tflite/linux-jetson-nano/libeitrt.h"
EiTrt* ei_trt_handle = NULL;

inline bool file_exists(char *model_file_name) {
    if (FILE *file = fopen(model_file_name, "r")) {
        fclose(file);
        return true;
    } else {
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
    ei::matrix_t *fmatrix,
    ei_impulse_result_t *result,
    bool debug = false)
{

    #if EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1
    #error "TensorRT requires an unquantized network"
    #endif

    if (impulse->object_detection) {
        ei_printf("ERR: Object detection models are not supported with TensorRT\n");
        return EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE;
    }

    static char model_file_name[128];
    snprintf(model_file_name, 128, "/tmp/%s-%d-%d", impulse->project_name, impulse->project_id, impulse->deploy_version);

    static bool first_run = !file_exists(model_file_name);
    if (first_run) {
        ei_printf("INFO: Model file '%s' does not exist, writing now \n", model_file_name);

        FILE *file = fopen(model_file_name, "w");
        if (!file) {
            ei_printf("ERR: TensorRT init failed to open '%s'\n", model_file_name);
            return EI_IMPULSE_TENSORRT_INIT_FAILED;
        }

        if (fwrite(impulse->model_arr, impulse->model_arr_size, 1, file) != 1) {
            ei_printf("ERR: TensorRT init fwrite failed\n");
            return EI_IMPULSE_TENSORRT_INIT_FAILED;
        }

        if (fclose(file) != 0) {
            ei_printf("ERR: TensorRT init fclose failed\n");
            return EI_IMPULSE_TENSORRT_INIT_FAILED;
        }
    }

    float tensorrt_output[impulse->label_count];

    // lazy initialize tensorRT context
    if( ei_trt_handle == nullptr ) {
        ei_trt_handle = libeitrt::create_EiTrt(model_file_name, debug);
    }

    uint64_t ctx_start_us = ei_read_timer_us();

    libeitrt::infer(ei_trt_handle, fmatrix->buffer, tensorrt_output, impulse->label_count);

    uint64_t ctx_end_us = ei_read_timer_us();

    result->timing.classification_us = ctx_end_us - ctx_start_us;
    result->timing.classification = (int)(result->timing.classification_us / 1000);

    for( int i = 0; i < impulse->label_count; ++i) {
        result->classification[i].label = ei_classifier_inferencing_categories[i];
        result->classification[i].value = tensorrt_output[i];
    }

    return EI_IMPULSE_OK;

}

/**
 * Special function to run the classifier on images, only works on TFLite models (either interpreter or EON or for tensaiflow)
 * that allocates a lot less memory by quantizing in place. This only works if 'can_run_classifier_image_quantized'
 * returns EI_IMPULSE_OK.
 */
EI_IMPULSE_ERROR run_nn_inference_image_quantized(
    ei_impulse_t *impulse,
    signal_t *signal,
    ei_impulse_result_t *result,
    bool debug = false)
{
    return EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE;
}

#endif // #if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSORRT)
#endif // _EI_CLASSIFIER_INFERENCING_ENGINE_TENSORRT_H_
