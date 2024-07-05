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

#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/softmax.h"

#include "edge-impulse-sdk/CMSIS/NN/Include/arm_nnfunctions.h"
#include "edge-impulse-sdk/tensorflow/lite/c/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/quantization_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/softmax.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/op_macros.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_log.h"

namespace tflite {
namespace {

struct CMSISNNSoftmaxParams {
  SoftmaxParams softmax_params;
  int32_t num_rows;
  int32_t row_size;
};

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context,
                                           sizeof(CMSISNNSoftmaxParams));
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  MicroContext* micro_context = GetMicroContext(context);

  TF_LITE_ENSURE_EQ(context, NumInputs(node), 1);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);
  TfLiteTensor* input = micro_context->AllocateTempInputTensor(node, 0);
  TF_LITE_ENSURE(context, input != nullptr);
  TF_LITE_ENSURE(context, NumDimensions(input) >= 1);
  TfLiteTensor* output = micro_context->AllocateTempOutputTensor(node, 0);
  TF_LITE_ENSURE(context, output != nullptr);

  TF_LITE_ENSURE(context, node->user_data != nullptr);
  CMSISNNSoftmaxParams* op_data =
      static_cast<CMSISNNSoftmaxParams*>(node->user_data);

  auto* params = static_cast<TfLiteSoftmaxParams*>(node->builtin_data);
  auto ret_val = CalculateSoftmaxParams(context, input, output, params,
                                        &op_data->softmax_params);

  const auto input_shape = GetTensorShape(input);
  const auto output_shape = GetTensorShape(output);
  const int trailing_dim = input_shape.DimensionsCount() - 1;
  const int outer_size =
      MatchingFlatSizeSkipDim(input_shape, trailing_dim, output_shape);
  const int depth =
      MatchingDim(input_shape, trailing_dim, output_shape, trailing_dim);
  op_data->num_rows = outer_size;
  op_data->row_size = depth;

  micro_context->DeallocateTempTfLiteTensor(input);
  micro_context->DeallocateTempTfLiteTensor(output);
  return ret_val;
}

TfLiteStatus SoftmaxEval(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input = tflite::micro::GetEvalInput(context, node, 0);
  TfLiteEvalTensor* output = tflite::micro::GetEvalOutput(context, node, 0);

  TFLITE_DCHECK(node->user_data != nullptr);
  const CMSISNNSoftmaxParams op_data =
      *static_cast<const CMSISNNSoftmaxParams*>(node->user_data);

  switch (input->type) {
    case kTfLiteFloat32: {
#if EI_TFLITE_DISABLE_SOFTMAX_IN_F32
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
      tflite::reference_ops::Softmax(
          op_data.softmax_params, tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<float>(input),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<float>(output));
      return kTfLiteOk;
    }
    case kTfLiteInt8: {
#if EI_TFLITE_DISABLE_SOFTMAX_IN_I8
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
      if (output->type == kTfLiteInt8) {
#if EI_TFLITE_DISABLE_SOFTMAX_OUT_I8
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  output->type);
      return kTfLiteError;
#endif
        arm_softmax_s8(tflite::micro::GetTensorData<int8_t>(input),
                       op_data.num_rows, op_data.row_size,
                       op_data.softmax_params.input_multiplier,
                       op_data.softmax_params.input_left_shift,
                       op_data.softmax_params.diff_min,
                       tflite::micro::GetTensorData<int8_t>(output));
      } else {
#if EI_TFLITE_DISABLE_SOFTMAX_OUT_I16
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  output->type);
      return kTfLiteError;
#endif
        arm_softmax_s8_s16(tflite::micro::GetTensorData<int8_t>(input),
                           op_data.num_rows, op_data.row_size,
                           op_data.softmax_params.input_multiplier,
                           op_data.softmax_params.input_left_shift,
                           op_data.softmax_params.diff_min,
                           tflite::micro::GetTensorData<int16_t>(output));
      }
      return kTfLiteOk;
    }
    case kTfLiteInt16: {
#if EI_TFLITE_DISABLE_SOFTMAX_IN_I16
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
      const cmsis_nn_softmax_lut_s16 softmax_params = {
          .exp_lut = op_data.softmax_params.exp_lut,
          .one_by_one_lut = op_data.softmax_params.one_over_one_plus_x_lut};

      TFLITE_DCHECK_EQ(
          arm_softmax_s16(
              tflite::micro::GetTensorData<int16_t>(input), op_data.num_rows,
              op_data.row_size, op_data.softmax_params.input_multiplier,
              op_data.softmax_params.input_left_shift, &softmax_params,
              tflite::micro::GetTensorData<int16_t>(output)),
          ARM_CMSIS_NN_SUCCESS);
      return kTfLiteOk;
    }
    default:
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
  }
}

TfLiteStatus SoftmaxEvalInt8(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input = tflite::micro::GetEvalInput(context, node, 0);
  TfLiteEvalTensor* output = tflite::micro::GetEvalOutput(context, node, 0);

  TFLITE_DCHECK(node->user_data != nullptr);
  const CMSISNNSoftmaxParams op_data =
      *static_cast<const CMSISNNSoftmaxParams*>(node->user_data);

  arm_softmax_s8(tflite::micro::GetTensorData<int8_t>(input), op_data.num_rows,
                 op_data.row_size, op_data.softmax_params.input_multiplier,
                 op_data.softmax_params.input_left_shift,
                 op_data.softmax_params.diff_min,
                 tflite::micro::GetTensorData<int8_t>(output));

  return kTfLiteOk;
}

TfLiteStatus SoftmaxEvalInt8_Int16(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input = tflite::micro::GetEvalInput(context, node, 0);
  TfLiteEvalTensor* output = tflite::micro::GetEvalOutput(context, node, 0);

  TFLITE_DCHECK(node->user_data != nullptr);
  const CMSISNNSoftmaxParams op_data =
      *static_cast<const CMSISNNSoftmaxParams*>(node->user_data);

  arm_softmax_s8_s16(
      tflite::micro::GetTensorData<int8_t>(input), op_data.num_rows,
      op_data.row_size, op_data.softmax_params.input_multiplier,
      op_data.softmax_params.input_left_shift, op_data.softmax_params.diff_min,
      tflite::micro::GetTensorData<int16_t>(output));

  return kTfLiteOk;
}

TfLiteStatus SoftmaxEvalInt16(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input = tflite::micro::GetEvalInput(context, node, 0);
  TfLiteEvalTensor* output = tflite::micro::GetEvalOutput(context, node, 0);

  TFLITE_DCHECK(node->user_data != nullptr);
  const CMSISNNSoftmaxParams op_data =
      *static_cast<const CMSISNNSoftmaxParams*>(node->user_data);

  const cmsis_nn_softmax_lut_s16 softmax_params = {
      .exp_lut = op_data.softmax_params.exp_lut,
      .one_by_one_lut = op_data.softmax_params.one_over_one_plus_x_lut};

  TFLITE_DCHECK_EQ(
      arm_softmax_s16(tflite::micro::GetTensorData<int16_t>(input),
                      op_data.num_rows, op_data.row_size,
                      op_data.softmax_params.input_multiplier,
                      op_data.softmax_params.input_left_shift, &softmax_params,
                      tflite::micro::GetTensorData<int16_t>(output)),
      ARM_CMSIS_NN_SUCCESS);

  return kTfLiteOk;
}

}  // namespace

TfLiteRegistration Register_SOFTMAX() {
  return tflite::micro::RegisterOp(Init, Prepare, SoftmaxEval);
}

TfLiteRegistration Register_SOFTMAX_INT8() {
  return tflite::micro::RegisterOp(Init, Prepare, SoftmaxEvalInt8);
}

TfLiteRegistration Register_SOFTMAX_INT8_INT16() {
  return tflite::micro::RegisterOp(Init, Prepare, SoftmaxEvalInt8_Int16);
}

TfLiteRegistration Register_SOFTMAX_INT16() {
  return tflite::micro::RegisterOp(Init, Prepare, SoftmaxEvalInt16);
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

#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/softmax.h"

#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/c/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/quantization_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/softmax.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/op_macros.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_log.h"

#include <esp_timer.h>

#if ESP_NN
#include "edge-impulse-sdk/porting/espressif/ESP-NN/include/esp_nn.h"
#endif

long long softmax_total_time = 0;

namespace tflite {
namespace {
// Softmax parameter data that persists in user_data
const int kInt16LUTArraySize = 513;

struct NodeData {
  SoftmaxParams op_data;
#if ESP_NN
  int buffer_idx;
#endif
};

static void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  TFLITE_DCHECK(context->AllocatePersistentBuffer != nullptr);
  return context->AllocatePersistentBuffer(context, sizeof(NodeData));
}

void SoftmaxQuantized(TfLiteContext* context, const TfLiteEvalTensor* input,
                      TfLiteEvalTensor* output, const NodeData* data) {
  if (input->type == kTfLiteInt8) {
#if EI_TFLITE_DISABLE_SOFTMAX_IN_I8
      TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                      TfLiteTypeGetName(input->type), input->type);
      return;
#endif
    if (output->type == kTfLiteInt16) {
#if EI_TFLITE_DISABLE_SOFTMAX_OUT_I16
      TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                      TfLiteTypeGetName(output->type), output->type);
      return;
#endif
      tflite::reference_ops::Softmax(
          data->op_data, tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<int8_t>(input),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<int16_t>(output));
    } else {
#if EI_TFLITE_DISABLE_SOFTMAX_OUT_I8
      TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                      TfLiteTypeGetName(output->type), output->type);
      return;
#endif
#if ESP_NN
      const int32_t input_beta_multiplier = data->op_data.input_multiplier;
      const int32_t input_beta_left_shift = data->op_data.input_left_shift;
      const int diff_min = data->op_data.diff_min;
      const RuntimeShape input_shape = tflite::micro::GetTensorShape(input);
      const RuntimeShape output_shape = tflite::micro::GetTensorShape(output);
      const int trailing_dim = input_shape.DimensionsCount() - 1;
      const int outer_size =
          MatchingFlatSizeSkipDim(input_shape, trailing_dim, output_shape);
      const int depth =
          MatchingDim(input_shape, trailing_dim, output_shape, trailing_dim);
      const int8_t *in_ptr = tflite::micro::GetTensorData<int8_t>(input);
      int8_t *out_ptr = tflite::micro::GetTensorData<int8_t>(output);
      void *scratch_buf = NULL;
      if (data->buffer_idx > -1) {
        scratch_buf = context->GetScratchBuffer(context, data->buffer_idx);
      }
      esp_nn_set_softmax_scratch_buf(scratch_buf);
      esp_nn_softmax_s8(in_ptr, outer_size, depth, input_beta_multiplier,
                        input_beta_left_shift, diff_min, out_ptr);
#else
      tflite::reference_ops::Softmax(
          data->op_data, tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<int8_t>(input),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<int8_t>(output));
#endif
    }
  } else {
#if EI_TFLITE_DISABLE_SOFTMAX_IN_I16
    TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                    TfLiteTypeGetName(input->type), input->type);
    return;
#endif
    tflite::reference_ops::SoftmaxInt16(
        data->op_data, tflite::micro::GetTensorShape(input),
        tflite::micro::GetTensorData<int16_t>(input),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<int16_t>(output));
  }
}

static TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input = tflite::micro::GetEvalInput(context, node, 0);
  TfLiteEvalTensor* output = tflite::micro::GetEvalOutput(context, node, 0);

  TFLITE_DCHECK(node->user_data != nullptr);
  NodeData data = *static_cast<NodeData*>(node->user_data);

  long long start_time = esp_timer_get_time();
  switch (input->type) {
    case kTfLiteFloat32: {
#if EI_TFLITE_DISABLE_SOFTMAX_IN_F32
      TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                      TfLiteTypeGetName(input->type), input->type);
      return kTfLiteError;
#endif
      tflite::reference_ops::Softmax(
          data.op_data, tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<float>(input),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<float>(output));
    }
    break;
    case kTfLiteInt8:
#if EI_TFLITE_DISABLE_SOFTMAX_IN_I8
      TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                      TfLiteTypeGetName(input->type), input->type);
      return kTfLiteError;
#endif
      SoftmaxQuantized(context, input, output, &data);
    break;
    case kTfLiteInt16: {
#if EI_TFLITE_DISABLE_SOFTMAX_IN_I16
      TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                      TfLiteTypeGetName(input->type), input->type);
      return kTfLiteError;
#endif
      SoftmaxQuantized(context, input, output, &data);
    }
    break;
    default:
      TF_LITE_KERNEL_LOG(context, "Type %s (%d) not supported.",
                         TfLiteTypeGetName(input->type), input->type);
      return kTfLiteError;
  }
  softmax_total_time += esp_timer_get_time() - start_time;
  return kTfLiteOk;
}

static TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  MicroContext* micro_context = GetMicroContext(context);

  TF_LITE_ENSURE_EQ(context, NumInputs(node), 1);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);
  TfLiteTensor* input = micro_context->AllocateTempInputTensor(node, 0);
  TF_LITE_ENSURE(context, input != nullptr);
  TF_LITE_ENSURE(context, NumDimensions(input) >= 1);
  TfLiteTensor* output = micro_context->AllocateTempOutputTensor(node, 0);
  TF_LITE_ENSURE(context, output != nullptr);

  TF_LITE_ENSURE(context, node->user_data != nullptr);
  NodeData* data = static_cast<NodeData*>(node->user_data);
  SoftmaxParams* op_data = static_cast<SoftmaxParams*>(&data->op_data);

  auto* params = static_cast<TfLiteSoftmaxParams*>(node->builtin_data);
  auto ret_val =
      CalculateSoftmaxParams(context, input, output, params, op_data);

#if ESP_NN
  if (output->type == kTfLiteInt8 && input->type == kTfLiteInt8) {
    const int32_t input_width = input->dims->data[1];
    const int32_t input_height = input->dims->data[2];
    int scratch_buf_size = esp_nn_get_softmax_scratch_size(input_width,
                                                           input_height);
    if (scratch_buf_size > 0) {
      TF_LITE_ENSURE_STATUS(context->RequestScratchBufferInArena(
        context, scratch_buf_size, &data->buffer_idx));
    }
  }
#endif

  micro_context->DeallocateTempTfLiteTensor(input);
  micro_context->DeallocateTempTfLiteTensor(output);
  return ret_val;
}

}  // namespace

TfLiteRegistration Register_SOFTMAX() {
  return tflite::micro::RegisterOp(Init, Prepare, Eval);
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

#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/softmax.h"

#include "edge-impulse-sdk/tensorflow/lite/c/builtin_op_data.h"
#include "edge-impulse-sdk/tensorflow/lite/c/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/common.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/quantization_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/reference/softmax.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/kernels/op_macros.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/kernel_util.h"
#include "edge-impulse-sdk/tensorflow/lite/micro/micro_log.h"

namespace tflite {
namespace {

void SoftmaxQuantized(const TfLiteEvalTensor* input, TfLiteEvalTensor* output,
                      const SoftmaxParams& op_data) {
  if (input->type == kTfLiteInt8) {
    if (output->type == kTfLiteInt16) {
      tflite::reference_ops::Softmax(
          op_data, tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<int8_t>(input),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<int16_t>(output));
    } else {
      tflite::reference_ops::Softmax(
          op_data, tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<int8_t>(input),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<int8_t>(output));
    }
  } else {
    tflite::reference_ops::SoftmaxInt16(
        op_data, tflite::micro::GetTensorShape(input),
        tflite::micro::GetTensorData<int16_t>(input),
        tflite::micro::GetTensorShape(output),
        tflite::micro::GetTensorData<int16_t>(output));
  }
}

TfLiteStatus SoftmaxEval(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteEvalTensor* input = tflite::micro::GetEvalInput(context, node, 0);
  TfLiteEvalTensor* output = tflite::micro::GetEvalOutput(context, node, 0);

  TFLITE_DCHECK(node->user_data != nullptr);
  SoftmaxParams op_data = *static_cast<SoftmaxParams*>(node->user_data);

  switch (input->type) {
    case kTfLiteFloat32: {
#if EI_TFLITE_DISABLE_SOFTMAX_IN_F32
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
      tflite::reference_ops::Softmax(
          op_data, tflite::micro::GetTensorShape(input),
          tflite::micro::GetTensorData<float>(input),
          tflite::micro::GetTensorShape(output),
          tflite::micro::GetTensorData<float>(output));
      return kTfLiteOk;
    }
    case kTfLiteInt8: {
#if EI_TFLITE_DISABLE_SOFTMAX_IN_I8
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
      SoftmaxQuantized(input, output, op_data);
      return kTfLiteOk;
    }
    case kTfLiteInt16: {
#if EI_TFLITE_DISABLE_SOFTMAX_IN_I16
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
#endif
      SoftmaxQuantized(input, output, op_data);
      return kTfLiteOk;
    }
    default:
      MicroPrintf("Type %s (%d) not supported.", TfLiteTypeGetName(input->type),
                  input->type);
      return kTfLiteError;
  }
}
}  // namespace

TfLiteRegistration Register_SOFTMAX() {
  return tflite::micro::RegisterOp(SoftmaxInit, SoftmaxPrepare, SoftmaxEval);
}

}  // namespace tflite

#endif
