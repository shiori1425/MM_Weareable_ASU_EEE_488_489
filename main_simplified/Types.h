#ifndef TYPES_H
#define TYPES_H

#include "stdint.h"
#include <CST816_TouchLib.h>
using namespace MDO;

enum class FaceType {
    Clock,
    RawData,
    SweatRate,
    Menu,
    Alert_Temperature, 
    Alert_Sweat_Rate
};

enum FaceState {
    CLOCK,
    SWEAT_RATE,
    RAW_DATA,
    MENU,
    NUM_STATES_FACE
};


struct touchEvent {
    int x;
    int y;
    CST816Touch::gesture_t gesture;
    bool gestureEvent;
};

struct FaceTransition {
  FaceType current;
  FaceType left;
  FaceType right;
};

#endif
