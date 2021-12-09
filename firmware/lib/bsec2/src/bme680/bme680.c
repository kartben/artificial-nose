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
 * @file	bme680.c
 * @date	7 Nov 2018
 * @version	4.1.0
 * @brief	Source file for the BME680 Sensor API
 */

#include "bme680.h"

/*! @addtogroup coeff_idx Coefficient index macros
 * @{
 */
/** Length for all coefficients */
#define LEN_COEFF_ALL		UINT8_C(42)
/** Length for 1st group of coefficients */
#define LEN_COEFF1		UINT8_C(23)
/** Length for 2nd group of coefficients */
#define LEN_COEFF2		UINT8_C(14)
/** Length for 3rd group of coefficients */
#define LEN_COEFF3		UINT8_C(5)
/** Length of the field */
#define LEN_FIELD		UINT8_C(17)
/** Length between two fields */
#define LEN_FIELD_OFFSET	UINT8_C(17)
/** Length of the configuration register */
#define LEN_CONFIG		UINT8_C(5)
/** Length of the interleaved buffer */
#define LEN_INTERLEAVE_BUFF	UINT8_C(20)
/*! @} */

/*! @addtogroup coeff_idx Coefficient index macros
 * @{
 */
/** Coefficient T2 LSB position */
#define T2_LSB_IDX		(0)
/** Coefficient T2 MSB position */
#define T2_MSB_IDX		(1)
/** Coefficient T3 position */
#define T3_IDX			(2)
/** Coefficient P1 LSB position */
#define P1_LSB_IDX		(4)
/** Coefficient P1 MSB position */
#define P1_MSB_IDX		(5)
/** Coefficient P2 LSB position */
#define P2_LSB_IDX		(6)
/** Coefficient P2 MSB position */
#define P2_MSB_IDX		(7)
/** Coefficient P3 position */
#define P3_IDX			(8)
/** Coefficient P4 LSB position */
#define P4_LSB_IDX		(10)
/** Coefficient P4 MSB position */
#define P4_MSB_IDX		(11)
/** Coefficient P5 LSB position */
#define P5_LSB_IDX		(12)
/** Coefficient P5 MSB position */
#define P5_MSB_IDX		(13)
/** Coefficient P7 position */
#define P7_IDX			(14)
/** Coefficient P6 position */
#define P6_IDX			(15)
/** Coefficient P8 LSB position */
#define P8_LSB_IDX		(18)
/** Coefficient P8 MSB position */
#define P8_MSB_IDX		(19)
/** Coefficient P9 LSB position */
#define P9_LSB_IDX		(20)
/** Coefficient P9 MSB position */
#define P9_MSB_IDX		(21)
/** Coefficient P10 position */
#define P10_IDX			(22)
/** Coefficient H2 MSB position */
#define H2_MSB_IDX		(23)
/** Coefficient H2 LSB position */
#define H2_LSB_IDX		(24)
/** Coefficient H1 LSB position */
#define H1_LSB_IDX		(24)
/** Coefficient H1 MSB position */
#define H1_MSB_IDX		(25)
/** Coefficient H3 position */
#define H3_IDX			(26)
/** Coefficient H4 position */
#define H4_IDX			(27)
/** Coefficient H5 position */
#define H5_IDX			(28)
/** Coefficient H6 position */
#define H6_IDX			(29)
/** Coefficient H7 position */
#define H7_IDX			(30)
/** Coefficient T1 LSB position */
#define T1_LSB_IDX		(31)
/** Coefficient T1 MSB position */
#define T1_MSB_IDX		(32)
/** Coefficient GH2 LSB position */
#define GH2_LSB_IDX		(33)
/** Coefficient GH2 MSB position */
#define GH2_MSB_IDX		(34)
/** Coefficient GH1 position */
#define GH1_IDX			(35)
/** Coefficient GH3 position */
#define GH3_IDX			(36)
/** Coefficient res heat value position */
#define RES_HEAT_VAL_IDX	(37)
/** Coefficient res heat range position */
#define RES_HEAT_RANGE_IDX	(39)
/** Coefficient range switching error position */
#define RANGE_SW_ERR_IDX	(41)
/*! @} */

/*! @addtogroup heatr_ctrl Heater control macros
 * @{
 */
/** Enable heater */
#define ENABLE_HEATER	UINT8_C(0x00)
/** Disable heater */
#define DISABLE_HEATER	UINT8_C(0x01)
/*! @} */

/*! @addtogroup gas_meas Gas measurement macros
 * @{
 */
/** Disable gas measurement */
#define DISABLE_GAS_MEAS	UINT8_C(0x00)
/** Enable gas measurement low */
#define ENABLE_GAS_MEAS_L	UINT8_C(0x01)
/** Enable gas measurement high */
#define ENABLE_GAS_MEAS_H	UINT8_C(0x02)
/*! @} */

/* This internal API is used to read the calibration coefficients */
static int8_t get_calib_data(struct bme680_dev *dev);

/* This internal API is used to calculate the gas wait */
static uint8_t calc_gas_wait(uint16_t dur);

#ifndef BME680_USE_FPU

/* This internal API is used to calculate the temperature in integer */
static int16_t calc_temperature(uint32_t temp_adc, struct bme680_dev *dev);

/* This internal API is used to calculate the pressure in integer */
static uint32_t calc_pressure(uint32_t pres_adc, const struct bme680_dev *dev);

/* This internal API is used to calculate the humidity in integer */
static uint32_t calc_humidity(uint16_t hum_adc, const struct bme680_dev *dev);

/* This internal API is used to calculate the gas resistance */
static uint32_t calc_gas_resistance_low(uint16_t gas_res_adc, uint8_t gas_range, const struct bme680_dev *dev);

/* This internal API is used to calculate the gas resistance */
static uint32_t calc_gas_resistance_high(uint16_t gas_res_adc, uint8_t gas_range, const struct bme680_dev *dev);

/* This internal API is used to calculate the heater resistance using integer */
static uint8_t calc_res_heat(uint16_t temp, const struct bme680_dev *dev);

#else
/* This internal API is used to calculate the temperature value in float */
static float calc_temperature(uint32_t temp_adc, struct bme680_dev *dev);

/* This internal API is used to calculate the pressure value in float */
static float calc_pressure(uint32_t pres_adc, const struct bme680_dev *dev);

/* This internal API is used to calculate the humidity value in float */
static float calc_humidity(uint16_t hum_adc, const struct bme680_dev *dev);

/* This internal API is used to calculate the gas resistance value in float */
static float calc_gas_resistance_low(uint16_t gas_res_adc, uint8_t gas_range, const struct bme680_dev *dev);

/* This internal API is used to calculate the gas resistance value in float */
static float calc_gas_resistance_high(uint16_t gas_res_adc, uint8_t gas_range, const struct bme680_dev *dev);

/* This internal API is used to calculate the heater resistance value using float */
static uint8_t calc_res_heat(uint16_t temp, const struct bme680_dev *dev);

#endif

/* This internal API is used to calculate the field data of sensor */
static int8_t read_field_data(uint8_t index, struct bme680_data *data, struct bme680_dev *dev);

/* This internal API is used to switch between SPI memory pages */
static int8_t set_mem_page(uint8_t reg_addr, struct bme680_dev *dev);

/* This internal API is used to get the current SPI memory page */
static int8_t get_mem_page(struct bme680_dev *dev);

/* This internal API is used to check the bme680_dev for null pointers */
static int8_t null_ptr_check(const struct bme680_dev *dev);

/* This internal API is used to limit the max value of a parameter */
static int8_t boundary_check(uint8_t *value, uint8_t max, struct bme680_dev *dev);

/* This internal API is used to calculate the register value for
 * shared heater duration */
static uint8_t calc_heatr_dur_shared(uint16_t dur);

/* This internal API is used to swap two fields */
static void swap_fields(uint8_t index1, uint8_t index2, struct bme680_data *field[]);

/* This internal API is used sort the sensor data */
static void sort_sensor_data(uint8_t low_index, uint8_t high_index, struct bme680_data *field[]);

/*!
 * @brief This API initializes the sensor
 */
int8_t bme680_init(struct bme680_dev *dev)
{
	int8_t rslt;
	uint8_t uid_regs[4];

	rslt = bme680_soft_reset(dev);
	if (rslt == BME680_OK) {
		rslt = bme680_get_regs(BME680_REG_CHIP_ID, &dev->chip_id, 1, dev);
		if (rslt == BME680_OK) {
			if (dev->chip_id == BME680_CHIP_ID) {
				/* Get the Calibration data */
				rslt = get_calib_data(dev);

				if (rslt == BME680_OK) {
					rslt = bme680_get_regs(BME680_REG_UNIQUE_ID, uid_regs, 4, dev);
					dev->unique_id = ((((uint32_t)uid_regs[3] + ((uint32_t)uid_regs[2] << 8))
						& 0x7fff) << 16) + (((uint32_t)uid_regs[1]) << 8)
						+ (uint32_t)uid_regs[0];
				}
			} else {
				rslt = BME680_E_DEV_NOT_FOUND;
			}
		}
	}

	return rslt;
}

/*!
 * @brief This API writes the given data to the registers of the sensor.
 */
int8_t bme680_set_regs(const uint8_t *reg_addr, const uint8_t *reg_data, uint8_t len, struct bme680_dev *dev)
{
	int8_t rslt;
	/* Length of the temporary buffer is 2*(length of register)*/
	uint8_t tmp_buff[LEN_INTERLEAVE_BUFF] = { 0 };
	uint16_t index;

	/* Check for null pointer in the device structure*/
	rslt = null_ptr_check(dev);
	if ((rslt == BME680_OK) && reg_addr && reg_data) {
		if ((len > 0) && (len <= (LEN_INTERLEAVE_BUFF / 2))) {
			/* Interleave the 2 arrays */
			for (index = 0; index < len; index++) {
				if (dev->intf == BME680_SPI_INTF) {
					/* Set the memory page */
					rslt = set_mem_page(reg_addr[index], dev);
					tmp_buff[(2 * index)] = reg_addr[index] & BME680_SPI_WR_MSK;
				} else {
					tmp_buff[(2 * index)] = reg_addr[index];
				}
				tmp_buff[(2 * index) + 1] = reg_data[index];
			}
			/* Write the interleaved array */
			if (rslt == BME680_OK) {
				dev->com_rslt = dev->write(dev->dev_id, tmp_buff[0], &tmp_buff[1], (2 * len) - 1);
				if (dev->com_rslt != 0)
					rslt = BME680_E_COM_FAIL;
			}
		} else {
			rslt = BME680_E_INVALID_LENGTH;
		}
	} else {
		rslt = BME680_E_NULL_PTR;
	}

	return rslt;
}


/*!
 * @brief This API reads the data from the registers of the sensor.
 */
int8_t bme680_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint16_t len, struct bme680_dev *dev)
{
	int8_t rslt;

	/* Check for null pointer in the device structure*/
	rslt = null_ptr_check(dev);
	if ((rslt == BME680_OK) && reg_data) {
		if (dev->intf == BME680_SPI_INTF) {
			/* Set the memory page */
			rslt = set_mem_page(reg_addr, dev);
			if (rslt == BME680_OK)
				reg_addr = reg_addr | BME680_SPI_RD_MSK;
		}

		dev->com_rslt = dev->read(dev->dev_id, reg_addr, reg_data, len);
		if (dev->com_rslt != 0)
			rslt = BME680_E_COM_FAIL;

	} else
		rslt = BME680_E_NULL_PTR;

	return rslt;
}

/*!
 * @brief This API performs the soft reset of the sensor.
 */
int8_t bme680_soft_reset(struct bme680_dev *dev)
{
	int8_t rslt;
	uint8_t reg_addr = BME680_REG_SOFT_RESET;
	/* 0xb6 is the soft reset command */
	uint8_t soft_rst_cmd = BME680_SOFT_RESET_CMD;

	/* Check for null pointer in the device structure*/
	rslt = null_ptr_check(dev);
	if (rslt == BME680_OK) {
		if (dev->intf == BME680_SPI_INTF)
			rslt = get_mem_page(dev);

		/* Reset the device */
		if (rslt == BME680_OK) {
			rslt = bme680_set_regs(&reg_addr, &soft_rst_cmd, 1, dev);
			/* Wait for 5ms */
			dev->delay_ms(BME680_PERIOD_RESET);

			if (rslt == BME680_OK) {
				/* After reset get the memory page */
				if (dev->intf == BME680_SPI_INTF)
					rslt = get_mem_page(dev);
			}
		}
	}

	return rslt;
}

/*!
 * @brief This API is used to set the oversampling, filter and odr
 * configuration
 */
int8_t bme680_set_conf(struct bme680_conf *conf, struct bme680_dev *dev)
{
	int8_t rslt;
	uint8_t odr20 = 0, odr3 = 1;
	uint8_t current_op_mode;
	/* Register data starting from BME680_REG_CTRL_GAS_1(0x71) up to BME680_REG_CONFIG(0x75) */
	uint8_t reg_array[LEN_CONFIG] = { 0x71, 0x72, 0x73, 0x74, 0x75 };
	uint8_t data_array[LEN_CONFIG] = { 0 };

	rslt = bme680_get_op_mode(&current_op_mode, dev);

	if (rslt == BME680_OK) {
		/* Configure only in the sleep mode */
		rslt = bme680_set_op_mode(BME680_SLEEP_MODE, dev);
	}

	if (conf == NULL)
		rslt = BME680_E_NULL_PTR;
	else if (rslt == BME680_OK) {
		/* Read the whole configuration and write it back once later */
		rslt = bme680_get_regs(reg_array[0], data_array, LEN_CONFIG, dev);

		dev->info_msg = BME680_OK;

		if (rslt == BME680_OK)
			rslt = boundary_check(&conf->filter, BME680_FILTER_SIZE_127, dev);

		if (rslt == BME680_OK)
			rslt = boundary_check(&conf->os_temp, BME680_OS_16X, dev);

		if (rslt == BME680_OK)
			rslt = boundary_check(&conf->os_pres, BME680_OS_16X, dev);

		if (rslt == BME680_OK)
			rslt = boundary_check(&conf->os_hum, BME680_OS_16X, dev);

		if (rslt == BME680_OK)
			rslt = boundary_check(&conf->odr, BME680_ODR_NONE, dev);

		if (rslt == BME680_OK) {
			data_array[4] = BME680_SET_BITS(data_array[4], BME680_FILTER, conf->filter);
			data_array[3] = BME680_SET_BITS(data_array[3], BME680_OST, conf->os_temp);
			data_array[3] = BME680_SET_BITS(data_array[3], BME680_OSP, conf->os_pres);
			data_array[1] = BME680_SET_BITS_POS_0(data_array[1], BME680_OSH, conf->os_hum);

			if (conf->odr != BME680_ODR_NONE) {
				odr20 = conf->odr;
				odr3 = 0;
			}

			data_array[4] = BME680_SET_BITS(data_array[4], BME680_ODR20, odr20);
			data_array[0] = BME680_SET_BITS(data_array[0], BME680_ODR3, odr3);
		}
	}

	if (rslt == BME680_OK)
		rslt = bme680_set_regs(reg_array, data_array, LEN_CONFIG, dev);

	if ((current_op_mode != BME680_SLEEP_MODE) && (rslt == BME680_OK))
		rslt = bme680_set_op_mode(current_op_mode, dev);

	return rslt;
}

/*!
 * @brief This API is used to get the oversampling, filter and odr
 * configuration
 */
int8_t bme680_get_conf(struct bme680_conf *conf, struct bme680_dev *dev)
{
	int8_t rslt;
	/* starting address of the register array for burst read*/
	uint8_t reg_addr = BME680_REG_CTRL_GAS_1;
	uint8_t data_array[LEN_CONFIG];

	rslt = bme680_get_regs(reg_addr, data_array, 5, dev);

	if (!conf)
		rslt = BME680_E_NULL_PTR;
	else if (rslt == BME680_OK) {
		conf->os_hum = BME680_GET_BITS_POS_0(data_array[1], BME680_OSH);
		conf->filter = BME680_GET_BITS(data_array[4], BME680_FILTER);
		conf->os_temp = BME680_GET_BITS(data_array[3], BME680_OST);
		conf->os_pres = BME680_GET_BITS(data_array[3], BME680_OSP);

		if (BME680_GET_BITS(data_array[0], BME680_ODR3))
			conf->odr = BME680_ODR_NONE;
		else
			conf->odr = BME680_GET_BITS(data_array[4], BME680_ODR20);
	}

	return rslt;
}

/*!
 * @brief This API is used to set the operating mode of the sensor.
 */
int8_t bme680_set_op_mode(const uint8_t op_mode, struct bme680_dev *dev)
{
	int8_t rslt;
	uint8_t tmp_pow_mode;
	uint8_t pow_mode = 0;
	uint8_t reg_addr = BME680_REG_CTRL_MEAS;

	/* Call until in sleep */
	do {
		rslt = bme680_get_regs(BME680_REG_CTRL_MEAS, &tmp_pow_mode, 1, dev);
		if (rslt == BME680_OK) {
			/* Put to sleep before changing mode */
			pow_mode = (tmp_pow_mode & BME680_MODE_MSK);

			if (pow_mode != BME680_SLEEP_MODE) {
				tmp_pow_mode &= ~BME680_MODE_MSK; /* Set to sleep */
				rslt = bme680_set_regs(&reg_addr, &tmp_pow_mode, 1, dev);
				dev->delay_ms(BME680_PERIOD_POLL);
			}
		}
	} while ((pow_mode != BME680_SLEEP_MODE) && (rslt == BME680_OK));

	/* Already in sleep */
	if ((op_mode != BME680_SLEEP_MODE) && (rslt == BME680_OK)) {
		tmp_pow_mode = (tmp_pow_mode & ~BME680_MODE_MSK) | (op_mode & BME680_MODE_MSK);
		rslt = bme680_set_regs(&reg_addr, &tmp_pow_mode, 1, dev);
	}

	return rslt;
}

/*!
 * @brief This API is used to get the operating mode of the sensor.
 */
int8_t bme680_get_op_mode(uint8_t *op_mode, struct bme680_dev *dev)
{
	int8_t rslt;
	uint8_t mode;

	if (op_mode) {
		rslt = bme680_get_regs(BME680_REG_CTRL_MEAS, &mode, 1, dev);
		/* Masking the other register bit info*/
		*op_mode = mode & BME680_MODE_MSK;
	} else
		rslt = BME680_E_NULL_PTR;

	return rslt;
}

/*!
 * @brief This API is used to get the remaining duration that can be used for heating.
 */
uint16_t bme680_get_meas_dur(const uint8_t op_mode, const struct bme680_conf *conf)
{
	uint32_t tph_dur = 0; /* Calculate in us */
	uint32_t meas_cycles;
	uint8_t os_to_meas_cycles[6] = { 0, 1, 2, 4, 8, 16 };

	if (conf != NULL) {
		meas_cycles = os_to_meas_cycles[conf->os_temp];
		meas_cycles += os_to_meas_cycles[conf->os_pres];
		meas_cycles += os_to_meas_cycles[conf->os_hum];

		/* TPH measurement duration */
		tph_dur = meas_cycles * UINT32_C(1963);
		tph_dur += UINT32_C(477 * 4); /* TPH switching duration */
		tph_dur += UINT32_C(477 * 5); /* Gas measurement duration */
		tph_dur += UINT32_C(500); /* Get it to the closest integer when converted to ms.*/
		tph_dur /= UINT32_C(1000); /* Convert to ms */

		if (op_mode != BME680_PARALLEL_MODE)
			tph_dur += UINT32_C(1); /* Wake up duration of 1ms */
	}

	return (uint16_t)tph_dur;
}

/*!
 * @brief This API reads the pressure, temperature and humidity and gas data
 * from the sensor, compensates the data and store it in the bme680_data
 * structure instance passed by the user.
 */
int8_t bme680_get_data(uint8_t op_mode, struct bme680_data *data, uint8_t *n_data,
		struct bme680_dev *dev)
{
	int8_t rslt = BME680_OK;
	uint8_t i = 0, j = 0, new_fields = 0;

	struct bme680_data *field_ptr[3];
	struct bme680_data field_data[3];

	field_ptr[0] = &field_data[0];
	field_ptr[1] = &field_data[1];
	field_ptr[2] = &field_data[2];

	/* Reading the sensor data in forced mode only */
	if (op_mode == BME680_FORCED_MODE) {
		rslt = read_field_data(0, data, dev);

		if (rslt == BME680_OK) {
			if (data->status & BME680_NEW_DATA_MSK)
				new_fields = 1;
			else {
				new_fields = 0;
				rslt = BME680_W_NO_NEW_DATA;
			}
		}
	} else if ((op_mode == BME680_PARALLEL_MODE) || (op_mode == BME680_SEQUENTIAL_MODE)) {

		/* Read the 3 fields and count the number of new data fields */
		for (i = 0; ((i < 3) && (rslt == BME680_OK)); i++) {
			rslt = read_field_data(i, field_ptr[i], dev);
			if (field_ptr[i]->status & BME680_NEW_DATA_MSK)
				new_fields++;
		}

		/* Sort the sensor data in parallel & sequential modes*/
		for (i = 0; (i < 2) && (rslt == BME680_OK); i++)
			for (j = i + 1; j < 3; j++)
				sort_sensor_data(i, j, field_ptr);

		/* Copy the sorted data */
		for (i = 0; ((i < 3) && (rslt == BME680_OK)); i++)
			data[i] = *field_ptr[i];

		if (new_fields == 0)
			rslt = BME680_W_NO_NEW_DATA;

	} else
		rslt = BME680_W_DEFINE_OP_MODE;

	if (n_data == NULL)
		rslt = BME680_E_NULL_PTR;
	else
		*n_data = new_fields;

	return rslt;
}

/* This internal API is used to read the calibration coefficients */
static int8_t get_calib_data(struct bme680_dev *dev)
{
	int8_t rslt;
	uint8_t coeff_array[LEN_COEFF_ALL];

	rslt = bme680_get_regs(BME680_REG_COEFF1, coeff_array, LEN_COEFF1, dev);

	if (rslt == BME680_OK)
		rslt = bme680_get_regs(BME680_REG_COEFF2, &coeff_array[LEN_COEFF1], LEN_COEFF2, dev);

	if (rslt == BME680_OK)
		rslt = bme680_get_regs(BME680_REG_COEFF3, &coeff_array[LEN_COEFF1 + LEN_COEFF2],
		LEN_COEFF3, dev);

	if (rslt == BME680_OK) {
		/* Temperature related coefficients */
		dev->calib.par_t1 = (uint16_t)(BME680_CONCAT_BYTES(coeff_array[T1_MSB_IDX],
				coeff_array[T1_LSB_IDX]));
		dev->calib.par_t2 = (int16_t)(BME680_CONCAT_BYTES(coeff_array[T2_MSB_IDX],
				coeff_array[T2_LSB_IDX]));
		dev->calib.par_t3 = (int8_t)(coeff_array[T3_IDX]);

		/* Pressure related coefficients */
		dev->calib.par_p1 = (uint16_t)(BME680_CONCAT_BYTES(coeff_array[P1_MSB_IDX],
				coeff_array[P1_LSB_IDX]));
		dev->calib.par_p2 = (int16_t)(BME680_CONCAT_BYTES(coeff_array[P2_MSB_IDX],
				coeff_array[P2_LSB_IDX]));
		dev->calib.par_p3 = (int8_t)coeff_array[P3_IDX];
		dev->calib.par_p4 = (int16_t)(BME680_CONCAT_BYTES(coeff_array[P4_MSB_IDX],
				coeff_array[P4_LSB_IDX]));
		dev->calib.par_p5 = (int16_t)(BME680_CONCAT_BYTES(coeff_array[P5_MSB_IDX],
				coeff_array[P5_LSB_IDX]));
		dev->calib.par_p6 = (int8_t)(coeff_array[P6_IDX]);
		dev->calib.par_p7 = (int8_t)(coeff_array[P7_IDX]);
		dev->calib.par_p8 = (int16_t)(BME680_CONCAT_BYTES(coeff_array[P8_MSB_IDX],
				coeff_array[P8_LSB_IDX]));
		dev->calib.par_p9 = (int16_t)(BME680_CONCAT_BYTES(coeff_array[P9_MSB_IDX],
				coeff_array[P9_LSB_IDX]));
		dev->calib.par_p10 = (uint8_t)(coeff_array[P10_IDX]);

		/* Humidity related coefficients */
		dev->calib.par_h1 = (uint16_t)(((uint16_t)coeff_array[H1_MSB_IDX] << 4)
			| (coeff_array[H1_LSB_IDX] & BME680_BIT_H1_DATA_MSK));
		dev->calib.par_h2 = (uint16_t)(((uint16_t)coeff_array[H2_MSB_IDX] << 4)
			| ((coeff_array[H2_LSB_IDX]) >> 4));
		dev->calib.par_h3 = (int8_t)coeff_array[H3_IDX];
		dev->calib.par_h4 = (int8_t)coeff_array[H4_IDX];
		dev->calib.par_h5 = (int8_t)coeff_array[H5_IDX];
		dev->calib.par_h6 = (uint8_t)coeff_array[H6_IDX];
		dev->calib.par_h7 = (int8_t)coeff_array[H7_IDX];

		/* Gas heater related coefficients */
		dev->calib.par_gh1 = (int8_t)coeff_array[GH1_IDX];
		dev->calib.par_gh2 = (int16_t)(BME680_CONCAT_BYTES(coeff_array[GH2_MSB_IDX],
				coeff_array[GH2_LSB_IDX]));
		dev->calib.par_gh3 = (int8_t)coeff_array[GH3_IDX];

		/* Other coefficients */
		dev->calib.res_heat_range = ((coeff_array[RES_HEAT_RANGE_IDX] & BME680_RHRANGE_MSK) / 16);
		dev->calib.res_heat_val = (int8_t)coeff_array[RES_HEAT_VAL_IDX];
		dev->calib.range_sw_err = ((int8_t)(coeff_array[RANGE_SW_ERR_IDX] & BME680_RSERROR_MSK)) / 16;
	}

	return rslt;
}

/*!
 * @brief This API is used to set the gas configuration of the sensor.
 */
int8_t bme680_set_heatr_conf(uint8_t op_mode, const struct bme680_heatr_conf *conf, struct bme680_dev *dev)
{
	int8_t rslt;
	uint8_t nb_conv = 0, hctrl, run_gas;
	uint8_t rh_reg_addr[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8_t rh_reg_data[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8_t gw_reg_addr[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8_t gw_reg_data[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8_t heater_dur_shared_addr = BME680_REG_SHD_HEATR_DUR;
	uint8_t shared_dur;
	uint8_t i;
	uint8_t write_len = 0;
	uint8_t ctrl_gas_data[2];
	uint8_t ctrl_gas_addr[2] = { BME680_REG_CTRL_GAS_0, BME680_REG_CTRL_GAS_1 };

	/* Check for null pointer in the device structure*/
	rslt = bme680_set_op_mode(BME680_SLEEP_MODE, dev);
	if (rslt == BME680_OK) {
		switch (op_mode) {
		case BME680_FORCED_MODE:
			if (conf == NULL) {
				rslt = BME680_E_NULL_PTR;
				break;
			}

			rh_reg_addr[0] = BME680_REG_RES_HEAT0;
			rh_reg_data[0] = calc_res_heat(conf->heatr_temp, dev);
			gw_reg_addr[0] = BME680_REG_GAS_WAIT0;
			gw_reg_data[0] = calc_gas_wait(conf->heatr_dur);
			nb_conv = 0;
			write_len = 1;
			break;

		case BME680_SEQUENTIAL_MODE:
			if ((!conf) || (!conf->heatr_dur_prof) || (!conf->heatr_temp_prof)) {
				rslt = BME680_E_NULL_PTR;
				break;
			}

			for (i = 0; i < conf->profile_len; i++) {
				rh_reg_addr[i] = BME680_REG_RES_HEAT0 + i;
				rh_reg_data[i] = calc_res_heat(conf->heatr_temp_prof[i], dev);
				gw_reg_addr[i] = BME680_REG_GAS_WAIT0 + i;
				gw_reg_data[i] = calc_gas_wait(conf->heatr_dur_prof[i]);
			}

			nb_conv = conf->profile_len;
			write_len = conf->profile_len;
			break;

		case BME680_PARALLEL_MODE:
			if ((!conf) || (!conf->heatr_dur_prof) || (!conf->heatr_temp_prof)) {
				rslt = BME680_E_NULL_PTR;
				break;
			}

			if (conf->shared_heatr_dur == 0)
				rslt = BME680_W_DEFINE_SHD_HEATR_DUR;

			for (i = 0; i < conf->profile_len; i++) {
				rh_reg_addr[i] = BME680_REG_RES_HEAT0 + i;
				rh_reg_data[i] = calc_res_heat(conf->heatr_temp_prof[i], dev);
				gw_reg_addr[i] = BME680_REG_GAS_WAIT0 + i;
				gw_reg_data[i] = (uint8_t)conf->heatr_dur_prof[i];
			}

			nb_conv = conf->profile_len;
			write_len = conf->profile_len;

			shared_dur = calc_heatr_dur_shared(conf->shared_heatr_dur);

			if (rslt == BME680_OK)
				rslt = bme680_set_regs(&heater_dur_shared_addr, &shared_dur, 1, dev);

			break;
		default:
			rslt = BME680_W_DEFINE_OP_MODE;
		}
		if (rslt == BME680_OK)
			rslt = bme680_set_regs(rh_reg_addr, rh_reg_data, write_len, dev);

		if (rslt == BME680_OK)
			rslt = bme680_set_regs(gw_reg_addr, gw_reg_data, write_len, dev);

		if (rslt == BME680_OK) {
			rslt = bme680_get_regs(BME680_REG_CTRL_GAS_0, ctrl_gas_data, 2, dev);

			if (rslt == BME680_OK) {
				if ((conf != NULL) && (conf->enable == BME680_ENABLE)) {
					hctrl = ENABLE_HEATER;
					if(dev->chip_id == BME680_CHIP_ID)
						run_gas = ENABLE_GAS_MEAS_L;
					else
						run_gas = ENABLE_GAS_MEAS_H;
				} else {
					hctrl = DISABLE_HEATER;
					run_gas = DISABLE_GAS_MEAS;
				}

				ctrl_gas_data[0] = BME680_SET_BITS(ctrl_gas_data[0], BME680_HCTRL, hctrl);
				ctrl_gas_data[1] = BME680_SET_BITS_POS_0(ctrl_gas_data[1], BME680_NBCONV, nb_conv);
				ctrl_gas_data[1] = BME680_SET_BITS(ctrl_gas_data[1], BME680_RUN_GAS, run_gas);

				rslt = bme680_set_regs(ctrl_gas_addr, ctrl_gas_data, 2, dev);
			}
		}
	}

	return rslt;
}

/*!
 * @brief This API is used to get the gas configuration of the sensor.
 */
int8_t bme680_get_heatr_conf(const struct bme680_heatr_conf *conf, struct bme680_dev *dev)
{
	int8_t rslt;
	uint8_t data_array[10] = { 0 };
	uint8_t i;

	/* FIXME: Add conversion to deg C and ms and add the other parameters */
	rslt = bme680_get_regs(BME680_REG_RES_HEAT0, data_array, 10, dev);
	if (rslt == BME680_OK) {
		if (conf && conf->heatr_dur_prof && conf->heatr_temp_prof) {
			for (i = 0; i < 10; i++)
				conf->heatr_temp_prof[i] = data_array[i];

			rslt = bme680_get_regs(BME680_REG_GAS_WAIT0, data_array, 10, dev);

			if (rslt == BME680_OK) {
				for (i = 0; i < 10; i++)
					conf->heatr_dur_prof[i] = data_array[i];
			}
		} else
			rslt = BME680_E_NULL_PTR;
	}

	return rslt;
}

#ifndef BME680_USE_FPU

/* This internal API is used to calculate the temperature in integer */
static int16_t calc_temperature(uint32_t temp_adc, struct bme680_dev *dev)
{
	int64_t var1;
	int64_t var2;
	int64_t var3;
	int16_t calc_temp;

	/*lint -save -e701 -e702 -e704 */
	var1 = ((int32_t)temp_adc >> 3) - ((int32_t)dev->calib.par_t1 << 1);
	var2 = (var1 * (int32_t)dev->calib.par_t2) >> 11;
	var3 = ((var1 >> 1) * (var1 >> 1)) >> 12;
	var3 = ((var3) * ((int32_t)dev->calib.par_t3 << 4)) >> 14;
	dev->calib.t_fine = (int32_t)(var2 + var3);
	calc_temp = (int16_t)(((dev->calib.t_fine * 5) + 128) >> 8);
	/*lint -restore */

	return calc_temp;
}

/* This internal API is used to calculate the pressure in integer */
static uint32_t calc_pressure(uint32_t pres_adc, const struct bme680_dev *dev)
{
	int32_t var1;
	int32_t var2;
	int32_t var3;
	int32_t pressure_comp;
	/*! This value is used to check precedence to multiplication or division
	 * in the pressure compensation equation to achieve least loss of precision and
	 * avoiding overflows.
	 * i.e Comparing value, pres_ovf_check = (1 << 31) >> 1
	 */
	const int32_t pres_ovf_check = INT32_C(0x40000000);

	/*lint -save -e701 -e702 -e713 */
	var1 = (((int32_t)dev->calib.t_fine) >> 1) - 64000;
	var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (int32_t)dev->calib.par_p6) >> 2;
	var2 = var2 + ((var1 * (int32_t)dev->calib.par_p5) << 1);
	var2 = (var2 >> 2) + ((int32_t)dev->calib.par_p4 << 16);
	var1 = (((((var1 >> 2) * (var1 >> 2)) >> 13) * ((int32_t)dev->calib.par_p3 << 5)) >> 3)
	+ (((int32_t)dev->calib.par_p2 * var1) >> 1);
	var1 = var1 >> 18;
	var1 = ((32768 + var1) * (int32_t)dev->calib.par_p1) >> 15;
	pressure_comp = 1048576 - pres_adc;
	pressure_comp = (int32_t)((pressure_comp - (var2 >> 12)) * ((uint32_t)3125));
	if (pressure_comp >= pres_ovf_check)
		pressure_comp = ((pressure_comp / var1) << 1);
	else
		pressure_comp = ((pressure_comp << 1) / var1);
	var1 = ((int32_t)dev->calib.par_p9 * (int32_t)(((pressure_comp >> 3) * (pressure_comp >> 3)) >> 13)) >> 12;
	var2 = ((int32_t)(pressure_comp >> 2) * (int32_t)dev->calib.par_p8) >> 13;
	var3 = ((int32_t)(pressure_comp >> 8) * (int32_t)(pressure_comp >> 8) *
			(int32_t)(pressure_comp >> 8) * (int32_t)dev->calib.par_p10) >> 17;

	pressure_comp = (int32_t)(pressure_comp) + ((var1 + var2 + var3 + ((int32_t)dev->calib.par_p7 << 7)) >> 4);
	/*lint -restore */

	return (uint32_t)pressure_comp;
}

/* This internal API is used to calculate the humidity in integer */
static uint32_t calc_humidity(uint16_t hum_adc, const struct bme680_dev *dev)
{
	int32_t var1;
	int32_t var2;
	int32_t var3;
	int32_t var4;
	int32_t var5;
	int32_t var6;
	int32_t temp_scaled;
	int32_t calc_hum;

	/*lint -save -e702 -e704 */
	temp_scaled = (((int32_t)dev->calib.t_fine * 5) + 128) >> 8;
	var1 = (int32_t)(hum_adc - ((int32_t)((int32_t)dev->calib.par_h1 * 16)))
	- (((temp_scaled * (int32_t)dev->calib.par_h3) / ((int32_t)100)) >> 1);
	var2 = ((int32_t)dev->calib.par_h2
			* (((temp_scaled * (int32_t)dev->calib.par_h4) / ((int32_t)100))
				+ (((temp_scaled * ((temp_scaled * (int32_t)dev->calib.par_h5) / ((int32_t)100))) >> 6)
					/ ((int32_t)100)) + (int32_t)(1 << 14))) >> 10;
	var3 = var1 * var2;
	var4 = (int32_t)dev->calib.par_h6 << 7;
	var4 = ((var4) + ((temp_scaled * (int32_t)dev->calib.par_h7) / ((int32_t)100))) >> 4;
	var5 = ((var3 >> 14) * (var3 >> 14)) >> 10;
	var6 = (var4 * var5) >> 1;
	calc_hum = (((var3 + var6) >> 10) * ((int32_t)1000)) >> 12;

	if (calc_hum > 100000) /* Cap at 100%rH */
		calc_hum = 100000;
	else if (calc_hum < 0)
		calc_hum = 0;
	/*lint -restore */

	return (uint32_t)calc_hum;
}

/* This internal API is used to calculate the gas resistance */
static uint32_t calc_gas_resistance_low(uint16_t gas_res_adc, uint8_t gas_range, const struct bme680_dev *dev)
{
	int64_t var1;
	uint64_t var2;
	int64_t var3;
	uint32_t calc_gas_res;
	uint32_t lookup_table1[16] = {UINT32_C(2147483647), UINT32_C(2147483647), UINT32_C(2147483647),
		UINT32_C(2147483647), UINT32_C(2147483647), UINT32_C(2126008810), UINT32_C(2147483647),
		UINT32_C(2130303777), UINT32_C(2147483647), UINT32_C(2147483647), UINT32_C(2143188679),
		UINT32_C(2136746228), UINT32_C(2147483647), UINT32_C(2126008810), UINT32_C(2147483647),
		UINT32_C(2147483647)};
	uint32_t lookup_table2[16] = {UINT32_C(4096000000), UINT32_C(2048000000), UINT32_C(1024000000),
		UINT32_C(512000000), UINT32_C(255744255), UINT32_C(127110228), UINT32_C(64000000),
		UINT32_C(32258064), UINT32_C(16016016), UINT32_C(8000000), UINT32_C(4000000),
		UINT32_C(2000000), UINT32_C(1000000), UINT32_C(500000), UINT32_C(250000), UINT32_C(125000)};

	/*lint -save -e704 */
	var1 = (int64_t)((1340 + (5 * (int64_t)dev->calib.range_sw_err)) *
			((int64_t)lookup_table1[gas_range])) >> 16;
	var2 = (((int64_t)((int64_t)gas_res_adc << 15) - (int64_t)(16777216)) + var1);
	var3 = (((int64_t)lookup_table2[gas_range] * (int64_t)var1) >> 9);
	calc_gas_res = (uint32_t)((var3 + ((int64_t)var2 >> 1)) / (int64_t)var2);
	/*lint -restore */

	return calc_gas_res;
}

/* This internal API is used to calculate the gas resistance */
static uint32_t calc_gas_resistance_high(uint16_t gas_res_adc, uint8_t gas_range, const struct bme680_dev *dev)
{
	uint32_t calc_gas_res;
	uint32_t var1 = UINT32_C(262144) >> gas_range;
	int32_t var2 = (int32_t)gas_res_adc - INT32_C(512);
	var2 *= INT32_C(3);
	var2 = INT32_C(4096) + var2;
	
	calc_gas_res = UINT32_C(1000000)* var1 / (uint32_t)var2;
	
	return calc_gas_res;
}

/* This internal API is used to calculate the heater resistance using integer */
static uint8_t calc_res_heat(uint16_t temp, const struct bme680_dev *dev)
{
	uint8_t heatr_res;
	int32_t var1;
	int32_t var2;
	int32_t var3;
	int32_t var4;
	int32_t var5;
	int32_t heatr_res_x100;

	if (temp > 400) /* Cap temperature */
		temp = 400;

	var1 = (((int32_t)dev->amb_temp * dev->calib.par_gh3) / 1000) * 256;
	var2 = (dev->calib.par_gh1 + 784) * (((((dev->calib.par_gh2 + 154009) * temp * 5) / 100) + 3276800) / 10);
	var3 = var1 + (var2 / 2);
	var4 = (var3 / (dev->calib.res_heat_range + 4));
	var5 = (131 * dev->calib.res_heat_val) + 65536;
	heatr_res_x100 = (int32_t)(((var4 / var5) - 250) * 34);
	heatr_res = (uint8_t)((heatr_res_x100 + 50) / 100);

	return heatr_res;
}

#else

/* This internal API is used to calculate the temperature value in float */
static float calc_temperature(uint32_t temp_adc, struct bme680_dev *dev)
{
	float var1;
	float var2;
	float calc_temp;

	/* calculate var1 data */
	var1 = ((((float)temp_adc / 16384.0f) - ((float)dev->calib.par_t1 / 1024.0f)) * ((float)dev->calib.par_t2));

	/* calculate var2 data */
	var2 = (((((float)temp_adc / 131072.0f) - ((float)dev->calib.par_t1 / 8192.0f))
		* (((float)temp_adc / 131072.0f) - ((float)dev->calib.par_t1 / 8192.0f)))
		* ((float)dev->calib.par_t3 * 16.0f));

	/* t_fine value*/
	dev->calib.t_fine = (var1 + var2);

	/* compensated temperature data*/
	calc_temp = ((dev->calib.t_fine) / 5120.0f);

	return calc_temp;
}

/* This internal API is used to calculate the pressure value in float */
static float calc_pressure(uint32_t pres_adc, const struct bme680_dev *dev)
{
	float var1;
	float var2;
	float var3;
	float calc_pres;

	var1 = (((float)dev->calib.t_fine / 2.0f) - 64000.0f);
	var2 = var1 * var1 * (((float)dev->calib.par_p6) / (131072.0f));
	var2 = var2 + (var1 * ((float)dev->calib.par_p5) * 2.0f);
	var2 = (var2 / 4.0f) + (((float)dev->calib.par_p4) * 65536.0f);
	var1 = (((((float)dev->calib.par_p3 * var1 * var1) / 16384.0f) + ((float)dev->calib.par_p2 * var1))
		/ 524288.0f);
	var1 = ((1.0f + (var1 / 32768.0f)) * ((float)dev->calib.par_p1));
	calc_pres = (1048576.0f - ((float)pres_adc));

	/* Avoid exception caused by division by zero */
	if ((int)var1 != 0) {
		calc_pres = (((calc_pres - (var2 / 4096.0f)) * 6250.0f) / var1);
		var1 = (((float)dev->calib.par_p9) * calc_pres * calc_pres) / 2147483648.0f;
		var2 = calc_pres * (((float)dev->calib.par_p8) / 32768.0f);
		var3 = ((calc_pres / 256.0f) * (calc_pres / 256.0f) * (calc_pres / 256.0f)
			* (dev->calib.par_p10 / 131072.0f));
		calc_pres = (calc_pres + (var1 + var2 + var3 + ((float)dev->calib.par_p7 * 128.0f)) / 16.0f);
	} else {
		calc_pres = 0;
	}

	return calc_pres;
}

/* This internal API is used to calculate the humidity value in float */
static float calc_humidity(uint16_t hum_adc, const struct bme680_dev *dev)
{
	float calc_hum;
	float var1;
	float var2;
	float var3;
	float var4;
	float temp_comp;

	/* compensated temperature data*/
	temp_comp = ((dev->calib.t_fine) / 5120.0f);

	var1 = (float)((float)hum_adc)
		- (((float)dev->calib.par_h1 * 16.0f) + (((float)dev->calib.par_h3 / 2.0f)
			* temp_comp));

	var2 = var1
		* ((float)(((float)dev->calib.par_h2 / 262144.0f)
			* (1.0f + (((float)dev->calib.par_h4 / 16384.0f) * temp_comp)
				+ (((float)dev->calib.par_h5 / 1048576.0f) * temp_comp * temp_comp))));
	var3 = (float)dev->calib.par_h6 / 16384.0f;
	var4 = (float)dev->calib.par_h7 / 2097152.0f;

	calc_hum = var2 + ((var3 + (var4 * temp_comp)) * var2 * var2);

	if (calc_hum > 100.0f)
		calc_hum = 100.0f;
	else if (calc_hum < 0.0f)
		calc_hum = 0.0f;

	return calc_hum;
}

/* This internal API is used to calculate the gas resistance value in float */
static float calc_gas_resistance_low(uint16_t gas_res_adc, uint8_t gas_range, const struct bme680_dev *dev)
{
	float calc_gas_res;
	float var1;
	float var2;
	float var3;
	float gas_res_f = gas_res_adc;
	float gas_range_f = (1U << gas_range); /*lint !e790 / Suspicious truncation, integral to float */

	const float lookup_k1_range[16] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, -0.8f, 0.0f, 0.0f, -0.2f,
			-0.5f, 0.0f, -1.0f, 0.0f, 0.0f };
	const float lookup_k2_range[16] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.7f, 0.0f, -0.8f, -0.1f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	var1 = (1340.0f + (5.0f * dev->calib.range_sw_err));
	var2 = (var1) * (1.0f + lookup_k1_range[gas_range] / 100.0f);
	var3 = 1.0f + (lookup_k2_range[gas_range] / 100.0f);

	calc_gas_res = 1.0f / (float)(var3 * (0.000000125f) * gas_range_f
		* (((gas_res_f - 512.0f) / var2) + 1.0f));

	return calc_gas_res;
}

/* This internal API is used to calculate the gas resistance value in float */
static float calc_gas_resistance_high(uint16_t gas_res_adc, uint8_t gas_range, const struct bme680_dev *dev)
{
	float calc_gas_res;
	uint32_t var1 = UINT32_C(262144) >> gas_range;
	int32_t var2 = (int32_t)gas_res_adc - INT32_C(512);
	var2 *= INT32_C(3);
	var2 = INT32_C(4096) + var2;
	
	calc_gas_res = 1000000.0f * (float)var1 / (float)var2;
	
	return calc_gas_res;
}

/* This internal API is used to calculate the gas resistance value using float */
static uint8_t calc_res_heat(uint16_t temp, const struct bme680_dev *dev)
{
	float var1;
	float var2;
	float var3;
	float var4;
	float var5;
	uint8_t res_heat;

	if (temp > 400) /* Cap temperature */
		temp = 400;

	var1 = (((float)dev->calib.par_gh1 / (16.0f)) + 49.0f);
	var2 = ((((float)dev->calib.par_gh2 / (32768.0f)) * (0.0005f)) + 0.00235f);
	var3 = ((float)dev->calib.par_gh3 / (1024.0f));
	var4 = (var1 * (1.0f + (var2 * (float)temp)));
	var5 = (var4 + (var3 * (float)dev->amb_temp));
	res_heat = (uint8_t)(3.4f * ((var5 * (4 / (4 + (float)dev->calib.res_heat_range))
			* (1 / (1 + ((float)dev->calib.res_heat_val * 0.002f)))) - 25));

	return res_heat;
}

#endif

/* This internal API is used to calculate the gas wait */
static uint8_t calc_gas_wait(uint16_t dur)
{
	uint8_t factor = 0;
	uint8_t durval;

	if (dur >= 0xfc0) {
		durval = 0xff; /* Max duration*/
	} else {
		while (dur > 0x3F) {
			dur = dur / 4;
			factor += 1;
		}
		durval = (uint8_t)(dur + (factor * 64));
	}

	return durval;
}

/* This internal API is used to calculate the field data of sensor */
static int8_t read_field_data(uint8_t index, struct bme680_data *data, struct bme680_dev *dev)
{
	int8_t rslt = BME680_OK;
	uint8_t buff[LEN_FIELD] = { 0 };
	uint8_t gas_range_l, gas_range_h;
	uint32_t adc_temp;
	uint32_t adc_pres;
	uint16_t adc_hum;
	uint16_t adc_gas_res_low, adc_gas_res_high;

	rslt = bme680_get_regs(((uint8_t)(BME680_REG_FIELD0 + (index * LEN_FIELD_OFFSET))),
			buff, (uint16_t)LEN_FIELD, dev);

	if (!data) {
		rslt = BME680_E_NULL_PTR;
		return rslt;
	}

	data->status = buff[0] & BME680_NEW_DATA_MSK;
	data->gas_index = buff[0] & BME680_GAS_INDEX_MSK;
	data->meas_index = buff[1];

	/* read the raw data from the sensor */
	adc_pres = (uint32_t)(((uint32_t)buff[2] * 4096) | ((uint32_t)buff[3] * 16) | ((uint32_t)buff[4] / 16));
	adc_temp = (uint32_t)(((uint32_t)buff[5] * 4096) | ((uint32_t)buff[6] * 16) | ((uint32_t)buff[7] / 16));
	adc_hum = (uint16_t)(((uint32_t)buff[8] * 256) | (uint32_t)buff[9]);
	adc_gas_res_low = (uint16_t)((uint32_t)buff[13] * 4 | (((uint32_t)buff[14]) / 64));
	adc_gas_res_high = (uint16_t)((uint32_t)buff[15] * 4 | (((uint32_t)buff[16]) / 64));
	gas_range_l = buff[14] & BME680_GAS_RANGE_MSK;
	gas_range_h = buff[16] & BME680_GAS_RANGE_MSK;

	if(dev->chip_id == BME680_CHIP_ID) {
		data->status |= buff[14] & BME680_GASM_VALID_MSK;
		data->status |= buff[14] & BME680_HEAT_STAB_MSK;
	}
	else {
		data->status |= buff[16] & BME680_GASM_VALID_MSK;
		data->status |= buff[16] & BME680_HEAT_STAB_MSK;
	}

	if ((data->status & BME680_NEW_DATA_MSK) && (rslt == BME680_OK)) {
		rslt = bme680_get_regs(BME680_REG_RES_HEAT0 + data->gas_index, &data->res_heat, 1, dev);
		if (rslt == BME680_OK)
			rslt = bme680_get_regs(BME680_REG_IDAC_HEAT0 + data->gas_index, &data->idac, 1, dev);
		if (rslt == BME680_OK)
			rslt = bme680_get_regs(BME680_REG_GAS_WAIT0 + data->gas_index, &data->gas_wait, 1, dev);

		if (rslt == BME680_OK) {
			data->temperature = calc_temperature(adc_temp, dev);
			data->pressure = calc_pressure(adc_pres, dev);
			data->humidity = calc_humidity(adc_hum, dev);
			if(dev->chip_id == BME680_CHIP_ID)
				data->gas_resistance = calc_gas_resistance_low(adc_gas_res_low, gas_range_l, dev);
			else
				data->gas_resistance = calc_gas_resistance_high(adc_gas_res_high, gas_range_h, dev);
		}
	}

	return rslt;
}

/* This internal API is used to switch between SPI memory pages */
static int8_t set_mem_page(uint8_t reg_addr, struct bme680_dev *dev)
{
	int8_t rslt;
	uint8_t reg;
	uint8_t mem_page;

	/* Check for null pointers in the device structure*/
	rslt = null_ptr_check(dev);
	if (rslt == BME680_OK) {
		if (reg_addr > 0x7f)
			mem_page = BME680_MEM_PAGE1;
		else
			mem_page = BME680_MEM_PAGE0;

		if (mem_page != dev->mem_page) {
			dev->mem_page = mem_page;

			dev->com_rslt = dev->read(dev->dev_id, BME680_REG_MEM_PAGE | BME680_SPI_RD_MSK, &reg, 1);
			if (dev->com_rslt != 0)
				rslt = BME680_E_COM_FAIL;

			if (rslt == BME680_OK) {
				reg = reg & (~BME680_MEM_PAGE_MSK);
				reg = reg | (dev->mem_page & BME680_MEM_PAGE_MSK);

				dev->com_rslt = dev->write(dev->dev_id, BME680_REG_MEM_PAGE & BME680_SPI_WR_MSK, &reg,
						1);
				if (dev->com_rslt != 0)
					rslt = BME680_E_COM_FAIL;
			}
		}
	}

	return rslt;
}

/* This internal API is used to get the current SPI memory page */
static int8_t get_mem_page(struct bme680_dev *dev)
{
	int8_t rslt;
	uint8_t reg;

	/* Check for null pointer in the device structure*/
	rslt = null_ptr_check(dev);
	if (rslt == BME680_OK) {
		dev->com_rslt = dev->read(dev->dev_id, BME680_REG_MEM_PAGE | BME680_SPI_RD_MSK, &reg, 1);
		if (dev->com_rslt != 0)
			rslt = BME680_E_COM_FAIL;
		else
			dev->mem_page = reg & BME680_MEM_PAGE_MSK;
	}

	return rslt;
}

/* This internal API is used to limit the max value of a parameter */
static int8_t boundary_check(uint8_t *value, uint8_t max, struct bme680_dev *dev)
{
	int8_t rslt;

	rslt = null_ptr_check(dev);

	if ((value != NULL) && (rslt == BME680_OK)) {
		/* Check if value is above maximum value */
		if (*value > max) {
			/* Auto correct the invalid value to maximum value */
			*value = max;
			dev->info_msg |= BME680_I_PARAM_CORR;
		}
	} else {
		rslt = BME680_E_NULL_PTR;
	}

	return rslt;
}

/* This internal API is used to check the bme680_dev for null pointers */
static int8_t null_ptr_check(const struct bme680_dev *dev)
{
	int8_t rslt = BME680_OK;

	if ((dev == NULL) || (dev->read == NULL) || (dev->write == NULL) || (dev->delay_ms == NULL)) {
		/* Device structure pointer is not valid */
		rslt = BME680_E_NULL_PTR;
	}

	return rslt;
}

/* This internal API is used to calculate the register value for
 * shared heater duration */
static uint8_t calc_heatr_dur_shared(uint16_t dur)
{
	uint8_t factor = 0;
	uint8_t heatdurval;

	if (dur >= 0x783) {
		heatdurval = 0xff; /* Max duration */
	} else {
		/* Step size of 0.477ms */
		dur = (uint16_t)(((uint32_t)dur * 1000) / 477);
		while (dur > 0x3F) {
			dur = dur >> 2;
			factor += 1;
		}
		heatdurval = (uint8_t)(dur + (factor * 64));
	}

	return heatdurval;
}

/* This internal API is used sort the sensor data */
static void sort_sensor_data(uint8_t low_index, uint8_t high_index, struct bme680_data *field[])
{
	int16_t meas_index1;
	int16_t meas_index2;

	meas_index1 = (int16_t)field[low_index]->meas_index;
	meas_index2 = (int16_t)field[high_index]->meas_index;

	if ((field[low_index]->status & BME680_NEW_DATA_MSK) && (field[high_index]->status & BME680_NEW_DATA_MSK)) {
		int16_t diff = meas_index2 - meas_index1;

		if (((diff > -3) && (diff < 0)) || (diff > 2))
			swap_fields(low_index, high_index, field);

	} else if (field[high_index]->status & BME680_NEW_DATA_MSK) {
		swap_fields(low_index, high_index, field);
	}
	/* Sorting field data
	 *
	 * The 3 fields are filled in a fixed order with data in an incrementing
	 * 8-bit sub-measurement index which looks like
	 * Field index | Sub-meas index
	 *      0      |        0
	 *      1      |        1
	 *      2      |        2
	 *      0      |        3
	 *      1      |        4
	 *      2      |        5
	 *      ...
	 *      0      |        252
	 *      1      |        253
	 *      2      |        254
	 *      0      |        255
	 *      1      |        0
	 *      2      |        1
	 *
	 * The fields are sorted in a way so as to always deal with only a snapshot
	 * of comparing 2 fields at a time. The order being
	 * field0 & field1
	 * field0 & field2
	 * field1 & field2
	 * Here the oldest data should be in field0 while the newest is in field2.
	 * In the following documentation, field0's position would referred to as
	 * the lowest and field2 as the highest.

	 * In order to sort we have to consider the following cases,
	 *
	 * Case A: No fields have new data
	 *     Then do not sort, as this data has already been read.
	 *
	 * Case B: Higher field has new data
	 *     Then the new field get's the lowest position.
	 *
	 * Case C: Both fields have new data
	 *     We have to put the oldest sample in the lowest position. Since the
	 *     sub-meas index contains in essence the age of the sample, we calculate
	 *     the difference between the higher field and the lower field.
	 *     Here we have 3 sub-cases,
	 *     Case 1: Regular read without overwrite
	 *         Field index | Sub-meas index
	 *              0      |        3
	 *              1      |        4
	 *
	 *         Field index | Sub-meas index
	 *              0      |        3
	 *              2      |        5
	 *
	 *         The difference is always <= 2. There is no need to swap as the
	 *         oldest sample is already in the lowest position.
	 *
	 *     Case 2: Regular read with an overflow and without an overwrite
	 *         Field index | Sub-meas index
	 *              0      |        255
	 *              1      |        0
	 *
	 *         Field index | Sub-meas index
	 *              0      |        254
	 *              2      |        0
	 *
	 *         The difference is always <= -3. There is no need to swap as the
	 *         oldest sample is already in the lowest position.
	 *
	 *     Case 3: Regular read with overwrite
	 *         Field index | Sub-meas index
	 *              0      |        6
	 *              1      |        4
	 *
	 *         Field index | Sub-meas index
	 *              0      |        6
	 *              2      |        5
	 *
	 *         The difference is always > -3. There is a need to swap as the
	 *         oldest sample is not in the lowest position.
	 *
	 *     Case 4: Regular read with overwrite and overflow
	 *         Field index | Sub-meas index
	 *              0      |        0
	 *              1      |        254
	 *
	 *         Field index | Sub-meas index
	 *              0      |        0
	 *              2      |        255
	 *
	 *         The difference is always > 2. There is a need to swap as the
	 *         oldest sample is not in the lowest position.
	 *
	 * To summarize, we have to swap when
	 *     - The higher field has new data and the lower field does not.
	 *     - If both fields have new data, then the difference of sub-meas index
	 *         between the higher field and the lower field creates the
	 *         following condition for swapping.
	 *         - (diff > -3) && (diff < 0), combination of cases 1, 2, and 3.
	 *         - diff > 2, case 4.
	 *
	 *     Here the limits of -3 and 2 derive from the fact that there are 3 fields.
	 *     These values decrease or increase respectively if the number of fields increases.
	 */
}

/* This internal API is used to swap two fields */
static void swap_fields(uint8_t index1, uint8_t index2, struct bme680_data *field[])
{
	struct bme680_data *temp;

	temp = field[index1];
	field[index1] = field[index2];
	field[index2] = temp;
}

