// Patched by Edge Impulse to include reference and hardware-accelerated kernels
#include "../../../../classifier/ei_classifier_config.h"
#if 0 == 1
/* noop */
#elif EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN == 1
/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/add.h"

#include "edge-impulse-sdk/CMSIS/NN/Include/arm_nnfunctions.h"
#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/quantization_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/integer_ops/add.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/process_broadcast_shapes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/op_macros.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/memory_helpers.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_log.h"

namespace tflite {

namespace {
constexpr int kInputTensor1 = 0;
constexpr int kInputTensor2 = 1;
constexpr int kOutputTensor = 0;

struct OpData {
  bool requires_broadcast;

  // These fields are used in both the general 8-bit -> 8bit quantized path,
  // and the special 16-bit -> 16bit quantized path
  int input1_shift;
  int input2_shift;
  int32_t output_activation_min;
  int32_t output_activation_max;

  // These fields are used only in the general 8-bit -> 8bit quantized path
  int32_t input1_multiplier;
  int32_t input2_multiplier;
  int32_t output_multiplier;

  int output_shift;
  int left_shift;

  int32_t input1_offset;
  int32_t input2_offset;
  int32_t output_offset;

  // Used only for float evals:
  float output_activation_min_f32;
  float output_activation_max_f32;
};

TfLiteStatus CalculateOpData(TfLiteContext* context, TfLiteAddParams* params,
                             const TfLiteTensor* input1,
                             const TfLiteTensor* input2, TfLiteTensor* output,
                             OpData* data) {
  data->requires_broadcast = !HaveSameShapes(input1, input2);

  if (output->type == kTfLiteInt8 || output->type == kTfLiteInt16) {
    // 8bit -> 8bit general quantized path, with general rescalings
    data->input1_offset = -input1->params.zero_point;
    data->input2_offset = -input2->params.zero_point;
    data->output_offset = output->params.zero_point;
    data->left_shift = (output->type == kTfLiteInt16) ? 15 : 20;
    const double twice_max_input_scale =
        2 * static_cast<double>(
                std::max(input1->params.scale, input2->params.scale));
    const double real_input1_multiplier =
        static_cast<double>(input1->params.scale) / twice_max_input_scale;
    const double real_input2_multiplier =
        static_cast<double>(input2->params.scale) / twice_max_input_scale;
    const double real_output_multiplier =
        twice_max_input_scale /
        ((1 << data->left_shift) * static_cast<double>(output->params.scale));

    QuantizeMultiplierSmallerThanOneExp(
        real_input1_multiplier, &data->input1_multiplier, &data->input1_shift);

    QuantizeMultiplierSmallerThanOneExp(
        real_input2_multiplier, &data->input2_multiplier, &data->input2_shift);

    QuantizeMultiplierSmallerThanOneExp(
        real_output_multiplier, &data->output_multiplier, &data->output_shift);

    TF_LITE_ENSURE_STATUS(CalculateActivationRangeQuantized(
        context, params->activation, output, &data->output_activation_min,
        &data->output_activation_max));
  } else if (output->type == kTfLiteFloat32) {
    CalculateActivationRange(params->activation,
                             &data->output_activation_min_f32,
                             &data->output_activation_max_f32);
  }

  return kTfLiteOk;
}

void UpdateOpParams(tflite::ArithmeticParams* const op_params,
                    const OpData* data) {
  op_params->left_shift = data->left_shift;
  op_params->input1_offset = data->input1_offset;
  op_params->input1_multiplier = data->input1_multiplier;
  op_params->input1_shift = data->input1_shift;
  op_params->input2_offset = data->input2_offset;
  op_params->input2_multiplier = data->input2_multiplier;
  op_params->input2_shift = data->input2_shift;
  op_params->output_offset = data->output_offset;
  op_params->output_multiplier = data->output_multiplier;
  op_params->output_shift = data->output_shift;
  SetActivationParams(data->output_activation_min, data->output_activation_max,
                      op_params);
}

TfLiteStatus EvalAddQuantizedInt8(TfLiteContext* context, TfLiteNode* node,
                                  TfLiteAddParams* params, const OpData* data,
                                  const TfLiteEvalTensor* input1,
                                  const TfLiteEvalTensor* input2,
                                  TfLiteEvalTensor* output) {
  tflite::ArithmeticParams op_params;
  UpdateOpParams(&op_params, data);

  bool need_broadcast = reference_ops::ProcessBroadcastShapes(
      tflite::micro::GetTensorShape(input1),
      tflite::micro::GetTensorShape(input2), &op_params);

  if (need_broadcast) {
    reference_integer_ops::BroadcastAdd4DSlow(
        op_params, tflite::micro::GetTensorShape(input1),
        tflite::micro::GetTensorData<int8_t>(input1),
        tflite::micro::GetTensorShape(input2),
        tflite::micro::GetTensorData<int8_t>(input2),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<int8_t>(output));
  } else {
    arm_elementwise_add_s8(
        tflite::micro::GetTensorData<int8_t>(input1),

        tflite::micro::GetTensorData<int8_t>(input2), op_params.input1_offset,
        op_params.input1_multiplier, op_params.input1_shift,
        op_params.input2_offset, op_params.input2_multiplier,
        op_params.input2_shift, op_params.left_shift,
        tflite::micro::GetTensorData<int8_t>(output), op_params.output_offset,
        op_params.output_multiplier, op_params.output_shift,
        op_params.quantized_activation_min, op_params.quantized_activation_max,
        MatchingElementsSize(tflite::micro::GetTensorShape(input1),
                             tflite::micro::GetTensorShape(input2),
                             tflite::micro::GetTensorShape(output)));
  }

  return kTfLiteOk;
}

TfLiteStatus EvalAddQuantizedInt16(TfLiteContext* context, TfLiteNode* node,
                                   TfLiteAddParams* params, const OpData* data,
                                   const TfLiteEvalTensor* input1,
                                   const TfLiteEvalTensor* input2,
                                   TfLiteEvalTensor* output) {
  tflite::ArithmeticParams op_params;
  UpdateOpParams(&op_params, data);

  bool need_broadcast = reference_ops::ProcessBroadcastShapes(
      tflite::micro::GetTensorShape(input1),
      tflite::micro::GetTensorShape(input2), &op_params);

  if (need_broadcast) {
    reference_ops::BroadcastAdd4DSlow(
        op_params, tflite::micro::GetTensorShape(input1),
        tflite::micro::GetTensorData<int16_t>(input1),
        tflite::micro::GetTensorShape(input2),
        tflite::micro::GetTensorData<int16_t>(input2),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<int16_t>(output));
  } else {
    arm_elementwise_add_s16(
        tflite::micro::GetTensorData<int16_t>(input1),
        tflite::micro::GetTensorData<int16_t>(input2), op_params.input1_offset,
        op_params.input1_multiplier, op_params.input1_shift,
        op_params.input2_offset, op_params.input2_multiplier,
        op_params.input2_shift, op_params.left_shift,
        tflite::micro::GetTensorData<int16_t>(output), op_params.output_offset,
        op_params.output_multiplier, op_params.output_shift,
        op_params.quantized_activation_min, op_params.quantized_activation_max,
        MatchingElementsSize(tflite::micro::GetTensorShape(input1),
                             tflite::micro::GetTensorShape(input2),
                             tflite::micro::GetTensorShape(output)));
  }

  return kTfLiteOk;
}

void EvalAddFloat(TfLiteContext* context, TfLiteNode* node,
                  TfLiteAddParams* params, const OpData* data,
                  const TfLiteEvalTensor* input1,
                  const TfLiteEvalTensor* input2, TfLiteEvalTensor* output) {
  tflite::ArithmeticParams op_params;
  SetActivationParams(data->output_activation_min_f32,
                      data->output_activation_max_f32, &op_params);
  if (data->requires_broadcast) {
    reference_ops::BroadcastAdd4DSlow(
        op_params, tflite::micro::GetTensorShape(input1),
        tflite::micro::GetTensorData<float>(input1),
        tflite::micro::GetTensorShape(input2),
        tflite::micro::GetTensorData<float>(input2),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<float>(output));
  } else {
    reference_ops::Add(op_params, tflite::micro::GetTensorShape(input1),
                       tflite::micro::GetTensorData<float>(input1),
                       tflite::micro::GetTensorShape(input2),
                       tflite::micro::GetTensorData<float>(input2),
                       tflite::micro::GetTensorShape(output),
                       tflite::micro::GetTensorData<float>(output));
  }
}

TfLiteStatus EvalAddQuantized(TfLiteContext* context, TfLiteNode* node,
                              TfLiteAddParams* params, const OpData* data,
                              const TfLiteEvalTensor* input1,
                              const TfLiteEvalTensor* input2,
                              TfLiteEvalTensor* output) {
  switch (output->type) {
    case kTfLiteInt8: {
      EvalAddQuantizedInt8(context, node, params, data, input1, input2, output);
      break;
    }
    case kTfLiteInt16: {
      EvalAddQuantizedInt16(context, node, params, data, input1, input2,
                            output);
      break;
    }
    default:
      MicroPrintf("Type %s (%d) not supported.",
                  TfLiteTypeGetName(output->type), output->type);
      return kTfLiteError;
  }

  return kTfLiteOk;
}

}  // namespace

void* InitAdd(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpData));
}

TfLiteStatus PrepareAdd(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  MicroContext* micro_context = GetMicroContext(context);

  TfLiteTensor* input1 =
      micro_context->AllocateTempInputTensor(node, kInputTensor1);
  TF_LITE_ENSURE(context, input1 != nullptr);
  TfLiteTensor* input2 =
      micro_context->AllocateTempInputTensor(node, kInputTensor2);
  TF_LITE_ENSURE(context, input2 != nullptr);
  TfLiteTensor* output =
      micro_context->AllocateTempOutputTensor(node, kOutputTensor);
  TF_LITE_ENSURE(context, output != nullptr);

  if (input1->type == kTfLiteInt16) {
    TF_LITE_ENSURE_EQ(context, input1->params.zero_point, 0);
    TF_LITE_ENSURE_EQ(context, input2->params.zero_point, 0);
    TF_LITE_ENSURE_EQ(context, output->params.zero_point, 0);
  }

  OpData* data = static_cast<OpData*>(node->user_data);
  auto* params = reinterpret_cast<TfLiteAddParams*>(node->builtin_data);

  TF_LITE_ENSURE_STATUS(
      CalculateOpData(context, params, input1, input2, output, data));

  micro_context->DeallocateTempTfLiteTensor(input1);
  micro_context->DeallocateTempTfLiteTensor(input2);
  micro_context->DeallocateTempTfLiteTensor(output);

  return kTfLiteOk;
}

TfLiteStatus EvalAdd(TfLiteContext* context, TfLiteNode* node) {
  auto* params = reinterpret_cast<TfLiteAddParams*>(node->builtin_data);

  const TfLiteEvalTensor* input1 =
      tflite::micro::GetEvalInput(context, node, kInputTensor1);
  const TfLiteEvalTensor* input2 =
      tflite::micro::GetEvalInput(context, node, kInputTensor2);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData* data = static_cast<const OpData*>(node->user_data);

  if (output->type == kTfLiteFloat32) {
    EvalAddFloat(context, node, params, data, input1, input2, output);
  } else if (output->type == kTfLiteInt8 || output->type == kTfLiteInt16) {
    TF_LITE_ENSURE_OK(context, EvalAddQuantized(context, node, params, data,
                                                input1, input2, output));
  } else {
    MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(output->type),
                output->type);
    return kTfLiteError;
  }

  return kTfLiteOk;
}

TfLiteStatus EvalAddInt8(TfLiteContext* context, TfLiteNode* node) {
  auto* params = reinterpret_cast<TfLiteAddParams*>(node->builtin_data);

  const TfLiteEvalTensor* input1 =
      tflite::micro::GetEvalInput(context, node, kInputTensor1);
  const TfLiteEvalTensor* input2 =
      tflite::micro::GetEvalInput(context, node, kInputTensor2);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);

  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(output->type == kTfLiteInt8);
  const OpData* data = static_cast<const OpData*>(node->user_data);

  TF_LITE_ENSURE_OK(context, EvalAddQuantizedInt8(context, node, params, data,
                                                  input1, input2, output));

  return kTfLiteOk;
}

TfLiteStatus EvalAddInt16(TfLiteContext* context, TfLiteNode* node) {
  auto* params = reinterpret_cast<TfLiteAddParams*>(node->builtin_data);

  const TfLiteEvalTensor* input1 =
      tflite::micro::GetEvalInput(context, node, kInputTensor1);
  const TfLiteEvalTensor* input2 =
      tflite::micro::GetEvalInput(context, node, kInputTensor2);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);

  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(output->type == kTfLiteInt16);
  const OpData* data = static_cast<const OpData*>(node->user_data);

  TF_LITE_ENSURE_OK(context, EvalAddQuantizedInt16(context, node, params, data,
                                                   input1, input2, output));

  return kTfLiteOk;
}

TfLiteRegistration Register_ADD() {
  return tflite::micro::RegisterOp(InitAdd, PrepareAdd, EvalAdd);
}

TfLiteRegistration Register_ADD_INT8() {
  return tflite::micro::RegisterOp(InitAdd, PrepareAdd, EvalAddInt8);
}

TfLiteRegistration Register_ADD_INT16() {
  return tflite::micro::RegisterOp(InitAdd, PrepareAdd, EvalAddInt16);
}

}  // namespace tflite

#elif EI_CLASSIFIER_TFLITE_ENABLE_ARC == 1
/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/add.h"

#include <algorithm>
#include <limits>

#include "mli_api.h"  // NOLINT
#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/c/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/quantization_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/integer_ops/add.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/process_broadcast_shapes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/op_macros.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/add.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/mli_slicers.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/mli_tf_utils.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/scratch_buf_mgr.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/scratch_buffers.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/memory_helpers.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_log.h"

namespace tflite {

constexpr int kInputTensor1 = 0;
constexpr int kInputTensor2 = 1;
constexpr int kOutputTensor = 0;

struct OpData {
  bool requires_broadcast;

  // These fields are used in both the general 8-bit -> 8bit quantized path,
  // and the special 16-bit -> 16bit quantized path
  int input1_shift;
  int input2_shift;
  int32_t output_activation_min;
  int32_t output_activation_max;

  // These fields are used only in the general 8-bit -> 8bit quantized path
  int32_t input1_multiplier;
  int32_t input2_multiplier;
  int32_t output_multiplier;
  int output_shift;
  int left_shift;
  int32_t input1_offset;
  int32_t input2_offset;
  int32_t output_offset;

  // Used only for float evals:
  float output_activation_min_f32;
  float output_activation_max_f32;

  // The result of checking if MLI optimized version of tensors can be used.
  bool is_mli_applicable;

  // Tensors in MLI format.
  mutable ops::micro::MliTensorInterface mli_input1;
  mutable ops::micro::MliTensorInterface mli_input2;
  mutable ops::micro::MliTensorInterface mli_out;
};

TfLiteStatus CalculateOpData(TfLiteContext* context, TfLiteAddParams* params,
                             const TfLiteTensor* input1,
                             const TfLiteTensor* input2, TfLiteTensor* output,
                             OpData* data) {
  data->requires_broadcast = !HaveSameShapes(input1, input2);

  if (output->type == kTfLiteUInt8 || output->type == kTfLiteInt8) {
    TF_LITE_ENSURE_STATUS(CalculateActivationRangeQuantized(
        context, params->activation, output, &data->output_activation_min,
        &data->output_activation_max));

    // MLI 2.0 optimized version only supports int8_t datatype and min/max
    // within container range. Broadcasting isn't supported on the primitive
    // level (but might be implemented as part of slicing in future)
#ifdef MLI_2_0  //
    data->is_mli_applicable =
        (input1->type == kTfLiteInt8) && (input2->type == kTfLiteInt8) &&
        (output->type == kTfLiteInt8) && !data->requires_broadcast &&
        data->output_activation_min == std::numeric_limits<int8_t>::min() &&
        data->output_activation_max == std::numeric_limits<int8_t>::max();
#else
    data->is_mli_applicable = false;
#endif

    if (data->is_mli_applicable) {
      data->mli_input1 =
          ops::micro::MliTensorInterface(static_cast<mli_tensor*>(
              context->AllocatePersistentBuffer(context, sizeof(mli_tensor))));
      data->mli_input2 =
          ops::micro::MliTensorInterface(static_cast<mli_tensor*>(
              context->AllocatePersistentBuffer(context, sizeof(mli_tensor))));
      data->mli_out = ops::micro::MliTensorInterface(static_cast<mli_tensor*>(
          context->AllocatePersistentBuffer(context, sizeof(mli_tensor))));

      ops::micro::ConvertToMliTensor(input1, &data->mli_input1);
      ops::micro::ConvertToMliTensor(input2, &data->mli_input2);
      ops::micro::ConvertToMliTensor(output, &data->mli_out);
      /* Flatten tensors to simplify the process (as we don't support
       * broadcasting). */
      data->mli_input1.Shape()[0] =
          mli_hlp_count_elem_num(data->mli_input1.MliTensor(), 0);
      data->mli_input2.Shape()[0] =
          mli_hlp_count_elem_num(data->mli_input2.MliTensor(), 0);
      data->mli_out.Shape()[0] =
          mli_hlp_count_elem_num(data->mli_out.MliTensor(), 0);
      data->mli_input1.MemStride()[0] = data->mli_input2.MemStride()[0] = 1;
      data->mli_out.MemStride()[0] = 1;
      *data->mli_input1.Rank() = *data->mli_input2.Rank() = 1;
      *data->mli_out.Rank() = 1;
    }
  } else {
    data->is_mli_applicable = false;
  }

#if !defined(TF_LITE_STRIP_REFERENCE_IMPL)
  if (output->type == kTfLiteInt8 || output->type == kTfLiteInt16) {
    // 8bit -> 8bit general quantized path, with general rescalings
    data->input1_offset = -input1->params.zero_point;
    data->input2_offset = -input2->params.zero_point;
    data->output_offset = output->params.zero_point;
    data->left_shift = (output->type == kTfLiteInt16) ? 15 : 20;
    const double twice_max_input_scale =
        2 * static_cast<double>(
                std::max(input1->params.scale, input2->params.scale));
    const double real_input1_multiplier =
        static_cast<double>(input1->params.scale) / twice_max_input_scale;
    const double real_input2_multiplier =
        static_cast<double>(input2->params.scale) / twice_max_input_scale;
    const double real_output_multiplier =
        twice_max_input_scale /
        ((1 << data->left_shift) * static_cast<double>(output->params.scale));

    QuantizeMultiplierSmallerThanOneExp(
        real_input1_multiplier, &data->input1_multiplier, &data->input1_shift);

    QuantizeMultiplierSmallerThanOneExp(
        real_input2_multiplier, &data->input2_multiplier, &data->input2_shift);

    QuantizeMultiplierSmallerThanOneExp(
        real_output_multiplier, &data->output_multiplier, &data->output_shift);

    TF_LITE_ENSURE_STATUS(CalculateActivationRangeQuantized(
        context, params->activation, output, &data->output_activation_min,
        &data->output_activation_max));
  } else if (output->type == kTfLiteFloat32) {
    CalculateActivationRange(params->activation,
                             &data->output_activation_min_f32,
                             &data->output_activation_max_f32);
#endif  // !defined(TF_LITE_STRIP_REFERENCE_IMPL)
  }

  return kTfLiteOk;
}

TfLiteStatus EvalAdd(TfLiteContext* context, TfLiteNode* node,
                     TfLiteAddParams* params, const OpData* data,
                     const TfLiteEvalTensor* input1,
                     const TfLiteEvalTensor* input2, TfLiteEvalTensor* output) {
#if !defined(TF_LITE_STRIP_REFERENCE_IMPL)
  tflite::ArithmeticParams op_params;
  SetActivationParams(data->output_activation_min_f32,
                      data->output_activation_max_f32, &op_params);
  if (data->requires_broadcast) {
    reference_ops::BroadcastAdd4DSlow(
        op_params, tflite::micro::GetTensorShape(input1),
        tflite::micro::GetTensorData<float>(input1),
        tflite::micro::GetTensorShape(input2),
        tflite::micro::GetTensorData<float>(input2),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<float>(output));
  } else {
    reference_ops::Add(op_params, tflite::micro::GetTensorShape(input1),
                       tflite::micro::GetTensorData<float>(input1),
                       tflite::micro::GetTensorShape(input2),
                       tflite::micro::GetTensorData<float>(input2),
                       tflite::micro::GetTensorShape(output),
                       tflite::micro::GetTensorData<float>(output));
  }
  return kTfLiteOk;
#else
  MicroPrintf("Node configuration is not supported by ARC MLI Library.");
  return kTfLiteError;
#endif
}

TfLiteStatus EvalAddQuantized(TfLiteContext* context, TfLiteNode* node,
                              TfLiteAddParams* params, const OpData* data,
                              const TfLiteEvalTensor* input1,
                              const TfLiteEvalTensor* input2,
                              TfLiteEvalTensor* output) {
#if !defined(TF_LITE_STRIP_REFERENCE_IMPL)
  tflite::ArithmeticParams op_params;
  op_params.left_shift = data->left_shift;
  op_params.input1_offset = data->input1_offset;
  op_params.input1_multiplier = data->input1_multiplier;
  op_params.input1_shift = data->input1_shift;
  op_params.input2_offset = data->input2_offset;
  op_params.input2_multiplier = data->input2_multiplier;
  op_params.input2_shift = data->input2_shift;
  op_params.output_offset = data->output_offset;
  op_params.output_multiplier = data->output_multiplier;
  op_params.output_shift = data->output_shift;
  SetActivationParams(data->output_activation_min, data->output_activation_max,
                      &op_params);
  bool need_broadcast = reference_ops::ProcessBroadcastShapes(
      tflite::micro::GetTensorShape(input1),
      tflite::micro::GetTensorShape(input2), &op_params);

  switch (output->type) {
    case kTfLiteInt8: {
      if (need_broadcast) {
        reference_integer_ops::BroadcastAdd4DSlow(
            op_params, tflite::micro::GetTensorShape(input1),
            tflite::micro::GetTensorData<int8_t>(input1),
            tflite::micro::GetTensorShape(input2),
            tflite::micro::GetTensorData<int8_t>(input2),
            tflite::micro::GetTensorShape(output),
            tflite::micro::GetTensorData<int8_t>(output));
      } else {
        reference_integer_ops::Add(
            op_params, tflite::micro::GetTensorShape(input1),
            tflite::micro::GetTensorData<int8_t>(input1),
            tflite::micro::GetTensorShape(input2),
            tflite::micro::GetTensorData<int8_t>(input2),
            tflite::micro::GetTensorShape(output),
            tflite::micro::GetTensorData<int8_t>(output));
      }
      break;
    }
    case kTfLiteInt16: {
      if (need_broadcast) {
        reference_ops::BroadcastAdd4DSlow(
            op_params, tflite::micro::GetTensorShape(input1),
            tflite::micro::GetTensorData<int16_t>(input1),
            tflite::micro::GetTensorShape(input2),
            tflite::micro::GetTensorData<int16_t>(input2),
            tflite::micro::GetTensorShape(output),
            tflite::micro::GetTensorData<int16_t>(output));
      } else {
        reference_ops::Add(op_params, tflite::micro::GetTensorShape(input1),
                           tflite::micro::GetTensorData<int16_t>(input1),
                           tflite::micro::GetTensorShape(input2),
                           tflite::micro::GetTensorData<int16_t>(input2),
                           tflite::micro::GetTensorShape(output),
                           tflite::micro::GetTensorData<int16_t>(output),
                           false);
      }
      break;
    }
    default:
      MicroPrintf("Type %s (%d) not supported.",
                  TfLiteTypeGetName(output->type), output->type);
      return kTfLiteError;
  }

  return kTfLiteOk;
#else
  MicroPrintf("Node configuration is not supported by ARC MLI Library.");
  return kTfLiteError;
#endif
}

TfLiteStatus EvalMLIAddInt8(TfLiteContext* context, TfLiteNode* node,
                            TfLiteAddParams* params, const OpData* data,
                            const TfLiteEvalTensor* input1,
                            const TfLiteEvalTensor* input2,
                            TfLiteEvalTensor* output) {
#ifdef MLI_2_0
  TF_LITE_ENSURE(context, data->is_mli_applicable == true);
  TF_LITE_ENSURE(context, input1->type == kTfLiteInt8);
  TF_LITE_ENSURE(context, input2->type == kTfLiteInt8);
  TF_LITE_ENSURE(context, output->type == kTfLiteInt8);

  ops::micro::MliTensorAttachBuffer<int8_t>(input1, &data->mli_input1);
  ops::micro::MliTensorAttachBuffer<int8_t>(input2, &data->mli_input2);
  ops::micro::MliTensorAttachBuffer<int8_t>(output, &data->mli_out);

  // mli_mov config and tensors for data in fast (local) memory with interface
  mli_mov_cfg_t copy_config;
  mli_mov_cfg_for_copy(&copy_config);
  mli_tensor input1_local_tsr = *data->mli_input1.MliTensor();
  mli_tensor input2_local_tsr = *data->mli_input2.MliTensor();
  mli_tensor out_local_tsr = *data->mli_out.MliTensor();
  ops::micro::MliTensorInterface input1_local(&input1_local_tsr);
  ops::micro::MliTensorInterface input2_local(&input2_local_tsr);
  ops::micro::MliTensorInterface out_local(&out_local_tsr);

  /* allocate the local buffers, and compute the slice size */
  TF_LITE_ENSURE_STATUS(ops::micro::get_arc_scratch_buffer_for_eltwise_tensors(
      context, &input1_local, &input2_local, &out_local));
  TF_LITE_ENSURE(context, *input1_local.Rank() == 1 &&
                              *input2_local.Rank() == 1 &&
                              *out_local.Rank() == 1);
  uint32_t min_capacity = *input1_local.DataCapacity();
  min_capacity = std::min(min_capacity, *input2_local.DataCapacity());
  min_capacity = std::min(min_capacity, *out_local.DataCapacity());
  const int slice_dim = 0;
  const int slice_size =
      min_capacity / mli_hlp_tensor_element_size(out_local.MliTensor());

  /* is_local indicates that the tensor is already in local memory,
     so in that case the original tensor can be used,
     and there is no need to copy it to the local tensor*/
  const bool input1_is_local =
      input1_local.Data<int8_t>() == data->mli_input1.Data<int8_t>();
  const bool input2_is_local =
      input2_local.Data<int8_t>() == data->mli_input2.Data<int8_t>();
  const bool out_is_local =
      out_local.Data<int8_t>() == data->mli_out.Data<int8_t>();

  ops::micro::TensorSlicer input1_slice(data->mli_input1.MliTensor(), slice_dim,
                                        slice_size);
  ops::micro::TensorSlicer input2_slice(data->mli_input2.MliTensor(), slice_dim,
                                        slice_size);
  ops::micro::TensorSlicer out_slice(data->mli_out.MliTensor(), slice_dim,
                                     slice_size);

  mli_tensor* input1_tsr =
      input1_is_local ? input1_slice.Sub() : input1_local.MliTensor();
  mli_tensor* input2_tsr =
      input2_is_local ? input2_slice.Sub() : input2_local.MliTensor();
  mli_tensor* out_tsr = out_is_local ? out_slice.Sub() : out_local.MliTensor();

  while (!out_slice.Done()) {
    mli_mov_tensor_sync(input1_slice.Sub(), &copy_config, input1_tsr);
    mli_mov_tensor_sync(input2_slice.Sub(), &copy_config, input2_tsr);

    mli_krn_eltwise_add_sa8(input1_tsr, input2_tsr, out_tsr);

    mli_mov_tensor_sync(out_tsr, &copy_config, out_slice.Sub());
    input1_slice.Next();
    input2_slice.Next();
    out_slice.Next();
  }
  return kTfLiteOk;
#else
  return kTfLiteError;
#endif
}

void* AddInit(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpData));
}

TfLiteStatus AddEval(TfLiteContext* context, TfLiteNode* node) {
  TfLiteStatus ret_val = kTfLiteOk;
  auto* params = reinterpret_cast<TfLiteAddParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData* data = static_cast<const OpData*>(node->user_data);

  const TfLiteEvalTensor* input1 =
      tflite::micro::GetEvalInput(context, node, kInputTensor1);
  const TfLiteEvalTensor* input2 =
      tflite::micro::GetEvalInput(context, node, kInputTensor2);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);
  if (data->is_mli_applicable) {
    ret_val =
        EvalMLIAddInt8(context, node, params, data, input1, input2, output);
  } else if (output->type == kTfLiteFloat32) {
    ret_val = EvalAdd(context, node, params, data, input1, input2, output);
  } else if (output->type == kTfLiteInt8 || output->type == kTfLiteInt16) {
    ret_val =
        EvalAddQuantized(context, node, params, data, input1, input2, output);
  } else {
    MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(output->type),
                output->type);
    ret_val = kTfLiteError;
  }

  return ret_val;
}

TfLiteRegistration Register_ADD() {
  return tflite::micro::RegisterOp(AddInit, AddPrepare, AddEval);
}

}  // namespace tflite

#elif EI_CLASSIFIER_TFLITE_ENABLE_SILABS_MVP == 1
/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/add.h"

#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/c/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/quantization_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/integer_ops/add.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/process_broadcast_shapes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/op_macros.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"
#include "sl_mvp_ml_add.h"

namespace tflite {
namespace sl {
namespace add {

constexpr int kInputTensor1 = 0;
constexpr int kInputTensor2 = 1;
constexpr int kOutputTensor = 0;

struct OpData {
  bool requires_broadcast;

  int input1_shift;
  int input2_shift;
  int32_t input1_multiplier;
  int32_t input2_multiplier;
  int32_t output_multiplier;
  int output_shift;
  int left_shift;

  sli_mvp_ml_add_s8_params_t params;

  // Used only for float evals:
  float output_activation_min_f32;
  float output_activation_max_f32;
};

TfLiteStatus CalculateOpData(TfLiteContext* context, TfLiteAddParams* params,
                             const TfLiteTensor* input1,
                             const TfLiteTensor* input2, TfLiteTensor* output,
                             OpData* data) {
  data->requires_broadcast = !HaveSameShapes(input1, input2);

  if (output->type == kTfLiteInt8) {
    data->params.input1_offset = -input1->params.zero_point;
    data->params.input2_offset = -input2->params.zero_point;
    data->params.output_offset = output->params.zero_point;
    data->params.input1_multiplier = input1->params.scale;
    data->params.input2_multiplier = input2->params.scale;
    data->params.output_multiplier = 1.0 / output->params.scale;
    data->params.length = GetTensorShape(input1).FlatSize();

    int32_t activation_min;
    int32_t activation_max;
    TF_LITE_ENSURE_STATUS(CalculateActivationRangeQuantized(
        context, params->activation, output, &activation_min,
        &activation_max));
    data->params.activation_min = static_cast<int8_t>(activation_min);
    data->params.activation_max = static_cast<int8_t>(activation_max);

    // These multipliers and parameters are not used by the MVP codepath,
    // however are needed in cases where broadcast is used.
    data->left_shift = 20;
    const double twice_max_input_scale =
        2 * static_cast<double>(
                std::max(input1->params.scale, input2->params.scale));
    const double real_input1_multiplier =
        static_cast<double>(input1->params.scale) / twice_max_input_scale;
    const double real_input2_multiplier =
        static_cast<double>(input2->params.scale) / twice_max_input_scale;
    const double real_output_multiplier =
        twice_max_input_scale /
        ((1 << data->left_shift) * static_cast<double>(output->params.scale));

    QuantizeMultiplierSmallerThanOneExp(
        real_input1_multiplier, &data->input1_multiplier, &data->input1_shift);

    QuantizeMultiplierSmallerThanOneExp(
        real_input2_multiplier, &data->input2_multiplier, &data->input2_shift);

    QuantizeMultiplierSmallerThanOneExp(
        real_output_multiplier, &data->output_multiplier, &data->output_shift);

  } else if (output->type == kTfLiteFloat32) {
    CalculateActivationRange(params->activation,
                             &data->output_activation_min_f32,
                             &data->output_activation_max_f32);
  }

  return kTfLiteOk;
}

void EvalAdd(TfLiteContext* context, TfLiteNode* node, TfLiteAddParams* params,
             const OpData* data, const TfLiteEvalTensor* input1,
             const TfLiteEvalTensor* input2, TfLiteEvalTensor* output) {
  tflite::ArithmeticParams op_params;
  SetActivationParams(data->output_activation_min_f32,
                      data->output_activation_max_f32, &op_params);
  if (data->requires_broadcast) {
    reference_ops::BroadcastAdd4DSlow(op_params, tflite::micro::GetTensorShape(input1), tflite::micro::GetTensorData<float>(input1),
                                      tflite::micro::GetTensorShape(input2), tflite::micro::GetTensorData<float>(input2),
                                      tflite::micro::GetTensorShape(output), tflite::micro::GetTensorData<float>(output));
  } else {
    reference_ops::Add(op_params,
                       tflite::micro::GetTensorShape(input1), tflite::micro::GetTensorData<float>(input1),
                       tflite::micro::GetTensorShape(input2), tflite::micro::GetTensorData<float>(input2),
                       tflite::micro::GetTensorShape(output), tflite::micro::GetTensorData<float>(output));
  }
}

TfLiteStatus EvalAddQuantized(TfLiteContext* context, TfLiteNode* node,
                              TfLiteAddParams* params, const OpData* data,
                              const TfLiteEvalTensor* input1,
                              const TfLiteEvalTensor* input2,
                              TfLiteEvalTensor* output) {
  TfLiteStatus status = kTfLiteOk;
  tflite::ArithmeticParams op_params;
  op_params.left_shift = data->left_shift;
  op_params.input1_offset = data->params.input1_offset;
  op_params.input1_multiplier = data->input1_multiplier;
  op_params.input1_shift = data->input1_shift;
  op_params.input2_offset = data->params.input2_offset;
  op_params.input2_multiplier = data->input2_multiplier;
  op_params.input2_shift = data->input2_shift;
  op_params.output_offset = data->params.output_offset;
  op_params.output_multiplier = data->output_multiplier;
  op_params.output_shift = data->output_shift;
  op_params.quantized_activation_min = data->params.activation_min;
  op_params.quantized_activation_max = data->params.activation_max;

  // TODO: Do we need to support the broadcast scenario?
  bool need_broadcast = reference_ops::ProcessBroadcastShapes(tflite::micro::GetTensorShape(input1), tflite::micro::GetTensorShape(input2), &op_params);

  if (need_broadcast) {
    reference_integer_ops::BroadcastAdd4DSlow(op_params,
                                              tflite::micro::GetTensorShape(input1), tflite::micro::GetTensorData<int8_t>(input1),
                                              tflite::micro::GetTensorShape(input2), tflite::micro::GetTensorData<int8_t>(input2),
                                              tflite::micro::GetTensorShape(output), tflite::micro::GetTensorData<int8_t>(output));
  } else {
    sli_mvp_ml_add_s8_params_t params = data->params;
    params.input1 = tflite::micro::GetTensorData<int8_t>(input1);
    params.input2 = tflite::micro::GetTensorData<int8_t>(input2);
    params.output = tflite::micro::GetTensorData<int8_t>(output);
    sl_status_t ret = sli_mvp_ml_add_s8(&params);
    if (ret != SL_STATUS_OK) {
        status = kTfLiteError;
    }
  }

  return status;
}

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpData));
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  const TfLiteTensor* input1 = GetInput(context, node, kInputTensor1);
  TF_LITE_ENSURE(context, input1 != nullptr);
  const TfLiteTensor* input2 = GetInput(context, node, kInputTensor2);
  TF_LITE_ENSURE(context, input2 != nullptr);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);
  TF_LITE_ENSURE(context, output != nullptr);

  OpData* data = static_cast<OpData*>(node->user_data);
  auto* params = reinterpret_cast<TfLiteAddParams*>(node->builtin_data);

  TF_LITE_ENSURE_STATUS(
      CalculateOpData(context, params, input1, input2, output, data));

  return kTfLiteOk;
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  auto* params = reinterpret_cast<TfLiteAddParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData* data = static_cast<const OpData*>(node->user_data);

  const TfLiteEvalTensor* input1 = tflite::micro::GetEvalInput(context, node, kInputTensor1);
  const TfLiteEvalTensor* input2 = tflite::micro::GetEvalInput(context, node, kInputTensor2);
  TfLiteEvalTensor* output = tflite::micro::GetEvalOutput(context, node, kOutputTensor);

  if (output->type == kTfLiteFloat32) {
    EvalAdd(context, node, params, data, input1, input2, output);
  } else if (output->type == kTfLiteInt8) {
    TF_LITE_ENSURE_OK(context, EvalAddQuantized(context, node, params, data,
                                                input1, input2, output));
  } else {
    TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                       TfLiteTypeGetName(output->type), output->type);
    return kTfLiteError;
  }

  return kTfLiteOk;
}

}  // namespace add
}  // namespace sl

TfLiteRegistration Register_ADD() {
  return {/*init=*/sl::add::Init,
          /*free=*/nullptr,
          /*prepare=*/sl::add::Prepare,
          /*invoke=*/sl::add::Eval,
          /*profiling_string=*/nullptr,
          /*builtin_code=*/0,
          /*custom_name=*/nullptr,
          /*version=*/0};
}

}  // namespace tflite

#elif EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN == 1
/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/add.h"

#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/c/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/quantization_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/integer_ops/add.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/process_broadcast_shapes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/op_macros.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/add.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/memory_helpers.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_log.h"

#include <esp_timer.h>

#if ESP_NN
#include "edge-impulse-sdk/porting/espressif/ESP-NN/include/esp_nn.h"
#endif

long long add_total_time = 0;

namespace tflite {

void EvalAdd(TfLiteContext* context, TfLiteNode* node, TfLiteAddParams* params,
             const OpDataAdd* data, const TfLiteEvalTensor* input1,
             const TfLiteEvalTensor* input2, TfLiteEvalTensor* output) {
  tflite::ArithmeticParams op_params;
  SetActivationParams(data->output_activation_min_f32,
                      data->output_activation_max_f32, &op_params);
  if (data->requires_broadcast) {
    reference_ops::BroadcastAdd4DSlow(
        op_params, tflite::micro::GetTensorShape(input1),
        tflite::micro::GetTensorData<float>(input1),
        tflite::micro::GetTensorShape(input2),
        tflite::micro::GetTensorData<float>(input2),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<float>(output));
  } else {
    reference_ops::Add(op_params, tflite::micro::GetTensorShape(input1),
                       tflite::micro::GetTensorData<float>(input1),
                       tflite::micro::GetTensorShape(input2),
                       tflite::micro::GetTensorData<float>(input2),
                       tflite::micro::GetTensorShape(output),
                       tflite::micro::GetTensorData<float>(output));
  }
}

TfLiteStatus EvalAddQuantized(TfLiteContext* context, TfLiteNode* node,
                              TfLiteAddParams* params, const OpDataAdd* data,
                              const TfLiteEvalTensor* input1,
                              const TfLiteEvalTensor* input2,
                              TfLiteEvalTensor* output) {
  tflite::ArithmeticParams op_params;
  op_params.left_shift = data->left_shift;
  op_params.input1_offset = data->input1_offset;
  op_params.input1_multiplier = data->input1_multiplier;
  op_params.input1_shift = data->input1_shift;
  op_params.input2_offset = data->input2_offset;
  op_params.input2_multiplier = data->input2_multiplier;
  op_params.input2_shift = data->input2_shift;
  op_params.output_offset = data->output_offset;
  op_params.output_multiplier = data->output_multiplier;
  op_params.output_shift = data->output_shift;
  SetActivationParams(data->output_activation_min, data->output_activation_max,
                      &op_params);
  bool need_broadcast = reference_ops::ProcessBroadcastShapes(
      tflite::micro::GetTensorShape(input1),
      tflite::micro::GetTensorShape(input2), &op_params);

  switch (output->type) {
    case kTfLiteInt8: {
      if (need_broadcast) {
        reference_integer_ops::BroadcastAdd4DSlow(
            op_params, tflite::micro::GetTensorShape(input1),
            tflite::micro::GetTensorData<int8_t>(input1),
            tflite::micro::GetTensorShape(input2),
            tflite::micro::GetTensorData<int8_t>(input2),
            tflite::micro::GetTensorShape(output),
            tflite::micro::GetTensorData<int8_t>(output));
      } else {
#if ESP_NN
        const int8_t *input1_data = tflite::micro::GetTensorData<int8_t>(input1);
        const int8_t *input2_data = tflite::micro::GetTensorData<int8_t>(input2);
        int8_t *out_data = tflite::micro::GetTensorData<int8_t>(output);

        esp_nn_add_elementwise_s8(input1_data,
                                  input2_data,
                                  data->input1_offset,
                                  data->input2_offset,
                                  data->input1_multiplier,
                                  data->input2_multiplier,
                                  data->input1_shift,
                                  data->input2_shift,
                                  data->left_shift,
                                  out_data,
                                  data->output_offset,
                                  data->output_multiplier,
                                  data->output_shift,
                                  data->output_activation_min,
                                  data->output_activation_max,
                                  MatchingElementsSize(tflite::micro::GetTensorShape(input1),
                                                       tflite::micro::GetTensorShape(input2),
                                                       tflite::micro::GetTensorShape(output))
                                  );
#else
        reference_integer_ops::Add(
            op_params, tflite::micro::GetTensorShape(input1),
            tflite::micro::GetTensorData<int8_t>(input1),
            tflite::micro::GetTensorShape(input2),
            tflite::micro::GetTensorData<int8_t>(input2),
            tflite::micro::GetTensorShape(output),
            tflite::micro::GetTensorData<int8_t>(output));
#endif
      }
      break;
    }
    case kTfLiteInt16: {
      if (need_broadcast) {
        reference_ops::BroadcastAdd4DSlow(
            op_params, tflite::micro::GetTensorShape(input1),
            tflite::micro::GetTensorData<int16_t>(input1),
            tflite::micro::GetTensorShape(input2),
            tflite::micro::GetTensorData<int16_t>(input2),
            tflite::micro::GetTensorShape(output),
            tflite::micro::GetTensorData<int16_t>(output));
      } else {
        reference_ops::Add(op_params, tflite::micro::GetTensorShape(input1),
                           tflite::micro::GetTensorData<int16_t>(input1),
                           tflite::micro::GetTensorShape(input2),
                           tflite::micro::GetTensorData<int16_t>(input2),
                           tflite::micro::GetTensorShape(output),
                           tflite::micro::GetTensorData<int16_t>(output),
                           false);
      }
      break;
    }
    default:
      MicroPrintf("Type %s (%d) not supported.",
                  TfLiteTypeGetName(output->type), output->type);
      return kTfLiteError;
  }

  return kTfLiteOk;
}

void* AddInit(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpDataAdd));
}

TfLiteStatus AddEval(TfLiteContext* context, TfLiteNode* node) {
  auto* params = reinterpret_cast<TfLiteAddParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpDataAdd* data = static_cast<const OpDataAdd*>(node->user_data);

  const TfLiteEvalTensor* input1 =
      tflite::micro::GetEvalInput(context, node, kAddInputTensor1);
  const TfLiteEvalTensor* input2 =
      tflite::micro::GetEvalInput(context, node, kAddInputTensor2);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kAddOutputTensor);

  long long start_time = esp_timer_get_time();

  if (output->type == kTfLiteFloat32) {
    EvalAdd(context, node, params, data, input1, input2, output);
  } else if (output->type == kTfLiteInt8 || output->type == kTfLiteInt16) {
    TF_LITE_ENSURE_OK(context, EvalAddQuantized(context, node, params, data,
                                                input1, input2, output));
  } else {
    MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(output->type),
                output->type);
    return kTfLiteError;
  }
  add_total_time += esp_timer_get_time() - start_time;

  return kTfLiteOk;
}

TfLiteRegistration Register_ADD() {
  return tflite::micro::RegisterOp(AddInit, AddPrepare, AddEval);
}

}  // namespace tflite

#else
/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/add.h"

#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/c/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/quantization_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/integer_ops/add.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/process_broadcast_shapes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/op_macros.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/add.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/memory_helpers.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_log.h"

namespace tflite {

void EvalAdd(TfLiteContext* context, TfLiteNode* node, TfLiteAddParams* params,
             const OpDataAdd* data, const TfLiteEvalTensor* input1,
             const TfLiteEvalTensor* input2, TfLiteEvalTensor* output) {
  tflite::ArithmeticParams op_params;
  SetActivationParams(data->output_activation_min_f32,
                      data->output_activation_max_f32, &op_params);
  if (data->requires_broadcast) {
    reference_ops::BroadcastAdd4DSlow(
        op_params, tflite::micro::GetTensorShape(input1),
        tflite::micro::GetTensorData<float>(input1),
        tflite::micro::GetTensorShape(input2),
        tflite::micro::GetTensorData<float>(input2),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<float>(output));
  } else {
    reference_ops::Add(op_params, tflite::micro::GetTensorShape(input1),
                       tflite::micro::GetTensorData<float>(input1),
                       tflite::micro::GetTensorShape(input2),
                       tflite::micro::GetTensorData<float>(input2),
                       tflite::micro::GetTensorShape(output),
                       tflite::micro::GetTensorData<float>(output));
  }
}

TfLiteStatus EvalAddQuantized(TfLiteContext* context, TfLiteNode* node,
                              TfLiteAddParams* params, const OpDataAdd* data,
                              const TfLiteEvalTensor* input1,
                              const TfLiteEvalTensor* input2,
                              TfLiteEvalTensor* output) {
  tflite::ArithmeticParams op_params;
  op_params.left_shift = data->left_shift;
  op_params.input1_offset = data->input1_offset;
  op_params.input1_multiplier = data->input1_multiplier;
  op_params.input1_shift = data->input1_shift;
  op_params.input2_offset = data->input2_offset;
  op_params.input2_multiplier = data->input2_multiplier;
  op_params.input2_shift = data->input2_shift;
  op_params.output_offset = data->output_offset;
  op_params.output_multiplier = data->output_multiplier;
  op_params.output_shift = data->output_shift;
  SetActivationParams(data->output_activation_min, data->output_activation_max,
                      &op_params);
  bool need_broadcast = reference_ops::ProcessBroadcastShapes(
      tflite::micro::GetTensorShape(input1),
      tflite::micro::GetTensorShape(input2), &op_params);

  switch (output->type) {
    case kTfLiteInt8: {
      if (need_broadcast) {
        reference_integer_ops::BroadcastAdd4DSlow(
            op_params, tflite::micro::GetTensorShape(input1),
            tflite::micro::GetTensorData<int8_t>(input1),
            tflite::micro::GetTensorShape(input2),
            tflite::micro::GetTensorData<int8_t>(input2),
            tflite::micro::GetTensorShape(output),
            tflite::micro::GetTensorData<int8_t>(output));
      } else {
        reference_integer_ops::Add(
            op_params, tflite::micro::GetTensorShape(input1),
            tflite::micro::GetTensorData<int8_t>(input1),
            tflite::micro::GetTensorShape(input2),
            tflite::micro::GetTensorData<int8_t>(input2),
            tflite::micro::GetTensorShape(output),
            tflite::micro::GetTensorData<int8_t>(output));
      }
      break;
    }
    case kTfLiteInt16: {
      if (need_broadcast) {
        reference_ops::BroadcastAdd4DSlow(
            op_params, tflite::micro::GetTensorShape(input1),
            tflite::micro::GetTensorData<int16_t>(input1),
            tflite::micro::GetTensorShape(input2),
            tflite::micro::GetTensorData<int16_t>(input2),
            tflite::micro::GetTensorShape(output),
            tflite::micro::GetTensorData<int16_t>(output));
      } else {
        reference_ops::Add(op_params, tflite::micro::GetTensorShape(input1),
                           tflite::micro::GetTensorData<int16_t>(input1),
                           tflite::micro::GetTensorShape(input2),
                           tflite::micro::GetTensorData<int16_t>(input2),
                           tflite::micro::GetTensorShape(output),
                           tflite::micro::GetTensorData<int16_t>(output),
                           false);
      }
      break;
    }
    default:
      MicroPrintf("Type %s (%d) not supported.",
                  TfLiteTypeGetName(output->type), output->type);
      return kTfLiteError;
  }

  return kTfLiteOk;
}

void* AddInit(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpDataAdd));
}

TfLiteStatus AddEval(TfLiteContext* context, TfLiteNode* node) {
  auto* params = reinterpret_cast<TfLiteAddParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpDataAdd* data = static_cast<const OpDataAdd*>(node->user_data);

  const TfLiteEvalTensor* input1 =
      tflite::micro::GetEvalInput(context, node, kAddInputTensor1);
  const TfLiteEvalTensor* input2 =
      tflite::micro::GetEvalInput(context, node, kAddInputTensor2);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kAddOutputTensor);

  if (output->type == kTfLiteFloat32) {
    EvalAdd(context, node, params, data, input1, input2, output);
  } else if (output->type == kTfLiteInt8 || output->type == kTfLiteInt16) {
    TF_LITE_ENSURE_OK(context, EvalAddQuantized(context, node, params, data,
                                                input1, input2, output));
  } else {
    MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(output->type),
                output->type);
    return kTfLiteError;
  }

  return kTfLiteOk;
}

TfLiteRegistration Register_ADD() {
  return tflite::micro::RegisterOp(AddInit, AddPrepare, AddEval);
}

}  // namespace tflite

#endif
