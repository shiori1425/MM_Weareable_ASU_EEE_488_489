## Functions Module Documentation

### Overview
The `Functions` module provides a collection of utility functions that support various operations such as temperature conversions, I2C communication setup, WiFi connectivity, battery voltage reading, and more. This module also handles more complex operations like NTP time synchronization, heat index calculation, and logging battery data to non-volatile memory (NVM).

### Functions

- **`void debugPrint(const char* message)`**
  - Prints debug messages to the serial monitor.
  
- **`const char* formatResistance(float resistance, char* buffer)`**
  - Formats the resistance value into a human-readable string with appropriate units (Ohms, Kiloohms, Megaohms, Gigaohms), depending on the magnitude of the resistance.
  
- **`float change_to_f(float temp_c)`**
  - Converts a temperature from Celsius to Fahrenheit.
  
- **`float change_to_C(float temp_f)`**
  - Converts a temperature from Fahrenheit to Celsius.
  
- **`void initIO()`**
  - Initializes input/output pins and I2C communication settings. This includes setting up pins as input or output based on their intended use, and initializing the I2C bus for sensor data communication.
  
- **`void initWiFi()`**
  - Initializes and connects to a WiFi network. It tries multiple times if the initial attempts fail, and outputs the status and IP address upon successful connection.
  
- **`void checkWiFiClient()`**
  - Checks for WiFi client connections and handles client communication. If a new client connects, a greeting message is sent.
  
- **`void initializeNTP()`**
  - Sets up Network Time Protocol (NTP) to keep the system time synchronized.
  
- **`void set_time_from_wifi()`**
  - Waits for the NTP to set the system time and adjusts the time according to the specified time zone offset.
  
- **`double calculateHeatIndex(double temp, double humidity)`**
  - Calculates the heat index using both the simple formula for lower temperatures and the Rothfusz regression for higher temperatures, with adjustments for specific humidity and temperature conditions.
  
- **`float readBatteryVoltage()`**
  - Reads the battery voltage through an ADC channel, calibrates the reading, and logs it if battery logging is enabled.
  
- **`void logBatteryToNVM(uint32_t bat_volt)`**
  - Logs battery voltage data to non-volatile memory (NVM) using a key-value system with timestamps.
  
- **`void printBatteryLog()`**
  - Retrieves and prints all battery voltage logs from NVM to the serial monitor.
  
- **`void eraseBatteryLog()`**
  - Clears all battery log data from the non-volatile memory.
  
- **`void toggleBatteryLogging()`**
  - Toggles the logging of battery voltage data on and off.

### Dependencies

- **Libraries**: Wire, TimeLib, WiFi, SPIFFS, esp_adc_cal, Preferences
- **External Files**: pin_config.h, Types.h

This module serves as a core part of the system, providing essential utility functions for device setup, configuration, and maintenance activities.
