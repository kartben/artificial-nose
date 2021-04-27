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

#ifndef _EDGE_IMPULSE_RUN_CLASSIFIER_H_
#define _EDGE_IMPULSE_RUN_CLASSIFIER_H_

#include "model-parameters/model_metadata.h"

#if EI_CLASSIFIER_HAS_ANOMALY == 1
#include "model-parameters/anomaly_clusters.h"
#endif
#include "ei_run_dsp.h"
#include "ei_classifier_types.h"
#include "ei_classifier_smooth.h"
#if defined(EI_CLASSIFIER_HAS_SAMPLER) && EI_CLASSIFIER_HAS_SAMPLER == 1
#include "ei_sampler.h"
#endif
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "model-parameters/dsp_blocks.h"

#if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE) && (EI_CLASSIFIER_COMPILED != 1)
#include <cmath>
#include "edge-impulse-sdk/tensorflow/lite/micro/all_ops_resolver.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_error_reporter.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_interpreter.h"
#include "edge-impulse-sdk/tensorflow/lite/schema/schema_generated.h"
#include "edge-impulse-sdk/tensorflow/lite/version.h"
#include "edge-impulse-sdk/classifier/ei_aligned_malloc.h"

#include "tflite-model/tflite-trained.h"
#if defined(EI_CLASSIFIER_HAS_TFLITE_OPS_RESOLVER) && EI_CLASSIFIER_HAS_TFLITE_OPS_RESOLVER == 1
#include "tflite-model/tflite-resolver.h"
#endif // EI_CLASSIFIER_HAS_TFLITE_OPS_RESOLVER

#if defined(EI_CLASSIFIER_ENABLE_DETECTION_POSTPROCESS_OP)
namespace tflite {
namespace ops {
namespace micro {
extern TfLiteRegistration *Register_TFLite_Detection_PostProcess(void);
}  // namespace micro
}  // namespace ops

extern float post_process_boxes[10 * 4 * sizeof(float)];
extern float post_process_classes[10];
extern float post_process_scores[10];

}  // namespace tflite
#endif

static tflite::MicroErrorReporter micro_error_reporter;
static tflite::ErrorReporter* error_reporter = &micro_error_reporter;
#elif EI_CLASSIFIER_COMPILED == 1
#include "edge-impulse-sdk/tensorflow/lite/c/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tflite-model/trained_model_compiled.h"
#include "edge-impulse-sdk/classifier/ei_aligned_malloc.h"

namespace tflite {
extern float post_process_boxes[10 * 4 * sizeof(float)];
extern float post_process_classes[10];
extern float post_process_scores[10];
} // namespace tflite

#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE_FULL

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"
#include "tflite-model/tflite-trained.h"

#elif (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSORRT)
#include <stdlib.h>
#include "tflite-model/onnx-trained.h"
#include "tflite/linux-jetson-nano/libeitrt.h"
EiTrt* ei_trt_handle = NULL;

#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_NONE
// noop
#else
#error "Unknown inferencing engine"
#endif

#if ECM3532
void*   __dso_handle = (void*) &__dso_handle;
#endif

#ifdef __cplusplus
namespace {
#endif // __cplusplus

/* Function prototypes ----------------------------------------------------- */
extern "C" EI_IMPULSE_ERROR run_inference(ei::matrix_t *fmatrix, ei_impulse_result_t *result, bool debug);
extern "C" EI_IMPULSE_ERROR run_classifier_image_quantized(signal_t *signal, ei_impulse_result_t *result, bool debug);
static EI_IMPULSE_ERROR can_run_classifier_image_quantized();
static void calc_cepstral_mean_and_var_normalization_mfcc(ei_matrix *matrix, void *config_ptr);
static void calc_cepstral_mean_and_var_normalization_mfe(ei_matrix *matrix, void *config_ptr);
static void calc_cepstral_mean_and_var_normalization_spectrogram(ei_matrix *matrix, void *config_ptr);

/* Private variables ------------------------------------------------------- */
#if EI_CLASSIFIER_LABEL_COUNT > 0
ei_impulse_maf classifier_maf[EI_CLASSIFIER_LABEL_COUNT] = {{0}};
#else
ei_impulse_maf classifier_maf[0];
#endif
static size_t slice_offset = 0;
static bool feature_buffer_full = false;

/* Private functions ------------------------------------------------------- */

/**
 * @brief      Run a moving average filter over the classification result.
 *             The size of the filter determines the response of the filter.
 *             It is now set to the number of slices per window.
 * @param      maf             Pointer to maf object
 * @param[in]  classification  Classification output on current slice
 *
 * @return     Averaged classification value
 */
extern "C" float run_moving_average_filter(ei_impulse_maf *maf, float classification)
{
    maf->running_sum -= maf->maf_buffer[maf->buf_idx];
    maf->running_sum += classification;
    maf->maf_buffer[maf->buf_idx] = classification;

    if (++maf->buf_idx >= (EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW >> 1)) {
        maf->buf_idx = 0;
    }

#if (EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW > 1)
    return maf->running_sum / (float)(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW >> 1);
#else
    return maf->running_sum;
#endif
}

/**
 * @brief      Reset all values in filter to 0
 *
 * @param      maf   Pointer to maf object
 */
static void clear_moving_average_filter(ei_impulse_maf *maf)
{
    maf->running_sum = 0;

    for (int i = 0; i < (EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW >> 1); i++) {
        maf->maf_buffer[i] = 0.f;
    }
}

/**
 * @brief      Init static vars
 */
extern "C" void run_classifier_init(void)
{
    slice_offset = 0;
    feature_buffer_full = false;

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        clear_moving_average_filter(&classifier_maf[ix]);
    }
}

/**
 * @brief      Fill the complete matrix with sample slices. From there, run inference
 *             on the matrix.
 *
 * @param      signal  Sample data
 * @param      result  Classification output
 * @param[in]  debug   Debug output enable boot
 * @param      enable_maf Enables the moving average filter
 *
 * @return     The ei impulse error.
 */
extern "C" EI_IMPULSE_ERROR run_classifier_continuous(signal_t *signal, ei_impulse_result_t *result,
                                                      bool debug = false, bool enable_maf = true)
{
    static ei::matrix_t static_features_matrix(1, EI_CLASSIFIER_NN_INPUT_FRAME_SIZE);
    if (!static_features_matrix.buffer) {
        return EI_IMPULSE_ALLOC_FAILED;
    }

    EI_IMPULSE_ERROR ei_impulse_error = EI_IMPULSE_OK;

    uint64_t dsp_start_ms = ei_read_timer_ms();

    size_t out_features_index = 0;
    size_t feature_size;
    bool is_mfcc = false;
    bool is_mfe = false;
    bool is_spectrogram = false;

    for (size_t ix = 0; ix < ei_dsp_blocks_size; ix++) {
        ei_model_dsp_t block = ei_dsp_blocks[ix];

        if (out_features_index + block.n_output_features > EI_CLASSIFIER_NN_INPUT_FRAME_SIZE) {
            ei_printf("ERR: Would write outside feature buffer\n");
            return EI_IMPULSE_DSP_ERROR;
        }

        ei::matrix_t fm(1, block.n_output_features,
                        static_features_matrix.buffer + out_features_index + slice_offset);

        /* Switch to the slice version of the mfcc feature extract function */
        if (block.extract_fn == extract_mfcc_features) {
            block.extract_fn = &extract_mfcc_per_slice_features;
            is_mfcc = true;
        }
        else if (block.extract_fn == extract_spectrogram_features) {
            block.extract_fn = &extract_spectrogram_per_slice_features;
            is_spectrogram = true;
        }
        else if (block.extract_fn == extract_mfe_features) {
            block.extract_fn = &extract_mfe_per_slice_features;
            is_mfe = true;
        }
        else {
            ei_printf("ERR: Unknown extract function, only MFCC, MFE and spectrogram supported\n");
            return EI_IMPULSE_DSP_ERROR;
        }

        int ret = block.extract_fn(signal, &fm, block.config, EI_CLASSIFIER_FREQUENCY);
        if (ret != EIDSP_OK) {
            ei_printf("ERR: Failed to run DSP process (%d)\n", ret);
            return EI_IMPULSE_DSP_ERROR;
        }

        if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
            return EI_IMPULSE_CANCELED;
        }

        out_features_index += block.n_output_features;

        feature_size = (fm.rows * fm.cols);
    }

    /* For as long as the feature buffer isn't completely full, keep moving the slice offset */
    if (feature_buffer_full == false) {
        slice_offset += feature_size;

        if (slice_offset > (EI_CLASSIFIER_NN_INPUT_FRAME_SIZE - feature_size)) {
            feature_buffer_full = true;
            slice_offset -= feature_size;
        }
    }

    result->timing.dsp = ei_read_timer_ms() - dsp_start_ms;

    if (debug) {
        ei_printf("\r\nFeatures (%d ms.): ", result->timing.dsp);
        for (size_t ix = 0; ix < static_features_matrix.cols; ix++) {
            ei_printf_float(static_features_matrix.buffer[ix]);
            ei_printf(" ");
        }
        ei_printf("\n");
    }

#if EI_CLASSIFIER_INFERENCING_ENGINE != EI_CLASSIFIER_NONE
    if (debug) {
        ei_printf("Running neural network...\n");
    }
#endif

    if (feature_buffer_full == true) {
        dsp_start_ms = ei_read_timer_ms();
        ei::matrix_t classify_matrix(1, EI_CLASSIFIER_NN_INPUT_FRAME_SIZE);

        /* Create a copy of the matrix for normalization */
        for (size_t m_ix = 0; m_ix < EI_CLASSIFIER_NN_INPUT_FRAME_SIZE; m_ix++) {
            classify_matrix.buffer[m_ix] = static_features_matrix.buffer[m_ix];
        }

        if (is_mfcc) {
            calc_cepstral_mean_and_var_normalization_mfcc(&classify_matrix, ei_dsp_blocks[0].config);
        }
        else if (is_spectrogram) {
            calc_cepstral_mean_and_var_normalization_spectrogram(&classify_matrix, ei_dsp_blocks[0].config);
        }
        else if (is_mfe) {
            calc_cepstral_mean_and_var_normalization_mfe(&classify_matrix, ei_dsp_blocks[0].config);
        }
        result->timing.dsp += ei_read_timer_ms() - dsp_start_ms;

        ei_impulse_error = run_inference(&classify_matrix, result, debug);

        if (enable_maf) {
            for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    #if EI_CLASSIFIER_OBJECT_DETECTION != 1
                result->classification[ix].value =
                    run_moving_average_filter(&classifier_maf[ix], result->classification[ix].value);
    #endif
            }
        }

        /* Shift the feature buffer for new data */
        for (size_t i = 0; i < (EI_CLASSIFIER_NN_INPUT_FRAME_SIZE - feature_size); i++) {
            static_features_matrix.buffer[i] = static_features_matrix.buffer[i + feature_size];
        }
    }
    return ei_impulse_error;
}

#if EI_CLASSIFIER_OBJECT_DETECTION

/**
 * Fill the result structure from an unquantized output tensor
 * (we don't support quantized here a.t.m.)
 */
__attribute__((unused)) static void fill_result_struct_f32(ei_impulse_result_t *result, float *data, float *scores, float *labels, bool debug) {
    for (size_t ix = 0; ix < EI_CLASSIFIER_OBJECT_DETECTION_COUNT; ix++) {

        float score = scores[ix];
        float label = labels[ix];

#if EI_CLASSIFIER_TFLITE_INPUT_DATATYPE == EI_CLASSIFIER_DATATYPE_INT8
        // so for i8 inputs this seems to be 0.5..1.0 instead of 0.0..1.0
        // let's fix it by hand
        score = (score - 0.5f) * 2;
#endif

        if (score >= EI_CLASSIFIER_OBJECT_DETECTION_THRESHOLD) {
            float ystart = data[(ix * 4) + 0];
            float xstart = data[(ix * 4) + 1];
            float yend = data[(ix * 4) + 2];
            float xend = data[(ix * 4) + 3];

            if (xstart < 0) xstart = 0;
            if (xstart > 1) xstart = 1;
            if (ystart < 0) ystart = 0;
            if (ystart > 1) ystart = 1;
            if (yend < 0) yend = 0;
            if (yend > 1) yend = 1;
            if (xend < 0) xend = 0;
            if (xend > 1) xend = 1;
            if (xend < xstart) xend = xstart;
            if (yend < ystart) yend = ystart;

            if (debug) {
                ei_printf("%s (%f): %f [ %f, %f, %f, %f ]\n",
                    ei_classifier_inferencing_categories[(uint32_t)label], label, score, xstart, ystart, xend, yend);
            }

            result->bounding_boxes[ix].label = ei_classifier_inferencing_categories[(uint32_t)label];
            result->bounding_boxes[ix].x = static_cast<uint32_t>(xstart * static_cast<float>(EI_CLASSIFIER_INPUT_WIDTH));
            result->bounding_boxes[ix].y = static_cast<uint32_t>(ystart * static_cast<float>(EI_CLASSIFIER_INPUT_HEIGHT));
            result->bounding_boxes[ix].width = static_cast<uint32_t>((xend - xstart) * static_cast<float>(EI_CLASSIFIER_INPUT_WIDTH));
            result->bounding_boxes[ix].height = static_cast<uint32_t>((yend - ystart) * static_cast<float>(EI_CLASSIFIER_INPUT_HEIGHT));
            result->bounding_boxes[ix].value = score;
        }
        else {
            result->bounding_boxes[ix].value = 0.0f;
        }
    }
}

#else

/**
 * Fill the result structure from a quantized output tensor
 */
__attribute__((unused)) static void fill_result_struct_i8(ei_impulse_result_t *result, int8_t *data, float zero_point, float scale, bool debug) {
    for (uint32_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        float value = static_cast<float>(data[ix] - zero_point) * scale;

        if (debug) {
            ei_printf("%s:\t", ei_classifier_inferencing_categories[ix]);
            ei_printf_float(value);
            ei_printf("\n");
        }
        result->classification[ix].label = ei_classifier_inferencing_categories[ix];
        result->classification[ix].value = value;
    }
}

/**
 * Fill the result structure from an unquantized output tensor
 */
__attribute__((unused)) static void fill_result_struct_f32(ei_impulse_result_t *result, float *data, bool debug) {
    for (uint32_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        float value = data[ix];

        if (debug) {
            ei_printf("%s:\t", ei_classifier_inferencing_categories[ix]);
            ei_printf_float(value);
            ei_printf("\n");
        }
        result->classification[ix].label = ei_classifier_inferencing_categories[ix];
        result->classification[ix].value = value;
    }
}

#endif // EI_CLASSIFIER_OBJECT_DETECTION

#if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE)

/**
 * Setup the TFLite runtime
 *
 * @param      ctx_start_ms       Pointer to the start time
 * @param      input              Pointer to input tensor
 * @param      output             Pointer to output tensor
 * @param      micro_interpreter  Pointer to interpreter (for non-compiled models)
 * @param      micro_tensor_arena Pointer to the arena that will be allocated
 *
 * @return  EI_IMPULSE_OK if successful
 */
static EI_IMPULSE_ERROR inference_tflite_setup(uint64_t *ctx_start_ms, TfLiteTensor** input, TfLiteTensor** output,
#if EI_CLASSIFIER_OBJECT_DETECTION
    TfLiteTensor** output_labels,
    TfLiteTensor** output_scores,
#endif
#if (EI_CLASSIFIER_COMPILED != 1)
    tflite::MicroInterpreter** micro_interpreter,
#endif
    uint8_t** micro_tensor_arena) {
#if (EI_CLASSIFIER_COMPILED == 1)
    TfLiteStatus init_status = trained_model_init(ei_aligned_malloc);
    if (init_status != kTfLiteOk) {
        ei_printf("Failed to allocate TFLite arena (error code %d)\n", init_status);
        return EI_IMPULSE_TFLITE_ARENA_ALLOC_FAILED;
    }
#else
    // Create an area of memory to use for input, output, and intermediate arrays.
    uint8_t *tensor_arena = (uint8_t*)ei_aligned_malloc(16, EI_CLASSIFIER_TFLITE_ARENA_SIZE);
    if (tensor_arena == NULL) {
        ei_printf("Failed to allocate TFLite arena (%d bytes)\n", EI_CLASSIFIER_TFLITE_ARENA_SIZE);
        return EI_IMPULSE_TFLITE_ARENA_ALLOC_FAILED;
    }
    *micro_tensor_arena = tensor_arena;
#endif

    *ctx_start_ms = ei_read_timer_ms();

    static bool tflite_first_run = true;

#if (EI_CLASSIFIER_COMPILED != 1)
        static const tflite::Model* model = nullptr;
#endif

#if (EI_CLASSIFIER_COMPILED != 1)
    // ======
    // Initialization code start
    // This part can be run once, but that would require the TFLite arena
    // to be allocated at all times, which is not ideal (e.g. when doing MFCC)
    // ======
    if (tflite_first_run) {
        // Map the model into a usable data structure. This doesn't involve any
        // copying or parsing, it's a very lightweight operation.
        model = tflite::GetModel(trained_tflite);
        if (model->version() != TFLITE_SCHEMA_VERSION) {
            error_reporter->Report(
                "Model provided is schema version %d not equal "
                "to supported version %d.",
                model->version(), TFLITE_SCHEMA_VERSION);
            ei_aligned_free(tensor_arena);
            return EI_IMPULSE_TFLITE_ERROR;
        }
    }
#endif

#if (EI_CLASSIFIER_COMPILED != 1)
#ifdef EI_TFLITE_RESOLVER
    EI_TFLITE_RESOLVER
#else
    tflite::AllOpsResolver resolver;
#endif
#if defined(EI_CLASSIFIER_ENABLE_DETECTION_POSTPROCESS_OP)
    resolver.AddCustom("TFLite_Detection_PostProcess", tflite::ops::micro::Register_TFLite_Detection_PostProcess());
#endif
#endif // EI_CLASSIFIER_COMPILED != 1

#if (EI_CLASSIFIER_COMPILED == 1)
    *input = trained_model_input(0);
    *output = trained_model_output(0);
#if EI_CLASSIFIER_OBJECT_DETECTION
    *output_scores = trained_model_output(EI_CLASSIFIER_TFLITE_OUTPUT_SCORE_TENSOR);
    *output_labels = trained_model_output(EI_CLASSIFIER_TFLITE_OUTPUT_LABELS_TENSOR);
#endif // EI_CLASSIFIER_OBJECT_DETECTION
#else
    // Build an interpreter to run the model with.
    tflite::MicroInterpreter *interpreter = new tflite::MicroInterpreter(
        model, resolver, tensor_arena, EI_CLASSIFIER_TFLITE_ARENA_SIZE, error_reporter);

    *micro_interpreter = interpreter;

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        error_reporter->Report("AllocateTensors() failed");
        ei_aligned_free(tensor_arena);
        return EI_IMPULSE_TFLITE_ERROR;
    }

    // Obtain pointers to the model's input and output tensors.
    *input = interpreter->input(0);
    *output = interpreter->output(0);
#if EI_CLASSIFIER_OBJECT_DETECTION
    *output_scores = interpreter->output(EI_CLASSIFIER_TFLITE_OUTPUT_SCORE_TENSOR);
    *output_labels = interpreter->output(EI_CLASSIFIER_TFLITE_OUTPUT_LABELS_TENSOR);
#endif // EI_CLASSIFIER_OBJECT_DETECTION
#endif

    // Assert that our quantization parameters match the model
    if (tflite_first_run) {
        assert((*input)->type == EI_CLASSIFIER_TFLITE_INPUT_DATATYPE);
        assert((*output)->type == EI_CLASSIFIER_TFLITE_OUTPUT_DATATYPE);
#if EI_CLASSIFIER_OBJECT_DETECTION
        assert((*output_scores)->type == EI_CLASSIFIER_TFLITE_OUTPUT_DATATYPE);
        assert((*output_labels)->type == EI_CLASSIFIER_TFLITE_OUTPUT_DATATYPE);
#endif
#if defined(EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED) || defined(EI_CLASSIFIER_TFLITE_OUTPUT_QUANTIZED)
        if (EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED) {
            assert((*input)->params.scale == EI_CLASSIFIER_TFLITE_INPUT_SCALE);
            assert((*input)->params.zero_point == EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
        }
        if (EI_CLASSIFIER_TFLITE_OUTPUT_QUANTIZED) {
            assert((*output)->params.scale == EI_CLASSIFIER_TFLITE_OUTPUT_SCALE);
            assert((*output)->params.zero_point == EI_CLASSIFIER_TFLITE_OUTPUT_ZEROPOINT);
        }
#endif
        tflite_first_run = false;
    }
    return EI_IMPULSE_OK;
}

/**
 * Run TFLite model
 *
 * @param   ctx_start_ms    Start time of the setup function (see above)
 * @param   output          Output tensor
 * @param   interpreter     TFLite interpreter (non-compiled models)
 * @param   tensor_arena    Allocated arena (will be freed)
 * @param   result          Struct for results
 * @param   debug           Whether to print debug info
 *
 * @return  EI_IMPULSE_OK if successful
 */
static EI_IMPULSE_ERROR inference_tflite_run(uint64_t ctx_start_ms,
    TfLiteTensor* output,
#if EI_CLASSIFIER_OBJECT_DETECTION
    TfLiteTensor* labels_tensor,
    TfLiteTensor* scores_tensor,
#endif
#if (EI_CLASSIFIER_COMPILED != 1)
    tflite::MicroInterpreter* interpreter,
#endif
    uint8_t* tensor_arena,
    ei_impulse_result_t *result,
    bool debug) {
#if (EI_CLASSIFIER_COMPILED == 1)
    trained_model_invoke();
#else
    // Run inference, and report any error
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        error_reporter->Report("Invoke failed (%d)\n", invoke_status);
        ei_aligned_free(tensor_arena);
        return EI_IMPULSE_TFLITE_ERROR;
    }
    delete interpreter;
#endif

    uint64_t ctx_end_ms = ei_read_timer_ms();

    result->timing.classification = ctx_end_ms - ctx_start_ms;

    // Read the predicted y value from the model's output tensor
    if (debug) {
        ei_printf("Predictions (time: %d ms.):\n", result->timing.classification);
    }
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    fill_result_struct_f32(result, tflite::post_process_boxes, tflite::post_process_scores, tflite::post_process_classes, debug);
    // fill_result_struct_f32(result, output->data.f, scores_tensor->data.f, labels_tensor->data.f, debug);
#else
    bool int8_output = output->type == TfLiteType::kTfLiteInt8;
    if (int8_output) {
        fill_result_struct_i8(result, output->data.int8, output->params.zero_point, output->params.scale, debug);
    }
    else {
        fill_result_struct_f32(result, output->data.f, debug);
    }
#endif

#if (EI_CLASSIFIER_COMPILED == 1)
    trained_model_reset(ei_aligned_free);
#else
    ei_aligned_free(tensor_arena);
#endif

    if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
        return EI_IMPULSE_CANCELED;
    }

    return EI_IMPULSE_OK;
}
#endif // (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE)

/**
 * @brief      Do inferencing over the processed feature matrix
 *
 * @param      fmatrix  Processed matrix
 * @param      result   Output classifier results
 * @param[in]  debug    Debug output enable
 *
 * @return     The ei impulse error.
 */
extern "C" EI_IMPULSE_ERROR run_inference(
    ei::matrix_t *fmatrix,
    ei_impulse_result_t *result,
    bool debug = false)
{
#if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE)
    {
        uint64_t ctx_start_ms;
        TfLiteTensor* input;
        TfLiteTensor* output;
#if EI_CLASSIFIER_OBJECT_DETECTION
        TfLiteTensor* output_scores;
        TfLiteTensor* output_labels;
#endif
        uint8_t* tensor_arena;

#if (EI_CLASSIFIER_COMPILED == 1)
        EI_IMPULSE_ERROR init_res = inference_tflite_setup(&ctx_start_ms, &input, &output,
    #if EI_CLASSIFIER_OBJECT_DETECTION
            &output_labels,
            &output_scores,
    #endif
            &tensor_arena);
#else
        tflite::MicroInterpreter* interpreter;
        EI_IMPULSE_ERROR init_res = inference_tflite_setup(&ctx_start_ms, &input, &output,
    #if EI_CLASSIFIER_OBJECT_DETECTION
            &output_labels,
            &output_scores,
    #endif
            &interpreter, &tensor_arena);
#endif
        if (init_res != EI_IMPULSE_OK) {
            return init_res;
        }

        // Place our calculated x value in the model's input tensor
#if EI_CLASSIFIER_OBJECT_DETECTION
        bool uint8_input = input->type == TfLiteType::kTfLiteUInt8;
        for (size_t ix = 0; ix < fmatrix->rows * fmatrix->cols; ix++) {
            if (uint8_input) {
                float pixel = (float)fmatrix->buffer[ix];
                input->data.uint8[ix] = static_cast<uint8_t>((pixel / EI_CLASSIFIER_TFLITE_INPUT_SCALE) + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
            }
            else {
                input->data.f[ix] = fmatrix->buffer[ix];
            }
        }
#else
        bool int8_input = input->type == TfLiteType::kTfLiteInt8;
        for (size_t ix = 0; ix < fmatrix->rows * fmatrix->cols; ix++) {
            // Quantize the input if it is int8
            if (int8_input) {
                input->data.int8[ix] = static_cast<int8_t>(round(fmatrix->buffer[ix] / input->params.scale) + input->params.zero_point);
                // printf("float %ld : %d\r\n", ix, input->data.int8[ix]);
            } else {
                input->data.f[ix] = fmatrix->buffer[ix];
            }
        }
#endif

#if (EI_CLASSIFIER_COMPILED == 1)
        EI_IMPULSE_ERROR run_res = inference_tflite_run(ctx_start_ms, output,
    #if EI_CLASSIFIER_OBJECT_DETECTION
            output_labels,
            output_scores,
    #endif
            tensor_arena, result, debug);
#else
        EI_IMPULSE_ERROR run_res = inference_tflite_run(ctx_start_ms, output,
    #if EI_CLASSIFIER_OBJECT_DETECTION
            output_labels,
            output_scores,
    #endif
            interpreter, tensor_arena, result, debug);
#endif

        if (run_res != EI_IMPULSE_OK) {
            return run_res;
        }
    }

#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE_FULL

    {
        static std::unique_ptr<tflite::FlatBufferModel> model = nullptr;
        static std::unique_ptr<tflite::Interpreter> interpreter = nullptr;
        if (!model) {
            model = tflite::FlatBufferModel::BuildFromBuffer((const char*)trained_tflite, trained_tflite_len);
            if (!model) {
                ei_printf("Failed to build TFLite model from buffer\n");
                return EI_IMPULSE_TFLITE_ERROR;
            }

            tflite::ops::builtin::BuiltinOpResolver resolver;
            tflite::InterpreterBuilder builder(*model, resolver);
            builder(&interpreter);

            if (!interpreter) {
                ei_printf("Failed to construct interpreter\n");
                return EI_IMPULSE_TFLITE_ERROR;
            }

            if (interpreter->AllocateTensors() != kTfLiteOk) {
                ei_printf("AllocateTensors failed\n");
                return EI_IMPULSE_TFLITE_ERROR;
            }
        }

        // Obtain pointers to the model's input and output tensors.
    #if EI_CLASSIFIER_OBJECT_DETECTION == 1
        #if EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1
            int8_t* input = interpreter->typed_input_tensor<int8_t>(0);
        #else
            float* input = interpreter->typed_input_tensor<float>(0);
        #endif
    #elif EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1
        int8_t* input = interpreter->typed_input_tensor<int8_t>(0);
    #else
        float* input = interpreter->typed_input_tensor<float>(0);
    #endif

        if (!input) {
            return EI_IMPULSE_INPUT_TENSOR_WAS_NULL;
        }

        for (uint32_t ix = 0; ix < fmatrix->rows * fmatrix->cols; ix++) {
    #if EI_CLASSIFIER_OBJECT_DETECTION == 1
        #if EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1
            float pixel = (float)fmatrix->buffer[ix];
            input[ix] = static_cast<uint8_t>((pixel / EI_CLASSIFIER_TFLITE_INPUT_SCALE) + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
        #else
            input[ix] = fmatrix->buffer[ix];
        #endif
    #elif EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1
            input[ix] = static_cast<int8_t>(round(fmatrix->buffer[ix] / EI_CLASSIFIER_TFLITE_INPUT_SCALE) + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
    #else
            input[ix] = fmatrix->buffer[ix];
    #endif
        }

        uint64_t ctx_start_ms = ei_read_timer_ms();

        interpreter->Invoke();

        uint64_t ctx_end_ms = ei_read_timer_ms();

        result->timing.classification = ctx_end_ms - ctx_start_ms;
    #if EI_CLASSIFIER_TFLITE_OUTPUT_QUANTIZED == 1
        int8_t* out_data = interpreter->typed_output_tensor<int8_t>(0);
    #else
        float* out_data = interpreter->typed_output_tensor<float>(0);
    #endif

        if (!out_data) {
            return EI_IMPULSE_OUTPUT_TENSOR_WAS_NULL;
        }

        if (debug) {
            ei_printf("Predictions (time: %d ms.):\n", result->timing.classification);
        }

#if EI_CLASSIFIER_OBJECT_DETECTION == 1
        float *scores_tensor = interpreter->typed_output_tensor<float>(EI_CLASSIFIER_TFLITE_OUTPUT_SCORE_TENSOR);
        float *label_tensor = interpreter->typed_output_tensor<float>(EI_CLASSIFIER_TFLITE_OUTPUT_LABELS_TENSOR);
        if (!scores_tensor) {
            return EI_IMPULSE_SCORE_TENSOR_WAS_NULL;
        }
        if (!label_tensor) {
            return EI_IMPULSE_LABEL_TENSOR_WAS_NULL;
        }
        fill_result_struct_f32(result, out_data, scores_tensor, label_tensor, debug);
#else

    #if EI_CLASSIFIER_TFLITE_OUTPUT_QUANTIZED == 1
        fill_result_struct_i8(result, out_data, EI_CLASSIFIER_TFLITE_OUTPUT_ZEROPOINT, EI_CLASSIFIER_TFLITE_OUTPUT_SCALE, debug);
    #else
        fill_result_struct_f32(result, out_data, debug);
    #endif

#endif

        // on Linux we're not worried about free'ing (for now)
    }

#elif (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSAIFLOW)
    {
        uint64_t ctx_start_ms = ei_read_timer_ms();
        int8_t *input;
        int8_t output[EI_CLASSIFIER_LABEL_COUNT];

        input = (int8_t *)ei_malloc(fmatrix->rows * fmatrix->cols);

        if (!input) {
            return EI_IMPULSE_ALLOC_FAILED;
        }

        for (size_t ix = 0; ix < fmatrix->rows * fmatrix->cols; ix++) {
            input[ix] = static_cast<int8_t>(
                round(fmatrix->buffer[ix] / EI_CLASSIFIER_TFLITE_INPUT_SCALE) +
                EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
        }

        /* Run tensaiflow inference */
        infer(input, output);

        for (uint32_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            float value;
            // Dequantize the output if it is int8
            value = static_cast<float>(output[ix] - EI_CLASSIFIER_TFLITE_OUTPUT_ZEROPOINT) *
                EI_CLASSIFIER_TFLITE_OUTPUT_SCALE;

            if (debug) {
                ei_printf("%s:\t", ei_classifier_inferencing_categories[ix]);
                ei_printf_float(value);
                ei_printf("\n");
            }
            result->classification[ix].label = ei_classifier_inferencing_categories[ix];
            result->classification[ix].value = value;
        }

        result->timing.classification = ei_read_timer_ms() - ctx_start_ms;

        ei_free(input);
    }

#elif (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSORRT)
    {
        #if EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1
        #error "TensorRT requires an unquantized network"
        #endif

        static bool first_run = true;
        static char model_file_name[128];

        if (first_run) {
            snprintf(model_file_name, 128, "/tmp/ei-%s", trained_onnx_hash);

            FILE *file = fopen(model_file_name, "w");
            if (!file) {
                ei_printf("ERR: TensorRT init failed to open '%s'\n", model_file_name);
                return EI_IMPULSE_TENSORRT_INIT_FAILED;
            }

            if (fwrite(trained_onnx, trained_onnx_len, 1, file) != 1) {
                ei_printf("ERR: TensorRT init fwrite failed\n");
                return EI_IMPULSE_TENSORRT_INIT_FAILED;
            }

            if (fclose(file) != 0) {
                ei_printf("ERR: TensorRT init fclose failed\n");
                return EI_IMPULSE_TENSORRT_INIT_FAILED;
            }
        }

        float tensorrt_output[EI_CLASSIFIER_LABEL_COUNT];

        // lazy initialize tensorRT context
        if( ei_trt_handle == nullptr ) {
            ei_trt_handle = libeitrt::create_EiTrt(model_file_name, debug);
        }

        uint64_t ctx_start_ms = ei_read_timer_ms();

        libeitrt::infer(ei_trt_handle, fmatrix->buffer, tensorrt_output, EI_CLASSIFIER_LABEL_COUNT);
        uint64_t ctx_end_ms = ei_read_timer_ms();
        result->timing.classification = ctx_end_ms - ctx_start_ms;

        for( int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; ++i) {
            result->classification[i].label = ei_classifier_inferencing_categories[i];
            result->classification[i].value = tensorrt_output[i];
        }
    }
#endif

#if EI_CLASSIFIER_HAS_ANOMALY == 1

    // Anomaly detection
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
    }

#endif

    if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
        return EI_IMPULSE_CANCELED;
    }

    return EI_IMPULSE_OK;
}

extern "C" EI_IMPULSE_ERROR run_inference_i16(
    ei::matrix_i32_t *fmatrix,
    ei_impulse_result_t *result,
    bool debug = false)
{
#if EI_CLASSIFIER_OBJECT_DETECTION
    return EI_IMPULSE_NOT_SUPPORTED_WITH_I16;
#else

#if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE)
    {
        uint64_t ctx_start_ms;
        TfLiteTensor* input;
        TfLiteTensor* output;
        uint8_t* tensor_arena;

#if (EI_CLASSIFIER_COMPILED == 1)
        EI_IMPULSE_ERROR init_res = inference_tflite_setup(&ctx_start_ms, &input, &output,
            &tensor_arena);
#else
        tflite::MicroInterpreter* interpreter;
        EI_IMPULSE_ERROR init_res = inference_tflite_setup(&ctx_start_ms, &input, &output,
            &interpreter,
            &tensor_arena);
#endif
        if (init_res != EI_IMPULSE_OK) {
            return init_res;
        }

        EIDSP_i16 scale;
        numpy::float_to_int16(&input->params.scale, &scale, 1);

        // Place our calculated x value in the model's input tensor
        bool int8_input = input->type == TfLiteType::kTfLiteInt8;
        for (size_t ix = 0; ix < fmatrix->rows * fmatrix->cols; ix++) {
            // Quantize the input if it is int8
            if (int8_input) {
                int32_t calc = (int32_t)fmatrix->buffer[ix] << 8; // Shift for scaler
                calc /= scale;
                calc += 0x80; // Round by adding 0.5
                calc >>= 8; // Shift to int8_t domain
                input->data.int8[ix] = static_cast<int8_t>(calc + input->params.zero_point);
            } else {
                numpy::int16_to_float((EIDSP_i16 *)&fmatrix->buffer[ix], &input->data.f[ix], 1);
            }
        }

#if (EI_CLASSIFIER_COMPILED == 1)
        EI_IMPULSE_ERROR run_res = inference_tflite_run(ctx_start_ms, output,
            tensor_arena, result, debug);
#else
        EI_IMPULSE_ERROR run_res = inference_tflite_run(ctx_start_ms, output,
            interpreter, tensor_arena, result, debug);
#endif

        if (run_res != EI_IMPULSE_OK) {
            return run_res;
        }
    }

#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE_FULL

    ei_printf("ERR: run_classifier_i16 is not supported with full TensorFlow Lite\n");
    return EI_IMPULSE_TFLITE_ERROR;

#endif

#if EI_CLASSIFIER_HAS_ANOMALY == 1

    // Anomaly detection
    {
        uint64_t anomaly_start_ms = ei_read_timer_ms();

        float input[EI_CLASSIFIER_ANOM_AXIS_SIZE];
        for (size_t ix = 0; ix < EI_CLASSIFIER_ANOM_AXIS_SIZE; ix++) {
            // input[ix] = fmatrix->buffer[EI_CLASSIFIER_ANOM_AXIS[ix]];
            // numpy::int16_to_float(&fmatrix->buffer[EI_CLASSIFIER_ANOM_AXIS[ix]], &input[ix], 1);
            input[ix] = (float)fmatrix->buffer[EI_CLASSIFIER_ANOM_AXIS[ix]] / 32768.f;
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
    }

#endif

    if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
        return EI_IMPULSE_CANCELED;
    }

    return EI_IMPULSE_OK;
#endif // OBJECT_DETECTION
}

/**
 * Run the classifier over a raw features array
 * @param raw_features Raw features array
 * @param raw_features_size Size of the features array
 * @param result Object to store the results in
 * @param debug Whether to show debug messages (default: false)
 */
extern "C" EI_IMPULSE_ERROR run_classifier(
    signal_t *signal,
    ei_impulse_result_t *result,
    bool debug = false)
{
#if EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1 && EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE
    // Shortcut for quantized image models
    if (can_run_classifier_image_quantized() == EI_IMPULSE_OK) {
        return run_classifier_image_quantized(signal, result, debug);
    }
#endif

    // if (debug) {
    // static float buf[1000];
    // printf("Raw data: ");
    // for (size_t ix = 0; ix < 16000; ix += 1000) {
    //     int r = signal->get_data(ix, 1000, buf);
    //     for (size_t jx = 0; jx < 1000; jx++) {
    //         printf("%.0f, ", buf[jx]);
    //     }
    // }
    // printf("\n");
    // }

    memset(result, 0, sizeof(ei_impulse_result_t));

    ei::matrix_t features_matrix(1, EI_CLASSIFIER_NN_INPUT_FRAME_SIZE);

    uint64_t dsp_start_ms = ei_read_timer_ms();

    size_t out_features_index = 0;

    for (size_t ix = 0; ix < ei_dsp_blocks_size; ix++) {
        ei_model_dsp_t block = ei_dsp_blocks[ix];

        if (out_features_index + block.n_output_features > EI_CLASSIFIER_NN_INPUT_FRAME_SIZE) {
            ei_printf("ERR: Would write outside feature buffer\n");
            return EI_IMPULSE_DSP_ERROR;
        }

        ei::matrix_t fm(1, block.n_output_features, features_matrix.buffer + out_features_index);

        int ret = block.extract_fn(signal, &fm, block.config, EI_CLASSIFIER_FREQUENCY);
        if (ret != EIDSP_OK) {
            ei_printf("ERR: Failed to run DSP process (%d)\n", ret);
            return EI_IMPULSE_DSP_ERROR;
        }

        if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
            return EI_IMPULSE_CANCELED;
        }

        out_features_index += block.n_output_features;
    }

    result->timing.dsp = ei_read_timer_ms() - dsp_start_ms;

    if (debug) {
        ei_printf("Features (%d ms.): ", result->timing.dsp);
        for (size_t ix = 0; ix < features_matrix.cols; ix++) {
            ei_printf_float(features_matrix.buffer[ix]);
            ei_printf(" ");
        }
        ei_printf("\n");
    }

#if EI_CLASSIFIER_INFERENCING_ENGINE != EI_CLASSIFIER_NONE
    if (debug) {
        ei_printf("Running neural network...\n");
    }
#endif

    return run_inference(&features_matrix, result, debug);
}

#if defined(EI_CLASSIFIER_USE_QUANTIZED_DSP_BLOCK) && EI_CLASSIFIER_USE_QUANTIZED_DSP_BLOCK == 1

extern "C" EI_IMPULSE_ERROR run_classifier_i16(
    signal_i16_t *signal,
    ei_impulse_result_t *result,
    bool debug = false)
{

    memset(result, 0, sizeof(ei_impulse_result_t));

    ei::matrix_i32_t features_matrix(1, EI_CLASSIFIER_NN_INPUT_FRAME_SIZE);

    uint64_t dsp_start_ms = ei_read_timer_ms();

    size_t out_features_index = 0;

    for (size_t ix = 0; ix < ei_dsp_blocks_size; ix++) {
        ei_model_dsp_i16_t block = ei_dsp_blocks_i16[ix];

        if (out_features_index + block.n_output_features > EI_CLASSIFIER_NN_INPUT_FRAME_SIZE) {
            ei_printf("ERR: Would write outside feature buffer\n");
            return EI_IMPULSE_DSP_ERROR;
        }

        ei::matrix_i32_t fm(1, block.n_output_features, features_matrix.buffer + out_features_index);

        int ret = block.extract_fn(signal, &fm, ei_dsp_blocks[ix].config, EI_CLASSIFIER_FREQUENCY);

        if (ret != EIDSP_OK) {
            ei_printf("ERR: Failed to run DSP process (%d)\n", ret);
            return EI_IMPULSE_DSP_ERROR;
        }

        if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
            return EI_IMPULSE_CANCELED;
        }

        out_features_index += block.n_output_features;
    }

    result->timing.dsp = ei_read_timer_ms() - dsp_start_ms;

    if (debug) {
        ei_printf("Features (%d ms.): ", result->timing.dsp);
        for (size_t ix = 0; ix < features_matrix.cols; ix++) {
            ei_printf_float((float)features_matrix.buffer[ix] / 32768.f);
            ei_printf(" ");
        }
        ei_printf("\n");
    }

#if EI_CLASSIFIER_INFERENCING_ENGINE != EI_CLASSIFIER_NONE
    if (debug) {
        ei_printf("Running neural network...\n");
    }
#endif

    return run_inference_i16(&features_matrix, result, debug);
}
#endif //EI_CLASSIFIER_USE_QUANTIZED_DSP_BLOCK

/**
 * @brief      Calculates the cepstral mean and variable normalization.
 *
 * @param      matrix      Source and destination matrix
 * @param      config_ptr  ei_dsp_config_mfcc_t struct pointer
 */
static void calc_cepstral_mean_and_var_normalization_mfcc(ei_matrix *matrix, void *config_ptr)
{
    ei_dsp_config_mfcc_t *config = (ei_dsp_config_mfcc_t *)config_ptr;

    /* Modify rows and colums ration for matrix normalization */
    matrix->rows = EI_CLASSIFIER_NN_INPUT_FRAME_SIZE / config->num_cepstral;
    matrix->cols = config->num_cepstral;

    // cepstral mean and variance normalization
    int ret = speechpy::processing::cmvnw(matrix, config->win_size, true, false);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: cmvnw failed (%d)\n", ret);
        return;
    }

    /* Reset rows and columns ratio */
    matrix->rows = 1;
    matrix->cols = EI_CLASSIFIER_NN_INPUT_FRAME_SIZE;
}

/**
 * @brief      Calculates the cepstral mean and variable normalization.
 *
 * @param      matrix      Source and destination matrix
 * @param      config_ptr  ei_dsp_config_mfe_t struct pointer
 */
static void calc_cepstral_mean_and_var_normalization_mfe(ei_matrix *matrix, void *config_ptr)
{
    ei_dsp_config_mfe_t *config = (ei_dsp_config_mfe_t *)config_ptr;

    /* Modify rows and colums ration for matrix normalization */
    matrix->rows = EI_CLASSIFIER_NN_INPUT_FRAME_SIZE / config->num_filters;
    matrix->cols = config->num_filters;

    // cepstral mean and variance normalization
    int ret = speechpy::processing::cmvnw(matrix, config->win_size, false, true);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: cmvnw failed (%d)\n", ret);
        return;
    }

    /* Reset rows and columns ratio */
    matrix->rows = 1;
    matrix->cols = EI_CLASSIFIER_NN_INPUT_FRAME_SIZE;
}

/**
 * @brief      Calculates the cepstral mean and variable normalization.
 *
 * @param      matrix      Source and destination matrix
 * @param      config_ptr  ei_dsp_config_spectrogram_t struct pointer
 */
static void calc_cepstral_mean_and_var_normalization_spectrogram(ei_matrix *matrix, void *config_ptr)
{
    ei_dsp_config_spectrogram_t *config = (ei_dsp_config_spectrogram_t *)config_ptr;

    /* Modify rows and colums ration for matrix normalization */
    matrix->cols = config->fft_length / 2 + 1;
    matrix->rows = EI_CLASSIFIER_NN_INPUT_FRAME_SIZE / matrix->cols;

    int ret = numpy::normalize(matrix);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: normalization failed (%d)\n", ret);
        return;
    }

    /* Reset rows and columns ratio */
    matrix->rows = 1;
    matrix->cols = EI_CLASSIFIER_NN_INPUT_FRAME_SIZE;
}

/**
 * Check if the current impulse could be used by 'run_classifier_image_quantized'
 */
__attribute__((unused)) static EI_IMPULSE_ERROR can_run_classifier_image_quantized() {
#if (EI_CLASSIFIER_INFERENCING_ENGINE != EI_CLASSIFIER_TFLITE)
    return EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE;
#endif

#if EI_CLASSIFIER_HAS_ANOMALY == 1
    return EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES;
#endif

    // Check if we have a quantized NN
#if EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED != 1
    return EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES;
#endif

    // And if we have one DSP block which operates on images...
    if (ei_dsp_blocks_size != 1 || ei_dsp_blocks[0].extract_fn != extract_image_features) {
        return EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES;
    }

    return EI_IMPULSE_OK;
}

#if EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1 && EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE
/**
 * Special function to run the classifier on images, only works on TFLite models (either interpreter or EON)
 * that allocates a lot less memory by quantizing in place. This only works if 'can_run_classifier_image_quantized'
 * returns EI_IMPULSE_OK.
 */
extern "C" EI_IMPULSE_ERROR run_classifier_image_quantized(
    signal_t *signal,
    ei_impulse_result_t *result,
    bool debug = false)
{
    EI_IMPULSE_ERROR verify_res = can_run_classifier_image_quantized();
    if (verify_res != EI_IMPULSE_OK) {
        return verify_res;
    }

    memset(result, 0, sizeof(ei_impulse_result_t));

#if (EI_CLASSIFIER_INFERENCING_ENGINE != EI_CLASSIFIER_TFLITE)
    return EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE;
#else
    uint64_t ctx_start_ms;
    TfLiteTensor* input;
    TfLiteTensor* output;
#if EI_CLASSIFIER_OBJECT_DETECTION
    TfLiteTensor* output_scores;
    TfLiteTensor* output_labels;
#endif
    uint8_t* tensor_arena;

#if (EI_CLASSIFIER_COMPILED == 1)
    EI_IMPULSE_ERROR init_res = inference_tflite_setup(&ctx_start_ms, &input, &output,
    #if EI_CLASSIFIER_OBJECT_DETECTION
        &output_labels,
        &output_scores,
    #endif
        &tensor_arena);
#else
    tflite::MicroInterpreter* interpreter;
    EI_IMPULSE_ERROR init_res = inference_tflite_setup(&ctx_start_ms, &input, &output,
    #if EI_CLASSIFIER_OBJECT_DETECTION
        &output_labels,
        &output_scores,
    #endif
        &interpreter,
        &tensor_arena);
#endif
    if (init_res != EI_IMPULSE_OK) {
        return init_res;
    }

    if (input->type != TfLiteType::kTfLiteInt8) {
        return EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES;
    }

    uint64_t dsp_start_ms = ei_read_timer_ms();

    // features matrix maps around the input tensor to not allocate any memory
    ei::matrix_i8_t features_matrix(1, EI_CLASSIFIER_NN_INPUT_FRAME_SIZE, input->data.int8);

    // run DSP process and quantize automatically
    int ret = extract_image_features_quantized(signal, &features_matrix, ei_dsp_blocks[0].config, EI_CLASSIFIER_FREQUENCY);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: Failed to run DSP process (%d)\n", ret);
        return EI_IMPULSE_DSP_ERROR;
    }

    if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
        return EI_IMPULSE_CANCELED;
    }

    result->timing.dsp = ei_read_timer_ms() - dsp_start_ms;

    if (debug) {
        ei_printf("Features (%d ms.): ", result->timing.dsp);
        for (size_t ix = 0; ix < features_matrix.cols; ix++) {
            ei_printf_float((features_matrix.buffer[ix] - EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT) * EI_CLASSIFIER_TFLITE_INPUT_SCALE);
            ei_printf(" ");
        }
        ei_printf("\n");
    }

    ctx_start_ms = ei_read_timer_ms();

#if (EI_CLASSIFIER_COMPILED == 1)
    EI_IMPULSE_ERROR run_res = inference_tflite_run(ctx_start_ms, output,
    #if EI_CLASSIFIER_OBJECT_DETECTION
        output_labels,
        output_scores,
    #endif
        tensor_arena, result, debug);
#else
    EI_IMPULSE_ERROR run_res = inference_tflite_run(ctx_start_ms, output,
    #if EI_CLASSIFIER_OBJECT_DETECTION
        output_labels,
        output_scores,
    #endif
        interpreter, tensor_arena, result, debug);
#endif

    if (run_res != EI_IMPULSE_OK) {
        return run_res;
    }

    return EI_IMPULSE_OK;
#endif // EI_CLASSIFIER_INFERENCING_ENGINE != EI_CLASSIFIER_TFLITE
}
#endif // #if EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1 && EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE

#if EIDSP_SIGNAL_C_FN_POINTER == 0

/**
 * Run the impulse, if you provide an instance of sampler it will also persist the data for you
 * @param sampler Instance to an **initialized** sampler
 * @param result Object to store the results in
 * @param data_fn Function to retrieve data from sensors
 * @param debug Whether to log debug messages (default false)
 */
__attribute__((unused)) EI_IMPULSE_ERROR run_impulse(
#if defined(EI_CLASSIFIER_HAS_SAMPLER) && EI_CLASSIFIER_HAS_SAMPLER == 1
        EdgeSampler *sampler,
#endif
        ei_impulse_result_t *result,
#ifdef __MBED__
        mbed::Callback<void(float*, size_t)> data_fn,
#else
        std::function<void(float*, size_t)> data_fn,
#endif
        bool debug = false) {

    float *x = (float*)calloc(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(float));
    if (!x) {
        return EI_IMPULSE_OUT_OF_MEMORY;
    }

    uint64_t next_tick = 0;

    uint64_t sampling_us_start = ei_read_timer_us();

    // grab some data
    for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i += EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME) {
        uint64_t curr_us = ei_read_timer_us() - sampling_us_start;

        next_tick = curr_us + (EI_CLASSIFIER_INTERVAL_MS * 1000);

        data_fn(x + i, EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME);
#if defined(EI_CLASSIFIER_HAS_SAMPLER) && EI_CLASSIFIER_HAS_SAMPLER == 1
        if (sampler != NULL) {
            sampler->write_sensor_data(x + i, EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME);
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
    int err = numpy::signal_from_buffer(x, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
        free(x);
        ei_printf("ERR: signal_from_buffer failed (%d)\n", err);
        return EI_IMPULSE_DSP_ERROR;
    }

    EI_IMPULSE_ERROR r = run_classifier(&signal, result, debug);
    free(x);
    return r;
}

#if defined(EI_CLASSIFIER_USE_QUANTIZED_DSP_BLOCK) && EI_CLASSIFIER_USE_QUANTIZED_DSP_BLOCK == 1

__attribute__((unused)) EI_IMPULSE_ERROR run_impulse_i16(
#if defined(EI_CLASSIFIER_HAS_SAMPLER) && EI_CLASSIFIER_HAS_SAMPLER == 1
        EdgeSampler *sampler,
#endif
        ei_impulse_result_t *result,
#ifdef __MBED__
        mbed::Callback<void(signed short*, size_t)> data_fn,
#else
        std::function<void(signed short*, size_t)> data_fn,
#endif
        bool debug = false) {

    int16_t x[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

    uint64_t next_tick = 0;

    uint64_t sampling_us_start = ei_read_timer_us();

    // grab some data
    for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i += EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME) {
        uint64_t curr_us = ei_read_timer_us() - sampling_us_start;

        next_tick = curr_us + (EI_CLASSIFIER_INTERVAL_MS * 1000);

        data_fn(x + i, EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME);
#if defined(EI_CLASSIFIER_HAS_SAMPLER) && EI_CLASSIFIER_HAS_SAMPLER == 1
        if (sampler != NULL) {
            sampler->write_sensor_data(x + i, EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME);
        }
#endif

        if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
            return EI_IMPULSE_CANCELED;
        }

        while (next_tick > ei_read_timer_us() - sampling_us_start);
    }

    result->timing.sampling = (ei_read_timer_us() - sampling_us_start) / 1000;

    signal_i16_t signal;
    int err = numpy::signal_from_buffer_i16(x, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
        ei_printf("ERR: signal_from_buffer failed (%d)\n", err);
        return EI_IMPULSE_DSP_ERROR;
    }

    return run_classifier_i16(&signal, result, debug);
}
#endif //EI_CLASSIFIER_USE_QUANTIZED_DSP_BLOCK

#if defined(EI_CLASSIFIER_HAS_SAMPLER) && EI_CLASSIFIER_HAS_SAMPLER == 1
/**
 * Run the impulse, does not persist data
 * @param result Object to store the results in
 * @param data_fn Function to retrieve data from sensors
 * @param debug Whether to log debug messages (default false)
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
