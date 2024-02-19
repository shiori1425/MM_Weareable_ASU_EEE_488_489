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


extern "C" {
  #include "lwip/apps/sntp.h"
}

extern TwoWire SensorsI2C;

void debugPrint(const char* message);


const char* formatResistance(float resistance, char* buffer) ;
float change_to_f(float temp_c);
float change_to_C(float temp_f);
void initIO();
void initializeNTP();
double calculateHeatIndex(double temp, double humidity);
float readBatteryVoltage();



#endif // FUNCTIONS_H