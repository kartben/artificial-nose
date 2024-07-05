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

#ifndef _EI_CLASSIFIER_FILL_RESULT_STRUCT_H_
#define _EI_CLASSIFIER_FILL_RESULT_STRUCT_H_

using namespace ei;

#include "model-parameters/model_metadata.h"
#if EI_CLASSIFIER_HAS_MODEL_VARIABLES == 1
#include "model-parameters/model_variables.h"
#endif
#include "edge-impulse-sdk/classifier/ei_model_types.h"
#include "edge-impulse-sdk/classifier/ei_classifier_types.h"
#include "edge-impulse-sdk/classifier/ei_nms.h"
#include "edge-impulse-sdk/dsp/ei_vector.h"

#ifndef EI_HAS_OBJECT_DETECTION
    #if (EI_CLASSIFIER_OBJECT_DETECTION_LAST_LAYER == EI_CLASSIFIER_LAST_LAYER_SSD)
    #define EI_HAS_SSD 1
    #endif
    #if (EI_CLASSIFIER_OBJECT_DETECTION_LAST_LAYER == EI_CLASSIFIER_LAST_LAYER_FOMO)
    #define EI_HAS_FOMO 1
    #endif
    #if (EI_CLASSIFIER_OBJECT_DETECTION_LAST_LAYER == EI_CLASSIFIER_LAST_LAYER_YOLOV5) || (EI_CLASSIFIER_OBJECT_DETECTION_LAST_LAYER == EI_CLASSIFIER_LAST_LAYER_YOLOV5_V5_DRPAI)
    #define EI_HAS_YOLOV5 1
    #endif
    #if (EI_CLASSIFIER_OBJECT_DETECTION_LAST_LAYER == EI_CLASSIFIER_LAST_LAYER_YOLOX)
    #define EI_HAS_YOLOX 1
    #endif
    #if (EI_CLASSIFIER_OBJECT_DETECTION_LAST_LAYER == EI_CLASSIFIER_LAST_LAYER_YOLOV7)
    #define EI_HAS_YOLOV7 1
    #endif
    #if (EI_CLASSIFIER_OBJECT_DETECTION_LAST_LAYER == EI_CLASSIFIER_LAST_LAYER_TAO_RETINANET) || (EI_CLASSIFIER_OBJECT_DETECTION_LAST_LAYER == EI_CLASSIFIER_LAST_LAYER_TAO_SSD)
    #define EI_HAS_TAO_DECODE_DETECTIONS 1
    #endif
    #if (EI_CLASSIFIER_OBJECT_DETECTION_LAST_LAYER == EI_CLASSIFIER_LAST_LAYER_TAO_YOLOV3) || (EI_CLASSIFIER_OBJECT_DETECTION_LAST_LAYER == EI_CLASSIFIER_LAST_LAYER_TAO_YOLOV4)
    #define EI_HAS_TAO_YOLO 1
    #endif
    #if (EI_CLASSIFIER_OBJECT_DETECTION_LAST_LAYER == EI_CLASSIFIER_LAST_LAYER_TAO_YOLOV3)
    #define EI_HAS_TAO_YOLOV3 1
    #endif
    #if (EI_CLASSIFIER_OBJECT_DETECTION_LAST_LAYER == EI_CLASSIFIER_LAST_LAYER_TAO_YOLOV4)
    #define EI_HAS_TAO_YOLOV4 1
    #endif
    #if (EI_CLASSIFIER_OBJECT_DETECTION_LAST_LAYER == EI_CLASSIFIER_LAST_LAYER_YOLOV2)
    #define EI_HAS_YOLOV2 1
    #endif
#endif

__attribute__((unused)) inline float sigmoid(float a) {
    return 1.0f / (1.0f + exp(-a));
}

#ifdef EI_HAS_FOMO
typedef struct cube {
    size_t x;
    size_t y;
    size_t width;
    size_t height;
    float confidence;
    const char *label;
} ei_classifier_cube_t;

/**
 * Checks whether a new section overlaps with a cube,
 * and if so, will **update the cube**
 */
__attribute__((unused)) static bool ei_cube_check_overlap(ei_classifier_cube_t *c, int x, int y, int width, int height, float confidence) {
    bool is_overlapping = !(c->x + c->width < x || c->y + c->height < y || c->x > x + width || c->y > y + height);
    if (!is_overlapping) return false;

    // if we overlap, but the x of the new box is lower than the x of the current box
    if (x < c->x) {
        // update x to match new box and make width larger (by the diff between the boxes)
        c->x = x;
        c->width += c->x - x;
    }
    // if we overlap, but the y of the new box is lower than the y of the current box
    if (y < c->y) {
        // update y to match new box and make height larger (by the diff between the boxes)
        c->y = y;
        c->height += c->y - y;
    }
    // if we overlap, and x+width of the new box is higher than the x+width of the current box
    if (x + width > c->x + c->width) {
        // just make the box wider
        c->width += (x + width) - (c->x + c->width);
    }
    // if we overlap, and y+height of the new box is higher than the y+height of the current box
    if (y + height > c->y + c->height) {
        // just make the box higher
        c->height += (y + height) - (c->y + c->height);
    }
    // if the new box has higher confidence, then override confidence of the whole box
    if (confidence > c->confidence) {
        c->confidence = confidence;
    }
    return true;
}

__attribute__((unused)) static void ei_handle_cube(std::vector<ei_classifier_cube_t*> *cubes, int x, int y, float vf, const char *label, float detection_threshold) {
    if (vf < detection_threshold) return;

    bool has_overlapping = false;
    int width = 1;
    int height = 1;

    for (auto c : *cubes) {
        // not cube for same class? continue
        if (strcmp(c->label, label) != 0) continue;

        if (ei_cube_check_overlap(c, x, y, width, height, vf)) {
            has_overlapping = true;
            break;
        }
    }

    if (!has_overlapping) {
        ei_classifier_cube_t *cube = new ei_classifier_cube_t();
        cube->x = x;
        cube->y = y;
        cube->width = 1;
        cube->height = 1;
        cube->confidence = vf;
        cube->label = label;
        cubes->push_back(cube);
    }
}

__attribute__((unused)) static void fill_result_struct_from_cubes(ei_impulse_result_t *result, std::vector<ei_classifier_cube_t*> *cubes, int out_width_factor, uint32_t object_detection_count) {
    std::vector<ei_classifier_cube_t*> bbs;
    static std::vector<ei_impulse_result_bounding_box_t> results;
    int added_boxes_count = 0;
    results.clear();
    for (auto sc : *cubes) {
        bool has_overlapping = false;

        int x = sc->x;
        int y = sc->y;
        int width = sc->width;
        int height = sc->height;
        const char *label = sc->label;
        float vf = sc->confidence;

        for (auto c : bbs) {
            // not cube for same class? continue
            if (strcmp(c->label, label) != 0) continue;

            if (ei_cube_check_overlap(c, x, y, width, height, vf)) {
                has_overlapping = true;
                break;
            }
        }

        if (has_overlapping) {
            continue;
        }

        bbs.push_back(sc);

        ei_impulse_result_bounding_box_t tmp = {
            .label = sc->label,
            .x = (uint32_t)(sc->x * out_width_factor),
            .y = (uint32_t)(sc->y * out_width_factor),
            .width = (uint32_t)(sc->width * out_width_factor),
            .height = (uint32_t)(sc->height * out_width_factor),
            .value = sc->confidence
        };

        results.push_back(tmp);
        added_boxes_count++;
    }

    // if we didn't detect min required objects, fill the rest with fixed value
    if (added_boxes_count < object_detection_count) {
        results.resize(object_detection_count);
        for (size_t ix = added_boxes_count; ix < object_detection_count; ix++) {
            results[ix].value = 0.0f;
        }
    }

    for (auto c : *cubes) {
        delete c;
    }

    result->bounding_boxes = results.data();
    result->bounding_boxes_count = results.size();
}
#endif

__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_f32_fomo(const ei_impulse_t *impulse,
                                                                            const ei_learning_block_config_tflite_graph_t *block_config,
                                                                            ei_impulse_result_t *result,
                                                                            float *data,
                                                                            int out_width,
                                                                            int out_height) {
#ifdef EI_HAS_FOMO
    std::vector<ei_classifier_cube_t*> cubes;

    int out_width_factor = impulse->input_width / out_width;

    for (size_t y = 0; y < out_width; y++) {
        // ei_printf("    [ ");
        for (size_t x = 0; x < out_height; x++) {
            size_t loc = ((y * out_height) + x) * (impulse->label_count + 1);

            for (size_t ix = 1; ix < impulse->label_count + 1; ix++) {
                float vf = data[loc+ix];

                ei_handle_cube(&cubes, x, y, vf, impulse->categories[ix - 1], block_config->threshold);
            }
        }
    }

    fill_result_struct_from_cubes(result, &cubes, out_width_factor, impulse->object_detection_count);

    return EI_IMPULSE_OK;
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif
}

__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_i8_fomo(const ei_impulse_t *impulse,
                                                                           const ei_learning_block_config_tflite_graph_t *block_config,
                                                                           ei_impulse_result_t *result,
                                                                           int8_t *data,
                                                                           float zero_point,
                                                                           float scale,
                                                                           int out_width,
                                                                           int out_height) {
#ifdef EI_HAS_FOMO
    std::vector<ei_classifier_cube_t*> cubes;

    int out_width_factor = impulse->input_width / out_width;

    for (size_t y = 0; y < out_width; y++) {
        // ei_printf("    [ ");
        for (size_t x = 0; x < out_height; x++) {
            size_t loc = ((y * out_height) + x) * (impulse->label_count + 1);

            for (size_t ix = 1; ix < impulse->label_count + 1; ix++) {
                int8_t v = data[loc+ix];
                float vf = static_cast<float>(v - zero_point) * scale;

                ei_handle_cube(&cubes, x, y, vf, impulse->categories[ix - 1], block_config->threshold);
            }
        }
    }

    fill_result_struct_from_cubes(result, &cubes, out_width_factor, impulse->object_detection_count);

    return EI_IMPULSE_OK;
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif
}

/**
 * Fill the result structure from an unquantized output tensor
 * (we don't support quantized here a.t.m.)
 */
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_f32_object_detection(const ei_impulse_t *impulse,
                                                                                        const ei_learning_block_config_tflite_graph_t *block_config,
                                                                                        ei_impulse_result_t *result,
                                                                                        float *data,
                                                                                        float *scores,
                                                                                        float *labels,
                                                                                        bool debug) {
#ifdef EI_HAS_SSD
    static std::vector<ei_impulse_result_bounding_box_t> results;
    results.clear();
    results.resize(impulse->object_detection_count);
    for (size_t ix = 0; ix < impulse->object_detection_count; ix++) {

        float score = scores[ix];
        float label = labels[ix];

        if (score >= block_config->threshold) {
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
                ei_printf("%s (", impulse->categories[(uint32_t)label]);
                ei_printf_float(label);
                ei_printf("): ");
                ei_printf_float(score);
                ei_printf(" [ ");
                ei_printf_float(xstart);
                ei_printf(", ");
                ei_printf_float(ystart);
                ei_printf(", ");
                ei_printf_float(xend);
                ei_printf(", ");
                ei_printf_float(yend);
                ei_printf(" ]\n");
            }

            results[ix].label = impulse->categories[(uint32_t)label];
            results[ix].x = static_cast<uint32_t>(xstart * static_cast<float>(impulse->input_width));
            results[ix].y = static_cast<uint32_t>(ystart * static_cast<float>(impulse->input_height));
            results[ix].width = static_cast<uint32_t>((xend - xstart) * static_cast<float>(impulse->input_width));
            results[ix].height = static_cast<uint32_t>((yend - ystart) * static_cast<float>(impulse->input_height));
            results[ix].value = score;
        }
        else {
            results[ix].value = 0.0f;
        }
    }
    result->bounding_boxes = results.data();
    result->bounding_boxes_count = results.size();

    return EI_IMPULSE_OK;
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif
}

/**
 * Fill the result structure from a quantized output tensor
 */
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_i8(const ei_impulse_t *impulse,
                                                                      ei_impulse_result_t *result,
                                                                      int8_t *data,
                                                                      float zero_point,
                                                                      float scale,
                                                                      bool debug) {
    for (uint32_t ix = 0; ix < impulse->label_count; ix++) {
        float value = static_cast<float>(data[ix] - zero_point) * scale;

        if (debug) {
            ei_printf("%s:\t", impulse->categories[ix]);
            ei_printf_float(value);
            ei_printf("\n");
        }
        result->classification[ix].label = impulse->categories[ix];
        result->classification[ix].value = value;
    }

    return EI_IMPULSE_OK;
}

/**
 * Fill the result structure from an unquantized output tensor
 */
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_f32(const ei_impulse_t *impulse,
                                                                       ei_impulse_result_t *result,
                                                                       float *data,
                                                                       bool debug) {
    for (uint32_t ix = 0; ix < impulse->label_count; ix++) {
        float value = data[ix];

        if (debug) {
            ei_printf("%s:\t", impulse->categories[ix]);
            ei_printf_float(value);
            ei_printf("\n");
        }
        result->classification[ix].label = impulse->categories[ix];
        result->classification[ix].value = value;
    }

    return EI_IMPULSE_OK;
}

/**
 * Fill the visual anomaly result structures from an unquantized output tensor
 */
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_visual_ad_struct_f32(const ei_impulse_t *impulse,
                                                                       ei_impulse_result_t *result,
                                                                       float *data,
                                                                       float threshold,
                                                                       bool debug) {
#if EI_CLASSIFIER_HAS_VISUAL_ANOMALY
    float max_val = 0;
    float sum_val = 0;
    // the feature extractor output will be 1/8 of input
    // due to the cut-off layer chosen in MobileNetV2
    uint32_t grid_size_x = (impulse->input_width / 8) / 2 - 1;
    uint32_t grid_size_y = (impulse->input_height / 8) / 2 - 1;

    for (uint32_t ix = 0; ix < grid_size_x * grid_size_y; ix++) {
        float value = data[ix];
        sum_val += value;
        if (value > max_val) {
            max_val = value;
        }
    }

    result->visual_ad_result.mean_value = sum_val / (grid_size_x * grid_size_y);
    result->visual_ad_result.max_value = max_val;

    static ei_vector<ei_impulse_result_bounding_box_t> results;

    int added_boxes_count = 0;
    results.clear();

    for (uint32_t x = 0; x <= grid_size_x - 1; x++) {
        for (uint32_t y = 0; y <= grid_size_y - 1; y++) {
            if (data[x * grid_size_x + y] >= threshold) {
                ei_impulse_result_bounding_box_t tmp = {
                    .label = "anomaly",
                    .x = static_cast<uint32_t>(y * (static_cast<float>(impulse->input_height) / grid_size_y)),
                    .y = static_cast<uint32_t>(x * (static_cast<float>(impulse->input_width) / grid_size_x)),
                    .width = (impulse->input_width / grid_size_x),
                    .height = (impulse->input_height / grid_size_y),
                    .value = data[x * grid_size_x + y]
                };

                results.push_back(tmp);
                added_boxes_count++;
            }
        }
    }

    // if we didn't detect min required objects, fill the rest with fixed value
    if (added_boxes_count < impulse->object_detection_count) {
        results.resize(impulse->object_detection_count);
        for (size_t ix = added_boxes_count; ix < impulse->object_detection_count; ix++) {
            results[ix].value = 0.0f;
        }
    }

    result->visual_ad_grid_cells = results.data();
    result->visual_ad_count = results.size();
#endif // EI_CLASSIFIER_HAS_VISUAL_ANOMALY
    return EI_IMPULSE_OK;
}

/**
  * Fill the result structure from an unquantized output tensor
  */
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_f32_yolov5(const ei_impulse_t *impulse,
                                                                              const ei_learning_block_config_tflite_graph_t *block_config,
                                                                              ei_impulse_result_t *result,
                                                                              int version,
                                                                              float *data,
                                                                              size_t output_features_count,
                                                                              bool debug = false) {
#ifdef EI_HAS_YOLOV5
    static std::vector<ei_impulse_result_bounding_box_t> results;
    results.clear();

    size_t col_size = 5 + impulse->label_count;
    size_t row_count = output_features_count / col_size;

    for (size_t ix = 0; ix < row_count; ix++) {
        size_t base_ix = ix * col_size;
        float xc = data[base_ix + 0];
        float yc = data[base_ix + 1];
        float w = data[base_ix + 2];
        float h = data[base_ix + 3];
        float x = xc - (w / 2.0f);
        float y = yc - (h / 2.0f);
        if (x < 0) {
            x = 0;
        }
        if (y < 0) {
            y = 0;
        }
        if (x + w > impulse->input_width) {
            w = impulse->input_width - x;
        }
        if (y + h > impulse->input_height) {
            h = impulse->input_height - y;
        }

        if (w < 0 || h < 0) {
            continue;
        }

        float score = data[base_ix + 4];

        uint32_t label = 0;
        for (size_t lx = 0; lx < impulse->label_count; lx++) {
            float l = data[base_ix + 5 + lx];
            if (l > 0.5f) {
                label = lx;
                break;
            }
        }

        if (score >= block_config->threshold && score <= 1.0f) {
            ei_impulse_result_bounding_box_t r;
            r.label = impulse->categories[label];

            if (version != 5) {
                x *= static_cast<float>(impulse->input_width);
                y *= static_cast<float>(impulse->input_height);
                w *= static_cast<float>(impulse->input_width);
                h *= static_cast<float>(impulse->input_height);
            }

            r.x = static_cast<uint32_t>(x);
            r.y = static_cast<uint32_t>(y);
            r.width = static_cast<uint32_t>(w);
            r.height = static_cast<uint32_t>(h);
            r.value = score;
            results.push_back(r);
        }
    }

    EI_IMPULSE_ERROR nms_res = ei_run_nms(impulse, &results, debug);
    if (nms_res != EI_IMPULSE_OK) {
        return nms_res;
    }

    // if we didn't detect min required objects, fill the rest with fixed value
    size_t added_boxes_count = results.size();
    size_t min_object_detection_count = impulse->object_detection_count;
    if (added_boxes_count < min_object_detection_count) {
        results.resize(min_object_detection_count);
        for (size_t ix = added_boxes_count; ix < min_object_detection_count; ix++) {
            results[ix].value = 0.0f;
        }
    }

    result->bounding_boxes = results.data();
    result->bounding_boxes_count = results.size();

    return EI_IMPULSE_OK;
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif
}

/**
 * Fill the result structure from a quantized output tensor
*/
template<typename T>
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_quantized_yolov5(const ei_impulse_t *impulse,
                                                                                    const ei_learning_block_config_tflite_graph_t *block_config,
                                                                                    ei_impulse_result_t *result,
                                                                                    int version,
                                                                                    T *data,
                                                                                    float zero_point,
                                                                                    float scale,
                                                                                    size_t output_features_count,
                                                                                    bool debug = false) {
#ifdef EI_HAS_YOLOV5
    static std::vector<ei_impulse_result_bounding_box_t> results;
    results.clear();

    size_t col_size = 5 + impulse->label_count;
    size_t row_count = output_features_count / col_size;

    for (size_t ix = 0; ix < row_count; ix++) {
        size_t base_ix = ix * col_size;
        float xc = (data[base_ix + 0] - zero_point) * scale;
        float yc = (data[base_ix + 1] - zero_point) * scale;
        float w = (data[base_ix + 2] - zero_point) * scale;
        float h = (data[base_ix + 3] - zero_point) * scale;
        float x = xc - (w / 2.0f);
        float y = yc - (h / 2.0f);
        if (x < 0) {
            x = 0;
        }
        if (y < 0) {
            y = 0;
        }
        if (x + w > impulse->input_width) {
            w = impulse->input_width - x;
        }
        if (y + h > impulse->input_height) {
            h = impulse->input_height - y;
        }

        if (w < 0 || h < 0) {
            continue;
        }

        float score = (data[base_ix + 4] - zero_point) * scale;

        uint32_t label = 0;
        for (size_t lx = 0; lx < impulse->label_count; lx++) {
            float l = (data[base_ix + 5 + lx] - zero_point) * scale;
            if (l > 0.5f) {
                label = lx;
                break;
            }
        }

        if (score >= block_config->threshold && score <= 1.0f) {
            ei_impulse_result_bounding_box_t r;
            r.label = ei_classifier_inferencing_categories[label];

            if (version != 5) {
                x *= static_cast<float>(impulse->input_width);
                y *= static_cast<float>(impulse->input_height);
                w *= static_cast<float>(impulse->input_width);
                h *= static_cast<float>(impulse->input_height);
            }

            r.x = static_cast<uint32_t>(x);
            r.y = static_cast<uint32_t>(y);
            r.width = static_cast<uint32_t>(w);
            r.height = static_cast<uint32_t>(h);
            r.value = score;
            results.push_back(r);
        }
    }

    EI_IMPULSE_ERROR nms_res = ei_run_nms(impulse, &results, debug);
    if (nms_res != EI_IMPULSE_OK) {
        return nms_res;
    }

    // if we didn't detect min required objects, fill the rest with fixed value
    size_t added_boxes_count = results.size();
    size_t min_object_detection_count = impulse->object_detection_count;
    if (added_boxes_count < min_object_detection_count) {
        results.resize(min_object_detection_count);
        for (size_t ix = added_boxes_count; ix < min_object_detection_count; ix++) {
            results[ix].value = 0.0f;
        }
    }

    result->bounding_boxes = results.data();
    result->bounding_boxes_count = results.size();

    return EI_IMPULSE_OK;
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif
}

/**
  * Fill the result structure from an unquantized output tensor
  * (we don't support quantized here a.t.m.)
  */
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_f32_yolox(const ei_impulse_t *impulse,
                                                                             const ei_learning_block_config_tflite_graph_t *block_config,
                                                                             ei_impulse_result_t *result,
                                                                             float *data,
                                                                             size_t output_features_count,
                                                                             bool debug = false) {
#ifdef EI_HAS_YOLOX
    static std::vector<ei_impulse_result_bounding_box_t> results;
    results.clear();

    // START: def yolox_postprocess()

    // if not p6:
    //     strides = [8, 16, 32]
    // else:
    //     strides = [8, 16, 32, 64]
    const std::vector<int> strides { 8, 16, 32 };

    // hsizes = [img_size[0] // stride for stride in strides]
    // wsizes = [img_size[1] // stride for stride in strides]
    std::vector<int> hsizes(strides.size());
    std::vector<int> wsizes(strides.size());
    for (int ix = 0; ix < (int)strides.size(); ix++) {
        hsizes[ix] = (int)floor((float)impulse->input_width / (float)strides[ix]);
        wsizes[ix] = (int)floor((float)impulse->input_height / (float)strides[ix]);
    }

    // for hsize, wsize, stride in zip(hsizes, wsizes, strides):
    //      grid = np.stack((xv, yv), 2).reshape(1, -1, 2)
    //      grids.append(grid)
    //      shape = grid.shape[:2]
    //      expanded_strides.append(np.full((*shape, 1), stride))
    std::vector<matrix_i32_t*> grids;
    std::vector<matrix_i32_t*> expanded_strides;

    for (int ix = 0; ix < (int)strides.size(); ix++) {
        int hsize = hsizes.at(ix);
        int wsize = wsizes.at(ix);
        int stride = strides.at(ix);

        // xv, yv = np.meshgrid(np.arange(wsize), np.arange(hsize))
        // grid = np.stack((xv, yv), 2).reshape(1, -1, 2)
        matrix_i32_t *grid = new matrix_i32_t(hsize * wsize, 2);
        int grid_ix = 0;
        for (int h = 0; h < hsize; h++) {
            for (int w = 0; w < wsize; w++) {
                grid->buffer[grid_ix + 0] = w;
                grid->buffer[grid_ix + 1] = h;
                grid_ix += 2;
            }
        }
        grids.push_back(grid);

        // shape = grid.shape[:2]
        // expanded_strides.append(np.full((*shape, 1), stride))
        matrix_i32_t *expanded_stride = new matrix_i32_t(hsize * wsize, 1);
        for (int ix = 0; ix < hsize * wsize; ix++) {
            expanded_stride->buffer[ix] = stride;
        }
        expanded_strides.push_back(expanded_stride);
    }

    // grids = np.concatenate(grids, 1)
    int total_grid_rows = 0;
    for (auto g : grids) {
        total_grid_rows += g->rows;
    }
    matrix_i32_t c_grid(total_grid_rows, 2);
    int c_grid_ix = 0;
    for (auto g : grids) {
        for (int row = 0; row < (int)g->rows; row++) {
            c_grid.buffer[c_grid_ix + 0] = g->buffer[(row * 2) + 0];
            c_grid.buffer[c_grid_ix + 1] = g->buffer[(row * 2) + 1];
            c_grid_ix += 2;
        }
        delete g;
    }

    // expanded_strides = np.concatenate(expanded_strides, 1)
    int total_stride_rows = 0;
    for (auto g : expanded_strides) {
        total_stride_rows += g->rows;
    }
    matrix_i32_t c_expanded_strides(total_stride_rows, 1);
    int c_expanded_strides_ix = 0;
    for (auto g : expanded_strides) {
        for (int row = 0; row < (int)g->rows; row++) {
            c_expanded_strides.buffer[c_expanded_strides_ix + 0] = g->buffer[(row * 1) + 0];
            c_expanded_strides_ix += 1;
        }
        delete g;
    }

    const int output_rows = output_features_count / (5 + impulse->label_count);
    matrix_t outputs(output_rows, 5 + impulse->label_count, data);
    for (int row = 0; row < (int)outputs.rows; row++) {
        float v0 = outputs.buffer[(row * outputs.cols) + 0];
        float v1 = outputs.buffer[(row * outputs.cols) + 1];
        float v2 = outputs.buffer[(row * outputs.cols) + 2];
        float v3 = outputs.buffer[(row * outputs.cols) + 3];

        float cgrid0 = (float)c_grid.buffer[(row * c_grid.cols) + 0];
        float cgrid1 = (float)c_grid.buffer[(row * c_grid.cols) + 1];

        float stride = (float)c_expanded_strides.buffer[row];

        // outputs[..., :2] = (outputs[..., :2] + grids) * expanded_strides
        outputs.buffer[(row * outputs.cols) + 0] = (v0 + cgrid0) * stride;
        outputs.buffer[(row * outputs.cols) + 1] = (v1 + cgrid1) * stride;

        // outputs[..., 2:4] = np.exp(outputs[..., 2:4]) * expanded_strides
        outputs.buffer[(row * outputs.cols) + 2] = exp(v2) * stride;
        outputs.buffer[(row * outputs.cols) + 3] = exp(v3) * stride;
    }

    // END: def yolox_postprocess()

    // boxes = predictions[:, :4]
    matrix_t boxes(outputs.rows, 4);
    for (int row = 0; row < (int)outputs.rows; row++) {
        boxes.buffer[(row * boxes.cols) + 0] = outputs.buffer[(row * outputs.cols) + 0];
        boxes.buffer[(row * boxes.cols) + 1] = outputs.buffer[(row * outputs.cols) + 1];
        boxes.buffer[(row * boxes.cols) + 2] = outputs.buffer[(row * outputs.cols) + 2];
        boxes.buffer[(row * boxes.cols) + 3] = outputs.buffer[(row * outputs.cols) + 3];
    }

    // scores = predictions[:, 4:5] * predictions[:, 5:]
    matrix_t scores(outputs.rows, impulse->label_count);
    for (int row = 0; row < (int)outputs.rows; row++) {
        float confidence = outputs.buffer[(row * outputs.cols) + 4];
        for (int cc = 0; cc < impulse->label_count; cc++) {
            scores.buffer[(row * scores.cols) + cc] = confidence * outputs.buffer[(row * outputs.cols) + (5 + cc)];
        }
    }

    // iterate through scores to see if we have anything with confidence
    for (int row = 0; row < (int)scores.rows; row++) {
        for (int col = 0; col < (int)scores.cols; col++) {
            float confidence = scores.buffer[(row * scores.cols) + col];

            if (confidence >= block_config->threshold && confidence <= 1.0f) {
                ei_impulse_result_bounding_box_t r;
                r.label = impulse->categories[col];
                r.value = confidence;

                // now find the box...
                float xcenter = boxes.buffer[(row * boxes.cols) + 0];
                float ycenter = boxes.buffer[(row * boxes.cols) + 1];
                float width = boxes.buffer[(row * boxes.cols) + 2];
                float height = boxes.buffer[(row * boxes.cols) + 3];

                int x = (int)(xcenter - (width / 2.0f));
                int y = (int)(ycenter - (height / 2.0f));

                if (x < 0) {
                    x = 0;
                }
                if (x > (int)impulse->input_width) {
                    x = impulse->input_width;
                }
                if (y < 0) {
                    y = 0;
                }
                if (y > (int)impulse->input_height) {
                    y = impulse->input_height;
                }

                r.x = x;
                r.y = y;
                r.width = (int)round(width);
                r.height = (int)round(height);

                results.push_back(r);
            }
        }
    }

    EI_IMPULSE_ERROR nms_res = ei_run_nms(impulse, &results, debug);
    if (nms_res != EI_IMPULSE_OK) {
        return nms_res;
    }

    // if we didn't detect min required objects, fill the rest with fixed value
    size_t added_boxes_count = results.size();
    size_t min_object_detection_count = impulse->object_detection_count;
    if (added_boxes_count < min_object_detection_count) {
        results.resize(min_object_detection_count);
        for (size_t ix = added_boxes_count; ix < min_object_detection_count; ix++) {
            results[ix].value = 0.0f;
        }
    }

    result->bounding_boxes = results.data();
    result->bounding_boxes_count = results.size();

    return EI_IMPULSE_OK;
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif // EI_HAS_YOLOX
}

/**
  * Fill the result structure from an unquantized output tensor
  * (we don't support quantized here a.t.m.)
  */
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_f32_yolox_detect(const ei_impulse_t *impulse,
                                                                             const ei_learning_block_config_tflite_graph_t *block_config,
                                                                             ei_impulse_result_t *result,
                                                                             float *data,
                                                                             size_t output_features_count) {
#ifdef EI_HAS_YOLOX
    static std::vector<ei_impulse_result_bounding_box_t> results;
    results.clear();

    // expected format [xmin ymin xmax ymax score label]
    const int output_rows = output_features_count / 6;
    matrix_t outputs(output_rows, 6, data);

    // iterate through scores to see if we have anything with confidence
    for (int row = 0; row < (int)outputs.rows; row++) {
        float confidence = outputs.buffer[(row * outputs.cols) + 4];
        int class_idx = (int)outputs.buffer[(row * outputs.cols) + 5];

        if (confidence >= block_config->threshold && confidence <= 1.0f) {
            ei_impulse_result_bounding_box_t r;
            r.label = ei_classifier_inferencing_categories[class_idx];
            r.value = confidence;

            // now find the box...
            float xmin = outputs.buffer[(row * outputs.cols) + 0];
            float ymin = outputs.buffer[(row * outputs.cols) + 1];
            float xmax = outputs.buffer[(row * outputs.cols) + 2];
            float ymax = outputs.buffer[(row * outputs.cols) + 3];

            float width  = xmax - xmin;
            float height = ymax - ymin;

            int x = (int)xmin;
            int y = (int)ymin;

            if (x < 0) {
                x = 0;
            }
            if (x > (int)impulse->input_width) {
                x = impulse->input_width;
            }
            if (y < 0) {
                y = 0;
            }
            if (y > (int)impulse->input_height) {
                y = impulse->input_height;
            }

            r.x = x;
            r.y = y;
            r.width = (int)round(width);
            r.height = (int)round(height);

            results.push_back(r);
        }
    }

    result->bounding_boxes = results.data();
    result->bounding_boxes_count = results.size();

    return EI_IMPULSE_OK;
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif // EI_HAS_YOLOX
}

/**
  * Fill the result structure from an unquantized output tensor
  * (we don't support quantized here a.t.m.)
  */
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_f32_yolov7(const ei_impulse_t *impulse,
                                                                              const ei_learning_block_config_tflite_graph_t *block_config,
                                                                              ei_impulse_result_t *result,
                                                                              float *data,
                                                                              size_t output_features_count) {
#ifdef EI_HAS_YOLOV7
    static std::vector<ei_impulse_result_bounding_box_t> results;
    results.clear();

    size_t col_size = 7;
    size_t row_count = output_features_count / col_size;

    // output is:
    // batch_id, xmin, ymin, xmax, ymax, cls_id, score
    for (size_t ix = 0; ix < row_count; ix++) {
        size_t base_ix = ix * col_size;
        float xmin = data[base_ix + 1];
        float ymin = data[base_ix + 2];
        float xmax = data[base_ix + 3];
        float ymax = data[base_ix + 4];
        uint32_t label = (uint32_t)data[base_ix + 5];
        float score = data[base_ix + 6];

        if (score >= block_config->threshold && score <= 1.0f) {
            ei_impulse_result_bounding_box_t r;
            r.label = ei_classifier_inferencing_categories[label];

            r.x = static_cast<uint32_t>(xmin);
            r.y = static_cast<uint32_t>(ymin);
            r.width = static_cast<uint32_t>(xmax - xmin);
            r.height = static_cast<uint32_t>(ymax - ymin);
            r.value = score;
            results.push_back(r);
        }
    }

    // if we didn't detect min required objects, fill the rest with fixed value
    size_t added_boxes_count = results.size();
    size_t min_object_detection_count = impulse->object_detection_count;
    if (added_boxes_count < min_object_detection_count) {
        results.resize(min_object_detection_count);
        for (size_t ix = added_boxes_count; ix < min_object_detection_count; ix++) {
            results[ix].value = 0.0f;
        }
    }

    result->bounding_boxes = results.data();
    result->bounding_boxes_count = results.size();

    return EI_IMPULSE_OK;
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif // #ifdef EI_HAS_YOLOV7
}

#if (EI_HAS_TAO_DECODE_DETECTIONS == 1) || (EI_HAS_TAO_YOLO == 1)

__attribute__((unused)) static void prepare_tao_results_common(const ei_impulse_t *impulse,
                                                               ei_impulse_result_t *result,
                                                               std::vector<ei_impulse_result_bounding_box_t> *results) {
    #define EI_CLASSIFIER_OBJECT_DETECTION_KEEP_TOPK 200

    // if we didn't detect min required objects, fill the rest with fixed value
    size_t added_boxes_count = results->size();
    size_t object_detection_count = impulse->object_detection_count;
    if (added_boxes_count < object_detection_count) {
        results->resize(object_detection_count);
        for (size_t ix = added_boxes_count; ix < object_detection_count; ix++) {
            (*results)[ix].value = 0.0f;
        }
    }

    // we sort in reverse order accross all classes,
    // since results for each class are pushed to the end.
    std::sort(results->begin(), results->end(), [ ]( const ei_impulse_result_bounding_box_t& lhs, const ei_impulse_result_bounding_box_t& rhs )
    {
        return lhs.value > rhs.value;
    });

    // keep topK
    if (results->size() > EI_CLASSIFIER_OBJECT_DETECTION_KEEP_TOPK) {
        results->erase(results->begin() + EI_CLASSIFIER_OBJECT_DETECTION_KEEP_TOPK, results->end());
    }

    result->bounding_boxes = results->data();
    result->bounding_boxes_count = results->size();
}


#endif

#ifdef EI_HAS_TAO_DECODE_DETECTIONS
/**
 * Fill the result structure from an output tensor
*/
template<typename T>
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_tao_decode_detections_common(const ei_impulse_t *impulse,
                                                                                                ei_impulse_result_t *result,
                                                                                                T *data,
                                                                                                float zero_point,
                                                                                                float scale,
                                                                                                size_t output_features_count,
                                                                                                float threshold,
                                                                                                bool debug = false) {

    size_t col_size = 12 + impulse->label_count + 1;
    size_t row_count = output_features_count / col_size;

    static std::vector<ei_impulse_result_bounding_box_t> results;
    static std::vector<ei_impulse_result_bounding_box_t> class_results;
    results.clear();

    for (size_t cls_idx = 1; cls_idx < (size_t)(impulse->label_count + 1); cls_idx++)  {

        std::vector<float> boxes;
        std::vector<float> scores;
        std::vector<int> classes;
        class_results.clear();

        for (size_t ix = 0; ix < row_count; ix++) {

            float score = (static_cast<float>(data[ix * col_size + cls_idx]) - zero_point) * scale;

            if ((score < threshold) || (score > 1.0f)) {
                continue;
            }

            // # 1. calculate boxes location
            size_t base_ix = ix * col_size + col_size; // references the end of the row

            float r_12 = (static_cast<float>(data[base_ix - 12]) - zero_point) * scale;
            float r_11 = (static_cast<float>(data[base_ix - 11]) - zero_point) * scale;
            float r_10 = (static_cast<float>(data[base_ix - 10]) - zero_point) * scale;
            float r_9  = (static_cast<float>(data[base_ix -  9]) - zero_point) * scale;
            float r_8  = (static_cast<float>(data[base_ix -  8]) - zero_point) * scale;
            float r_7  = (static_cast<float>(data[base_ix -  7]) - zero_point) * scale;
            float r_6  = (static_cast<float>(data[base_ix -  6]) - zero_point) * scale;
            float r_5  = (static_cast<float>(data[base_ix -  5]) - zero_point) * scale;
            float r_4  = (static_cast<float>(data[base_ix -  4]) - zero_point) * scale;
            float r_3  = (static_cast<float>(data[base_ix -  3]) - zero_point) * scale;
            float r_2  = (static_cast<float>(data[base_ix -  2]) - zero_point) * scale;
            float r_1  = (static_cast<float>(data[base_ix -  1]) - zero_point) * scale;

            // cx_pred = y_pred[..., -12]
            // cy_pred = y_pred[..., -11]
            // w_pred = y_pred[..., -10]
            // h_pred = y_pred[..., -9]
            float cx_pred = r_12;
            float cy_pred = r_11;
            float w_pred  = r_10;
            float h_pred  = r_9;

            // w_anchor = y_pred[..., -6] - y_pred[..., -8]
            // h_anchor = y_pred[..., -5] - y_pred[..., -7]
            float w_anchor = r_6 - r_8;
            float h_anchor = r_5 - r_7;

            // cx_anchor = tf.truediv(y_pred[..., -6] + y_pred[..., -8], 2.0)
            // cy_anchor = tf.truediv(y_pred[..., -5] + y_pred[..., -7], 2.0)
            float cx_anchor = (r_6 + r_8) / 2.0f;
            float cy_anchor = (r_5 + r_7) / 2.0f;

            // cx_variance = y_pred[..., -4]
            // cy_variance = y_pred[..., -3]
            float cx_variance = r_4;
            float cy_variance = r_3;

            // variance_w = y_pred[..., -2]
            // variance_h = y_pred[..., -1]
            float variance_w = r_2;
            float variance_h = r_1;

            // # Convert anchor box offsets to image offsets.
            // cx = cx_pred * cx_variance * w_anchor + cx_anchor
            // cy = cy_pred * cy_variance * h_anchor + cy_anchor
            // w = tf.exp(w_pred * variance_w) * w_anchor
            // h = tf.exp(h_pred * variance_h) * h_anchor
            float cx = cx_pred * cx_variance * w_anchor + cx_anchor;
            float cy = cy_pred * cy_variance * h_anchor + cy_anchor;
            float w = exp(w_pred * variance_w) * w_anchor;
            float h = exp(h_pred * variance_h) * h_anchor;

            // # Convert 'centroids' to 'corners'.
            float xmin = cx - (w / 2.0f);
            float ymin = cy - (h / 2.0f);
            float xmax = cx + (w / 2.0f);
            float ymax = cy + (h / 2.0f);

            xmin *= impulse->input_width;
            ymin *= impulse->input_height;
            xmax *= impulse->input_width;
            ymax *= impulse->input_height;

            boxes.push_back(ymin);
            boxes.push_back(xmin);
            boxes.push_back(ymax);
            boxes.push_back(xmax);
            scores.push_back(score);
            classes.push_back((int)(cls_idx-1));
        }

        size_t nr_boxes = scores.size();
        EI_IMPULSE_ERROR nms_res = ei_run_nms(impulse, &class_results,
                                              boxes.data(), scores.data(), classes.data(),
                                              nr_boxes,
                                              true /*clip_boxes*/,
                                              debug);

        if (nms_res != EI_IMPULSE_OK) {
            return nms_res;
        }

        for (auto bb: class_results) {
            results.push_back(bb);
        }
    }

    prepare_tao_results_common(impulse, result, &results);

    return EI_IMPULSE_OK;
}
#endif // #ifdef EI_HAS_TAO_DETECT_DETECTIONS

/**
 * Fill the result structure from a quantized output tensor
*/
template<typename T>
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_quantized_tao_decode_detections(const ei_impulse_t *impulse,
                                                                                                   const ei_learning_block_config_tflite_graph_t *block_config,
                                                                                                   ei_impulse_result_t *result,
                                                                                                   T *data,
                                                                                                   float zero_point,
                                                                                                   float scale,
                                                                                                   size_t output_features_count,
                                                                                                   bool debug = false) {
#ifdef EI_HAS_TAO_DECODE_DETECTIONS
    return fill_result_struct_tao_decode_detections_common(impulse, result, data, zero_point, scale, output_features_count, block_config->threshold, debug);
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif // #ifdef EI_HAS_TAO_DETECT_DETECTIONS
}


/**
  * Fill the result structure from an unquantized output tensor
  */
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_f32_tao_decode_detections(const ei_impulse_t *impulse,
                                                                                     const ei_learning_block_config_tflite_graph_t *block_config,
                                                                                     ei_impulse_result_t *result,
                                                                                     float *data,
                                                                                     size_t output_features_count,
                                                                                     bool debug = false) {
#ifdef EI_HAS_TAO_DECODE_DETECTIONS
    return fill_result_struct_tao_decode_detections_common(impulse, result, data, 0.0f, 1.0f, output_features_count, block_config->threshold, debug);
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif // #ifdef EI_HAS_TAO_DETECT_DETECTIONS
}

#ifdef EI_HAS_TAO_YOLOV3
/**
 * Fill the result structure from an output tensor
*/
template<typename T>
__attribute__((unused)) static EI_IMPULSE_ERROR  fill_result_struct_tao_yolov3_common(const ei_impulse_t *impulse,
                                                                         ei_impulse_result_t *result,
                                                                         T *data,
                                                                         float zero_point,
                                                                         float scale,
                                                                         size_t output_features_count,
                                                                         float threshold,
                                                                         bool debug) {
    // # x: 3-D tensor. Last dimension is
    //          (cy, cx, ph, pw, step_y, step_x, pred_y, pred_x, pred_h, pred_w, object, cls...)
    size_t col_size = 11 + impulse->label_count;
    size_t row_count = output_features_count / col_size;

    static std::vector<ei_impulse_result_bounding_box_t> results;
    static std::vector<ei_impulse_result_bounding_box_t> class_results;

    results.clear();
    for (size_t cls_idx = 0; cls_idx < (size_t)impulse->label_count; cls_idx++)  {

        std::vector<float> boxes;
        std::vector<float> scores;
        std::vector<int> classes;
        class_results.clear();

        for (size_t ix = 0; ix < row_count; ix++) {
            size_t data_ix = ix * col_size;
            float r_0  = (static_cast<float>(data[data_ix +  0]) - zero_point) * scale;
            float r_1  = (static_cast<float>(data[data_ix +  1]) - zero_point) * scale;
            float r_2  = (static_cast<float>(data[data_ix +  2]) - zero_point) * scale;
            float r_3  = (static_cast<float>(data[data_ix +  3]) - zero_point) * scale;
            float r_4  = (static_cast<float>(data[data_ix +  4]) - zero_point) * scale;
            float r_5  = (static_cast<float>(data[data_ix +  5]) - zero_point) * scale;
            float r_6  = (static_cast<float>(data[data_ix +  6]) - zero_point) * scale;
            float r_7  = (static_cast<float>(data[data_ix +  7]) - zero_point) * scale;
            float r_8  = (static_cast<float>(data[data_ix +  8]) - zero_point) * scale;
            float r_9  = (static_cast<float>(data[data_ix +  9]) - zero_point) * scale;
            float r_10 = (static_cast<float>(data[data_ix + 10]) - zero_point) * scale;

            float cls = (static_cast<float>(data[data_ix + 11 + cls_idx]) - zero_point) * scale;
            float score = sigmoid(cls) * sigmoid(r_10);

            if ((score < threshold) || (score > 1.0f)) {
                continue;
            }

            float by = r_0 + sigmoid(r_6) * r_4;
            float bx = r_1 + sigmoid(r_7) * r_5;
            float bh = r_2 * exp(r_8);
            float bw = r_3 * exp(r_9);

            float ymin = by - 0.5 * bh;
            float xmin = bx - 0.5 * bw;
            float ymax = by + 0.5 * bh;
            float xmax = bx + 0.5 * bw;

            // from relative to absolute
            ymin *= impulse->input_height;
            xmin *= impulse->input_width;
            ymax *= impulse->input_height;
            xmax *= impulse->input_width;

            boxes.push_back(ymin);
            boxes.push_back(xmin);
            boxes.push_back(ymax);
            boxes.push_back(xmax);
            scores.push_back(score);
            classes.push_back((int)cls_idx);
        }

        size_t nr_boxes = scores.size();
        EI_IMPULSE_ERROR nms_res = ei_run_nms(impulse, &class_results,
                                              boxes.data(), scores.data(), classes.data(),
                                              nr_boxes,
                                              true /*clip_boxes*/,
                                              debug);
        if (nms_res != EI_IMPULSE_OK) {
            return nms_res;
        }

        for (auto bb: class_results) {
            results.push_back(bb);
        }
    }

    prepare_tao_results_common(impulse, result, &results);
    return EI_IMPULSE_OK;
}
#endif // #ifdef EI_HAS_TAO_YOLOV3

#ifdef EI_HAS_TAO_YOLOV4
/**
 * Fill the result structure from an output tensor
*/
template<typename T>
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_tao_yolov4_common(const ei_impulse_t *impulse,
                                                                         ei_impulse_result_t *result,
                                                                         T *data,
                                                                         float zero_point,
                                                                         float scale,
                                                                         size_t output_features_count,
                                                                         float threshold,
                                                                         bool debug) {
    // # x: 3-D tensor. Last dimension is
    //          (cy, cx, ph, pw, step_y, step_x, pred_y, pred_x, pred_h, pred_w, object, cls...)
    size_t col_size = 11 + impulse->label_count;
    size_t row_count = output_features_count / col_size;

    static std::vector<ei_impulse_result_bounding_box_t> results;
    static std::vector<ei_impulse_result_bounding_box_t> class_results;
    results.clear();

    const float grid_scale_xy = 1.0f;

    for (size_t cls_idx = 0; cls_idx < (size_t)impulse->label_count; cls_idx++)  {

        std::vector<float> boxes;
        std::vector<float> scores;
        std::vector<int> classes;
        class_results.clear();

        for (size_t ix = 0; ix < row_count; ix++) {

            float r_0  = (static_cast<float>(data[ix * col_size +  0]) - zero_point) * scale;
            float r_1  = (static_cast<float>(data[ix * col_size +  1]) - zero_point) * scale;
            float r_2  = (static_cast<float>(data[ix * col_size +  2]) - zero_point) * scale;
            float r_3  = (static_cast<float>(data[ix * col_size +  3]) - zero_point) * scale;
            float r_4  = (static_cast<float>(data[ix * col_size +  4]) - zero_point) * scale;
            float r_5  = (static_cast<float>(data[ix * col_size +  5]) - zero_point) * scale;
            float r_6  = (static_cast<float>(data[ix * col_size +  6]) - zero_point) * scale;
            float r_7  = (static_cast<float>(data[ix * col_size +  7]) - zero_point) * scale;
            float r_8  = (static_cast<float>(data[ix * col_size +  8]) - zero_point) * scale;
            float r_9  = (static_cast<float>(data[ix * col_size +  9]) - zero_point) * scale;
            float r_10 = (static_cast<float>(data[ix * col_size + 10]) - zero_point) * scale;

            float cls = (static_cast<float>(data[ix * col_size + 11 + cls_idx]) - zero_point) * scale;
            float score = sigmoid(cls) * sigmoid(r_10);

            if ((score < threshold) || (score > 1.0f)) {
                continue;
            }

            float pred_y = sigmoid(r_6) * grid_scale_xy - (grid_scale_xy - 1.0f) / 2.0f;
            float pred_x = sigmoid(r_7) * grid_scale_xy - (grid_scale_xy - 1.0f) / 2.0f;
            float pred_h = exp(std::min(r_8, 8.0f));
            float pred_w = exp(std::min(r_9, 8.0f));

            r_6 = pred_y;
            r_7 = pred_x;
            r_8 = pred_h;
            r_9 = pred_w;

            float by = r_0 + r_6 * r_4;
            float bx = r_1 + r_7 * r_5;
            float bh = r_2 * r_8;
            float bw = r_3 * r_9;

            float ymin = by - 0.5 * bh;
            float xmin = bx - 0.5 * bw;
            float ymax = by + 0.5 * bh;
            float xmax = bx + 0.5 * bw;

            // from relative to absolute
            ymin *= impulse->input_height;
            xmin *= impulse->input_width;
            ymax *= impulse->input_height;
            xmax *= impulse->input_width;

            boxes.push_back(ymin);
            boxes.push_back(xmin);
            boxes.push_back(ymax);
            boxes.push_back(xmax);
            scores.push_back(score);
            classes.push_back((int)cls_idx);
        }

        size_t nr_boxes = scores.size();
        EI_IMPULSE_ERROR nms_res = ei_run_nms(impulse, &class_results,
                                              boxes.data(), scores.data(), classes.data(),
                                              nr_boxes,
                                              true /*clip_boxes*/,
                                              debug);
        if (nms_res != EI_IMPULSE_OK) {
            return nms_res;
        }

        for (auto bb: class_results) {
            results.push_back(bb);
        }
    }

    prepare_tao_results_common(impulse, result, &results);
    return EI_IMPULSE_OK;
}
#endif // #ifdef EI_HAS_TAO_YOLOV4

/**
  * Fill the result structure from an unquantized output tensor
  */
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_f32_tao_yolov3(const ei_impulse_t *impulse,
                                                                                const ei_learning_block_config_tflite_graph_t *block_config,
                                                                                ei_impulse_result_t *result,
                                                                                float *data,
                                                                                size_t output_features_count,
                                                                                bool debug = false) {
#ifdef EI_HAS_TAO_YOLOV3
    return fill_result_struct_tao_yolov3_common(impulse, result, data, 0.0f, 1.0f, output_features_count, block_config->threshold, debug);
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif // #ifdef EI_HAS_TAO_YOLOV3
}

/**
 * Fill the result structure from a quantized output tensor
*/
template<typename T>
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_quantized_tao_yolov3(const ei_impulse_t *impulse,
                                                                                      const ei_learning_block_config_tflite_graph_t *block_config,
                                                                                      ei_impulse_result_t *result,
                                                                                      T *data,
                                                                                      float zero_point,
                                                                                      float scale,
                                                                                      size_t output_features_count,
                                                                                      bool debug = false) {
#ifdef EI_HAS_TAO_YOLOV3
    return fill_result_struct_tao_yolov3_common(impulse, result, data, zero_point, scale, output_features_count, block_config->threshold, debug);
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif // #ifdef EI_HAS_TAO_YOLOV3
}

/**
  * Fill the result structure from an unquantized output tensor
  */
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_f32_tao_yolov4(const ei_impulse_t *impulse,
                                                                                const ei_learning_block_config_tflite_graph_t *block_config,
                                                                                ei_impulse_result_t *result,
                                                                                float *data,
                                                                                size_t output_features_count,
                                                                                bool debug = false) {
#ifdef EI_HAS_TAO_YOLOV4
    return fill_result_struct_tao_yolov4_common(impulse, result, data, 0.0f, 1.0f, output_features_count, block_config->threshold, debug);
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif // #ifdef EI_HAS_TAO_YOLOV4
}

/**
 * Fill the result structure from a quantized output tensor
*/
template<typename T>
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_quantized_tao_yolov4(const ei_impulse_t *impulse,
                                                                                      const ei_learning_block_config_tflite_graph_t *block_config,
                                                                                      ei_impulse_result_t *result,
                                                                                      T *data,
                                                                                      float zero_point,
                                                                                      float scale,
                                                                                      size_t output_features_count,
                                                                                      bool debug = false) {
#ifdef EI_HAS_TAO_YOLOV4
    return fill_result_struct_tao_yolov4_common(impulse, result, data, zero_point, scale, output_features_count, block_config->threshold, debug);
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif // #ifdef EI_HAS_TAO_YOLOV4
}

#ifdef EI_HAS_YOLOV2
// based on akida_models-1.2.0/detection/processing.py
// input is "2D" array with shape [grid_h * grid_w * nb_box, nb_classes]
__attribute__((unused)) static void softmax(std::vector<float>& input, const size_t nb_classes)
{
    const float max = *std::max_element(input.begin(), input.end());
    const float min = *std::min_element(input.begin(), input.end());
    const float t = -100.0f;

    // x = x - np.max(x)
    std::transform(input.begin(), input.end(), input.begin(),
                   [max](float x) { return x - max; });

    // if np.min(x) < t: x = x / np.min(x) * t
    std::transform(input.begin(), input.end(), input.begin(),
                   [min, t](float x) { return x < t ? (x / min * t): x; });

    // e_x = np.exp(x)
    // do it in place as we don't need raw the input anymore
    std::transform(input.begin(), input.end(), input.begin(),
                   [](float x) { return std::exp(x); });

    // e_x / e_x.sum(axis, keepdims=True)
    // calculated for each 'row', across nb_classes
    for(auto it = input.begin(); it != input.end(); it += nb_classes) {
        float sum = 0.0f;
        // e_x.sum(axis, keepdims=True)
        for(auto it2 = it; it2 != it + nb_classes; it2++) {
            sum += *it2;
        }
        // e_x / e_x.sum(axis, keepdims=True)
        std::transform(it, it + nb_classes, it,
                       [sum](float ex) { return ex / sum; });
    }
}

class BoundingBox {
public:
    float x1, y1, x2, y2, confidence;
    std::vector<float> classes;

    BoundingBox(float x1, float y1, float x2, float y2, float confidence, const std::vector<float>& classes)
        : x1(x1), y1(y1), x2(x2), y2(y2), confidence(confidence), classes(classes) {}

    float get_score() const {
        return confidence;
    }

    int get_label() const {
        auto maxElementIndex = std::max_element(classes.begin(), classes.end()) - classes.begin();
        return maxElementIndex;
    }

    float _interval_overlap(float x1, float x2, float x3, float x4) const {
        if(x3 < x1) {
            if(x4 < x1) {
                return 0;
            }
            return std::min(x2, x4) - x1;
        }
        if(x2 < x3) {
            return 0;
        }
        return std::min(x2, x4) - x3;
    }


    float iou(const BoundingBox& other) const {
        // Implementation of the Intersection over Union calculation
        float intersect_w = this->_interval_overlap(this->x1, this->x2, other.x1, other.x2);
        float intersect_h = this->_interval_overlap(this->y1, this->y2, other.y1, other.y2);

        float intersect = intersect_w * intersect_h;

        float w1 = this->x2 - this->x1;
        float h1 = this->y2 - this->y1;
        float w2 = other.x2 - other.x1;
        float h2 = other.y2 - other.y1;

        float un = w1 * h1 + w2 * h2 - intersect;

        return float(intersect) / un;
    }
};
#endif // EI_HAS_YOLOV2
/**
  * Fill the result structure from an unquantized output tensor
  */
__attribute__((unused)) static EI_IMPULSE_ERROR fill_result_struct_f32_yolov2(const ei_impulse_t *impulse,
                                                                              const ei_learning_block_config_tflite_graph_t *block_config,
                                                                              ei_impulse_result_t *result,
                                                                              float *data,
                                                                              size_t output_features_count,
                                                                              bool debug = false) {
#ifdef EI_HAS_YOLOV2
    static std::vector<ei_impulse_result_bounding_box_t> results;
    results.clear();

    // Example output shape: (7, 7, 5, 7)
    // TODO: calculate grid_h, grid_w, nb_box from output_features_count or get as a param
    // grid_h, grid_w, nb_box = output.shape[:3]
    const size_t grid_h = 7;
    const size_t grid_w = 7;
    const size_t nb_box = 5;
    const std::vector<std::pair<float, float>> anchors = {{0.56594, 1.05012}, {1.0897, 2.03908}, {2.37823, 3.00376}, {2.4593, 4.913}, {5.15981, 5.56699}};

    const size_t nb_classes = impulse->label_count;
    const float obj_threshold = 0.5;
    const float nms_threshold = 0.5;
    std::vector<float> output;
    const int stride = 4 + 1 + nb_classes;

    output.assign(data, data + output_features_count);

    // boxes = []
    std::vector<BoundingBox> boxes;

    // equivalent to: classes_confidences = output[..., 5:]
    std::vector<float> classes_confidences;
    const size_t dim = 5;
    for(auto it = output.begin() + dim; it <= output.end(); it += (dim + nb_classes)) {
        classes_confidences.insert(classes_confidences.end(), it, it + nb_classes);
    }
    // calculate softmax for later use, we need to calculate it across the whole input data so operate on a sliced output
    softmax(classes_confidences, nb_classes);

    for (size_t row = 0; row < grid_h; ++row) {
        for (size_t col = 0; col < grid_w; ++col) {
            for (size_t b = 0; b < nb_box; ++b) {
                size_t idx = row * grid_w * nb_box * stride + col * nb_box * stride + b * stride;
                size_t classes_idx = row * grid_w * nb_box * nb_classes + col * nb_box * nb_classes + b * nb_classes;

                // Apply sigmoid to the 4th element
                // output[..., 4] = _sigmoid(output[..., 4])
                float sigmoid_val = sigmoid(output[idx + 4]);
                output[idx + 4] = sigmoid_val;

                // classes = output[row, col, b, 5:]
                std::vector<float> classes(classes_confidences.begin() + classes_idx, classes_confidences.begin() + classes_idx + nb_classes);

                // output[..., 5:] = output[..., 4][..., np.newaxis] * _softmax(output[..., 5:])
                // output[..., 5:] *= output[..., 5:] > obj_threshold
                std::transform(classes.begin(), classes.end(), classes.begin(),
                               [sigmoid_val, obj_threshold](float c) { c *= sigmoid_val; return c > obj_threshold ? c : 0.0f; });

                // if np.sum(classes) > 0:
                float sum = 0.0f;
                for(auto it = classes.begin(); it != classes.end(); it++) {
                    sum += *it;
                }
                if(sum > 0.0f) {
                    // x, y, w, h = output[row, col, b, :4]
                    float x = output[idx + 0];
                    float y = output[idx + 1];
                    float w = output[idx + 2];
                    float h = output[idx + 3];

                    // x = (col + _sigmoid(x)) / grid_w  # center position, unit: image width
                    x = (col + sigmoid(x)) / grid_w;
                    // y = (row + _sigmoid(y)) / grid_h  # center position, unit: image height
                    y = (row + sigmoid(y)) / grid_h;
                    // w = anchors[b][0] * np.exp(w) / grid_w  # unit: image width
                    w = anchors[b].first * std::exp(w) / grid_w;
                    // h = anchors[b][1] * np.exp(h) / grid_h  # unit: image height
                    h = anchors[b].second * std::exp(h) / grid_h;

                    // confidence = output[row, col, b, 4]
                    float confidence = output[idx + 4];

                    // x1 = max(x - w / 2, 0)
                    float x1 = std::max(x - w / 2, 0.0f);
                    // y1 = max(y - h / 2, 0)
                    float y1 = std::max(y - h / 2, 0.0f);
                    // x2 = min(x + w / 2, grid_w)
                    float x2 = std::min(x + w / 2, static_cast<float>(grid_w));
                    // y2 = min(y + h / 2, grid_h)
                    float y2 = std::min(y + h / 2, static_cast<float>(grid_h));

                    boxes.emplace_back(x1, y1, x2, y2, confidence, classes);
                }
            }
        }
    }

    // Non-maximal suppression (on boxes)
    for (size_t c = 0; c < nb_classes; ++c) {
        std::vector<std::pair<float, int>> sorted_indices;
        for (size_t i = 0; i < boxes.size(); ++i) {
            sorted_indices.emplace_back(boxes[i].classes[c], i);
        }

        std::sort(sorted_indices.begin(), sorted_indices.end(),
                  [](const std::pair<float, int>& a, const std::pair<float, int>& b) {
                      return a.first > b.first;
                  });

        for (size_t i = 0; i < sorted_indices.size(); ++i) {
            int index_i = sorted_indices[i].second;
            if (boxes[index_i].classes[c] == 0)
                continue;

            for (size_t j = i + 1; j < sorted_indices.size(); ++j) {
                int index_j = sorted_indices[j].second;

                if ((boxes[index_i].iou(boxes[index_j]) >= nms_threshold) &&
                    (boxes[index_i].get_label() == (int)c) &&
                    (boxes[index_j].get_label() == (int)c)) {
                    boxes[index_j].confidence = 0;
                }
            }
        }
    }

    // remove the boxes which are less likely than a obj_threshold
    boxes.erase(std::remove_if(boxes.begin(), boxes.end(),
                               [obj_threshold](const BoundingBox& box) {
                                   return box.get_score() <= obj_threshold;
                               }), boxes.end());

    // sort boxes by box.get_score()
    std::sort(boxes.begin(), boxes.end(),
                [](const BoundingBox& a, const BoundingBox& b) {
                return a.get_score() > b.get_score();
                });

    // convert relative coordinates to absolute coordinates
    for(auto & box: boxes) {
        ei_impulse_result_bounding_box_t res;
        res.label = ei_classifier_inferencing_categories[box.get_label()];
        res.x = ceil(box.x1 * impulse->input_width);
        res.y = ceil(box.y1 * impulse->input_height);
        res.width = ceil((box.x2 - box.x1) * impulse->input_width);
        res.height = ceil((box.y2 - box.y1) * impulse->input_height);
        res.value = box.get_score();
        results.push_back(res);
    }

    // if we didn't detect min required objects, fill the rest with fixed value
    size_t added_boxes_count = results.size();
    size_t min_object_detection_count = impulse->object_detection_count;
    if (added_boxes_count < min_object_detection_count) {
        results.resize(min_object_detection_count);
        for (size_t ix = added_boxes_count; ix < min_object_detection_count; ix++) {
            results[ix].value = 0.0f;
        }
    }

    result->bounding_boxes = results.data();
    result->bounding_boxes_count = results.size();

    return EI_IMPULSE_OK;
#else
    return EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE;
#endif // #ifdef EI_HAS_YOLOV7
}

#if EI_CLASSIFIER_SINGLE_FEATURE_INPUT == 0
bool find_mtx_by_idx(ei_feature_t* mtx, ei::matrix_t** matrix, uint32_t mtx_id, size_t mtx_size) {
    for (size_t i = 0; i < mtx_size; i++) {
        if (mtx[i].matrix == NULL) {
            continue;
        }
        if (mtx[i].blockId == mtx_id || mtx[i].blockId == 0) {
            *matrix = mtx[i].matrix;
            return true;
        }
    }
    return false;
}
#endif

#endif // _EI_CLASSIFIER_FILL_RESULT_STRUCT_H_
