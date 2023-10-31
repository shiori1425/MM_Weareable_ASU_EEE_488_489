#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "Wire.h"
#include "TimeLib.h"
#include "pin_config.h"
#include "Types.h"
#include <WiFi.h>
#include <SPIFFS.h>
#include <math.h>

extern "C" {
  #include "lwip/apps/sntp.h"
}

extern TwoWire SensorsI2C;

void debugPrint(const char* message);


const char* formatResistance(float resistance, char* buffer) ;
float change_to_f(float temp_c);
void initIO();
void initializeNTP();




#endif // FUNCTIONS_H