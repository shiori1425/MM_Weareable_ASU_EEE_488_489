#include "TFT_eSPI.h"
#include "DummyTouchSubscriber.h"
#include "DisplayControl.h"

//#define DEBUG_ENABLED

#define PIXEL_HEIGHT 170
#define PIXEL_WIDTH 320

uint16_t _textColor;
uint16_t _bgColor;
uint16_t _fgColor;
bool _printDigital;
bool _temp_c;
long _UTC_OFF;


int32_t width, height, left_center, right_center;

TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);

CST816Touch oTouch;

FaceType prevFace;
FaceType nextFace;
bool isMenu = false;

static FaceTransition faceTransitions[] = {
    { FaceType::Clock,     FaceType::RawData,     FaceType::SweatRate },
    { FaceType::RawData,   FaceType::SweatRate,   FaceType::Clock },
    { FaceType::SweatRate, FaceType::Clock,       FaceType::RawData },
};

touchEvent getTouch() {

    touchEvent result;
    result.x = -1;
    result.y = -1;
    result.gesture = CST816Touch::GESTURE_NONE; 
    result.gestureEvent = oTouch.hadGesture();

    bool printTouchDebug = true;

    if (oTouch.hadTouch()) {
      oTouch.getLastTouchPosition(result.x,result.y);
      result.y = tft.height() - result.y; // Flip y axis on touch to align with screen rotation
    } else if (result.gestureEvent) {
        oTouch.getLastGesture(result.gesture, result.x, result.y);
        result.y = tft.height() - result.y; // Flip y axis on touch to align with screen rotation
        if(printTouchDebug){
          Serial.println("- - - - - - - - - - - - - - - - ");
          Serial.println("Gesture Event:");
          Serial.print("Motion: ");
          Serial.println(oTouch.gestureIdToString(result.gesture));
          Serial.print("At location: ");
          Serial.printf("x: %d, y: %d \n", result.x, result.y);
        }
    }
    return result;
}

void initializeTFT() {
  /*setup screen params*/
  width = PIXEL_WIDTH;
  height = PIXEL_HEIGHT;
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(5,5);
  tft.drawString("Initializing.",5,5);

  left_center = width*0.25;
  right_center = width*0.75;

  ledcSetup(0, 2000, 8);
  ledcAttachPin(PIN_LCD_BL, 0);
  ledcWrite(0, 255);

  spr.createSprite(PIXEL_WIDTH, PIXEL_HEIGHT);

  oTouch.begin(Wire, nullptr, PIN_TOUCH_INT);
  oTouch.resetChip(true);
  tft.drawString("Initializing..",5,5);
  oTouch.setAutoSleep(false);
}

void resetDisplay(){
    tft.fillScreen(TFT_WHITE);
    delay(1);
    tft.fillScreen(TFT_BLACK);
    delay(1);
    spr.deleteSprite();
    delay(1);
    spr.createSprite(PIXEL_WIDTH, PIXEL_HEIGHT);
    delay(1);
}

FaceTransition* getFaceTransition(FaceType type) {
    for (int i = 0; i < sizeof(faceTransitions) / sizeof(faceTransitions[0]); i++) {
        if (faceTransitions[i].current == type) {
            return &faceTransitions[i];
        }
    }
    Serial.println("!!!!  Transition Not found  !!!!");
    return nullptr;  // Not found
}

void handleGestures(FaceType *currentFace, touchEvent *currTouch) {
    if (isMenu) {
        switch (currTouch->gesture) {
            case CST816Touch::GESTURE_TOUCH_BUTTON:
                *currentFace = prevFace;
                isMenu = false;
                break;
        }
    } else {
        FaceTransition* transition = getFaceTransition(*currentFace);
        if (!transition) return;

        switch (currTouch->gesture) {
            case CST816Touch::GESTURE_LEFT:
                *currentFace = transition->left;
                break;
            case CST816Touch::GESTURE_RIGHT:
                *currentFace = transition->right;
                break;
            case CST816Touch::GESTURE_TOUCH_BUTTON:
                prevFace = *currentFace;
                *currentFace = FaceType::Menu;
                isMenu = true;
                break;
        }
    }
    currTouch->gesture = CST816Touch::GESTURE_NONE;
}

/**********************************************************************************************
*                        Screen Printing
*
*
**********************************************************************************************/
void printDigitalClock(TFT_eSprite* sprite){
  // Set sprite to display digital clock
  sprite->setTextColor(_textColor);
 
  // Print time
    //Serial.println("Update Clock Face");
    const int TIME_SIZE = 6; // Adjust this as per your desired size
    const int TIME_X_POS = 15; // Starting X position. Adjust to move horizontally.
    const int TIME_Y_POS = 55; // Starting Y position. Adjust to move vertically.
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d:%02d", hourFormat12(),minute(), second());
    // Set the text properties
    sprite->setTextSize(TIME_SIZE);
    sprite->setCursor(TIME_X_POS, TIME_Y_POS);
    // Print the time to the sprite
    sprite->print(timeStr);

  // Print Date
    // Format the time to hh:mm:ss
    const int DATE_SIZE = 2; // Adjust this as per your desired size
    const int DATE_X_POS = 25; // Starting X position. Adjust to move horizontally.
    const int DATE_Y_POS = 115; // Starting Y position. Adjust to move vertically.

    char dateStr[24];

    // Weekdays array (Sunday = 1)
    const char* daysOfWeek[] = {"", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    // Months array (January = 1)
    const char* months[] = {"", "January", "February", "March", "April", "May", "June",
                            "July", "August", "September", "October", "November", "December"};
    // Use the functions from TimeLib.h to get the current date components
    int currentYear = year();
    int currentMonth = month();
    int currentDay = day();
    int currentWeekday = weekday();  

    // Format the date string using snprintf
    snprintf(dateStr, sizeof(dateStr), "%s %s %d, %d",
            daysOfWeek[currentWeekday], months[currentMonth], currentDay, currentYear);

    sprite->setTextSize(DATE_SIZE);
    sprite->setCursor(DATE_X_POS, DATE_Y_POS);
    // Print the time to the sprite
    sprite->print(dateStr);
    
}

void printAnalogClock(TFT_eSprite* sprite){

    const int HOUR_HAND_LENGTH = 35;
    const int MINUTE_HAND_LENGTH = 63;
    const int SECOND_HAND_LENGTH = 70;
    const int CLOCK_CENTER_X = 85;
    const int CLOCK_CENTER_Y = 85;
    const int CLOCK_RADIUS = 70;
    const int ROTATION = 2;

    // Clear the display
    sprite->fillScreen(TFT_BLACK);
    sprite->setTextColor(_textColor);

    // Draw the clock face
    sprite->drawCircle(CLOCK_CENTER_X, CLOCK_CENTER_Y, CLOCK_RADIUS, TFT_WHITE);

    // Calculate angles for each hand
    float hourAngle = (hourFormat12() + minute() / 60.0) * 30;    // Each hour is 30 degrees
    float minuteAngle = minute() * 6;   // Each minute is 6 degrees
    float secondAngle = second() * 6;   // Each second is 6 degrees

    // Calculate hand endpoints
    int16_t hourEndX = CLOCK_CENTER_X + HOUR_HAND_LENGTH * sin(-ROTATION*PI/2-radians(hourAngle));
    int16_t hourEndY = CLOCK_CENTER_Y + HOUR_HAND_LENGTH * cos(-ROTATION*PI/2-radians(hourAngle));
    
    int16_t minEndX = CLOCK_CENTER_X + MINUTE_HAND_LENGTH * sin(-ROTATION*PI/2-radians(minuteAngle));
    int16_t minEndY = CLOCK_CENTER_Y + MINUTE_HAND_LENGTH * cos(-ROTATION*PI/2-radians(minuteAngle));

    int16_t secEndX = CLOCK_CENTER_X + SECOND_HAND_LENGTH * sin(-ROTATION*PI/2-radians(secondAngle));
    int16_t secEndY = CLOCK_CENTER_Y + SECOND_HAND_LENGTH * cos(-ROTATION*PI/2-radians(secondAngle));

    // Draw the clock hands
    //sprite->drawWedgeLine(float ax, float ay, float bx, float by, float aw, float bw, uint32_t fg_color)
    sprite->drawLine(CLOCK_CENTER_X, CLOCK_CENTER_Y, hourEndX, hourEndY, TFT_RED);
    sprite->drawLine(CLOCK_CENTER_X, CLOCK_CENTER_Y, minEndX, minEndY, TFT_GREEN);
    sprite->drawLine(CLOCK_CENTER_X, CLOCK_CENTER_Y, secEndX, secEndY, TFT_BLUE);
    
}

void printRightHalfDate(TFT_eSprite* sprite){
    // Print Date
    // Format the time to hh:mm:ss
    const int DATE_SIZE = 3; // Adjust this as per your desired size
    const int DATE_X_POS = 185; // Starting X position. Adjust to move horizontally.
    const int DATE_Y_POS = 45; // Starting Y position. Adjust to move vertically.

    char monthStr[10];
    char weekdayStr[9];
    char dateStr[9];

    // Weekdays array (Sunday = 1)
    const char* daysOfWeek[] = {"", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Satday"};
    // Months array (January = 1)
    const char* months[] = {"", "January", "February", "March", "April", "May", "June",
                            "July", "August", "September", "October", "November", "December"};
    // Use the functions from TimeLib.h to get the current date components
    int currentYear = year();
    int currentMonth = month();
    int currentDay = day();
    int currentWeekday = weekday();  

    // Format date strings
    snprintf(monthStr,sizeof(monthStr),"%s",months[currentMonth]);
    snprintf(weekdayStr,sizeof(weekdayStr),"%s",daysOfWeek[currentWeekday]);
    snprintf(dateStr, sizeof(dateStr), "%d, %d", currentDay, currentYear);

    sprite->setTextSize(DATE_SIZE);
    sprite->setCursor(DATE_X_POS, DATE_Y_POS);
    // Print the time to the sprite
    sprite->println(weekdayStr);
    sprite->setCursor(DATE_X_POS, DATE_Y_POS + sprite->fontHeight());
    sprite->println(monthStr);
    sprite->setCursor(DATE_X_POS, DATE_Y_POS + sprite->fontHeight() * 2);
    sprite->println(dateStr);
}

void printMenuLayout(TFT_eSprite* sprite){

  // Due to sprites not respecting TFT rotation, we swap X and Y coordinates 
        // to manually rotate the design 90 degrees on the sprite
  int MENU_X = 5;
  int MENU_Y = 5;
  int MENU_HEIGHT = 300;
  int MENU_WIDTH = 150;
  int MENU_TXT_X = MENU_X + 5;
  int MENU_TXT_Y = MENU_X + 5;
  int MENU_NL_SPACE = sprite->fontHeight() + 10;
  int LINE_NUM = 0;
  int LINE_WEIGHT = 3;
  sprite->fillSprite(_fgColor);

  // Set Menu Text Settings
  sprite->setTextSize(2);
  sprite->setTextColor(_bgColor);

  // New Menu Line Item
  const char* menu_item_1 = "Text Color";
  sprite->drawString(menu_item_1,MENU_TXT_Y, MENU_TXT_X); 
  
  // New Menu Line Item
  LINE_NUM++;
  sprite->drawString("BG Color",MENU_TXT_Y, MENU_TXT_X + MENU_NL_SPACE * LINE_NUM + 5 ); 

  // New Menu Line Item
  LINE_NUM++;
  sprite->drawString("FG Color",MENU_TXT_Y, MENU_TXT_X + MENU_NL_SPACE * LINE_NUM + 5);

  // New Menu Line Item
  LINE_NUM++;
  sprite->drawString("UTC Offset",MENU_TXT_Y, MENU_TXT_X + MENU_NL_SPACE * LINE_NUM + 5);

  // New Menu Line Item
  LINE_NUM++;
  sprite->drawString("Temp Unit",MENU_TXT_Y, MENU_TXT_X + MENU_NL_SPACE * LINE_NUM + 5);

  // New Menu Line Item
  LINE_NUM++;
  sprite->drawString("Digital",MENU_TXT_Y, MENU_TXT_X + MENU_NL_SPACE * LINE_NUM + 5);
}

void printRawData(TFT_eSprite* sprite){
  
  int x = 15;

  //Get updated raw data values
  updateSensors();
  
  sprite->fillSprite(_bgColor);

  // Displaying data points on sprite
  sprite->setTextSize(3);
  sprite->setTextColor(_textColor);

  sprite->setCursor(x, 20);
  sprite->print("Bod Tmp: ");

  if (_temp_c){
  sprite->print(sensorData_body.temperature);
  sprite->println(" C");
  } else {
  sprite->print(change_to_f(sensorData_body.temperature));
  sprite->println(" F");
  }

  sprite->setCursor(x, 80);
  sprite->print("Amb Tmp: ");
  if (_temp_c){
  sprite->print(sensorData_ambi.temperature);
  sprite->println(" C");
  } else {
  sprite->print(change_to_f(sensorData_ambi.temperature));
  sprite->println(" F");
  }

  sprite->setCursor(x, 50);
  sprite->print("Bod Hum: ");
  sprite->print(sensorData_body.humidity);
  sprite->println(" %");

  sprite->setCursor(x, 110);
  sprite->print("Amb Hum: ");
  sprite->print(sensorData_ambi.humidity);
  sprite->println(" %");

  char str_buf[20] = {0};
  sprite->setCursor(x, 140);
  sprite->print("Skin Res: ");
  sprite->print(formatResistance(skinRes, str_buf));
  //sprite->drawGlyph(0x03A9);
}

void printSweatRate(TFT_eSprite* sprite) {

    // call Update Sweat Rate before printing it
    calcSweatRate(&sweatRate);

    //sprite->pushToSprite(&spr_base, 0, 0);
    sprite->setTextSize(4);
    sprite->setTextColor(_textColor);
    sprite->setCursor(20, 30); // Centering might need adjusting
    sprite->println("Sweat Rate:");
    sprite->setCursor(20, 80); // Centering might need adjusting
    sprite->printf("%d mL/min", sweatRate); // Placeholder value
}

/**********************************************************************************************
*
*                   Update Calls
*
**********************************************************************************************/
void updateBatterySprite(TFT_eSprite* sprite){
  float bat_volt = (analogRead(PIN_BAT_VOLT) / 1023.0) * 3.6;
  //bat_volt = 3.2;
  uint16_t* iconData;
  //sprite->fillRect(295,0,25,10,TFT_WHITE);

  if (bat_volt >= 3.6) {
    iconData = (uint16_t*)gImage_Full_Battery; 
    //sprite->fillRect(295,0,25,10,TFT_GREEN);
  } else if (bat_volt >= 3.4) {  // (3.6 + 3.2) / 2 = 3.4
    iconData = (uint16_t*)gImage_Battery_2;
    //sprite->fillRect(295,0,25,10,TFT_GREENYELLOW);  
  } else if (bat_volt >= 3.0) {  // (3.2 + 2.8) / 2 = 3.0
    iconData = (uint16_t*)gImage_Battery_1;
    //sprite->fillRect(295,0,25,10,TFT_ORANGE);  
  } else {
    iconData = (uint16_t*)gImage_Battery_0;
    //sprite->fillRect(295,0,25,10,0xFE19); 
  }  
  sprite->pushImage(295,0,25,10,iconData);
}

void updateDisplay(FaceType* currentFace, TFT_eSprite* sprite){
  #ifdef DEBUG_ENABLED
    Serial.print("Update: Calling update display for state ");
    Serial.println(static_cast<int>(*currentFace));
  #endif
  sprite->fillSprite(_bgColor);  
    switch (*currentFace) {
        case FaceType::Clock:
            updateClockDisplay(sprite);
            break;
        case FaceType::RawData:
            updateRawDataDisplay(sprite);
            break;
        case FaceType::SweatRate:
            updateSweatRateDisplay(sprite);
            break;
        case FaceType::Menu:
            updateMenuDisplay(sprite);
            break;
        default:
            Serial.println("Update display currentface mismatch");
    }
    updateBatterySprite(sprite);
    sprite->pushSprite(0,0);
}

// Function to handle touch for the current state:
/**********************************************************************************************
***                        Functions for updating the sprite to specific displays
***
***
**********************************************************************************************/

void updateClockDisplay(TFT_eSprite* sprite) {
  #ifdef DEBUG_ENABLED
    Serial.println("Update: Printing Clock Display");
  #endif
    // Update the sprite to reflect the Clock face
  if (_printDigital){
    printDigitalClock(sprite);
  } else {
    printAnalogClock(sprite);
    printRightHalfDate(sprite);
  }
}

void updateRawDataDisplay(TFT_eSprite* sprite) {
    // Update the sprite to reflect the Raw Data face
  #ifdef DEBUG_ENABLED
    Serial.println("Update: Printing Raw Data Display");
  #endif
  printRawData(sprite);
}

void updateSweatRateDisplay(TFT_eSprite* sprite) {
    // Update the sprite to reflect the Sweat Rate face  
  #ifdef DEBUG_ENABLED
    Serial.println("Update: Printing Sweat Rate Display");
  #endif
  printSweatRate(sprite);
}

void updateMenuDisplay(TFT_eSprite* sprite) {
  #ifdef DEBUG_ENABLED
    Serial.println("Update: Printing Menu Display");
  #endif
  printMenuLayout(sprite);
  
}

// Function to handle touch for the current state:
/**********************************************************************************************
***
***
***
**********************************************************************************************/
void handleTouchForState(FaceType* currentFace, touchEvent* touch, TFT_eSprite* sprite) {
    switch (*currentFace) {
        case FaceType::Clock:
            handleTouchForClockDisplay(touch, sprite);
            break;
        case FaceType::RawData:
            handleTouchForRawDataDisplay(touch, sprite);
            break;
        case FaceType::SweatRate:
            handleTouchForSweatRateDisplay(touch, sprite);
            break;
        case FaceType::Menu:
            handleTouchForMenuDisplay(touch, sprite);
            break;
    }
}

void handleTouchForClockDisplay(touchEvent* touch, TFT_eSprite* sprite) {
    // Handle touch events specific to the Clock Display

}

/**********************************************************************************************
*
*
*
**********************************************************************************************/
void handleTouchForRawDataDisplay(touchEvent* touch, TFT_eSprite* sprite) {
    // Handle touch events specific to the Raw Data Display

}

void handleTouchForSweatRateDisplay(touchEvent* touch, TFT_eSprite* sprite) {
    // Handle touch events specific to the Sweat Rate Display

}

void handleTouchForMenuDisplay(touchEvent* touch, TFT_eSprite* sprite) {
    // Handle touch events specific to the Menu display

}

void loadMenuSettings(){
    // Placeholder code: replace with logic to load from flash
    _textColor = TFT_SKYBLUE;
    _bgColor = TFT_BLACK;
    _fgColor = TFT_DARKGREY;
    _printDigital = false;
    _temp_c = false;
    _UTC_OFF = -4;  
}