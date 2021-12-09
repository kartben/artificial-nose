#include <Arduino.h>
#include <bsec.h>
#include <bsec_serialized_configurations_selectivity.h>

/* Macros used */
#define PANIC_LED LED_BUILTIN
#define ERROR_DUR 1000

/* Helper functions declarations */
/**
 * @brief : This function toggles the led when a fault was detected
 */
void errLeds(void);

/**
 * @brief : This function setups the BSEC stack for "bsec" sensor over I2C
 * @param[in] bsec  : Bsec class object
 * @param[in] i2cAddr : I2C address of the Bsec2 sensor
 * @param[in] i2c : I2C bus to use
 */
void setupBsec(Bsec &bsec, uint8_t i2cAddr, TwoWire &i2c);

/**
 * @brief : This function checks the BSEC status, prints the respective error code. Halts in case of error
 * @param[in] bsec  : Bsec class object
 */
void checkBsecStatus(Bsec &bsec);

/**
 * @brief : This function is called by the BSEC library when a new output is available
 * @param[in] input     : BME68X sensor data before processing
 * @param[in] outputs   : Processed BSEC output data
 * @param[in] bsec      : Instance of BSEC calling the callback
 */
void bsecCallback(const bme68x_data &input, const BsecOutput &outputs);