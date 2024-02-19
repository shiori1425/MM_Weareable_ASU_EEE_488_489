#include "ClockFace.h"

// Constructor
ClockFace::ClockFace() {
  // Initialize lastUpdateTime to the current time
  lastUpdateTime = millis();
}

void ClockFace::enter() {
    Serial.println("Enter Clock Face");
    delay(250);
    // Initialize Screen Background images    
    spr.createSprite(TFT_HEIGHT, TFT_WIDTH);  // Assumes standard width and height. Adjust if different.
    spr.fillSprite(TFT_BLACK);
    spr.pushToSprite(&spr_base, 0, 0);
    spr.setTextColor(_textColor, _bgColor);
    spr.pushSprite(0,0);
}

void ClockFace::update() {

    unsigned long currentMillis = millis();

    if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
        lastUpdate = currentMillis;
        Serial.println("Entering periodic Update on Clock Face");
        // perform interval update
    }

    // print clock to sprite
    printDigitalClock();

    // Push the sprite to the screen
    spr.pushSprite(0, 0);
}

void ClockFace::exit() {
    Serial.println("Exit Clock Face");
    spr.deleteSprite();
    delay(250);
}


void ClockFace::printDigitalClock(){
  // Set sprite to display digital clock
 
  // Print time
    //Serial.println("Update Clock Face");
    const int TIME_SIZE = 6; // Adjust this as per your desired size
    const int TIME_X_POS = 15; // Starting X position. Adjust to move horizontally.
    const int TIME_Y_POS = 55; // Starting Y position. Adjust to move vertically.
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d:%02d", hourFormat12(),minute(), second());
    // Set the text properties
    spr.setTextSize(TIME_SIZE);
    spr.setCursor(TIME_X_POS, TIME_Y_POS);
    // Print the time to the sprite
    spr.print(timeStr);

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

    spr.setTextSize(DATE_SIZE);
    spr.setCursor(DATE_X_POS, DATE_Y_POS);
    // Print the time to the sprite
    spr.print(dateStr);
}

void ClockFace::printAnalogClock(){

}
