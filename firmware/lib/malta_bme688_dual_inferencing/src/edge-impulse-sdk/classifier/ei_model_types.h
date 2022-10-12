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

#ifndef _EDGE_IMPULSE_MODEL_TYPES_H_
#define _EDGE_IMPULSE_MODEL_TYPES_H_

#include <stdint.h>

#include "edge-impulse-sdk/dsp/numpy.hpp"
#include "edge-impulse-sdk/tensorflow/lite/c/common.h"

#define EI_CLASSIFIER_NONE                       255
#define EI_CLASSIFIER_UTENSOR                    1
#define EI_CLASSIFIER_TFLITE                     2
#define EI_CLASSIFIER_CUBEAI                     3
#define EI_CLASSIFIER_TFLITE_FULL                4
#define EI_CLASSIFIER_TENSAIFLOW                 5
#define EI_CLASSIFIER_TENSORRT                   6
#define EI_CLASSIFIER_DRPAI                      7

#define EI_CLASSIFIER_SENSOR_UNKNOWN             -1
#define EI_CLASSIFIER_SENSOR_MICROPHONE          1
#define EI_CLASSIFIER_SENSOR_ACCELEROMETER       2
#define EI_CLASSIFIER_SENSOR_CAMERA              3
#define EI_CLASSIFIER_SENSOR_9DOF                4
#define EI_CLASSIFIER_SENSOR_ENVIRONMENTAL       5
#define EI_CLASSIFIER_SENSOR_FUSION              6

// These must match the enum values in TensorFlow Lite's "TfLiteType"
#define EI_CLASSIFIER_DATATYPE_FLOAT32           1
#define EI_CLASSIFIER_DATATYPE_INT8              9

#define EI_CLASSIFIER_LAST_LAYER_UNKNOWN         -1
#define EI_CLASSIFIER_LAST_LAYER_SSD             1
#define EI_CLASSIFIER_LAST_LAYER_FOMO            2
#define EI_CLASSIFIER_LAST_LAYER_YOLOV5          3

struct ei_impulse;

typedef struct {
    uint16_t implementation_version;
    bool is_configured;
    uint32_t average_window_duration_ms;
    float detection_threshold;
    uint32_t suppression_ms;
    uint32_t suppression_flags;
} ei_model_performance_calibration_t;

typedef struct {
    size_t n_output_features;
    int (*extract_fn)(ei::signal_t *signal, ei::matrix_t *output_matrix, void *config, const float frequency);
    void *config;
    uint8_t *axes;
    size_t axes_size;
} ei_model_dsp_t;

typedef struct ei_impulse {
    uint32_t project_id;
    const char *project_owner;
    const char *project_name;

    uint32_t deploy_version;
    uint32_t nn_input_frame_size;
    uint32_t raw_sample_count;
    uint32_t raw_samples_per_frame;
    uint32_t dsp_input_frame_size;

    uint32_t input_width;
    uint32_t input_height;
    uint32_t input_frames;

    float interval_ms;
    uint16_t label_count;
    bool has_anomaly;
    float frequency;
    bool use_quantized_dsp_block;
    size_t dsp_blocks_size;
    ei_model_dsp_t *dsp_blocks;

    bool object_detection;
    uint16_t object_detection_count;
    float object_detection_threshold;
    int8_t object_detection_last_layer;
    uint8_t tflite_output_labels_tensor;
    uint8_t tflite_output_score_tensor;
    uint8_t tflite_output_data_tensor;
    uint32_t tflite_output_features_count;

    uint32_t tflite_arena_size;
    const uint8_t *model_arr;
    size_t model_arr_size;
    uint8_t tflite_input_datatype;
    bool tflite_input_quantized;
    float tflite_input_scale;
    float tflite_input_zeropoint;
    uint8_t tflite_output_datatype;
    bool tflite_output_quantized;
    float tflite_output_scale;
    float tflite_output_zeropoint;

    uint32_t inferencing_engine;
    bool compiled;
    bool has_tflite_ops_resolver;
    uint32_t sensor;
    const char *fusion_string;
    uint32_t slice_size;
    uint32_t slices_per_model_window;

    TfLiteTensor* (*model_input)(int);
    TfLiteTensor* (*model_output)(int);
    TfLiteStatus (*model_init)(void*(*alloc_fnc)(size_t,size_t));
    TfLiteStatus (*model_invoke)();
    TfLiteStatus (*model_reset)(void (*free)(void* ptr));
    const char **categories;
} ei_impulse_t;

#endif // _EDGE_IMPULSE_MODEL_TYPES_H_
