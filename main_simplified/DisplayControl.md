## Display Control Module Documentation

### Overview
The `DisplayControl` module handles the graphical user interface and user interactions through a TFT display and touch input. It manages various display states (or "faces") like clock views, menus, and data displays. It also incorporates gesture handling for navigating through different screens and adjusting settings.

### Functions

#### Initialization and Configuration
- **`void initializeTFT()`**
  - Initializes the TFT display with specific settings for orientation, color, and brightness.
- **`void resetDisplay()`**
  - Resets the display to a default state.

#### Touch Handling
- **`touchEvent getTouch()`**
  - Retrieves the latest touch event, including position and gesture type.
- **`void handleGestures(FaceType *currentFace, touchEvent *currTouch)`**
  - Processes touch gestures to navigate between different display faces or interact with menus.

#### Display Update Functions
- **`void updateDisplay(FaceType* currentFace, TFT_eSprite* sprite)`**
  - Central function to refresh the display based on the current face type.
- **`void updateClockDisplay(TFT_eSprite* sprite)`**
  - Updates the display to show the clock face.
- **`void updateRawDataDisplay(TFT_eSprite* sprite)`**
  - Shows sensor readings like temperature and humidity on the display.
- **`void updateSweatRateDisplay(TFT_eSprite* sprite)`**
  - Displays calculated sweat rate and related data.
- **`void updateMenuDisplay(TFT_eSprite* sprite)`**
  - Renders the menu interface based on the active menu items.
- **`void updateAlertTemperatureDisplay(TFT_eSprite* sprite)`**
  - Displays a temperature alert view.
- **`void updateAlertSweatRateDisplay(TFT_eSprite* sprite)`**
  - Shows an alert if the sweat rate exceeds a certain threshold.

#### Print Functions
- **`void printDigitalClock()`**
  - Prints the time in a digital format.
- **`void printAnalogClock()`**
  - Displays an analog clock face.
- **`void printMenu()`**
  - Renders the menu items on the display.
- **`void printRawData(TFT_eSprite* sprite)`**
  - Displays the raw sensor data.
- **`void printSweatRate(TFT_eSprite* sprite)`**
  - Shows the sweat rate and related calculations.
- **`void printRightHalfDate(TFT_eSprite* sprite)`**
  - Displays the date on the right half of the display.
- **`void printAlertTemperature(TFT_eSprite* sprite)`**
  - Shows a high temperature alert.
- **`void printAlertSweatRate(TFT_eSprite* sprite)`**
  - Alerts the user to a high sweat rate.

#### Menu Management
- **`void loadMenuSettings()`**
  - Loads menu settings from non-volatile memory.
- **`void writeMenuSettings()`**
  - Saves current menu settings to memory.
- **`void eraseLoggedMenuSettings()`**
  - Clears menu settings from memory.

### Dependencies

- **Libraries**: TFT_eSPI, CST816_TouchLib
- **External Files**: functions.h, Types.h, bat_power.h, SensorControl.h

This module serves as the interface layer, allowing users to interact with the device through visual feedback and touch inputs. It supports various operational modes and settings adjustments, facilitating user engagement and control.