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
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/pooling.h"

#include "edge-impulse-sdk/CMSIS/NN/Include/arm_nnfunctions.h"
#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/c/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/pooling.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_log.h"

namespace tflite {

namespace {

struct OpData {
  OpDataPooling reference_op_data;

  // Index to buffer for optimizations if applicable.
  int buffer_idx;
};

void PopulateCommonParams(
    TfLiteContext* const context, cmsis_nn_dims* const input_dims,
    cmsis_nn_dims* const output_dims, cmsis_nn_pool_params* const pool_params,
    cmsis_nn_context* const ctx, cmsis_nn_dims* const filter_dims,
    const OpData& data, const RuntimeShape& input_shape,
    const RuntimeShape& output_shape, const TfLitePoolParams* params) {
  const int depth = MatchingDim(input_shape, 3, output_shape, 3);

  input_dims->n = 1;
  input_dims->h = input_shape.Dims(1);
  input_dims->w = input_shape.Dims(2);
  input_dims->c = depth;

  output_dims->n = 1;
  output_dims->h = output_shape.Dims(1);
  output_dims->w = output_shape.Dims(2);
  output_dims->c = depth;

  pool_params->stride.h = params->stride_height;
  pool_params->stride.w = params->stride_width;
  pool_params->padding.h = data.reference_op_data.padding.height;
  pool_params->padding.w = data.reference_op_data.padding.width;
  pool_params->activation.min = data.reference_op_data.activation_min;
  pool_params->activation.max = data.reference_op_data.activation_max;

  filter_dims->n = 1;
  filter_dims->h = params->filter_height;
  filter_dims->w = params->filter_width;
  filter_dims->c = 1;
  ctx->buf = nullptr;
  ctx->size = 0;
  if (data.buffer_idx > -1) {
    ctx->buf = context->GetScratchBuffer(context, data.buffer_idx);
  }
}

void AverageEvalQuantized(TfLiteContext* context, const TfLiteNode* node,
                          const TfLitePoolParams* params, const OpData& data,
                          const TfLiteEvalTensor* input,
                          TfLiteEvalTensor* output) {
  TFLITE_DCHECK((input->type == kTfLiteInt8) || (input->type == kTfLiteInt16));

  RuntimeShape input_shape = micro::GetTensorShape(input);
  TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);

  RuntimeShape output_shape = micro::GetTensorShape(output);
  TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);

  cmsis_nn_dims input_dims;
  cmsis_nn_dims output_dims;
  cmsis_nn_pool_params pool_params;
  cmsis_nn_dims filter_dims;
  cmsis_nn_context ctx;

  PopulateCommonParams(context, &input_dims, &output_dims, &pool_params, &ctx,
                       &filter_dims, data, input_shape, output_shape, params);

  if (input->type == kTfLiteInt8) {
    TFLITE_DCHECK_EQ(
        arm_avgpool_s8(&ctx, &pool_params, &input_dims,
                       micro::GetTensorData<int8_t>(input), &filter_dims,
                       &output_dims, micro::GetTensorData<int8_t>(output)),
        ARM_CMSIS_NN_SUCCESS);
  } else {
    TFLITE_DCHECK_EQ(
        arm_avgpool_s16(&ctx, &pool_params, &input_dims,
                        micro::GetTensorData<int16_t>(input), &filter_dims,
                        &output_dims, micro::GetTensorData<int16_t>(output)),
        ARM_CMSIS_NN_SUCCESS);
  }
}

TfLiteStatus MaxEvalQuantized(TfLiteContext* context, const TfLiteNode* node,
                              const TfLitePoolParams* params,
                              const OpData& data, const TfLiteEvalTensor* input,
                              TfLiteEvalTensor* output) {
  TFLITE_DCHECK((input->type == kTfLiteInt8) || (input->type == kTfLiteInt16));

  RuntimeShape input_shape = micro::GetTensorShape(input);
  TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);

  RuntimeShape output_shape = micro::GetTensorShape(output);
  TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);

  cmsis_nn_dims input_dims;
  cmsis_nn_dims output_dims;
  cmsis_nn_pool_params pool_params;
  cmsis_nn_dims filter_dims;
  cmsis_nn_context ctx;

  PopulateCommonParams(context, &input_dims, &output_dims, &pool_params, &ctx,
                       &filter_dims, data, input_shape, output_shape, params);

  if (input->type == kTfLiteInt8) {
    TFLITE_DCHECK_EQ(
        arm_max_pool_s8(&ctx, &pool_params, &input_dims,
                        micro::GetTensorData<int8_t>(input), &filter_dims,
                        &output_dims, micro::GetTensorData<int8_t>(output)),
        ARM_CMSIS_NN_SUCCESS);
  } else {
    TFLITE_DCHECK_EQ(
        arm_max_pool_s16(&ctx, &pool_params, &input_dims,
                         micro::GetTensorData<int16_t>(input), &filter_dims,
                         &output_dims, micro::GetTensorData<int16_t>(output)),
        ARM_CMSIS_NN_SUCCESS);
  }

  return kTfLiteOk;
}

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpData));
}

TfLiteStatus MaxPrepare(TfLiteContext* context, TfLiteNode* node) {
  TF_LITE_ENSURE_STATUS(PoolingPrepare(context, node));
  // Set buffer index to a reset value
  static_cast<OpData*>(node->user_data)->buffer_idx = -1;
  return kTfLiteOk;
}

TfLiteStatus AveragePrepare(TfLiteContext* context, TfLiteNode* node) {
  TF_LITE_ENSURE_STATUS(PoolingPrepare(context, node));

  MicroContext* micro_context = GetMicroContext(context);

  TfLiteTensor* input =
      micro_context->AllocateTempInputTensor(node, kPoolingInputTensor);
  TfLiteTensor* output =
      micro_context->AllocateTempOutputTensor(node, kPoolingOutputTensor);

  if (input->type == kTfLiteInt8 || input->type == kTfLiteInt16) {
    RuntimeShape input_shape = GetTensorShape(input);
    TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);

    RuntimeShape output_shape = GetTensorShape(output);
    TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);

    const int depth = MatchingDim(input_shape, 3, output_shape, 3);
    const int output_width = output_shape.Dims(2);

    const int32_t buffer_size =
        input->type == kTfLiteInt16
            ? arm_avgpool_s16_get_buffer_size(output_width, depth)
            : arm_avgpool_s8_get_buffer_size(output_width, depth);

    auto* data = static_cast<OpData*>(node->user_data);
    if (buffer_size > 0) {
      TF_LITE_ENSURE_STATUS(context->RequestScratchBufferInArena(
          context, buffer_size, &data->buffer_idx));
    } else {
      data->buffer_idx = -1;
    }
  }

  micro_context->DeallocateTempTfLiteTensor(output);
  micro_context->DeallocateTempTfLiteTensor(input);
  return kTfLiteOk;
}

TfLiteStatus AverageEval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  const TfLiteEvalTensor* input =
      micro::GetEvalInput(context, node, kPoolingInputTensor);
  TfLiteEvalTensor* output =
      micro::GetEvalOutput(context, node, kPoolingOutputTensor);

  // Inputs and outputs share the same type, guaranteed by the converter.
  if (input->type == kTfLiteFloat32) {
#if EI_TFLITE_DISABLE_AVERAGE_POOL_2D_IN_F32
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
    AveragePoolingEvalFloat(context, node, params, &data.reference_op_data,
                            input, output);
  } else if (input->type == kTfLiteInt8) {
#if EI_TFLITE_DISABLE_AVERAGE_POOL_2D_IN_I8
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
    AverageEvalQuantized(context, node, params, data, input, output);
  } else if (input->type == kTfLiteInt16) {
    AverageEvalQuantized(context, node, params, data, input, output);
  } else {
    MicroPrintf("Input type %s is not currently supported",
                TfLiteTypeGetName(input->type));
    return kTfLiteError;
  }

  return kTfLiteOk;
}

TfLiteStatus AverageEvalInt8(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  const TfLiteEvalTensor* input =
      micro::GetEvalInput(context, node, kPoolingInputTensor);
  TFLITE_DCHECK(input->type == kTfLiteInt8);
  TfLiteEvalTensor* output =
      micro::GetEvalOutput(context, node, kPoolingOutputTensor);

  AverageEvalQuantized(context, node, params, data, input, output);

  return kTfLiteOk;
}

TfLiteStatus AverageEvalInt16(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  const TfLiteEvalTensor* input =
      micro::GetEvalInput(context, node, kPoolingInputTensor);
  TFLITE_DCHECK(input->type == kTfLiteInt16);
  TfLiteEvalTensor* output =
      micro::GetEvalOutput(context, node, kPoolingOutputTensor);

  AverageEvalQuantized(context, node, params, data, input, output);

  return kTfLiteOk;
}
TfLiteStatus MaxEval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  const TfLiteEvalTensor* input =
      micro::GetEvalInput(context, node, kPoolingInputTensor);
  TfLiteEvalTensor* output =
      micro::GetEvalOutput(context, node, kPoolingOutputTensor);

  if (input->type == kTfLiteFloat32) {
#if EI_TFLITE_DISABLE_MAX_POOL_2D_IN_F32
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
    MaxPoolingEvalFloat(context, node, params, &data.reference_op_data, input,
                        output);
  } else if (input->type == kTfLiteInt8) {
#if EI_TFLITE_DISABLE_MAX_POOL_2D_IN_I8
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
    MaxEvalQuantized(context, node, params, data, input, output);
  } else if (input->type == kTfLiteInt16) {
    MaxEvalQuantized(context, node, params, data, input, output);
  } else {
    MicroPrintf("Input type %s is not currently supported",
                TfLiteTypeGetName(input->type));
    return kTfLiteError;
  }

  return kTfLiteOk;
}

TfLiteStatus MaxEvalInt8(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  const TfLiteEvalTensor* input =
      micro::GetEvalInput(context, node, kPoolingInputTensor);
  TFLITE_DCHECK(input->type == kTfLiteInt8);
  TfLiteEvalTensor* output =
      micro::GetEvalOutput(context, node, kPoolingOutputTensor);

  MaxEvalQuantized(context, node, params, data, input, output);
  return kTfLiteOk;
}

TfLiteStatus MaxEvalInt16(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpData& data = *(static_cast<const OpData*>(node->user_data));

  const TfLiteEvalTensor* input =
      micro::GetEvalInput(context, node, kPoolingInputTensor);
  TFLITE_DCHECK(input->type == kTfLiteInt16);
  TfLiteEvalTensor* output =
      micro::GetEvalOutput(context, node, kPoolingOutputTensor);

  MaxEvalQuantized(context, node, params, data, input, output);
  return kTfLiteOk;
}

}  // namespace

TfLiteRegistration Register_AVERAGE_POOL_2D_INT8() {
  return tflite::micro::RegisterOp(Init, AveragePrepare, AverageEvalInt8);
}

TfLiteRegistration Register_AVERAGE_POOL_2D_INT16() {
  return tflite::micro::RegisterOp(Init, AveragePrepare, AverageEvalInt16);
}

TfLiteRegistration Register_AVERAGE_POOL_2D() {
  return tflite::micro::RegisterOp(Init, AveragePrepare, AverageEval);
}

TfLiteRegistration Register_MAX_POOL_2D_INT8() {
  return tflite::micro::RegisterOp(Init, MaxPrepare, MaxEvalInt8);
}

TfLiteRegistration Register_MAX_POOL_2D_INT16() {
  return tflite::micro::RegisterOp(Init, MaxPrepare, MaxEvalInt16);
}

TfLiteRegistration Register_MAX_POOL_2D() {
  return tflite::micro::RegisterOp(Init, MaxPrepare, MaxEval);
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
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/pooling.h"

#include "mli_api.h"  // NOLINT
#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/integer_ops/pooling.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/padding.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/mli_function_specializations.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/mli_slicers.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/mli_tf_utils.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/scratch_buf_mgr.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/scratch_buffers.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_log.h"

namespace tflite {

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
  mutable ops::micro::MliTensorInterface mli_in;
  mutable ops::micro::MliTensorInterface mli_out;
  mli_pool_cfg* cfg;

  // Pointer to the mli convolution function.
  pooling_func_ptr p_mli_krn_avepool_hwc_sa8;
  pooling_func_ptr p_mli_krn_maxpool_hwc_sa8;
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

  MicroContext* micro_context = GetMicroContext(context);

  TfLiteTensor* input =
      micro_context->AllocateTempInputTensor(node, kInputTensor);
  TF_LITE_ENSURE(context, input != nullptr);
  TfLiteTensor* output =
      micro_context->AllocateTempOutputTensor(node, kOutputTensor);
  TF_LITE_ENSURE(context, output != nullptr);

  data->is_mli_applicable = IsMliApplicable(context, input, params);

  TF_LITE_ENSURE_STATUS(CalculateOpData(context, params, input, output, data));

  if (input->type == kTfLiteFloat32) {
    CalculateActivationRange(params->activation, &data->activation_min_f32,
                             &data->activation_max_f32);
  } else if (input->type == kTfLiteInt8) {
    CalculateActivationRangeQuantized(context, params->activation, output,
                                      &data->activation_min,
                                      &data->activation_max);
  }

  if (data->is_mli_applicable) {
    data->mli_in = ops::micro::MliTensorInterface(static_cast<mli_tensor*>(
        context->AllocatePersistentBuffer(context, sizeof(mli_tensor))));
    data->mli_out = ops::micro::MliTensorInterface(static_cast<mli_tensor*>(
        context->AllocatePersistentBuffer(context, sizeof(mli_tensor))));
    data->cfg = static_cast<mli_pool_cfg*>(
        context->AllocatePersistentBuffer(context, sizeof(mli_pool_cfg)));

    ops::micro::ConvertToMliTensor(input, &data->mli_in);
    ops::micro::ConvertToMliTensor(output, &data->mli_out);

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

    // Choose pooling mli specialized functions.
    data->p_mli_krn_avepool_hwc_sa8 = mli_krn_avepool(data->cfg);
    data->p_mli_krn_maxpool_hwc_sa8 = mli_krn_maxpool(data->cfg);
  }

  micro_context->DeallocateTempTfLiteTensor(input);
  micro_context->DeallocateTempTfLiteTensor(output);
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
  MicroPrintf("Type %s (%d) is not supported by ARC MLI Library.",
              TfLiteTypeGetName(input->type), input->type);
#endif
}

// Prepare MLI tensors and run Average or Max Pooling
TfLiteStatus EvalMli(TfLiteContext* context, const TfLitePoolParams* params,
                     const OpData& data, const TfLiteEvalTensor* input,
                     TfLiteEvalTensor* output,
                     const MliPoolingType pooling_type) {
  mli_pool_cfg cfg_local = *data.cfg;

  ops::micro::MliTensorAttachBuffer<int8_t>(input, &data.mli_in);
  ops::micro::MliTensorAttachBuffer<int8_t>(output, &data.mli_out);

  const int height_dimension = 1;
  int in_slice_height = 0;
  int out_slice_height = 0;
  const int overlap = cfg_local.kernel_height - cfg_local.stride_height;

  // Tensors for data in fast (local) memory and config to copy data from
  // external to local memory
  mli_tensor in_local = *data.mli_in.MliTensor();
  mli_tensor out_local = *data.mli_out.MliTensor();

  ops::micro::MliTensorInterface in_local_interface(&in_local);
  ops::micro::MliTensorInterface out_local_interface(&out_local);

  mli_mov_cfg_t copy_config;
  mli_mov_cfg_for_copy(&copy_config);
  TF_LITE_ENSURE_STATUS(get_arc_scratch_buffer_for_pooling_tensors(
      context, &in_local_interface, &out_local_interface));

  bool in_is_local =
      in_local_interface.Data<int8_t>() == data.mli_in.Data<int8_t>();
  bool out_is_local =
      out_local_interface.Data<int8_t>() == data.mli_out.Data<int8_t>();

  TF_LITE_ENSURE_STATUS(arc_scratch_buffer_calc_slice_size_io(
      &in_local_interface, &out_local_interface, cfg_local.kernel_height,
      cfg_local.stride_height, cfg_local.padding_top, cfg_local.padding_bottom,
      &in_slice_height, &out_slice_height));

  /* mli_in tensor contains batches of HWC tensors. so it is a 4 dimensional
     tensor. because the mli kernel will process one HWC tensor at a time, the 4
     dimensional tensor needs to be sliced into nBatch 3 dimensional tensors. on
     top of that there could be a need to also slice in the Height dimension.
     for that the sliceHeight has been calculated. The tensor slicer is
     configured that it will completely slice the nBatch dimension (0) and slice
     the height dimension (1) in chunks of 'sliceHeight' */
  ops::micro::TensorSlicer in_slice(data.mli_in.MliTensor(), height_dimension,
                                    in_slice_height, cfg_local.padding_top,
                                    cfg_local.padding_bottom, overlap);
  ops::micro::TensorSlicer out_slice(data.mli_out.MliTensor(), height_dimension,
                                     out_slice_height);

  /* is_local indicates that the tensor is already in local memory,
     so in that case the original tensor can be used,
     and there is no need to copy it to the local tensor*/
  mli_tensor* in_ptr = in_is_local ? in_slice.Sub() : &in_local;
  mli_tensor* out_ptr = out_is_local ? out_slice.Sub() : &out_local;

  while (!out_slice.Done()) {
    if (!out_is_local) {
      ops::micro::PrepareLocalTensor(out_slice.Sub(), &out_local);
      ops::micro::PrepareLocalTensor(in_slice.Sub(), &in_local);
    }
    cfg_local.padding_top = in_slice.GetPaddingPre();
    cfg_local.padding_bottom = in_slice.GetPaddingPost();

    mli_mov_tensor_sync(in_slice.Sub(), &copy_config, in_ptr);
    if (pooling_type == AveragePooling) {
      TFLITE_DCHECK(data.p_mli_krn_avepool_hwc_sa8 != nullptr);
      data.p_mli_krn_avepool_hwc_sa8(in_ptr, &cfg_local, out_ptr);
    } else if (pooling_type == MaxPooling) {
      TFLITE_DCHECK(data.p_mli_krn_maxpool_hwc_sa8 != nullptr);
      data.p_mli_krn_maxpool_hwc_sa8(in_ptr, &cfg_local, out_ptr);
    }
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
  TFLITE_DCHECK(input->type == kTfLiteInt8);

  PoolParams op_params;
  op_params.stride_height = params->stride_height;
  op_params.stride_width = params->stride_width;
  op_params.filter_height = params->filter_height;
  op_params.filter_width = params->filter_width;
  op_params.padding_values.height = data.padding.height;
  op_params.padding_values.width = data.padding.width;
  op_params.quantized_activation_min = data.activation_min;
  op_params.quantized_activation_max = data.activation_max;

  reference_integer_ops::AveragePool(
      op_params, tflite::micro::GetTensorShape(input),
      tflite::micro::GetTensorData<int8_t>(input),
      tflite::micro::GetTensorShape(output),
      tflite::micro::GetTensorData<int8_t>(output));
#else
  MicroPrintf("Type %s (%d) is not supported by ARC MLI Library.",
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
  MicroPrintf(

      "Node configuration or type %s (%d) is not supported by ARC MLI Library.",
      TfLiteTypeGetName(input->type), input->type);
#endif
}

void MaxEvalQuantized(TfLiteContext* context, TfLiteNode* node,
                      TfLitePoolParams* params, const OpData& data,
                      const TfLiteEvalTensor* input, TfLiteEvalTensor* output) {
#if !defined(TF_LITE_STRIP_REFERENCE_IMPL)
  TFLITE_DCHECK(input->type == kTfLiteInt8);
  tflite::PoolParams op_params;
  op_params.stride_height = params->stride_height;
  op_params.stride_width = params->stride_width;
  op_params.filter_height = params->filter_height;
  op_params.filter_width = params->filter_width;
  op_params.padding_values.height = data.padding.height;
  op_params.padding_values.width = data.padding.width;
  op_params.quantized_activation_min = data.activation_min;
  op_params.quantized_activation_max = data.activation_max;

  reference_integer_ops::MaxPool(op_params,
                                 tflite::micro::GetTensorShape(input),
                                 tflite::micro::GetTensorData<int8_t>(input),
                                 tflite::micro::GetTensorShape(output),
                                 tflite::micro::GetTensorData<int8_t>(output));
#else
  MicroPrintf(

      "Node configuration or type %s (%d) is not supported by ARC MLI Library.",
      TfLiteTypeGetName(input->type), input->type);
#endif
}

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
#if EI_TFLITE_DISABLE_AVERAGE_POOL_2D_IN_F32
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
      AverageEvalFloat(context, node, params, data, input, output);
      break;
    case kTfLiteInt8:
#if EI_TFLITE_DISABLE_AVERAGE_POOL_2D_IN_I8
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
      if (data.is_mli_applicable) {
        EvalMli(context, params, data, input, output, AveragePooling);
      } else {
        AverageEvalQuantized(context, node, params, data, input, output);
      }
      break;
    default:
      MicroPrintf("Input type %s is not currently supported",
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
#if EI_TFLITE_DISABLE_MAX_POOL_2D_IN_F32
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
      MaxEvalFloat(context, node, params, data, input, output);
      break;
    case kTfLiteInt8:
#if EI_TFLITE_DISABLE_MAX_POOL_2D_IN_I8
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
      if (data.is_mli_applicable) {
        EvalMli(context, params, data, input, output, MaxPooling);
      } else {
        MaxEvalQuantized(context, node, params, data, input, output);
      }
      break;
    default:
      MicroPrintf("Type %s not currently supported.",
                  TfLiteTypeGetName(input->type));
      return kTfLiteError;
  }
  return kTfLiteOk;
}

}  // namespace

TfLiteRegistration Register_AVERAGE_POOL_2D() {
  return tflite::micro::RegisterOp(Init, Prepare, AverageEval);
}

TfLiteRegistration Register_MAX_POOL_2D() {
  return tflite::micro::RegisterOp(Init, Prepare, MaxEval);
}

}  // namespace tflite

#elif EI_CLASSIFIER_TFLITE_ENABLE_SILABS_MVP == 1

#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/pooling.h"
#include "edge-impulse-sdk/CMSIS/NN/Include/arm_nnfunctions.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/padding.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"

#include "sl_mvp_ml_pooling.h"

namespace tflite {
namespace sl {
namespace pooling {

namespace {

constexpr int kInputTensor = 0;
constexpr int kOutputTensor = 0;

enum op_support { kMvp, kCmsisNN, kTFLMrefF32};

struct OpData {
  float activation_min_f32;
  float activation_max_f32;
  sli_mvp_ml_pooling_s8_params_t op_params;
  op_support supported;
  int buffer_idx;
};

}  // namespace


void* Init(TfLiteContext* context, const char* buffer, size_t length)
{
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpData));
}


TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node)
{
  OpData* data = static_cast<OpData*>(node->user_data);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);
  const TfLiteTensor* input  = GetInput(context, node, kInputTensor);
  TfLiteTensor*       output = GetOutput(context, node, kOutputTensor);

  data->op_params.padding       = params->padding == kTfLitePaddingSame;
  data->op_params.stride_height = params->stride_height;
  data->op_params.stride_width  = params->stride_width;
  data->op_params.filter_height = params->filter_height;
  data->op_params.filter_width  = params->filter_width;
  data->op_params.batches       = MatchingDim(GetTensorShape(input),  0,
                                              GetTensorShape(output), 0);
  data->op_params.channels      = MatchingDim(GetTensorShape(input),  3,
                                              GetTensorShape(output), 3);
  data->op_params.input_height  = SizeOfDimension(input,  1);
  data->op_params.input_width   = SizeOfDimension(input,  2);
  data->op_params.output_height = SizeOfDimension(output, 1);
  data->op_params.output_width  = SizeOfDimension(output, 2);

  int out_height, out_width;
  auto padding = ComputePaddingHeightWidth(
                   params->stride_height, params->stride_width,
                   1, 1,  // dilation rate height/width.
                   data->op_params.input_height, data->op_params.input_width,
                   params->filter_height, params->filter_width,
                   params->padding,
                   &out_height, &out_width);
  TFLITE_DCHECK_EQ(out_height, data->op_params.output_height);
  TFLITE_DCHECK_EQ(out_width, data->op_params.output_width);
  data->op_params.pad_height = padding.height;
  data->op_params.pad_width  = padding.width;

  if (input->type == kTfLiteFloat32) {
    data->supported = kTFLMrefF32;
    CalculateActivationRange(params->activation,
                             &data->activation_min_f32,
                             &data->activation_max_f32);
  } else {
    CalculateActivationRangeQuantized(context, params->activation, output,
                                      reinterpret_cast<int32_t*>(&data->op_params.output_activation_min),
                                      reinterpret_cast<int32_t*>(&data->op_params.output_activation_max));
    if (input->type != kTfLiteInt8) {
      TF_LITE_KERNEL_LOG(context, "Type %s not currently supported.",
                         TfLiteTypeGetName(input->type));
      return kTfLiteError;
    }
  }

  return kTfLiteOk;
}

TfLiteStatus AveragePrepare(TfLiteContext* context, TfLiteNode* node)
{
  TFLITE_DCHECK(node->user_data    != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  OpData* data = static_cast<OpData*>(node->user_data);
  const TfLiteTensor* input  = GetInput(context, node, kInputTensor);
  TfLiteTensor*       output = GetOutput(context, node, kOutputTensor);
  TF_LITE_ENSURE(context, input  != nullptr);
  TF_LITE_ENSURE(context, output != nullptr);

  TfLiteStatus status = Prepare(context, node);

  if (status == kTfLiteOk) {
    if (input->type == kTfLiteInt8) {
      data->supported = sli_mvp_ml_average_pooling_s8_is_supported(&data->op_params)
                        ? kMvp : kCmsisNN;
      if (data->supported == kCmsisNN) {
        const int32_t buffer_size = arm_avgpool_s8_get_buffer_size(
                                      data->op_params.output_width,
                                      data->op_params.channels);

        if (buffer_size > 0) {
          TF_LITE_ENSURE_STATUS(context->RequestScratchBufferInArena(
                                  context, buffer_size, &data->buffer_idx));
        } else {
          data->buffer_idx = -1;
        }
      }
    }
  }
  return status;
}

TfLiteStatus MaxPrepare(TfLiteContext* context, TfLiteNode* node)
{
  TFLITE_DCHECK(node->user_data    != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  OpData* data = static_cast<OpData*>(node->user_data);
  const TfLiteTensor* input  = GetInput(context, node, kInputTensor);
  TfLiteTensor*       output = GetOutput(context, node, kOutputTensor);
  TF_LITE_ENSURE(context, input  != nullptr);
  TF_LITE_ENSURE(context, output != nullptr);

  TfLiteStatus status = Prepare(context, node);

  if (status == kTfLiteOk) {
    if (input->type == kTfLiteInt8) {
      data->supported = sli_mvp_ml_max_pooling_s8_is_supported(&data->op_params)
                        ? kMvp : kCmsisNN;
    }
  }

  return status;
}

TfLiteStatus AverageEval(TfLiteContext* context, TfLiteNode* node)
{
  TFLITE_DCHECK(node->user_data    != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  OpData* data = static_cast<OpData*>(node->user_data);
  const TfLiteEvalTensor* input  = tflite::micro::GetEvalInput(context, node, kInputTensor);
  TfLiteEvalTensor*       output = tflite::micro::GetEvalOutput(context, node, kOutputTensor);
  TF_LITE_ENSURE(context, input  != nullptr);
  TF_LITE_ENSURE(context, output != nullptr);
  data->op_params.input  = tflite::micro::GetTensorData<int8_t>(input);
  data->op_params.output = tflite::micro::GetTensorData<int8_t>(output);

  if (data->supported == kMvp) {
    // Use MVP accelerated kernel.
    TF_LITE_ENSURE_EQ(context,
                      SL_STATUS_OK,
                      sli_mvp_ml_average_pooling_s8(&data->op_params));

  } else if (data->supported == kCmsisNN) {
    // Use CMSIS-NN optimized kernel.
    cmsis_nn_dims input_dims;
    input_dims.n = 1;
    input_dims.h = data->op_params.input_height;
    input_dims.w = data->op_params.input_width;
    input_dims.c = data->op_params.channels;

    cmsis_nn_dims output_dims;
    output_dims.n = 1;
    output_dims.h = data->op_params.output_height;
    output_dims.w = data->op_params.output_width;
    output_dims.c = data->op_params.channels;

    cmsis_nn_pool_params pool_params;
    pool_params.stride.h = data->op_params.stride_height;
    pool_params.stride.w = data->op_params.stride_width;
    pool_params.padding.h = data->op_params.pad_height;
    pool_params.padding.w = data->op_params.pad_width;
    pool_params.activation.min = data->op_params.output_activation_min;
    pool_params.activation.max = data->op_params.output_activation_max;

    cmsis_nn_dims filter_dims;
    filter_dims.n = 1;
    filter_dims.h = data->op_params.filter_height;
    filter_dims.w = data->op_params.filter_width;
    filter_dims.c = 1;

    cmsis_nn_context ctx;
    ctx.buf = nullptr;
    ctx.size = 0;
    if (data->buffer_idx > -1) {
      ctx.buf = context->GetScratchBuffer(context, data->buffer_idx);
    }

    TFLITE_DCHECK_EQ(
        arm_avgpool_s8(&ctx, &pool_params, &input_dims,
                       data->op_params.input, &filter_dims,
                       &output_dims,
                       data->op_params.output),
        ARM_MATH_SUCCESS);
  } else if (data->supported == kTFLMrefF32) {
    #if EI_TFLITE_DISABLE_AVERAGE_POOL_2D_IN_F32
    TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                    TfLiteTypeGetName(input->type), input->type);
    return kTfLiteError;
    #endif

    // Use TFLM reference kernel.
    tflite::PoolParams op_params;
    op_params.stride_height         = data->op_params.stride_height;
    op_params.stride_width          = data->op_params.stride_width;
    op_params.filter_height         = data->op_params.filter_height;
    op_params.filter_width          = data->op_params.filter_width;
    op_params.padding_values.height = data->op_params.pad_height;
    op_params.padding_values.width  = data->op_params.pad_width;
    op_params.float_activation_min  = data->activation_min_f32;
    op_params.float_activation_max  = data->activation_max_f32;
    reference_ops::AveragePool(op_params,
                               tflite::micro::GetTensorShape(input),
                               tflite::micro::GetTensorData<float>(input),
                               tflite::micro::GetTensorShape(output),
                               tflite::micro::GetTensorData<float>(output));

  } else {
    return kTfLiteError;
  }

  return kTfLiteOk;
}

TfLiteStatus MaxEval(TfLiteContext* context, TfLiteNode* node)
{
  TFLITE_DCHECK(node->user_data    != nullptr);
  TFLITE_DCHECK(node->builtin_data != nullptr);

  OpData* data = static_cast<OpData*>(node->user_data);
  const TfLiteEvalTensor* input  = tflite::micro::GetEvalInput(context, node, kInputTensor);
  TfLiteEvalTensor*       output = tflite::micro::GetEvalOutput(context, node, kOutputTensor);
  TF_LITE_ENSURE(context, input  != nullptr);
  TF_LITE_ENSURE(context, output != nullptr);
  data->op_params.input  = tflite::micro::GetTensorData<int8_t>(input);
  data->op_params.output = tflite::micro::GetTensorData<int8_t>(output);

  if (data->supported == kMvp) {
    // Use MVP accelerated kernel.
    TF_LITE_ENSURE_EQ(context,
                      SL_STATUS_OK,
                      sli_mvp_ml_max_pooling_s8(&data->op_params));

  } else if (data->supported == kCmsisNN) {
    // Use CMSIS-NN optimized kernel.
    cmsis_nn_dims input_dims;
    input_dims.n = 1;
    input_dims.h = data->op_params.input_height;
    input_dims.w = data->op_params.input_width;
    input_dims.c = data->op_params.channels;

    cmsis_nn_dims output_dims;
    output_dims.n = 1;
    output_dims.h = data->op_params.output_height;
    output_dims.w = data->op_params.output_width;
    output_dims.c = data->op_params.channels;

    cmsis_nn_pool_params pool_params;
    pool_params.stride.h = data->op_params.stride_height;
    pool_params.stride.w = data->op_params.stride_width;
    pool_params.padding.h = data->op_params.pad_height;
    pool_params.padding.w = data->op_params.pad_width;
    pool_params.activation.min = data->op_params.output_activation_min;
    pool_params.activation.max = data->op_params.output_activation_max;

    cmsis_nn_dims filter_dims;
    filter_dims.n = 1;
    filter_dims.h = data->op_params.filter_height;
    filter_dims.w = data->op_params.filter_width;
    filter_dims.c = 1;

    cmsis_nn_context ctx;
    ctx.buf = nullptr;
    ctx.size = 0;

    TFLITE_DCHECK_EQ(
        arm_max_pool_s8(&ctx, &pool_params, &input_dims,
                        data->op_params.input, &filter_dims,
                        &output_dims,
                        data->op_params.output),
        ARM_MATH_SUCCESS);
  } else if (data->supported == kTFLMrefF32) {
    #if EI_TFLITE_DISABLE_MAX_POOL_2D_IN_F32
    TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                    TfLiteTypeGetName(input->type), input->type);
    return kTfLiteError;
    #endif

    // Use TFLM reference kernel.
    tflite::PoolParams op_params;
    op_params.stride_height         = data->op_params.stride_height;
    op_params.stride_width          = data->op_params.stride_width;
    op_params.filter_height         = data->op_params.filter_height;
    op_params.filter_width          = data->op_params.filter_width;
    op_params.padding_values.height = data->op_params.pad_height;
    op_params.padding_values.width  = data->op_params.pad_width;
    op_params.float_activation_min  = data->activation_min_f32;
    op_params.float_activation_max  = data->activation_max_f32;
    reference_ops::MaxPool(op_params,
                           tflite::micro::GetTensorShape(input),
                           tflite::micro::GetTensorData<float>(input),
                           tflite::micro::GetTensorShape(output),
                           tflite::micro::GetTensorData<float>(output));

  } else {
    return kTfLiteError;
  }

  return kTfLiteOk;
}

}  // namespace pooling
}  // namespace sl

TfLiteRegistration Register_MAX_POOL_2D() {
  static TfLiteRegistration max_pool_registration = {
    /*init=*/sl::pooling::Init,
    /*free=*/nullptr,
    /*prepare=*/sl::pooling::MaxPrepare,
    /*invoke=*/sl::pooling::MaxEval,
    /*profiling_string=*/nullptr,
    /*builtin_code=*/0,
    /*custom_name=*/nullptr,
    /*version=*/0
  };

  return max_pool_registration;
}

// Just to keep all_ops_resolver() happy during development ...
TfLiteRegistration Register_AVERAGE_POOL_2D() {
  static TfLiteRegistration avg_pool_registration = {
    /*init=*/sl::pooling::Init,
    /*free=*/nullptr,
    /*prepare=*/sl::pooling::AveragePrepare,
    /*invoke=*/sl::pooling::AverageEval,
    /*profiling_string=*/nullptr,
    /*builtin_code=*/0,
    /*custom_name=*/nullptr,
    /*version=*/0
  };

  return avg_pool_registration;
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
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/pooling.h"

#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/pooling.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_log.h"

#if ESP_NN
#include "edge-impulse-sdk/porting/espressif/ESP-NN/include/esp_nn.h"
#endif

#include <esp_timer.h>

long long pooling_total_time = 0;

namespace tflite {

namespace {
#if ESP_NN
void AverageEvalQuantized(TfLiteContext* context, const TfLiteNode* node,
                          const TfLitePoolParams* params, const OpDataPooling* data,
                          const TfLiteEvalTensor* input,
                          TfLiteEvalTensor* output) {

  const int stride_height = params->stride_height;
  const int stride_width = params->stride_width;
  const int filter_height = params->filter_height;
  const int filter_width = params->filter_width;
  const int activation_min = data->activation_min;
  const int activation_max = data->activation_max;
  const int pad_height = data->padding.height;
  const int pad_width = data->padding.width;

  const RuntimeShape& input_shape = tflite::micro::GetTensorShape(input);
  const RuntimeShape& output_shape = tflite::micro::GetTensorShape(output);
  TFLITE_DCHECK_LE(activation_min, activation_max);
  TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);
  const int batches = MatchingDim(input_shape, 0, output_shape, 0);
  const int depth = MatchingDim(input_shape, 3, output_shape, 3);
  const int input_height = input_shape.Dims(1);
  const int input_width = input_shape.Dims(2);
  const int output_height = output_shape.Dims(1);
  const int output_width = output_shape.Dims(2);

  const int8_t *input_data = tflite::micro::GetTensorData<int8_t>(input);
  int8_t *output_data = tflite::micro::GetTensorData<int8_t>(output);

  const int input_size = input_width * input_height * depth;
  const int output_size = output_width * output_height * depth;

  if (depth % 4 == 0) { // S3 version only supports channels multiple of 4
    for (int batch = 0; batch < batches; ++batch) {
      esp_nn_avg_pool_s8(input_data, input_width, input_height,
                         output_data, output_width, output_height,
                         stride_width, stride_height,
                         filter_width, filter_height,
                         pad_width, pad_height,
                         activation_min, activation_max, depth);
      input_data += input_size;
      output_data += output_size;
    }
  } else {
    for (int batch = 0; batch < batches; ++batch) {
      esp_nn_avg_pool_s8_ansi(input_data, input_width, input_height,
                              output_data, output_width, output_height,
                              stride_width, stride_height,
                              filter_width, filter_height,
                              pad_width, pad_height,
                              activation_min, activation_max, depth);
      input_data += input_size;
      output_data += output_size;
    }
  }
}

void MaxEvalQuantized(TfLiteContext* context, TfLiteNode* node,
                      TfLitePoolParams* params, const OpDataPooling* data,
                      const TfLiteEvalTensor* input, TfLiteEvalTensor* output) {

  const int stride_height = params->stride_height;
  const int stride_width = params->stride_width;
  const int filter_height = params->filter_height;
  const int filter_width = params->filter_width;
  const int activation_min = data->activation_min;
  const int activation_max = data->activation_max;
  const int pad_height = data->padding.height;
  const int pad_width = data->padding.width;

  const RuntimeShape& input_shape = tflite::micro::GetTensorShape(input);
  const RuntimeShape& output_shape = tflite::micro::GetTensorShape(output);
  TFLITE_DCHECK_LE(activation_min, activation_max);
  TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);
  const int batches = MatchingDim(input_shape, 0, output_shape, 0);
  const int depth = MatchingDim(input_shape, 3, output_shape, 3);
  const int input_height = input_shape.Dims(1);
  const int input_width = input_shape.Dims(2);
  const int output_height = output_shape.Dims(1);
  const int output_width = output_shape.Dims(2);

  const int8_t *input_data = tflite::micro::GetTensorData<int8_t>(input);
  int8_t *output_data = tflite::micro::GetTensorData<int8_t>(output);

  const int input_size = input_width * input_height * depth;
  const int output_size = output_width * output_height * depth;
  if (depth % 4 == 0) { // S3 version only supports channels multiple of 4
    for (int batch = 0; batch < batches; ++batch) {
      esp_nn_max_pool_s8(input_data, input_width, input_height,
                         output_data, output_width, output_height,
                         stride_width, stride_height,
                         filter_width, filter_height,
                         pad_width, pad_height,
                         activation_min, activation_max, depth);
      input_data += input_size;
      output_data += output_size;
    }
  } else {
    for (int batch = 0; batch < batches; ++batch) {
      esp_nn_max_pool_s8_ansi(input_data, input_width, input_height,
                              output_data, output_width, output_height,
                              stride_width, stride_height,
                              filter_width, filter_height,
                              pad_width, pad_height,
                              activation_min, activation_max, depth);
      input_data += input_size;
      output_data += output_size;
    }
  }
}
#endif

TfLiteStatus AverageEval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpDataPooling* data =
      static_cast<const OpDataPooling*>(node->user_data);

  const TfLiteEvalTensor* input =
      micro::GetEvalInput(context, node, kPoolingInputTensor);
  TfLiteEvalTensor* output =
      micro::GetEvalOutput(context, node, kPoolingOutputTensor);

  long long start_time = esp_timer_get_time();
  // Inputs and outputs share the same type, guaranteed by the converter.
  switch (input->type) {
    case kTfLiteFloat32:
#if EI_TFLITE_DISABLE_AVERAGE_POOL_2D_IN_F32
      TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                      TfLiteTypeGetName(input->type), input->type);
      return kTfLiteError;
#endif
      AveragePoolingEvalFloat(context, node, params, data, input, output);
      break;
    case kTfLiteInt8:
#if EI_TFLITE_DISABLE_AVERAGE_POOL_2D_IN_I8
      TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                      TfLiteTypeGetName(input->type), input->type);
      return kTfLiteError;
#endif
#if ESP_NN
      AverageEvalQuantized(context, node, params, data, input, output);
#else
      AveragePoolingEvalQuantized<int8_t>(context, node, params, data, input, output);
#endif
      break;
    default:
      TF_LITE_KERNEL_LOG(context, "Input type %s is not currently supported",
                         TfLiteTypeGetName(input->type));
      return kTfLiteError;
  }
  pooling_total_time += esp_timer_get_time() - start_time;
  return kTfLiteOk;
}

TfLiteStatus MaxEval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpDataPooling* data =
      static_cast<const OpDataPooling*>(node->user_data);

  const TfLiteEvalTensor* input =
      micro::GetEvalInput(context, node, kPoolingInputTensor);
  TfLiteEvalTensor* output =
      micro::GetEvalOutput(context, node, kPoolingOutputTensor);

  long long start_time = esp_timer_get_time();
  switch (input->type) {
    case kTfLiteFloat32:
#if EI_TFLITE_DISABLE_MAX_POOL_2D_IN_F32
      TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                      TfLiteTypeGetName(input->type), input->type);
      return kTfLiteError;
#endif
      MaxPoolingEvalFloat(context, node, params, data, input, output);
      break;
    case kTfLiteInt8:
#if EI_TFLITE_DISABLE_MAX_POOL_2D_IN_I8
      TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                      TfLiteTypeGetName(input->type), input->type);
      return kTfLiteError;
#endif
#if ESP_NN
      MaxEvalQuantized(context, node, params, data, input, output);
#else
      MaxPoolingEvalQuantized<int8_t>(context, node, params, data, input, output);
#endif
      break;
    default:
      TF_LITE_KERNEL_LOG(context, "Type %s not currently supported.",
                         TfLiteTypeGetName(input->type));
      return kTfLiteError;
  }
  pooling_total_time += esp_timer_get_time() - start_time;
  return kTfLiteOk;
}

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpDataPooling));
}

}  // namespace

TfLiteRegistration Register_AVERAGE_POOL_2D() {
  return tflite::micro::RegisterOp(Init, PoolingPrepare, AverageEval);
}

TfLiteRegistration Register_MAX_POOL_2D() {
  return tflite::micro::RegisterOp(Init, PoolingPrepare, MaxEval);
}

}  // namespace tflite

#else
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
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/pooling.h"

#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/pooling.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_log.h"

namespace tflite {

namespace {

TfLiteStatus AverageEval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpDataPooling* data =
      static_cast<const OpDataPooling*>(node->user_data);

  const TfLiteEvalTensor* input =
      micro::GetEvalInput(context, node, kPoolingInputTensor);
  TfLiteEvalTensor* output =
      micro::GetEvalOutput(context, node, kPoolingOutputTensor);

  // Inputs and outputs share the same type, guaranteed by the converter.
  switch (input->type) {
    case kTfLiteFloat32:
#if EI_TFLITE_DISABLE_AVERAGE_POOL_2D_IN_F32
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
      AveragePoolingEvalFloat(context, node, params, data, input, output);
      break;
    case kTfLiteInt8:
#if EI_TFLITE_DISABLE_AVERAGE_POOL_2D_IN_I8
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
      AveragePoolingEvalQuantized<int8_t>(context, node, params, data, input,
                                          output);
      break;
    case kTfLiteInt16:
      AveragePoolingEvalQuantized<int16_t>(context, node, params, data, input,
                                           output);
      break;
    default:
      MicroPrintf("Input type %s is not currently supported",
                  TfLiteTypeGetName(input->type));
      return kTfLiteError;
  }
  return kTfLiteOk;
}

TfLiteStatus MaxEval(TfLiteContext* context, TfLiteNode* node) {
  TFLITE_DCHECK(node->builtin_data != nullptr);
  auto* params = reinterpret_cast<TfLitePoolParams*>(node->builtin_data);

  TFLITE_DCHECK(node->user_data != nullptr);
  const OpDataPooling* data =
      static_cast<const OpDataPooling*>(node->user_data);

  const TfLiteEvalTensor* input =
      micro::GetEvalInput(context, node, kPoolingInputTensor);
  TfLiteEvalTensor* output =
      micro::GetEvalOutput(context, node, kPoolingOutputTensor);

  switch (input->type) {
    case kTfLiteFloat32:
#if EI_TFLITE_DISABLE_MAX_POOL_2D_IN_F32
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
      MaxPoolingEvalFloat(context, node, params, data, input, output);
      break;
    case kTfLiteInt8:
#if EI_TFLITE_DISABLE_MAX_POOL_2D_IN_I8
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
      MaxPoolingEvalQuantized<int8_t>(context, node, params, data, input,
                                      output);
      break;
    case kTfLiteInt16:
      MaxPoolingEvalQuantized<int16_t>(context, node, params, data, input,
                                       output);
      break;
    default:
      MicroPrintf("Type %s not currently supported.",
                  TfLiteTypeGetName(input->type));
      return kTfLiteError;
  }
  return kTfLiteOk;
}

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(OpDataPooling));
}

}  // namespace

TfLiteRegistration Register_AVERAGE_POOL_2D() {
  return tflite::micro::RegisterOp(Init, PoolingPrepare, AverageEval);
}

TfLiteRegistration Register_MAX_POOL_2D() {
  return tflite::micro::RegisterOp(Init, PoolingPrepare, MaxEval);
}

}  // namespace tflite

#endif
