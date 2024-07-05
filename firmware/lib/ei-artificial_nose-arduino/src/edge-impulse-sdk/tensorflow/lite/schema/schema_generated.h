#ifndef FLATBUFFERS_GENERATED_SCHEMA_SUPPL_TFLITE_H_
#define FLATBUFFERS_GENERATED_SCHEMA_SUPPL_TFLITE_H_

#include "edge-impulse-sdk/third_party/flatbuffers/include/flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 2 &&
              FLATBUFFERS_VERSION_MINOR == 0 &&
              FLATBUFFERS_VERSION_REVISION == 6,
             "Non-compatible flatbuffers version included");

namespace tflite {

struct CustomQuantization;
struct CustomQuantizationBuilder;
struct CustomQuantizationT;

struct QuantizationParameters;
struct QuantizationParametersBuilder;
struct QuantizationParametersT;

struct Int32Vector;
struct Int32VectorBuilder;
struct Int32VectorT;

struct Uint16Vector;
struct Uint16VectorBuilder;
struct Uint16VectorT;

struct Uint8Vector;
struct Uint8VectorBuilder;
struct Uint8VectorT;

struct DimensionMetadata;
struct DimensionMetadataBuilder;
struct DimensionMetadataT;

struct SparsityParameters;
struct SparsityParametersBuilder;
struct SparsityParametersT;

struct VariantSubType;
struct VariantSubTypeBuilder;
struct VariantSubTypeT;

struct Tensor;
struct TensorBuilder;
struct TensorT;

struct Conv2DOptions;
struct Conv2DOptionsBuilder;
struct Conv2DOptionsT;

struct Conv3DOptions;
struct Conv3DOptionsBuilder;
struct Conv3DOptionsT;

struct Pool2DOptions;
struct Pool2DOptionsBuilder;
struct Pool2DOptionsT;

struct DepthwiseConv2DOptions;
struct DepthwiseConv2DOptionsBuilder;
struct DepthwiseConv2DOptionsT;

struct ConcatEmbeddingsOptions;
struct ConcatEmbeddingsOptionsBuilder;
struct ConcatEmbeddingsOptionsT;

struct LSHProjectionOptions;
struct LSHProjectionOptionsBuilder;
struct LSHProjectionOptionsT;

struct SVDFOptions;
struct SVDFOptionsBuilder;
struct SVDFOptionsT;

struct RNNOptions;
struct RNNOptionsBuilder;
struct RNNOptionsT;

struct SequenceRNNOptions;
struct SequenceRNNOptionsBuilder;
struct SequenceRNNOptionsT;

struct BidirectionalSequenceRNNOptions;
struct BidirectionalSequenceRNNOptionsBuilder;
struct BidirectionalSequenceRNNOptionsT;

struct FullyConnectedOptions;
struct FullyConnectedOptionsBuilder;
struct FullyConnectedOptionsT;

struct SoftmaxOptions;
struct SoftmaxOptionsBuilder;
struct SoftmaxOptionsT;

struct ConcatenationOptions;
struct ConcatenationOptionsBuilder;
struct ConcatenationOptionsT;

struct AddOptions;
struct AddOptionsBuilder;
struct AddOptionsT;

struct MulOptions;
struct MulOptionsBuilder;
struct MulOptionsT;

struct L2NormOptions;
struct L2NormOptionsBuilder;
struct L2NormOptionsT;

struct LocalResponseNormalizationOptions;
struct LocalResponseNormalizationOptionsBuilder;
struct LocalResponseNormalizationOptionsT;

struct LSTMOptions;
struct LSTMOptionsBuilder;
struct LSTMOptionsT;

struct UnidirectionalSequenceLSTMOptions;
struct UnidirectionalSequenceLSTMOptionsBuilder;
struct UnidirectionalSequenceLSTMOptionsT;

struct BidirectionalSequenceLSTMOptions;
struct BidirectionalSequenceLSTMOptionsBuilder;
struct BidirectionalSequenceLSTMOptionsT;

struct ResizeBilinearOptions;
struct ResizeBilinearOptionsBuilder;
struct ResizeBilinearOptionsT;

struct ResizeNearestNeighborOptions;
struct ResizeNearestNeighborOptionsBuilder;
struct ResizeNearestNeighborOptionsT;

struct CallOptions;
struct CallOptionsBuilder;
struct CallOptionsT;

struct PadOptions;
struct PadOptionsBuilder;
struct PadOptionsT;

struct PadV2Options;
struct PadV2OptionsBuilder;
struct PadV2OptionsT;

struct ReshapeOptions;
struct ReshapeOptionsBuilder;
struct ReshapeOptionsT;

struct SpaceToBatchNDOptions;
struct SpaceToBatchNDOptionsBuilder;
struct SpaceToBatchNDOptionsT;

struct BatchToSpaceNDOptions;
struct BatchToSpaceNDOptionsBuilder;
struct BatchToSpaceNDOptionsT;

struct SkipGramOptions;
struct SkipGramOptionsBuilder;
struct SkipGramOptionsT;

struct SpaceToDepthOptions;
struct SpaceToDepthOptionsBuilder;
struct SpaceToDepthOptionsT;

struct DepthToSpaceOptions;
struct DepthToSpaceOptionsBuilder;
struct DepthToSpaceOptionsT;

struct SubOptions;
struct SubOptionsBuilder;
struct SubOptionsT;

struct DivOptions;
struct DivOptionsBuilder;
struct DivOptionsT;

struct TopKV2Options;
struct TopKV2OptionsBuilder;
struct TopKV2OptionsT;

struct EmbeddingLookupSparseOptions;
struct EmbeddingLookupSparseOptionsBuilder;
struct EmbeddingLookupSparseOptionsT;

struct GatherOptions;
struct GatherOptionsBuilder;
struct GatherOptionsT;

struct TransposeOptions;
struct TransposeOptionsBuilder;
struct TransposeOptionsT;

struct ExpOptions;
struct ExpOptionsBuilder;
struct ExpOptionsT;

struct CosOptions;
struct CosOptionsBuilder;
struct CosOptionsT;

struct ReducerOptions;
struct ReducerOptionsBuilder;
struct ReducerOptionsT;

struct SqueezeOptions;
struct SqueezeOptionsBuilder;
struct SqueezeOptionsT;

struct SplitOptions;
struct SplitOptionsBuilder;
struct SplitOptionsT;

struct SplitVOptions;
struct SplitVOptionsBuilder;
struct SplitVOptionsT;

struct StridedSliceOptions;
struct StridedSliceOptionsBuilder;
struct StridedSliceOptionsT;

struct LogSoftmaxOptions;
struct LogSoftmaxOptionsBuilder;
struct LogSoftmaxOptionsT;

struct CastOptions;
struct CastOptionsBuilder;
struct CastOptionsT;

struct DequantizeOptions;
struct DequantizeOptionsBuilder;
struct DequantizeOptionsT;

struct MaximumMinimumOptions;
struct MaximumMinimumOptionsBuilder;
struct MaximumMinimumOptionsT;

struct TileOptions;
struct TileOptionsBuilder;
struct TileOptionsT;

struct ArgMaxOptions;
struct ArgMaxOptionsBuilder;
struct ArgMaxOptionsT;

struct ArgMinOptions;
struct ArgMinOptionsBuilder;
struct ArgMinOptionsT;

struct GreaterOptions;
struct GreaterOptionsBuilder;
struct GreaterOptionsT;

struct GreaterEqualOptions;
struct GreaterEqualOptionsBuilder;
struct GreaterEqualOptionsT;

struct LessOptions;
struct LessOptionsBuilder;
struct LessOptionsT;

struct LessEqualOptions;
struct LessEqualOptionsBuilder;
struct LessEqualOptionsT;

struct NegOptions;
struct NegOptionsBuilder;
struct NegOptionsT;

struct SelectOptions;
struct SelectOptionsBuilder;
struct SelectOptionsT;

struct SliceOptions;
struct SliceOptionsBuilder;
struct SliceOptionsT;

struct TransposeConvOptions;
struct TransposeConvOptionsBuilder;
struct TransposeConvOptionsT;

struct ExpandDimsOptions;
struct ExpandDimsOptionsBuilder;
struct ExpandDimsOptionsT;

struct SparseToDenseOptions;
struct SparseToDenseOptionsBuilder;
struct SparseToDenseOptionsT;

struct EqualOptions;
struct EqualOptionsBuilder;
struct EqualOptionsT;

struct NotEqualOptions;
struct NotEqualOptionsBuilder;
struct NotEqualOptionsT;

struct ShapeOptions;
struct ShapeOptionsBuilder;
struct ShapeOptionsT;

struct RankOptions;
struct RankOptionsBuilder;
struct RankOptionsT;

struct PowOptions;
struct PowOptionsBuilder;
struct PowOptionsT;

struct FakeQuantOptions;
struct FakeQuantOptionsBuilder;
struct FakeQuantOptionsT;

struct PackOptions;
struct PackOptionsBuilder;
struct PackOptionsT;

struct LogicalOrOptions;
struct LogicalOrOptionsBuilder;
struct LogicalOrOptionsT;

struct OneHotOptions;
struct OneHotOptionsBuilder;
struct OneHotOptionsT;

struct AbsOptions;
struct AbsOptionsBuilder;
struct AbsOptionsT;

struct HardSwishOptions;
struct HardSwishOptionsBuilder;
struct HardSwishOptionsT;

struct LogicalAndOptions;
struct LogicalAndOptionsBuilder;
struct LogicalAndOptionsT;

struct LogicalNotOptions;
struct LogicalNotOptionsBuilder;
struct LogicalNotOptionsT;

struct UnpackOptions;
struct UnpackOptionsBuilder;
struct UnpackOptionsT;

struct FloorDivOptions;
struct FloorDivOptionsBuilder;
struct FloorDivOptionsT;

struct SquareOptions;
struct SquareOptionsBuilder;
struct SquareOptionsT;

struct ZerosLikeOptions;
struct ZerosLikeOptionsBuilder;
struct ZerosLikeOptionsT;

struct FillOptions;
struct FillOptionsBuilder;
struct FillOptionsT;

struct FloorModOptions;
struct FloorModOptionsBuilder;
struct FloorModOptionsT;

struct RangeOptions;
struct RangeOptionsBuilder;
struct RangeOptionsT;

struct LeakyReluOptions;
struct LeakyReluOptionsBuilder;
struct LeakyReluOptionsT;

struct SquaredDifferenceOptions;
struct SquaredDifferenceOptionsBuilder;
struct SquaredDifferenceOptionsT;

struct MirrorPadOptions;
struct MirrorPadOptionsBuilder;
struct MirrorPadOptionsT;

struct UniqueOptions;
struct UniqueOptionsBuilder;
struct UniqueOptionsT;

struct ReverseV2Options;
struct ReverseV2OptionsBuilder;
struct ReverseV2OptionsT;

struct AddNOptions;
struct AddNOptionsBuilder;
struct AddNOptionsT;

struct GatherNdOptions;
struct GatherNdOptionsBuilder;
struct GatherNdOptionsT;

struct WhereOptions;
struct WhereOptionsBuilder;
struct WhereOptionsT;

struct ReverseSequenceOptions;
struct ReverseSequenceOptionsBuilder;
struct ReverseSequenceOptionsT;

struct MatrixDiagOptions;
struct MatrixDiagOptionsBuilder;
struct MatrixDiagOptionsT;

struct QuantizeOptions;
struct QuantizeOptionsBuilder;
struct QuantizeOptionsT;

struct MatrixSetDiagOptions;
struct MatrixSetDiagOptionsBuilder;
struct MatrixSetDiagOptionsT;

struct IfOptions;
struct IfOptionsBuilder;
struct IfOptionsT;

struct CallOnceOptions;
struct CallOnceOptionsBuilder;
struct CallOnceOptionsT;

struct WhileOptions;
struct WhileOptionsBuilder;
struct WhileOptionsT;

struct NonMaxSuppressionV4Options;
struct NonMaxSuppressionV4OptionsBuilder;
struct NonMaxSuppressionV4OptionsT;

struct NonMaxSuppressionV5Options;
struct NonMaxSuppressionV5OptionsBuilder;
struct NonMaxSuppressionV5OptionsT;

struct ScatterNdOptions;
struct ScatterNdOptionsBuilder;
struct ScatterNdOptionsT;

struct SelectV2Options;
struct SelectV2OptionsBuilder;
struct SelectV2OptionsT;

struct DensifyOptions;
struct DensifyOptionsBuilder;
struct DensifyOptionsT;

struct SegmentSumOptions;
struct SegmentSumOptionsBuilder;
struct SegmentSumOptionsT;

struct BatchMatMulOptions;
struct BatchMatMulOptionsBuilder;
struct BatchMatMulOptionsT;

struct CumsumOptions;
struct CumsumOptionsBuilder;
struct CumsumOptionsT;

struct BroadcastToOptions;
struct BroadcastToOptionsBuilder;
struct BroadcastToOptionsT;

struct Rfft2dOptions;
struct Rfft2dOptionsBuilder;
struct Rfft2dOptionsT;

struct HashtableOptions;
struct HashtableOptionsBuilder;
struct HashtableOptionsT;

struct HashtableFindOptions;
struct HashtableFindOptionsBuilder;
struct HashtableFindOptionsT;

struct HashtableImportOptions;
struct HashtableImportOptionsBuilder;
struct HashtableImportOptionsT;

struct HashtableSizeOptions;
struct HashtableSizeOptionsBuilder;
struct HashtableSizeOptionsT;

struct VarHandleOptions;
struct VarHandleOptionsBuilder;
struct VarHandleOptionsT;

struct ReadVariableOptions;
struct ReadVariableOptionsBuilder;
struct ReadVariableOptionsT;

struct AssignVariableOptions;
struct AssignVariableOptionsBuilder;
struct AssignVariableOptionsT;

struct RandomOptions;
struct RandomOptionsBuilder;
struct RandomOptionsT;

struct BucketizeOptions;
struct BucketizeOptionsBuilder;
struct BucketizeOptionsT;

struct GeluOptions;
struct GeluOptionsBuilder;
struct GeluOptionsT;

struct DynamicUpdateSliceOptions;
struct DynamicUpdateSliceOptionsBuilder;
struct DynamicUpdateSliceOptionsT;

struct UnsortedSegmentProdOptions;
struct UnsortedSegmentProdOptionsBuilder;
struct UnsortedSegmentProdOptionsT;

struct UnsortedSegmentMaxOptions;
struct UnsortedSegmentMaxOptionsBuilder;
struct UnsortedSegmentMaxOptionsT;

struct UnsortedSegmentSumOptions;
struct UnsortedSegmentSumOptionsBuilder;
struct UnsortedSegmentSumOptionsT;

struct ATan2Options;
struct ATan2OptionsBuilder;
struct ATan2OptionsT;

struct UnsortedSegmentMinOptions;
struct UnsortedSegmentMinOptionsBuilder;
struct UnsortedSegmentMinOptionsT;

struct SignOptions;
struct SignOptionsBuilder;
struct SignOptionsT;

struct OperatorCode;
struct OperatorCodeBuilder;
struct OperatorCodeT;

struct Operator;
struct OperatorBuilder;
struct OperatorT;

struct SubGraph;
struct SubGraphBuilder;
struct SubGraphT;

struct Buffer;
struct BufferBuilder;
struct BufferT;

struct Metadata;
struct MetadataBuilder;
struct MetadataT;

struct TensorMap;
struct TensorMapBuilder;
struct TensorMapT;

struct SignatureDef;
struct SignatureDefBuilder;
struct SignatureDefT;

struct Model;
struct ModelBuilder;
struct ModelT;

enum TensorType : int8_t {
  TensorType_FLOAT32 = 0,
  TensorType_FLOAT16 = 1,
  TensorType_INT32 = 2,
  TensorType_UINT8 = 3,
  TensorType_INT64 = 4,
  TensorType_STRING = 5,
  TensorType_BOOL = 6,
  TensorType_INT16 = 7,
  TensorType_COMPLEX64 = 8,
  TensorType_INT8 = 9,
  TensorType_FLOAT64 = 10,
  TensorType_COMPLEX128 = 11,
  TensorType_UINT64 = 12,
  TensorType_RESOURCE = 13,
  TensorType_VARIANT = 14,
  TensorType_UINT32 = 15,
  TensorType_UINT16 = 16,
  TensorType_INT4 = 17,
  TensorType_MIN = TensorType_FLOAT32,
  TensorType_MAX = TensorType_INT4
};

inline const TensorType (&EnumValuesTensorType())[18] {
  static const TensorType values[] = {
    TensorType_FLOAT32,
    TensorType_FLOAT16,
    TensorType_INT32,
    TensorType_UINT8,
    TensorType_INT64,
    TensorType_STRING,
    TensorType_BOOL,
    TensorType_INT16,
    TensorType_COMPLEX64,
    TensorType_INT8,
    TensorType_FLOAT64,
    TensorType_COMPLEX128,
    TensorType_UINT64,
    TensorType_RESOURCE,
    TensorType_VARIANT,
    TensorType_UINT32,
    TensorType_UINT16,
    TensorType_INT4
  };
  return values;
}

inline const char * const *EnumNamesTensorType() {
  static const char * const names[19] = {
    "FLOAT32",
    "FLOAT16",
    "INT32",
    "UINT8",
    "INT64",
    "STRING",
    "BOOL",
    "INT16",
    "COMPLEX64",
    "INT8",
    "FLOAT64",
    "COMPLEX128",
    "UINT64",
    "RESOURCE",
    "VARIANT",
    "UINT32",
    "UINT16",
    "INT4",
    nullptr
  };
  return names;
}

inline const char *EnumNameTensorType(TensorType e) {
  if (flatbuffers::IsOutRange(e, TensorType_FLOAT32, TensorType_INT4)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesTensorType()[index];
}


enum BuiltinOperator : int32_t {
  BuiltinOperator_ADD = 0,
  BuiltinOperator_AVERAGE_POOL_2D = 1,
  BuiltinOperator_CONCATENATION = 2,
  BuiltinOperator_CONV_2D = 3,
  BuiltinOperator_DEPTHWISE_CONV_2D = 4,
  BuiltinOperator_DEPTH_TO_SPACE = 5,
  BuiltinOperator_DEQUANTIZE = 6,
  BuiltinOperator_EMBEDDING_LOOKUP = 7,
  BuiltinOperator_FLOOR = 8,
  BuiltinOperator_FULLY_CONNECTED = 9,
  BuiltinOperator_HASHTABLE_LOOKUP = 10,
  BuiltinOperator_L2_NORMALIZATION = 11,
  BuiltinOperator_L2_POOL_2D = 12,
  BuiltinOperator_LOCAL_RESPONSE_NORMALIZATION = 13,
  BuiltinOperator_LOGISTIC = 14,
  BuiltinOperator_LSH_PROJECTION = 15,
  BuiltinOperator_LSTM = 16,
  BuiltinOperator_MAX_POOL_2D = 17,
  BuiltinOperator_MUL = 18,
  BuiltinOperator_RELU = 19,
  BuiltinOperator_RELU_N1_TO_1 = 20,
  BuiltinOperator_RELU6 = 21,
  BuiltinOperator_RESHAPE = 22,
  BuiltinOperator_RESIZE_BILINEAR = 23,
  BuiltinOperator_RNN = 24,
  BuiltinOperator_SOFTMAX = 25,
  BuiltinOperator_SPACE_TO_DEPTH = 26,
  BuiltinOperator_SVDF = 27,
  BuiltinOperator_TANH = 28,
  BuiltinOperator_CONCAT_EMBEDDINGS = 29,
  BuiltinOperator_SKIP_GRAM = 30,
  BuiltinOperator_CALL = 31,
  BuiltinOperator_CUSTOM = 32,
  BuiltinOperator_EMBEDDING_LOOKUP_SPARSE = 33,
  BuiltinOperator_PAD = 34,
  BuiltinOperator_UNIDIRECTIONAL_SEQUENCE_RNN = 35,
  BuiltinOperator_GATHER = 36,
  BuiltinOperator_BATCH_TO_SPACE_ND = 37,
  BuiltinOperator_SPACE_TO_BATCH_ND = 38,
  BuiltinOperator_TRANSPOSE = 39,
  BuiltinOperator_MEAN = 40,
  BuiltinOperator_SUB = 41,
  BuiltinOperator_DIV = 42,
  BuiltinOperator_SQUEEZE = 43,
  BuiltinOperator_UNIDIRECTIONAL_SEQUENCE_LSTM = 44,
  BuiltinOperator_STRIDED_SLICE = 45,
  BuiltinOperator_BIDIRECTIONAL_SEQUENCE_RNN = 46,
  BuiltinOperator_EXP = 47,
  BuiltinOperator_TOPK_V2 = 48,
  BuiltinOperator_SPLIT = 49,
  BuiltinOperator_LOG_SOFTMAX = 50,
  BuiltinOperator_DELEGATE = 51,
  BuiltinOperator_BIDIRECTIONAL_SEQUENCE_LSTM = 52,
  BuiltinOperator_CAST = 53,
  BuiltinOperator_PRELU = 54,
  BuiltinOperator_MAXIMUM = 55,
  BuiltinOperator_ARG_MAX = 56,
  BuiltinOperator_MINIMUM = 57,
  BuiltinOperator_LESS = 58,
  BuiltinOperator_NEG = 59,
  BuiltinOperator_PADV2 = 60,
  BuiltinOperator_GREATER = 61,
  BuiltinOperator_GREATER_EQUAL = 62,
  BuiltinOperator_LESS_EQUAL = 63,
  BuiltinOperator_SELECT = 64,
  BuiltinOperator_SLICE = 65,
  BuiltinOperator_SIN = 66,
  BuiltinOperator_TRANSPOSE_CONV = 67,
  BuiltinOperator_SPARSE_TO_DENSE = 68,
  BuiltinOperator_TILE = 69,
  BuiltinOperator_EXPAND_DIMS = 70,
  BuiltinOperator_EQUAL = 71,
  BuiltinOperator_NOT_EQUAL = 72,
  BuiltinOperator_LOG = 73,
  BuiltinOperator_SUM = 74,
  BuiltinOperator_SQRT = 75,
  BuiltinOperator_RSQRT = 76,
  BuiltinOperator_SHAPE = 77,
  BuiltinOperator_POW = 78,
  BuiltinOperator_ARG_MIN = 79,
  BuiltinOperator_FAKE_QUANT = 80,
  BuiltinOperator_REDUCE_PROD = 81,
  BuiltinOperator_REDUCE_MAX = 82,
  BuiltinOperator_PACK = 83,
  BuiltinOperator_LOGICAL_OR = 84,
  BuiltinOperator_ONE_HOT = 85,
  BuiltinOperator_LOGICAL_AND = 86,
  BuiltinOperator_LOGICAL_NOT = 87,
  BuiltinOperator_UNPACK = 88,
  BuiltinOperator_REDUCE_MIN = 89,
  BuiltinOperator_FLOOR_DIV = 90,
  BuiltinOperator_REDUCE_ANY = 91,
  BuiltinOperator_SQUARE = 92,
  BuiltinOperator_ZEROS_LIKE = 93,
  BuiltinOperator_FILL = 94,
  BuiltinOperator_FLOOR_MOD = 95,
  BuiltinOperator_RANGE = 96,
  BuiltinOperator_RESIZE_NEAREST_NEIGHBOR = 97,
  BuiltinOperator_LEAKY_RELU = 98,
  BuiltinOperator_SQUARED_DIFFERENCE = 99,
  BuiltinOperator_MIRROR_PAD = 100,
  BuiltinOperator_ABS = 101,
  BuiltinOperator_SPLIT_V = 102,
  BuiltinOperator_UNIQUE = 103,
  BuiltinOperator_CEIL = 104,
  BuiltinOperator_REVERSE_V2 = 105,
  BuiltinOperator_ADD_N = 106,
  BuiltinOperator_GATHER_ND = 107,
  BuiltinOperator_COS = 108,
  BuiltinOperator_WHERE = 109,
  BuiltinOperator_RANK = 110,
  BuiltinOperator_ELU = 111,
  BuiltinOperator_REVERSE_SEQUENCE = 112,
  BuiltinOperator_MATRIX_DIAG = 113,
  BuiltinOperator_QUANTIZE = 114,
  BuiltinOperator_MATRIX_SET_DIAG = 115,
  BuiltinOperator_ROUND = 116,
  BuiltinOperator_HARD_SWISH = 117,
  BuiltinOperator_IF = 118,
  BuiltinOperator_WHILE = 119,
  BuiltinOperator_NON_MAX_SUPPRESSION_V4 = 120,
  BuiltinOperator_NON_MAX_SUPPRESSION_V5 = 121,
  BuiltinOperator_SCATTER_ND = 122,
  BuiltinOperator_SELECT_V2 = 123,
  BuiltinOperator_DENSIFY = 124,
  BuiltinOperator_SEGMENT_SUM = 125,
  BuiltinOperator_BATCH_MATMUL = 126,
  BuiltinOperator_PLACEHOLDER_FOR_GREATER_OP_CODES = 127,
  BuiltinOperator_CUMSUM = 128,
  BuiltinOperator_CALL_ONCE = 129,
  BuiltinOperator_BROADCAST_TO = 130,
  BuiltinOperator_RFFT2D = 131,
  BuiltinOperator_CONV_3D = 132,
  BuiltinOperator_IMAG = 133,
  BuiltinOperator_REAL = 134,
  BuiltinOperator_COMPLEX_ABS = 135,
  BuiltinOperator_HASHTABLE = 136,
  BuiltinOperator_HASHTABLE_FIND = 137,
  BuiltinOperator_HASHTABLE_IMPORT = 138,
  BuiltinOperator_HASHTABLE_SIZE = 139,
  BuiltinOperator_REDUCE_ALL = 140,
  BuiltinOperator_CONV_3D_TRANSPOSE = 141,
  BuiltinOperator_VAR_HANDLE = 142,
  BuiltinOperator_READ_VARIABLE = 143,
  BuiltinOperator_ASSIGN_VARIABLE = 144,
  BuiltinOperator_BROADCAST_ARGS = 145,
  BuiltinOperator_RANDOM_STANDARD_NORMAL = 146,
  BuiltinOperator_BUCKETIZE = 147,
  BuiltinOperator_RANDOM_UNIFORM = 148,
  BuiltinOperator_MULTINOMIAL = 149,
  BuiltinOperator_GELU = 150,
  BuiltinOperator_DYNAMIC_UPDATE_SLICE = 151,
  BuiltinOperator_RELU_0_TO_1 = 152,
  BuiltinOperator_UNSORTED_SEGMENT_PROD = 153,
  BuiltinOperator_UNSORTED_SEGMENT_MAX = 154,
  BuiltinOperator_UNSORTED_SEGMENT_SUM = 155,
  BuiltinOperator_ATAN2 = 156,
  BuiltinOperator_UNSORTED_SEGMENT_MIN = 157,
  BuiltinOperator_SIGN = 158,
  BuiltinOperator_MIN = BuiltinOperator_ADD,
  BuiltinOperator_MAX = BuiltinOperator_SIGN
};

inline const BuiltinOperator (&EnumValuesBuiltinOperator())[159] {
  static const BuiltinOperator values[] = {
    BuiltinOperator_ADD,
    BuiltinOperator_AVERAGE_POOL_2D,
    BuiltinOperator_CONCATENATION,
    BuiltinOperator_CONV_2D,
    BuiltinOperator_DEPTHWISE_CONV_2D,
    BuiltinOperator_DEPTH_TO_SPACE,
    BuiltinOperator_DEQUANTIZE,
    BuiltinOperator_EMBEDDING_LOOKUP,
    BuiltinOperator_FLOOR,
    BuiltinOperator_FULLY_CONNECTED,
    BuiltinOperator_HASHTABLE_LOOKUP,
    BuiltinOperator_L2_NORMALIZATION,
    BuiltinOperator_L2_POOL_2D,
    BuiltinOperator_LOCAL_RESPONSE_NORMALIZATION,
    BuiltinOperator_LOGISTIC,
    BuiltinOperator_LSH_PROJECTION,
    BuiltinOperator_LSTM,
    BuiltinOperator_MAX_POOL_2D,
    BuiltinOperator_MUL,
    BuiltinOperator_RELU,
    BuiltinOperator_RELU_N1_TO_1,
    BuiltinOperator_RELU6,
    BuiltinOperator_RESHAPE,
    BuiltinOperator_RESIZE_BILINEAR,
    BuiltinOperator_RNN,
    BuiltinOperator_SOFTMAX,
    BuiltinOperator_SPACE_TO_DEPTH,
    BuiltinOperator_SVDF,
    BuiltinOperator_TANH,
    BuiltinOperator_CONCAT_EMBEDDINGS,
    BuiltinOperator_SKIP_GRAM,
    BuiltinOperator_CALL,
    BuiltinOperator_CUSTOM,
    BuiltinOperator_EMBEDDING_LOOKUP_SPARSE,
    BuiltinOperator_PAD,
    BuiltinOperator_UNIDIRECTIONAL_SEQUENCE_RNN,
    BuiltinOperator_GATHER,
    BuiltinOperator_BATCH_TO_SPACE_ND,
    BuiltinOperator_SPACE_TO_BATCH_ND,
    BuiltinOperator_TRANSPOSE,
    BuiltinOperator_MEAN,
    BuiltinOperator_SUB,
    BuiltinOperator_DIV,
    BuiltinOperator_SQUEEZE,
    BuiltinOperator_UNIDIRECTIONAL_SEQUENCE_LSTM,
    BuiltinOperator_STRIDED_SLICE,
    BuiltinOperator_BIDIRECTIONAL_SEQUENCE_RNN,
    BuiltinOperator_EXP,
    BuiltinOperator_TOPK_V2,
    BuiltinOperator_SPLIT,
    BuiltinOperator_LOG_SOFTMAX,
    BuiltinOperator_DELEGATE,
    BuiltinOperator_BIDIRECTIONAL_SEQUENCE_LSTM,
    BuiltinOperator_CAST,
    BuiltinOperator_PRELU,
    BuiltinOperator_MAXIMUM,
    BuiltinOperator_ARG_MAX,
    BuiltinOperator_MINIMUM,
    BuiltinOperator_LESS,
    BuiltinOperator_NEG,
    BuiltinOperator_PADV2,
    BuiltinOperator_GREATER,
    BuiltinOperator_GREATER_EQUAL,
    BuiltinOperator_LESS_EQUAL,
    BuiltinOperator_SELECT,
    BuiltinOperator_SLICE,
    BuiltinOperator_SIN,
    BuiltinOperator_TRANSPOSE_CONV,
    BuiltinOperator_SPARSE_TO_DENSE,
    BuiltinOperator_TILE,
    BuiltinOperator_EXPAND_DIMS,
    BuiltinOperator_EQUAL,
    BuiltinOperator_NOT_EQUAL,
    BuiltinOperator_LOG,
    BuiltinOperator_SUM,
    BuiltinOperator_SQRT,
    BuiltinOperator_RSQRT,
    BuiltinOperator_SHAPE,
    BuiltinOperator_POW,
    BuiltinOperator_ARG_MIN,
    BuiltinOperator_FAKE_QUANT,
    BuiltinOperator_REDUCE_PROD,
    BuiltinOperator_REDUCE_MAX,
    BuiltinOperator_PACK,
    BuiltinOperator_LOGICAL_OR,
    BuiltinOperator_ONE_HOT,
    BuiltinOperator_LOGICAL_AND,
    BuiltinOperator_LOGICAL_NOT,
    BuiltinOperator_UNPACK,
    BuiltinOperator_REDUCE_MIN,
    BuiltinOperator_FLOOR_DIV,
    BuiltinOperator_REDUCE_ANY,
    BuiltinOperator_SQUARE,
    BuiltinOperator_ZEROS_LIKE,
    BuiltinOperator_FILL,
    BuiltinOperator_FLOOR_MOD,
    BuiltinOperator_RANGE,
    BuiltinOperator_RESIZE_NEAREST_NEIGHBOR,
    BuiltinOperator_LEAKY_RELU,
    BuiltinOperator_SQUARED_DIFFERENCE,
    BuiltinOperator_MIRROR_PAD,
    BuiltinOperator_ABS,
    BuiltinOperator_SPLIT_V,
    BuiltinOperator_UNIQUE,
    BuiltinOperator_CEIL,
    BuiltinOperator_REVERSE_V2,
    BuiltinOperator_ADD_N,
    BuiltinOperator_GATHER_ND,
    BuiltinOperator_COS,
    BuiltinOperator_WHERE,
    BuiltinOperator_RANK,
    BuiltinOperator_ELU,
    BuiltinOperator_REVERSE_SEQUENCE,
    BuiltinOperator_MATRIX_DIAG,
    BuiltinOperator_QUANTIZE,
    BuiltinOperator_MATRIX_SET_DIAG,
    BuiltinOperator_ROUND,
    BuiltinOperator_HARD_SWISH,
    BuiltinOperator_IF,
    BuiltinOperator_WHILE,
    BuiltinOperator_NON_MAX_SUPPRESSION_V4,
    BuiltinOperator_NON_MAX_SUPPRESSION_V5,
    BuiltinOperator_SCATTER_ND,
    BuiltinOperator_SELECT_V2,
    BuiltinOperator_DENSIFY,
    BuiltinOperator_SEGMENT_SUM,
    BuiltinOperator_BATCH_MATMUL,
    BuiltinOperator_PLACEHOLDER_FOR_GREATER_OP_CODES,
    BuiltinOperator_CUMSUM,
    BuiltinOperator_CALL_ONCE,
    BuiltinOperator_BROADCAST_TO,
    BuiltinOperator_RFFT2D,
    BuiltinOperator_CONV_3D,
    BuiltinOperator_IMAG,
    BuiltinOperator_REAL,
    BuiltinOperator_COMPLEX_ABS,
    BuiltinOperator_HASHTABLE,
    BuiltinOperator_HASHTABLE_FIND,
    BuiltinOperator_HASHTABLE_IMPORT,
    BuiltinOperator_HASHTABLE_SIZE,
    BuiltinOperator_REDUCE_ALL,
    BuiltinOperator_CONV_3D_TRANSPOSE,
    BuiltinOperator_VAR_HANDLE,
    BuiltinOperator_READ_VARIABLE,
    BuiltinOperator_ASSIGN_VARIABLE,
    BuiltinOperator_BROADCAST_ARGS,
    BuiltinOperator_RANDOM_STANDARD_NORMAL,
    BuiltinOperator_BUCKETIZE,
    BuiltinOperator_RANDOM_UNIFORM,
    BuiltinOperator_MULTINOMIAL,
    BuiltinOperator_GELU,
    BuiltinOperator_DYNAMIC_UPDATE_SLICE,
    BuiltinOperator_RELU_0_TO_1,
    BuiltinOperator_UNSORTED_SEGMENT_PROD,
    BuiltinOperator_UNSORTED_SEGMENT_MAX,
    BuiltinOperator_UNSORTED_SEGMENT_SUM,
    BuiltinOperator_ATAN2,
    BuiltinOperator_UNSORTED_SEGMENT_MIN,
    BuiltinOperator_SIGN
  };
  return values;
}

inline const char * const *EnumNamesBuiltinOperator() {
  static const char * const names[160] = {
    "ADD",
    "AVERAGE_POOL_2D",
    "CONCATENATION",
    "CONV_2D",
    "DEPTHWISE_CONV_2D",
    "DEPTH_TO_SPACE",
    "DEQUANTIZE",
    "EMBEDDING_LOOKUP",
    "FLOOR",
    "FULLY_CONNECTED",
    "HASHTABLE_LOOKUP",
    "L2_NORMALIZATION",
    "L2_POOL_2D",
    "LOCAL_RESPONSE_NORMALIZATION",
    "LOGISTIC",
    "LSH_PROJECTION",
    "LSTM",
    "MAX_POOL_2D",
    "MUL",
    "RELU",
    "RELU_N1_TO_1",
    "RELU6",
    "RESHAPE",
    "RESIZE_BILINEAR",
    "RNN",
    "SOFTMAX",
    "SPACE_TO_DEPTH",
    "SVDF",
    "TANH",
    "CONCAT_EMBEDDINGS",
    "SKIP_GRAM",
    "CALL",
    "CUSTOM",
    "EMBEDDING_LOOKUP_SPARSE",
    "PAD",
    "UNIDIRECTIONAL_SEQUENCE_RNN",
    "GATHER",
    "BATCH_TO_SPACE_ND",
    "SPACE_TO_BATCH_ND",
    "TRANSPOSE",
    "MEAN",
    "SUB",
    "DIV",
    "SQUEEZE",
    "UNIDIRECTIONAL_SEQUENCE_LSTM",
    "STRIDED_SLICE",
    "BIDIRECTIONAL_SEQUENCE_RNN",
    "EXP",
    "TOPK_V2",
    "SPLIT",
    "LOG_SOFTMAX",
    "DELEGATE",
    "BIDIRECTIONAL_SEQUENCE_LSTM",
    "CAST",
    "PRELU",
    "MAXIMUM",
    "ARG_MAX",
    "MINIMUM",
    "LESS",
    "NEG",
    "PADV2",
    "GREATER",
    "GREATER_EQUAL",
    "LESS_EQUAL",
    "SELECT",
    "SLICE",
    "SIN",
    "TRANSPOSE_CONV",
    "SPARSE_TO_DENSE",
    "TILE",
    "EXPAND_DIMS",
    "EQUAL",
    "NOT_EQUAL",
    "LOG",
    "SUM",
    "SQRT",
    "RSQRT",
    "SHAPE",
    "POW",
    "ARG_MIN",
    "FAKE_QUANT",
    "REDUCE_PROD",
    "REDUCE_MAX",
    "PACK",
    "LOGICAL_OR",
    "ONE_HOT",
    "LOGICAL_AND",
    "LOGICAL_NOT",
    "UNPACK",
    "REDUCE_MIN",
    "FLOOR_DIV",
    "REDUCE_ANY",
    "SQUARE",
    "ZEROS_LIKE",
    "FILL",
    "FLOOR_MOD",
    "RANGE",
    "RESIZE_NEAREST_NEIGHBOR",
    "LEAKY_RELU",
    "SQUARED_DIFFERENCE",
    "MIRROR_PAD",
    "ABS",
    "SPLIT_V",
    "UNIQUE",
    "CEIL",
    "REVERSE_V2",
    "ADD_N",
    "GATHER_ND",
    "COS",
    "WHERE",
    "RANK",
    "ELU",
    "REVERSE_SEQUENCE",
    "MATRIX_DIAG",
    "QUANTIZE",
    "MATRIX_SET_DIAG",
    "ROUND",
    "HARD_SWISH",
    "IF",
    "WHILE",
    "NON_MAX_SUPPRESSION_V4",
    "NON_MAX_SUPPRESSION_V5",
    "SCATTER_ND",
    "SELECT_V2",
    "DENSIFY",
    "SEGMENT_SUM",
    "BATCH_MATMUL",
    "PLACEHOLDER_FOR_GREATER_OP_CODES",
    "CUMSUM",
    "CALL_ONCE",
    "BROADCAST_TO",
    "RFFT2D",
    "CONV_3D",
    "IMAG",
    "REAL",
    "COMPLEX_ABS",
    "HASHTABLE",
    "HASHTABLE_FIND",
    "HASHTABLE_IMPORT",
    "HASHTABLE_SIZE",
    "REDUCE_ALL",
    "CONV_3D_TRANSPOSE",
    "VAR_HANDLE",
    "READ_VARIABLE",
    "ASSIGN_VARIABLE",
    "BROADCAST_ARGS",
    "RANDOM_STANDARD_NORMAL",
    "BUCKETIZE",
    "RANDOM_UNIFORM",
    "MULTINOMIAL",
    "GELU",
    "DYNAMIC_UPDATE_SLICE",
    "RELU_0_TO_1",
    "UNSORTED_SEGMENT_PROD",
    "UNSORTED_SEGMENT_MAX",
    "UNSORTED_SEGMENT_SUM",
    "ATAN2",
    "UNSORTED_SEGMENT_MIN",
    "SIGN",
    nullptr
  };
  return names;
}

inline const char *EnumNameBuiltinOperator(BuiltinOperator e) {
  if (flatbuffers::IsOutRange(e, BuiltinOperator_ADD, BuiltinOperator_SIGN)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesBuiltinOperator()[index];
}

enum BuiltinOptions : uint8_t {
  BuiltinOptions_NONE = 0,
  BuiltinOptions_Conv2DOptions = 1,
  BuiltinOptions_DepthwiseConv2DOptions = 2,
  BuiltinOptions_ConcatEmbeddingsOptions = 3,
  BuiltinOptions_LSHProjectionOptions = 4,
  BuiltinOptions_Pool2DOptions = 5,
  BuiltinOptions_SVDFOptions = 6,
  BuiltinOptions_RNNOptions = 7,
  BuiltinOptions_FullyConnectedOptions = 8,
  BuiltinOptions_SoftmaxOptions = 9,
  BuiltinOptions_ConcatenationOptions = 10,
  BuiltinOptions_AddOptions = 11,
  BuiltinOptions_L2NormOptions = 12,
  BuiltinOptions_LocalResponseNormalizationOptions = 13,
  BuiltinOptions_LSTMOptions = 14,
  BuiltinOptions_ResizeBilinearOptions = 15,
  BuiltinOptions_CallOptions = 16,
  BuiltinOptions_ReshapeOptions = 17,
  BuiltinOptions_SkipGramOptions = 18,
  BuiltinOptions_SpaceToDepthOptions = 19,
  BuiltinOptions_EmbeddingLookupSparseOptions = 20,
  BuiltinOptions_MulOptions = 21,
  BuiltinOptions_PadOptions = 22,
  BuiltinOptions_GatherOptions = 23,
  BuiltinOptions_BatchToSpaceNDOptions = 24,
  BuiltinOptions_SpaceToBatchNDOptions = 25,
  BuiltinOptions_TransposeOptions = 26,
  BuiltinOptions_ReducerOptions = 27,
  BuiltinOptions_SubOptions = 28,
  BuiltinOptions_DivOptions = 29,
  BuiltinOptions_SqueezeOptions = 30,
  BuiltinOptions_SequenceRNNOptions = 31,
  BuiltinOptions_StridedSliceOptions = 32,
  BuiltinOptions_ExpOptions = 33,
  BuiltinOptions_TopKV2Options = 34,
  BuiltinOptions_SplitOptions = 35,
  BuiltinOptions_LogSoftmaxOptions = 36,
  BuiltinOptions_CastOptions = 37,
  BuiltinOptions_DequantizeOptions = 38,
  BuiltinOptions_MaximumMinimumOptions = 39,
  BuiltinOptions_ArgMaxOptions = 40,
  BuiltinOptions_LessOptions = 41,
  BuiltinOptions_NegOptions = 42,
  BuiltinOptions_PadV2Options = 43,
  BuiltinOptions_GreaterOptions = 44,
  BuiltinOptions_GreaterEqualOptions = 45,
  BuiltinOptions_LessEqualOptions = 46,
  BuiltinOptions_SelectOptions = 47,
  BuiltinOptions_SliceOptions = 48,
  BuiltinOptions_TransposeConvOptions = 49,
  BuiltinOptions_SparseToDenseOptions = 50,
  BuiltinOptions_TileOptions = 51,
  BuiltinOptions_ExpandDimsOptions = 52,
  BuiltinOptions_EqualOptions = 53,
  BuiltinOptions_NotEqualOptions = 54,
  BuiltinOptions_ShapeOptions = 55,
  BuiltinOptions_PowOptions = 56,
  BuiltinOptions_ArgMinOptions = 57,
  BuiltinOptions_FakeQuantOptions = 58,
  BuiltinOptions_PackOptions = 59,
  BuiltinOptions_LogicalOrOptions = 60,
  BuiltinOptions_OneHotOptions = 61,
  BuiltinOptions_LogicalAndOptions = 62,
  BuiltinOptions_LogicalNotOptions = 63,
  BuiltinOptions_UnpackOptions = 64,
  BuiltinOptions_FloorDivOptions = 65,
  BuiltinOptions_SquareOptions = 66,
  BuiltinOptions_ZerosLikeOptions = 67,
  BuiltinOptions_FillOptions = 68,
  BuiltinOptions_BidirectionalSequenceLSTMOptions = 69,
  BuiltinOptions_BidirectionalSequenceRNNOptions = 70,
  BuiltinOptions_UnidirectionalSequenceLSTMOptions = 71,
  BuiltinOptions_FloorModOptions = 72,
  BuiltinOptions_RangeOptions = 73,
  BuiltinOptions_ResizeNearestNeighborOptions = 74,
  BuiltinOptions_LeakyReluOptions = 75,
  BuiltinOptions_SquaredDifferenceOptions = 76,
  BuiltinOptions_MirrorPadOptions = 77,
  BuiltinOptions_AbsOptions = 78,
  BuiltinOptions_SplitVOptions = 79,
  BuiltinOptions_UniqueOptions = 80,
  BuiltinOptions_ReverseV2Options = 81,
  BuiltinOptions_AddNOptions = 82,
  BuiltinOptions_GatherNdOptions = 83,
  BuiltinOptions_CosOptions = 84,
  BuiltinOptions_WhereOptions = 85,
  BuiltinOptions_RankOptions = 86,
  BuiltinOptions_ReverseSequenceOptions = 87,
  BuiltinOptions_MatrixDiagOptions = 88,
  BuiltinOptions_QuantizeOptions = 89,
  BuiltinOptions_MatrixSetDiagOptions = 90,
  BuiltinOptions_HardSwishOptions = 91,
  BuiltinOptions_IfOptions = 92,
  BuiltinOptions_WhileOptions = 93,
  BuiltinOptions_DepthToSpaceOptions = 94,
  BuiltinOptions_NonMaxSuppressionV4Options = 95,
  BuiltinOptions_NonMaxSuppressionV5Options = 96,
  BuiltinOptions_ScatterNdOptions = 97,
  BuiltinOptions_SelectV2Options = 98,
  BuiltinOptions_DensifyOptions = 99,
  BuiltinOptions_SegmentSumOptions = 100,
  BuiltinOptions_BatchMatMulOptions = 101,
  BuiltinOptions_CumsumOptions = 102,
  BuiltinOptions_CallOnceOptions = 103,
  BuiltinOptions_BroadcastToOptions = 104,
  BuiltinOptions_Rfft2dOptions = 105,
  BuiltinOptions_Conv3DOptions = 106,
  BuiltinOptions_HashtableOptions = 107,
  BuiltinOptions_HashtableFindOptions = 108,
  BuiltinOptions_HashtableImportOptions = 109,
  BuiltinOptions_HashtableSizeOptions = 110,
  BuiltinOptions_VarHandleOptions = 111,
  BuiltinOptions_ReadVariableOptions = 112,
  BuiltinOptions_AssignVariableOptions = 113,
  BuiltinOptions_RandomOptions = 114,
  BuiltinOptions_BucketizeOptions = 115,
  BuiltinOptions_GeluOptions = 116,
  BuiltinOptions_DynamicUpdateSliceOptions = 117,
  BuiltinOptions_UnsortedSegmentProdOptions = 118,
  BuiltinOptions_UnsortedSegmentMaxOptions = 119,
  BuiltinOptions_UnsortedSegmentMinOptions = 120,
  BuiltinOptions_UnsortedSegmentSumOptions = 121,
  BuiltinOptions_ATan2Options = 122,
  BuiltinOptions_SignOptions = 123,
  BuiltinOptions_MIN = BuiltinOptions_NONE,
  BuiltinOptions_MAX = BuiltinOptions_SignOptions
};

inline const BuiltinOptions (&EnumValuesBuiltinOptions())[124] {
  static const BuiltinOptions values[] = {
    BuiltinOptions_NONE,
    BuiltinOptions_Conv2DOptions,
    BuiltinOptions_DepthwiseConv2DOptions,
    BuiltinOptions_ConcatEmbeddingsOptions,
    BuiltinOptions_LSHProjectionOptions,
    BuiltinOptions_Pool2DOptions,
    BuiltinOptions_SVDFOptions,
    BuiltinOptions_RNNOptions,
    BuiltinOptions_FullyConnectedOptions,
    BuiltinOptions_SoftmaxOptions,
    BuiltinOptions_ConcatenationOptions,
    BuiltinOptions_AddOptions,
    BuiltinOptions_L2NormOptions,
    BuiltinOptions_LocalResponseNormalizationOptions,
    BuiltinOptions_LSTMOptions,
    BuiltinOptions_ResizeBilinearOptions,
    BuiltinOptions_CallOptions,
    BuiltinOptions_ReshapeOptions,
    BuiltinOptions_SkipGramOptions,
    BuiltinOptions_SpaceToDepthOptions,
    BuiltinOptions_EmbeddingLookupSparseOptions,
    BuiltinOptions_MulOptions,
    BuiltinOptions_PadOptions,
    BuiltinOptions_GatherOptions,
    BuiltinOptions_BatchToSpaceNDOptions,
    BuiltinOptions_SpaceToBatchNDOptions,
    BuiltinOptions_TransposeOptions,
    BuiltinOptions_ReducerOptions,
    BuiltinOptions_SubOptions,
    BuiltinOptions_DivOptions,
    BuiltinOptions_SqueezeOptions,
    BuiltinOptions_SequenceRNNOptions,
    BuiltinOptions_StridedSliceOptions,
    BuiltinOptions_ExpOptions,
    BuiltinOptions_TopKV2Options,
    BuiltinOptions_SplitOptions,
    BuiltinOptions_LogSoftmaxOptions,
    BuiltinOptions_CastOptions,
    BuiltinOptions_DequantizeOptions,
    BuiltinOptions_MaximumMinimumOptions,
    BuiltinOptions_ArgMaxOptions,
    BuiltinOptions_LessOptions,
    BuiltinOptions_NegOptions,
    BuiltinOptions_PadV2Options,
    BuiltinOptions_GreaterOptions,
    BuiltinOptions_GreaterEqualOptions,
    BuiltinOptions_LessEqualOptions,
    BuiltinOptions_SelectOptions,
    BuiltinOptions_SliceOptions,
    BuiltinOptions_TransposeConvOptions,
    BuiltinOptions_SparseToDenseOptions,
    BuiltinOptions_TileOptions,
    BuiltinOptions_ExpandDimsOptions,
    BuiltinOptions_EqualOptions,
    BuiltinOptions_NotEqualOptions,
    BuiltinOptions_ShapeOptions,
    BuiltinOptions_PowOptions,
    BuiltinOptions_ArgMinOptions,
    BuiltinOptions_FakeQuantOptions,
    BuiltinOptions_PackOptions,
    BuiltinOptions_LogicalOrOptions,
    BuiltinOptions_OneHotOptions,
    BuiltinOptions_LogicalAndOptions,
    BuiltinOptions_LogicalNotOptions,
    BuiltinOptions_UnpackOptions,
    BuiltinOptions_FloorDivOptions,
    BuiltinOptions_SquareOptions,
    BuiltinOptions_ZerosLikeOptions,
    BuiltinOptions_FillOptions,
    BuiltinOptions_BidirectionalSequenceLSTMOptions,
    BuiltinOptions_BidirectionalSequenceRNNOptions,
    BuiltinOptions_UnidirectionalSequenceLSTMOptions,
    BuiltinOptions_FloorModOptions,
    BuiltinOptions_RangeOptions,
    BuiltinOptions_ResizeNearestNeighborOptions,
    BuiltinOptions_LeakyReluOptions,
    BuiltinOptions_SquaredDifferenceOptions,
    BuiltinOptions_MirrorPadOptions,
    BuiltinOptions_AbsOptions,
    BuiltinOptions_SplitVOptions,
    BuiltinOptions_UniqueOptions,
    BuiltinOptions_ReverseV2Options,
    BuiltinOptions_AddNOptions,
    BuiltinOptions_GatherNdOptions,
    BuiltinOptions_CosOptions,
    BuiltinOptions_WhereOptions,
    BuiltinOptions_RankOptions,
    BuiltinOptions_ReverseSequenceOptions,
    BuiltinOptions_MatrixDiagOptions,
    BuiltinOptions_QuantizeOptions,
    BuiltinOptions_MatrixSetDiagOptions,
    BuiltinOptions_HardSwishOptions,
    BuiltinOptions_IfOptions,
    BuiltinOptions_WhileOptions,
    BuiltinOptions_DepthToSpaceOptions,
    BuiltinOptions_NonMaxSuppressionV4Options,
    BuiltinOptions_NonMaxSuppressionV5Options,
    BuiltinOptions_ScatterNdOptions,
    BuiltinOptions_SelectV2Options,
    BuiltinOptions_DensifyOptions,
    BuiltinOptions_SegmentSumOptions,
    BuiltinOptions_BatchMatMulOptions,
    BuiltinOptions_CumsumOptions,
    BuiltinOptions_CallOnceOptions,
    BuiltinOptions_BroadcastToOptions,
    BuiltinOptions_Rfft2dOptions,
    BuiltinOptions_Conv3DOptions,
    BuiltinOptions_HashtableOptions,
    BuiltinOptions_HashtableFindOptions,
    BuiltinOptions_HashtableImportOptions,
    BuiltinOptions_HashtableSizeOptions,
    BuiltinOptions_VarHandleOptions,
    BuiltinOptions_ReadVariableOptions,
    BuiltinOptions_AssignVariableOptions,
    BuiltinOptions_RandomOptions,
    BuiltinOptions_BucketizeOptions,
    BuiltinOptions_GeluOptions,
    BuiltinOptions_DynamicUpdateSliceOptions,
    BuiltinOptions_UnsortedSegmentProdOptions,
    BuiltinOptions_UnsortedSegmentMaxOptions,
    BuiltinOptions_UnsortedSegmentMinOptions,
    BuiltinOptions_UnsortedSegmentSumOptions,
    BuiltinOptions_ATan2Options,
    BuiltinOptions_SignOptions
  };
  return values;
}

inline const char * const *EnumNamesBuiltinOptions() {
  static const char * const names[125] = {
    "NONE",
    "Conv2DOptions",
    "DepthwiseConv2DOptions",
    "ConcatEmbeddingsOptions",
    "LSHProjectionOptions",
    "Pool2DOptions",
    "SVDFOptions",
    "RNNOptions",
    "FullyConnectedOptions",
    "SoftmaxOptions",
    "ConcatenationOptions",
    "AddOptions",
    "L2NormOptions",
    "LocalResponseNormalizationOptions",
    "LSTMOptions",
    "ResizeBilinearOptions",
    "CallOptions",
    "ReshapeOptions",
    "SkipGramOptions",
    "SpaceToDepthOptions",
    "EmbeddingLookupSparseOptions",
    "MulOptions",
    "PadOptions",
    "GatherOptions",
    "BatchToSpaceNDOptions",
    "SpaceToBatchNDOptions",
    "TransposeOptions",
    "ReducerOptions",
    "SubOptions",
    "DivOptions",
    "SqueezeOptions",
    "SequenceRNNOptions",
    "StridedSliceOptions",
    "ExpOptions",
    "TopKV2Options",
    "SplitOptions",
    "LogSoftmaxOptions",
    "CastOptions",
    "DequantizeOptions",
    "MaximumMinimumOptions",
    "ArgMaxOptions",
    "LessOptions",
    "NegOptions",
    "PadV2Options",
    "GreaterOptions",
    "GreaterEqualOptions",
    "LessEqualOptions",
    "SelectOptions",
    "SliceOptions",
    "TransposeConvOptions",
    "SparseToDenseOptions",
    "TileOptions",
    "ExpandDimsOptions",
    "EqualOptions",
    "NotEqualOptions",
    "ShapeOptions",
    "PowOptions",
    "ArgMinOptions",
    "FakeQuantOptions",
    "PackOptions",
    "LogicalOrOptions",
    "OneHotOptions",
    "LogicalAndOptions",
    "LogicalNotOptions",
    "UnpackOptions",
    "FloorDivOptions",
    "SquareOptions",
    "ZerosLikeOptions",
    "FillOptions",
    "BidirectionalSequenceLSTMOptions",
    "BidirectionalSequenceRNNOptions",
    "UnidirectionalSequenceLSTMOptions",
    "FloorModOptions",
    "RangeOptions",
    "ResizeNearestNeighborOptions",
    "LeakyReluOptions",
    "SquaredDifferenceOptions",
    "MirrorPadOptions",
    "AbsOptions",
    "SplitVOptions",
    "UniqueOptions",
    "ReverseV2Options",
    "AddNOptions",
    "GatherNdOptions",
    "CosOptions",
    "WhereOptions",
    "RankOptions",
    "ReverseSequenceOptions",
    "MatrixDiagOptions",
    "QuantizeOptions",
    "MatrixSetDiagOptions",
    "HardSwishOptions",
    "IfOptions",
    "WhileOptions",
    "DepthToSpaceOptions",
    "NonMaxSuppressionV4Options",
    "NonMaxSuppressionV5Options",
    "ScatterNdOptions",
    "SelectV2Options",
    "DensifyOptions",
    "SegmentSumOptions",
    "BatchMatMulOptions",
    "CumsumOptions",
    "CallOnceOptions",
    "BroadcastToOptions",
    "Rfft2dOptions",
    "Conv3DOptions",
    "HashtableOptions",
    "HashtableFindOptions",
    "HashtableImportOptions",
    "HashtableSizeOptions",
    "VarHandleOptions",
    "ReadVariableOptions",
    "AssignVariableOptions",
    "RandomOptions",
    "BucketizeOptions",
    "GeluOptions",
    "DynamicUpdateSliceOptions",
    "UnsortedSegmentProdOptions",
    "UnsortedSegmentMaxOptions",
    "UnsortedSegmentMinOptions",
    "UnsortedSegmentSumOptions",
    "ATan2Options",
    "SignOptions",
    nullptr
  };
  return names;
}

inline const char *EnumNameBuiltinOptions(BuiltinOptions e) {
  if (flatbuffers::IsOutRange(e, BuiltinOptions_NONE, BuiltinOptions_SignOptions)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesBuiltinOptions()[index];
}

template<typename T> struct BuiltinOptionsTraits {
  static const BuiltinOptions enum_value = BuiltinOptions_NONE;
};

template<> struct BuiltinOptionsTraits<tflite::Conv2DOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_Conv2DOptions;
};

template<> struct BuiltinOptionsTraits<tflite::DepthwiseConv2DOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_DepthwiseConv2DOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ConcatEmbeddingsOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ConcatEmbeddingsOptions;
};

template<> struct BuiltinOptionsTraits<tflite::LSHProjectionOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_LSHProjectionOptions;
};

template<> struct BuiltinOptionsTraits<tflite::Pool2DOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_Pool2DOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SVDFOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SVDFOptions;
};

template<> struct BuiltinOptionsTraits<tflite::RNNOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_RNNOptions;
};

template<> struct BuiltinOptionsTraits<tflite::FullyConnectedOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_FullyConnectedOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SoftmaxOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SoftmaxOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ConcatenationOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ConcatenationOptions;
};

template<> struct BuiltinOptionsTraits<tflite::AddOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_AddOptions;
};

template<> struct BuiltinOptionsTraits<tflite::L2NormOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_L2NormOptions;
};

template<> struct BuiltinOptionsTraits<tflite::LocalResponseNormalizationOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_LocalResponseNormalizationOptions;
};

template<> struct BuiltinOptionsTraits<tflite::LSTMOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_LSTMOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ResizeBilinearOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ResizeBilinearOptions;
};

template<> struct BuiltinOptionsTraits<tflite::CallOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_CallOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ReshapeOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ReshapeOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SkipGramOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SkipGramOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SpaceToDepthOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SpaceToDepthOptions;
};

template<> struct BuiltinOptionsTraits<tflite::EmbeddingLookupSparseOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_EmbeddingLookupSparseOptions;
};

template<> struct BuiltinOptionsTraits<tflite::MulOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_MulOptions;
};

template<> struct BuiltinOptionsTraits<tflite::PadOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_PadOptions;
};

template<> struct BuiltinOptionsTraits<tflite::GatherOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_GatherOptions;
};

template<> struct BuiltinOptionsTraits<tflite::BatchToSpaceNDOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_BatchToSpaceNDOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SpaceToBatchNDOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SpaceToBatchNDOptions;
};

template<> struct BuiltinOptionsTraits<tflite::TransposeOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_TransposeOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ReducerOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ReducerOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SubOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SubOptions;
};

template<> struct BuiltinOptionsTraits<tflite::DivOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_DivOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SqueezeOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SqueezeOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SequenceRNNOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SequenceRNNOptions;
};

template<> struct BuiltinOptionsTraits<tflite::StridedSliceOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_StridedSliceOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ExpOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ExpOptions;
};

template<> struct BuiltinOptionsTraits<tflite::TopKV2Options> {
  static const BuiltinOptions enum_value = BuiltinOptions_TopKV2Options;
};

template<> struct BuiltinOptionsTraits<tflite::SplitOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SplitOptions;
};

template<> struct BuiltinOptionsTraits<tflite::LogSoftmaxOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_LogSoftmaxOptions;
};

template<> struct BuiltinOptionsTraits<tflite::CastOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_CastOptions;
};

template<> struct BuiltinOptionsTraits<tflite::DequantizeOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_DequantizeOptions;
};

template<> struct BuiltinOptionsTraits<tflite::MaximumMinimumOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_MaximumMinimumOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ArgMaxOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ArgMaxOptions;
};

template<> struct BuiltinOptionsTraits<tflite::LessOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_LessOptions;
};

template<> struct BuiltinOptionsTraits<tflite::NegOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_NegOptions;
};

template<> struct BuiltinOptionsTraits<tflite::PadV2Options> {
  static const BuiltinOptions enum_value = BuiltinOptions_PadV2Options;
};

template<> struct BuiltinOptionsTraits<tflite::GreaterOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_GreaterOptions;
};

template<> struct BuiltinOptionsTraits<tflite::GreaterEqualOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_GreaterEqualOptions;
};

template<> struct BuiltinOptionsTraits<tflite::LessEqualOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_LessEqualOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SelectOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SelectOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SliceOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SliceOptions;
};

template<> struct BuiltinOptionsTraits<tflite::TransposeConvOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_TransposeConvOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SparseToDenseOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SparseToDenseOptions;
};

template<> struct BuiltinOptionsTraits<tflite::TileOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_TileOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ExpandDimsOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ExpandDimsOptions;
};

template<> struct BuiltinOptionsTraits<tflite::EqualOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_EqualOptions;
};

template<> struct BuiltinOptionsTraits<tflite::NotEqualOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_NotEqualOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ShapeOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ShapeOptions;
};

template<> struct BuiltinOptionsTraits<tflite::PowOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_PowOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ArgMinOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ArgMinOptions;
};

template<> struct BuiltinOptionsTraits<tflite::FakeQuantOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_FakeQuantOptions;
};

template<> struct BuiltinOptionsTraits<tflite::PackOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_PackOptions;
};

template<> struct BuiltinOptionsTraits<tflite::LogicalOrOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_LogicalOrOptions;
};

template<> struct BuiltinOptionsTraits<tflite::OneHotOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_OneHotOptions;
};

template<> struct BuiltinOptionsTraits<tflite::LogicalAndOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_LogicalAndOptions;
};

template<> struct BuiltinOptionsTraits<tflite::LogicalNotOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_LogicalNotOptions;
};

template<> struct BuiltinOptionsTraits<tflite::UnpackOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_UnpackOptions;
};

template<> struct BuiltinOptionsTraits<tflite::FloorDivOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_FloorDivOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SquareOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SquareOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ZerosLikeOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ZerosLikeOptions;
};

template<> struct BuiltinOptionsTraits<tflite::FillOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_FillOptions;
};

template<> struct BuiltinOptionsTraits<tflite::BidirectionalSequenceLSTMOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_BidirectionalSequenceLSTMOptions;
};

template<> struct BuiltinOptionsTraits<tflite::BidirectionalSequenceRNNOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_BidirectionalSequenceRNNOptions;
};

template<> struct BuiltinOptionsTraits<tflite::UnidirectionalSequenceLSTMOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_UnidirectionalSequenceLSTMOptions;
};

template<> struct BuiltinOptionsTraits<tflite::FloorModOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_FloorModOptions;
};

template<> struct BuiltinOptionsTraits<tflite::RangeOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_RangeOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ResizeNearestNeighborOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ResizeNearestNeighborOptions;
};

template<> struct BuiltinOptionsTraits<tflite::LeakyReluOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_LeakyReluOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SquaredDifferenceOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SquaredDifferenceOptions;
};

template<> struct BuiltinOptionsTraits<tflite::MirrorPadOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_MirrorPadOptions;
};

template<> struct BuiltinOptionsTraits<tflite::AbsOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_AbsOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SplitVOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SplitVOptions;
};

template<> struct BuiltinOptionsTraits<tflite::UniqueOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_UniqueOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ReverseV2Options> {
  static const BuiltinOptions enum_value = BuiltinOptions_ReverseV2Options;
};

template<> struct BuiltinOptionsTraits<tflite::AddNOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_AddNOptions;
};

template<> struct BuiltinOptionsTraits<tflite::GatherNdOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_GatherNdOptions;
};

template<> struct BuiltinOptionsTraits<tflite::CosOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_CosOptions;
};

template<> struct BuiltinOptionsTraits<tflite::WhereOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_WhereOptions;
};

template<> struct BuiltinOptionsTraits<tflite::RankOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_RankOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ReverseSequenceOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ReverseSequenceOptions;
};

template<> struct BuiltinOptionsTraits<tflite::MatrixDiagOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_MatrixDiagOptions;
};

template<> struct BuiltinOptionsTraits<tflite::QuantizeOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_QuantizeOptions;
};

template<> struct BuiltinOptionsTraits<tflite::MatrixSetDiagOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_MatrixSetDiagOptions;
};

template<> struct BuiltinOptionsTraits<tflite::HardSwishOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_HardSwishOptions;
};

template<> struct BuiltinOptionsTraits<tflite::IfOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_IfOptions;
};

template<> struct BuiltinOptionsTraits<tflite::WhileOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_WhileOptions;
};

template<> struct BuiltinOptionsTraits<tflite::DepthToSpaceOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_DepthToSpaceOptions;
};

template<> struct BuiltinOptionsTraits<tflite::NonMaxSuppressionV4Options> {
  static const BuiltinOptions enum_value = BuiltinOptions_NonMaxSuppressionV4Options;
};

template<> struct BuiltinOptionsTraits<tflite::NonMaxSuppressionV5Options> {
  static const BuiltinOptions enum_value = BuiltinOptions_NonMaxSuppressionV5Options;
};

template<> struct BuiltinOptionsTraits<tflite::ScatterNdOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ScatterNdOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SelectV2Options> {
  static const BuiltinOptions enum_value = BuiltinOptions_SelectV2Options;
};

template<> struct BuiltinOptionsTraits<tflite::DensifyOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_DensifyOptions;
};

template<> struct BuiltinOptionsTraits<tflite::SegmentSumOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SegmentSumOptions;
};

template<> struct BuiltinOptionsTraits<tflite::BatchMatMulOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_BatchMatMulOptions;
};

template<> struct BuiltinOptionsTraits<tflite::CumsumOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_CumsumOptions;
};

template<> struct BuiltinOptionsTraits<tflite::CallOnceOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_CallOnceOptions;
};

template<> struct BuiltinOptionsTraits<tflite::BroadcastToOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_BroadcastToOptions;
};

template<> struct BuiltinOptionsTraits<tflite::Rfft2dOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_Rfft2dOptions;
};

template<> struct BuiltinOptionsTraits<tflite::Conv3DOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_Conv3DOptions;
};

template<> struct BuiltinOptionsTraits<tflite::HashtableOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_HashtableOptions;
};

template<> struct BuiltinOptionsTraits<tflite::HashtableFindOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_HashtableFindOptions;
};

template<> struct BuiltinOptionsTraits<tflite::HashtableImportOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_HashtableImportOptions;
};

template<> struct BuiltinOptionsTraits<tflite::HashtableSizeOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_HashtableSizeOptions;
};

template<> struct BuiltinOptionsTraits<tflite::VarHandleOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_VarHandleOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ReadVariableOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_ReadVariableOptions;
};

template<> struct BuiltinOptionsTraits<tflite::AssignVariableOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_AssignVariableOptions;
};

template<> struct BuiltinOptionsTraits<tflite::RandomOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_RandomOptions;
};

template<> struct BuiltinOptionsTraits<tflite::BucketizeOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_BucketizeOptions;
};

template<> struct BuiltinOptionsTraits<tflite::GeluOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_GeluOptions;
};

template<> struct BuiltinOptionsTraits<tflite::DynamicUpdateSliceOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_DynamicUpdateSliceOptions;
};

template<> struct BuiltinOptionsTraits<tflite::UnsortedSegmentProdOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_UnsortedSegmentProdOptions;
};

template<> struct BuiltinOptionsTraits<tflite::UnsortedSegmentMaxOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_UnsortedSegmentMaxOptions;
};

template<> struct BuiltinOptionsTraits<tflite::UnsortedSegmentMinOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_UnsortedSegmentMinOptions;
};

template<> struct BuiltinOptionsTraits<tflite::UnsortedSegmentSumOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_UnsortedSegmentSumOptions;
};

template<> struct BuiltinOptionsTraits<tflite::ATan2Options> {
  static const BuiltinOptions enum_value = BuiltinOptions_ATan2Options;
};

template<> struct BuiltinOptionsTraits<tflite::SignOptions> {
  static const BuiltinOptions enum_value = BuiltinOptions_SignOptions;
};

template<typename T> struct BuiltinOptionsUnionTraits {
  static const BuiltinOptions enum_value = BuiltinOptions_NONE;
};

template<> struct BuiltinOptionsUnionTraits<tflite::Conv2DOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_Conv2DOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::DepthwiseConv2DOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_DepthwiseConv2DOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ConcatEmbeddingsOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ConcatEmbeddingsOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::LSHProjectionOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_LSHProjectionOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::Pool2DOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_Pool2DOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SVDFOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SVDFOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::RNNOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_RNNOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::FullyConnectedOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_FullyConnectedOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SoftmaxOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SoftmaxOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ConcatenationOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ConcatenationOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::AddOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_AddOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::L2NormOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_L2NormOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::LocalResponseNormalizationOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_LocalResponseNormalizationOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::LSTMOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_LSTMOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ResizeBilinearOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ResizeBilinearOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::CallOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_CallOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ReshapeOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ReshapeOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SkipGramOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SkipGramOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SpaceToDepthOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SpaceToDepthOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::EmbeddingLookupSparseOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_EmbeddingLookupSparseOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::MulOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_MulOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::PadOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_PadOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::GatherOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_GatherOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::BatchToSpaceNDOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_BatchToSpaceNDOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SpaceToBatchNDOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SpaceToBatchNDOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::TransposeOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_TransposeOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ReducerOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ReducerOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SubOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SubOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::DivOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_DivOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SqueezeOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SqueezeOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SequenceRNNOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SequenceRNNOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::StridedSliceOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_StridedSliceOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ExpOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ExpOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::TopKV2OptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_TopKV2Options;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SplitOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SplitOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::LogSoftmaxOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_LogSoftmaxOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::CastOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_CastOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::DequantizeOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_DequantizeOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::MaximumMinimumOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_MaximumMinimumOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ArgMaxOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ArgMaxOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::LessOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_LessOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::NegOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_NegOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::PadV2OptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_PadV2Options;
};

template<> struct BuiltinOptionsUnionTraits<tflite::GreaterOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_GreaterOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::GreaterEqualOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_GreaterEqualOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::LessEqualOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_LessEqualOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SelectOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SelectOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SliceOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SliceOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::TransposeConvOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_TransposeConvOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SparseToDenseOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SparseToDenseOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::TileOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_TileOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ExpandDimsOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ExpandDimsOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::EqualOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_EqualOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::NotEqualOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_NotEqualOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ShapeOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ShapeOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::PowOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_PowOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ArgMinOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ArgMinOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::FakeQuantOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_FakeQuantOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::PackOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_PackOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::LogicalOrOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_LogicalOrOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::OneHotOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_OneHotOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::LogicalAndOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_LogicalAndOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::LogicalNotOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_LogicalNotOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::UnpackOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_UnpackOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::FloorDivOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_FloorDivOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SquareOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SquareOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ZerosLikeOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ZerosLikeOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::FillOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_FillOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::BidirectionalSequenceLSTMOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_BidirectionalSequenceLSTMOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::BidirectionalSequenceRNNOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_BidirectionalSequenceRNNOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::UnidirectionalSequenceLSTMOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_UnidirectionalSequenceLSTMOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::FloorModOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_FloorModOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::RangeOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_RangeOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ResizeNearestNeighborOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ResizeNearestNeighborOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::LeakyReluOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_LeakyReluOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SquaredDifferenceOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SquaredDifferenceOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::MirrorPadOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_MirrorPadOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::AbsOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_AbsOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SplitVOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SplitVOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::UniqueOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_UniqueOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ReverseV2OptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ReverseV2Options;
};

template<> struct BuiltinOptionsUnionTraits<tflite::AddNOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_AddNOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::GatherNdOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_GatherNdOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::CosOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_CosOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::WhereOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_WhereOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::RankOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_RankOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ReverseSequenceOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ReverseSequenceOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::MatrixDiagOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_MatrixDiagOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::QuantizeOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_QuantizeOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::MatrixSetDiagOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_MatrixSetDiagOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::HardSwishOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_HardSwishOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::IfOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_IfOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::WhileOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_WhileOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::DepthToSpaceOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_DepthToSpaceOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::NonMaxSuppressionV4OptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_NonMaxSuppressionV4Options;
};

template<> struct BuiltinOptionsUnionTraits<tflite::NonMaxSuppressionV5OptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_NonMaxSuppressionV5Options;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ScatterNdOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ScatterNdOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SelectV2OptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SelectV2Options;
};

template<> struct BuiltinOptionsUnionTraits<tflite::DensifyOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_DensifyOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SegmentSumOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SegmentSumOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::BatchMatMulOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_BatchMatMulOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::CumsumOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_CumsumOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::CallOnceOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_CallOnceOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::BroadcastToOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_BroadcastToOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::Rfft2dOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_Rfft2dOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::Conv3DOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_Conv3DOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::HashtableOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_HashtableOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::HashtableFindOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_HashtableFindOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::HashtableImportOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_HashtableImportOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::HashtableSizeOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_HashtableSizeOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::VarHandleOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_VarHandleOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ReadVariableOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ReadVariableOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::AssignVariableOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_AssignVariableOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::RandomOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_RandomOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::BucketizeOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_BucketizeOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::GeluOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_GeluOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::DynamicUpdateSliceOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_DynamicUpdateSliceOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::UnsortedSegmentProdOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_UnsortedSegmentProdOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::UnsortedSegmentMaxOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_UnsortedSegmentMaxOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::UnsortedSegmentMinOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_UnsortedSegmentMinOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::UnsortedSegmentSumOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_UnsortedSegmentSumOptions;
};

template<> struct BuiltinOptionsUnionTraits<tflite::ATan2OptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_ATan2Options;
};

template<> struct BuiltinOptionsUnionTraits<tflite::SignOptionsT> {
  static const BuiltinOptions enum_value = BuiltinOptions_SignOptions;
};

struct OperatorCodeT : public flatbuffers::NativeTable {
  typedef OperatorCode TableType;
  int8_t deprecated_builtin_code = 0;
  std::string custom_code{};
  int32_t version = 1;
  tflite::BuiltinOperator builtin_code = tflite::BuiltinOperator_ADD;
};

struct OperatorCode FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef OperatorCodeT NativeTableType;
  typedef OperatorCodeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_DEPRECATED_BUILTIN_CODE = 4,
    VT_CUSTOM_CODE = 6,
    VT_VERSION = 8,
    VT_BUILTIN_CODE = 10
  };
  int8_t deprecated_builtin_code() const {
    return GetField<int8_t>(VT_DEPRECATED_BUILTIN_CODE, 0);
  }
  const flatbuffers::String *custom_code() const {
    return GetPointer<const flatbuffers::String *>(VT_CUSTOM_CODE);
  }
  int32_t version() const {
    return GetField<int32_t>(VT_VERSION, 1);
  }
  tflite::BuiltinOperator builtin_code() const {
    return static_cast<tflite::BuiltinOperator>(GetField<int32_t>(VT_BUILTIN_CODE, 0));
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, VT_DEPRECATED_BUILTIN_CODE, 1) &&
           VerifyOffset(verifier, VT_CUSTOM_CODE) &&
           verifier.VerifyString(custom_code()) &&
           VerifyField<int32_t>(verifier, VT_VERSION, 4) &&
           VerifyField<int32_t>(verifier, VT_BUILTIN_CODE, 4) &&
           verifier.EndTable();
  }
  OperatorCodeT *UnPack(const flatbuffers::resolver_function_t *_resolver = nullptr) const;
  void UnPackTo(OperatorCodeT *_o, const flatbuffers::resolver_function_t *_resolver = nullptr) const;
  static flatbuffers::Offset<OperatorCode> Pack(flatbuffers::FlatBufferBuilder &_fbb, const OperatorCodeT* _o, const flatbuffers::rehasher_function_t *_rehasher = nullptr);
};

}
#endif  // FLATBUFFERS_GENERATED_SCHEMA_SUPPL_TFLITE_H_