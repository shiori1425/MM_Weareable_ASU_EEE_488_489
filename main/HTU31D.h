#ifndef _SIMPLE_HTU31D_H
#define _SIMPLE_HTU31D_H

#include <Wire.h>

// OSR Resolution definitions 

#define HUMIDITY_OSR_3   // 0.007%RH, 7.8ms Conversion time
//#define HUMIDITY_OSR_2   // 0.010%RH, 3.9ms Conversion time
//#define HUMIDITY_OSR_1   // 0.014%RH, 2.0ms Conversion time
//#define HUMIDITY_OSR_0   // 0.020%RH, 1.0ms Conversion time

#define TEMPERATURE_OSR_3   // 0.012°C, 12.1ms Conversion time
//#define TEMPERATURE_OSR_2   // 0.016°C, 6.1ms Conversion time
//#define TEMPERATURE_OSR_1   // 0.025°C, 3.1ms Conversion time
//#define TEMPERATURE_OSR_0   // 0.040°C, 1.6ms Conversion time

#define BASE_CONVERSION 0x40  // 01000000 in binary

#ifdef HUMIDITY_OSR_3
    #define OSRRH_BITS 0x18  // 0b00011000 in binary
    uint16_t humidityDelayTime = 7.8;
#elif defined(HUMIDITY_OSR_2)
    #define OSRRH_BITS 0x10  // 0b00010000 in binary
    uint16_t humidityDelayTime = 3.9;
#elif defined(HUMIDITY_OSR_1)
    #define OSRRH_BITS 0x08  // 0b00001000 in binary
    uint16_t humidityDelayTime = 2.0;
#elif defined(HUMIDITY_OSR_0)
    #define OSRRH_BITS 0x00  // 0b00000000 in binary
    uint16_t humidityDelayTime = 1.0;
#else
    // No OSR defined. Default to fastest resolution
    #define OSRRH_BITS 0x00
    uint16_t humidityDelayTime = 1.0;
#endif

#ifdef TEMPERATURE_OSR_3
    #define OSRT_BITS 0x06  // 0b00000110 in binary
    uint16_t temperatureDelayTime = 12.1;
#elif defined(TEMPERATURE_OSR_2)
    #define OSRT_BITS 0x04  // 0b00000100 in binary
    uint16_t temperatureDelayTime = 6.1;
#elif defined(TEMPERATURE_OSR_1)
    #define OSRT_BITS 0x02  // 0b00000010 in binary
    uint16_t temperatureDelayTime = 3.1;
#elif defined(TEMPERATURE_OSR_0)
    #define OSRT_BITS 0x00  // 0b00000000 in binary
    uint16_t temperatureDelayTime = 1.6;
#else
    // No OSR defined. Default to fastest resolution
    #define OSRT_BITS 0x00  // 0b00000000 in binary
    uint16_t temperatureDelayTime = 1.6;
#endif




// HTU31D Default I2C Address
#define HTU31D_DEFAULT_I2CADDR (0x40)
#define HTU31D_I2C_ADDRESS_LOW  0x40  // when HTU31D Addr Pin = 0
#define HTU31D_I2C_ADDRESS_HIGH 0x41  // when HTU31D Addr Pin = 1

// HTU31D Commands
#define HTU31D_CONVERSION (BASE_CONVERSION | OSRRH_BITS | OSRT_BITS)
#define HTU31D_READTEMPHUM (0x00)
#define HTU31D_READSERIAL (0x0A)
#define HTU31D_HEATERON (0x04)
#define HTU31D_HEATEROFF (0x02)
#define HTU31D_RESET (0x1E)
#define HTU31D_READDIAGNOSTIC

// Diagnostic register bit definitions
#define NVM_ERROR_BIT         7
#define HUMIDITY_OVERRUN_BIT  6
#define HUMIDITY_HIGH_ERROR_BIT 5
#define HUMIDITY_LOW_ERROR_BIT 4
#define TEMP_OVERRUN_BIT      3
#define TEMP_HIGH_ERROR_BIT   2
#define TEMP_LOW_ERROR_BIT    1
#define HEATER_ON_BIT         0

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

class htu31D {
public:
  htu31D();
  ~htu31D();

  bool begin(uint8_t i2c_addr = 0x40, TwoWire *theWire = &Wire);
  bool reset(void);
  bool enableHeater(bool en);
  float readTemperature(void);
  float readHumidity(void);
  DiagnosticInfo readDiagnosticRegister(void);
  THData readTempAndHumidity(void);


private:
    TwoWire *_wire;
    uint8_t _i2caddr;
    float _humidity;
    float _temperature;
    unit16_t temperatureDelayTime;
    uint16_t humidityDelayTime;
    uint8_t htu31d_crc(uint16_t value);  
};

#endif /* _SIMPLE_HTU31D_H */
