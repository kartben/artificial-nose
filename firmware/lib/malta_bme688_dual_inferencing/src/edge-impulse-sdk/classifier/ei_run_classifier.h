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

#ifndef _EDGE_IMPULSE_RUN_CLASSIFIER_H_
#define _EDGE_IMPULSE_RUN_CLASSIFIER_H_

#include "model-parameters/model_metadata.h"

#include "ei_run_dsp.h"
#include "ei_classifier_types.h"
#include "ei_signal_with_axes.h"
#include "ei_performance_calibration.h"

#include "edge-impulse-sdk/porting/ei_classifier_porting.h"

// for the release we'll put an actual studio version here
#ifndef EI_CLASSIFIER_STUDIO_VERSION
#define EI_CLASSIFIER_STUDIO_VERSION 2
#endif

#if EI_CLASSIFIER_STUDIO_VERSION < 3
#include "model-parameters/dsp_blocks.h"
#endif

#if EI_CLASSIFIER_HAS_MODEL_VARIABLES == 1
#include "model-parameters/model_variables.h"
#endif

#if EI_CLASSIFIER_HAS_ANOMALY == 1
#include "model-parameters/anomaly_clusters.h"
#include "inferencing_engines/anomaly.h"
#endif

#if defined(EI_CLASSIFIER_HAS_SAMPLER) && EI_CLASSIFIER_HAS_SAMPLER == 1
#include "ei_sampler.h"
#endif

#if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE) && (EI_CLASSIFIER_COMPILED != 1)
#include "edge-impulse-sdk/classifier/inferencing_engines/tflite_micro.h"
#include "tflite-model/tflite-trained.h"
#elif EI_CLASSIFIER_COMPILED == 1
#include "edge-impulse-sdk/classifier/inferencing_engines/tflite_eon.h"
#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE_FULL
#include "edge-impulse-sdk/classifier/inferencing_engines/tflite_full.h"
#include "tflite-model/tflite-trained.h"
#elif (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSORRT)
#include "edge-impulse-sdk/classifier/inferencing_engines/tensorrt.h"
#include "tflite-model/onnx-trained.h"
#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSAIFLOW
#include "edge-impulse-sdk/classifier/inferencing_engines/tensaiflow.h"
#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_DRPAI
#include "edge-impulse-sdk/classifier/inferencing_engines/drpai.h"
#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_NONE
// noop
#else
#error "Unknown inferencing engine"
#endif

#if ECM3532
void*   __dso_handle = (void*) &__dso_handle;
#endif

// EI_CLASSIFIER_CALIBRATION_ENABLED needs to be added to new
// model metadata, since we are getting rid of macro for sensors
// (multiple impulses means we can have multiple sensors)
// for now we just enable it if EI_CLASSIFIER_SENSOR is present and
// is microphone (performance calibration only works for mic).
#if defined(EI_CLASSIFIER_SENSOR) && (EI_CLASSIFIER_SENSOR == EI_CLASSIFIER_SENSOR_MICROPHONE)
#define EI_CLASSIFIER_CALIBRATION_ENABLED 1
#else
#define EI_CLASSIFIER_CALIBRATION_ENABLED 0
#endif

#ifdef __cplusplus
namespace {
#endif // __cplusplus

/* Function prototypes ----------------------------------------------------- */
extern "C" EI_IMPULSE_ERROR run_inference(const ei_impulse_t *impulse, ei::matrix_t *fmatrix, ei_impulse_result_t *result, bool debug);
extern "C" EI_IMPULSE_ERROR run_classifier_image_quantized(const ei_impulse_t *impulse, signal_t *signal, ei_impulse_result_t *result, bool debug);
static EI_IMPULSE_ERROR can_run_classifier_image_quantized(const ei_impulse_t *impulse);

/* Private variables ------------------------------------------------------- */

static uint64_t classifier_continuous_features_written = 0;
static RecognizeEvents *avg_scores = NULL;

/* Private functions ------------------------------------------------------- */

/* These functions (up to Public functions section) are not exposed to end-user,
therefore changes are allowed. */

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
    const ei_impulse_t *impulse,
    ei::matrix_t *fmatrix,
    ei_impulse_result_t *result,
    bool debug = false)
{
#if (EI_CLASSIFIER_INFERENCING_ENGINE != EI_CLASSIFIER_NONE && EI_CLASSIFIER_INFERENCING_ENGINE != EI_CLASSIFIER_DRPAI)
    EI_IMPULSE_ERROR nn_res = run_nn_inference(impulse, fmatrix, result, debug);
    if (nn_res != EI_IMPULSE_OK) {
        return nn_res;
    }
#endif

#if EI_CLASSIFIER_HAS_ANOMALY == 1
    if (impulse->has_anomaly) {
        EI_IMPULSE_ERROR anomaly_res = inference_anomaly_invoke(impulse, fmatrix, result, debug);
        if (anomaly_res != EI_IMPULSE_OK) {
            return anomaly_res;
        }
    }
#endif

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
 * @param[in]  debug    Debug output enable
 *
 * @return     The ei impulse error.
 */
extern "C" EI_IMPULSE_ERROR process_impulse(const ei_impulse_t *impulse,
                                            signal_t *signal,
                                            ei_impulse_result_t *result,
                                            bool debug = false)
{

#if (EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1 && (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE || EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSAIFLOW)) || EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_DRPAI
    // Shortcut for quantized image models
    if (can_run_classifier_image_quantized(impulse) == EI_IMPULSE_OK) {
        return run_classifier_image_quantized(impulse, signal, result, debug);
    }
#endif

    memset(result, 0, sizeof(ei_impulse_result_t));

    ei::matrix_t features_matrix(1, impulse->nn_input_frame_size);

    uint64_t dsp_start_us = ei_read_timer_us();

    size_t out_features_index = 0;

    for (size_t ix = 0; ix < impulse->dsp_blocks_size; ix++) {
        ei_model_dsp_t block = impulse->dsp_blocks[ix];

        if (out_features_index + block.n_output_features > impulse->nn_input_frame_size) {
            ei_printf("ERR: Would write outside feature buffer\n");
            return EI_IMPULSE_DSP_ERROR;
        }

        ei::matrix_t fm(1, block.n_output_features, features_matrix.buffer + out_features_index);

#if EIDSP_SIGNAL_C_FN_POINTER
        if (block.axes_size != impulse->raw_samples_per_frame) {
            ei_printf("ERR: EIDSP_SIGNAL_C_FN_POINTER can only be used when all axes are selected for DSP blocks\n");
            return EI_IMPULSE_DSP_ERROR;
        }
        int ret = block.extract_fn(signal, &fm, block.config, impulse->frequency);
#else
        SignalWithAxes swa(signal, block.axes, block.axes_size, impulse);
        int ret = block.extract_fn(swa.get_signal(), &fm, block.config, impulse->frequency);
#endif

        if (ret != EIDSP_OK) {
            ei_printf("ERR: Failed to run DSP process (%d)\n", ret);
            return EI_IMPULSE_DSP_ERROR;
        }

        if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
            return EI_IMPULSE_CANCELED;
        }

        out_features_index += block.n_output_features;
    }

    result->timing.dsp_us = ei_read_timer_us() - dsp_start_us;
    result->timing.dsp = (int)(result->timing.dsp_us / 1000);

    if (debug) {
        ei_printf("Features (%d ms.): ", result->timing.dsp);
        for (size_t ix = 0; ix < features_matrix.cols; ix++) {
            ei_printf_float(features_matrix.buffer[ix]);
            ei_printf(" ");
        }
        ei_printf("\n");
    }

    if (debug) {
        ei_printf("Running impulse...\n");
    }

    return run_inference(impulse, &features_matrix, result, debug);

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
extern "C" EI_IMPULSE_ERROR process_impulse_continuous(const ei_impulse_t *impulse,
                                            signal_t *signal,
                                            ei_impulse_result_t *result,
                                            bool debug,
                                            bool enable_maf)
{

    static ei::matrix_t static_features_matrix(1, impulse->nn_input_frame_size);
    if (!static_features_matrix.buffer) {
        return EI_IMPULSE_ALLOC_FAILED;
    }

    memset(result, 0, sizeof(ei_impulse_result_t));

    EI_IMPULSE_ERROR ei_impulse_error = EI_IMPULSE_OK;

    uint64_t dsp_start_us = ei_read_timer_us();

    size_t out_features_index = 0;
    bool is_mfcc = false;
    bool is_mfe = false;
    bool is_spectrogram = false;

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
            is_mfcc = true;
        }
        else if (block.extract_fn == extract_spectrogram_features) {
            extract_fn_slice = &extract_spectrogram_per_slice_features;
            is_spectrogram = true;
        }
        else if (block.extract_fn == extract_mfe_features) {
            extract_fn_slice = &extract_mfe_per_slice_features;
            is_mfe = true;
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

    if (debug) {
        ei_printf("\r\nFeatures (%d ms.): ", result->timing.dsp);
        for (size_t ix = 0; ix < static_features_matrix.cols; ix++) {
            ei_printf_float(static_features_matrix.buffer[ix]);
            ei_printf(" ");
        }
        ei_printf("\n");
    }

    if (classifier_continuous_features_written >= impulse->nn_input_frame_size) {
        dsp_start_us = ei_read_timer_us();
        ei::matrix_t classify_matrix(1, impulse->nn_input_frame_size);

        /* Create a copy of the matrix for normalization */
        for (size_t m_ix = 0; m_ix < impulse->nn_input_frame_size; m_ix++) {
            classify_matrix.buffer[m_ix] = static_features_matrix.buffer[m_ix];
        }

        if (is_mfcc) {
            calc_cepstral_mean_and_var_normalization_mfcc(&classify_matrix, impulse->dsp_blocks[0].config);
        }
        else if (is_spectrogram) {
            calc_cepstral_mean_and_var_normalization_spectrogram(&classify_matrix, impulse->dsp_blocks[0].config);
        }
        else if (is_mfe) {
            calc_cepstral_mean_and_var_normalization_mfe(&classify_matrix, impulse->dsp_blocks[0].config);
        }
        result->timing.dsp_us += ei_read_timer_us() - dsp_start_us;
        result->timing.dsp = (int)(result->timing.dsp_us / 1000);

        if (debug) {
            ei_printf("Running impulse...\n");
        }

        ei_impulse_error = run_inference(impulse, &classify_matrix, result, debug);

#if EI_CLASSIFIER_CALIBRATION_ENABLED
        if (impulse->sensor == EI_CLASSIFIER_SENSOR_MICROPHONE) {
            if((void *)avg_scores != NULL && enable_maf == true) {
                if (enable_maf && !ei_calibration.is_configured) {
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
    }
    else {
        if (!impulse->object_detection) {
            for (int i = 0; i < impulse->label_count; i++) {
                // set label correctly in the result struct if we have no results (otherwise is nullptr)
                result->classification[i].label = impulse->categories[(uint32_t)i];
            }
        }
    }

    return ei_impulse_error;


}

#if EI_CLASSIFIER_STUDIO_VERSION < 3
/**
 * @brief      Construct impulse from macros - for run_classifer compatibility
 */
extern "C" const ei_impulse_t ei_construct_impulse()
{

const ei_impulse_t impulse =
    {
    .project_id = EI_CLASSIFIER_PROJECT_ID,
    .project_owner = EI_CLASSIFIER_PROJECT_OWNER,
    .project_name = EI_CLASSIFIER_PROJECT_NAME,

    .deploy_version = EI_CLASSIFIER_PROJECT_DEPLOY_VERSION,
    .nn_input_frame_size = EI_CLASSIFIER_NN_INPUT_FRAME_SIZE,
    .raw_sample_count = EI_CLASSIFIER_RAW_SAMPLE_COUNT,
    .raw_samples_per_frame = EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME,
    .dsp_input_frame_size = (EI_CLASSIFIER_RAW_SAMPLE_COUNT * EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME),

    .input_width = EI_CLASSIFIER_INPUT_WIDTH,
    .input_height = EI_CLASSIFIER_INPUT_HEIGHT,
    .input_frames = EI_CLASSIFIER_INPUT_FRAMES,

    .interval_ms = EI_CLASSIFIER_INTERVAL_MS,
    .label_count = EI_CLASSIFIER_LABEL_COUNT,
    .has_anomaly = EI_CLASSIFIER_HAS_ANOMALY,
    .frequency = EI_CLASSIFIER_FREQUENCY,
    .use_quantized_dsp_block = EI_CLASSIFIER_USE_QUANTIZED_DSP_BLOCK,
    .dsp_blocks_size = ei_dsp_blocks_size,
    .dsp_blocks = ei_dsp_blocks,

#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    .object_detection = true,
    .object_detection_count = EI_CLASSIFIER_OBJECT_DETECTION_COUNT,
    .object_detection_threshold = EI_CLASSIFIER_OBJECT_DETECTION_THRESHOLD,
    .object_detection_last_layer = EI_CLASSIFIER_OBJECT_DETECTION_LAST_LAYER,
    .tflite_output_labels_tensor = EI_CLASSIFIER_TFLITE_OUTPUT_LABELS_TENSOR,
    .tflite_output_score_tensor = EI_CLASSIFIER_TFLITE_OUTPUT_SCORE_TENSOR,
    .tflite_output_data_tensor = EI_CLASSIFIER_TFLITE_OUTPUT_DATA_TENSOR,
#else
    .object_detection = false,
    .object_detection_count = 0,
    .object_detection_threshold = 0.0,
    .object_detection_last_layer = EI_CLASSIFIER_LAST_LAYER_UNKNOWN,
    .tflite_output_labels_tensor = 0,
    .tflite_output_score_tensor = 0,
    .tflite_output_data_tensor = 0,
#endif

#ifdef EI_CLASSIFIER_NN_OUTPUT_COUNT
    .tflite_output_features_count = EI_CLASSIFIER_NN_OUTPUT_COUNT,
#else
    .tflite_output_features_count = 0,
#endif

#if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE) && (EI_CLASSIFIER_COMPILED != 1)
    .tflite_arena_size = EI_CLASSIFIER_TFLITE_ARENA_SIZE,
#else
    .tflite_arena_size = 0,
#endif

#if ((EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE) || (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE_FULL)) && (EI_CLASSIFIER_COMPILED != 1)
    .model_arr = trained_tflite,
    .model_arr_size = trained_tflite_len,
#elif (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSORRT)
    .model_arr = trained_onnx,
    .model_arr_size = trained_onnx_len,
#else
    .model_arr = 0,
    .model_arr_size = 0,
#endif

#if (EI_CLASSIFIER_INFERENCING_ENGINE != EI_CLASSIFIER_NONE)
    .tflite_input_datatype = EI_CLASSIFIER_TFLITE_INPUT_DATATYPE,
    .tflite_input_quantized = EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED,
    .tflite_input_scale = EI_CLASSIFIER_TFLITE_INPUT_SCALE,
    .tflite_input_zeropoint = EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT,
    .tflite_output_datatype = EI_CLASSIFIER_TFLITE_OUTPUT_DATATYPE,
    .tflite_output_quantized = EI_CLASSIFIER_TFLITE_OUTPUT_QUANTIZED,
    .tflite_output_scale = EI_CLASSIFIER_TFLITE_OUTPUT_SCALE,
    .tflite_output_zeropoint = EI_CLASSIFIER_TFLITE_OUTPUT_ZEROPOINT,
    .inferencing_engine = EI_CLASSIFIER_INFERENCING_ENGINE,
    .compiled = EI_CLASSIFIER_COMPILED,
    .has_tflite_ops_resolver = EI_CLASSIFIER_HAS_TFLITE_OPS_RESOLVER,
#else
    .tflite_input_datatype = 0,
    .tflite_input_quantized = 0,
    .tflite_input_scale = 0,
    .tflite_input_zeropoint = 0,
    .tflite_output_datatype = 0,
    .tflite_output_quantized = 0,
    .tflite_output_scale = 0,
    .tflite_output_zeropoint = 0,
    .inferencing_engine = 0,
    .compiled = 0,
    .has_tflite_ops_resolver = 0,
#endif

    .sensor = EI_CLASSIFIER_SENSOR,
#ifdef EI_CLASSIFIER_FUSION_AXES_STRING
    .fusion_string = EI_CLASSIFIER_FUSION_AXES_STRING,
#else
    .fusion_string = "null",
#endif

    .slice_size = (EI_CLASSIFIER_RAW_SAMPLE_COUNT / EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW),
    .slices_per_model_window = EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW,

#if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE) && (EI_CLASSIFIER_COMPILED == 1)
    .model_input = &trained_model_input,
    .model_output = &trained_model_output,
    .model_init = &trained_model_init,
    .model_invoke = &trained_model_invoke,
    .model_reset = &trained_model_reset,
#else
    .model_input = NULL,
    .model_output = NULL,
    .model_init =  NULL,
    .model_invoke = NULL,
    .model_reset = NULL,
#endif
    .categories = ei_classifier_inferencing_categories
    };

    return impulse;
}
#endif

/**
 * Check if the current impulse could be used by 'run_classifier_image_quantized'
 */
__attribute__((unused)) static EI_IMPULSE_ERROR can_run_classifier_image_quantized(const ei_impulse_t *impulse) {

    if (impulse->inferencing_engine != EI_CLASSIFIER_TFLITE
        && impulse->inferencing_engine != EI_CLASSIFIER_TENSAIFLOW
        && impulse->inferencing_engine != EI_CLASSIFIER_DRPAI) // check later
    {
        return EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE;
    }

    if (impulse->has_anomaly == 1){
        return EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES;
    }

        // Check if we have a quantized NN Input layer (input is always quantized for DRP-AI)
    if (impulse->tflite_input_quantized != 1) {
        return EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES;
    }

    // And if we have one DSP block which operates on images...
    if (impulse->dsp_blocks_size != 1 || impulse->dsp_blocks[0].extract_fn != extract_image_features) {
        return EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES;
    }

    return EI_IMPULSE_OK;
}

#if EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1 && (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE || EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSAIFLOW || EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_DRPAI)

/**
 * Special function to run the classifier on images, only works on TFLite models (either interpreter or EON or for tensaiflow)
 * that allocates a lot less memory by quantizing in place. This only works if 'can_run_classifier_image_quantized'
 * returns EI_IMPULSE_OK.
 */
extern "C" EI_IMPULSE_ERROR run_classifier_image_quantized(
    const ei_impulse_t *impulse,
    signal_t *signal,
    ei_impulse_result_t *result,
    bool debug = false)
{
    EI_IMPULSE_ERROR verify_res = can_run_classifier_image_quantized(impulse);
    if (verify_res != EI_IMPULSE_OK) {
        return verify_res;
    }

    memset(result, 0, sizeof(ei_impulse_result_t));

    return run_nn_inference_image_quantized(impulse, signal, result, debug);

}

#endif // #if EI_CLASSIFIER_TFLITE_INPUT_QUANTIZED == 1 && (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE || EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TENSAIFLOW || EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_DRPAI)

/* Public functions ------------------------------------------------------- */

/* Thread carefully: public functions are not to be changed
to preserve backwards compatibility. */

/**
 * @brief      Init static vars
 */
extern "C" void run_classifier_init()
{

    classifier_continuous_features_written = 0;
    ei_dsp_clear_continuous_audio_state();

#if EI_CLASSIFIER_CALIBRATION_ENABLED

#if EI_CLASSIFIER_STUDIO_VERSION < 3
        const ei_impulse_t impulse = ei_construct_impulse();
#else
       const ei_impulse_t impulse = ei_default_impulse;
#endif

    const ei_model_performance_calibration_t *calibration = &ei_calibration;

    if(calibration != NULL) {
        avg_scores = new RecognizeEvents(calibration,
            impulse.label_count, impulse.slice_size, impulse.interval_ms);
    }
#endif
}

/**
 * @brief      Init static vars, for multi-model support
 */
__attribute__((unused)) void run_classifier_init(const ei_impulse_t *impulse)
{
    classifier_continuous_features_written = 0;
    ei_dsp_clear_continuous_audio_state();

#if EI_CLASSIFIER_CALIBRATION_ENABLED
    const ei_model_performance_calibration_t *calibration = &ei_calibration;

    if(calibration != NULL) {
        avg_scores = new RecognizeEvents(calibration,
            impulse->label_count, impulse->slice_size, impulse->interval_ms);
    }
#endif
}

extern "C" void run_classifier_deinit(void)
{
    if((void *)avg_scores != NULL) {
        delete avg_scores;
    }
}

/**
 * @brief      Fill the complete matrix with sample slices. From there, run inference
 *             on the matrix.
 *
 * @param      signal  Sample data
 * @param      result  Classification output
 * @param[in]  debug   Debug output enable boot
 *
 * @return     The ei impulse error.
 */
extern "C" EI_IMPULSE_ERROR run_classifier_continuous(
    signal_t *signal,
    ei_impulse_result_t *result,
    bool debug = false,
    bool enable_maf = true)
{
#if EI_CLASSIFIER_STUDIO_VERSION < 3
        const ei_impulse_t impulse = ei_construct_impulse();
#else
       const ei_impulse_t impulse = ei_default_impulse;
#endif
    return process_impulse_continuous(&impulse, signal, result, debug, enable_maf);
}

/**
 * @brief      Fill the complete matrix with sample slices. From there, run impulse
 *             on the matrix.
 *
 * @param      impulse struct with information about model and DSP
 * @param      signal  Sample data
 * @param      result  Classification output
 * @param[in]  debug   Debug output enable boot
 *
 * @return     The ei impulse error.
 */
__attribute__((unused)) EI_IMPULSE_ERROR run_classifier_continuous(
    const ei_impulse_t *impulse,
    signal_t *signal,
    ei_impulse_result_t *result,
    bool debug = false,
    bool enable_maf = true)
{
    return process_impulse_continuous(impulse, signal, result, debug, enable_maf);
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
#if EI_CLASSIFIER_STUDIO_VERSION < 3
        const ei_impulse_t impulse = ei_construct_impulse();
#else
       const ei_impulse_t impulse = ei_default_impulse;
#endif
    return process_impulse(&impulse, signal, result, debug);
}

/**
 * Run the impulse over a raw features array
 * @param impulse struct with information about model and DSP
 * @param raw_features Raw features array
 * @param raw_features_size Size of the features array
 * @param result Object to store the results in
 * @param debug Whether to show debug messages (default: false)
 */
__attribute__((unused)) EI_IMPULSE_ERROR run_classifier(
    const ei_impulse_t *impulse,
    signal_t *signal,
    ei_impulse_result_t *result,
    bool debug = false)
{
    ei_printf("%s\n", impulse->project_name);
    return process_impulse(impulse, signal, result, debug);
}

/* Deprecated functions ------------------------------------------------------- */

/* These functions are being deprecated and possibly will be removed or moved in future.
Do not use these - if possible, change your code to reflect the upcoming changes. */

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

#if EI_CLASSIFIER_STUDIO_VERSION < 3
        const ei_impulse_t impulse = ei_construct_impulse();
#else
       const ei_impulse_t impulse = ei_default_impulse;
#endif

    float *x = (float*)calloc(impulse.dsp_input_frame_size, sizeof(float));
    if (!x) {
        return EI_IMPULSE_OUT_OF_MEMORY;
    }

    uint64_t next_tick = 0;

    uint64_t sampling_us_start = ei_read_timer_us();

    // grab some data
    for (int i = 0; i < impulse.dsp_input_frame_size; i += impulse.raw_samples_per_frame) {
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
