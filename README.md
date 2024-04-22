# Project ReadMe

## Overview
This project integrates a comprehensive system for monitoring and displaying environmental and physiological parameters such as temperature, humidity, and sweat rate. The firmware facilitates sensor data acquisition, processing, display management, and user interaction through a touch-enabled graphical user interface and physical buttons.

## File Hierarchy and Dependencies

The project is structured with a main entry file and various modules that are interdependent:

- **`Main.ino`**
  - Initializes the system, handling the main loop that orchestrates operations like sensor reading, user input, and display and power state management. For detailed functionality, refer to `Main_Module_Description.md`.
  - **Dependencies**:
    - `pin_config.h`
    - `functions.h`
    - `DisplayControl.h`

- **`DisplayControl.h/cpp`**
  - Manages the device's display, including rendering different screens and handling user input via touch gestures. Refer to `DisplayControl_Module_Description.md` for more details.
  - **Dependencies**:
    - `functions.h`
    - `Types.h`
    - `bat_power.h`
    - `SensorControl.h`

- **`SensorControl.h/cpp`**
  - Manages sensor data acquisition, processing, and logging. It calculates sweat rate and interfaces with temperature, humidity, and bioimpedance sensors. Detailed functions are described in `SensorControl_Module_Description.md`.
  - **Dependencies**:
    - `functions.h`
    - `Types.h`

- **`Functions.h/cpp`**
  - Provides utility functions including WiFi management, time synchronization, data formatting, and memory operations. For a detailed description of each function, see `Functions_Module_Description.md`.
  - **Dependencies**:
    - `pin_config.h`
    - `Types.h`

- **`pin_config.h`**
  - Contains configuration for hardware pin assignments.

- **`Types.h`**
  - Defines custom types and enumerations used throughout the project.

- **`bat_power.h`**
  - Stores raw image data for battery level display.


## Hardware Description

The system is built around an ESP32 microcontroller (specifically the T-Display-S3 from LilyGo), interfacing with a TFT display for output and a touch panel for input. Temperature and humidity are measured via HTU31D sensors, while bioimpedance is read using an AD5933 sensor. WiFi connectivity is included for some limited network-related features.

## Configuration

Set the WiFi SSID and password in `pin_config.h` before building the project if you wish to connect to wifi for data streamikng or time automatic time sync. 

## Building and Flashing

Use the Arduino IDE to open `main_simplified.ino`, verify the installation of required libraries, choose the appropriate board and port, and upload the firmware to the ESP32.

## Battery Level Icons

The battery level is represented through custom icons in the firmware, formatted for the TFT_eSPI library and included as monochrome images in `bat_power.h`.