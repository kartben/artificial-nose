// Patched by Edge Impulse to include reference, CMSIS-NN and ARC kernels
#include "../../../../classifier/ei_classifier_config.h"
#if 0 == 1
/* noop */
#elif EI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN == 1
/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

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
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/pooling.h"

#include "edge-impulse-sdk/CMSIS/NN/Include/arm_nnfunctions.h"
#include "edge-impulse-sdk/third_party/flatbuffers/include/flatbuffers/base.h"  // from @flatbuffers
#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/integer_ops/pooling.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/padding.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"

namespace tflite {
namespace ops {
namespace micro {
namespace pooling {

namespace {

constexpr int kInputTensor = 0;
constexpr int kOutputTensor = 0;

struct OpData {
  TfLitePaddingValues padding;
  // Index to buffer for optimizations if applicable.
  int buffer_idx;

  int32_t activation_min;
  int32_t activation_max;
  float activation_min_f32;
  float activation_max_f32;
};

TfLiteStatus CalculateOpData(TfLiteContext* context,
                             const TfLitePoolParams* params,
                             const TfLiteTensor* input, TfLiteTensor* output,
                             OpData* data) {
  // input: batch, height, width, channel
  int height = SizeOfDimension(input, 1);
  int width = SizeOfDimension(input, 2);

  int out_height, out_width;

  data->padding = ComputePaddingHeightWidth(
      params->stride_height, params->stride_width,
      /*dilation_rate_height=*/1,
      /*dilation_rate_width=*/1, height, width, params->filter_height,
      params->filter_width, params->padding, &out_height, &out_width);

  if (input->type == kTfLiteFloat32) {
    CalculateActivationRange(params->activation, &data->activation_min_f32,
                             &data->activation_max_f32);
  } else {
    TF_LITE_ENSURE_STATUS(CalculateActivationRangeQuantized(
        context, params->activation, output, &data->activation_min,
        &data->activation_max));
    TFLITE_DCHECK_LE(data->activation_min, data->activation_max);
  }

  // Set buffer index to a reset value
  data->buffer_idx = -1;

  return kTfLiteOk;
}

void AverageEvalFloat(const TfLiteContext* context, const TfLiteNode* node,
                      const TfLitePoolParams* params, const OpData& data,
                      const TfLiteEvalTensor* input, TfLiteEvalTensor* output) {
  float activation_min, activation_max;
  CalculateActivationRange(params->activation, &activation_min,
                           &activation_max);

  PoolParams op_params;
  op_params.stride_height = params->stride_height;
  op_params.stride_width = params->stride_width;
  op_params.filter_height = params->filter_height;
  op_params.filter_width = params->filter_width;
  op_params.padding_values.height = data.padding.height;
  op_params.padding_values.width = data.padding.width;
  op_params.float_activation_min = activation_min;
  op_params.float_activation_max = activation_max;
  reference_ops::AveragePool(op_params, tflite::micro::GetTensorShape(input),
                             tflite::micro::GetTensorData<float>(input),
                             tflite::micro::GetTensorShape(output),
                             tflite::micro::GetTensorData<float>(output));
}

void AverageEvalQuantized(TfLiteContext* context, const TfLiteNode* node,
                          const TfLitePoolParams* params, const OpData& data,
                          const TfLiteEvalTensor* input,
                          TfLiteEvalTensor* output) {
  TFLITE_DCHECK(input->type == kTfLiteUInt8 || input->type == kTfLiteInt8);

  if (input->type == kTfLiteUInt8) {
    PoolParams op_params;
    op_params.stride_height = params->stride_height;
    op_params.stride_width = params->stride_width;
    op_params.filter_height = params->filter_height;
    op_params.filter_width = params->filter_width;
    op_params.padding_values.height = data.padding.height;
    op_params.padding_values.width = data.padding.width;
    op_params.quantized_activation_min = data.activation_min;
    op_params.quantized_activation_max = data.activation_max;

    reference_ops::AveragePool(op_params, tflite::micro::GetTensorShape(input),
                               tflite::micro::GetTensorData<uint8_t>(input),
                               tflite::micro::GetTensorShape(output),
                               tflite::micro::GetTensorData<uint8_t>(output));
  } else {
    RuntimeShape input_shape = tflite::micro::GetTensorShape(input);
    TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);

    RuntimeShape output_shape = tflite::micro::GetTensorShape(output);
    TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);

    const int depth = MatchingDim(input_shape, 3, output_shape, 3);

    cmsis_nn_dims input_dims;
    input_dims.n = 1;
    input_dims.h = input_shape.Dims(1);
    input_dims.w = input_shape.Dims(2);
    input_dims.c = depth;

    cmsis_nn_dims output_dims;
    output_dims.n = 1;
    output_dims.h = output_shape.Dims(1);
    output_dims.w = output_shape.Dims(2);
    output_dims.c = depth;

    cmsis_nn_pool_params pool_params;
    pool_params.stride.h = params->stride_height;
    pool_params.stride.w = params->stride_width;
    pool_params.padding.h = data.padding.height;
    pool_params.padding.w = data.padding.width;
    pool_params.activation.min = data.activation_min;
    pool_params.activation.max = data.activation_max;

    cmsis_nn_dims filter_dims;
    filter_dims.n = 1;
    filter_dims.h = params->filter_height;
    filter_dims.w = params->filter_width;
    filter_dims.c = 1;

    cmsis_nn_context ctx;
    ctx.buf = nullptr;
    ctx.size = 0;
    if (data.buffer_idx > -1) {
      ctx.buf = context->GetScratchBuffer(context, data.buffer_idx);
    }

    TFLITE_DCHECK_EQ(
        arm_avgpool_s8(&ctx, &pool_params, &input_dims,
                       tflite::micro::GetTensorData<int8_t>(input),
                       &filter_dims, &output_dims,
                       tflite::micro::GetTensorData<int8_t>(output)),
        ARM_MATH_SUCCESS);
  }
}

void MaxEvalFloat(TfLiteContext* context, TfLiteNode* node,
                  TfLitePoolParams* params, const OpData& data,
                  const TfLiteEvalTensor* input, TfLiteEvalTensor* output) {
  float activation_min, activation_max;
  CalculateActivationRange(params->activation, &activation_min,
                           &activation_max);
  tflite::PoolParams op_params;
  op_params.stride_height = params->stride_height;
  op_params.stride_width = params->stride_width;
  op_params.filter_height = params->filter_height;
  op_params.filter_width = params->filter_width;
  op_params.padding_values.height = data.padding.height;
  op_params.padding_values.width = data.padding.width;
  op_params.float_activation_min = data.activation_min_f32;
  op_params.float_activation_max = data.activation_max_f32;
  reference_ops::MaxPool(op_params, tflite::micro::GetTensorShape(input),
                         tflite::micro::GetTensorData<float>(input),
                         tflite::micro::GetTensorShape(output),
                         tflite::micro::GetTensorData<float>(output));
}

void MaxEvalQuantizedUInt8(TfLiteContext* context, TfLiteNode* node,
                           TfLitePoolParams* params, const OpData& data,
                           const TfLiteEvalTensor* input,
                           TfLiteEvalTensor* output) {
  tflite::PoolParams op_params;
  op_params.stride_height = params->stride_height;
  op_params.stride_width = params->stride_width;
  op_params.filter_height = params->filter_height;
  op_params.filter_width = params->filter_width;
  op_params.padding_values.height = data.padding.height;
  op_params.padding_values.width = data.padding.width;
  op_params.quantized_activation_min = data.activation_min;
  op_params.quantized_activation_max = data.activation_max;
  reference_ops::MaxPool(op_params, tflite::micro::GetTensorShape(input),
                         tflite::micro::GetTensorData<uint8_t>(input),
                         tflite::micro::GetTensorShape(output),
                         tflite::micro::GetTensorData<uint8_t>(output));
}

TfLiteStatus MaxEvalInt8(TfLiteContext* context, const TfLiteNode* node,
                         const TfLitePoolParams* params, const OpData& data,
                         const TfLiteEvalTensor* input,
                         TfLiteEvalTensor* output) {
  RuntimeShape input_shape = tflite::micro::GetTensorShape(input);
  RuntimeShape output_shape = tflite::micro::GetTensorShape(output);
  const int depth = MatchingDim(input_shape, 3, output_shape, 3);

  cmsis_nn_dims input_dims;
  input_dims.n = 1;
  input_dims.h = input_shape.Dims(1);
  input_dims.w = input_shape.Dims(2);
  input_dims.c = depth;

  cmsis_nn_dims output_dims;
  output_dims.n = 1;
  output_dims.h = output_shape.Dims(1);
  output_dims.w = output_shape.Dims(2);
  output_dims.c = depth;

  cmsis_nn_pool_params pool_params;
  pool_params.stride.h = params->stride_height;
  pool_params.stride.w = params->stride_width;
  pool_params.padding.h = data.padding.height;
  pool_params.padding.w = data.padding.width;
  pool_params.activation.min = data.activation_min;
  pool_params.activation.max = data.activation_max;

  cmsis_nn_dims filter_dims;
  filter_dims.n = 1;
  filter_dims.h = params->filter_height;
  filter_dims.w = params->filter_width;
  filter_dims.c = 1;

  cmsis_nn_context ctx;
  ctx.buf = nullptr;
  ctx.size = 0;
  if (data.buffer_idx > -1) {
    ctx.buf = context->GetScratchBuffer(context, data.buffer_idx);
  }

  TFLITE_DCHECK_EQ(
      arm_max_pool_s8(&ctx, &pool_params, &input_dims,
                      tflite::micro::GetTensorData<int8_t>(input), &filter_dims,
                      &output_dims,
                      tflite::micro::GetTensorData<int8_t>(output)),
      ARM_MATH_SUCCESS);

  return kTfLiteOk;
}

}  // namespace

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpData));
}

TfLiteStatus MaxPrepare(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  OpData* data = static_cast<OpData*>(node->user_data);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  TF_LITE_ENSURE(context, input != nullptr);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);
  TF_LITE_ENSURE(context, output != nullptr);

  TF_LITE_ENSURE_STATUS(CalculateOpData(context, params, input, output, data));

  return kTfLiteOk;
}

TfLiteStatus AveragePrepare(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  OpData* data = static_cast<OpData*>(node->user_data);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

  TF_LITE_ENSURE_STATUS(CalculateOpData(context, params, input, output, data));

  if (input->type == kTfLiteInt8) {
    RuntimeShape input_shape = GetTensorShape(input);
    TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);

    RuntimeShape output_shape = GetTensorShape(output);
    TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);

    const int depth = MatchingDim(input_shape, 3, output_shape, 3);
    const int output_width = output_shape.Dims(2);

    const int32_t buffer_size =
        arm_avgpool_s8_get_buffer_size(output_width, depth);

    if (buffer_size > 0) {
      TF_LITE_ENSURE_STATUS(context->RequestScratchBufferInArena(
          context, buffer_size, &data->buffer_idx));
    } else {
      data->buffer_idx = -1;
    }
  }
  return kTfLiteOk;
}

TfLiteStatus AverageEval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kInputTensor);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);

  // Inputs and outputs share the same type, guaranteed by the converter.
  switch (input->type) {
    case kTfLiteFloat32:
      AverageEvalFloat(context, node, params, data, input, output);
      break;
    case kTfLiteUInt8:
    case kTfLiteInt8:
      AverageEvalQuantized(context, node, params, data, input, output);
      break;
    default:
      TF_LITE_KERNEL_LOG(context, "Input type %s is not currently supported",
                         TfLiteTypeGetName(input->type));
      return kTfLiteError;
  }
  return kTfLiteOk;
}

TfLiteStatus MaxEval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kInputTensor);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);

  switch (input->type) {
    case kTfLiteFloat32:
      MaxEvalFloat(context, node, params, data, input, output);
      break;
    case kTfLiteUInt8:
      MaxEvalQuantizedUInt8(context, node, params, data, input, output);
      break;
    case kTfLiteInt8:
      MaxEvalInt8(context, node, params, data, input, output);
      break;
    default:
      TF_LITE_KERNEL_LOG(context, "Type %s not currently supported.",
                         TfLiteTypeGetName(input->type));
      return kTfLiteError;
  }
  return kTfLiteOk;
}

}  // namespace pooling

TfLiteRegistration Register_AVERAGE_POOL_2D() {
  return {/*init=*/pooling::Init,
          /*free=*/nullptr,
          /*prepare=*/pooling::AveragePrepare,
          /*invoke=*/pooling::AverageEval,
          /*profiling_string=*/nullptr,
          /*builtin_code=*/0,
          /*custom_name=*/nullptr,
          /*version=*/0};
}

TfLiteRegistration Register_MAX_POOL_2D() {
  return {/*init=*/pooling::Init,
          /*free=*/nullptr,
          /*prepare=*/pooling::MaxPrepare,
          /*invoke=*/pooling::MaxEval,
          /*profiling_string=*/nullptr,
          /*builtin_code=*/0,
          /*custom_name=*/nullptr,
          /*version=*/0};
}

}  // namespace micro
}  // namespace ops
}  // namespace tflite

#elif EI_CLASSIFIER_TFLITE_ENABLE_ARC == 1
/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

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
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/pooling.h"

#include "mli_api.h"  // NOLINT
#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/integer_ops/pooling.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/padding.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/mli_slicers.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/mli_tf_utils.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/scratch_buf_mgr.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/scratch_buffers.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"

namespace tflite {
namespace ops {
namespace micro {
namespace pooling {

namespace {

constexpr int kInputTensor = 0;
constexpr int kOutputTensor = 0;

struct OpData {
  TfLitePaddingValues padding;
  int32_t activation_min;
  int32_t activation_max;
  float activation_min_f32;
  float activation_max_f32;

  // The result of checking if MLI optimized version of tensors can be used.
  bool is_mli_applicable;

  // Tensors in MLI format.
  mli_tensor* mli_in;
  mli_tensor* mli_out;
  mli_pool_cfg* cfg;
};

enum MliPoolingType { AveragePooling = 0, MaxPooling = 1 };

bool IsMliApplicable(TfLiteContext* context, const TfLiteTensor* input,
                     const TfLitePoolParams* params) {
  // MLI optimized version only supports int8_t datatype and no fused Relu
  return (input->type == kTfLiteInt8 && params->activation == kTfLiteActNone);
}

TfLiteStatus CalculateOpData(TfLiteContext* context,
                             const TfLitePoolParams* params,
                             const TfLiteTensor* input,
                             const TfLiteTensor* output, OpData* data) {
  // input: batch, height, width, channel
  int height = SizeOfDimension(input, 1);
  int width = SizeOfDimension(input, 2);

  int out_height, out_width;

  data->padding = ComputePaddingHeightWidth(
      params->stride_height, params->stride_width,
      /*dilation_rate_height=*/1,
      /*dilation_rate_width=*/1, height, width, params->filter_height,
      params->filter_width, params->padding, &out_height, &out_width);
  return kTfLiteOk;
}

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpData));
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  OpData* data = static_cast<OpData*>(node->user_data);

  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  TF_LITE_ENSURE(context, input != nullptr);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);
  TF_LITE_ENSURE(context, output != nullptr);

  data->is_mli_applicable = IsMliApplicable(context, input, params);

  TF_LITE_ENSURE_STATUS(CalculateOpData(context, params, input, output, data));

  if (input->type == kTfLiteFloat32) {
    CalculateActivationRange(params->activation, &data->activation_min_f32,
                             &data->activation_max_f32);
  } else if (input->type == kTfLiteInt8 || input->type == kTfLiteUInt8) {
    CalculateActivationRangeQuantized(context, params->activation, output,
                                      &data->activation_min,
                                      &data->activation_max);
  }

  if (data->is_mli_applicable) {
    data->mli_in = static_cast<mli_tensor*>(
        context->AllocatePersistentBuffer(context, sizeof(mli_tensor)));
    data->mli_out = static_cast<mli_tensor*>(
        context->AllocatePersistentBuffer(context, sizeof(mli_tensor)));
    data->cfg = static_cast<mli_pool_cfg*>(
        context->AllocatePersistentBuffer(context, sizeof(mli_pool_cfg)));

    ops::micro::ConvertToMliTensor(input, data->mli_in);
    ops::micro::ConvertToMliTensor(output, data->mli_out);

    data->cfg->kernel_width = params->filter_width;
    data->cfg->kernel_height = params->filter_height;
    data->cfg->stride_width = params->stride_width;
    data->cfg->stride_height = params->stride_height;

    if (params->padding == kTfLitePaddingValid) {
      data->cfg->padding_left = 0;
      data->cfg->padding_right = 0;
      data->cfg->padding_top = 0;
      data->cfg->padding_bottom = 0;
    } else {
      data->cfg->padding_left = data->padding.width;
      data->cfg->padding_right =
          data->padding.width + data->padding.width_offset;
      data->cfg->padding_top = data->padding.height;
      data->cfg->padding_bottom =
          data->padding.height + data->padding.height_offset;
    }
  }
  return kTfLiteOk;
}

void AverageEvalFloat(TfLiteContext* context, const TfLiteNode* node,
                      const TfLitePoolParams* params, const OpData& data,
                      const TfLiteEvalTensor* input, TfLiteEvalTensor* output) {
#if !defined(TF_LITE_STRIP_REFERENCE_IMPL)
  float activation_min, activation_max;
  CalculateActivationRange(params->activation, &activation_min,
                           &activation_max);

  PoolParams op_params;
  op_params.stride_height = params->stride_height;
  op_params.stride_width = params->stride_width;
  op_params.filter_height = params->filter_height;
  op_params.filter_width = params->filter_width;
  op_params.padding_values.height = data.padding.height;
  op_params.padding_values.width = data.padding.width;
  op_params.float_activation_min = activation_min;
  op_params.float_activation_max = activation_max;
  reference_ops::AveragePool(op_params, tflite::micro::GetTensorShape(input),
                             tflite::micro::GetTensorData<float>(input),
                             tflite::micro::GetTensorShape(output),
                             tflite::micro::GetTensorData<float>(output));
#else
  TF_LITE_KERNEL_LOG(context,
                     "Type %s (%d) is not supported by ARC MLI Library.",
                     TfLiteTypeGetName(input->type), input->type);
#endif
}

// Prepare MLI tensors and run Average or Max Pooling
TfLiteStatus EvalMli(TfLiteContext* context, const TfLitePoolParams* params,
                     const OpData& data, const TfLiteEvalTensor* input,
                     TfLiteEvalTensor* output,
                     const MliPoolingType pooling_type) {
  mli_pool_cfg cfg_local = *data.cfg;

  ops::micro::MliTensorAttachBuffer<int8_t>(input, data.mli_in);
  ops::micro::MliTensorAttachBuffer<int8_t>(output, data.mli_out);

  const int height_dimension = 1;
  int in_slice_height = 0;
  int out_slice_height = 0;
  const int overlap = cfg_local.kernel_height - cfg_local.stride_height;

  // Tensors for data in fast (local) memory and config to copy data from
  // external to local memory
  mli_tensor in_local = *data.mli_in;
  mli_tensor out_local = *data.mli_out;
  mli_mov_cfg_t copy_config;
  mli_mov_cfg_for_copy(&copy_config);
  TF_LITE_ENSURE_STATUS(get_arc_scratch_buffer_for_pooling_tensors(
      context, &in_local, &out_local));
  bool in_is_local = in_local.data == data.mli_in->data;
  bool out_is_local = out_local.data == data.mli_out->data;
  TF_LITE_ENSURE_STATUS(arc_scratch_buffer_calc_slice_size_io(
      &in_local, &out_local, cfg_local.kernel_height, cfg_local.stride_height,
      cfg_local.padding_top, cfg_local.padding_bottom, &in_slice_height,
      &out_slice_height));

  /* mli_in tensor contains batches of HWC tensors. so it is a 4 dimensional
     tensor. because the mli kernel will process one HWC tensor at a time, the 4
     dimensional tensor needs to be sliced into nBatch 3 dimensional tensors. on
     top of that there could be a need to also slice in the Height dimension.
     for that the sliceHeight has been calculated. The tensor slicer is
     configured that it will completely slice the nBatch dimension (0) and slice
     the height dimension (1) in chunks of 'sliceHeight' */
  TensorSlicer in_slice(data.mli_in, height_dimension, in_slice_height,
                        cfg_local.padding_top, cfg_local.padding_bottom,
                        overlap);
  TensorSlicer out_slice(data.mli_out, height_dimension, out_slice_height);

  /* is_local indicates that the tensor is already in local memory,
     so in that case the original tensor can be used,
     and there is no need to copy it to the local tensor*/
  mli_tensor* in_ptr = in_is_local ? in_slice.Sub() : &in_local;
  mli_tensor* out_ptr = out_is_local ? out_slice.Sub() : &out_local;

  while (!out_slice.Done()) {
    cfg_local.padding_top = in_slice.GetPaddingPre();
    cfg_local.padding_bottom = in_slice.GetPaddingPost();

    mli_mov_tensor_sync(in_slice.Sub(), &copy_config, in_ptr);
    if (pooling_type == AveragePooling)
      mli_krn_avepool_hwc_sa8(in_ptr, &cfg_local, out_ptr);
    else if (pooling_type == MaxPooling)
      mli_krn_maxpool_hwc_sa8(in_ptr, &cfg_local, out_ptr);
    mli_mov_tensor_sync(out_ptr, &copy_config, out_slice.Sub());

    in_slice.Next();
    out_slice.Next();
  }
  return kTfLiteOk;
}

void AverageEvalQuantized(TfLiteContext* context, const TfLiteNode* node,
                          const TfLitePoolParams* params, const OpData& data,
                          const TfLiteEvalTensor* input,
                          TfLiteEvalTensor* output) {
#if !defined(TF_LITE_STRIP_REFERENCE_IMPL)
  TFLITE_DCHECK(input->type == kTfLiteUInt8 || input->type == kTfLiteInt8);

  PoolParams op_params;
  op_params.stride_height = params->stride_height;
  op_params.stride_width = params->stride_width;
  op_params.filter_height = params->filter_height;
  op_params.filter_width = params->filter_width;
  op_params.padding_values.height = data.padding.height;
  op_params.padding_values.width = data.padding.width;
  op_params.quantized_activation_min = data.activation_min;
  op_params.quantized_activation_max = data.activation_max;

  if (input->type == kTfLiteUInt8) {
    reference_ops::AveragePool(op_params, tflite::micro::GetTensorShape(input),
                               tflite::micro::GetTensorData<uint8_t>(input),
                               tflite::micro::GetTensorShape(output),
                               tflite::micro::GetTensorData<uint8_t>(output));
  } else {
    reference_integer_ops::AveragePool(
        op_params, tflite::micro::GetTensorShape(input),
        tflite::micro::GetTensorData<int8_t>(input),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<int8_t>(output));
  }
#else
  TF_LITE_KERNEL_LOG(context,
                     "Type %s (%d) is not supported by ARC MLI Library.",
                     TfLiteTypeGetName(input->type), input->type);
#endif
}

void MaxEvalFloat(TfLiteContext* context, TfLiteNode* node,
                  TfLitePoolParams* params, const OpData& data,
                  const TfLiteEvalTensor* input, TfLiteEvalTensor* output) {
#if !defined(TF_LITE_STRIP_REFERENCE_IMPL)
  tflite::PoolParams op_params;
  op_params.stride_height = params->stride_height;
  op_params.stride_width = params->stride_width;
  op_params.filter_height = params->filter_height;
  op_params.filter_width = params->filter_width;
  op_params.padding_values.height = data.padding.height;
  op_params.padding_values.width = data.padding.width;
  op_params.float_activation_min = data.activation_min_f32;
  op_params.float_activation_max = data.activation_max_f32;
  reference_ops::MaxPool(op_params, tflite::micro::GetTensorShape(input),
                         tflite::micro::GetTensorData<float>(input),
                         tflite::micro::GetTensorShape(output),
                         tflite::micro::GetTensorData<float>(output));
#else
  TF_LITE_KERNEL_LOG(
      context,
      "Node configuration or type %s (%d) is not supported by ARC MLI Library.",
      TfLiteTypeGetName(input->type), input->type);
#endif
}

void MaxEvalQuantized(TfLiteContext* context, TfLiteNode* node,
                      TfLitePoolParams* params, const OpData& data,
                      const TfLiteEvalTensor* input, TfLiteEvalTensor* output) {
#if !defined(TF_LITE_STRIP_REFERENCE_IMPL)
  tflite::PoolParams op_params;
  op_params.stride_height = params->stride_height;
  op_params.stride_width = params->stride_width;
  op_params.filter_height = params->filter_height;
  op_params.filter_width = params->filter_width;
  op_params.padding_values.height = data.padding.height;
  op_params.padding_values.width = data.padding.width;
  op_params.quantized_activation_min = data.activation_min;
  op_params.quantized_activation_max = data.activation_max;

  if (input->type == kTfLiteUInt8) {
    reference_ops::MaxPool(op_params, tflite::micro::GetTensorShape(input),
                           tflite::micro::GetTensorData<uint8_t>(input),
                           tflite::micro::GetTensorShape(output),
                           tflite::micro::GetTensorData<uint8_t>(output));
  } else {
    reference_integer_ops::MaxPool(
        op_params, tflite::micro::GetTensorShape(input),
        tflite::micro::GetTensorData<int8_t>(input),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<int8_t>(output));
  }
#else
  TF_LITE_KERNEL_LOG(
      context,
      "Node configuration or type %s (%d) is not supported by ARC MLI Library.",
      TfLiteTypeGetName(input->type), input->type);
#endif
}
}  // namespace

TfLiteStatus AverageEval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kInputTensor);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  // Inputs and outputs share the same type, guaranteed by the converter.
  switch (input->type) {
    case kTfLiteFloat32:
      AverageEvalFloat(context, node, params, data, input, output);
      break;
    case kTfLiteUInt8:
    case kTfLiteInt8:
      if (data.is_mli_applicable) {
        EvalMli(context, params, data, input, output, AveragePooling);
      } else {
        AverageEvalQuantized(context, node, params, data, input, output);
      }
      break;
    default:
      TF_LITE_KERNEL_LOG(context, "Input type %s is not currently supported",
                         TfLiteTypeGetName(input->type));
      return kTfLiteError;
  }
  return kTfLiteOk;
}

TfLiteStatus MaxEval(TfLiteContext* context, TfLiteNode* node) {
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kInputTensor);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  switch (input->type) {
    case kTfLiteFloat32:
      MaxEvalFloat(context, node, params, data, input, output);
      break;
    case kTfLiteUInt8:
    case kTfLiteInt8:
      if (data.is_mli_applicable) {
        EvalMli(context, params, data, input, output, MaxPooling);
      } else {
        MaxEvalQuantized(context, node, params, data, input, output);
      }
      break;
    default:
      TF_LITE_KERNEL_LOG(context, "Type %s not currently supported.",
                         TfLiteTypeGetName(input->type));
      return kTfLiteError;
  }
  return kTfLiteOk;
}

}  // namespace pooling

TfLiteRegistration Register_AVERAGE_POOL_2D() {
  return {/*init=*/pooling::Init,
          /*free=*/nullptr,
          /*prepare=*/pooling::Prepare,
          /*invoke=*/pooling::AverageEval,
          /*profiling_string=*/nullptr,
          /*builtin_code=*/0,
          /*custom_name=*/nullptr,
          /*version=*/0};
}

TfLiteRegistration Register_MAX_POOL_2D() {
  return {/*init=*/pooling::Init,
          /*free=*/nullptr,
          /*prepare=*/pooling::Prepare,
          /*invoke=*/pooling::MaxEval,
          /*profiling_string=*/nullptr,
          /*builtin_code=*/0,
          /*custom_name=*/nullptr,
          /*version=*/0};
}

}  // namespace micro
}  // namespace ops
}  // namespace tflite

#else
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
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/pooling.h"

#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/integer_ops/pooling.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/padding.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"

namespace tflite {
namespace ops {
namespace micro {
namespace pooling {

namespace {

constexpr int kInputTensor = 0;
constexpr int kOutputTensor = 0;

struct OpData {
  TfLitePaddingValues padding;
  int32_t activation_min;
  int32_t activation_max;
  float activation_min_f32;
  float activation_max_f32;
};

TfLiteStatus CalculateOpData(const TfLiteContext* context,
                             const TfLitePoolParams* params,
                             const TfLiteTensor* input,
                             const TfLiteTensor* output, OpData* data) {
  // input: batch, height, width, channel
  int height = SizeOfDimension(input, 1);
  int width = SizeOfDimension(input, 2);

  int out_height, out_width;

  data->padding = ComputePaddingHeightWidth(
      params->stride_height, params->stride_width,
      /*dilation_rate_height=*/1,
      /*dilation_rate_width=*/1, height, width, params->filter_height,
      params->filter_width, params->padding, &out_height, &out_width);

  return kTfLiteOk;
}

void AverageEvalFloat(const TfLiteContext* context, const TfLiteNode* node,
                      const TfLitePoolParams* params, const OpData* data,
                      const TfLiteEvalTensor* input, TfLiteEvalTensor* output) {
  PoolParams op_params;
  op_params.stride_height = params->stride_height;
  op_params.stride_width = params->stride_width;
  op_params.filter_height = params->filter_height;
  op_params.filter_width = params->filter_width;
  op_params.padding_values.height = data->padding.height;
  op_params.padding_values.width = data->padding.width;
  op_params.float_activation_min = data->activation_min_f32;
  op_params.float_activation_max = data->activation_max_f32;
  reference_ops::AveragePool(op_params, tflite::micro::GetTensorShape(input),
                             tflite::micro::GetTensorData<float>(input),
                             tflite::micro::GetTensorShape(output),
                             tflite::micro::GetTensorData<float>(output));
}

void AverageEvalQuantized(TfLiteContext* context, const TfLiteNode* node,
                          const TfLitePoolParams* params, const OpData* data,
                          const TfLiteEvalTensor* input,
                          TfLiteEvalTensor* output) {
  TFLITE_DCHECK(input->type == kTfLiteUInt8 || input->type == kTfLiteInt8);

  PoolParams op_params;
  op_params.stride_height = params->stride_height;
  op_params.stride_width = params->stride_width;
  op_params.filter_height = params->filter_height;
  op_params.filter_width = params->filter_width;
  op_params.padding_values.height = data->padding.height;
  op_params.padding_values.width = data->padding.width;
  op_params.quantized_activation_min = data->activation_min;
  op_params.quantized_activation_max = data->activation_max;

  if (input->type == kTfLiteUInt8) {
    reference_ops::AveragePool(op_params, tflite::micro::GetTensorShape(input),
                               tflite::micro::GetTensorData<uint8_t>(input),
                               tflite::micro::GetTensorShape(output),
                               tflite::micro::GetTensorData<uint8_t>(output));
  } else {
    reference_integer_ops::AveragePool(
        op_params, tflite::micro::GetTensorShape(input),
        tflite::micro::GetTensorData<int8_t>(input),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<int8_t>(output));
  }
}

void MaxEvalFloat(TfLiteContext* context, TfLiteNode* node,
                  TfLitePoolParams* params, const OpData* data,
                  const TfLiteEvalTensor* input, TfLiteEvalTensor* output) {
  tflite::PoolParams op_params;
  op_params.stride_height = params->stride_height;
  op_params.stride_width = params->stride_width;
  op_params.filter_height = params->filter_height;
  op_params.filter_width = params->filter_width;
  op_params.padding_values.height = data->padding.height;
  op_params.padding_values.width = data->padding.width;
  op_params.float_activation_min = data->activation_min_f32;
  op_params.float_activation_max = data->activation_max_f32;
  reference_ops::MaxPool(op_params, tflite::micro::GetTensorShape(input),
                         tflite::micro::GetTensorData<float>(input),
                         tflite::micro::GetTensorShape(output),
                         tflite::micro::GetTensorData<float>(output));
}

void MaxEvalQuantized(TfLiteContext* context, TfLiteNode* node,
                      TfLitePoolParams* params, const OpData* data,
                      const TfLiteEvalTensor* input, TfLiteEvalTensor* output) {
  tflite::PoolParams op_params;
  op_params.stride_height = params->stride_height;
  op_params.stride_width = params->stride_width;
  op_params.filter_height = params->filter_height;
  op_params.filter_width = params->filter_width;
  op_params.padding_values.height = data->padding.height;
  op_params.padding_values.width = data->padding.width;
  op_params.quantized_activation_min = data->activation_min;
  op_params.quantized_activation_max = data->activation_max;

  if (input->type == kTfLiteUInt8) {
    reference_ops::MaxPool(op_params, tflite::micro::GetTensorShape(input),
                           tflite::micro::GetTensorData<uint8_t>(input),
                           tflite::micro::GetTensorShape(output),
                           tflite::micro::GetTensorData<uint8_t>(output));
  } else {
    reference_integer_ops::MaxPool(
        op_params, tflite::micro::GetTensorShape(input),
        tflite::micro::GetTensorData<int8_t>(input),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<int8_t>(output));
  }
}
}  // namespace

TfLiteStatus AverageEval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData* data = static_cast<const OpData*>(node->user_data);

  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kInputTensor);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);

  // Inputs and outputs share the same type, guaranteed by the converter.
  switch (input->type) {
    case kTfLiteFloat32:
      AverageEvalFloat(context, node, params, data, input, output);
      break;
    case kTfLiteUInt8:
    case kTfLiteInt8:
      AverageEvalQuantized(context, node, params, data, input, output);
      break;
    default:
      TF_LITE_KERNEL_LOG(context, "Input type %s is not currently supported",
                         TfLiteTypeGetName(input->type));
      return kTfLiteError;
  }
  return kTfLiteOk;
}

TfLiteStatus MaxEval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData* data = static_cast<const OpData*>(node->user_data);

  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kInputTensor);
  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);

  switch (input->type) {
    case kTfLiteFloat32:
      MaxEvalFloat(context, node, params, data, input, output);
      break;
    case kTfLiteUInt8:
    case kTfLiteInt8:
      MaxEvalQuantized(context, node, params, data, input, output);
      break;
    default:
      TF_LITE_KERNEL_LOG(context, "Type %s not currently supported.",
                         TfLiteTypeGetName(input->type));
      return kTfLiteError;
  }
  return kTfLiteOk;
}

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpData));
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  OpData* data = static_cast<OpData*>(node->user_data);

  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  TF_LITE_ENSURE(context, input != nullptr);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);
  TF_LITE_ENSURE(context, output != nullptr);

  TF_LITE_ENSURE_STATUS(CalculateOpData(context, params, input, output, data));

  if (input->type == kTfLiteFloat32) {
    CalculateActivationRange(params->activation, &data->activation_min_f32,
                             &data->activation_max_f32);
  } else if (input->type == kTfLiteInt8 || input->type == kTfLiteUInt8) {
    CalculateActivationRangeQuantized(context, params->activation, output,
                                      &data->activation_min,
                                      &data->activation_max);
  }

  return kTfLiteOk;
}

}  // namespace pooling

TfLiteRegistration Register_AVERAGE_POOL_2D() {
  return {/*init=*/pooling::Init,
          /*free=*/nullptr,
          /*prepare=*/pooling::Prepare,
          /*invoke=*/pooling::AverageEval,
          /*profiling_string=*/nullptr,
          /*builtin_code=*/0,
          /*custom_name=*/nullptr,
          /*version=*/0};
}

TfLiteRegistration Register_MAX_POOL_2D() {
  return {/*init=*/pooling::Init,
          /*free=*/nullptr,
          /*prepare=*/pooling::Prepare,
          /*invoke=*/pooling::MaxEval,
          /*profiling_string=*/nullptr,
          /*builtin_code=*/0,
          /*custom_name=*/nullptr,
          /*version=*/0};
}

}  // namespace micro
}  // namespace ops
}  // namespace tflite

#endif
