/*!
 * @file HTU31D.h
 */

#ifndef _HTU31D_H
#define _HTU31D_H

#include <BusIO_Register.h>
#include <I2CDevice.h>
#include <Sensor.h>

/** Default I2C address for the HTU21D. */
#define HTU31D_DEFAULT_I2CADDR (0x40)

/** Read temperature and humidity. */
#define HTU31D_READTEMPHUM (0x00)

/** Start a conversion! */
#define HTU31D_CONVERSION (0x40)

/** Read serial number command. */
#define HTU31D_READSERIAL (0x0A)

/** Enable heater */
#define HTU31D_HEATERON (0x04)

/** Disable heater */
#define HTU31D_HEATEROFF (0x02)

/** Reset command. */
#define HTU31D_RESET (0x1E)

class HTU31D;

/**
 * @brief  Adafruit Unified Sensor interface for the humidity sensor component
 * of HTU31D
 *
 */
class HTU31D_Humidity : public Sensor {
public:
  /** @brief Create an Sensor compatible object for the humidity sensor
    @param parent A pointer to the HTU31D class */
  HTU31D_Humidity(HTU31D *parent) { _theHTU31D = parent; }
  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0x31D1;
  HTU31D *_theHTU31D = NULL;
};

/**
 * @brief Adafruit Unified Sensor interface for the temperature sensor component
 * of HTU31D
 *
 */
class HTU31D_Temp : public Sensor {
public:
  /** @brief Create an Sensor compatible object for the temp sensor
      @param parent A pointer to the HTU31D class */
  HTU31D_Temp(HTU31D *parent) { _theHTU31D = parent; }

  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0x31D0;
  HTU31D *_theHTU31D = NULL;
};

/**
 * Driver for the Adafruit HTU31D breakout board.
 */
class HTU31D {
public:
  HTU31D();
  ~HTU31D(void);

  bool begin(uint8_t i2c_addr = HTU31D_DEFAULT_I2CADDR,
             TwoWire *theWire = &Wire);
  bool reset(void);
  uint32_t readSerial(void);

  bool enableHeater(bool en);

  bool getEvent(sensors_event_t *humidity, sensors_event_t *temp);
  Sensor *getTemperatureSensor(void);
  Sensor *getHumiditySensor(void);

protected:
  float _temperature, ///< Last reading's temperature (C)
      _humidity;      ///< Last reading's humidity (percent)

  uint16_t _sensorid_humidity; ///< ID number for humidity
  uint16_t _sensorid_temp;     ///< ID number for temperature

  I2CDevice *i2c_dev = NULL;       ///< Pointer to I2C bus interface
  HTU31D_Temp *temp_sensor = NULL; ///< Temp sensor data object
  HTU31D_Humidity *humidity_sensor =
      NULL; ///< Humidity sensor data object

private:
	TwoWire *_wire;
  friend class HTU31D_Temp;     ///< Gives access to private members to
                                         ///< Temp data object
  friend class HTU31D_Humidity; ///< Gives access to private members to
                                         ///< Humidity data object
  void fillTempEvent(sensors_event_t *temp, uint32_t timestamp);
  void fillHumidityEvent(sensors_event_t *humidity, uint32_t timestamp);
};

#endif /* _HTU31D_H */
