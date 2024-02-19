#include "MenuFace.h"

// Constructor
MenuFace::MenuFace() {
  // Initialize lastUpdateTime to the current time
  lastUpdateTime = millis();
}

void MenuFace::enter() {
    Serial.println("Enter Menu Face");
    // Initialize touch regions
    delay(250);
    // Initialize Screen Background images
    spr.createSprite(TFT_HEIGHT, TFT_WIDTH);
    spr.pushToSprite(&spr_base, 0, 0);
    spr.setTextSize(2);
    printMenuLayout();
    spr.pushSprite(0,0);
    //addTouchRegion(MENU_TXT_Y, yE, MENU_TXT_X, MENU_WIDTH, [this](Face* instance) { this -> updateMenuItem();});

}

void MenuFace::update() {

    unsigned long currentMillis = millis();

    if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
        lastUpdate = currentMillis;
        // perform interval update
            // Draw a grid
    }

    // Push the sprite to the screen
    spr.setTextSize(2);
    spr.pushSprite(0,0);
}

void MenuFace::exit() {
    Serial.println("Exit Menu Face");
    spr.deleteSprite();
    delay(250);
}


void MenuFace::changeTextColor(){ 
  Serial.println("Increment CLock Color");
  currentColorIndex = (currentColorIndex + 1) % 16; // Cycle through the 16 colors
  Serial.print("New Clock Color:");
  _textColor = pgm_read_word(&default_4bit_palette[currentColorIndex]);
  Serial.print(_textColor);
}

void MenuFace::updateMenuItem(){
  Serial.println("perform menu item update");
}

void MenuFace::updateUTCOffset(){
    adjustTime(_UTC_OFF * 3600); // apply offset in seconds
}

void MenuFace::writeSettingsToFlash(){

}

void MenuFace::printMenuLayout(){

  // Due to sprites not respecting TFT rotation, we swap X and Y coordinates 
        // to manually rotate the design 90 degrees on the sprite.
  int MENU_X = 5;
  int MENU_Y = 5;
  int MENU_HEIGHT = 300;
  int MENU_WIDTH = 150;
  int MENU_TXT_X = MENU_X + 5;
  int MENU_TXT_Y = MENU_X + 5;
  int MENU_NL_SPACE = spr.fontHeight() + 10;
  int LINE_NUM = 0;
  int LINE_WEIGHT = 3;
  spr.fillRect(0, 0, TFT_HEIGHT, TFT_WIDTH,  _fgColor);

  // Set Menu Text Settings
  spr.setTextSize(2);
  spr.setTextColor(_bgColor);

  // New Menu Line Item
  const char* menu_item_1 = "Text Color";
  spr.drawString(menu_item_1,MENU_TXT_Y, MENU_TXT_X); 
  const char* menu_select_1 = "pixels";
  int yE = MENU_TXT_X + MENU_HEIGHT - spr.textWidth(menu_select_1) - 5;
  spr.drawString(menu_select_1, MENU_TXT_Y, yE);
  
  // New Menu Line Item
  LINE_NUM++;
  spr.fillRect(MENU_Y--, MENU_TXT_X-- + MENU_NL_SPACE * LINE_NUM + 3, MENU_HEIGHT++, MENU_TXT_X++ + MENU_NL_SPACE * LINE_NUM + 3,  TFT_DARKGREY);
  spr.drawString("BG Color",MENU_TXT_Y, MENU_TXT_X + MENU_NL_SPACE * LINE_NUM + 5 ); 

  // New Menu Line Item
  LINE_NUM++;
  spr.fillRect(MENU_Y--, MENU_TXT_X-- + MENU_NL_SPACE * LINE_NUM + 3, MENU_HEIGHT++, MENU_TXT_X++ + MENU_NL_SPACE * LINE_NUM + 3,  TFT_DARKGREY);
  spr.drawString("FG Color",MENU_TXT_Y, MENU_TXT_X + MENU_NL_SPACE * LINE_NUM + 5);

  // New Menu Line Item
  LINE_NUM++;
  spr.fillRect(MENU_Y--, MENU_TXT_X-- + MENU_NL_SPACE * LINE_NUM + 3, MENU_HEIGHT++, MENU_TXT_X++ + MENU_NL_SPACE * LINE_NUM + 3,  TFT_DARKGREY);
  spr.drawString("UTC Offset",MENU_TXT_Y, MENU_TXT_X + MENU_NL_SPACE * LINE_NUM + 5);

  // New Menu Line Item
  LINE_NUM++;
  spr.fillRect(MENU_Y--, MENU_TXT_X-- + MENU_NL_SPACE * LINE_NUM + 3, MENU_HEIGHT++, MENU_TXT_X++ + MENU_NL_SPACE * LINE_NUM + 3,  TFT_DARKGREY);
  spr.drawString("Temp Unit",MENU_TXT_Y, MENU_TXT_X + MENU_NL_SPACE * LINE_NUM + 5);

  // New Menu Line Item
  LINE_NUM++;
  spr.fillRect(MENU_Y--, MENU_TXT_X-- + MENU_NL_SPACE * LINE_NUM + 3, MENU_HEIGHT++, MENU_TXT_X++ + MENU_NL_SPACE * LINE_NUM + 3,  TFT_DARKGREY);
  spr.drawString("Digital",MENU_TXT_Y, MENU_TXT_X + MENU_NL_SPACE * LINE_NUM + 5);
}