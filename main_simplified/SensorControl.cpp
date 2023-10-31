#include <type_traits>
#include "SensorControl.h"

/* Temp/Humidity Sensor Setup*/
HTU31D* htu31d_body = new HTU31D();
HTU31D* htu31d_ambi = new HTU31D();
  
HTU31D::THData sensorData_body;
HTU31D::THData sensorData_ambi;

float sweatRate = 0;
float skinRes = 0;

void initializeTempSensors() {
  if(!htu31d_body->begin(0x40, &SensorsI2C)){
    Serial.println("HTU_INT Failed to init first try");
  }

  if(!htu31d_ambi->begin(0x41, &SensorsI2C)){
    Serial.println("HTU_ext Failed to init first try");
  }
  delay(1);
  uint32_t serial;
  Serial.println("Setup: Body Sensor");
  htu31d_body->readSerial(&serial);
  Serial.print("Setup:Serial ");
  Serial.println(serial);
  htu31d_body->disableHeater();
  sensorData_body = readSensors(htu31d_body);
  printDiagnosticInfo(htu31d_body);

  delay(10);

  Serial.println("Setup: Ambient Sensor");
  htu31d_ambi->readSerial(&serial);
  Serial.print("Setup:Serial ");
  Serial.println(serial);
  htu31d_ambi->disableHeater();
  sensorData_ambi = readSensors(htu31d_ambi);
  printDiagnosticInfo(htu31d_ambi);

}

HTU31D::THData readSensors(HTU31D* sensorInstance) {
    HTU31D::THData data;  // Declare an instance of the struct
    #ifdef DEBUG_ENABLED
      Serial.println("Attempting to read sensor data!");
    #endif

    if (sensorInstance->readTempAndHumidity(&data)) {
        // Successfully read data
    } else {
        // Failed to read data
        Serial.println("Failed to read successfully!");
        return data;

    }
    #ifdef DEBUG_ENABLED
      Serial.println("Sensor read successful!");
      Serial.print("Temperature: ");
      Serial.println(data.temperature);
      Serial.print("Humidity: "); 
      Serial.println(data.humidity);
    #endif
    return data;
}

void logSensorDataToNVM() {
    // Replace with your actual logging code
    // This could be writing to EEPROM, SPIFFS, or using the NVS library for ESP32
    // Log external data
      ///htu_ext.getEvent(&humidity, &temp);
    //log internal data
      ///htu_int.getEvent(&humidity, &temp);
    // log skin res

    Serial.println("Logged Data");
}

void readSkinRes(float* skinRes){
    // implement code later to read skin res
    float rand = random(800000,1200000);
    *skinRes = rand;
    return;
}

void calcSweatRate(float* sweatRate){
    // implement code later to read skin res
    *sweatRate = random(0.9,8.7);
    return;
}

void updateSensors(){
  // This variable will keep the last time the sensors were updated.
  // 'static' means this variable retains its value across function calls.
  static unsigned long lastUpdateTime = 0;

  // Current time since the board started.
  unsigned long currentTime = millis();

  // Check if it's been more than 60 seconds since the last update.
  if (currentTime - lastUpdateTime >= 60000 || lastUpdateTime == 0) {
    // Read Body Temp Sensor
    sensorData_body = readSensors(htu31d_body); 
    // Read Ambient Temp Sensor
    sensorData_ambi = readSensors(htu31d_ambi); 
    // Read Skin Resistance
    readSkinRes(&skinRes);

    // Update the last update time.
    lastUpdateTime = currentTime;
  }

  return;
}

void printDiagnosticInfo(HTU31D* sensorInstance) {
  HTU31D::DiagnosticInfo info = {};

  if (sensorInstance->readDiagnosticRegister(&info)){

    Serial.print("NVM Error: ");
    Serial.println(info.nvmError ? "True" : "False");

    Serial.print("Humidity Overrun: ");
    Serial.println(info.humidityOverrun ? "True" : "False");

    Serial.print("Humidity High Error: ");
    Serial.println(info.humidityHighError ? "True" : "False");

    Serial.print("Humidity Low Error: ");
    Serial.println(info.humidityLowError ? "True" : "False");

    Serial.print("Temperature Overrun: ");
    Serial.println(info.tempOverrun ? "True" : "False");

    Serial.print("Temperature High Error: ");
    Serial.println(info.tempHighError ? "True" : "False");

    Serial.print("Temperature Low Error: ");
    Serial.println(info.tempLowError ? "True" : "False");

    Serial.print("Heater On: ");
    Serial.println(info.heaterOn ? "True" : "False");
  } else {
    //Serial.println("Failed to read diagnostic register");
  }

}