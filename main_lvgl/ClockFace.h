#ifndef CLOCKFACE_H
#define CLOCKFACE_H

#include "Face.h"
#include "TimeLib.h"

class ClockFace : public Face {
public:
    ClockFace();

    void enter() override;
    void update() override;
    void exit() override;
    FaceType getType() const override { return FaceType::Clock; }

protected:
    

private:
    void printDigitalClock();
    void printAnalogClock();

    int _hh = 10; 
    int _mm = 0;
    int _ss = 0;
    int _m = 1;
    int _d = 1;
    int _y = 2023;
    
    unsigned long UPDATE_INTERVAL = 120000; // 2 minutes in milliseconds
    unsigned long lastUpdate = UPDATE_INTERVAL + 1;
};

#endif
