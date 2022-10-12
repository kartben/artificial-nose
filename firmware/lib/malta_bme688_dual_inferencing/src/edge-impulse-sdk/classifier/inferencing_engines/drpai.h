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

#ifndef _EI_CLASSIFIER_INFERENCING_ENGINE_DRPAI_H_
#define _EI_CLASSIFIER_INFERENCING_ENGINE_DRPAI_H_

#if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_DRPAI)

/*****************************************
 * includes
 ******************************************/
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <fstream>
#include <iomanip>
#include <map>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include <linux/drpai.h>

#include <tflite-model/drpai_model.h>

#include "edge-impulse-sdk/classifier/ei_run_dsp.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "edge-impulse-sdk/classifier/ei_fill_result_struct.h"
#include "edge-impulse-sdk/classifier/ei_model_types.h"
#include "edge-impulse-sdk/porting/ei_logging.h"

#include "model-parameters/dsp_blocks.h"
#include <model-parameters/model_metadata.h>

/*****************************************
 * Macro
 ******************************************/
/*Maximum DRP-AI Timeout threshold*/
#define DRPAI_TIMEOUT (5)

/*Buffer size for writing data to memory via DRP-AI Driver.*/
#define BUF_SIZE (1024)

/*Index to access drpai_file_path[]*/
#define INDEX_D (0)
#define INDEX_C (1)
#define INDEX_P (2)
#define INDEX_A (3)
#define INDEX_W (4)

/*****************************************
 * Public global vars
 ******************************************/
// input and output buffer pointers for memory mapped regions used by DRP-AI
uint8_t *drpai_input_buf = (uint8_t *)NULL;
float *drpai_output_buf = (float *)NULL;

/*****************************************
 * Typedef
 ******************************************/
/* For DRP-AI Address List */
typedef struct {
  unsigned long desc_aimac_addr;
  unsigned long desc_aimac_size;
  unsigned long desc_drp_addr;
  unsigned long desc_drp_size;
  unsigned long drp_param_addr;
  unsigned long drp_param_size;
  unsigned long data_in_addr;
  unsigned long data_in_size;
  unsigned long data_addr;
  unsigned long data_size;
  unsigned long work_addr;
  unsigned long work_size;
  unsigned long data_out_addr;
  unsigned long data_out_size;
  unsigned long drp_config_addr;
  unsigned long drp_config_size;
  unsigned long weight_addr;
  unsigned long weight_size;
} st_addr_t;

/*****************************************
 * static vars
 ******************************************/
static st_addr_t drpai_address;
static uint64_t udmabuf_address = 0;

static int drpai_fd = -1;

drpai_data_t proc[DRPAI_INDEX_NUM];

void get_udmabuf_memory_start_addr()
{   /* Obtain udmabuf memory area starting address */

    int8_t fd = 0;
    char addr[1024];
    int32_t read_ret = 0;
    errno = 0;

    fd = open("/sys/class/u-dma-buf/udmabuf0/phys_addr", O_RDONLY);
    if (0 > fd)
    {
        fprintf(stderr, "[ERROR] Failed to open udmabuf0/phys_addr : errno=%d\n", errno);
    }

    read_ret = read(fd, addr, 1024);
    if (0 > read_ret)
    {
        fprintf(stderr, "[ERROR] Failed to read udmabuf0/phys_addr : errno=%d\n", errno);
        close(fd);
    }

    sscanf(addr, "%lx", &udmabuf_address);
    close(fd);

    /* Filter the bit higher than 32 bit */
    udmabuf_address &=0xFFFFFFFF;
}

uint8_t drpai_init_mem(uint32_t input_frame_size) {
  int32_t i = 0;

  int udmabuf_fd0 = open("/dev/udmabuf0", O_RDWR);
  if (udmabuf_fd0 < 0) {
    return -1;
  }

  uint8_t *addr =
      (uint8_t *)mmap(NULL, input_frame_size,
                      PROT_READ | PROT_WRITE, MAP_SHARED, udmabuf_fd0, 0);

  /* Write once to allocate physical memory to u-dma-buf virtual space.
   * Note: Do not use memset() for this.
   *       Because it does not work as expected. */
  for (i = 0; i < input_frame_size; i++) {
    addr[i] = 0;
  }

  drpai_input_buf = addr;
  drpai_output_buf = (float *)ei_malloc(10000 * sizeof(float));

  get_udmabuf_memory_start_addr();
  if (0 == udmabuf_address) {
    return EI_IMPULSE_DRPAI_INIT_FAILED;
  }

  // ei_printf("INFO: udmabuf_addr: %p\n", udmabuf_address);

  return 0;
}

/*****************************************
 * Function Name : read_addrmap_txt
 * Description	: Loads address and size of DRP-AI Object files into struct
 *addr. Arguments		: addr_file = filename of addressmap file (from
 *DRP-AI Object files) Return value	: 0 if succeeded not 0 otherwise
 ******************************************/
static int8_t read_addrmap_txt() {
  // create a stream from the DRP-AI model data without copying
  std::istringstream ifs;
  ifs.rdbuf()->pubsetbuf((char *)ei_ei_addrmap_intm_txt, ei_ei_addrmap_intm_txt_len);

  std::string str;
  unsigned long l_addr;
  unsigned long l_size;
  std::string element, a, s;

  if (ifs.fail()) {
    return -1;
  }

  while (getline(ifs, str)) {
    std::istringstream iss(str);
    iss >> element >> a >> s;
    l_addr = strtol(a.c_str(), NULL, 16);
    l_size = strtol(s.c_str(), NULL, 16);

    if (element == "drp_config") {
      drpai_address.drp_config_addr = l_addr;
      drpai_address.drp_config_size = l_size;
    } else if (element == "desc_aimac") {
      drpai_address.desc_aimac_addr = l_addr;
      drpai_address.desc_aimac_size = l_size;
    } else if (element == "desc_drp") {
      drpai_address.desc_drp_addr = l_addr;
      drpai_address.desc_drp_size = l_size;
    } else if (element == "drp_param") {
      drpai_address.drp_param_addr = l_addr;
      drpai_address.drp_param_size = l_size;
    } else if (element == "weight") {
      drpai_address.weight_addr = l_addr;
      drpai_address.weight_size = l_size;
    } else if (element == "data_in") {
      drpai_address.data_in_addr = l_addr;
      drpai_address.data_in_size = l_size;
    } else if (element == "data") {
      drpai_address.data_addr = l_addr;
      drpai_address.data_size = l_size;
    } else if (element == "data_out") {
      drpai_address.data_out_addr = l_addr;
      drpai_address.data_out_size = l_size;
    } else if (element == "work") {
      drpai_address.work_addr = l_addr;
      drpai_address.work_size = l_size;
    }
  }

  return 0;
}

/*****************************************
 * Function Name : load_data_to_mem
 * Description	: Loads a binary blob DRP-AI Driver Memory
 * Arguments		: data_ptr = pointer to the bytes to write
 *				  drpai_fd = file descriptor of DRP-AI Driver
 *				  from = memory start address where the data is
 *written size = data size to be written Return value	: 0 if succeeded not 0
 *otherwise
 ******************************************/
static int8_t load_data_to_mem(unsigned char *data_ptr, int drpai_fd,
                               unsigned long from, unsigned long size) {
  drpai_data_t drpai_data;

  drpai_data.address = from;
  drpai_data.size = size;

  errno = 0;
  if (-1 == ioctl(drpai_fd, DRPAI_ASSIGN, &drpai_data)) {
    return -1;
  }

  errno = 0;
  if (-1 == write(drpai_fd, data_ptr, size)) {
    return -1;
  }

  return 0;
}

/*****************************************
 * Function Name :  load_drpai_data
 * Description	: Loads DRP-AI Object files to memory via DRP-AI Driver.
 * Arguments		: drpai_fd = file descriptor of DRP-AI Driver
 * Return value	: 0 if succeeded
 *				: not 0 otherwise
 ******************************************/
static int load_drpai_data(int drpai_fd) {
  unsigned long addr, size;
  unsigned char *data_ptr;
  for (int i = 0; i < 5; i++) {
    switch (i) {
    case (INDEX_W):
      addr = drpai_address.weight_addr;
      size = drpai_address.weight_size;
      data_ptr = ei_ei_weight_dat;
      break;
    case (INDEX_C):
      addr = drpai_address.drp_config_addr;
      size = drpai_address.drp_config_size;
      data_ptr = ei_ei_drpcfg_mem;
      break;
    case (INDEX_P):
      addr = drpai_address.drp_param_addr;
      size = drpai_address.drp_param_size;
      data_ptr = ei_drp_param_bin;
      break;
    case (INDEX_A):
      addr = drpai_address.desc_aimac_addr;
      size = drpai_address.desc_aimac_size;
      data_ptr = ei_aimac_desc_bin;
      break;
    case (INDEX_D):
      addr = drpai_address.desc_drp_addr;
      size = drpai_address.desc_drp_size;
      data_ptr = ei_drp_desc_bin;
      break;
    default:
      return -1;
      break;
    }
    if (0 != load_data_to_mem(data_ptr, drpai_fd, addr, size)) {
      return -1;
    }
  }
  return 0;
}

EI_IMPULSE_ERROR drpai_init_classifier() {
  // retval for drpai status
  int ret_drpai;

  // Read DRP-AI Object files address and size
  if (0 != read_addrmap_txt()) {
    ei_printf("ERR: read_addrmap_txt failed : %d\n", errno);
    return EI_IMPULSE_DRPAI_INIT_FAILED;
  }

  // DRP-AI Driver Open
  drpai_fd = open("/dev/drpai0", O_RDWR);
  if (drpai_fd < 0) {
    ei_printf("ERR: Failed to Open DRP-AI Driver: errno=%d\n", errno);
    return EI_IMPULSE_DRPAI_INIT_FAILED;
  }

  // Load DRP-AI Data from Filesystem to Memory via DRP-AI Driver
  ret_drpai = load_drpai_data(drpai_fd);
  if (ret_drpai != 0) {
    ei_printf("ERR: Failed to load DRPAI Data\n");
    if (0 != close(drpai_fd)) {
      ei_printf("ERR: Failed to Close DRPAI Driver: errno=%d\n", errno);
    }
    return EI_IMPULSE_DRPAI_INIT_FAILED;
  }

  // statically store DRP object file addresses and sizes
  proc[DRPAI_INDEX_INPUT].address = (uint32_t)udmabuf_address;
  proc[DRPAI_INDEX_INPUT].size = drpai_address.data_in_size;
  proc[DRPAI_INDEX_DRP_CFG].address = drpai_address.drp_config_addr;
  proc[DRPAI_INDEX_DRP_CFG].size = drpai_address.drp_config_size;
  proc[DRPAI_INDEX_DRP_PARAM].address = drpai_address.drp_param_addr;
  proc[DRPAI_INDEX_DRP_PARAM].size = drpai_address.drp_param_size;
  proc[DRPAI_INDEX_AIMAC_DESC].address = drpai_address.desc_aimac_addr;
  proc[DRPAI_INDEX_AIMAC_DESC].size = drpai_address.desc_aimac_size;
  proc[DRPAI_INDEX_DRP_DESC].address = drpai_address.desc_drp_addr;
  proc[DRPAI_INDEX_DRP_DESC].size = drpai_address.desc_drp_size;
  proc[DRPAI_INDEX_WEIGHT].address = drpai_address.weight_addr;
  proc[DRPAI_INDEX_WEIGHT].size = drpai_address.weight_size;
  proc[DRPAI_INDEX_OUTPUT].address = drpai_address.data_out_addr;
  proc[DRPAI_INDEX_OUTPUT].size = drpai_address.data_out_size;

  return EI_IMPULSE_OK;
}

EI_IMPULSE_ERROR drpai_run_classifier_image_quantized() {
#if EI_CLASSIFIER_COMPILED == 1
#error "DRP-AI is not compatible with EON Compiler"
#endif
  // output data from DRPAI model
  drpai_data_t drpai_data;
  // status used to query if any internal errors occured during inferencing
  drpai_status_t drpai_status;
  // descriptor used for checking if DRPAI is done inferencing
  fd_set rfds;
  // struct used to define DRPAI timeout
  struct timespec tv;
  // retval for drpai status
  int ret_drpai;
  // retval when querying drpai status
  int inf_status = 0;

  // DRP-AI Output Memory Preparation
  drpai_data.address = drpai_address.data_out_addr;
  drpai_data.size = drpai_address.data_out_size;

  // Start DRP-AI driver
  int ioret = ioctl(drpai_fd, DRPAI_START, &proc[0]);
  if (0 != ioret) {
    EI_LOGE("Failed to Start DRPAI Inference: %d\n", errno);
    return EI_IMPULSE_DRPAI_RUNTIME_FAILED;
  }

  // Settings For pselect - this is how DRPAI signals inferencing complete
  FD_ZERO(&rfds);
  FD_SET(drpai_fd, &rfds);
  // Define a timeout for DRP-AI to complete
  tv.tv_sec = DRPAI_TIMEOUT;
  tv.tv_nsec = 0;

  // Wait until DRP-AI ends
  ret_drpai = pselect(drpai_fd + 1, &rfds, NULL, NULL, &tv, NULL);
  if (ret_drpai == 0) {
      EI_LOGE("DRPAI Inference pselect() Timeout: %d\n", errno);
    return EI_IMPULSE_DRPAI_RUNTIME_FAILED;
  } else if (ret_drpai < 0) {
      EI_LOGE("DRPAI Inference pselect() Error: %d\n", errno);
    return EI_IMPULSE_DRPAI_RUNTIME_FAILED;
  }

  // Checks for DRPAI inference status errors
  inf_status = ioctl(drpai_fd, DRPAI_GET_STATUS, &drpai_status);
  if (inf_status != 0) {
      EI_LOGE("DRPAI Internal Error: %d\n", errno);
    return EI_IMPULSE_DRPAI_RUNTIME_FAILED;
  }

  if (ioctl(drpai_fd, DRPAI_ASSIGN, &drpai_data) != 0) {
      EI_LOGE("Failed to Assign DRPAI data: %d\n", errno);
    return EI_IMPULSE_DRPAI_RUNTIME_FAILED;
  }

  if (read(drpai_fd, drpai_output_buf, drpai_data.size) < 0) {
      EI_LOGE("Failed to read DRPAI output data: %d\n", errno);
    return EI_IMPULSE_DRPAI_RUNTIME_FAILED;
  }
  return EI_IMPULSE_OK;
}

// close the driver (reset file handles)
EI_IMPULSE_ERROR drpai_close(uint32_t input_frame_size) {
  munmap(drpai_input_buf, input_frame_size);
  if (drpai_fd > 0) {
    if (0 != close(drpai_fd)) {
        EI_LOGE("Failed to Close DRP-AI Driver: errno=%d\n", errno);
      return EI_IMPULSE_DRPAI_RUNTIME_FAILED;
    }
    drpai_fd = -1;
  }
  return EI_IMPULSE_OK;
}

/**
 * Special function to run the classifier on images, only works on TFLite models (either interpreter or EON or for tensaiflow)
 * that allocates a lot less memory by quantizing in place. This only works if 'can_run_classifier_image_quantized'
 * returns EI_IMPULSE_OK.
 */
EI_IMPULSE_ERROR run_nn_inference_image_quantized(
    const ei_impulse_t *impulse,
    signal_t *signal,
    ei_impulse_result_t *result,
    bool debug = false)
{
    static bool first_run = true;
    uint64_t ctx_start_us;
    uint64_t dsp_start_us = ei_read_timer_us();

    if (first_run) {
        // map memory regions to the DRP-AI UDMA. This is required for passing data
        // to and from DRP-AI
        int t = drpai_init_mem(impulse->nn_input_frame_size);
        if (t != 0) {
            return EI_IMPULSE_DRPAI_INIT_FAILED;
        }

        EI_IMPULSE_ERROR ret = drpai_init_classifier();
        if (ret != EI_IMPULSE_OK) {
            drpai_close(impulse->nn_input_frame_size);
            return EI_IMPULSE_DRPAI_INIT_FAILED;
        }
    }

    EI_LOGD("Starting DSP...\n");

    // Creates a features matrix mapped to the DRP-AI UDMA input region
    ei::matrix_i8_t features_matrix(1, proc[DRPAI_INDEX_INPUT].size, (int8_t *)drpai_input_buf);

    // Grabs the raw image buffer from the signal, DRP-AI will automatically
    // extract features
    int ret = extract_drpai_features_quantized(
        signal,
        &features_matrix,
        ei_dsp_blocks[0].config,
        EI_CLASSIFIER_FREQUENCY);
    if (ret != EIDSP_OK) {
        ei_printf("ERR: Failed to run DSP process (%d)\n", ret);
        return EI_IMPULSE_DSP_ERROR;
    }

    if (ei_run_impulse_check_canceled() == EI_IMPULSE_CANCELED) {
        return EI_IMPULSE_CANCELED;
    }

    result->timing.dsp_us = ei_read_timer_us() - dsp_start_us;
    result->timing.dsp = (int)(result->timing.dsp_us / 1000);

#if EI_LOG_LEVEL >= EI_LOG_LEVEL_DEBUG
        ei_printf("DSP was assigned address: 0x%lx\n", drpai_input_buf);
        ei_printf("Features (%d ms.): ", result->timing.dsp);
        for (size_t ix = 0; ix < EI_CLASSIFIER_NN_INPUT_FRAME_SIZE; ix++) {
            ei_printf("0x%hhx, ", drpai_input_buf[ix]);
        }
        ei_printf("\n");
#endif

    ctx_start_us = ei_read_timer_us();

    // Run DRP-AI inference, a static buffer is used to store the raw output
    // results
    ret = drpai_run_classifier_image_quantized();

    // close driver to reset memory, file pointer
    if (ret != EI_IMPULSE_OK) {
        drpai_close(impulse->nn_input_frame_size);
        first_run = true;
    }
    else {
        // drpai_reset();
        first_run = false;
    }

    if (impulse->object_detection) {
        switch (impulse->object_detection_last_layer) {
            case EI_CLASSIFIER_LAST_LAYER_FOMO: {
                #if EI_LOG_LEVEL >= EI_LOG_LEVEL_DEBUG
                    ei_printf("DEBUG: raw drpai output");
                    ei_printf("\n[");
                    for (uint32_t i = 0; i < 12 * 12 * 2; i++) {
                        ei_printf_float(drpai_output_buf[i]);
                        ei_printf(" ");
                    }
                    ei_printf("]\n");
                #endif

                fill_result_struct_f32_fomo(
                    impulse,
                    result,
                    drpai_output_buf,
                    impulse->input_width / 8,
                    impulse->input_height / 8);

                break;
            }
            case EI_CLASSIFIER_LAST_LAYER_SSD: {
                ei_printf("ERR: MobileNet SSD models are not implemented for DRP-AI (%d)\n",
                    impulse->object_detection_last_layer);
                return EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE;
            }
            case EI_CLASSIFIER_LAST_LAYER_YOLOV5: {
                #if EI_LOG_LEVEL >= EI_LOG_LEVEL_DEBUG
                    ei_printf("DEBUG: raw drpai output");
                    ei_printf("\n[");
                    for (uint32_t i = 0; i < 12 * 12 * 2; i++) {
                        ei_printf_float(drpai_output_buf[i]);
                        ei_printf(" ");
                    }
                    ei_printf("]\n");
                #endif

                #if EI_CLASSIFIER_TFLITE_OUTPUT_QUANTIZED == 1
                    ei_printf("ERR: YOLOv5 does not support quantized inference\n");
                    return EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE;
                #else
                    fill_result_struct_f32_yolov5(
                        impulse,
                        result,
                        drpai_output_buf,
                        impulse->tflite_output_features_count);
                #endif

                break;
            }
            default: {
                ei_printf("ERR: Unsupported object detection last layer (%d)\n",
                    impulse->object_detection_last_layer);
                return EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE;
            }
        }
    }
    else {
        fill_result_struct_f32(impulse, result, drpai_output_buf, debug);
    }

    result->timing.classification_us = ei_read_timer_us() - ctx_start_us;
    result->timing.classification = (int)(result->timing.classification_us / 1000);
    return EI_IMPULSE_OK;
}

#endif // #if (EI_CLASSIFIER_INFERENCING_ENGINE == EI_CLASSIFIER_DRPAI)
#endif // _EI_CLASSIFIER_INFERENCING_ENGINE_DRPAI_H_
