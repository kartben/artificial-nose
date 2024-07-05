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

#include "edge-impulse-sdk/tensorflow/lite/micro/all_ops_resolver.h"

#include "edge-impulse-sdk/tensorflow/lite/micro/kernels/micro_ops.h"

namespace tflite {

AllOpsResolver::AllOpsResolver() {
  // Please keep this list of Builtin Operators in alphabetical order.
  AddAbs();
  AddAdd();
  AddAddN();
  AddArgMax();
  AddArgMin();
  AddAssignVariable();
  AddAveragePool2D();
  AddBatchMatMul();
  AddBatchToSpaceNd();
  AddBroadcastArgs();
  AddBroadcastTo();
  AddCallOnce();
  AddCast();
  AddCeil();
  AddComplexAbs();
  AddCircularBuffer();
  AddConcatenation();
  AddConv2D();
  AddCos();
  AddCumSum();
  AddDepthToSpace();
  AddDepthwiseConv2D();
  AddDequantize();
  AddDetectionPostprocess();
  AddDiv();
  AddElu();
  AddEqual();
  AddEthosU();
  AddExp();
  AddExpandDims();
  AddFill();
  AddFloor();
  AddFloorDiv();
  AddFloorMod();
  AddFullyConnected();
#ifndef TF_LITE_STATIC_MEMORY
  AddGather();
#endif // TF_LITE_STATIC_MEMORY
  AddGatherNd();
  AddGreater();
  AddGreaterEqual();
  AddHardSwish();
  AddImag();
  AddIf();
  AddL2Normalization();
  AddL2Pool2D();
  AddLeakyRelu();
  AddLess();
  AddLessEqual();
  AddLog();
  AddLogicalAnd();
  AddLogicalNot();
  AddLogicalOr();
  AddLogistic();
  AddLogSoftmax();
  AddMaxPool2D();
  AddMaximum();
  AddMean();
  AddMinimum();
  AddMirrorPad();
  AddMul();
  AddNeg();
  AddNotEqual();
  AddPack();
  AddPad();
  AddPadV2();
  AddPrelu();
  AddQuantize();
  AddReal();
  AddReadVariable();
  AddReduceMax();
  AddReduceMin();
  AddRelu();
  AddRelu6();
  AddReshape();
  AddResizeBilinear();
  AddResizeNearestNeighbor();
  AddRfft2D();
  AddRound();
  AddRsqrt();
#ifndef TF_LITE_STATIC_MEMORY
  AddSelect();
  AddSelectV2();
#endif // TF_LITE_STATIC_MEMORY
  AddShape();
  AddSin();
  AddSlice();
  AddSoftmax();
  AddSpaceToBatchNd();
  AddSpaceToDepth();
  AddSplit();
  AddSplitV();
  AddSqrt();
  AddSquare();
  AddSquaredDifference();
  AddSqueeze();
  AddStridedSlice();
  AddSub();
  AddSum();
  AddSvdf();
  AddTanh();
  AddTranspose();
  AddTransposeConv();
  AddTreeEnsembleClassifier();
  AddUnidirectionalSequenceLstm();
  AddUnpack();
  AddVarHandle();
  AddWhile();
  AddZerosLike();
}

}  // namespace tflite
