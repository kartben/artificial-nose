#include "bme688_helper.h"

void setupBsec(Bsec &bsec, uint8_t i2cAddr, TwoWire &i2c)
{
    /* Desired subscription list of BSEC outputs */
    bsec_virtual_sensor_t sensorList[] = {
        BSEC_OUTPUT_RAW_TEMPERATURE,
        // BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_RAW_GAS_INDEX,
        BSEC_OUTPUT_COMPENSATED_GAS,
        BSEC_OUTPUT_CO2_EQUIVALENT,
        BSEC_OUTPUT_RUN_IN_STATUS,
        BSEC_OUTPUT_IAQ,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
        // BSEC_OUTPUT_GAS_ESTIMATE_1,
        // BSEC_OUTPUT_GAS_ESTIMATE_2,
        // BSEC_OUTPUT_GAS_ESTIMATE_3,
        // BSEC_OUTPUT_GAS_ESTIMATE_4
    };

    /* Initialize the library and interfaces */
    if (!bsec.begin(i2cAddr, i2c))
    {
        checkBsecStatus(bsec);
    }

    /* Initialize the library and interfaces */
    if (!bsec.setConfig(bsec_config_selectivity))
    {
        checkBsecStatus(bsec);
    }

    /* Subsribe to the desired BSEC outputs */
    if (!bsec.updateSubscription(sensorList, ARRAY_LEN(sensorList), BSEC_SAMPLE_RATE_HIGH_PERFORMANCE))
    {
        checkBsecStatus(bsec);
    }

    //  Serial.println("\nBSEC library version " + String(bsecInst.getVersion().major) + "." + String(bsecInst.getVersion().minor) + "." + String(bsecInst.getVersion().major_bugfix) + "." + String(bsecInst.getVersion().minor_bugfix));
}

void errLeds(void)
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}

void checkBsecStatus(Bsec &bsec)
{
    int bme68x_status = (int)bsec.getBme68xStatus();
    int bsec_status = (int)bsec.getBsecStatus();

    if (bsec_status < BSEC_OK)
    {
        Serial.println("BSEC error code : " + String(bsec_status));
        for (;;)
            errLeds(); /* Halt in case of failure */
    }
    else if (bsec_status > BSEC_OK)
    {
        Serial.println("BSEC warning code : " + String(bsec_status));
    }

    if (bme68x_status < BME68X_OK)
    {
        Serial.println("BME68X error code : " + String(bme68x_status));
        for (;;)
            errLeds(); /* Halt in case of failure */
    }
    else if (bme68x_status > BME68X_OK)
    {
        Serial.println("BME68X warning code : " + String(bme68x_status));
    }
}
