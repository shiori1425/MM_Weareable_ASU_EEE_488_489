#include "htu31d.h"

HTU31D::HTU31D() : _humidity(0.0f), _temperature(0.0f) {}

HTU31D::~HTU31D() {}

bool HTU31D::begin(uint8_t i2c_addr, TwoWire *theWire) {
  _i2caddr = i2c_addr;
  _wire = theWire;
  _wire->begin();
  
  if (!reset()) {
    return false;
  }

  // Add any other initialization logic if necessary.
  return true;
}

bool HTU31D::reset(void) {
  _wire->beginTransmission(_i2caddr);
  _wire->write(HTU31D_RESET);
  if (_wire->endTransmission() != 0) {
    return false;
  }
  delay(15);
  return true;
}

bool HTU31D::enableHeater(bool en) {
  _wire->beginTransmission(_i2caddr);
  _wire->write(en ? HTU31D_HEATERON : HTU31D_HEATEROFF);
  return (_wire->endTransmission() == 0);
}

float HTU31D::readTemperature(void) {
    uint8_t raw_data[3];

    // 1) Send Conversion command
    _wire->beginTransmission(_i2caddr);
    _wire->write(HTU31D_CONVERSION);
    if (_wire->endTransmission() != 0) {
        return false;
    }

    // 2) Wait 10ms
    delay(10);

    // 3) Send Read Temperature command
    _wire->beginTransmission(_i2caddr);
    _wire->write(read_temp_cmd);
    if (_wire->endTransmission() != 0) {
        return false;
    }

    // 4) Read 3 bytes of data for temperature
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
    _temperature = (raw_temp / 65535.0) * 165 - 40;

  return _temperature;
}

float HTU31D::readHumidity(void) {
    uint8_t raw_data[3];

    // 1) Send Conversion command
    _wire->beginTransmission(_i2caddr);
    _wire->write(HTU31D_CONVERSION);
    if (_wire->endTransmission() != 0) {
        return false;
    }

    // 2) Wait 10ms
    delay(10);

    // 3) Send Read Humidity command
    _wire->beginTransmission(_i2caddr);
    _wire->write(read_humidity_cmd);
    if (_wire->endTransmission() != 0) {
        return false;
    }

    // 4) Read 3 bytes of data for humidity
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
    _humidity = (raw_hum / 65535.0) * 100;
  return _humidity;
}

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

DiagnosticInfo HTU31D::readDiagnosticRegister(void) {
  
  DiagnosticInfo info;

  // Send command to read diagnostic register
  if (!i2c_dev->write(HTU31D_READDIAGNOSTIC, 1)) {
    return false;
  }

  // Read the diagnostic register
  if (!i2c_dev->read(&diagRegister, 1)) {
    return false;
  }

  // Populate the DiagnosticInfo struct based on the diagnostic register
  info.nvmError = diagRegister & (1 << NVM_ERROR_BIT);
  info.humidityOverrun = diagRegister & (1 << HUMIDITY_OVERRUN_BIT);
  info.humidityHighError = diagRegister & (1 << HUMIDITY_HIGH_ERROR_BIT);
  info.humidityLowError = diagRegister & (1 << HUMIDITY_LOW_ERROR_BIT);
  info.tempOverrun = diagRegister & (1 << TEMP_OVERRUN_BIT);
  info.tempHighError = diagRegister & (1 << TEMP_HIGH_ERROR_BIT);
  info.tempLowError = diagRegister & (1 << TEMP_LOW_ERROR_BIT);
  info.heaterOn = diagRegister & (1 << HEATER_ON_BIT);
  
  return info;
}

THData HTU31D::readTempAndHumidity(void) {
    THData result;
    result.temperature = NAN;
    result.humidity = NAN;

    uint8_t raw_data[6];

    // 1) Send Conversion command
    _wire->beginTransmission(_i2caddr);
    _wire->write(HTU31D_CONVERSION);
    if (_wire->endTransmission() != 0) {
        return false;
    }

    // 2) Wait 10ms
    delay(10);

    // 3) Send Read T&H command
    _wire->beginTransmission(_i2caddr);
    _wire->write(HTU31D_READTEMPHUM);
    if (_wire->endTransmission() != 0) {
        return false;
    }

    // 4) Read 6 bytes of data (3 for temperature and 3 for humidity)
    if (_wire->requestFrom(_i2caddr, (uint8_t)6) != 6) {
        return false;
    }
    for(int i = 0; i < 6; i++) {
        raw_data[i] = _wire->read();
    }

    uint16_t raw_temp = (raw_data[0] << 8) | raw_data[1];
    uint16_t raw_humidity = (raw_data[3] << 8) | raw_data[4];

    // Check CRC for temperature and humidity
    if (htu31d_crc(raw_temp) != raw_data[2] || htu31d_crc(raw_humidity) != raw_data[5]) {
        return false;
    }

    // Convert raw values to temperature and humidity
    result.temperature = (raw_temp / 65535.0) * 165 - 40;
    result.humidity = (raw_humidity / 65535.0) * 100;

    return result;
}

