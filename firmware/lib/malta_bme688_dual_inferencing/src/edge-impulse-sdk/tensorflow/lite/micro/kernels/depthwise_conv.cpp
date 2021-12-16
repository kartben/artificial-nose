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

#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/depthwise_conv.h"

#include "edge-impulse-sdk/CMSIS/NN/Include/arm_nnfunctions.h"
#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/c/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/quantization_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/depthwiseconv_float.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/depthwiseconv_uint8.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/integer_ops/depthwise_conv.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/padding.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/conv.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"

namespace tflite {
namespace {

struct OpData {
  OpDataConv reference_op_data;

  // Index to buffer for optimizations if applicable.
  int buffer_idx;
};

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpData));
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  OpData* data = static_cast<OpData*>(node->user_data);
  const auto& params =
      *(reinterpret_cast<TfLiteDepthwiseConvParams*>(node->builtin_data));

  const TfLiteTensor* input =
      GetInput(context, node, kDepthwiseConvInputTensor);
  TF_LITE_ENSURE(context, input != nullptr);
  const TfLiteTensor* filter =
      GetInput(context, node, kDepthwiseConvWeightsTensor);
  TF_LITE_ENSURE(context, filter != nullptr);
  TfLiteTensor* output = GetOutput(context, node, kDepthwiseConvOutputTensor);
  TF_LITE_ENSURE(context, output != nullptr);

  const TfLiteType data_type = input->type;
  int input_width = SizeOfDimension(input, 2);
  int input_height = SizeOfDimension(input, 1);
  int filter_width = SizeOfDimension(filter, 2);
  int filter_height = SizeOfDimension(filter, 1);
  int output_width = SizeOfDimension(output, 2);
  int output_height = SizeOfDimension(output, 1);

  if (input->type == kTfLiteInt8) {
    TF_LITE_ENSURE_EQ(context, filter->quantization.type,
                      kTfLiteAffineQuantization);

    // All per-channel quantized tensors need valid zero point and scale arrays.
    const auto* affine_quantization =
        reinterpret_cast<TfLiteAffineQuantization*>(
            filter->quantization.params);
    TF_LITE_ENSURE(context, affine_quantization);
    TF_LITE_ENSURE(context, affine_quantization->scale);
    TF_LITE_ENSURE(context, affine_quantization->zero_point);
    TF_LITE_ENSURE(
        context, affine_quantization->scale->size == 1 ||
                     affine_quantization->scale->size ==
                         filter->dims->data[kDepthwiseConvQuantizedDimension]);
    TF_LITE_ENSURE_EQ(context, affine_quantization->scale->size,
                      affine_quantization->zero_point->size);
  }

  // Allocate memory for per-channel quantization parameters
  const int num_channels = filter->dims->data[kDepthwiseConvQuantizedDimension];

  data->reference_op_data.per_channel_output_multiplier =
      reinterpret_cast<int32_t*>(context->AllocatePersistentBuffer(
          context, num_channels * sizeof(int32_t)));
  data->reference_op_data.per_channel_output_shift =
      reinterpret_cast<int32_t*>(context->AllocatePersistentBuffer(
          context, num_channels * sizeof(int32_t)));

  TF_LITE_ENSURE_STATUS(CalculateOpDataDepthwiseConv(
      context, node, params, input_width, input_height, filter_width,
      filter_height, output_width, output_height, data_type,
      &data->reference_op_data));

  if (input->type == kTfLiteInt8) {
    RuntimeShape input_shape = GetTensorShape(input);
    RuntimeShape output_shape = GetTensorShape(output);
    RuntimeShape filter_shape = GetTensorShape(filter);
    TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);
    TFLITE_DCHECK_EQ(filter_shape.DimensionsCount(), 4);
    TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);

    const int batch_size = MatchingDim(input_shape, 0, output_shape, 0);
    const int output_depth = MatchingDim(output_shape, 3, filter_shape, 3);
    TFLITE_DCHECK_EQ(batch_size, 1); /* Only batch = 1 is supported */

    cmsis_nn_dims input_dims;
    input_dims.n = batch_size;
    input_dims.h = input_height;
    input_dims.w = input_width;
    input_dims.c = input_shape.Dims(3);

    cmsis_nn_dims filter_dims;
    filter_dims.n = 1;
    filter_dims.h = filter_height;
    filter_dims.w = filter_width;
    filter_dims.c = output_depth;

    cmsis_nn_dims output_dims;
    output_dims.n = batch_size;
    output_dims.h = output_height;
    output_dims.w = output_width;
    output_dims.c = output_depth;

    cmsis_nn_dw_conv_params dw_conv_params;
    dw_conv_params.padding.h = data->reference_op_data.padding.height;
    dw_conv_params.padding.w = data->reference_op_data.padding.width;

    const int32_t buf_size = arm_depthwise_conv_wrapper_s8_get_buffer_size(
        &dw_conv_params, &input_dims, &filter_dims, &output_dims);

    if (buf_size > 0) {
      TF_LITE_ENSURE_STATUS(context->RequestScratchBufferInArena(
          context, buf_size, &data->buffer_idx));
    } else {
      data->buffer_idx = -1;
    }
  }
  return kTfLiteOk;
}

void EvalQuantizedPerChannel(TfLiteContext* context, TfLiteNode* node,
                             const TfLiteDepthwiseConvParams& params,
                             const OpData& data, const TfLiteEvalTensor* input,
                             const TfLiteEvalTensor* filter,
                             const TfLiteEvalTensor* bias,
                             TfLiteEvalTensor* output) {
  cmsis_nn_dw_conv_params dw_conv_params;
  dw_conv_params.dilation.h = params.dilation_height_factor;
  dw_conv_params.dilation.w = params.dilation_width_factor;
  // Call to reference implementation can be removed when dilation is supported
  // in the optimized implementations.
  if (1 == dw_conv_params.dilation.h && 1 == dw_conv_params.dilation.w) {
    dw_conv_params.input_offset = -data.reference_op_data.input_zero_point;
    dw_conv_params.output_offset = data.reference_op_data.output_zero_point;
    dw_conv_params.stride.h = params.stride_height;
    dw_conv_params.stride.w = params.stride_width;
    dw_conv_params.padding.h = data.reference_op_data.padding.height;
    dw_conv_params.padding.w = data.reference_op_data.padding.width;
    // TODO(b/130439627): Use calculated value for clamping.
    dw_conv_params.activation.min = std::numeric_limits<int8_t>::min();
    dw_conv_params.activation.max = std::numeric_limits<int8_t>::max();
    dw_conv_params.ch_mult = params.depth_multiplier;

    cmsis_nn_per_channel_quant_params quant_params;
    quant_params.multiplier =
        data.reference_op_data.per_channel_output_multiplier;
    quant_params.shift = data.reference_op_data.per_channel_output_shift;

    RuntimeShape filter_shape = tflite::micro::GetTensorShape(filter);
    RuntimeShape input_shape = tflite::micro::GetTensorShape(input);
    RuntimeShape output_shape = tflite::micro::GetTensorShape(output);
    RuntimeShape bias_shape = tflite::micro::GetTensorShape(bias);

    TFLITE_DCHECK_LE(dw_conv_params.activation.min,
                     dw_conv_params.activation.max);

    const int batch_size = MatchingDim(input_shape, 0, output_shape, 0);
    const int output_depth = MatchingDim(filter_shape, 3, output_shape, 3);

    if (tflite::micro::GetTensorData<int8_t>(bias)) {
      TFLITE_DCHECK_EQ(bias_shape.FlatSize(), output_depth);
    }

    cmsis_nn_dims input_dims;
    input_dims.n = batch_size;
    input_dims.h = input_shape.Dims(1);
    input_dims.w = input_shape.Dims(2);
    input_dims.c = input_shape.Dims(3);

    cmsis_nn_dims filter_dims;
    filter_dims.n = filter_shape.Dims(0);
    filter_dims.h = filter_shape.Dims(1);
    filter_dims.w = filter_shape.Dims(2);
    filter_dims.c = output_depth;

    cmsis_nn_dims bias_dims;
    bias_dims.n = 1;
    bias_dims.h = 1;
    bias_dims.w = 1;
    bias_dims.c = output_depth;

    cmsis_nn_dims output_dims;
    output_dims.n = batch_size;
    output_dims.h = output_shape.Dims(1);
    output_dims.w = output_shape.Dims(2);
    output_dims.c = output_depth;

    cmsis_nn_context ctx;
    ctx.buf = nullptr;
    /* 'size' is unused */
    ctx.size = 0;

    if (data.buffer_idx > -1) {
      ctx.buf = context->GetScratchBuffer(context, data.buffer_idx);
    }

    TFLITE_DCHECK_EQ(
        arm_depthwise_conv_wrapper_s8(
            &ctx, &dw_conv_params, &quant_params, &input_dims,
            tflite::micro::GetTensorData<int8_t>(input), &filter_dims,
            tflite::micro::GetTensorData<int8_t>(filter), &bias_dims,
            tflite::micro::GetTensorData<int32_t>(bias), &output_dims,
            tflite::micro::GetTensorData<int8_t>(output)),
        ARM_MATH_SUCCESS);
  } else {
    reference_integer_ops::DepthwiseConvPerChannel(
        DepthwiseConvParamsQuantized(params, data.reference_op_data),
        data.reference_op_data.per_channel_output_multiplier,
        data.reference_op_data.per_channel_output_shift,
        tflite::micro::GetTensorShape(input),
        tflite::micro::GetTensorData<int8_t>(input),
        tflite::micro::GetTensorShape(filter),
        tflite::micro::GetTensorData<int8_t>(filter),
        tflite::micro::GetTensorShape(bias),
        tflite::micro::GetTensorData<int32_t>(bias),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<int8_t>(output));
  }
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  const auto& params =
      *(reinterpret_cast<TfLiteDepthwiseConvParams*>(node->builtin_data));
  const OpData& data = *(static_cast<OpData*>(node->user_data));

  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kDepthwiseConvOutputTensor);
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kDepthwiseConvInputTensor);
  const TfLiteEvalTensor* filter =
      tflite::micro::GetEvalInput(context, node, kDepthwiseConvWeightsTensor);
  const TfLiteEvalTensor* bias =
      (NumInputs(node) == 3)
          ? tflite::micro::GetEvalInput(context, node, kDepthwiseConvBiasTensor)
          : nullptr;

  switch (input->type) {  // Already know in/out types are same.
    case kTfLiteFloat32: {
      tflite::reference_ops::DepthwiseConv(
          DepthwiseConvParamsFloat(params, data.reference_op_data),
          tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<float>(input),
          tflite::micro::GetTensorShape(filter),
          tflite::micro::GetTensorData<float>(filter),
          tflite::micro::GetTensorShape(bias),
          tflite::micro::GetTensorData<float>(bias),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<float>(output));
      break;
    }
    case kTfLiteInt8:
      EvalQuantizedPerChannel(context, node, params, data, input, filter, bias,
                              output);
      break;
    default:
      TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                         TfLiteTypeGetName(input->type), input->type);
      return kTfLiteError;
  }
  return kTfLiteOk;
}

}  // namespace

TfLiteRegistration Register_DEPTHWISE_CONV_2D() {
  return {/*init=*/Init,
          /*free=*/nullptr,
          /*prepare=*/Prepare,
          /*invoke=*/Eval,
          /*profiling_string=*/nullptr,
          /*builtin_code=*/0,
          /*custom_name=*/nullptr,
          /*version=*/0};
}

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

#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/integer_ops/depthwise_conv.h"

#include "mli_api.h"  // NOLINT
#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/c/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/quantization_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/depthwiseconv_float.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/depthwiseconv_uint8.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/padding.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/mli_slicers.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/mli_tf_utils.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/scratch_buf_mgr.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/scratch_buffers.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"

namespace tflite {
namespace {

constexpr int kInputTensor = 0;
constexpr int kFilterTensor = 1;
constexpr int kBiasTensor = 2;
constexpr int kOutputTensor = 0;

// Depthwise conv is quantized along dimension 3:
// https://www.tensorflow.org/lite/performance/quantization_spec
constexpr int kDepthwiseConvQuantizedDimension = 3;

struct OpData {
  TfLitePaddingValues padding;

  // Cached tensor zero point values for quantized operations.
  int32_t input_zero_point;
  int32_t filter_zero_point;
  int32_t output_zero_point;

  // The scaling factor from input to output (aka the 'real multiplier') can
  // be represented as a fixed point multiplier plus a left shift.
  int32_t output_multiplier;
  int output_shift;

  // Per channel output multiplier and shift.
  int32_t* per_channel_output_multiplier;
  int32_t* per_channel_output_shift;

  // The range of the fused activation layer. For example for kNone and
  // uint8_t these would be 0 and 255.
  int32_t output_activation_min;
  int32_t output_activation_max;

  // The result of checking if MLI optimized version of tensors can be used.
  bool is_mli_applicable;

  // Tensors in MLI format.
  mli_tensor* mli_in;
  mli_tensor* mli_weights;
  mli_tensor* mli_bias;
  mli_tensor* mli_out;
  mli_conv2d_cfg* cfg;
};

bool IsMliApplicable(TfLiteContext* context, const TfLiteTensor* input,
                     const TfLiteTensor* filter, const TfLiteTensor* bias,
                     const TfLiteDepthwiseConvParams* params) {
  const auto* affine_quantization =
      reinterpret_cast<TfLiteAffineQuantization*>(filter->quantization.params);
  const int in_ch = SizeOfDimension(input, 3);
  const int filters_num = SizeOfDimension(filter, 3);

  // MLI optimized version only supports int8_t datatype, dilation factor of 1
  // and per-axis quantization of weights (no broadcasting/per-tensor) (in_ch ==
  // filters_num) || (in_ch == 1)) is a forbidding of channel multiplier logic
  // for multichannel input.
  bool ret_val = (filter->type == kTfLiteInt8) &&
                 (input->type == kTfLiteInt8) && (bias->type == kTfLiteInt32) &&
                 (params->dilation_width_factor == 1) &&
                 (params->dilation_height_factor == 1) &&
                 (affine_quantization->scale->size ==
                  filter->dims->data[kDepthwiseConvQuantizedDimension]) &&
                 ((in_ch == filters_num) || (in_ch == 1));
  return ret_val;
}

TfLiteStatus CalculateOpData(TfLiteContext* context, TfLiteNode* node,
                             TfLiteDepthwiseConvParams* params, int width,
                             int height, int filter_width, int filter_height,
                             const TfLiteType data_type, OpData* data) {
  bool has_bias = node->inputs->size == 3;
  // Check number of inputs/outputs
  TF_LITE_ENSURE(context, has_bias || node->inputs->size == 2);
  TF_LITE_ENSURE_EQ(context, node->outputs->size, 1);

  int unused_output_height, unused_output_width;
  data->padding = ComputePaddingHeightWidth(
      params->stride_height, params->stride_width, 1, 1, height, width,
      filter_height, filter_width, params->padding, &unused_output_height,
      &unused_output_width);

  // Note that quantized inference requires that all tensors have their
  // parameters set. This is usually done during quantized training.
#if !defined(TF_LITE_STRIP_REFERENCE_IMPL)
  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  const TfLiteTensor* filter = GetInput(context, node, kFilterTensor);
  const TfLiteTensor* bias = GetOptionalInputTensor(context, node, kBiasTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

  if (data_type != kTfLiteFloat32 && !data->is_mli_applicable) {
    int num_channels = filter->dims->data[kDepthwiseConvQuantizedDimension];

    return tflite::PopulateConvolutionQuantizationParams(
        context, input, filter, bias, output, params->activation,
        &data->output_multiplier, &data->output_shift,
        &data->output_activation_min, &data->output_activation_max,
        data->per_channel_output_multiplier,
        reinterpret_cast<int*>(data->per_channel_output_shift), num_channels);
  }
#endif
  return kTfLiteOk;
}

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpData));
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  auto* params =
      reinterpret_cast<TfLiteDepthwiseConvParams*>(node->builtin_data);
  OpData* data = static_cast<OpData*>(node->user_data);

  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);
  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  const TfLiteTensor* filter = GetInput(context, node, kFilterTensor);
  const TfLiteTensor* bias = GetOptionalInputTensor(context, node, kBiasTensor);

  const TfLiteType data_type = input->type;
  int width = SizeOfDimension(input, 2);
  int height = SizeOfDimension(input, 1);
  int filter_width = SizeOfDimension(filter, 2);
  int filter_height = SizeOfDimension(filter, 1);

  // Per channel quantization is only needed for int8 inference. For other
  // quantized types, only a single scale and zero point is needed.
  const int num_channels = filter->dims->data[kDepthwiseConvQuantizedDimension];
  // Dynamically allocate per-channel quantization parameters.
  data->per_channel_output_multiplier =
      reinterpret_cast<int32_t*>(context->AllocatePersistentBuffer(
          context, num_channels * sizeof(int32_t)));
  data->per_channel_output_shift =
      reinterpret_cast<int32_t*>(context->AllocatePersistentBuffer(
          context, num_channels * sizeof(int32_t)));

  data->is_mli_applicable =
      IsMliApplicable(context, input, filter, bias, params);

  // All per-channel quantized tensors need valid zero point and scale arrays.
  if (input->type == kTfLiteInt8) {
    TF_LITE_ENSURE_EQ(context, filter->quantization.type,
                      kTfLiteAffineQuantization);

    const auto* affine_quantization =
        reinterpret_cast<TfLiteAffineQuantization*>(
            filter->quantization.params);
    TF_LITE_ENSURE(context, affine_quantization);
    TF_LITE_ENSURE(context, affine_quantization->scale);
    TF_LITE_ENSURE(context, affine_quantization->zero_point);
    TF_LITE_ENSURE(
        context, affine_quantization->scale->size == 1 ||
                     affine_quantization->scale->size ==
                         filter->dims->data[kDepthwiseConvQuantizedDimension]);
    TF_LITE_ENSURE_EQ(context, affine_quantization->scale->size,
                      affine_quantization->zero_point->size);
  }

  TF_LITE_ENSURE_STATUS(CalculateOpData(context, node, params, width, height,
                                        filter_width, filter_height, data_type,
                                        data));

  data->input_zero_point = input->params.zero_point;
  data->filter_zero_point = filter->params.zero_point;
  data->output_zero_point = output->params.zero_point;

  if (data->is_mli_applicable) {
    data->mli_in = static_cast<mli_tensor*>(
        context->AllocatePersistentBuffer(context, sizeof(mli_tensor)));
    data->mli_weights = static_cast<mli_tensor*>(
        context->AllocatePersistentBuffer(context, sizeof(mli_tensor)));
    data->mli_bias = static_cast<mli_tensor*>(
        context->AllocatePersistentBuffer(context, sizeof(mli_tensor)));
    data->mli_out = static_cast<mli_tensor*>(
        context->AllocatePersistentBuffer(context, sizeof(mli_tensor)));
    data->cfg = static_cast<mli_conv2d_cfg*>(
        context->AllocatePersistentBuffer(context, sizeof(mli_conv2d_cfg)));

    // reuse space allocated for OpData parameters
    data->mli_weights->el_params.asym.scale.pi32 =
        static_cast<int32_t*>(data->per_channel_output_multiplier);
    data->mli_bias->el_params.asym.scale.pi32 =
        static_cast<int32_t*>(data->per_channel_output_shift);

    data->mli_weights->el_params.asym.zero_point.pi16 =
        reinterpret_cast<int16_t*>(&data->filter_zero_point);
    data->mli_bias->el_params.asym.zero_point.pi16 =
        reinterpret_cast<int16_t*>(&data->filter_zero_point) + sizeof(int16_t);

    ops::micro::ConvertToMliTensor(input, data->mli_in);
    ops::micro::ConvertToMliTensorPerChannel(filter, data->mli_weights);
    ops::micro::ConvertToMliTensorPerChannel(bias, data->mli_bias);
    ops::micro::ConvertToMliTensor(output, data->mli_out);

    if (params->activation == kTfLiteActRelu) {
      data->cfg->relu.type = MLI_RELU_GEN;
    } else if (params->activation == kTfLiteActRelu6) {
      data->cfg->relu.type = MLI_RELU_6;
    } else if (params->activation == kTfLiteActReluN1To1) {
      data->cfg->relu.type = MLI_RELU_1;
    } else {
      data->cfg->relu.type = MLI_RELU_NONE;
    }

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

void EvalFloat(TfLiteContext* context, TfLiteNode* node,
               TfLiteDepthwiseConvParams* params, const OpData& data,
               const TfLiteEvalTensor* input, const TfLiteEvalTensor* filter,
               const TfLiteEvalTensor* bias, TfLiteEvalTensor* output) {
#if !defined(TF_LITE_STRIP_REFERENCE_IMPL)
  float output_activation_min, output_activation_max;
  CalculateActivationRange(params->activation, &output_activation_min,
                           &output_activation_max);

  tflite::DepthwiseParams op_params;
  // Padding type is ignored, but still set.
  op_params.padding_type = PaddingType::kSame;
  op_params.padding_values.width = data.padding.width;
  op_params.padding_values.height = data.padding.height;
  op_params.stride_width = params->stride_width;
  op_params.stride_height = params->stride_height;
  op_params.dilation_width_factor = params->dilation_width_factor;
  op_params.dilation_height_factor = params->dilation_height_factor;
  op_params.depth_multiplier = params->depth_multiplier;
  op_params.float_activation_min = output_activation_min;
  op_params.float_activation_max = output_activation_max;

  tflite::reference_ops::DepthwiseConv(
      op_params, tflite::micro::GetTensorShape(input),
      tflite::micro::GetTensorData<float>(input),
      tflite::micro::GetTensorShape(filter),
      tflite::micro::GetTensorData<float>(filter),
      tflite::micro::GetTensorShape(bias),
      tflite::micro::GetTensorData<float>(bias),
      tflite::micro::GetTensorShape(output),
      tflite::micro::GetTensorData<float>(output));
#else
  TF_LITE_KERNEL_LOG(context,
                     "Type %s (%d) is not supported by ARC MLI Library.",
                     TfLiteTypeGetName(input->type), input->type);
#endif
}
TfLiteStatus EvalMliQuantizedPerChannel(
    TfLiteContext* context, TfLiteNode* node, TfLiteDepthwiseConvParams* params,
    const OpData& data, const TfLiteEvalTensor* input,
    const TfLiteEvalTensor* filter, const TfLiteEvalTensor* bias,
    TfLiteEvalTensor* output) {
  // Run Depthwise Conv MLI kernel
  // MLI optimized version only supports int8_t dataype and dilation factor of 1
  if (data.is_mli_applicable) {
    // Copy configuration data from external to local memory
    mli_conv2d_cfg cfg_local = *data.cfg;

    ops::micro::MliTensorAttachBuffer<int8_t>(input, data.mli_in);
    ops::micro::MliTensorAttachBuffer<int8_t>(filter, data.mli_weights);
    ops::micro::MliTensorAttachBuffer<int32_t>(bias, data.mli_bias);
    ops::micro::MliTensorAttachBuffer<int8_t>(output, data.mli_out);

    // for height slicing
    const int heightDimension = 1;
    int inSliceHeight = 0;
    int outSliceHeight = 0;
    const int kernelHeight =
        static_cast<int>(data.mli_weights->shape[KRNL_DW_H_DIM_HWC]);
    const int overlap = kernelHeight - cfg_local.stride_height;

    // for weight slicing (on output channels)
    // HWCN layout for weights, output channel dimension is the first dimension.
    const int weight_out_ch_dimension = 3;
    // bias has only 1 dimension
    const int bias_out_ch_dimension = 0;
    // Batch-Height-Width-Channel layout means last dimension is output
    // channels.
    const int out_tensor_ch_dimension = 3;
    const int32_t in_channels = data.mli_in->shape[out_tensor_ch_dimension];
    const int32_t out_channels = data.mli_out->shape[out_tensor_ch_dimension];
    int slice_channels =
        static_cast<int>(data.mli_weights->shape[weight_out_ch_dimension]);

    // Tensors for data in fast (local) memory
    // and config to copy data from external to local memory
    mli_tensor weights_local = *data.mli_weights;
    mli_tensor bias_local = *data.mli_bias;
    mli_tensor in_local = *data.mli_in;
    mli_tensor out_local =
        *data.mli_out;  // this assumes that output shape
                        // is already filled in the tensor struct.
    mli_mov_cfg_t copy_config;
    mli_mov_cfg_for_copy(&copy_config);

    TF_LITE_ENSURE_STATUS(ops::micro::get_arc_scratch_buffer_for_conv_tensors(
        context, &in_local, &weights_local, &bias_local, &out_local));
    /* is_local indicates that the tensor is already in local memory,
     so in that case the original tensor can be used,
     and there is no need to copy it to the local tensor*/
    const bool in_is_local = in_local.data == data.mli_in->data;
    const bool out_is_local = out_local.data == data.mli_out->data;
    const bool w_is_local = weights_local.data == data.mli_weights->data;
    const bool b_is_local = bias_local.data == data.mli_bias->data;

    TF_LITE_ENSURE_STATUS(ops::micro::arc_scratch_buffer_calc_slice_size_io(
        &in_local, &out_local, kernelHeight, cfg_local.stride_height,
        cfg_local.padding_top, cfg_local.padding_bottom, &inSliceHeight,
        &outSliceHeight));
    TF_LITE_ENSURE_STATUS(
        ops::micro::arc_scratch_buffer_calc_slice_size_weights(
            &weights_local, &bias_local, weight_out_ch_dimension,
            &slice_channels));

    /* if input channels is not equal to output channels, a channel multiplier
       is used. in this case the slice channels needs to be rounded down to a
       multiple of the input channels */
    if (in_channels != out_channels) {
      slice_channels = (slice_channels / in_channels) * in_channels;
    }

    ops::micro::TensorSlicer b_slice(data.mli_bias, bias_out_ch_dimension,
                                     slice_channels);
    ops::micro::TensorSlicer w_slice(data.mli_weights, weight_out_ch_dimension,
                                     slice_channels, 0, 0, 0, true);
    ops::micro::TensorSlicer out_ch_slice(data.mli_out, out_tensor_ch_dimension,
                                          slice_channels, 0, 0, 0, true);
    ops::micro::TensorSlicer in_ch_slice(data.mli_in, out_tensor_ch_dimension,
                                         slice_channels, 0, 0, 0, true);

    mli_tensor* w_ptr = w_is_local ? w_slice.Sub() : &weights_local;
    mli_tensor* b_ptr = b_is_local ? b_slice.Sub() : &bias_local;

    void* input_buffer_ptr = NULL;
    uint32_t input_buffer_size = 0;
    int padding_top = cfg_local.padding_top;
    int padding_bottom = cfg_local.padding_bottom;

    while (!w_slice.Done()) {
      mli_mov_tensor_sync(w_slice.Sub(), &copy_config, w_ptr);
      mli_mov_tensor_sync(b_slice.Sub(), &copy_config, b_ptr);

      /* input tensor is already sliced in the  channel dimension.
      out_ch_slice.Sub() is the tensor for the amount of channels of this
      iteration of the weight slice loop. This tensor needs to be further
      sliced over the batch and height dimension. in_ch_slice.Sub() tensor
      contains batches of HWC tensors. so it is a 4 dimensional tensor. because
      the mli kernel will process one HWC tensor at a time, the 4 dimensional
      tensor needs to be sliced into nBatch 3 dimensional tensors. on top of
      that there could be a need to also slice in the Height dimension. for that
      the sliceHeight has been calculated. The tensor slicer is configured that
      it will completely slice the nBatch dimension (0) and slice the height
      dimension (1) in chunks of 'sliceHeight' */
      ops::micro::TensorSlicer in_slice(in_ch_slice.Sub(), heightDimension,
                                        inSliceHeight, padding_top,
                                        padding_bottom, overlap);

      /* output tensor is already sliced in the output channel dimension.
      out_ch_slice.Sub() is the tensor for the amount of output channels of this
      iteration of the weight slice loop. This tensor needs to be further
      sliced over the batch and height dimension. */
      ops::micro::TensorSlicer out_slice(out_ch_slice.Sub(), heightDimension,
                                         outSliceHeight);

      /* setup the pointers to the local or remote tensor to make the code
       * inside the loop easier. */
      mli_tensor* in_ptr = in_is_local ? in_slice.Sub() : &in_local;
      mli_tensor* out_ptr = out_is_local ? out_slice.Sub() : &out_local;

      while (!out_slice.Done()) {
        TF_LITE_ENSURE(context, !in_slice.Done());
        cfg_local.padding_top = in_slice.GetPaddingPre();
        cfg_local.padding_bottom = in_slice.GetPaddingPost();

        // if same input copy as previous iteration, skip the copy of input
        if ((in_slice.Sub()->data != input_buffer_ptr) ||
            (mli_hlp_count_elem_num(in_slice.Sub(), 0) != input_buffer_size)) {
          mli_mov_tensor_sync(in_slice.Sub(), &copy_config, in_ptr);
          input_buffer_ptr = in_slice.Sub()->data;
          input_buffer_size = mli_hlp_count_elem_num(in_slice.Sub(), 0);
        }
        mli_krn_depthwise_conv2d_hwcn_sa8_sa8_sa32(in_ptr, w_ptr, b_ptr,
                                                   &cfg_local, out_ptr);
        mli_mov_tensor_sync(out_ptr, &copy_config, out_slice.Sub());

        in_slice.Next();
        out_slice.Next();
      }
      w_slice.Next();
      b_slice.Next();
      out_ch_slice.Next();
      in_ch_slice.Next();
      TF_LITE_ENSURE(context, in_slice.Done());
    }
  }
  return kTfLiteOk;
}

void EvalQuantizedPerChannel(TfLiteContext* context, TfLiteNode* node,
                             TfLiteDepthwiseConvParams* params,
                             const OpData& data, const TfLiteEvalTensor* input,
                             const TfLiteEvalTensor* filter,
                             const TfLiteEvalTensor* bias,
                             TfLiteEvalTensor* output) {
#if !defined(TF_LITE_STRIP_REFERENCE_IMPL)
  DepthwiseParams op_params;
  op_params.padding_type = PaddingType::kSame;
  op_params.padding_values.width = data.padding.width;
  op_params.padding_values.height = data.padding.height;
  op_params.stride_width = params->stride_width;
  op_params.stride_height = params->stride_height;
  op_params.dilation_width_factor = params->dilation_width_factor;
  op_params.dilation_height_factor = params->dilation_height_factor;
  op_params.depth_multiplier = params->depth_multiplier;
  op_params.input_offset = -data.input_zero_point;
  op_params.weights_offset = 0;
  op_params.output_offset = data.output_zero_point;
  op_params.quantized_activation_min = std::numeric_limits<int8_t>::min();
  op_params.quantized_activation_max = std::numeric_limits<int8_t>::max();

  reference_integer_ops::DepthwiseConvPerChannel(
      op_params, data.per_channel_output_multiplier,
      data.per_channel_output_shift, tflite::micro::GetTensorShape(input),
      tflite::micro::GetTensorData<int8_t>(input),
      tflite::micro::GetTensorShape(filter),
      tflite::micro::GetTensorData<int8_t>(filter),
      tflite::micro::GetTensorShape(bias),
      tflite::micro::GetTensorData<int32_t>(bias),
      tflite::micro::GetTensorShape(output),
      tflite::micro::GetTensorData<int8_t>(output));
#else
  TF_LITE_KERNEL_LOG(context,
                     "Node configuration is not supported by ARC MLI Library.");
#endif
}

void EvalQuantized(TfLiteContext* context, TfLiteNode* node,
                   TfLiteDepthwiseConvParams* params, const OpData& data,
                   const TfLiteEvalTensor* input,
                   const TfLiteEvalTensor* filter, const TfLiteEvalTensor* bias,
                   TfLiteEvalTensor* output) {
#if !defined(TF_LITE_STRIP_REFERENCE_IMPL)
  const int32_t input_offset = -data.input_zero_point;
  const int32_t filter_offset = -data.filter_zero_point;
  const int32_t output_offset = data.output_zero_point;

  tflite::DepthwiseParams op_params;
  // Padding type is ignored, but still set.
  op_params.padding_type = PaddingType::kSame;
  op_params.padding_values.width = data.padding.width;
  op_params.padding_values.height = data.padding.height;
  op_params.stride_width = params->stride_width;
  op_params.stride_height = params->stride_height;
  op_params.dilation_width_factor = params->dilation_width_factor;
  op_params.dilation_height_factor = params->dilation_height_factor;
  op_params.depth_multiplier = params->depth_multiplier;
  op_params.quantized_activation_min = data.output_activation_min;
  op_params.quantized_activation_max = data.output_activation_max;
  op_params.input_offset = input_offset;
  op_params.weights_offset = filter_offset;
  op_params.output_offset = output_offset;
  op_params.output_multiplier = data.output_multiplier;
  // Legacy ops used mixed left and right shifts. Now all are +ve-means-left.
  op_params.output_shift = -data.output_shift;

  tflite::reference_ops::DepthwiseConv(
      op_params, tflite::micro::GetTensorShape(input),
      tflite::micro::GetTensorData<uint8_t>(input),
      tflite::micro::GetTensorShape(filter),
      tflite::micro::GetTensorData<uint8_t>(filter),
      tflite::micro::GetTensorShape(bias),
      tflite::micro::GetTensorData<int32_t>(bias),
      tflite::micro::GetTensorShape(output),
      tflite::micro::GetTensorData<uint8_t>(output));
#else
  TF_LITE_KERNEL_LOG(context,
                     "Type %s (%d) is not supported by ARC MLI Library.",
                     TfLiteTypeGetName(input->type), input->type);
#endif
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  auto* params =
      reinterpret_cast<TfLiteDepthwiseConvParams*>(node->builtin_data);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kOutputTensor);
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kInputTensor);
  const TfLiteEvalTensor* filter =
      tflite::micro::GetEvalInput(context, node, kFilterTensor);
  const TfLiteEvalTensor* bias =
      (NumInputs(node) == 3)
          ? tflite::micro::GetEvalInput(context, node, kBiasTensor)
          : nullptr;

  switch (input->type) {  // Already know in/out types are same.
    case kTfLiteFloat32:
      EvalFloat(context, node, params, data, input, filter, bias, output);
      break;
    case kTfLiteInt8:
      if (data.is_mli_applicable) {
        EvalMliQuantizedPerChannel(context, node, params, data, input, filter,
                                   bias, output);
      } else {
        EvalQuantizedPerChannel(context, node, params, data, input, filter,
                                bias, output);
      }
      break;
    case kTfLiteUInt8:
      EvalQuantized(context, node, params, data, input, filter, bias, output);
      break;
    default:
      TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                         TfLiteTypeGetName(input->type), input->type);
      return kTfLiteError;
  }
  return kTfLiteOk;
}

}  // namespace

TfLiteRegistration Register_DEPTHWISE_CONV_2D() {
  return {/*init=*/Init,
          /*free=*/nullptr,
          /*prepare=*/Prepare,
          /*invoke=*/Eval,
          /*profiling_string=*/nullptr,
          /*builtin_code=*/0,
          /*custom_name=*/nullptr,
          /*version=*/0};
}

}  // namespace tflite

#else
/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

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

#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/depthwise_conv.h"

#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/c/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/quantization_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/depthwiseconv_float.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/depthwiseconv_uint8.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/integer_ops/depthwise_conv.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/padding.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"

namespace tflite {
namespace {

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpDataConv));
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->user_data != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  auto& params =
      *(reinterpret_cast<TfLiteDepthwiseConvParams*>(node->builtin_data));
  const OpDataConv& data = *(static_cast<const OpDataConv*>(node->user_data));

  TfLiteEvalTensor* output =
      tflite::micro::GetEvalOutput(context, node, kDepthwiseConvOutputTensor);
  const TfLiteEvalTensor* input =
      tflite::micro::GetEvalInput(context, node, kDepthwiseConvInputTensor);
  const TfLiteEvalTensor* filter =
      tflite::micro::GetEvalInput(context, node, kDepthwiseConvWeightsTensor);
  const TfLiteEvalTensor* bias =
      (NumInputs(node) == 3)
          ? tflite::micro::GetEvalInput(context, node, kDepthwiseConvBiasTensor)
          : nullptr;

  switch (input->type) {  // Already know in/out types are same.
    case kTfLiteFloat32: {
      tflite::reference_ops::DepthwiseConv(
          DepthwiseConvParamsFloat(params, data),
          tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<float>(input),
          tflite::micro::GetTensorShape(filter),
          tflite::micro::GetTensorData<float>(filter),
          tflite::micro::GetTensorShape(bias),
          tflite::micro::GetTensorData<float>(bias),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<float>(output));
      break;
    }
    case kTfLiteInt8: {
      reference_integer_ops::DepthwiseConvPerChannel(
          DepthwiseConvParamsQuantized(params, data),
          data.per_channel_output_multiplier, data.per_channel_output_shift,
          tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<int8_t>(input),
          tflite::micro::GetTensorShape(filter),
          tflite::micro::GetTensorData<int8_t>(filter),
          tflite::micro::GetTensorShape(bias),
          tflite::micro::GetTensorData<int32_t>(bias),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<int8_t>(output));
      break;
    }
    default:
      TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                         TfLiteTypeGetName(input->type), input->type);
      return kTfLiteError;
  }
  return kTfLiteOk;
}

}  // namespace

TfLiteRegistration Register_DEPTHWISE_CONV_2D() {
  return {/*init=*/Init,
          /*free=*/nullptr,
          /*prepare=*/DepthwiseConvPrepare,
          /*invoke=*/Eval,
          /*profiling_string=*/nullptr,
          /*builtin_code=*/0,
          /*custom_name=*/nullptr,
          /*version=*/0};
}

}  // namespace tflite

#endif
