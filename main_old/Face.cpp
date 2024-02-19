#include "Face.h"

// Declaration of static members without initialization
uint16_t Face::_textColor;
uint16_t Face::_bgColor;
uint16_t Face::_fgColor;
bool Face::_printDigital;
bool Face::_temp_c;
long Face::_UTC_OFF;


Face::Face() = default;

Face::~Face() = default;

void Face::handleTouch(const touchEvent& touchInfo) {
    for (int i = 0; i < currentTouchRegionIndex; i++) {
        TouchRegion &region = touchRegions[i];
        if (touchInfo.x >= region.xStart && touchInfo.x <= region.xEnd &&
            touchInfo.y >= region.yStart && touchInfo.y <= region.yEnd) {
            region.callback(this);
            return;  // Exit after first matched touch region to avoid multiple callbacks
        }
    }
}

void Face::addTouchRegion(int xStart, int xEnd, int yStart, int yEnd, TouchCallback callback) {
    if (currentTouchRegionIndex < MAX_TOUCH_REGIONS) {
        touchRegions[currentTouchRegionIndex] = TouchRegion(xStart, xEnd, yStart, yEnd, callback);
        currentTouchRegionIndex++;
    } else {
        Serial.println("!!! MAX TOUCH REGIONS REACHED !!!");
    }
}

void Face::resetTouchRegions() {
    for (int i = 0; i < MAX_TOUCH_REGIONS; i++) {
        touchRegions[i] = TouchRegion(); // Reset to the default state
    }
    currentTouchRegionIndex = 0;
}

Face::TouchRegion::TouchRegion() : xStart(0), xEnd(0), yStart(0), yEnd(0), callback(nullptr) {}

Face::TouchRegion::TouchRegion(int xs, int xe, int ys, int ye, TouchCallback cb) : xStart(xs), xEnd(xe), yStart(ys), yEnd(ye), callback(cb) {}

void Face::loadGlobalSettings(){
    // Placeholder code: replace with logic to load from flash
    _textColor = TFT_SKYBLUE;
    _bgColor = TFT_BLACK;
    _fgColor = TFT_DARKGREY;
    _printDigital = true;
    _temp_c = false;
    _UTC_OFF = -4;  
}
