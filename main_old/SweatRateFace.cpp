#include "SweatRateFace.h"
// #include "img_sweatrate.h"

// Constructor
SweatRateFace::SweatRateFace() {
  // Initialize lastUpdateTime to the current time
  lastUpdateTime = millis();
}

void SweatRateFace::enter() {
    Serial.println("Enter SweatRate Face");
    // Initialize touch regions
    delay(250);
    // Initialize Screen Background images
    spr.createSprite(TFT_HEIGHT, TFT_WIDTH);
    spr.fillSprite(_bgColor);
    spr.setTextColor(_textColor, _bgColor);
    spr.pushToSprite(&spr_base, 0, 0);
    spr.pushSprite(0,0);
}

void SweatRateFace::update() {
    unsigned long currentMillis = millis();

    if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
      lastUpdate = currentMillis;
      // perform interval update
      Serial.println("Entering periodic Update on SweatRate Face");
      // get Sweat Rate
      sweatRate = calcSweatRate();
    }
    
    spr.pushToSprite(&spr_base, 0, 0);

    spr.setTextSize(4);
    spr.setCursor(20, 30); // Centering might need adjusting
    spr.println("Sweat Rate:");
    spr.setCursor(20, 80); // Centering might need adjusting
    spr.printf("%d mL/min", sweatRate); // Placeholder value

    // Push the sprite to the screen
    spr.pushSprite(0, 0);
}

void SweatRateFace::exit() {
    // Delete the sprite to free up the RAM
    Serial.println("Exiting SweatRate menu");
    spr.deleteSprite();
    delay(250);
}