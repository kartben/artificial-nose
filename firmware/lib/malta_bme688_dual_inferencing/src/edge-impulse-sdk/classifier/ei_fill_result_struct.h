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

#ifndef _EI_CLASSIFIER_FILL_RESULT_STRUCT_H_
#define _EI_CLASSIFIER_FILL_RESULT_STRUCT_H_

#include "model-parameters/model_metadata.h"
#if EI_CLASSIFIER_HAS_MODEL_VARIABLES == 1
#include "model-parameters/model_variables.h"
#endif

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
    if(added_boxes_count < object_detection_count) {
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

__attribute__((unused)) static void fill_result_struct_f32_fomo(const ei_impulse_t *impulse, ei_impulse_result_t *result, float *data, int out_width, int out_height) {
    std::vector<ei_classifier_cube_t*> cubes;

    int out_width_factor = impulse->input_width / out_width;

    for (size_t y = 0; y < out_width; y++) {
        // ei_printf("    [ ");
        for (size_t x = 0; x < out_height; x++) {
            size_t loc = ((y * out_height) + x) * (impulse->label_count + 1);

            for (size_t ix = 1; ix < impulse->label_count + 1; ix++) {
                float vf = data[loc+ix];

                ei_handle_cube(&cubes, x, y, vf, impulse->categories[ix - 1], impulse->object_detection_threshold);
            }
        }
    }

    fill_result_struct_from_cubes(result, &cubes, out_width_factor, impulse->object_detection_count);
}

__attribute__((unused)) static void fill_result_struct_i8_fomo(const ei_impulse_t *impulse, ei_impulse_result_t *result, int8_t *data, float zero_point, float scale, int out_width, int out_height) {
    std::vector<ei_classifier_cube_t*> cubes;

    int out_width_factor = impulse->input_width / out_width;

    for (size_t y = 0; y < out_width; y++) {
        // ei_printf("    [ ");
        for (size_t x = 0; x < out_height; x++) {
            size_t loc = ((y * out_height) + x) * (impulse->label_count + 1);

            for (size_t ix = 1; ix < impulse->label_count + 1; ix++) {
                int8_t v = data[loc+ix];
                float vf = static_cast<float>(v - zero_point) * scale;

                ei_handle_cube(&cubes, x, y, vf, impulse->categories[ix - 1], impulse->object_detection_threshold);
            }
        }
    }

    fill_result_struct_from_cubes(result, &cubes, out_width_factor, impulse->object_detection_count);
}

/**
 * Fill the result structure from an unquantized output tensor
 * (we don't support quantized here a.t.m.)
 */
__attribute__((unused)) static void fill_result_struct_f32_object_detection(const ei_impulse_t *impulse, ei_impulse_result_t *result, float *data, float *scores, float *labels, bool debug) {
    static std::vector<ei_impulse_result_bounding_box_t> results;
    results.clear();
    results.resize(impulse->object_detection_count);
    for (size_t ix = 0; ix < impulse->object_detection_count; ix++) {

        float score = scores[ix];
        float label = labels[ix];

        if (score >= impulse->object_detection_threshold) {
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
                    impulse->categories[(uint32_t)label], label, score, xstart, ystart, xend, yend);
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
}

/**
 * Fill the result structure from a quantized output tensor
 */
__attribute__((unused)) static void fill_result_struct_i8(const ei_impulse_t *impulse, ei_impulse_result_t *result, int8_t *data, float zero_point, float scale, bool debug) {
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
}

/**
 * Fill the result structure from an unquantized output tensor
 */
__attribute__((unused)) static void fill_result_struct_f32(const ei_impulse_t *impulse, ei_impulse_result_t *result, float *data, bool debug) {
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
}

/**
  * Fill the result structure from an unquantized output tensor
  * (we don't support quantized here a.t.m.)
  */
__attribute__((unused)) static void fill_result_struct_f32_yolov5(const ei_impulse_t *impulse, ei_impulse_result_t *result, float *data, size_t output_features_count) {
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

        if (score >= impulse->object_detection_threshold && score <= 1.0f) {
            ei_impulse_result_bounding_box_t r;
            r.label = ei_classifier_inferencing_categories[label];
            r.x = static_cast<uint32_t>(x * static_cast<float>(impulse->input_width));
            r.y = static_cast<uint32_t>(y * static_cast<float>(impulse->input_height));
            r.width = static_cast<uint32_t>(w * static_cast<float>(impulse->input_width));
            r.height = static_cast<uint32_t>(h * static_cast<float>(impulse->input_height));
            r.value = score;
            results.push_back(r);
        }
    }

    result->bounding_boxes = results.data();
    result->bounding_boxes_count = results.size();
}


#endif // _EI_CLASSIFIER_FILL_RESULT_STRUCT_H_
