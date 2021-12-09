#include <EEPROM.h>
#include <bsec.h>
#include <bsec_serialized_configurations_selectivity.h>

#define STATE_SAVE_PERIOD	UINT32_C(360 * 60 * 1000) // 360 minutes - 4 times a day

// Helper functions declarations
void errLeds(void);
void checkBsecStatus(Bsec& bsec);
void updateBsecState(Bsec& bsec);
void bsecCallback(const bme68x_data& input, const BsecOutput& outputs);
bool loadState(Bsec& bsec);
bool saveState(Bsec& bsec);

// Create an object of the class Bsec
Bsec bsecInst(bsecCallback);

// Entry point for the example
void setup(void)
{ 
  bsec_virtual_sensor_t sensorList[] = { 
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
	  BSEC_OUTPUT_RAW_GAS_INDEX,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_GAS_ESTIMATE_1,
  	BSEC_OUTPUT_GAS_ESTIMATE_2,
  	BSEC_OUTPUT_GAS_ESTIMATE_3,
  	BSEC_OUTPUT_GAS_ESTIMATE_4
  };
    
  Serial.begin(500000);
  EEPROM.begin(BSEC_MAX_STATE_BLOB_SIZE + 1);

  if(!bsecInst.begin(BME68X_I2C_ADDR_LOW, Wire)  		||
     !bsecInst.setConfig(bsec_config_selectivity)       ||
     !loadState(bsecInst)                               ||
     !bsecInst.updateSubscription(sensorList, ARRAY_LEN(sensorList), BSEC_SAMPLE_RATE_HIGH_PERFORMANCE))
     checkBsecStatus(bsecInst);
     
  Serial.println("\nBSEC library version " + String(bsecInst.getVersion().major) + "." + String(bsecInst.getVersion().minor) + "." + String(bsecInst.getVersion().major_bugfix) + "." + String(bsecInst.getVersion().minor_bugfix));
  delay(10);
}

// Function that is looped forever
void loop(void)
{
  if(!bsecInst.run())
    checkBsecStatus(bsecInst);
}

void errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

void updateBsecState(Bsec& bsec)
{
  static uint16_t stateUpdateCounter = 0;
	bool update = false;
  
	if (stateUpdateCounter == 0) {
    const bsec_output_t* iaq = bsec.getOutput(BSEC_OUTPUT_IAQ);
		/* First state update when IAQ accuracy is >= 3 */
    if (iaq && iaq->accuracy >= 3) {
      update = true;
      stateUpdateCounter++;
    }
	} else if ((stateUpdateCounter * STATE_SAVE_PERIOD) < millis()) {
    /* Update every STATE_SAVE_PERIOD minutes */
    update = true;
    stateUpdateCounter++;
	}

	if (update && !saveState(bsec))
    checkBsecStatus(bsec);
}

void bsecCallback(const bme68x_data& input, const BsecOutput& outputs) 
{ 
  if (!outputs.len) 
    return;
    
  Serial.println("BSEC outputs:\n\ttimestamp = " + String((int)(outputs.outputs[0].time_stamp / INT64_C(1000000))));
  for (uint8_t i = 0; i < outputs.len; i++) {
    const bsec_output_t& output = outputs.outputs[i];
    switch (output.sensor_id) {
      case BSEC_OUTPUT_IAQ:
        Serial.println("\tiaq = " + String(output.signal));
        Serial.println("\tiaq accuracy = " + String((int)output.accuracy));
        break;
      case BSEC_OUTPUT_RAW_TEMPERATURE:
        Serial.println("\ttemperature = " + String(output.signal));
        break;
      case BSEC_OUTPUT_RAW_PRESSURE:
        Serial.println("\tpressure = " + String(output.signal));
        break;
      case BSEC_OUTPUT_RAW_HUMIDITY:
        Serial.println("\thumidity = " + String(output.signal));
        break;
      case BSEC_OUTPUT_RAW_GAS:
        Serial.println("\tgas resistance = " + String(output.signal));
        break;
	  case BSEC_OUTPUT_RAW_GAS_INDEX:
        Serial.println("\tgas index = " + String(output.signal));
        break;
      case BSEC_OUTPUT_STABILIZATION_STATUS:
        Serial.println("\tstabilization status = " + String(output.signal));
        break;
      case BSEC_OUTPUT_RUN_IN_STATUS:
        Serial.println("\trun in status = " + String(output.signal));
        break;
      case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE:
        Serial.println("\tcompensated temperature = " + String(output.signal));
        break;
      case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY:
        Serial.println("\tcompensated humidity = " + String(output.signal));
        break;
      case BSEC_OUTPUT_GAS_ESTIMATE_1:
      case BSEC_OUTPUT_GAS_ESTIMATE_2:
      case BSEC_OUTPUT_GAS_ESTIMATE_3:
      case BSEC_OUTPUT_GAS_ESTIMATE_4:
        Serial.println("\tgas estimate " + String((int)(output.sensor_id + 1 - BSEC_OUTPUT_GAS_ESTIMATE_1)) + String(" = ") + String(output.signal) + "-- accuracy = " + String((int)output.accuracy) );
        break;
      default:
        break;
    }
  }
  
  updateBsecState(bsecInst);
}

void checkBsecStatus(Bsec& bsec)
{
  int bme68x_status = (int)bsec.getBme68xStatus();
  int bsec_status = (int)bsec.getBsecStatus();
  
  if (bsec_status < BSEC_OK) {
      Serial.println("BSEC error code : " + String(bsec_status));
      for (;;)
        errLeds(); /* Halt in case of failure */
  } else if (bsec_status > BSEC_OK) {
      Serial.println("BSEC warning code : " + String(bsec_status));
  }

  if (bme68x_status < BME68X_OK) {
      Serial.println("BME68X error code : " + String(bme68x_status));
      for (;;)
        errLeds(); /* Halt in case of failure */
  } else if (bme68x_status > BME68X_OK) {
      Serial.println("BME68X warning code : " + String(bme68x_status));
  }
}

bool loadState(Bsec& bsec)
{
  uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE];
  
  if (EEPROM.read(0) == BSEC_MAX_STATE_BLOB_SIZE) {
    // Existing state in EEPROM
    Serial.println("Reading state from EEPROM");

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
      bsecState[i] = EEPROM.read(i + 1);
      Serial.println(bsecState[i], HEX);
    }

    if(!bsec.setState(bsecState))
      return false;
  } else {
    // Erase the EEPROM with zeroes
    Serial.println("Erasing EEPROM");

    for (uint8_t i = 0; i <= BSEC_MAX_STATE_BLOB_SIZE; i++)
      EEPROM.write(i, 0);

    EEPROM.commit();
  }
  return true;
}

bool saveState(Bsec& bsec)
{
  uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE];
  
    if (!bsec.getState(bsecState))
    return false;
    
  Serial.println("Writing state to EEPROM");

  for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
    EEPROM.write(i + 1, bsecState[i]);
    Serial.println(bsecState[i], HEX);
  }

  EEPROM.write(0, BSEC_MAX_STATE_BLOB_SIZE);
  EEPROM.commit();
  
  return true;
}
