#ifndef SENSORCONTROL_H
#define SENSORCONTROL_H

#include "Arduino.h"
#include "functions.h"
#include "Types.h"
#include "AD5933.h"
#include "HTU31D.h"

extern FaceState currentFaceState;
extern HTU31D* htu31d_body;
extern HTU31D* htu31d_ambi;

extern HTU31D::THData sensorData_body;
extern HTU31D::THData sensorData_ambi;

extern float skinRes;
extern float sweatRate;

void initSensors();
void initializeTempSensors();
void initialize5933();
void printDiagnosticInfo(HTU31D* sensorInstance); 

void logSensorDataToNVM();
HTU31D::THData readSensors(HTU31D* sensorInstance);

void readSkinRes(float* skinRes);
void calcSweatRate(float* sweatRate);
void updateSensors();
void frequencySweepRaw(float* res);

#endif  // SENSORCONTROL_H
