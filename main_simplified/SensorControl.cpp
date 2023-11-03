#include <type_traits>
#include "SensorControl.h"

/* Temp/Humidity Sensor Setup*/
HTU31D* htu31d_body = new HTU31D();
HTU31D* htu31d_ambi = new HTU31D();
HTU31D::THData sensorData_body;
HTU31D::THData sensorData_ambi;

float sweatRate = 0;

/* BioImpedance Sensor Setup*/
#define START_FREQ  (5000)
#define FREQ_INCR   (15000)
#define NUM_INCR    (6)
#define REF_RESIST  (200000)

double gain[NUM_INCR+1];
float phase[NUM_INCR+1];
float skinRes = 0;


/* HTU31D Functions*/
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
    frequencySweepRaw(&skinRes);
    //readSkinRes(&skinRes);

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

/* AD5933 Functions*/

void initialize5933() {
    Serial.println("Initializing AD5933...");

    Serial.print("Starting communication with AD5933...");
    if (!AD5933::begin(&SensorsI2C)) {
        Serial.println("FAILED in starting communication with AD5933!");
        while (true);
    }
    Serial.println("Communication started successfully!");

    Serial.print("Setting internal clock...");
    if (!AD5933::setInternalClock(true)) {
        Serial.println("FAILED to set internal clock!");
        while (true);
    }
    Serial.println("Internal clock set successfully!");

    Serial.print("Setting start frequency...");
    if (!AD5933::setStartFrequency(START_FREQ)) {
        Serial.println("FAILED to set start frequency!");
        while (true);
    }
    Serial.println("Start frequency set successfully!");

    Serial.print("Setting increment frequency...");
    if (!AD5933::setIncrementFrequency(FREQ_INCR)) {
        Serial.println("FAILED to set increment frequency!");
        while (true);
    }
    Serial.println("Increment frequency set successfully!");

    Serial.print("Setting number of increments...");
    if (!AD5933::setNumberIncrements(NUM_INCR)) {
        Serial.println("FAILED to set number of increments!");
        while (true);
    }
    Serial.println("Number of increments set successfully!");

    Serial.print("Setting PGA gain...");
    if (!AD5933::setPGAGain(PGA_GAIN_X1)) {
        Serial.println("FAILED to set PGA gain!");
        while (true);
    }
    Serial.println("PGA gain set successfully!");

    Serial.print("Starting calibration...");
    if (AD5933::calibrate(gain, phase, REF_RESIST, NUM_INCR+1)) {
        Serial.println("Calibrated!");
    } else {
        Serial.println("Calibration failed...");
    }
}

void frequencySweepRaw(float* res) {
    // Create variables to hold the impedance data and track frequency
    int real, imag, i = 0, cfreq = START_FREQ/1000;

    // Initialize the frequency sweep
    if (!(AD5933::setPowerMode(POWER_STANDBY) &&          // place in standby
          AD5933::setControlMode(CTRL_INIT_START_FREQ) && // init start freq
          AD5933::setControlMode(CTRL_START_FREQ_SWEEP))) // begin frequency sweep
         {
             Serial.println("Could not initialize frequency sweep...");
         }

    // Perform the actual sweep
    while ((AD5933::readStatusRegister() & STATUS_SWEEP_DONE) != STATUS_SWEEP_DONE) {
        // Get the frequency data for this frequency point
        if (!AD5933::getComplexData(&real, &imag)) {
            Serial.println("Could not get raw frequency data...");
        }

        
        // Print out the frequency data
        //Serial.print(cfreq);
        //Serial.print(": R=");
        //Serial.print(real);
        //Serial.print("/I=");
        //Serial.print(imag);

        // Compute impedance
        double magnitude = sqrt(pow(real, 2) + pow(imag, 2));
        double impedance = 1/(magnitude*gain[i]);
        //Serial.print("  |Z|=");
        //Serial.println(impedance);
        *res += impedance;


        // Increment the frequency
        i++;
        cfreq += FREQ_INCR/1000;
        AD5933::setControlMode(CTRL_INCREMENT_FREQ);
    }
    *res = *res / (NUM_INCR + 1); 
    Serial.print("|Z|=");
    Serial.println(*res);
    return;
}

void readSkinRes(float* skinRes){
    // implement code later to read skin res
    float rand = random(800000,1200000);
    *skinRes = rand;
    return;
}

/* Sweat Rate Functions */
void calcSweatRate(float* sweatRate){
    // implement code later to read skin res
    *sweatRate = random(0.9,8.7);
    return;
}

/* Data Logging Functions */
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