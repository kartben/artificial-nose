/**
 * Copyright (c) 2020 Bosch Sensortec GmbH. All rights reserved.
 *
 * BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file	bsec.cpp
 * @date	23 April 2021
 * @version	1.5.0
 *
 */

#include "bsec.h"

TwoWire* Bsec::wireObj = NULL;
SPIClass* Bsec::spiObj = NULL;

/**
 * @brief Constructor
 */
Bsec::Bsec(BsecCallback proc)
{
	_timer_overflow_counter = 0;
	_timer_last_value = 0;
	_step = CONTROL_STEP;
	_bme68x_status = BME68X_OK;
	_bsec_status = BSEC_OK;
	_temp_offset = 5.0f;
	_proc = proc;

	memset(&_bme68x, 0, sizeof(_bme68x));
	memset(&_bsec_version, 0, sizeof(_bsec_version));
	memset(&_bsec_bme_settings, 0, sizeof(_bsec_bme_settings));
	memset(&_output, 0, sizeof(_output));
}

/**
 * @brief Function to initialize the BSEC library and the BME68X sensor
 */
bool Bsec::begin(void *intf_ptr, enum bme68x_intf intf, bme68x_read_fptr_t read, bme68x_write_fptr_t write, bme68x_delay_us_fptr_t idleTask)
{
	_bme68x.intf_ptr = intf_ptr;
	_bme68x.intf = intf;
	_bme68x.read = read;
	_bme68x.write = write;
	_bme68x.delay_us = idleTask;
	_bme68x.amb_temp = 25;

	return beginCommon();
}

/**
 * @brief Function to initialize the BSEC library and the BME68X sensor with i2C communication setting
 */
bool Bsec::begin(uint8_t i2cAddr, TwoWire &i2c)
{
	_bme68x.intf_ptr = (void*)(intptr_t)i2cAddr;
	_bme68x.intf = BME68X_I2C_INTF;
	_bme68x.read = Bsec::i2cRead;
	_bme68x.write = Bsec::i2cWrite;
	_bme68x.delay_us = Bsec::delay_us;
	_bme68x.amb_temp = 25;	

	Bsec::wireObj = &i2c;
	Bsec::wireObj->begin();

	return beginCommon();
}

/**
 * @brief Function to initialize the BSEC library and the BME68X sensor with SPI communication setting
 */
bool Bsec::begin(uint8_t chipSelect, SPIClass &spi)
{
	_bme68x.intf_ptr = (void*)(intptr_t)chipSelect;
	_bme68x.intf = BME68X_SPI_INTF;
	_bme68x.read = Bsec::spiRead;
	_bme68x.write = Bsec::spiWrite;
	_bme68x.delay_us = Bsec::delay_us;
	_bme68x.amb_temp = 25;	

	pinMode(chipSelect, OUTPUT);
	digitalWrite(chipSelect, HIGH);
	Bsec::spiObj = &spi;
	Bsec::spiObj->begin();

	return beginCommon();
}

/**
 * @brief Common code for the begin function
 */
bool Bsec::beginCommon()
{
	_bsec_status = bsec_init();
	if (_bsec_status < BSEC_OK)
		return false;

	_bsec_status = bsec_get_version(&_bsec_version);
	if (_bsec_status < BSEC_OK)
		return false;

	memset(&_bsec_bme_settings, 0, sizeof(_bsec_bme_settings));
	memset(&_output, 0, sizeof(_output));
	_step = CONTROL_STEP;

	last_meas_index = 0;

	return true;
}

/**
 * @brief Function that sets the desired sensors and the sample rates (by subscription)
 */
bool Bsec::updateSubscription(bsec_virtual_sensor_t sensorList[], uint8_t nSensors, float sampleRate)
{
	bsec_sensor_configuration_t virtualSensors[BSEC_NUMBER_OUTPUTS], sensorSettings[BSEC_MAX_PHYSICAL_SENSOR];
	uint8_t nSensorSettings = BSEC_MAX_PHYSICAL_SENSOR;

	for (uint8_t i = 0; i < nSensors; i++) {
		virtualSensors[i].sensor_id = sensorList[i];
		virtualSensors[i].sample_rate = sampleRate;
	}

	_bsec_status = bsec_update_subscription(virtualSensors, nSensors, sensorSettings, &nSensorSettings);
	if (_bsec_status < BSEC_OK)
		return false;

	_bme68x_status = bme68x_init(&_bme68x);
	if (_bme68x_status < BME68X_OK)
		return false;	
	return true;
}


/**
 * @brief Callback from the user to read data from the BME68X using parallel mode/forced mode, process and store outputs
 */
bool Bsec::run(void)
{
	uint8_t n_fields = 0;
	int64_t currTimeNs = getTimeMs() * INT64_C(1000000);
	_op_mode = _bsec_bme_settings.op_mode;

	if (currTimeNs >=  _bsec_bme_settings.next_call)
	{
		_bsec_status = bsec_sensor_control(currTimeNs, &_bsec_bme_settings);
		if (_bsec_status < BSEC_OK)
			return false;

		switch(_bsec_bme_settings.op_mode) {
		case BME68X_FORCED_MODE:
			setBme68xConfigForced();
			break;
		case BME68X_PARALLEL_MODE:
			if (_op_mode != _bsec_bme_settings.op_mode)
			{
				setBme68xConfigParallel();
			}
			break;

		case BME68X_SLEEP_MODE:
			if (_op_mode != _bsec_bme_settings.op_mode)
			{
				setBme68xConfigSleep();
			}
			break;
		}

		if (_bme68x_status < BME68X_OK)
			return false;

		if(_bsec_bme_settings.trigger_measurement && _bsec_bme_settings.op_mode != BME68X_SLEEP_MODE)
		{
			_bme68x_status = bme68x_get_data(_op_mode, _bme68x_data, &n_fields, &_bme68x);
			for(uint8_t i = 0; i < n_fields; i++)
			{
				if (_bme68x_data[i].status & BME68X_GASM_VALID_MSK)
				{
					/* Measurement index check to track the first valid sample after operation mode change */
					if(check_meas_idx == true )
					{
						/* After changing the operation mode, Measurement index expected to be zero
						 * however with considering the data miss case as well, condition shall be checked less
						 * than last received measurement index */
						if(last_meas_index == 0 || _bme68x_data[i].meas_index == 0 ||  _bme68x_data[i].meas_index < last_meas_index)
						{
							check_meas_idx = false;
						}
						else
						{
							continue; // Skip the invalid data samples or  data from last duty cycle scan
						}
					}

					last_meas_index = _bme68x_data[i].meas_index;

					if(!processData(currTimeNs, _bme68x_data[i]))
					{
						return false;
					}
				}
			}
		}

	}
	return true;
}

/**
 * @brief Function to get the state of the algorithm to save to non-volatile memory
 */
bool Bsec::getState(uint8_t *state)
{
	uint8_t workBuffer[BSEC_MAX_STATE_BLOB_SIZE];
	uint32_t n_serialized_state = BSEC_MAX_STATE_BLOB_SIZE;

	_bsec_status = bsec_get_state(0, state, BSEC_MAX_STATE_BLOB_SIZE, workBuffer, BSEC_MAX_STATE_BLOB_SIZE, &n_serialized_state);
	if (_bsec_status < BSEC_OK)
		return false;
	return true;
}

/**
 * @brief Function to set the state of the algorithm from non-volatile memory
 */
bool Bsec::setState(uint8_t *state)
{
	uint8_t workBuffer[BSEC_MAX_STATE_BLOB_SIZE];

	_bsec_status = bsec_set_state(state, BSEC_MAX_STATE_BLOB_SIZE, workBuffer, BSEC_MAX_STATE_BLOB_SIZE);
	if (_bsec_status < BSEC_OK)
		return false;

	memset(&_bsec_bme_settings, 0, sizeof(_bsec_bme_settings));
	_step = CONTROL_STEP;

	return true;
}

/**
 * @brief Function to set the configuration of the algorithm from memory
 */
bool Bsec::setConfig(const uint8_t *state)
{
	uint8_t workBuffer[BSEC_MAX_PROPERTY_BLOB_SIZE];

	_bsec_status = bsec_set_configuration(state, BSEC_MAX_PROPERTY_BLOB_SIZE, workBuffer, BSEC_MAX_PROPERTY_BLOB_SIZE);
	if (_bsec_status < BSEC_OK)
		return false;

	memset(&_bsec_bme_settings, 0, sizeof(_bsec_bme_settings));
	_step = CONTROL_STEP;

	return true;
}

/* Private functions */

/**
 * @brief Read data from the BME68X and process it
 */
bool Bsec::processData(int64_t currTimeNs, const bme68x_data& data)
{
	bsec_input_t inputs[BSEC_MAX_PHYSICAL_SENSOR]; // Temp, Pres, Hum & Gas
	uint8_t nInputs = 0;

	if (CHECK_BSEC_INPUT(_bsec_bme_settings.process_data, BSEC_INPUT_HEATSOURCE)) {
		inputs[nInputs].sensor_id = BSEC_INPUT_HEATSOURCE;
		inputs[nInputs].signal = _temp_offset;
		inputs[nInputs].time_stamp = currTimeNs;
		nInputs++;
	}
	if (CHECK_BSEC_INPUT(_bsec_bme_settings.process_data, BSEC_INPUT_TEMPERATURE)) {
		inputs[nInputs].sensor_id = BSEC_INPUT_TEMPERATURE;
		inputs[nInputs].signal = data.temperature;
		inputs[nInputs].time_stamp = currTimeNs;
		nInputs++;
	}
	if (CHECK_BSEC_INPUT(_bsec_bme_settings.process_data, BSEC_INPUT_HUMIDITY)) {
		inputs[nInputs].sensor_id = BSEC_INPUT_HUMIDITY;
		inputs[nInputs].signal = data.humidity;
		inputs[nInputs].time_stamp = currTimeNs;
		nInputs++;
	}
	if (CHECK_BSEC_INPUT(_bsec_bme_settings.process_data, BSEC_INPUT_PRESSURE)) {
		inputs[nInputs].sensor_id = BSEC_INPUT_PRESSURE;
		inputs[nInputs].signal = data.pressure;
		inputs[nInputs].time_stamp = currTimeNs;
		nInputs++;
	}
	if (CHECK_BSEC_INPUT(_bsec_bme_settings.process_data, BSEC_INPUT_GASRESISTOR)) {
		inputs[nInputs].sensor_id = BSEC_INPUT_GASRESISTOR;
		inputs[nInputs].signal = data.gas_resistance;
		inputs[nInputs].time_stamp = currTimeNs;
		nInputs++;
	}
	if (CHECK_BSEC_INPUT(_bsec_bme_settings.process_data, BSEC_INPUT_PROFILE_PART)) {
		inputs[nInputs].sensor_id = BSEC_INPUT_PROFILE_PART;
		inputs[nInputs].signal = (_op_mode == BME68X_FORCED_MODE) ? 0 : data.gas_index;
		inputs[nInputs].time_stamp = currTimeNs;
		nInputs++;
	}

	if (nInputs > 0) {
		_output.len = BSEC_NUMBER_OUTPUTS;
		memset(_output.outputs, 0, sizeof(_output.outputs));

		_bsec_status = bsec_do_steps(inputs, nInputs, _output.outputs, &_output.len);

		if (_bsec_status < BSEC_OK)
			return false;
	}

	if (_proc != NULL)
		_proc(data, _output);
	return true;
}

/**
 * @brief Set the BME68X sensor configuration to forced mode
 */
void Bsec::setBme68xConfigForced(void)
{
	/* Set the filter, odr, temperature, pressure and humidity settings */
	_bme68x_conf.filter = BME68X_FILTER_OFF;
	_bme68x_conf.odr = BME68X_ODR_NONE;
	_bme68x_conf.os_hum = _bsec_bme_settings.humidity_oversampling;
	_bme68x_conf.os_pres = _bsec_bme_settings.pressure_oversampling;
	_bme68x_conf.os_temp = _bsec_bme_settings.temperature_oversampling;

	_bme68x_status = bme68x_set_conf(&_bme68x_conf, &_bme68x);
	if (_bme68x_status < BME68X_OK)
		return;

	/* Set the gas sensor settings and link the heating profile */
	_bme68x_heatr_conf.enable = _bsec_bme_settings.run_gas;
	_bme68x_heatr_conf.shared_heatr_dur = 0;
	_bme68x_heatr_conf.heatr_temp = _bsec_bme_settings.heater_temperature;
	_bme68x_heatr_conf.heatr_dur = _bsec_bme_settings.heater_duration;
	_bme68x_heatr_conf.heatr_temp_prof = nullptr;
	_bme68x_heatr_conf.heatr_dur_prof = nullptr;
	_bme68x_heatr_conf.profile_len = 0;

	/* Select the power mode */
	_bme68x_status = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &_bme68x_heatr_conf, &_bme68x);
	if (_bme68x_status < BME68X_OK)
		return;

	_bme68x_status = bme68x_set_op_mode(BME68X_FORCED_MODE, &_bme68x);
	if (_bme68x_status < BME68X_OK)
		return;

	_op_mode = BME68X_FORCED_MODE;
}

/**
 * @brief Set the BME68X sensor configuration to sleep mode
 */
void Bsec::setBme68xConfigSleep(void)
{
	_bme68x_status = bme68x_set_op_mode(BME68X_SLEEP_MODE, &_bme68x);
	if (_bme68x_status < BME68X_OK)
		return;

	_op_mode = BME68X_SLEEP_MODE;
}

/**
 * @brief Set the BME68X sensor configuration to parallel mode
 */
void Bsec::setBme68xConfigParallel(void)
{
	uint8_t n_fields = 0;
	/* Set the filter, odr, temperature, pressure and humidity settings */
	_bme68x_conf.filter = BME68X_FILTER_OFF;
	_bme68x_conf.odr = BME68X_ODR_NONE;
	_bme68x_conf.os_hum = _bsec_bme_settings.humidity_oversampling;
	_bme68x_conf.os_pres = _bsec_bme_settings.pressure_oversampling;
	_bme68x_conf.os_temp = _bsec_bme_settings.temperature_oversampling;

	_bme68x_status = bme68x_set_conf(&_bme68x_conf, &_bme68x);
	if (_bme68x_status < BME68X_OK)
		return;

	/* Set the gas sensor settings and link the heating profile */
	_bme68x_heatr_conf.enable = _bsec_bme_settings.run_gas;
	_bme68x_heatr_conf.shared_heatr_dur = GAS_WAIT_SHARED - (bme68x_get_meas_dur(BME68X_PARALLEL_MODE, &_bme68x_conf, &_bme68x) / INT64_C(1000));
	_bme68x_heatr_conf.heatr_temp = 0;
	_bme68x_heatr_conf.heatr_dur = 0;
	_bme68x_heatr_conf.heatr_temp_prof = _bsec_bme_settings.heater_temperature_profile;
	_bme68x_heatr_conf.heatr_dur_prof = _bsec_bme_settings.heater_duration_profile;
	_bme68x_heatr_conf.profile_len = _bsec_bme_settings.heater_profile_len;

	/* Select the power mode */
	_bme68x_status = bme68x_set_heatr_conf(BME68X_PARALLEL_MODE, &_bme68x_heatr_conf, &_bme68x);
	if (_bme68x_status < BME68X_OK)
		return;

	_bme68x_status = bme68x_set_op_mode(BME68X_PARALLEL_MODE, &_bme68x);
	if (_bme68x_status < BME68X_OK)
		return;

	/* Enable measurement index check to track the first valid sample after operation mode change */
	check_meas_idx = true;

	_op_mode = BME68X_PARALLEL_MODE;
}

/**
 * @brief Function to calculate an int64_t timestamp in milliseconds
 */
int64_t Bsec::getTimeMs(void)
{
	int64_t timeMs = millis();

	if (_timer_last_value > timeMs) { // An overflow occured
		_timer_last_value = timeMs;
		_timer_overflow_counter++;
	}
	return timeMs + (_timer_overflow_counter * 0xFFFFFFFF);
}

/**
 @brief Task that delays for a period of time
 */
void Bsec::delay_us(uint32_t period, void *intf_ptr)
{
	(void) intf_ptr;
	// Wait for a period
	// The system may simply be idle, sleep or even perform background tasks
	delayMicroseconds(period);
}

/**
 @brief Callback function for reading registers over I2C
 */
int8_t Bsec::i2cRead(uint8_t regAddr, uint8_t *regData, uint32_t length, void *intf_ptr)
{
	intptr_t devId = (intptr_t)intf_ptr;
	int8_t rslt = 0;
	if(Bsec::wireObj) {
		Bsec::wireObj->beginTransmission(devId);
		Bsec::wireObj->write(regAddr);
		rslt = Bsec::wireObj->endTransmission();
		Bsec::wireObj->requestFrom((int) devId, (int) length);
		for (auto i = 0; (i < length) && Bsec::wireObj->available(); i++) {
			regData[i] = Bsec::wireObj->read();
		}
	} else {
		rslt = -1;
	}
	return rslt;
}

/**
 * @brief Callback function for writing registers over I2C
 */
int8_t Bsec::i2cWrite(uint8_t regAddr, const uint8_t *regData, uint32_t length, void *intf_ptr)
{
	intptr_t devId = (intptr_t)intf_ptr;
	int8_t rslt = 0;
	if(Bsec::wireObj) {
		Bsec::wireObj->beginTransmission(devId);
		Bsec::wireObj->write(regAddr);
		for (auto i = 0; i < length; i++) {
			Bsec::wireObj->write(regData[i]);
		}
		rslt = Bsec::wireObj->endTransmission();
	} else {
		rslt = -1;
	}
	return rslt;
}

/**
 * @brief Callback function for reading over SPI
 */
int8_t Bsec::spiRead(uint8_t regAddr, uint8_t *regData, uint32_t length, void *intf_ptr)
{
	intptr_t devId = (intptr_t)intf_ptr;
	int8_t rslt = 0;
	if(Bsec::spiObj) {
		Bsec::spiObj->beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0)); // Can be up to 10MHz

		digitalWrite(devId, LOW);

		Bsec::spiObj->transfer(regAddr); // Write the register address, ignore the return
		for (auto i = 0; i < length; i++)
			regData[i] = Bsec::spiObj->transfer(regData[i]);

		digitalWrite(devId, HIGH);
		Bsec::spiObj->endTransaction();
	} else {
		rslt = -1;
	}

	return rslt;
}

/**
 * @brief Callback function for writing registers over SPI
 */
int8_t Bsec::spiWrite(uint8_t regAddr, const uint8_t *regData, uint32_t length, void *intf_ptr)
{
	intptr_t devId = (intptr_t)intf_ptr;
	int8_t rslt = 0;
	if(Bsec::spiObj) {
		Bsec::spiObj->beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0)); // Can be up to 10MHz

		digitalWrite(devId, LOW);

		Bsec::spiObj->transfer(regAddr); // Write the register address, ignore the return
		for (auto i = 0; i < length; i++)
			Bsec::spiObj->transfer(regData[i]);

		digitalWrite(devId, HIGH);
		Bsec::spiObj->endTransaction();
	} else {
		rslt = -1;
	}

	return rslt;
}

