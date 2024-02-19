#ifndef SENSORCONTROL_H
#define SENSORCONTROL_H

#include "Arduino.h"
#include "functions.h"
#include "Types.h"
#include "AD5933.h"
#include "HTU31D.h"
#include "Preferences.h"

extern FaceState currentFaceState;
extern HTU31D* htu31d_body;
extern HTU31D* htu31d_ambi;

extern HTU31D::THData sensorData_body;
extern HTU31D::THData sensorData_ambi;

extern float skinRes;
extern float sweatRate;
extern Preferences preferences;

void initSensors();
void initializeTempSensors();
void initialize5933();
void printDiagnosticInfo(HTU31D* sensorInstance); 

void logSensorDataToNVM();
HTU31D::THData readSensors(HTU31D* sensorInstance);

void calcSweatRate(float* sweatRate, float height, float weight, float metabolic_rate);
void updateSensors(bool forceUpdate = false);
void frequencySweepRaw(float* res);
void calibrateAD5933();
void readCalConstantsFromMemory();
void writeCalConstantsToMemory();

#endif  // SENSORCONTROL_H
