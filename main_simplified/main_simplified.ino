#include "Arduino.h"
#include "OneButton.h"
#include "TFT_eSPI.h" 
#include "Wire.h"
#include "pin_config.h"
#include "functions.h"
#include "DisplayControl.h"
#include <CST816_TouchLib.h>

#define DEBUG_ENABLED

/* Configure additional IO*/
bool button_pressed = false;
bool sleep_wake_pressed = false;
OneButton button1(PIN_BUTTON_1, true);
OneButton button2(PIN_BUTTON_2, true);



const unsigned long SENSOR_READ_INTERVAL = 30000;  // 30 seconds in milliseconds
unsigned long lastSensorReadTime = SENSOR_READ_INTERVAL + 1;

touchEvent currTouch;
FaceType currentFace = FaceType::Clock;


void setup() {

  Serial.begin(115200);
  delay(1000); // Delay to let serial initialization complete
  Serial.println("Starting Up");

  button2.attachClick([] { button_pressed = 1; });
  button1.attachClick([] { sleep_wake_pressed = 1; }); // Button 1 is the Boot button

  initMCU();
  initializeTempSensors();
  initialize5933();
  
  delay(1);

  #ifdef DEBUG_ENABLED
      Serial.println("Setup Complete");
  #endif
}

void loop() {
  button1.tick();
  button2.tick();
  oTouch.control();

  if (oTouch.hadGesture() || oTouch.hadTouch()  ) {
      currTouch = getTouch();
      delay(10);
      switch (currTouch.gesture) {
        case CST816Touch::GESTURE_LEFT:
        case CST816Touch::GESTURE_RIGHT:
        case CST816Touch::GESTURE_TOUCH_BUTTON:
          // Swap faces with specific gestures
          handleGestures(&currentFace, &currTouch);
          break;
        default:
          #ifdef DEBUG_ENABLED
            Serial.println("Handling touch coordinates");
          #endif
          // handle face specific touches if anything else
          handleTouchForState(&currentFace, &currTouch, &spr);
      }
     
  }

  updateDisplay(&currentFace, &spr);

  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
      #ifdef DEBUG_ENABLED
          Serial.println("Logging sensor data");
      #endif
      //logSensorDataToNVM();
      lastSensorReadTime = currentMillis;
  }

  if (button_pressed){
    resetDisplay();

  }

  if (sleep_wake_pressed){
    // put esp to sleep
  }

  delay(1000);
}

void initMCU(){

  initIO();
  loadMenuSettings();
  initializeTFT();
  tft.drawString("Initializing...",5,5);
  int retry = 0;
  const int retry_count = 10;
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED && ++retry < retry_count) {
    delay(1000);
    tft.drawString("Connecting to WiFi...",5,25);
    Serial.println("Connecting to WiFi...");
  }
  tft.drawString("Connected to WiFi",5,45);
  Serial.println("Connected to WiFi");

  // Initialize NTP
  tft.drawString("Configuring RTC",5,65);
  initializeNTP();


  // Create base sprite
  tft.drawString("Setup Complete",5,85);
  delay(500);
}