#ifndef _SIMPLE_HTU31D_H
#define _SIMPLE_HTU31D_H

#include "Arduino.h"
#include <Wire.h>
#include <stdint.h>

// OSR Resolution definitions 

//#define HUMIDITY_OSR_3   // 0.007%RH, 7.8ms Conversion time
//#define HUMIDITY_OSR_2   // 0.010%RH, 3.9ms Conversion time
//#define HUMIDITY_OSR_1   // 0.014%RH, 2.0ms Conversion time
//#define HUMIDITY_OSR_0   // 0.020%RH, 1.0ms Conversion time

//#define TEMPERATURE_OSR_3   // 0.012°C, 12.1ms Conversion time
//#define TEMPERATURE_OSR_2   // 0.016°C, 6.1ms Conversion time
//#define TEMPERATURE_OSR_1   // 0.025°C, 3.1ms Conversion time
//#define TEMPERATURE_OSR_0   // 0.040°C, 1.6ms Conversion time

#define BASE_CONVERSION 0x40  // 01000000 in binary

#ifdef HUMIDITY_OSR_3
    #define OSRRH_BITS 0x18  // 0b00011000 in binary
#elif defined(HUMIDITY_OSR_2)
    #define OSRRH_BITS 0x10  // 0b00010000 in binary
#elif defined(HUMIDITY_OSR_1)
    #define OSRRH_BITS 0x08  // 0b00001000 in binary
#elif defined(HUMIDITY_OSR_0)
    #define OSRRH_BITS 0x00  // 0b00000000 in binary
#else
    // No OSR defined. Default to fastest resolution
    #define OSRRH_BITS 0x00
#endif

#ifdef TEMPERATURE_OSR_3
    #define OSRT_BITS 0x06  // 0b00000110 in binary
#elif defined(TEMPERATURE_OSR_2)
    #define OSRT_BITS 0x04  // 0b00000100 in binary
#elif defined(TEMPERATURE_OSR_1)
    #define OSRT_BITS 0x02  // 0b00000010 in binary
#elif defined(TEMPERATURE_OSR_0)
    #define OSRT_BITS 0x00  // 0b00000000 in binary
#else
    // No OSR defined. Default to fastest resolution
    #define OSRT_BITS 0x00  // 0b00000000 in binary
#endif

// HTU31D Default I2C Address
#define HTU31D_DEFAULT_I2CADDR (0x40)

// Diagnostic register bit definitions
#define NVM_ERROR_BIT         7
#define HUMIDITY_OVERRUN_BIT  6
#define HUMIDITY_HIGH_ERROR_BIT 5
#define HUMIDITY_LOW_ERROR_BIT 4
#define TEMP_OVERRUN_BIT      3
#define TEMP_HIGH_ERROR_BIT   2
#define TEMP_LOW_ERROR_BIT    1
#define HEATER_ON_BIT         0

class HTU31D {
private:

  // HTU31D Commands
  static uint8_t HTU31D_CONVERSION;
  static uint8_t HTU31D_READHUMIDITY;
  static uint8_t HTU31D_READTEMPHUM;
  static uint8_t HTU31D_READSERIAL;
  static uint8_t HTU31D_HEATERON;
  static uint8_t HTU31D_HEATEROFF;
  static uint8_t HTU31D_RESET;
  static uint8_t HTU31D_READDIAGNOSTIC;

  TwoWire *_wire;
  uint8_t _i2caddr;
  uint8_t htu31d_crc(uint16_t value);  
  void updateCommandLSB(uint8_t lsb);


public:
  struct DiagnosticInfo {
    bool nvmError;
    bool humidityOverrun;
    bool humidityHighError;
    bool humidityLowError;
    bool tempOverrun;
    bool tempHighError;
    bool tempLowError;
    bool heaterOn;
  };

  struct THData {
      float temperature;
      float humidity;
  };

  HTU31D();
  ~HTU31D();


  bool begin(uint8_t i2c_addr = 0x40, TwoWire *theWire = &Wire);
  bool reset(void);
  bool initMeasurement(void);
  bool enableHeater(void);
  bool disableHeater(void);
  bool readTemperature(float* temperature);
  bool readHumidity(float* humidity);
  bool readSerial(uint32_t* serial);
  bool readDiagnosticRegister(HTU31D::DiagnosticInfo* info);
  bool readTempAndHumidity(HTU31D::THData* result);

protected:

};

#endif 
