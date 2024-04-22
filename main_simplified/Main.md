## Main Module Documentation

### Overview
The `Main.ino` file contains the core logic for initializing and running the device's main loop. It handles hardware setup, power management (sleep/wake operations), display management, and continuous sensor data processing and logging. It also manages WiFi connectivity and button interactions for sleeping the device or toggling the display state.

### Initialization
- **`void setup()`**
  - Initializes the serial communication, buttons, MCU, temperature sensors, AD5933 bioimpedance sensor, and displays battery and sensor logs. Sets up deep sleep wake-up conditions.
  
- **`void initMCU()`**
  - Configures initial IO states, loads menu settings, initializes the TFT display, and connects to WiFi if enabled.

### Main Loop Functions
- **`void loop()`**
  - The main loop continuously checks button states, manages WiFi connectivity, handles touch inputs, and updates the display. It also manages data logging when the screen is off and handles sleep mode initiation.

#### Sleep and Screen Management
- Button interactions are used to toggle the device's sleep mode and screen state:
  - **`sleep_device`** button puts the device into deep sleep.
  - **`sleep_screen`** button toggles the display on and off.

#### Touch and Display Management
- Processes touch gestures and updates to the user interface through the display control module.
- **`void handleGestures(FaceType *currentFace, touchEvent *currTouch)`**
  - Manages screen transitions and interactions based on touch gestures.
- **`void updateDisplay(FaceType* currentFace, TFT_eSprite* sprite)`**
  - Updates the content shown on the display based on the current interface state.

### Power Management
- The device can enter deep sleep mode to conserve power, with wake-up triggered by a specific button press.
- **`esp_sleep_enable_ext0_wakeup()`** function is used to set up the wake-up condition based on a button press.

### Dependencies

- **Libraries**: Arduino standard libraries, OneButton for button handling, TFT_eSPI for display management, Wire for I2C communication, CST816_TouchLib for touch input, and ESP specific libraries for sleep management.
- **External Files**: `functions.h`, `DisplayControl.h`, `pin_config.h`

This module orchestrates the device's operation, coordinating between user interactions, sensor data processing, and display updates, ensuring efficient power management and system responsiveness.
