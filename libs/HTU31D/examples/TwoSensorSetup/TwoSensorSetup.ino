#include "Arduino.h"
#include "Wire.h"
#include "esp32-hal-i2c.h"
#include "HTU31D.h"

/* Temp Humidity I2C lines */
#define PIN_THIC_SCL                 44
#define PIN_THIC_SDA                 43

#define HTU31D_I2C_ADDRESS_LOW  0x40  // when HTU31D Addr Pin = 0
#define HTU31D_I2C_ADDRESS_HIGH 0x41  // when HTU31D Addr Pin = 1

/* Configure additional IO*/

const unsigned long SENSOR_READ_INTERVAL = 60000;  // 60 seconds in milliseconds
unsigned long lastSensorReadTime = SENSOR_READ_INTERVAL + 1;

/* Setup I2C bus connections*/
TwoWire SensorsI2C = TwoWire(1); // Create another I2C bus instance for sensor reads

HTU31D* htu31d_body = new HTU31D();
HTU31D* htu31d_ambi = new HTU31D();
  
HTU31D::THData sensorData_body;
HTU31D::THData sensorData_ambi;


bool heaterEnabled = false;
float temp;
float humidity;

void setup() {

  Serial.begin(115200);
  delay(1000); // Delay to let serial initialization complete
  Serial.println("Starting Up");

  // Start I2C
  SensorsI2C.begin(PIN_THIC_SDA, PIN_THIC_SCL); // Initialize Sensor I2C BUS
  SensorsI2C.setClock(400000);  // max clock speed for HTU31D
  delay(1);


  if(!htu31d_body->begin(HTU31D_I2C_ADDRESS_HIGH, &SensorsI2C)){
    Serial.println("HTU_INT Failed to init first try");
  }

  if(!htu31d_ambi->begin(HTU31D_I2C_ADDRESS_LOW, &SensorsI2C)){
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

  Serial.println("Setup Complete");
}

void loop() {


    unsigned long currentMillis = millis();
    if (currentMillis - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
      lastSensorReadTime = currentMillis;
      Serial.println("Loop: - - - - - - - - - - - - - - - - - - -");
      Serial.println("Loop: Body Sensor");
      //sensorData_body = readSensors(htu31d_body);
      sensorData_body.temperature = readTemperature(htu31d_body);
      sensorData_body.humidity = readHumidity(htu31d_body);
      printDiagnosticInfo(htu31d_body);
      delay(10);
      Serial.println("Loop: Ambient Sensor");
      //sensorData_ambi = readSensors(htu31d_ambi);
      sensorData_ambi.temperature = readTemperature(htu31d_ambi);
      sensorData_ambi.humidity = readHumidity(htu31d_ambi);
      printDiagnosticInfo(htu31d_ambi);
    }
}


HTU31D::THData readSensors(HTU31D* sensorInstance) {
    HTU31D::THData data;  // Declare an instance of the struct
    #ifdef DEBUG_ENABLED
      Serial.println("Attempting to read External Sensor!");
    #endif

    if (sensorInstance->readTempAndHumidity(&data)) {
        // Successfully read data
    } else {
        // Failed to read data
        Serial.println("Failed to read successfully!");
        return data;

    }

    Serial.println("Sensor read successful!");
    Serial.print("Temperature: ");
    Serial.println(data.temperature);
    Serial.print("Humidity: "); 
    Serial.println(data.humidity);
    return data;
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

float readTemperature(HTU31D* sensorInstance){
  float temperature;
  if (sensorInstance->readTemperature(&temperature)){
    Serial.print("Temperature: ");
    Serial.println(temperature);
    return temperature;
  }else{
    Serial.println("ReadTemp: Sensor Read failed");
    return NAN;
  }
}

float readHumidity(HTU31D* sensorInstance){
  float humidity;
  if (sensorInstance->readHumidity(&humidity)){
    Serial.print("Humidity: ");
    Serial.println(humidity);
    return humidity;
  }else{
    Serial.println("ReadHumid: Sensor Read failed");
    return NAN;
  }
}