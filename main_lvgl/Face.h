#ifndef FACE_H
#define FACE_H

#include "functions.h"
#include <functional>

class Face {
public:
    Face();
    
    virtual void enter() = 0;
    virtual void update() = 0;
    virtual void exit() = 0;
    virtual FaceType getType() const = 0;
    virtual ~Face();

    void handleTouch(const touchEvent& touchInfo);
    static void loadGlobalSettings();


protected:
    unsigned long lastUpdateTime;

    // Touch region handling
    using TouchCallback = std::function<void(Face*)>;

    void addTouchRegion(int xStart, int xEnd, int yStart, int yEnd, TouchCallback callback);
    void resetTouchRegions();

    struct TouchRegion {
        int xStart, xEnd, yStart, yEnd;
        TouchCallback callback;
        TouchRegion();
        TouchRegion(int xs, int xe, int ys, int ye, TouchCallback cb);
    };

    static uint16_t _textColor;
    static uint16_t _bgColor;
    static uint16_t _fgColor;
    static bool _printDigital;
    static bool _temp_c;
    static long _UTC_OFF;

private:
    static const int MAX_TOUCH_REGIONS = 8;  // Adjust as necessary
    TouchRegion touchRegions[MAX_TOUCH_REGIONS];
    int currentTouchRegionIndex;
    void loadSettingsFromFlash();
};

#endif
