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
 * @file	bsec.h
 * @date	23 April 2021
 * @version	1.5.0
 *
 */

#ifndef BSEC_CLASS_H
#define BSEC_CLASS_H

#define BME68X_PERIOD_POLL				UINT32_C(5000)

/* Includes */
#include "Arduino.h"
#include "Wire.h"
#include "SPI.h"
#include "inc/bsec_datatypes.h"
#include "inc/bsec_interface.h"
#include "bme68x/bme68x.h"
#include "bsec_defs.h"

#define ARRAY_LEN(array)				(sizeof(array)/sizeof(array[0]))
#define CHECK_BSEC_INPUT(x, shift)		(x & (1 << (shift-1)))
/*
 * Macro definition for valid new data (0x80) AND
 * heater stability (0x10) AND gas resistance (0x20) values
 */
#define BME68X_VALID_DATA  UINT8_C(0xB0)

enum BsecStep {
	CONTROL_STEP,
	MEASUREMENT_STEP,
	
	LAST_STEP
};

inline BsecStep& operator++(BsecStep& step)
{
	step = static_cast<BsecStep>((static_cast<int>(step) + 1) % LAST_STEP);
	return step;
}

struct BsecOutput {
	bsec_output_t outputs[BSEC_NUMBER_OUTPUTS];
	uint8_t len;
};



class Bsec;

typedef void (*BsecCallback)(const bme68x_data&, const BsecOutput&);

/* BSEC class definition */
class Bsec
{
public:
	/* Public variables */
	static TwoWire *wireObj;
	static SPIClass *spiObj;
	
	/* Public APIs */
	/**
	 * @brief Constructor
	 * @param	optional BSEC output processor callback can be provided
	 */
	Bsec(BsecCallback callback = nullptr);

	/**
	 * @brief Function to initialize the BSEC library and the BME68x sensor
	 * @param intf_ptr		: Device identifier parameter for the read/write interface functions
	 * @param intf		    : Physical communication interface
	 * @param read		    : Pointer to the read function
	 * @param write		    : Pointer to the write function
	 * @param idleTask	    : Pointer to the idling task
	 * @return	true for success, false otherwise
	 */
	bool begin(void *intf_ptr, enum bme68x_intf intf, bme68x_read_fptr_t read, bme68x_write_fptr_t write, bme68x_delay_us_fptr_t idleTask);
	
	/**
	 * @brief Function to initialize the BSEC library and the BME68x sensor
	 * @param i2cAddr	    : I2C address
	 * @param i2c		    : Pointer to the TwoWire object
	 * @return	true for success, false otherwise
	 */
	bool begin(uint8_t i2cAddr, TwoWire &i2c);
	
	/**
	 * @brief Function to initialize the BSEC library and the BME68x sensor
	 * @param chipSelect	: SPI chip select
	 * @param spi			: Pointer to the SPIClass object
	 * @return	true for success, false otherwise
	 */
	bool begin(uint8_t chipSelect, SPIClass &spi);

	/**
	 * @brief Function that sets the desired sensors and the sample rates
	 * @param sensorList	: The list of output sensors
	 * @param nSensors		: Number of outputs requested
	 * @param sampleRate	: The sample rate of requested sensors
	 * @return	true for success, false otherwise
	 */
	bool updateSubscription(bsec_virtual_sensor_t sensorList[], uint8_t nSensors, float sampleRate = BSEC_SAMPLE_RATE_ULP);

	/**
	 * @brief Callback from the user to read data from the BME68x using parallel/forced mode, process and store outputs
	 * @return	true for success, false otherwise
	 */
	bool run(void);
	
	/**
	 * @brief Function to get the BSEC outputs
	 * @return	pointer to BSEC outputs if available else nullptr
	 */
	const BsecOutput* getOutputs(void) const
	{
		if (_output.len)
			return &_output;
		return nullptr;
	}
	
	/**
	 * @brief Function to get the BSEC output by sensor id
	 * @return	pointer to BSEC output, nullptr otherwise
	 */
	const bsec_output_t* getOutput(bsec_virtual_sensor_t id) const
	{
		for (uint8_t i = 0; i < _output.len; i++)
			if (id == _output.outputs[i].sensor_id)
				return &_output.outputs[i];
		return nullptr;
	}
	
	/**
	 * @brief Function to get the BSEC error status
	 */
	bsec_library_return_t getBsecStatus(void) const
	{
		return _bsec_status;
	}
	
	/**
	 * @brief Function to get the bme68x error status
	 */
	int8_t getBme68xStatus(void) const
	{
		return _bme68x_status;
	}
	
	/**
	 * @brief Function to get the BSEC software version
	 */
	const bsec_version_t& getVersion(void)
	{
		_bsec_status = bsec_get_version(&_bsec_version);
		return _bsec_version;
	}
	
	/**
	 * @brief Function to get the BME68x heater configuration
	 */
	const bme68x_heatr_conf& getHeaterConfiguration(void)
	{
		return _bme68x_heatr_conf;
	}
	
	/**
	 * @brief Function to get the BME68x sensor configuration
	 */
	const bme68x_conf& getSensorConfiguration(void)
	{
		return _bme68x_conf;
	}

	/**
	 * @brief Function to get the state of the algorithm to save to non-volatile memory
	 * @param state			: Pointer to a memory location that contains the state
	 * @return	true for success, false otherwise
	 */
	bool getState(uint8_t *state);

	/**
	 * @brief Function to set the state of the algorithm from non-volatile memory
	 * @param state			: Pointer to a memory location that contains the state
	 * @return	true for success, false otherwise
	 */
	bool setState(uint8_t *state);

	/**
	 * @brief Function to set the configuration of the algorithm from memory
	 * @param state			: Pointer to a memory location that contains the configuration
	 * @return	true for success, false otherwise
	 */
	bool setConfig(const uint8_t *config);

	/**
	 * @brief Function to set the temperature offset
	 * @param tempOffset	: Temperature offset in degree Celsius
	 */
	void setTemperatureOffset(float tempOffset)
	{
		_temp_offset = tempOffset;
	}
	
	/**
	 * @brief Function to calculate an int64_t timestamp in milliseconds
	 */
	int64_t getTimeMs(void);
	
	/**
	* @brief Task that delays for a ms period of time
	* @param period	: Period of time in ms
	* @param intf_ptr : Pointer to device id
	*/
	static void delay_us(uint32_t period, void *intf_ptr);

	/**
	* @brief Callback function for reading registers over I2C
	* @param regAddr	: Register address
	* @param regData	: Pointer to the array containing the data to be read
	* @param length	: Length of the array of data
	* @param intf_ptr : Pointer to device id
	* @return	Zero for success, non-zero otherwise
	*/
	static int8_t i2cRead(uint8_t regAddr, uint8_t *regData, uint32_t length, void *intf_ptr);

	/**
	* @brief Callback function for writing registers over I2C
	* @param regAddr	: Register address
	* @param regData	: Pointer to the array containing the data to be written
	* @param length	: Length of the array of data
	* @param intf_ptr : Pointer to device id
	* @return	Zero for success, non-zero otherwise
	*/
	static int8_t i2cWrite(uint8_t regAddr, const uint8_t *regData, uint32_t length, void *intf_ptr);

	/**
	* @brief Callback function for reading and writing registers over SPI
	* @param regAddr	: Register address
	* @param regData	: Pointer to the array containing the data to be read or written
	* @param length	: Length of the array of data
	* @param intf_ptr : Pointer to device id
	* @return	Zero for success, non-zero otherwise
	*/
	static int8_t spiRead(uint8_t regAddr, uint8_t *regData, uint32_t length, void *intf_ptr);	
	
	/**
	* @brief Callback function for reading and writing registers over SPI
	* @param regAddr	: Register address
	* @param regData	: Pointer to the array containing the data to be read or written
	* @param length	: Length of the array of data
	* @param intf_ptr : Pointer to device id
	* @return	Zero for success, non-zero otherwise
	*/
	static int8_t spiWrite(uint8_t regAddr, const uint8_t *regData, uint32_t length, void *intf_ptr);	

private:
	/* Private variables */
	struct bme68x_dev _bme68x;
	struct bme68x_conf _bme68x_conf;
	struct bme68x_heatr_conf _bme68x_heatr_conf;	
	struct bme68x_data _bme68x_data[3]; // accessing bme68x data registers
	int8_t _bme68x_status;				// Placeholder for the BME68x driver's error codes
	
	bsec_version_t _bsec_version;		// Stores the version of the BSEC algorithm
	bsec_bme_settings_t _bsec_bme_settings;
	bsec_library_return_t _bsec_status;
	
	BsecStep _step;
	BsecOutput _output;
	BsecCallback _proc;
	uint8_t _op_mode;					// operating mode of sensor
	uint8_t prev_gas_idx;
	bool check_meas_idx;                // check measurement index to track the order
	uint8_t last_meas_index;            // last measurement index received from sensor
	
	float _temp_offset;
	// Global variables to help create a millisecond timestamp that doesn't overflow every 51 days.
	// If it overflows, it will have a negative value. Something that should never happen.
	uint32_t _timer_overflow_counter;
	uint32_t _timer_last_value;

	/**
	 * @brief Read data from the BME68x sensor and process it
	 * @param currTimeNs: Current time in ns
	 * @return true if there are new outputs. false otherwise
	 */
	bool processData(int64_t currTimeNs, const struct bme68x_data& data);
	
	/**
	 * @brief Common code for the begin function
	 */
	bool beginCommon();
	
	/**
	 * @brief Run BSEC sensor control
	 */
	bool doSensorControl();
	
	/**
	 * @brief Run BME68x sensor measurement
	 */
	bool doSensorMeasurement();
	
	/**
	 * @brief Set the BME68x sensor configuration to forced mode
	 */
	void setBme68xConfigForced(void);

	/**
	 * @brief Set the BME68x sensor configuration to sleep mode
	 */
	void setBme68xConfigSleep(void);

	/**
	 * @brief Set the BME68x sensor configuration to parallel mode
	 */
	void setBme68xConfigParallel(void);
};

#endif
