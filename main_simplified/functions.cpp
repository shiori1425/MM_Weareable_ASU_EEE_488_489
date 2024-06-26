#include "IPAddress.h"
#define DEBUG_ENABLED

#include "functions.h"

Preferences preferences;

long _UTC_OFF;
bool _battery_logging;

WiFiServer server(23);
WiFiClient client;
String ipAddress;


/* Setup I2C bus connections*/
TwoWire SensorsI2C = TwoWire(1); // Create another I2C bus instance for sensor reads

const char* formatResistance(float resistance, char* buffer) {
    char format[10]; // Buffer for the format string
    int decimalPlaces;

    if (resistance >= 1e9) {
        decimalPlaces = (resistance >= 10e9) ? 1 : 2; // Adjust for 10+ Giga
        sprintf(format, "%%.%dfG", decimalPlaces);
        sprintf(buffer, format, resistance / 1e9);
    } else if (resistance >= 1e6) {
        decimalPlaces = (resistance >= 10e6) ? 1 : (resistance >= 1e6) ? 2 : 3; // Adjust for Mega
        sprintf(format, "%%.%dfM", decimalPlaces);
        sprintf(buffer, format, resistance / 1e6);
    } else if (resistance >= 1e3) {
        decimalPlaces = (resistance >= 10e3) ? 1 : (resistance >= 1e3) ? 2 : 3; // Adjust for Kilo
        sprintf(format, "%%.%dfK", decimalPlaces);
        sprintf(buffer, format, resistance / 1e3);
    } else {
        decimalPlaces = (resistance >= 100) ? 1 : (resistance >= 10) ? 2 : 3; // For below 1K
        sprintf(format, "%%.%df", decimalPlaces);
        sprintf(buffer, format, resistance);
    }

    return buffer;
}

float change_to_f(float temp_c){
  return (temp_c * 9.0/5.0) + 32.0;
}

float change_to_C(float temp_f){
  return (temp_f - 32.0) * 5.0/9.0;
}

void initIO(){
  #ifdef DEBUG_ENABLED
      Serial.println("Initializing IO");
  #endif

  // Configure Pins
  gpio_hold_dis((gpio_num_t)PIN_TOUCH_RES);
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);
  pinMode(PIN_TOUCH_RES, OUTPUT);
  digitalWrite(PIN_TOUCH_RES, LOW);
  delay(500);
  digitalWrite(PIN_TOUCH_RES, HIGH);
  pinMode(PIN_BAT_VOLT, INPUT); 
  pinMode(PIN_GPIO_02, INPUT);
  pinMode(PIN_GPIO_03, INPUT);

  #ifdef DEBUG_ENABLED
      Serial.println("IO pins set. Starting I2C...");
  #endif

  Wire.begin(PIN_IIC_SDA, PIN_IIC_SCL);
  Wire.setClock(400000);   // Max clock speed for Touch Controller
  delay(1);

  #ifdef DEBUG_ENABLED
      Serial.println("I2C initialized.");
  #endif
}

void initWiFi(){
  // Check if wifi is connected already so we can all this func to reconnect as well
  if (!WiFi.isConnected()){
    int retry = 0;
    const int retry_count = 5;
    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED && ++retry < retry_count) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
      #ifdef DEBUG_ENABLED
        delay(5);
        Serial.println("Wifi Status:"); 
        Serial.println(WiFi.status());
      #endif      
    }
    if (WiFi.isConnected()) {
      Serial.println("Connected to WiFi");
      Serial.println("IP Address: ");
      Serial.println(WiFi.localIP());
      Serial.print("RRSI (signal strength): ");
      Serial.println(WiFi.RSSI());

      // Store IP address to display to user on device
      ipAddress = WiFi.localIP().toString();  
      server.begin();
    }
  }
}

void checkWiFiClient() {
  
  // Check for clients only if wifi is connected
  if(WiFi.isConnected()){
    // Check if a client is currently connected
    if (!client || !client.connected()) {
      if(client) {
        client.stop(); // Properly close the existing connection
        Serial.println("Client disconnected.");
      }
      client = server.available(); // Check for new client connections
      if(client) {
        Serial.println("Client connected.");
        wifiSerial("Hello from the Moisture Mavericks!");
      }
    }
  }
}

void wifiSerial(const String& str) {
    String messageWithNewLine = str + "\n"; 
    if (client.connected()) {
        client.write((const uint8_t*)messageWithNewLine.c_str(), messageWithNewLine.length());
    }
}

void initializeNTP() {
  #ifdef DEBUG_ENABLED
      Serial.println("Initializing NTP...");
  #endif

  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_init();

  set_time_from_wifi();

  #ifdef DEBUG_ENABLED
      Serial.println("NTP initialization completed.");
  #endif
}

void set_time_from_wifi(){

  // Wait till NTP sets the time
  time_t now = 0;
  struct tm timeinfo = { 0 };
  int retry = 0;
  const int retry_count = 3;
  while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
      delay(1000);
      time(&now);
      localtime_r(&now, &timeinfo);
      #ifdef DEBUG_ENABLED
          Serial.println("Waiting for NTP to set the time...");
      #endif
  }

  // Set the time for TimeLib.h
  if (timeinfo.tm_year >= (2016 - 1900)) {
      setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
      #ifdef DEBUG_ENABLED
          Serial.println("Time set via NTP.");
      #endif
  }

    // read offset from Flash on setup in future....
  const long TZ = _UTC_OFF * 3600;  // 4 hours * 3600 seconds/hour
  adjustTime(TZ);
}

double calculateHeatIndex(double temp, double humidity) {
    // convert Celsius to Fahrenheit
    temp = temp * 9 / 5 + 32; 

    double heatIndex;
    
    // Use the simple formula for low heat indices
    if (temp < 80) {
        heatIndex = 0.5 * (temp + 61.0 + ((temp - 68.0) * 1.2) + (humidity * 0.094));
    } else {
        // Rothfusz regression
        heatIndex = -42.379 + 2.04901523 * temp + 10.14333127 * humidity
                    - 0.22475541 * temp * humidity - 0.00683783 * temp * temp
                    - 0.05481717 * humidity * humidity + 0.00122874 * temp * temp * humidity
                    + 0.00085282 * temp * humidity * humidity - 0.00000199 * temp * temp * humidity * humidity;

        // Adjustments
        if ((humidity < 13) && (temp >= 80.0) && (temp <= 112.0)) {
            double adjustment = ((13.0 - humidity) / 4.0) * sqrt((17.0 - abs(temp - 95.0)) / 17.0);
            heatIndex -= adjustment;
        } else if ((humidity > 85.0) && (temp >= 80.0) && (temp <= 87.0)) {
            double adjustment = ((humidity - 85.0) / 10.0) * ((87.0 - temp) / 5.0);
            heatIndex += adjustment;
        }
    }

    return heatIndex;
}

float readBatteryVoltage(){
  esp_adc_cal_characteristics_t adc_chars;

  // Get the internal calibration value of the chip
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  uint32_t raw = analogRead(PIN_BAT_VOLT);
  uint32_t v1 = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2; 
  static unsigned long lastUpdateTime = 0;
  // Current time since the board started.
  unsigned long currentTime = millis();
  // Log Battery voltage every 2 minutes if enables
  if (currentTime - lastUpdateTime >= 90000 || lastUpdateTime == 0) {
    // Enable the battery level logging by 
    if(_battery_logging){
      logBatteryToNVM(v1);
    }
    lastUpdateTime = currentTime;
  }
  
  return v1;
}

/* Data Logging Functions */
void logBatteryToNVM(uint32_t bat_volt) {

    Serial.println("Logging battery data to memory");

    preferences.begin("bat_log", false);

    int log_index = preferences.getInt("index", 0);

    if (preferences.freeEntries() < 4){
      log_index = 0;
    }

    String key = "batteryLog" + String(log_index); // Create a unique key for each element

    char timeStr[10];
    sprintf(timeStr, "%02d:%02d:%02d", hourFormat12(),minute(), second());

    // Serialize the sensor readings into a string format, e.g., "timestamp,sensor1,sensor2,...;"
    String newEntry = String(timeStr) + "," + String(bat_volt);

    // Write sensor data to memory
    bool putSuccess  = preferences.putString(key.c_str(), newEntry);

    wifiSerial(newEntry);

    if (putSuccess){
      preferences.putInt("index", log_index+1);
    }
    
    preferences.end();
}

void printBatteryLog() {
    
    preferences.begin("bat_log", false);
    Serial.println(" - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ");
    Serial.println("Battery Data Log:");
    Serial.println("Timestamp, level");
    int i = 0;
    int upperLimit = 1000; // An example upper limit
    while (i < upperLimit){
      String key = "batteryLog" + String(i); // Create a unique key for each element
      String logged_data = preferences.getString(key.c_str(), "");
      if (logged_data == ""){
        break;
      }
      Serial.println(logged_data);
      wifiSerial(logged_data);
      i++;
    }
    preferences.end(); 
    Serial.println(" - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ");
}

void eraseBatteryLog(){
  Serial.println("Clearing battery log from memory");
  preferences.begin("bat_log");
  preferences.clear();
  preferences.end();
}

void toggleBatteryLogging(){
  _battery_logging = !_battery_logging;
    if (_battery_logging) {
    Serial.println("Battery Logging is now Enabled");
  } else{
    Serial.println("Battery Logging is now Disabled");
  }
}