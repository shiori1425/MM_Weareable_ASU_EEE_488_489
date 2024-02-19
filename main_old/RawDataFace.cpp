#include "RawDataFace.h"
// #include "img_rawdata.h"

// Constructor
RawDataFace::RawDataFace() {
  // Initialize lastUpdateTime to the current time
  lastUpdateTime = millis();
}

void RawDataFace::enter() {
  Serial.println("Enter Raw Data Face");
  delay(250);
  // Create a sprite
  spr.createSprite(TFT_HEIGHT, TFT_WIDTH);  // Assumes standard width and height. Adjust if different.
  spr.fillSprite(_bgColor);
  spr.setTextColor(_textColor, _bgColor);
  spr.pushToSprite(&spr_base, 0, 0);
}

void RawDataFace::update() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = currentMillis;
    // perform interval update
    Serial.println("Entering periodic Update on RawData Face");
    // Getting the readings
    externalReadings = readExternalSensors();
    internalReadings = readInternalSensors();
    skinRes = calcSkinRes();

  }

  int x = 15;

  spr.pushToSprite(&spr_base, 0, 0);
  
  // Displaying data points on sprite
  spr.setTextSize(3);

  spr.setCursor(x, 20);
  spr.print("Bod Tmp: ");

  if (_temp_c){
  spr.print(externalReadings.temp);
  spr.println(" C");
  } else {
  spr.print(change_to_f(externalReadings.temp));
  spr.println(" F");
  }

  spr.setCursor(x, 80);
  spr.print("Amb Tmp: ");
  if (_temp_c){
  spr.print(internalReadings.temp);
  spr.println(" C");
  } else {
  spr.print(change_to_f(internalReadings.temp));
  spr.println(" F");
  }

  spr.setCursor(x, 50);
  spr.print("Bod Hum: ");
  spr.print(externalReadings.Humidity);
  spr.println(" %");

  spr.setCursor(x, 110);
  spr.print("Amb Hum: ");
  spr.print(internalReadings.Humidity);
  spr.println(" %");

  char str_buf[20] = {0};
  spr.setCursor(x, 140);
  spr.print("Skin Res: ");
  spr.print(formatResistance(skinRes, str_buf));
  //spr.drawGlyph(0x03A9);


  // Push the sprite to the screen
  spr.pushSprite(0, 0);
}

void RawDataFace::exit() {
  // Delete the sprite to free up the RAM
  Serial.println("Exiting Data Menu");
  spr.deleteSprite();
  delay(250);
}
