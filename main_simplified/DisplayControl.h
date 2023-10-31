#ifndef DISPLAYCONTROL_H
#define DISPLAYCONTROL_H

#include "Arduino.h"
#include "functions.h"
#include "Types.h"
#include "TFT_eSPI.h" 
#include <CST816_TouchLib.h>
#include "bat_power.h"
#include "SensorControl.h"

// External vars
extern FaceType prevFace;
extern FaceType nextFace;

using namespace MDO;

extern TFT_eSPI tft;
extern TFT_eSprite spr;

extern CST816Touch oTouch;

// Function declarations:
void initializeTFT();
FaceTransition* getFaceTransition(FaceType type);
void resetDisplay();
touchEvent getTouch();
void handleGestures(FaceType *currentFace, touchEvent *currTouch);

// Functions for filling in the sprite
void printDigitalClock();
void printAnalogClock();
void printMenu();
void printRawData(TFT_eSprite* sprite);
void printSweatRate(TFT_eSprite* sprite);
void printRightHalfDate(TFT_eSprite* sprite);
void updateBatterySprite(); 

// 
void updateDisplay(FaceType* currentFace, TFT_eSprite* sprite);
void updateClockDisplay(TFT_eSprite* sprite);
void updateRawDataDisplay(TFT_eSprite* sprite);
void updateSweatRateDisplay(TFT_eSprite* sprite);
void updateMenuDisplay(TFT_eSprite* sprite);

//
void handleTouchForState(FaceType* currentFace, touchEvent* touch, TFT_eSprite* sprite);
void handleTouchForClockDisplay(touchEvent* touch, TFT_eSprite* sprite);
void handleTouchForRawDataDisplay(touchEvent* touch, TFT_eSprite* sprite);
void handleTouchForSweatRateDisplay(touchEvent* touch, TFT_eSprite* sprite);
void handleTouchForMenuDisplay(touchEvent* touch, TFT_eSprite* sprite);

//
void loadMenuSettings();
#endif  // DISPLAYCONTROL_H
