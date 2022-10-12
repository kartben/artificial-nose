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

#ifndef _EI_CLASSIFIER_INFERENCING_ENGINE_TENSAILFOW_H_
#define _EI_CLASSIFIER_INFERENCING_ENGINE_TENSAILFOW_H_

#if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSAIFLOW)

#include "model-parameters/model_metadata.h"
#if EI_CLASSIFIER_HAS_MODEL_VARIABLES == 1
#include "model-parameters/model_variables.h"
#endif

#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "model-parameters/dsp_blocks.h"
#include "edge-impulse-sdk/classifier/ei_fill_result_struct.h"

#include "mcu.h"

extern "C" void infer(const void *impulse_arg, uint32_t* time, uint32_t* cycles);
int8_t *processed_features;
int8_t infer_result[EI_CLASSIFIER_MAX_LABELS_COUNT];

extern "C" void get_data(const void *impulse_arg, int8_t *in_buf_0, uint16_t in_buf_0_dim_0, uint16_t in_buf_0_dim_1, uint16_t in_buf_0_dim_2)
{
    ei_impulse_t *impulse = (ei_impulse_t *) impulse_arg;

    if ((impulse->sensor == EI_CLASSIFIER_SENSOR_CAMERA) &&
        ((ei_dsp_blocks_size == 1) ||
        (ei_dsp_blocks[0].extract_fn == extract_image_features))) {

        memcpy(in_buf_0, processed_features, impulse->nn_input_frame_size);
    }
}

extern "C" void post_process(const void *impulse_arg, int8_t *out_buf_0, int8_t *out_buf_1)
{
    ei_impulse_t *impulse = (ei_impulse_t *) impulse_arg;
    memcpy(infer_result, out_buf_0, impulse->label_count);
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
    if (impulse->object_detection) {
        ei_printf("ERR: Object detection models are not supported with TensaiFlow\n");
        return EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE;
    }

    uint64_t ctx_start_us = ei_read_timer_us();
    uint32_t time, cycles;

    /* Run tensaiflow inference */
    infer((const void *)impulse, &time, &cycles);

    // Inference results returned by post_process() and copied into infer_results

    result->timing.classification_us = ei_read_timer_us() - ctx_start_us;
    result->timing.classification = (int)(result->timing.classification_us / 1000);

    for (uint32_t ix = 0; ix < impulse->label_count; ix++) {
        float value;
        // Dequantize the output if it is int8
        value = static_cast<float>(infer_result[ix] - impulse->tflite_output_zeropoint) *
            impulse->tflite_output_scale;

        if (debug) {
            ei_printf("%s:\t", ei_classifier_inferencing_categories[ix]);
            ei_printf_float(value);
            ei_printf("\n");
        }
        result->classification[ix].label = ei_classifier_inferencing_categories[ix];
        result->classification[ix].value = value;
    }

    return EI_IMPULSE_OK;

}

/**
 * Special function to run the classifier on images, only works on TFLite models (either interpreter or EON or for tensaiflow)
 * that allocates a lot less memory by quantizing in place. This only works if 'can_run_classifier_image_quantized'
 * returns EI_IMPULSE_OK.
 */
EI_IMPULSE_ERROR run_nn_inference_image_quantized(
    const ei_impulse_t *impulse,
    signal_t *signal,
    ei_impulse_result_t *result,
    bool debug = false)
{

    uint64_t ctx_start_us;
    uint64_t dsp_start_us = ei_read_timer_us();

    ei::matrix_i8_t features_matrix(1, impulse->nn_input_frame_size);
    processed_features = (int8_t *) features_matrix.buffer;

    // run DSP process and quantize automatically
    int ret = extract_image_features_quantized(impulse, signal, &features_matrix, ei_dsp_blocks[0].config, impulse->frequency);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: Failed to run DSP process (%d)\n", ret);
        return EI_IMPULSE_DSP_ERROR;
    }

    if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
        return EI_IMPULSE_CANCELED;
    }

    result->timing.dsp_us = ei_read_timer_us() - dsp_start_us;
    result->timing.dsp = (int)(result->timing.dsp_us / 1000);

    if (debug) {
        ei_printf("Features (%d ms.): ", result->timing.dsp);
        for (size_t ix = 0; ix < features_matrix.cols; ix++) {
            ei_printf_float((features_matrix.buffer[ix] - impulse->tflite_input_zeropoint) * impulse->tflite_input_scale);
            ei_printf(" ");
        }
        ei_printf("\n");
    }

    uint32_t time, cycles;
    ctx_start_us = ei_read_timer_us();

    /* Run tensaiflow inference */
    infer((const void *)impulse, &time, &cycles);

    // Inference results returned by post_process() and copied into infer_results

    result->timing.classification_us = ei_read_timer_us() - ctx_start_us;
    result->timing.classification = (int)(result->timing.classification_us / 1000);

    for (uint32_t ix = 0; ix < impulse->label_count; ix++) {
        float value;
        // Dequantize the output if it is int8
        value = static_cast<float>(infer_result[ix] - impulse->tflite_output_zeropoint) *
            impulse->tflite_output_scale;

        if (debug) {
            ei_printf("%s:\t", ei_classifier_inferencing_categories[ix]);
            ei_printf_float(value);
            ei_printf("\n");
        }
        result->classification[ix].label = ei_classifier_inferencing_categories[ix];
        result->classification[ix].value = value;
    }

    return EI_IMPULSE_OK;

}

#endif // #if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSAILFOW)
#endif // _EI_CLASSIFIER_INFERENCING_ENGINE_TENSAILFOW_H_
