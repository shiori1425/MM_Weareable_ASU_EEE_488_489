#ifndef RAWDATAFACE_H
#define RAWDATAFACE_H

#include "Face.h"

class RawDataFace : public Face {
public:
  RawDataFace();
    void enter() override;
    void update() override;
    void exit() override;
    FaceType getType() const override { return FaceType::RawData; }

    // Raw Data vars
    SensorReadings externalReadings;
    SensorReadings internalReadings;
    float skinRes;

protected:


private:

    bool menuOpen = false;
    unsigned long UPDATE_INTERVAL = 5000; // 30 seconds in milliseconds
    unsigned long lastUpdate = UPDATE_INTERVAL +1;
};

#endif