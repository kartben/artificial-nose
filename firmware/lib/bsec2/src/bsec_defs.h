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
 * @file	bsec_defs.h
 * @date	23 April 2021
 * @version	1.5.0
 *
 */

#ifndef BSEC_DEFS_H_
#define BSEC_DEFS_H_

/* Constants
 *
 */
/** Gas Wait Shared  */
#define GAS_WAIT_SHARED		UINT8_C(140)

/*!
 *@brief Structure to hold sensor data
 */
struct  bme680_sensor_data {		
	double pressure;
	double temperature;
	double humidity;
	double gas_resistance;
	uint16_t gas_valid;
	uint16_t heat_stab;
	uint16_t gas_raw;
	uint16_t gas_range;
	uint16_t gas_meas_index;
	uint16_t sub_meas_index;
	uint8_t idac;
	uint16_t res_heat;
	
	/* Add parameter for reading H-channel*/
	double gas_resistance_H;
	uint16_t gas_valid_H;
	uint16_t heat_stab_H;
	uint16_t gas_raw_H;
	uint16_t gas_range_H;
};

#endif
