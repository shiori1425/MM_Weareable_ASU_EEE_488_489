#include "TFT_eSPI.h"
#include "DummyTouchSubscriber.h"
#include "DisplayControl.h"


#define DEBUG_ENABLED
//#define DEBUG_ENABLED_FACE
#define DEBUG_ENABLED_TOUCH

#define PIXEL_HEIGHT 170
#define PIXEL_WIDTH 320

// Menu definitions
const int MENU_ITEM_WIDTH = PIXEL_WIDTH - 30;  // Some margin on the sides
const int ARROW_SIZE = 22;  // Size of the up and down arrows
const int MENU_ITEM_HEIGHT = (PIXEL_HEIGHT - 2*ARROW_SIZE) / 4;  // 4 items, minus space for arrows
const int SCROLLBAR_WIDTH = 10;  // Width of the scrollbar
int menuOffset = 0;  // Which item is at the top
int UTC_MAX_OFFSET = 13;  // This will give UTC values from -6 to +6

//Preferences preferences;

int _textColorIndex = 0;
uint16_t _textColor;
int _bgColorIndex = 0;
uint16_t _bgColor;
int _fgColorIndex = 0;
uint16_t _fgColor;
bool _printDigital;
bool _wifi;
bool _temp_c;
long _UTC_OFF;
uint16_t _heatIndexColor;
const char* _calExec = "Execute";
const char* _emptyChar = nullptr;

struct MenuItem {
    const char* name;
    enum { COLOR, BOOLEAN, INTEGER, FLOAT, STRING} type;
    void (*callback)();
    void* value;
};

// Declare Menu functions to load menu item arrays
void adjustTextColor();
void adjustBgColor();
void adjustFgColor();
void adjustUTCOffset();
void toggleTempUnit();
void toggleDigitalDisplay();
void adjustHeight();
void adjustWeight();
void toggleWifi();
void refreshMenuItem(int index, uint16_t value, TFT_eSprite* sprite);
void refreshMenuItem(int index, int value, TFT_eSprite* sprite);
void refreshMenuItem(int index, float value, TFT_eSprite* sprite);
void refreshMenuItem(int index, bool value, TFT_eSprite* sprite);
void refreshMenuItem(int index, const char* value, TFT_eSprite* sprite);
void setTopLevelMenu();
void setDisplayMenu();
void setSystemMenu();
void setBodyParamsMenu();
void setResetMenu();
void noop();
int getMenuSize(MenuItem* menu);

MenuItem allMenuItems[] = {
    //{"Label",         MenuItem::xxxx,    callback,       value}
    {"Text Color",      MenuItem::COLOR,    adjustTextColor,          &_textColor},
    {"BG Color",        MenuItem::COLOR,    adjustBgColor,            &_bgColor},
    {"FG Color",        MenuItem::COLOR,    adjustFgColor,            &_fgColor},
    {"Print Digital",   MenuItem::BOOLEAN,  toggleDigitalDisplay,     &_printDigital},
    {"Temp in C",       MenuItem::BOOLEAN,  toggleTempUnit,           &_temp_c},
    {"UTC Offset",      MenuItem::INTEGER,  adjustUTCOffset,          &_UTC_OFF},
    {"Height (in)",     MenuItem::FLOAT,    adjustHeight,             &_height},
    {"Weight (lbs)",    MenuItem::FLOAT,    adjustWeight,             &_weight},
    {"Cal AD5933",      MenuItem::STRING,   calibrateAD5933,          &_calExec},
    {"Print Log Data",  MenuItem::STRING,   printSensorLog,           &_emptyChar},
    {"",                MenuItem::STRING,   printSensorLog,           &_emptyChar},
    {"Reset Log Data",  MenuItem::STRING,   eraseLoggedSensorData,    &_emptyChar},
    {"Reset Settings",  MenuItem::STRING,   eraseLoggedMenuSettings,  &_emptyChar},
    {"Reset Bat Data",  MenuItem::STRING,   eraseBatteryLog,          &_emptyChar},
};

MenuItem bodyParamsMenuItems[] = {
    //{"Label",         MenuItem::xxxx,     callback,                 value}
    {"Height (in)",     MenuItem::INTEGER,    adjustHeight,             &_height},
    {"Weight (lbs)",    MenuItem::INTEGER,    adjustWeight,             &_weight},
    {"Back",            MenuItem::STRING,   setTopLevelMenu,          &_emptyChar}
};

MenuItem resetMenuItems[] = {
    //{"Label",         MenuItem::xxxx,    callback,       value}
    {"Reset Log Data",  MenuItem::STRING,   eraseLoggedSensorData,    &_emptyChar},
    {"Reset Bat Data",  MenuItem::STRING,   eraseBatteryLog,          &_emptyChar},
    {"Reset Settings",  MenuItem::STRING,   eraseLoggedMenuSettings,  &_emptyChar},
    {"Back",            MenuItem::STRING,   setTopLevelMenu,          &_emptyChar}
};

MenuItem colorMenuItems[] = {
    //{"Label",         MenuItem::xxxx,     callback,                 value}
    {"Text Color",      MenuItem::COLOR,    adjustTextColor,          &_textColor},
    {"BG Color",        MenuItem::COLOR,    adjustBgColor,            &_bgColor},
    {"FG Color",        MenuItem::COLOR,    adjustFgColor,            &_fgColor},
    {"Back",            MenuItem::STRING,   setTopLevelMenu,          &_emptyChar}
};

MenuItem displayMenuItems[] = {
    //{"Label",         MenuItem::xxxx,     callback,                 value}
    {"Text Color",      MenuItem::COLOR,    adjustTextColor,          &_textColor},
    {"BG Color",        MenuItem::COLOR,    adjustBgColor,            &_bgColor},
    {"FG Color",        MenuItem::COLOR,    adjustFgColor,            &_fgColor},
    {"Clock",           MenuItem::BOOLEAN,  toggleDigitalDisplay,     &_printDigital},
    {"Temp unit",       MenuItem::BOOLEAN,  toggleTempUnit,           &_temp_c},
    {"UTC Offset",      MenuItem::INTEGER,  adjustUTCOffset,          &_UTC_OFF},
    {"Back",            MenuItem::STRING,   setTopLevelMenu,          &_emptyChar}
};

MenuItem systemMenuItems[] = {
    //{"Label",         MenuItem::xxxx,     callback,                 value}
    {"Cal AD5933",      MenuItem::STRING,   calibrateAD5933,          &_emptyChar},
    {"Print Log Data",  MenuItem::STRING,   printSensorLog,           &_emptyChar},
    {"Print Bat Data",  MenuItem::STRING,   printBatteryLog,          &_emptyChar},
    {"WiFi",            MenuItem::BOOLEAN,  toggleWifi,               &_wifi},
    {"IP:",             MenuItem::STRING,   noop,                     &ipAddress},
    {"Back",            MenuItem::STRING,   setTopLevelMenu,          &_emptyChar}
};

MenuItem topMenuItems[] = {
    //{"Label",       MenuItem::xxxx,       callback,                 Value}
    {"Display",       MenuItem::STRING,     setDisplayMenu,           &_emptyChar},
    {"Body Params",   MenuItem::STRING,     setBodyParamsMenu,        &_emptyChar},
    {"System",        MenuItem::STRING,     setSystemMenu,            &_emptyChar},
    {"Reset",         MenuItem::STRING,     setResetMenu,             &_emptyChar},

};

MenuItem* activeMenu = topMenuItems;
MenuItem* requestedMenuChange = nullptr;

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
    result.gestureEvent = oTouch.hadGesture();
    result.gesture = CST816Touch::GESTURE_NONE; 

    bool printTouchDebug = true;
    
    if (oTouch.hadGesture()){
      #ifdef DEBUG_ENABLED_TOUCH
          Serial.println("getTouch: Had Gesture");
      #endif
      oTouch.getLastGesture(result.gesture, result.x, result.y);
    } else{
      #ifdef DEBUG_ENABLED_TOUCH
          Serial.println("getTouch: Had Touch");
      #endif
      oTouch.getLastTouchPosition(result.x, result.y);
      result.gesture = CST816Touch::GESTURE_NONE; 
    }

    result.y = tft.height() - result.y; // Flip y axis on touch to align with screen rotation
        
    #ifdef DEBUG_ENABLED_TOUCH
      Serial.println("- - - - - - - - - - - - - - - - ");
      Serial.println("Gesture Event:");
      Serial.print("Motion: ");
      Serial.println(oTouch.gestureIdToString(result.gesture));
      Serial.print("At location: ");
      Serial.printf("x: %d, y: %d \n", result.x, result.y);
    #endif
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
  oTouch.setOperatingModeHardwareBased();
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
                writeMenuSettings();
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
                setTopLevelMenu();
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
    const int DATE_X_POS = 170; // Starting X position. Adjust to move horizontally.
    const int DATE_Y_POS = 45; // Starting Y position. Adjust to move vertically.

    char monthStr[10];
    char weekdayStr[10];
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
    sprite->fillSprite(_fgColor);
    
    // Draw the up arrow (this can be replaced with a graphic later if needed)
    sprite->fillTriangle(
        PIXEL_WIDTH/2, 0,
        PIXEL_WIDTH/2 - ARROW_SIZE/2, ARROW_SIZE,
        PIXEL_WIDTH/2 + ARROW_SIZE/2, ARROW_SIZE,
        TFT_BLACK);
        
    // Draw the down arrow
    sprite->fillTriangle(
        PIXEL_WIDTH/2, PIXEL_HEIGHT,
        PIXEL_WIDTH/2 - ARROW_SIZE/2, PIXEL_HEIGHT - ARROW_SIZE,
        PIXEL_WIDTH/2 + ARROW_SIZE/2, PIXEL_HEIGHT - ARROW_SIZE,
        TFT_BLACK);
        
    // Draw the scrollbar (for now, static on the side; we'll make it dynamic later)
    sprite->fillRect(PIXEL_WIDTH - SCROLLBAR_WIDTH, ARROW_SIZE, SCROLLBAR_WIDTH, PIXEL_HEIGHT - 2*ARROW_SIZE, TFT_DARKGREY);
    
    // Draw visible menu items
    for(int i = 0; i < 4; i++) {
        int y = ARROW_SIZE + i * MENU_ITEM_HEIGHT;
        sprite->drawRect(10, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT, TFT_BLACK);

        if (i + menuOffset < getMenuSize(activeMenu)) {
            MenuItem& item = activeMenu[i + menuOffset];

            // Display the item name
            sprite->setTextSize(3);
            sprite->setTextColor(TFT_BLACK);
            sprite->drawString(item.name, 15, y + MENU_ITEM_HEIGHT/2 - sprite->fontHeight()/2);

            String val;
            int x_loc = 0;
            int y_loc = y + MENU_ITEM_HEIGHT/2 - sprite->fontHeight()/2;
            
            // Depending on type, display the value on the right side
            switch (item.type) {
                case MenuItem::COLOR:{
                    sprite->fillRect(PIXEL_WIDTH - 50, y, 40, MENU_ITEM_HEIGHT, *(uint16_t*)(item.value));
                    break;
                }
                case MenuItem::BOOLEAN:{
                    val = *(bool*)(item.value) ? "Enabled" : "Disabled";
                    // Set Bool String for specifioc menu items
                    if(item.name == "Clock"){
                        val = *(bool*)(item.value) ? "Digital" : "Analog";
                    }
                    if(item.name == "Temp unit"){
                        val = *(bool*)(item.value) ? "C" : "F";
                    }
                    x_loc = PIXEL_WIDTH - (30 + sprite->textWidth(val));
                    sprite->drawString(val, x_loc, y_loc);
                    break;
                }
                case MenuItem::INTEGER:{
                    int intValue = *(int*)(item.value);
                    Serial.println("OG Int Menu Item (PrintMenu)");
                    Serial.println(intValue);
                    val = String(*(int*)(item.value));
                    Serial.println("Int Menu Item (PrintMenu)");
                    Serial.println(val);
                    x_loc = PIXEL_WIDTH - (30 + sprite->textWidth(val));
                    sprite->drawString(val, x_loc, y_loc);
                    break;
                }
                case MenuItem::FLOAT:{                  
                    val = String(*(float*)(item.value), 2);
                    x_loc = PIXEL_WIDTH - (30 + sprite->textWidth(val));
                    sprite->drawString(val, x_loc, y_loc);
                    break;
                }
                case MenuItem::STRING:{
                    if (item.value != nullptr){    
                      val = String((char*)(item.value));
                      x_loc = PIXEL_WIDTH - (30 + sprite->textWidth(val));               
                      sprite->drawString(val, x_loc, y_loc);
                    }
                    break;
                }
            }
        }
    }
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
    //Get updated raw data values
    updateSensors();

    double heat_index = calculateHeatIndex(sensorData_ambi.temperature,sensorData_ambi.humidity); 

    Serial.println("Heat Index");
    Serial.println(heat_index);

    if (heat_index > 126) {
        _heatIndexColor = TFT_RED;
    } else if (heat_index > 103) {
        _heatIndexColor = 0xFDA0;
    } else if (heat_index > 91) {
        _heatIndexColor = 0xFEC0;
    } else if (heat_index > 80) {
        _heatIndexColor = 0xFFE0;
    } else {
        _heatIndexColor = TFT_GREEN;
    }

    sprite->setTextSize(3);
    sprite->setTextColor(_textColor);
    sprite->setCursor(20, 30);
    sprite->println("Heat Index:");
    sprite->setTextColor(_heatIndexColor);
    sprite->setCursor(20, 60);
    if (_temp_c){
      sprite->print(change_to_C(heat_index));
      sprite->println(" C");
    } else {
      sprite->print(heat_index);
      sprite->println(" F");
    }
  
    // Convert sweatRate to a string with 2 decimal places
    char sweatRateStr[15]; 
    dtostrf(sweatRate, 0, 2, sweatRateStr);
    strcat(sweatRateStr, " mL/min");

    Serial.println("Sweat Rate");
    Serial.println(sweatRateStr);

    sprite->setTextSize(3);
    sprite->setTextColor(_textColor);
    sprite->setCursor(20, 90);
    sprite->println("Sweat Rate:");
    sprite->setCursor(20, 120);
    sprite->println(sweatRateStr); 
}

void printAlertTemperature(TFT_eSprite* sprite){

    //Get updated raw data values
    updateSensors();

    //sprite->pushToSprite(&spr_base, 0, 0);
    sprite->setTextSize(4);
    sprite->setTextColor(TFT_RED);    // Red
    sprite->setCursor(20, 30); 
    sprite->println("WARNING High Temp:");
    sprite->setCursor(20, 80); 
    if (_temp_c){
    sprite->print(sensorData_ambi.temperature);
    sprite->println(" C!");
    } else {
    sprite->print(change_to_f(sensorData_ambi.temperature));
    sprite->println(" F!");
    }
}

void printAlertSweatRate(TFT_eSprite* sprite){

    //Get updated raw data values
    updateSensors();
    //sprite->pushToSprite(&spr_base, 0, 0);
    sprite->setTextSize(4);
    sprite->setTextColor(TFT_RED);  // Red
    sprite->setCursor(20, 30); 
    sprite->println("WARNING HIGH SWEAT RATE:");
    sprite->setCursor(20, 80); 
    sprite->printf("%d mL/min!", &sweatRate); 
}

/**********************************************************************************************
*
*                   Update Calls
*
**********************************************************************************************/
void updateBatterySprite(TFT_eSprite* sprite){

  // Static battery sprite stored across function calls
  static uint16_t* iconData;

  // Update battery sprite every 30 seconds
  static unsigned long lastUpdateTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= 30000 || lastUpdateTime == 0) {
    float bat_volt_mV = readBatteryVoltage();
    float bat_volt = bat_volt_mV/1000;
     if (bat_volt >= 3.6) {
      iconData = (uint16_t*)gImage_Full_Battery; 
    } else if (bat_volt >= 3.5) {  // (3.6 + 3.2) / 2 = 3.4
      iconData = (uint16_t*)gImage_Battery_2; 
    } else if (bat_volt >= 3.4) {  // (3.2 + 2.8) / 2 = 3.0
      iconData = (uint16_t*)gImage_Battery_1;
    } else {
      iconData = (uint16_t*)gImage_Battery_0;
    } 
    
    // Debug: Displays battery voltage on top left of screen
    if (false){
      // Print Batery voltage in mV
      sprite->fillRect(0, 0, 60, 10, _bgColor);
      char bat[10];
      snprintf(bat, sizeof(bat), "%.0fmV", bat_volt_mV); 
      sprite->setTextSize(2);
      sprite->setTextColor(_textColor, _bgColor); 
      sprite->drawString(bat, 0, 0);
    }

    lastUpdateTime = currentTime;
  }

  sprite->pushImage(295,0,25,10,iconData);
}

void updateDisplay(FaceType* currentFace, TFT_eSprite* sprite){
  #ifdef DEBUG_ENABLED_FACE
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
        case FaceType::Alert_Temperature:
            updateAlertTemperatureDisplay(sprite);
            break;
        case FaceType::Alert_Sweat_Rate:
            updateAlertSweatRateDisplay(sprite);
            break;
        default:
            Serial.println("Update display currentface mismatch");
    }

    updateBatterySprite(sprite);
    sprite->pushSprite(0,0);
}

/**********************************************************************************************
***                        Functions for updating the sprite to specific displays
***
***
**********************************************************************************************/

void updateClockDisplay(TFT_eSprite* sprite) {
  #ifdef DEBUG_ENABLED_FACE
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
  #ifdef DEBUG_ENABLED_FACE
    Serial.println("Update: Printing Raw Data Display");
  #endif
  printRawData(sprite);
}

void updateSweatRateDisplay(TFT_eSprite* sprite) {
    // Update the sprite to reflect the Sweat Rate face  
  #ifdef DEBUG_ENABLED_FACE
    Serial.println("Update: Printing Sweat Rate Display");
  #endif
  printSweatRate(sprite);
}

void updateMenuDisplay(TFT_eSprite* sprite) {
  #ifdef DEBUG_ENABLED_FACE
    Serial.println("Update: Printing Menu Display");
  #endif
  if (requestedMenuChange != nullptr) {
        activeMenu = requestedMenuChange;
        requestedMenuChange = nullptr;
        menuOffset = 0;
    }
  printMenuLayout(sprite);

}

void updateAlertTemperatureDisplay(TFT_eSprite* sprite) {
  #ifdef DEBUG_ENABLED_FACE
    Serial.println("Update: Printing Alert Display: Temperature");
  #endif
  printAlertTemperature(sprite);

}

void updateAlertSweatRateDisplay(TFT_eSprite* sprite) {
  #ifdef DEBUG_ENABLED_FACE
    Serial.println("Update: Printing Alert Display: Sweat Rate");
  #endif
  printAlertSweatRate(sprite);
}

/**********************************************************************************************
***                    Handle touch for the current state
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

void handleTouchForRawDataDisplay(touchEvent* touch, TFT_eSprite* sprite) {
    // Handle touch events specific to the Raw Data Display
    Serial.println("Handling raw data face touch event...");
    switch (touch->gesture) {
        case CST816Touch::GESTURE_NONE:
          updateSensors(true); 
    }

}

void handleTouchForSweatRateDisplay(touchEvent* touch, TFT_eSprite* sprite) {
    // Handle touch events specific to the Sweat Rate Display
    Serial.println("Handling sweat rate face touch event...");
    switch (touch->gesture) {
        case CST816Touch::GESTURE_NONE:
          updateSensors(true);
    }

}

void handleTouchForMenuDisplay(touchEvent* touch, TFT_eSprite* sprite) {

    int totalMenuItems = getMenuSize(activeMenu);

    Serial.println("Handling menu touch event...");

    switch (touch->gesture) {
        case CST816Touch::GESTURE_UP:
            Serial.println("Detected GESTURE_UP");
            menuOffset = (menuOffset < totalMenuItems - 4) ? menuOffset + 1 : totalMenuItems - 4;
            break;
        case CST816Touch::GESTURE_DOWN:
            Serial.println("Detected GESTURE_DOWN");
            menuOffset = (menuOffset > 0) ? menuOffset - 1 : 0;
            break;
        default:
            Serial.println("Detected Point Touch");
            if (touch->y <= ARROW_SIZE) {
                Serial.println("Touch on Up arrow area");
                menuOffset = (menuOffset > 0) ? menuOffset - 1 : 0;
            } else if (touch->y >= PIXEL_HEIGHT - ARROW_SIZE) {
                Serial.println("Touch on Down arrow area");
                menuOffset = (menuOffset < totalMenuItems - 4) ? menuOffset + 1 : totalMenuItems - 4;
            } else {
                Serial.println("Touch inside menu area");
                // Determine which menu item was touched based on the Y position and the menuOffset
                int menuItemTouched = menuOffset + (touch->y - ARROW_SIZE) / MENU_ITEM_HEIGHT;
                Serial.printf("Touched menu item index: %d\n", menuItemTouched);
                if (menuItemTouched >= 0 && menuItemTouched < totalMenuItems) {
                    // Call the corresponding callback for the touched menu item
                    Serial.printf("Calling callback for menu item: %s\n", activeMenu[menuItemTouched].name);
                    activeMenu[menuItemTouched].callback();
                    
                    // Given the type of value, cast and dereference it
                    switch(activeMenu[menuItemTouched].type) {
                        case MenuItem::COLOR:
                            refreshMenuItem(menuItemTouched, *(uint16_t*)activeMenu[menuItemTouched].value, sprite);
                            break;
                        case MenuItem::BOOLEAN:
                            refreshMenuItem(menuItemTouched, *(bool*)activeMenu[menuItemTouched].value ? 1 : 0, sprite);
                            break;
                        case MenuItem::INTEGER:
                            refreshMenuItem(menuItemTouched, *(int*)activeMenu[menuItemTouched].value, sprite);
                            break;
                        case MenuItem::FLOAT:
                            refreshMenuItem(menuItemTouched, *(float*)activeMenu[menuItemTouched].value, sprite);
                            break;
                        default:
                            break;
                    }
                } else {
                    Serial.println("Touched menu item out of bounds");
                }
            }
    }
    printMenuLayout(sprite);
}

int getMenuSize(MenuItem* menu) {
    if (menu == allMenuItems) {
        return sizeof(allMenuItems) / sizeof(allMenuItems[0]);
    } else if (menu == bodyParamsMenuItems) {
        return sizeof(bodyParamsMenuItems) / sizeof(bodyParamsMenuItems[0]);
    } else if (menu == resetMenuItems) {
        return sizeof(resetMenuItems) / sizeof(resetMenuItems[0]);
    } else if (menu == colorMenuItems) {
        return sizeof(colorMenuItems) / sizeof(colorMenuItems[0]);
    } else if (menu == displayMenuItems) {
        return sizeof(displayMenuItems) / sizeof(displayMenuItems[0]);
    } else if (menu == systemMenuItems) {
        return sizeof(systemMenuItems) / sizeof(systemMenuItems[0]);
    } else if (menu == topMenuItems) {
        return sizeof(topMenuItems) / sizeof(topMenuItems[0]);
    }else{
        Serial.println("Error: Menu Size Mismatch");
        return 0;
    }
}

void refreshMenuItem(int index, bool value, TFT_eSprite* sprite) {

    // Calculate the y-coordinate based on the index
    int y = ARROW_SIZE + index * MENU_ITEM_HEIGHT;
    
    // Redraw the menu item with the new value at its location
    sprite->fillRect(10, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT, _fgColor);  // Clear previous value
    sprite->setTextSize(3);
    sprite->setTextColor(TFT_BLACK);
    sprite->drawString(activeMenu[index].name, 15, y + MENU_ITEM_HEIGHT/2 - sprite->fontHeight()/2);
    sprite->drawString(String(value), PIXEL_WIDTH - 30 - sprite->textWidth(String(value)), y + MENU_ITEM_HEIGHT/2 - sprite->fontHeight()/2); 
}

void refreshMenuItem(int index, float value, TFT_eSprite* sprite) {
    // Calculate the y-coordinate based on the index
    int y = ARROW_SIZE + index * MENU_ITEM_HEIGHT;
    
    // Redraw the menu item with the new value at its location
    sprite->fillRect(10, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT, _fgColor);  // Clear previous value
    sprite->setTextSize(3);
    sprite->setTextColor(TFT_BLACK);
    sprite->drawString(activeMenu[index].name, 15, y + MENU_ITEM_HEIGHT/2 - sprite->fontHeight()/2);
    sprite->drawString(String(value), PIXEL_WIDTH - 30 - sprite->textWidth(String(value)), y + MENU_ITEM_HEIGHT/2 - sprite->fontHeight()/2); 
}

void refreshMenuItem(int index, int value, TFT_eSprite* sprite) {
    // Calculate the y-coordinate based on the index
    int y = ARROW_SIZE + index * MENU_ITEM_HEIGHT;

    Serial.println("Int Menu Item (RefreshMenu)");
    Serial.println(value);
    
    // Redraw the menu item with the new value at its location
    sprite->fillRect(10, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT, _fgColor);  // Clear previous value
    sprite->setTextSize(3);
    sprite->setTextColor(TFT_BLACK);
    sprite->drawString(activeMenu[index].name, 15, y + MENU_ITEM_HEIGHT/2 - sprite->fontHeight()/2);
    sprite->drawString(String(value), PIXEL_WIDTH - 30 - sprite->textWidth(String(value)), y + MENU_ITEM_HEIGHT/2 - sprite->fontHeight()/2); 
}

void refreshMenuItem(int index, uint16_t value, TFT_eSprite* sprite) {
    // Calculate the y-coordinate based on the index
    int y = ARROW_SIZE + index * MENU_ITEM_HEIGHT;
    
    // Redraw the menu item with the new value at its location
    sprite->fillRect(10, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT, _fgColor);  // Clear previous value
    sprite->setTextSize(3);
    sprite->setTextColor(TFT_BLACK);
    sprite->drawString(activeMenu[index].name, 15, y + MENU_ITEM_HEIGHT/2 - sprite->fontHeight()/2);
    sprite->drawString(String(value), PIXEL_WIDTH - 30 - sprite->textWidth(String(value)), y + MENU_ITEM_HEIGHT/2 - sprite->fontHeight()/2); 
}

void refreshMenuItem(int index, const char* value, TFT_eSprite* sprite) {
    // Calculate the y-coordinate based on the index
    int y = ARROW_SIZE + index * MENU_ITEM_HEIGHT;
    
    // Redraw the menu item with the new value at its location
    sprite->fillRect(10, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT, _fgColor);  // Clear previous value
    sprite->setTextSize(3);
    sprite->setTextColor(TFT_BLACK);
    sprite->drawString(activeMenu[index].name, 15, y + MENU_ITEM_HEIGHT/2 - sprite->fontHeight()/2);
    sprite->drawString(String(value), PIXEL_WIDTH - 30 - strlen(value), y + MENU_ITEM_HEIGHT/2 - sprite->fontHeight()/2); 
}

/**********************************************************************************************
*                                           Menu Actions
*
*
**********************************************************************************************/

void adjustTextColor() {
    _textColorIndex = (_textColorIndex + 1) % 16;
    _textColor = pgm_read_word(&default_4bit_palette[_textColorIndex]);
}

void adjustBgColor() {
    _bgColorIndex = (_bgColorIndex + 1) % 16;
    Serial.print("BG Color Index"); 
    Serial.println(_bgColorIndex);
    _bgColor = pgm_read_word(&default_4bit_palette[_bgColorIndex]);
}

void adjustFgColor() {
    _fgColorIndex = (_fgColorIndex + 1) % 16;
    _fgColor = pgm_read_word(&default_4bit_palette[_fgColorIndex]);
}

void adjustUTCOffset() {
    _UTC_OFF = (_UTC_OFF + 1) % UTC_MAX_OFFSET;  
    if (_UTC_OFF > ((UTC_MAX_OFFSET-1)/2)) {
        _UTC_OFF -= UTC_MAX_OFFSET;  
    }
}

void toggleTempUnit() {
    _temp_c = !_temp_c;
}

void toggleDigitalDisplay() {
    _printDigital = !_printDigital;
}

void adjustHeight() {

    _height += 4;  // Increment by four
    if (_height > 96) { // Assuming maximum height is 8 feet or 96 inches
        _height = 48;
    }
}

void adjustWeight() {
    _weight += 5;  // Increment by five
    if (_weight > 250) { // Assuming a maximum weight of 250 lbs for this example
        _weight = 80;
    }
}

void setTopLevelMenu() {
    Serial.println("Requesting Top LevelMenu");
    requestedMenuChange = topMenuItems;
}

void setDisplayMenu() {
    Serial.println("Requesting Display Menu");
    requestedMenuChange = displayMenuItems;
}

void setSystemMenu() {
    Serial.println("Requesting System Menu");
    requestedMenuChange = systemMenuItems;
}

void setBodyParamsMenu() {
    Serial.println("Requesting Body Params Menu");
    requestedMenuChange = bodyParamsMenuItems;
}

void setResetMenu() {
    Serial.println("Requesting Reset Menu");
    requestedMenuChange = resetMenuItems;
}

void toggleWifi(){
  _wifi = !_wifi;
}

/**********************************************************************************************
*                                      Memory Read Writes
*
*
**********************************************************************************************/

void loadMenuSettings(){
    Serial.println("Reading Menu values from memory.");
    preferences.begin("settings", true); // Open in read-only mode

    _textColor = preferences.getUInt("textColor", TFT_SKYBLUE);
    _bgColor = preferences.getUInt("bgColor", TFT_BLACK);
    _fgColor = preferences.getUInt("fgColor", TFT_DARKGREY);
    _printDigital = preferences.getBool("printDigital", false);
    _temp_c = preferences.getBool("tempC", true);
    _UTC_OFF = preferences.getInt("UTCoff", -4);
    _height = preferences.getInt("height", 72); // 6*12 inches as default
    _weight = preferences.getInt("weight", 155);
    _wifi = preferences.getBool("wifi", true);

    preferences.end();
}

void writeMenuSettings(){
    Serial.println("Writing Menu values to memory.");
    preferences.begin("settings", false); // Open in read-write mode

    preferences.putUInt("textColor", _textColor);
    preferences.putUInt("bgColor", _bgColor);
    preferences.putUInt("fgColor", _fgColor);
    preferences.putBool("printDigital", _printDigital);
    preferences.putBool("tempC", _temp_c);
    preferences.putInt("UTCoff", _UTC_OFF);
    preferences.putInt("height", _height);
    preferences.putInt("weight", _weight);
    preferences.putBool("wifi", _wifi);

    preferences.end();
}

void eraseLoggedMenuSettings(){
  Serial.println("Clearing menu settings from memory");
  preferences.begin("settings");
  preferences.clear();
  preferences.end();
}

void noop(){}