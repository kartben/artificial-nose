#ifndef _EIDSP_RETURN_TYPES_H_
#define _EIDSP_RETURN_TYPES_H_

#include <stdint.h>

/**
 * @defgroup ei_returntypes Return codes
 * 
 * Return codes for Edge Impulse functions.
 * 
 * **Source**: [dsp/returntypes.h](https://github.com/edgeimpulse/inferencing-sdk-cpp/blob/master/dsp/returntypes.h)
 * 
 * @addtogroup ei_returntypes
 * @{
 */

// outside of namespace for backwards compat
typedef enum {
    EI_IMPULSE_OK = 0, /**< Success */
    EI_IMPULSE_ERROR_SHAPES_DONT_MATCH = -1, /**< The shape of data does not match the shape of input layer. */
    EI_IMPULSE_CANCELED = -2, /**< Impulse execution is cancelled by user. */
    EI_IMPULSE_TFLITE_ERROR = -3, /**< Error in TesnorFlow Lite inference engine */
    EI_IMPULSE_DSP_ERROR = -5, /**< Error in processing portion of impulse */
    EI_IMPULSE_TFLITE_ARENA_ALLOC_FAILED = -6, /**< Failed to allocate memory in TensorFlow Lite arena, often caused by a lack of available heap memory. */
    EI_IMPULSE_CUBEAI_ERROR = -7, /**< Error in CubeAI inference engine (STM32) */
    EI_IMPULSE_ALLOC_FAILED = -8, /**< Memory allocation failed. Could be caused by a fragmented heap. Try to increase heap size. */
    EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES = -9, /**< This function is only supported for impulses with an image input. */
    EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE = -10, /**< The chosen inference engine (e.g. in Studio) is incapable of running this impulse. */
    EI_IMPULSE_OUT_OF_MEMORY = -11, /**< Out of memory. Could be caused by a fragmented heap. Try to increase heap size. */
    EI_IMPULSE_INPUT_TENSOR_WAS_NULL = -13, /**< Input tensor was null */
    EI_IMPULSE_OUTPUT_TENSOR_WAS_NULL = -14, /**< Output tensor was null */
    EI_IMPULSE_SCORE_TENSOR_WAS_NULL = -15, /**< Score tensor is null (for SSD Object Detection models). */
    EI_IMPULSE_LABEL_TENSOR_WAS_NULL = -16, /**< Label tensor is null (for SSD Object Detection models). */
    EI_IMPULSE_TENSORRT_INIT_FAILED = -17, /**< TensorRT (NVIDIA) initialization failed. */
    EI_IMPULSE_DRPAI_INIT_FAILED = -18, /**< DRP-AI (Renesas) initialization failed. */
    EI_IMPULSE_DRPAI_RUNTIME_FAILED = -19, /**< DRP-AI (Renesas) runtime failed. */
    EI_IMPULSE_DEPRECATED_MODEL = -20, /**< The model is deprecated and cannot be used. You should re-export the impulse from Studio. */
    EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE = -21, /**< The last layer is not available in the model. */
    EI_IMPULSE_INFERENCE_ERROR = -22, /**< Error during inference. */
    EI_IMPULSE_AKIDA_ERROR = -23, /**< Error in Akida inference engine (BrainChip) */
    EI_IMPULSE_INVALID_SIZE = -24, /**<The shape of data does not match the shape of input layer. */
    EI_IMPULSE_ONNX_ERROR = -25, /**< Error in ONNX inference engine */
    EI_IMPULSE_MEMRYX_ERROR = -26, /**< Error in Memryx inference engine */
} EI_IMPULSE_ERROR;

#endif // _EIDSP_RETURN_TYPES_H_

/** @} */