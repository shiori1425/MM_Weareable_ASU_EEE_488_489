#include <string>
#include "Print.h"
#include "htu31d.h"

#define DEBUG_ENABLED

/***************************************************************************************
** Function name:           HTU31D
** Description:             Class constructor
***************************************************************************************/
HTU31D::HTU31D(){}

/***************************************************************************************
** Function name:           ~HTU31D
** Description:             Class destructor
***************************************************************************************/
HTU31D::~HTU31D(){}

/***************************************************************************************
** Function name:           N/A
** Description:             Initialize the Command Values to talk to HTU31D
***************************************************************************************/
uint8_t HTU31D::HTU31D_CONVERSION = (BASE_CONVERSION | OSRRH_BITS | OSRT_BITS);
uint8_t HTU31D::HTU31D_READTEMPHUM = 0x00;
uint8_t HTU31D::HTU31D_READHUMIDITY = 0x10;
uint8_t HTU31D::HTU31D_READSERIAL = 0x0A;
uint8_t HTU31D::HTU31D_HEATERON = 0x04;
uint8_t HTU31D::HTU31D_HEATEROFF = 0x02;
uint8_t HTU31D::HTU31D_RESET = 0x1E;
uint8_t HTU31D::HTU31D_READDIAGNOSTIC = 0x08;

/***************************************************************************************
** Function name:           begin
** Description:             Initialize sensor and start communication
***************************************************************************************/
bool HTU31D::begin(uint8_t i2c_addr, TwoWire *theWire) {
  if (theWire == nullptr) {
    theWire = &Wire;
  }

  // if using two sensors we need the command LSB to match the address LSB.
  updateCommandLSB(i2c_addr & 0x01);

  _i2caddr = i2c_addr;
  _wire = theWire;
  _wire->begin();
  
  if (!reset()) {
    #ifdef DEBUG_ENABLED
        Serial.println("Begin: Reset Failed!");
    #endif
    return false;
  }

  uint32_t serial;
  if (!readSerial(&serial)) {
    #ifdef DEBUG_ENABLED
        Serial.println("Begin: Read Serial Failed!");
    #endif
    return false;
  }

  return true;
}

/***************************************************************************************
** Function name:           reset
** Description:             Reset the sensor
***************************************************************************************/
bool HTU31D::reset(void) {
  _wire->beginTransmission(_i2caddr);
  _wire->write(HTU31D_RESET);
  if (_wire->endTransmission() != 0) {
    return false;
  }
  delay(15);
  return true;
}

/***************************************************************************************
** Function name:           readSerial
** Description:             Read sensor's serial number
***************************************************************************************/
bool HTU31D::readSerial(uint32_t* serial){

  uint8_t raw_data[4];
  // Initialize serial to all zeros
  *serial = 0;

  // 3) Send Read Serial command
  _wire->beginTransmission(_i2caddr);
  _wire->write(HTU31D_READSERIAL);
  if (_wire->endTransmission() != 0) {
    #ifdef DEBUG_ENABLED
        Serial.println("ReadSN: Send ReadSn command end transmission Error!");
    #endif
      return false;
  }

  // 4) Read 5 bytes of data for temperature
  if (_wire->requestFrom(_i2caddr, (uint8_t)4) != 4) {
    #ifdef DEBUG_ENABLED
        Serial.println("ReadSN: requestFrom Wire Error!");
    #endif
      return false;
  }

  for(int i = 0; i < 4; i++) {
      raw_data[i] = _wire->read();
  }

  *serial = raw_data[0];
  *serial <<= 8;
  *serial |= raw_data[1];
  *serial <<= 8;
  *serial |= raw_data[2];

  #ifdef DEBUG_ENABLED
      Serial.println("ReadSN: Serial Read Successful!");
  #endif

  return true;
}

/***************************************************************************************
** Function name:           enableHeater
** Description:             Enable sensor's built-in heater
***************************************************************************************/
bool HTU31D::enableHeater(void) {
  _wire->beginTransmission(_i2caddr);
  _wire->write(HTU31D_HEATERON);
  return (_wire->endTransmission() == 0);
}

/***************************************************************************************
** Function name:           disableHeater
** Description:             Disable sensor's built-in heater
***************************************************************************************/
bool HTU31D::disableHeater(void) {
  _wire->beginTransmission(_i2caddr);
  _wire->write(HTU31D_HEATEROFF);
  return (_wire->endTransmission() == 0);
}

/***************************************************************************************
** Function name:           readTemperature
** Description:             Read current temperature value from sensor memory
***************************************************************************************/
bool HTU31D::readTemperature(float* temperature) {
    uint8_t raw_data[3];

    // Send Conversion command to measure temp and humidity
    if (!initMeasurement()) {
      #ifdef DEBUG_ENABLED
        Serial.println("ReadTemp: Error Sending Conversion Command");
      #endif
        return false;
    }

    // Send Read Temperature command
    _wire->beginTransmission(_i2caddr);
    _wire->write(HTU31D_READTEMPHUM);
    if (_wire->endTransmission() != 0) {
        return false;
    }

    // Read 6 bytes of data for temperature
    if (_wire->requestFrom(_i2caddr, (uint8_t)3) != 3) {
        return false;
    }
    for(int i = 0; i < 3; i++) {
        raw_data[i] = _wire->read();
    }

    uint16_t raw_temp = (raw_data[0] << 8) | raw_data[1];

    // Check CRC for temperature
    if (htu31d_crc(raw_temp) != raw_data[2]) {
        return false;
    }

    // Convert raw values to temperature
    *temperature = (raw_temp / 65535.0) * 165 - 40;

  return true;
}

/***************************************************************************************
** Function name:           readHumidity
** Description:             Read current humidity value from sensor memory
***************************************************************************************/
bool HTU31D::readHumidity(float* humidity) {
    uint8_t raw_data[3];

    // Send Conversion command to measure temp and humidity
    if (!initMeasurement()) {
      #ifdef DEBUG_ENABLED
        Serial.println("ReadHumid: Error Sending Conversion Command");
      #endif
        return false;
    }

    // Send Read Humidity command
    _wire->beginTransmission(_i2caddr);
    _wire->write(HTU31D_READTEMPHUM);
    if (_wire->endTransmission() != 0) {
        return false;
    }

    // Read 3 bytes of data for humidity
    if (_wire->requestFrom(_i2caddr, (uint8_t)3) != 3) {
        return false;
    }
    for(int i = 0; i < 3; i++) {
        raw_data[i] = _wire->read();
    }

    uint16_t raw_hum = (raw_data[0] << 8) | raw_data[1];

    // Check CRC for humidity
    if (htu31d_crc(raw_hum) != raw_data[2]) {
        return false;
    }

    // Convert raw values to humidity
    *humidity = (raw_hum / 65535.0) * 100;
  return true;
}

/***************************************************************************************
** Function name:           htu31d_crc
** Description:             Calculate CRC for data integrity verification
***************************************************************************************/
uint8_t HTU31D::htu31d_crc(uint16_t value) {
  uint32_t polynom = 0x988000; // x^8 + x^5 + x^4 + 1
  uint32_t msb = 0x800000;
  uint32_t mask = 0xFF8000;
  uint32_t result = (uint32_t)value << 8; // Pad with zeros as specified in spec

  while (msb != 0x80) {
    // Check if msb of current value is 1 and apply XOR mask
    if (result & msb)
      result = ((result ^ polynom) & mask) | (result & ~mask);

    // Shift by one
    msb >>= 1;
    mask >>= 1;
    polynom >>= 1;
  }
  return result;
}

/***************************************************************************************
** Function name:           readDiagnosticRegister
** Description:             Read diagnostic information from sensor memory
***************************************************************************************/
bool HTU31D::readDiagnosticRegister(HTU31D::DiagnosticInfo* info) {
  
  if (!info) {
    #ifdef DEBUG_ENABLED
        Serial.println("DiagReg: Result pointer is null!");
    #endif
      return false;
  }

  uint8_t raw_data[2];

  // 1) Send command to read diagnostic register
  _wire->beginTransmission(_i2caddr);
  _wire->write(HTU31D_READDIAGNOSTIC);
  if (_wire->endTransmission() != 0) {
    #ifdef DEBUG_ENABLED
        Serial.println("DiagReg: Send Read Diag Command Error!");
    #endif
      return false;
  }

  delay(5);

  // 2) Read the diagnostic byte
  if (_wire->requestFrom(_i2caddr, (uint8_t)2) != 2) {
    #ifdef DEBUG_ENABLED
        Serial.println("DiagReg: requestFrom Wire Error!");
    #endif
      return false;
  }

  for(int i = 0; i < 2; i++) {
        raw_data[i] = _wire->read();
  }

  // Check CRC
  if (htu31d_crc(raw_data[0]) != raw_data[1]) {
    #ifdef DEBUG_ENABLED
      Serial.println("DiagReg: CRC Error!");
    #endif
      return false;
  }

  // Populate the DiagnosticInfo struct based on the diagnostic register
  info->nvmError = (raw_data[0] & (1 << NVM_ERROR_BIT)) != 0;
  info->humidityOverrun = (raw_data[0] & (1 << HUMIDITY_OVERRUN_BIT)) != 0;
  info->humidityHighError = (raw_data[0] & (1 << HUMIDITY_HIGH_ERROR_BIT)) != 0;
  info->humidityLowError = (raw_data[0] & (1 << HUMIDITY_LOW_ERROR_BIT)) != 0;
  info->tempOverrun = (raw_data[0] & (1 << TEMP_OVERRUN_BIT)) != 0;
  info->tempHighError = (raw_data[0] & (1 << TEMP_HIGH_ERROR_BIT)) != 0;
  info->tempLowError = (raw_data[0] & (1 << TEMP_LOW_ERROR_BIT)) != 0;
  info->heaterOn = (raw_data[0] & (1 << HEATER_ON_BIT)) != 0;

  #ifdef DEBUG_ENABLED
      Serial.println("DiagReg: Read Successful");
  #endif

  return true;
}

/***************************************************************************************
** Function name:           readTempAndHumidity
** Description:             Read both temperature and humidity values from sensor
***************************************************************************************/
bool HTU31D::readTempAndHumidity(HTU31D::THData* result) {

    if (!result) {
      #ifdef DEBUG_ENABLED
        Serial.println("ReadTH: Result pointer is null!");
      #endif
        return false;
    }

    result->temperature = NAN;
    result->humidity = NAN;

    uint8_t raw_data[6];

    // Send Conversion command to measure temp and humidity
    if (!initMeasurement()) {
      #ifdef DEBUG_ENABLED
        Serial.println("ReadTRH: Error Sending Conversion Command");
      #endif
        return false;
    }

    // Send Read T&H command
    _wire->beginTransmission(_i2caddr);
    _wire->write(HTU31D_READTEMPHUM);
    if (_wire->endTransmission() != 0) {
      #ifdef DEBUG_ENABLED
        Serial.println("ReadTRH: ReadTRH Command End Transmission Error!");
      #endif
        return false;
    }

    // Read 6 bytes of data (3 for temperature and 3 for humidity)
    if (_wire->requestFrom(_i2caddr, (uint8_t)6) != 6) {
      #ifdef DEBUG_ENABLED
        Serial.println("ReadTRH: Read Bytes from Wire Error!");
      #endif
        return false;
    }
    for(int i = 0; i < 6; i++) {
        raw_data[i] = _wire->read();
    }

    uint16_t raw_temp = (raw_data[0] << 8) | raw_data[1];
    uint16_t raw_humidity = (raw_data[3] << 8) | raw_data[4];

    // Check CRC for temperature and humidity
    if (htu31d_crc(raw_temp) != raw_data[2] || htu31d_crc(raw_humidity) != raw_data[5]) {
      #ifdef DEBUG_ENABLED
        Serial.println("ReadTRH: CRC Error!");
      #endif
        return false;
    }

    // Convert raw values to temperature and humidity
    result->temperature = (raw_temp / 65535.0) * 165 - 40;
    result->humidity = (raw_humidity / 65535.0) * 100;

    #ifdef DEBUG_ENABLED
        Serial.println("ReadTH: Read Temp and Humidity Successful!");
    #endif

    return true;
}

/***************************************************************************************
** Function name:           updateCommandLSB
** Description:             Update command based on the I2C Address LSB
***************************************************************************************/
void HTU31D::updateCommandLSB(uint8_t lsb) {
      HTU31D_CONVERSION |= lsb;
      HTU31D_READHUMIDITY |= lsb;
      HTU31D_READTEMPHUM |= lsb;
      HTU31D_READSERIAL |= lsb;
      HTU31D_HEATERON |= lsb;
      HTU31D_HEATEROFF |= lsb;
      HTU31D_RESET |= lsb;
      HTU31D_READDIAGNOSTIC |= lsb;
}

/***************************************************************************************
** Function name:           initMeasurement
** Description:             Initialize Conversion process on IC
***************************************************************************************/
bool HTU31D::initMeasurement(void){
  // 1) Send Conversion command
  _wire->beginTransmission(_i2caddr);
  _wire->write(HTU31D_CONVERSION);
  if (_wire->endTransmission() != 0) {
      return false;
  }

  // 2) Wait for conversion to happen
  delay(15);
  
  return true;
}
