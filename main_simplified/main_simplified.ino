#include "Arduino.h"
#include "OneButton.h"
#include "TFT_eSPI.h" 
#include "Wire.h"
#include "pin_config.h"
#include "functions.h"
#include "DisplayControl.h"
#include <CST816_TouchLib.h>
#include "esp_sleep.h"

#define DEBUG_ENABLED

/* Configure button IO*/
bool button_pressed = false;
bool sleep_wake_pressed = false;
bool screen_state = false;
bool previous_screen_state = false;
bool screen_enabled = true;
OneButton sleep_device(PIN_BUTTON_2, true, false);
OneButton sleep_screen(PIN_BUTTON_1, true, false);

touchEvent currTouch;
FaceType currentFace = FaceType::Clock;



void setup() {
  

  Serial.begin(115200);
  delay(1000); // Delay to let serial initialization complete
  Serial.println("Starting Up");  

  sleep_device.attachClick([]() { sleep_wake_pressed = true; });
  sleep_screen.attachClick([]() { screen_state = !screen_state; });


  initMCU();
  initializeTempSensors();
  initialize5933();

  printBatteryLog();
  printSensorLog();

  esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(PIN_BUTTON_2), 0); // 1 = High, 0 = Low
  
  delay(1);


  #ifdef DEBUG_ENABLED
      Serial.println("Setup Complete");
  #endif
}


void loop() {
  // Check button states
  sleep_device.tick();
  sleep_screen.tick();
  
  // Check wifi connections
  if(_wifi){
    // check if connected, reconnects if not
    initWiFi();
    // check clients
    checkWiFiClient();
  }

  // Disable touch interface while screen is disabled
  if (screen_enabled){
    oTouch.control();
    delay(2);
    if (oTouch.hadGesture() || oTouch.hadTouch()) {
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
  } else {
    // Call functions needed to process data for logging 
    // This is done in updateDisplay but if we want to continue
    // logging while the screen is off we have to call these manually
    
    // Read sensors 
    updateSensors(false);
    // Calling Read Battery Voltage will log battery level if logging is enabled
    readBatteryVoltage();
  }
     

  
  
  if (sleep_wake_pressed){
    Serial.println("Sleep_wake button pressed");
    // Prepare for sleep

    sleep_wake_pressed = false;

    // Enter sleep
    Serial.println("Going to sleep now");
    delay(100); // Wait for serial print to complete
    esp_deep_sleep_start();
  }

  //Go into screen state if only on state change
  if (screen_state != previous_screen_state){
    if (screen_state){
      Serial.println("Screen is disabled");      
      screen_enabled = false;
      tft.sleep(screen_state);
    } else {
      Serial.println("Screen is enabled");
      screen_enabled = true;
      tft.sleep(screen_state);
    }
    previous_screen_state = screen_state;
  }

  delay(10);
}

void initMCU(){
  initIO();
  loadMenuSettings();
  initializeTFT();
  tft.drawString("Initializing...",5,5);
  tft.drawString("Connecting to WiFi...",5,25);
  if(_wifi){
    initWiFi();

    // Dont use NTP if WiFi is off
    tft.drawString("Configuring RTC",5,65);
    initializeNTP();
  }
  


  // Create base sprite
  tft.drawString("Setup Complete",5,85);
  delay(500);
}