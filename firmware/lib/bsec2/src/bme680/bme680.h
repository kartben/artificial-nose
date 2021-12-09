/**
 * @copyright
 * Copyright (C) 2017 - 2018 Bosch Sensortec GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the copyright holder nor the names of the
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 * The information provided is believed to be accurate and reliable.
 * The copyright holder assumes no responsibility
 * for the consequences of use
 * of such information nor for any infringement of patents or
 * other rights of third parties which may result from its use.
 * No license is granted by implication or otherwise under any patent or
 * patent rights of the copyright holder.
 *
 * @file	bme680.h
 * @date	7 Nov 2018
 * @version	4.1.0
 * @brief	Header file for the BME680 Sensor API
 */

#ifndef BME680_H_
#define BME680_H_

#include "bme680_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * @brief This API initializes the sensor
 *
 * @param[in,out] dev : Structure instance of bme680_dev
 *
 * @return Result of API execution status. Refer to @ref return_val
 * @retval = BME680_OK : Success
 * @retval < BME680_OK : Error
 * @retval > BME680_OK : Warning
 */
int8_t bme680_init(struct bme680_dev *dev);

/*!
 * @brief This API writes the given data to the registers of the sensor
 *
 * @param[in] reg_addr : Register addresses to where the data is to be written
 * @param[in] reg_data : Pointer to data buffer which is to be written
 * in the sensor.
 * @param[in] len      : No of bytes of data to write
 * @param[in,out] dev  : Structure instance of bme680_dev
 *
 * @return Result of API execution status. Refer to @ref return_val
 * @retval = BME680_OK : Success
 * @retval < BME680_OK : Error
 * @retval > BME680_OK : Warning
 */
int8_t bme680_set_regs(const uint8_t *reg_addr, const uint8_t *reg_data, uint8_t len, struct bme680_dev *dev);

/*!
 * @brief This API reads the data from the registers of the sensor.
 *
 * @param[in] reg_addr  : Register address from where the data to be read
 * @param[out] reg_data : Pointer to data buffer to store the read data.
 * @param[in] len       : No of bytes of data to be read.
 * @param[in,out] dev   : Structure instance of bme680_dev.
 *
 * @return Result of API execution status. Refer to @ref return_val.
 * @retval = BME680_OK : Success
 * @retval < BME680_OK : Error
 * @retval > BME680_OK : Warning
 */
int8_t bme680_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint16_t len, struct bme680_dev *dev);

/*!
 * @brief This API performs the soft reset of the sensor.
 *
 * @param[in,out] dev : Structure instance of bme680_dev.
 *
 * @return Result of API execution status. Refer to @ref return_val
 * @retval = BME680_OK : Success
 * @retval < BME680_OK : Error
 * @retval > BME680_OK : Warning
 */
int8_t bme680_soft_reset(struct bme680_dev *dev);

/*!
 * @brief This API is used to set the operating mode of the sensor.
 *
 * @param[in] op_mode : Desired operating mode.
 * @param[in] dev     : Structure instance of bme680_dev
 *
 * @return Result of API execution status. Refer to @ref return_val
 * @retval = BME680_OK : Success
 * @retval < BME680_OK : Error
 * @retval > BME680_OK : Warning
 */
int8_t bme680_set_op_mode(const uint8_t op_mode, struct bme680_dev *dev);

/*!
 * @brief This API is used to get the operating mode of the sensor.
 *
 * @param[out] op_mode : Desired operating mode.
 * @param[in,out] dev : Structure instance of bme680_dev
 *
 * @return Result of API execution status. Refer to @ref return_val
 * @retval = BME680_OK : Success
 * @retval < BME680_OK : Error
 * @retval > BME680_OK : Warning
 */
int8_t bme680_get_op_mode(uint8_t *op_mode, struct bme680_dev *dev);

/*!
 * @brief This API is used to get the remaining duration that can be used for heating.
 *
 * @param[in] op_mode  : Expected operating mode.
 * @param[in] conf     : Structure holding the tph settings.
 *
 * @return Duration taken for measurements in ms.
 */
uint16_t bme680_get_meas_dur(const uint8_t op_mode, const struct bme680_conf *conf);

/*!
 * @brief This API reads the pressure, temperature and humidity and gas data
 * from the sensor, compensates the data and store it in the bme680_data
 * structure instance passed by the user.
 *
 * @param[in]  op_mode : Expected operating operating mode.
 * @param[out] data    : Structure instance to hold the data.
 * @param[out] n_data  : Number of data instances available.
 * @param[in,out] dev  : Structure instance of bme680_dev
 *
 * @return Result of API execution status. Refer to @ref return_val
 * @retval = BME680_OK : Success
 * @retval < BME680_OK : Error
 * @retval > BME680_OK : Warning
 */
int8_t bme680_get_data(uint8_t op_mode, struct bme680_data *data, uint8_t *n_data, struct bme680_dev *dev);

/*!
 * @brief This API is used to set the oversampling, filter and odr
 * configuration
 *
 * @param[in] conf    : Desired sensor configuration.
 * @param[in,out] dev : Structure instance of bme680_dev.
 *
 * @return Result of API execution status. Refer to @ref return_val
 * @retval = BME680_OK : Success
 * @retval < BME680_OK : Error
 * @retval > BME680_OK : Warning
 */
int8_t bme680_set_conf(struct bme680_conf *conf, struct bme680_dev *dev);

/*!
 * @brief This API is used to get the oversampling, filter and odr
 * configuration
 *
 * @param[out] conf   : Present sensor configuration.
 * @param[in,out] dev : Structure instance of bme680_dev.
 *
 * @return Result of API execution status. Refer to @ref return_val
 * @retval = BME680_OK : Success
 * @retval < BME680_OK : Error
 * @retval > BME680_OK : Warning
 */
int8_t bme680_get_conf(struct bme680_conf *conf, struct bme680_dev *dev);

/*!
 * @brief This API is used to set the gas configuration of the sensor.
 *
 * @param[in] op_mode : Expected operation mode of the sensor.
 * @param[in] conf    : Desired heating configuration.
 * @param[in,out] dev : Structure instance of bme680_dev.
 *
 * @return Result of API execution status. Refer to @ref return_val.
 * @retval = BME680_OK : Success
 * @retval < BME680_OK : Error
 * @retval > BME680_OK : Warning
 */
int8_t bme680_set_heatr_conf(uint8_t op_mode, const struct bme680_heatr_conf *conf, struct bme680_dev *dev);

/*!
 * @brief This API is used to get the gas configuration of the sensor.
 *
 * @param[out] conf   : Current configurations of the gas sensor.
 * @param[in,out] dev : Structure instance of bme680_dev.
 *
 * @return Result of API execution status. Refer to @ref return_val.
 * @retval = BME680_OK : Success
 * @retval < BME680_OK : Error
 * @retval > BME680_OK : Warning
 */
int8_t bme680_get_heatr_conf(const struct bme680_heatr_conf *conf, struct bme680_dev *dev);

#ifdef __cplusplus
}
#endif
#endif /* BME680_H_ */
