#ifndef MENUFACE_H
#define MENUFACE_H

#include "Face.h"
class MenuFace : public Face {
public:
    MenuFace();

    void enter() override;
    void update() override;
    void exit() override;
    FaceType getType() const override { return FaceType::MenuFace; }
    
    // Custom functions. 
    void changeTextColor();

protected:
  void writeSettingsToFlash();

private:
    void updateMenuItem();
    void updateUTCOffset();
    void printMenuLayout();

    int currentColorIndex = 0;
    unsigned long UPDATE_INTERVAL = 1000; // 1 seconds in milliseconds
    unsigned long lastUpdate = UPDATE_INTERVAL + 1;
};

#endif
