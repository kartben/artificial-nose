/* Edge Impulse inferencing library
 * Copyright (c) 2020 EdgeImpulse Inc.
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
#if defined(EI_CLASSIFIER_HAS_SAMPLER) && EI_CLASSIFIER_HAS_SAMPLER == 1
#include "ei_sampler.h"
#endif
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "model-parameters/dsp_blocks.h"

#if EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_UTENSOR
#include "utensor-model/trained.hpp"
#include "utensor-model/trained_weight.hpp"            // keep the weights in ROM for now, we have plenty of internal flash
#elif (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE)
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/all_ops_resolver.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_error_reporter.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_interpreter.h"
#include "edge-impulse-sdk/tensorflow/lite/schema/schema_generated.h"
#include "edge-impulse-sdk/tensorflow/lite/version.h"

#include "tflite-model/tflite-trained.h"

static tflite::MicroErrorReporter micro_error_reporter;
static tflite::ErrorReporter* error_reporter = &micro_error_reporter;
#elif (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_CUBEAI)

#include <assert.h>
#include "cubeai-model/network.h"
#include "cubeai-model/network_data.h"

static_assert(AI_NETWORK_IN_1_SIZE == EI_CLASSIFIER_NN_INPUT_FRAME_SIZE, "AI_NETWORK_IN_1_SIZE should equal EI_CLASSIFIER_NN_INPUT_FRAME_SIZE");
AI_ALIGNED(4)
static ai_float in_data[AI_NETWORK_IN_1_SIZE];

static_assert(AI_NETWORK_OUT_1_SIZE == EI_CLASSIFIER_LABEL_COUNT, "AI_NETWORK_OUT_1_SIZE should equal EI_CLASSIFIER_LABEL_COUNT");
AI_ALIGNED(4)
static ai_float out_data[AI_NETWORK_OUT_1_SIZE];

/* Global buffer to handle the activations data buffer - R/W data */
AI_ALIGNED(4)
static ai_u8 activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE];

static ai_handle network = AI_HANDLE_NULL;

#else
#error "Unknown inferencing engine"
#endif

#ifdef __cplusplus
namespace {
#endif // __cplusplus

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
    // if (debug) {
    //     ei_printf("Input data: ");
    //     for (size_t ix = 0; ix < raw_features_size; ix++) {
    //         ei_printf("%f ", raw_features[ix]);
    //     }
    //     ei_printf("\n");
    // }

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

        int ret = block.extract_fn(signal, &fm, block.config);
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
            ei_printf("%f ", features_matrix.buffer[ix]);
        }
        ei_printf("\n");
    }

    if (debug) {
        ei_printf("Running neural network...\n");
    }
#if EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_UTENSOR
    // now turn into floats...
    RamTensor<float> *input_x = new RamTensor<float>({ 1, static_cast<unsigned int>(features_matrix.rows * features_matrix.cols) });
    float *buff = (float*)input_x->write(0, 0);
    memcpy(buff, features_matrix.buffer, features_matrix.rows * features_matrix.cols * sizeof(float));

    {
        uint64_t ctx_start_ms = ei_read_timer_ms();
        Context ctx;
        get_trained_ctx(ctx, input_x);
        ctx.eval();
        uint64_t ctx_end_ms = ei_read_timer_ms();

        if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
            return EI_IMPULSE_CANCELED;
        }

        result->timing.classification = ctx_end_ms - ctx_start_ms;

        S_TENSOR pred_tensor = ctx.get(EI_CLASSIFIER_OUT_TENSOR_NAME);  // getting a reference to the output tensor

        uint32_t output_neurons = pred_tensor->getShape()[1];

        if (output_neurons != EI_CLASSIFIER_LABEL_COUNT) {
            return EI_IMPULSE_ERROR_SHAPES_DONT_MATCH;
        }

        if (debug) {
            ei_printf("Predictions (time: %d ms.):\n", result->timing.classification);
        }
        const float* ptr_pred = pred_tensor->read<float>(0, 0);

        for (uint32_t ix = 0; ix < output_neurons; ix++) {
            if (debug) {
                ei_printf("%s:\t%f\n", ei_classifier_inferencing_categories[ix], *(ptr_pred + ix));
            }
            result->classification[ix].label = ei_classifier_inferencing_categories[ix];
            result->classification[ix].value = *(ptr_pred + ix);
        }
    }
#elif (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_TFLITE)
    {
        // Create an area of memory to use for input, output, and intermediate arrays.
        // Finding the minimum value for your model may require some trial and error.
        uint8_t *tensor_arena = (uint8_t*)malloc(EI_CLASSIFIER_TFLITE_ARENA_SIZE);
        if (!tensor_arena) {
            ei_printf("Failed to allocate TFLite arena (%d bytes)\n", EI_CLASSIFIER_TFLITE_ARENA_SIZE);
            return EI_IMPULSE_TFLITE_ARENA_ALLOC_FAILED;
        }

        uint64_t ctx_start_ms = ei_read_timer_ms();

        static bool tflite_first_run = true;
        static const tflite::Model* model = nullptr;

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
                free(tensor_arena);
                return EI_IMPULSE_TFLITE_ERROR;
            }

            tflite_first_run = false;
        }

        tflite::ops::micro::AllOpsResolver resolver;

        // Build an interpreter to run the model with.
        tflite::MicroInterpreter interpreter(
            model, resolver, tensor_arena, EI_CLASSIFIER_TFLITE_ARENA_SIZE, error_reporter);

        // Allocate memory from the tensor_arena for the model's tensors.
        TfLiteStatus allocate_status = interpreter.AllocateTensors();
        if (allocate_status != kTfLiteOk) {
            error_reporter->Report("AllocateTensors() failed");
            free(tensor_arena);
            return EI_IMPULSE_TFLITE_ERROR;
        }

        // Obtain pointers to the model's input and output tensors.
        TfLiteTensor* input = interpreter.input(0);
        TfLiteTensor* output = interpreter.output(0);
        // =====
        // Initialization code done
        // =====

        // Place our calculated x value in the model's input tensor
        for (size_t ix = 0; ix < features_matrix.cols; ix++) {
            input->data.f[ix] = features_matrix.buffer[ix];
        }

        // Run inference, and report any error
        TfLiteStatus invoke_status = interpreter.Invoke();
        if (invoke_status != kTfLiteOk) {
            error_reporter->Report("Invoke failed (%d)\n", invoke_status);
            free(tensor_arena);
            return EI_IMPULSE_TFLITE_ERROR;
        }

        uint64_t ctx_end_ms = ei_read_timer_ms();

        result->timing.classification = ctx_end_ms - ctx_start_ms;

        // Read the predicted y value from the model's output tensor
        if (debug) {
            ei_printf("Predictions (time: %d ms.):\n", result->timing.classification);
        }
        for (uint32_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            if (debug) {
                ei_printf("%s:\t%f\n", ei_classifier_inferencing_categories[ix], output->data.f[ix]);
            }
            result->classification[ix].label = ei_classifier_inferencing_categories[ix];
            result->classification[ix].value = output->data.f[ix];
        }

        free(tensor_arena);

        if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
            return EI_IMPULSE_CANCELED;
        }
    }
#elif EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_CUBEAI

    if (!network) {
#if defined(STM32H7)
		/* By default the CRC IP clock is enabled */
		__HAL_RCC_CRC_CLK_ENABLE();
#else
		if (!__HAL_RCC_CRC_IS_CLK_ENABLED()) {
			ei_printf("W: CRC IP clock is NOT enabled\r\n");
		}

		/* By default the CRC IP clock is enabled */
		__HAL_RCC_CRC_CLK_ENABLE();
#endif

    	ai_error err;

        if (debug) {
    	    ei_printf("Creating network...\n");
        }

    	// create the network
	    err = ai_network_create(&network, (const ai_buffer*)AI_NETWORK_DATA_CONFIG);
	    if (err.type != AI_ERROR_NONE) {
	    	ei_printf("CubeAI error - type=%d code=%d\r\n", err.type, err.code);
	    	return EI_IMPULSE_CUBEAI_ERROR;
	    }

        const ai_network_params params = {
           AI_NETWORK_DATA_WEIGHTS(ai_network_data_weights_get()),
           AI_NETWORK_DATA_ACTIVATIONS(activations) };

        if (debug) {
            ei_printf("Initializing network...\n");
        }

        if (!ai_network_init(network, &params)) {
            err = ai_network_get_error(network);
	    	ei_printf("CubeAI error - type=%d code=%d\r\n", err.type, err.code);
	    	return EI_IMPULSE_CUBEAI_ERROR;
        }
    }

    uint64_t ctx_start_ms = ei_read_timer_ms();

    // features_matrix.buffer <-- input data
    memcpy(in_data, features_matrix.buffer, AI_NETWORK_IN_1_SIZE * sizeof(float));

    ai_i32 n_batch;
    ai_error err;

    /* 1 - Create the AI buffer IO handlers */
    ai_buffer ai_input[AI_NETWORK_IN_NUM] = AI_NETWORK_IN ;
    ai_buffer ai_output[AI_NETWORK_OUT_NUM] = AI_NETWORK_OUT ;

    /* 2 - Initialize input/output buffer handlers */
    ai_input[0].n_batches = 1;
    ai_input[0].data = AI_HANDLE_PTR(in_data);
    ai_output[0].n_batches = 1;
    ai_output[0].data = AI_HANDLE_PTR(out_data);

    /* 3 - Perform the inference */
    n_batch = ai_network_run(network, &ai_input[0], &ai_output[0]);
    if (n_batch != 1) {
        err = ai_network_get_error(network);
    	ei_printf("CubeAI error - type=%d code=%d\r\n", err.type, err.code);
    	return EI_IMPULSE_CUBEAI_ERROR;
    }

    uint64_t ctx_end_ms = ei_read_timer_ms();

    result->timing.classification = ctx_end_ms - ctx_start_ms;

    if (debug) {
        ei_printf("Predictions (time: %d ms.):\n", result->timing.classification);
    }
    for (uint32_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        if (debug) {
            ei_printf("%s:\t%f\n", ei_classifier_inferencing_categories[ix], out_data[ix]);
        }
        result->classification[ix].label = ei_classifier_inferencing_categories[ix];
        result->classification[ix].value = out_data[ix];
    }

#endif

#if EI_CLASSIFIER_HAS_ANOMALY == 1

    // Anomaly detection
    {
        uint64_t anomaly_start_ms = ei_read_timer_ms();

        float input[EI_CLASSIFIER_ANOM_AXIS_SIZE];
        for (size_t ix = 0; ix < EI_CLASSIFIER_ANOM_AXIS_SIZE; ix++) {
            input[ix] = features_matrix.buffer[EI_CLASSIFIER_ANOM_AXIS[ix]];
        }
        standard_scaler(input, ei_classifier_anom_scale, ei_classifier_anom_mean, EI_CLASSIFIER_ANOM_AXIS_SIZE);
        float anomaly = get_min_distance_to_cluster(
            input, EI_CLASSIFIER_ANOM_AXIS_SIZE, ei_classifier_anom_clusters, EI_CLASSIFIER_ANOM_CLUSTER_COUNT);

        uint64_t anomaly_end_ms = ei_read_timer_ms();

        if (debug) {
            ei_printf("Anomaly score (time: %d ms.): %f\n", static_cast<int>(anomaly_end_ms - anomaly_start_ms), anomaly);
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
        Callback<void(float*, size_t)> data_fn,
#else
        std::function<void(float*, size_t)> data_fn,
#endif
        bool debug = false) {

    float x[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

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

    signal_t signal;
    int err = numpy::signal_from_buffer(x, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
        ei_printf("ERR: signal_from_buffer failed (%d)\n", err);
        return EI_IMPULSE_DSP_ERROR;
    }

    return run_classifier(&signal, result, debug);
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
        Callback<void(float*, size_t)> data_fn,
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
