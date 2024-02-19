#ifndef SWEATRATEFACE_H
#define SWEATRATEFACE_H

#include "Face.h"

class SweatRateFace : public Face {
public:
  SweatRateFace();
    void enter() override;
    void update() override;
    void exit() override;
    FaceType getType() const override { return FaceType::SweatRate; }

protected:
  float sweatRate;

private:

    bool menuOpen = false;
    
    unsigned long UPDATE_INTERVAL = 15000; // 30 seconds in milliseconds
    unsigned long lastUpdate = 0;
};

#endif