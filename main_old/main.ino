#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "OneButton.h"
#include "TFT_eSPI.h" 
#include "Wire.h"
#include "esp32-hal-i2c.h"
#include "pin_config.h"
#include "functions.h"
#include "Face.h"        
#include "ClockFace.h"   
#include "RawDataFace.h" 
#include "SweatRateFace.h" 
#include "MenuFace.h"
#include <CST816_TouchLib.h>

/* Configure additional IO*/
bool button_pressed = false;
bool soft_reset_pressed = false;
OneButton button1(PIN_BUTTON_1, true);
OneButton button2(PIN_BUTTON_2, true);

FaceState currentFaceState;;
Face* currentFace = nullptr;
FaceType prevFace;

touchEvent currTouch;

const unsigned long SENSOR_READ_INTERVAL = 30000;  // 30 seconds in milliseconds
unsigned long lastSensorReadTime = SENSOR_READ_INTERVAL + 1;

bool isMenu = false;

//Face* nextFace = new DebugFace();  // Starting state
Face* nextFace = new ClockFace();  // Starting state

FaceTransition faceTransitions[] = {
  // Current face, left swipte face, right swipe face
  { FaceType::Clock,     FaceType::RawData,     FaceType::SweatRate,},
  { FaceType::RawData,   FaceType::SweatRate,   FaceType::Clock,},
  { FaceType::SweatRate, FaceType::Clock,       FaceType::RawData, },
};

FaceTransition* getFaceTransition(FaceType type) {
  for (int i = 0; i < sizeof(faceTransitions) / sizeof(faceTransitions[0]); i++) {
    if (faceTransitions[i].current == type) {
      return &faceTransitions[i];
    }
  }
  Serial.println("!!!!  Transition Not found  !!!!");
  return nullptr;  // Not found
}

Face* createFaceInstance(FaceType type) {
    switch (type) {
        case FaceType::Clock:
            return new ClockFace();
        case FaceType::RawData:
            return new RawDataFace();
        case FaceType::SweatRate:
            return new SweatRateFace();
        // ... add more here
        default:
            return new ClockFace();  // Default case
    }
}

void handleGestures() {
  Serial.print("Get Heap: ");
  Serial.println(ESP.getFreeHeap());
    if (isMenu) {
        debugPrint("Menu is active.");
        switch (currTouch.gesture) {
            case CST816Touch::GESTURE_TOUCH_BUTTON:
                debugPrint("Gesture: TOUCH_BUTTON in menu context.");
                debugPrint("closing menu");
                nextFace = createFaceInstance(prevFace);
                isMenu = false;
                currTouch.gesture = CST816Touch::GESTURE_NONE;  
                break;
        }
    } else {
        debugPrint("Menu is not active.");
        FaceTransition* transition = getFaceTransition(currentFace->getType());
        if (!transition) {
            debugPrint("No valid transition found.");
            nextFace = new ClockFace();
            return;
        }

        switch (currTouch.gesture) {
            case CST816Touch::GESTURE_LEFT:
                debugPrint("Gesture: LEFT.");
                nextFace = createFaceInstance(transition->left);
                currTouch.gesture = CST816Touch::GESTURE_NONE;
                break;
            case CST816Touch::GESTURE_RIGHT:
                debugPrint("Gesture: RIGHT.");
                nextFace = createFaceInstance(transition->right);
                currTouch.gesture = CST816Touch::GESTURE_NONE;
                break;
            case CST816Touch::GESTURE_TOUCH_BUTTON:
                debugPrint("Gesture: TOUCH_BUTTON outside menu context.");
                prevFace = currentFace->getType();
                nextFace = new MenuFace();
                isMenu = true;
                currTouch.gesture = CST816Touch::GESTURE_NONE;
                break;
        }
        
    }
}



void setup() {

  Serial.begin(115200);
  delay(1000); // Delay to let serial initialization complete
  Serial.println("Starting Up");

  button2.attachClick([] { button_pressed = 1; });
  button1.attachClick([] { soft_reset_pressed = 1; }); // Button 1 is the Boot button

  initMCU();
  initializeTempSensors();

  // Load Settings from Flash into memory. 
  Face::loadGlobalSettings();
  
  delay(1);

  Serial.println("Setup Complete");
}

void loop() {
    button1.tick();
    button2.tick();
    oTouch.control();

    if (currentFace != nextFace) {
        if (currentFace) {
            currentFace->exit();
            delete currentFace;
            currentFace = nullptr;
        }
        currentFace = nextFace;
        currentFace->enter();
    }
    currentFace->update();

    if (oTouch.hadTouch() || oTouch.hadGesture()) {
        currTouch = getTouch();
        delay(10);
        if (currTouch.gestureEvent){
          // Handle gestures here
          handleGestures();
        } else {
          // pass touch data to Class Face
          currentFace->handleTouch(currTouch);
        }        
    }

    unsigned long currentMillis = millis();
    if (currentMillis - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
        logSensorDataToNVM();
        lastSensorReadTime = currentMillis;
    }
}
