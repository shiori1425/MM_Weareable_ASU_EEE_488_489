#define DEBUG_ENABLED

#include "functions.h"

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

  // Start I2C
  SensorsI2C.begin(PIN_THIC_SDA, PIN_THIC_SCL); // Initialize Sensor I2C BUS
  SensorsI2C.setClock(400000);  // max clock speed for HTU31D is 1M but for AD5933 is 400k
  delay(1);
  Wire.begin(PIN_IIC_SDA, PIN_IIC_SCL);
  Wire.setClock(400000);   // Max clock speed for Touch Controller
  delay(1);

  #ifdef DEBUG_ENABLED
      Serial.println("I2C initialized.");
  #endif
}

void initializeNTP() {
  #ifdef DEBUG_ENABLED
      Serial.println("Initializing NTP...");
  #endif

  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_init();

  // Wait till NTP sets the time
  time_t now = 0;
  struct tm timeinfo = { 0 };
  int retry = 0;
  const int retry_count = 10;
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
  const long EST = -4 * 3600;  // 4 hours * 3600 seconds/hour
  adjustTime(EST);

  #ifdef DEBUG_ENABLED
      Serial.println("NTP initialization completed.");
  #endif
}
