#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "Wire.h"
#include "Adafruit_HTU31D.h"
#include "TFT_eSPI.h" 
#include "TimeLib.h"
#include "pin_config.h"
#include "Types.h"
#include "bat_power.h"
#include <WiFi.h>
#include <SPIFFS.h>
#include <math.h>
#include <CST816_TouchLib.h>
#include "AD5933.h"
extern "C" {
  #include "lwip/apps/sntp.h"
}


using namespace MDO;

extern TFT_eSPI tft;
extern TFT_eSprite spr;
extern TFT_eSprite spr_base;

extern CST816Touch oTouch;

void debugPrint(const char* message);

float calcSkinRes();
float calcSweatRate();
const char* formatResistance(float resistance, char* buffer) ;
float change_to_f(float temp_c);

extern FaceState currentFaceState;

SensorReadings readExternalSensors();
SensorReadings readInternalSensors();

touchEvent getTouch();
void initMCU();
void initSensors();
void initializeTempSensors();
void resetDisplay();
void initializeNTP();
void initBaseSprite();
void updateBatterySprite();
void logSensorDataToNVM();

#endif // FUNCTIONS_H