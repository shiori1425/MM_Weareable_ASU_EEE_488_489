## Sensor Control Module Documentation

### Overview
The `SensorControl` module manages sensor operations including temperature, humidity, and bioimpedance measurements using HTU31D sensors and an AD5933 bioimpedance sensor. It provides functionality for initializing sensors, reading sensor data, calculating sweat rate based on bioimpedance, and logging sensor data to non-volatile memory (NVM).

### Functions

#### Initialization Functions
- **`void initSensors()`**
  - Initializes all sensors connected to the system.
- **`void initializeTempSensors()`**
  - Initializes and configures HTU31D temperature and humidity sensors.
- **`void initialize5933()`**
  - Sets up and configures the AD5933 bioimpedance sensor.

#### Sensor Reading Functions
- **`HTU31D::THData readSensors(HTU31D* sensorInstance)`**
  - Reads temperature and humidity data from a specified HTU31D sensor.
- **`void updateSensors(bool forceUpdate = false)`**
  - Updates all sensor readings and logs them if logging is enabled. This function can be forced to update regardless of the set update interval.

#### Diagnostic Functions
- **`void printDiagnosticInfo(HTU31D* sensorInstance)`**
  - Prints diagnostic information for a given HTU31D sensor, helping in troubleshooting and maintenance.

#### BioImpedance Sensor Functions
- **`void frequencySweepRaw(float* res)`**
  - Performs a frequency sweep with the AD5933 and calculates the bioimpedance.
- **`void calibrateAD5933()`**
  - Calibrates the AD5933 bioimpedance sensor using known reference resistance and stores calibration constants.

#### Sweat Rate Calculation Functions
- **`void calcSweatRate(float* sweatRate)`**
  - Calculates the sweat rate based on sensor data, body measurements, and calculated bioimpedance.

#### Data Logging Functions
- **`void logSensorDataToNVM()`**
  - Logs current sensor data to non-volatile memory.
- **`void printSensorLog()`**
  - Prints logged sensor data from NVM to the serial monitor.
- **`void eraseLoggedSensorData()`**
  - Clears all sensor data logged in NVM.

#### Calibration Data Management
- **`void readCalConstantsFromMemory()`**
  - Reads calibration constants for the AD5933 from NVM.
- **`void writeCalConstantsToMemory()`**
  - Writes current calibration constants for the AD5933 to NVM.
- **`void eraseCalConstants()`**
  - Clears calibration data for the AD5933 from NVM.

#### Miscellaneous Functions
- **`void zeroSweatRate()`**
  - Resets the sweat rate calibration to zero.
- **`void toggleSensorLogging()`**
  - Toggles the logging of sensor data on and off.

### Dependencies

- **Libraries**: Arduino, AD5933, HTU31D
- **External Files**: functions.h, Types.h

This module is central to the operation of the device's sensing capabilities, handling both the gathering and processing of environmental and physiological data to support health monitoring functions.
