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
 * @file	bme680_defs.h
 * @date	7 Nov 2018
 * @version	4.1.0
 * @brief	Definitions header file for the BME680 Sensor API
 */

#ifndef BME680_DEFS_H_
#define BME680_DEFS_H_

/********************************************************/
/* header includes */
#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/kernel.h>
#else
#include <stdint.h>
#include <stddef.h>
#endif


/**\name C99 standard macros */

#if !defined(UINT8_C) && !defined(INT8_C)
#define INT8_C(x)       S8_C(x)
#define UINT8_C(x)      U8_C(x)
#endif

#if !defined(UINT16_C) && !defined(INT16_C)
#define INT16_C(x)      S16_C(x)
#define UINT16_C(x)     U16_C(x)
#endif

#if !defined(INT32_C) && !defined(UINT32_C)
#define INT32_C(x)      S32_C(x)
#define UINT32_C(x)     U32_C(x)
#endif

#if !defined(INT64_C) && !defined(UINT64_C)
#define INT64_C(x)      S64_C(x)
#define UINT64_C(x)     U64_C(x)
#endif


#ifndef NULL
#ifdef __cplusplus
#define NULL   0
#else
#define NULL   ((void *) 0)
#endif
#endif

/**@}*/

#ifndef BME680_DO_NOT_USE_FPU
/** Comment or un-comment the macro to provide floating point data output */
#define BME680_USE_FPU
#endif

/** BME680 unique chip identifier */
#define BME680_CHIP_ID  UINT8_C(0x61)

/** Period between two polls */
#define BME680_PERIOD_POLL	UINT32_C(5)
/** Period for a soft reset */
#define BME680_PERIOD_RESET	UINT32_C(10)

/** BME680 lower I2C address */
#define BME680_I2C_ADDR_LOW	UINT8_C(0x76)
/** BME680 higher I2C address */
#define BME680_I2C_ADDR_HIGH	UINT8_C(0x77)

/** Soft reset command */
#define BME680_SOFT_RESET_CMD   UINT8_C(0xb6)

/*! @addtogroup return_val Return code definitions
 * @{
 */
/** Success */
#define BME680_OK		INT8_C(0)
/* Errors */
/** Null pointer passed */
#define BME680_E_NULL_PTR	INT8_C(-1)
/** Communication failure */
#define BME680_E_COM_FAIL	INT8_C(-2)
/** Sensor not found */
#define BME680_E_DEV_NOT_FOUND	INT8_C(-3)
/** Incorrect length parameter */
#define BME680_E_INVALID_LENGTH	INT8_C(-4)
/* Warnings */
/** Define a valid operation mode */
#define BME680_W_DEFINE_OP_MODE		INT8_C(1)
/** No new data was found */
#define BME680_W_NO_NEW_DATA		INT8_C(2)
/** Define the shared heating duration */
#define BME680_W_DEFINE_SHD_HEATR_DUR	INT8_C(3)
/** Information - only available via bme680_dev.info_msg */
#define BME680_I_PARAM_CORR		UINT8_C(1)
/*! @} */

/*! @addtogroup reg_map Register map addresses in I2C
 * @{
 */
/** Register for 3rd group of coefficients */
#define BME680_REG_COEFF3		UINT8_C(0x00)
/** 0th Field address*/
#define BME680_REG_FIELD0		UINT8_C(0x1d)
/** 0th Current DAC address*/
#define BME680_REG_IDAC_HEAT0		UINT8_C(0x50)
/** 0th Res heat address */
#define BME680_REG_RES_HEAT0		UINT8_C(0x5a)
/** 0th Gas wait address */
#define BME680_REG_GAS_WAIT0		UINT8_C(0x64)
/** Shared heating duration address */
#define BME680_REG_SHD_HEATR_DUR	UINT8_C(0x6E)
/** CTRL_GAS_0 address */
#define BME680_REG_CTRL_GAS_0		UINT8_C(0x70)
/** CTRL_GAS_1 address */
#define BME680_REG_CTRL_GAS_1		UINT8_C(0x71)
/** CTRL_HUM address */
#define BME680_REG_CTRL_HUM		UINT8_C(0x72)
/** CTRL_MEAS address */
#define BME680_REG_CTRL_MEAS		UINT8_C(0x74)
/** CONFIG address */
#define BME680_REG_CONFIG		UINT8_C(0x75)
/** MEM_PAGE address */
#define BME680_REG_MEM_PAGE		UINT8_C(0xf3)
/** Unique ID address */
#define BME680_REG_UNIQUE_ID		UINT8_C(0x83)
/** Register for 1st group of coefficients */
#define BME680_REG_COEFF1		UINT8_C(0x8a)
/** Chip ID address */
#define BME680_REG_CHIP_ID		UINT8_C(0xd0)
/** Soft reset address */
#define BME680_REG_SOFT_RESET		UINT8_C(0xe0)
/** Register for 2nd group of coefficients */
#define BME680_REG_COEFF2		UINT8_C(0xe1)
/*! @} */

/*! @addtogroup en_dis Enable/Disable macros
 * @{
 */
/** Enable */
#define BME680_ENABLE		UINT8_C(0x01)
/** Disable */
#define BME680_DISABLE		UINT8_C(0x00)
/*! @} */

/*! @addtogroup osx Oversampling setting macros
 * @{
 */
/** Switch off measurement */
#define BME680_OS_NONE		UINT8_C(0)
/** Perform 1 measurement */
#define BME680_OS_1X		UINT8_C(1)
/** Perform 2 measurements */
#define BME680_OS_2X		UINT8_C(2)
/** Perform 4 measurements */
#define BME680_OS_4X		UINT8_C(3)
/** Perform 8 measurements */
#define BME680_OS_8X		UINT8_C(4)
/** Perform 16 measurements */
#define BME680_OS_16X		UINT8_C(5)
/*! @} */

/*! @addtogroup filter IIR Filter settings
 * @{
 */
/** Switch off the filter */
#define BME680_FILTER_OFF	UINT8_C(0)
/** Filter coefficient of 2 */
#define BME680_FILTER_SIZE_1	UINT8_C(1)
/** Filter coefficient of 4 */
#define BME680_FILTER_SIZE_3	UINT8_C(2)
/** Filter coefficient of 8 */
#define BME680_FILTER_SIZE_7	UINT8_C(3)
/** Filter coefficient of 16 */
#define BME680_FILTER_SIZE_15	UINT8_C(4)
/** Filter coefficient of 32 */
#define BME680_FILTER_SIZE_31	UINT8_C(5)
/** Filter coefficient of 64 */
#define BME680_FILTER_SIZE_63	UINT8_C(6)
/** Filter coefficient of 128 */
#define BME680_FILTER_SIZE_127	UINT8_C(7)
/*! @} */

/*! @addtogroup odr ODR/Standby time macros
 * @{
 */
/** Standby time of 0.59ms */
#define BME680_ODR_0_59_MS	UINT8_C(0)
/** Standby time of 62.5ms */
#define BME680_ODR_62_5_MS	UINT8_C(1)
/** Standby time of 125ms */
#define BME680_ODR_125_MS	UINT8_C(2)
/** Standby time of 250ms */
#define BME680_ODR_250_MS	UINT8_C(3)
/** Standby time of 500ms */
#define BME680_ODR_500_MS	UINT8_C(4)
/** Standby time of 1s */
#define BME680_ODR_1000_MS	UINT8_C(5)
/** Standby time of 10ms */
#define BME680_ODR_10_MS	UINT8_C(6)
/** Standby time of 20ms */
#define BME680_ODR_20_MS	UINT8_C(7)
/** No standby time */
#define BME680_ODR_NONE		UINT8_C(8)
/*! @} */

/*! @addtogroup op_mode Operating mode macros
 * @{
 */
/** Sleep operation mode */
#define BME680_SLEEP_MODE	UINT8_C(0)
/** Forced operation mode */
#define BME680_FORCED_MODE	UINT8_C(1)
/** Parallel operation mode */
#define BME680_PARALLEL_MODE	UINT8_C(2)
/** Sequential operation mode */
#define BME680_SEQUENTIAL_MODE	UINT8_C(3)
/*! @} */

/*! @addtogroup mem_page SPI page macros
 * @{
 */
/** SPI memory page 0 */
#define BME680_MEM_PAGE0	UINT8_C(0x10)
/** SPI memory page 1 */
#define BME680_MEM_PAGE1	UINT8_C(0x00)
/*! @} */

/*! @addtogroup masks Mask macros
 * @{
 */
/** Mask for number of conversions */
#define BME680_NBCONV_MSK	UINT8_C(0X0f)
/** Mask for IIR filter */
#define BME680_FILTER_MSK	UINT8_C(0X1c)
/** Mask for ODR[3] */
#define BME680_ODR3_MSK		UINT8_C(0x80)
/** Mask for ODR[2:0] */
#define BME680_ODR20_MSK	UINT8_C(0xe0)
/** Mask for temperature oversampling */
#define BME680_OST_MSK		UINT8_C(0Xe0)
/** Mask for pressure oversampling */
#define BME680_OSP_MSK		UINT8_C(0X1c)
/** Mask for humidity oversampling */
#define BME680_OSH_MSK		UINT8_C(0X07)
/** Mask for heater control */
#define BME680_HCTRL_MSK	UINT8_C(0x08)
/** Mask for run gas */
#define BME680_RUN_GAS_MSK	UINT8_C(0x30)
/** Mask for operation mode */
#define BME680_MODE_MSK		UINT8_C(0x03)
/** Mask for res heat range */
#define BME680_RHRANGE_MSK	UINT8_C(0x30)
/** Mask for range switching error */
#define BME680_RSERROR_MSK	UINT8_C(0xf0)
/** Mask for new data */
#define BME680_NEW_DATA_MSK	UINT8_C(0x80)
/** Mask for gas index */
#define BME680_GAS_INDEX_MSK	UINT8_C(0x0f)
/** Mask for gas range */
#define BME680_GAS_RANGE_MSK	UINT8_C(0x0f)
/** Mask for gas measurement valid */
#define BME680_GASM_VALID_MSK	UINT8_C(0x20)
/** Mask for heater stability */
#define BME680_HEAT_STAB_MSK	UINT8_C(0x10)
/** Mask for SPI memory page */
#define BME680_MEM_PAGE_MSK	UINT8_C(0x10)
/** Mask for reading a register in SPI */
#define BME680_SPI_RD_MSK	UINT8_C(0x80)
/** Mask for writing a register in SPI */
#define BME680_SPI_WR_MSK	UINT8_C(0x7f)
/** Mask for the H1 calibration coefficient */
#define	BME680_BIT_H1_DATA_MSK	UINT8_C(0x0f)
/*! @} */

/*! @addtogroup pos Position macros
 * @{
 */
/** Filter bit position */
#define BME680_FILTER_POS	UINT8_C(2)
/** Temperature oversampling bit position */
#define BME680_OST_POS		UINT8_C(5)
/** Pressure oversampling bit position */
#define BME680_OSP_POS		UINT8_C(2)
/** ODR[3] bit position */
#define BME680_ODR3_POS		UINT8_C(7)
/** ODR[2:0] bit position */
#define BME680_ODR20_POS	UINT8_C(5)
/** Run gas bit position */
#define BME680_RUN_GAS_POS	UINT8_C(4)
/** Heater control bit position */
#define BME680_HCTRL_POS	UINT8_C(3)
/*! @} */

/** Macro to combine two 8 bit data's to form a 16 bit data */
#define BME680_CONCAT_BYTES(msb, lsb)	(((uint16_t)msb << 8) | (uint16_t)lsb)

/** Macro to set bits */
#define BME680_SET_BITS(reg_data, bitname, data) \
		((reg_data & ~(bitname##_MSK)) | \
		((data << bitname##_POS) & bitname##_MSK))
/** Macro to get bits */
#define BME680_GET_BITS(reg_data, bitname)	((reg_data & (bitname##_MSK)) >> \
	(bitname##_POS))

/** Macro to set bits starting from position 0 */
#define BME680_SET_BITS_POS_0(reg_data, bitname, data) \
				((reg_data & ~(bitname##_MSK)) | \
				(data & bitname##_MSK))
/** Macro to get bits starting from position 0 */
#define BME680_GET_BITS_POS_0(reg_data, bitname)  (reg_data & (bitname##_MSK))

/** Type definitions */
/*!
 * @brief Generic communication function pointer
 * @param[in] dev_id: Place holder to store the id of the device structure
 *                    Can be used to store the index of the Chip select or
 *                    I2C address of the device.
 * @param[in] reg_addr:	Used to select the register the where data needs to
 *                      be read from or written to.
 * @param[in,out] reg_data: Data array to read/write
 * @param[in] len: Length of the data array
 */
typedef int8_t (*bme680_com_fptr_t)(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);

/*!
 * @brief Delay function pointer
 * @param[in] period: Time period in milliseconds
 */
typedef void (*bme680_delay_fptr_t)(uint32_t period);

/*!
 * @brief Interface selection Enumerations
 */
enum bme680_intf {
	/*! SPI interface */
	BME680_SPI_INTF,
	/*! I2C interface */
	BME680_I2C_INTF
};

/* Structure definitions */
/*!
 * @brief Sensor field data structure
 */
struct bme680_data {
	/*! Contains new_data, gasm_valid & heat_stab */
	uint8_t status;
	/*! The index of the heater profile used */
	uint8_t gas_index;
	/*! Measurement index to track order */
	uint8_t meas_index;
	/*! Heater resistance */
	uint8_t res_heat;
	/*! Current DAC */
	uint8_t idac;
	/*! Gas wait period */
	uint8_t gas_wait;
#ifndef BME680_USE_FPU
	/*! Temperature in degree celsius x100 */
	int16_t temperature;
	/*! Pressure in Pascal */
	uint32_t pressure;
	/*! Humidity in % relative humidity x1000 */
	uint32_t humidity;
	/*! Gas resistance in Ohms */
	uint32_t gas_resistance;
#else
	/*! Temperature in degree celsius */
	float temperature;
	/*! Pressure in Pascal */
	float pressure;
	/*! Humidity in % relative humidity x1000 */
	float humidity;
	/*! Gas resistance in Ohms */
	float gas_resistance;

#endif

};

/*!
 * @brief Structure to hold the calibration coefficients
 */
struct bme680_calib_data {
	/*! Calibration coefficient for the humidity sensor */
	uint16_t par_h1;
	/*! Calibration coefficient for the humidity sensor */
	uint16_t par_h2;
	/*! Calibration coefficient for the humidity sensor */
	int8_t par_h3;
	/*! Calibration coefficient for the humidity sensor */
	int8_t par_h4;
	/*! Calibration coefficient for the humidity sensor */
	int8_t par_h5;
	/*! Calibration coefficient for the humidity sensor */
	uint8_t par_h6;
	/*! Calibration coefficient for the humidity sensor */
	int8_t par_h7;
	/*! Calibration coefficient for the gas sensor */
	int8_t par_gh1;
	/*! Calibration coefficient for the gas sensor */
	int16_t par_gh2;
	/*! Calibration coefficient for the gas sensor */
	int8_t par_gh3;
	/*! Calibration coefficient for the temperature sensor */
	uint16_t par_t1;
	/*! Calibration coefficient for the temperature sensor */
	int16_t par_t2;
	/*! Calibration coefficient for the temperature sensor */
	int8_t par_t3;
	/*! Calibration coefficient for the pressure sensor */
	uint16_t par_p1;
	/*! Calibration coefficient for the pressure sensor */
	int16_t par_p2;
	/*! Calibration coefficient for the pressure sensor */
	int8_t par_p3;
	/*! Calibration coefficient for the pressure sensor */
	int16_t par_p4;
	/*! Calibration coefficient for the pressure sensor */
	int16_t par_p5;
	/*! Calibration coefficient for the pressure sensor */
	int8_t par_p6;
	/*! Calibration coefficient for the pressure sensor */
	int8_t par_p7;
	/*! Calibration coefficient for the pressure sensor */
	int16_t par_p8;
	/*! Calibration coefficient for the pressure sensor */
	int16_t par_p9;
	/*! Calibration coefficient for the pressure sensor */
	uint8_t par_p10;
#ifndef BME680_USE_FPU
	/*! Variable to store the intermediate temperature coefficient */
	int32_t t_fine;
#else
	/*! Variable to store the intermediate temperature coefficient */
	float t_fine;
#endif
	/*! Heater resistance range coefficient */
	uint8_t res_heat_range;
	/*! Heater resistance value coefficient */
	int8_t res_heat_val;
	/*! Gas resistance range switching error coefficient */
	int8_t range_sw_err;
};

/*!
 * @brief BME680 sensor settings structure which comprises of ODR,
 * over-sampling and filter settings.
 */
struct bme680_conf {
	/*! Humidity oversampling. Refer @ref osx*/
	uint8_t os_hum;
	/*! Temperature oversampling. Refer @ref osx */
	uint8_t os_temp;
	/*! Pressure oversampling. Refer @ref osx */
	uint8_t os_pres;
	/*! Filter coefficient. Refer @ref filter*/
	uint8_t filter;
	/*! Standby time between sequential mode measurement profiles.
	 * Refer @ref odr*/
	uint8_t odr;
};

/*!
 * @brief BME680 gas heater configuration
 */
struct bme680_heatr_conf {
	/*! Enable gas measurement. Refer @ref en_dis */
	uint8_t enable;
	/*! Store the heater temperature for forced mode degree Celsius */
	uint16_t heatr_temp;
	/*! Store the heating duration for forced mode in milliseconds */
	uint16_t heatr_dur;
	/*! Store the heater temperature profile in degree Celsius */
	uint16_t *heatr_temp_prof;
	/*! Store the heating duration profile in milliseconds */
	uint16_t *heatr_dur_prof;
	/*! Variable to store the length of the heating profile */
	uint8_t profile_len;
	/*! Variable to store heating duration for parallel mode
	 * in milliseconds */
	uint16_t shared_heatr_dur;
};

/*!
 * @brief BME680 device structure
 */
struct bme680_dev {
	/*! Chip Id */
	uint8_t chip_id;
	/*! Device Id */
	uint8_t dev_id;
	/*! Unique id */
	uint32_t unique_id;
	/*! SPI/I2C interface */
	enum bme680_intf intf;
	/*! Memory page used */
	uint8_t mem_page;
	/*! Ambient temperature in Degree C*/
	int8_t amb_temp;
	/*! Sensor calibration data */
	struct bme680_calib_data calib;
	/*! Read function pointer */
	bme680_com_fptr_t read;
	/*! Write function pointer */
	bme680_com_fptr_t write;
	/*! Delay function pointer */
	bme680_delay_fptr_t delay_ms;
	/*! Communication function result */
	int8_t com_rslt;
	/*! Store the info messages */
	uint8_t info_msg;
};

#endif /* BME680_DEFS_H_ */
