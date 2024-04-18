#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "Wire.h"
#include "TimeLib.h"
#include "pin_config.h"
#include "Types.h"
#include <WiFi.h>
#include <SPIFFS.h>
#include <math.h>
#include <esp_adc_cal.h>
#include "Preferences.h"
#include <WiFi.h>

extern Preferences preferences;

extern WiFiServer server;
extern WiFiClient client;
extern String ipAddress;

extern long _UTC_OFF;
extern bool _battery_logging;

extern "C" {
  #include "lwip/apps/sntp.h"
}

extern TwoWire SensorsI2C;

void debugPrint(const char* message);


const char* formatResistance(float resistance, char* buffer) ;
float change_to_f(float temp_c);
float change_to_C(float temp_f);
void initIO();
void initWiFi();
void checkWiFiClient();
void initializeNTP();
void set_time_from_wifi();
void configureWiFiHost();
double calculateHeatIndex(double temp, double humidity);
float readBatteryVoltage();
void logBatteryToNVM(uint32_t bat_volt);
void printBatteryLog();
void eraseBatteryLog();
void wifiSerial(const String& str);
void toggleBatteryLogging();


#endif // FUNCTIONS_H