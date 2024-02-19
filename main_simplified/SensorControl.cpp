#include <type_traits>
#include "SensorControl.h"

#define DEBUG_ENABLED

/* Temp/Humidity Sensor Setup*/
HTU31D* htu31d_body = new HTU31D();
HTU31D* htu31d_ambi = new HTU31D();
HTU31D::THData sensorData_body;
HTU31D::THData sensorData_ambi;

float sweatRate = 0;

/* BioImpedance Sensor Setup*/
#define START_FREQ  (49500)
#define FREQ_INCR   (500)
#define NUM_INCR    (2)
#define REF_RESIST  (2000)

double gain[NUM_INCR+1];
float phase[NUM_INCR+1];
float skinRes = 0;

const float emissivity_skin = 0.95;
const float stefan_boltzmann_constant = 5.67e-8;
const float h_c = 3;

Preferences preferences;

bool psramInitialized = psramFound();
// Define the size of the PSRAM buffer
const size_t bufferSize = 100; 

// Buffer for storing sweat rate values
float *sweatRateBuffer;

// Variables to manage the buffer
size_t bufferHead = 0;
size_t bufferTail = 0;
size_t bufferCount = 0;

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

void updateSensors(bool forceUpdate){
  // This variable will keep the last time the sensors were updated.
  // 'static' means this variable retains its value across function calls.
  static unsigned long lastUpdateTime = 0;

  // Current time since the board started.
  unsigned long currentTime = millis();

  // Check if it's been more than 60 seconds since the last update.
  if (currentTime - lastUpdateTime >= 60000 || lastUpdateTime == 0 || forceUpdate) {
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
    }
    else{
      Serial.println("Communication started successfully!");
    }

    Serial.print("Setting internal clock...");
    if (!AD5933::setInternalClock(true)) {
        Serial.println("FAILED to set internal clock!");
    }
    else{
      Serial.println("Internal clock set successfully!");
    }

    Serial.print("Setting start frequency...");
    if (!AD5933::setStartFrequency(START_FREQ)) {
        Serial.println("FAILED to set start frequency!");
    }
    else{
      Serial.println("Start frequency set successfully!");
    }

    Serial.print("Setting increment frequency...");
    if (!AD5933::setIncrementFrequency(FREQ_INCR)) {
        Serial.println("FAILED to set increment frequency!");
    }
    else{
      Serial.println("Increment frequency set successfully!");
    }

    Serial.print("Setting number of increments...");
    if (!AD5933::setNumberIncrements(NUM_INCR)) {
        Serial.println("FAILED to set number of increments!");
    }
    else{
      Serial.println("Number of increments set successfully!");
    }

    Serial.print("Setting PGA gain...");
    if (!AD5933::setPGAGain(PGA_GAIN_X1)) {
        Serial.println("FAILED to set PGA gain!");
    }
    else{
      Serial.println("PGA gain set successfully!");
    }

    Serial.print("Setting Output Rainge...");
    if (!AD5933::setRange(CTRL_OUTPUT_RANGE_4)) {
        Serial.println("FAILED to set Output Rainge!");
    }
    else{
      Serial.println("Output Rainge set successfully!");
    }

    Serial.print("Loading calibration data...");
    readCalConstantsFromMemory();

}

void frequencySweepRaw(float* res) {
    // Create variables to hold the impedance data and track frequency
    int real, imag, i = 0, validReadings = 0;
    float cfreq = START_FREQ;
    double totalImpedance = 0; // Use to accumulate valid impedance readings

    // Initialize the frequency sweep
    if (!(AD5933::setPowerMode(POWER_STANDBY) &&          // place in standby
          AD5933::setControlMode(CTRL_INIT_START_FREQ) && // init start freq
          AD5933::setControlMode(CTRL_START_FREQ_SWEEP))) // begin frequency sweep
    {
        Serial.println("Could not initialize frequency sweep...");
        *res = 0; // Ensure output is set to 0 if initialization fails
        return;
    }

    // Perform the actual sweep
    while ((AD5933::readStatusRegister() & STATUS_SWEEP_DONE) != STATUS_SWEEP_DONE) {
        // Get the frequency data for this frequency point
        if (!AD5933::getComplexData(&real, &imag)) {
            Serial.println("Could not get raw frequency data...");
            continue; // Skip this iteration if data retrieval fails
        }

        // Ignore steps with 0 for real or imag component
        if (real == 0 || imag == 0) {
            Serial.println("Skipping step with zero in real or imaginary component...");
            cfreq += FREQ_INCR; // Still need to increment frequency
            AD5933::setControlMode(CTRL_INCREMENT_FREQ);
            continue;
        }
        #ifdef DEBUG_ENABLED
          // Debug print values 
          // Print out the frequency data
          Serial.print(cfreq);
          Serial.print(": R=");
          Serial.print(real);
          Serial.print("/I=");
          Serial.println(imag);

          // Compute and print impedance
          double magnitude = sqrt(pow(real, 2) + pow(imag, 2));
          double impedance = 1 / (magnitude * gain[i]);
          Serial.print("Mag=");
          Serial.println(magnitude);
          Serial.print("Gain=");
          Serial.println(gain[i]);
          Serial.print("  |Z|=");
          Serial.println(impedance);
        #endif

        // Accumulate impedance and count valid readings
        totalImpedance += impedance;
        validReadings++;

        // Increment the frequency
        i++;
        cfreq += FREQ_INCR;
        AD5933::setControlMode(CTRL_INCREMENT_FREQ);
    }

    if (validReadings > 0) {
        // Calculate average impedance from valid readings
        *res = totalImpedance / validReadings;
        Serial.print("Average |Z|=");
        Serial.println(*res);
    } else {
        // If no valid readings were obtained, output 0 resistance
        *res = 0;
        Serial.println("No valid readings obtained, resistance set to 0.");
    }
}

void calibrateAD5933(){
    Serial.println("Starting calibration... ");
    Serial.printf("Reference Resistance: %d \n", REF_RESIST);
    if (AD5933::calibrate(gain, phase, REF_RESIST, NUM_INCR+1)) {
        Serial.println("Calibrated!");
        writeCalConstantsToMemory();
    } else {
        Serial.println("Calibration failed...");
    }
    for (int i = 0; i < NUM_INCR+1; i++) {
        Serial.println(gain[i], 6); // Print each element, with 6 digits of precision for doubles
    }
}

void readCalConstantsFromMemory(){
    // Load the saved gain and phase arrays from memory.
    Serial.println("Reading Cal Constants from memory.");
    preferences.begin("my-app", true); // Open in read-only mode
    // Load gain values
    for (int i = 0; i <= NUM_INCR; i++) {
      String key = "gain" + String(i); // Use the same unique key for each element
      gain[i] = preferences.getDouble(key.c_str(), 1.0); // Default to 0.0 if not found
    }

    // Load phase values
    for (int i = 0; i <= NUM_INCR; i++) {
      String key = "phase" + String(i); // Use the same unique key for each element
      phase[i] = preferences.getFloat(key.c_str(), 1.0f); // Default to 0.0f if not found
    }

    preferences.end(); // Close the preferences
}

void writeCalConstantsToMemory(){
    Serial.println("Writing Cal Constants to memory.");
    preferences.begin("my-app", false); // Open in read-write mode

    // Store gain values
    for (int i = 0; i <= NUM_INCR; i++) {
      String key = "gain" + String(i); // Create a unique key for each element
      preferences.putDouble(key.c_str(), gain[i]);
    }

    // Store phase values
    for (int i = 0; i <= NUM_INCR; i++) {
      String key = "phase" + String(i); // Create a unique key for each element
      preferences.putFloat(key.c_str(), phase[i]);
    }

    preferences.end(); // Close the preferences
}

/* Sweat Rate Functions */
void calcSweatRate(float* sweatRate, float height, float weight, float metabolic_rate){
    
    // Convert height and weight
    // 1 in = 2.54cm
    float height_in_cm = height * 2.54;
    float weight_in_kg = weight * 0.453592;

    // Calculate Body Surface Area (BSA) using Du Bois formula  (m^2)
    float BSA = 0.007184 * pow(height_in_cm, 0.725) * pow(weight_in_kg, 0.425);

    // Calculate saturated vapor pressures using Magnus-Tetens approximation
    float e_s_ambient = 6.11 * pow(10, (17.625 * sensorData_ambi.temperature / (243.04 + sensorData_ambi.temperature)));
    float e_s_skin = 6.11 * pow(10, (17.625 * sensorData_body.temperature / (243.04 + sensorData_body.temperature)));

    // Calculate actual vapor pressure
    float e_ambient = sensorData_ambi.humidity * e_s_ambient / 100;
    float e_skin = sensorData_body.humidity * e_s_skin / 100;

    // Calculate Delta P water vapor and convert from hPa to Pa
    float delta_P_water_vapor = (e_skin - e_ambient) * 100;

    // Calculate Emax
    float Emax = (delta_P_water_vapor * BSA * 0.50) * (1 - REF_RESIST/skinRes);

    // Convert temperatures to Kelvin
    float T_body_K = sensorData_body.temperature + 273.15;
    float T_ambient_K = sensorData_ambi.temperature + 273.15;

    // Calculate Ereq
    float H_c = 6.45 * BSA * (T_ambient_K - T_body_K);
    float H_r = 1.5 * BSA;
    float H_l = 0.04 * BSA * stefan_boltzmann_constant * (pow(T_ambient_K, 4)+273.15);
    float Ereq = metabolic_rate + H_c + H_r + H_l;

    // Calculate sweat rate (mL/min)
    float calculatedSweatRate = 147 + 1.527 * Ereq - 0.87 * Emax;
    calculatedSweatRate = calculatedSweatRate / 60;
    // An average sweat rate during exersice is 15 to 25 mL/min

    // Assign the calculated value to the pointer
    //*sweatRate = (calculatedSweatRate > 0) ? calculatedSweatRate : 0;
    *sweatRate = calculatedSweatRate; 
    #ifdef DEBUG_ENABLED
      // Debug print values 
      Serial.println("-------  Calculate Sweat -------");
      printf("Sweat Rate in: %f\n", *sweatRate);
      printf("height: %f\n", height);
      printf("weight: %f\n", weight);
      printf("metabolic_rate: %f\n", metabolic_rate);
      printf("delta_P_water_vapor: %f\n", delta_P_water_vapor);
      printf("e_skin: %f\n", e_skin);
      printf("sensorData_ambi.humidity: %f\n", sensorData_ambi.humidity);
      printf("T_body_K: %f\n", T_body_K);
      printf("Emax: %f\n", Emax);
      printf("Ereq: %f\n", Ereq);
      printf("sweatRate out: %f\n", *sweatRate); 
      Serial.println("------------------------------");
    #endif
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

    //Serial.println("Logged Data");
}