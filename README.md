This project is an open hardware Load Cell pedals for sim racing.

# Presentation

## Introduction
Make / order the PCB (or circuit on a breadboard), flash the firwmare, and connect the USB cable to the PC : you have a new pedals Set :-D

Build tutorial are available here : [Instructables](https://www.instructables.com/id/OpenSimPedals-DIY-3D-Printed-Sim-Racing-Pedals-Wit/)

## Main feature
* Standalone : not required software on PC, pedals are seen like a Joystick, just setup the game ;)
* In the main.h, you can choose which pedal you want, you can have 1,2 or 3 pedals.
* 3 LoadCell with 16b accuracy
* High sampling/second : 80-1 Pedal / 40-2 Pedals / 27-3 Pedals
* Automatic start calibration
* Settings for Throttle / Brake / Clutch range and start DeadZone stored in permanently memory
* Oled screen to check range (click on +/- button) and setup parameters
* use the light keyboard and Oled Screen to setup parameters

# Hardware

## Pedals 
OpenSimPedals pedals will be share on thingiverse. They are designed to be print with this specification :
* Throttle : loadCell, 40kg push on pedal
* Brake : loadCell, 100kg push on pedal
* Clutch : loadCell, 20kg push on pedal
The BOM, will be shared on thingiverse

## Electronic

### The support board.
* The "Electronic" directory contain : the schematic, the gerber (to order board on web site)
* CPU : Arduino Micro Pro (5V-16Mhz)
* OLED : SSD1306 Oled 0.96 I2C screen
* ADC : 3*ADS1232
* Connector (KF301-4P) and button (6mm/6mm push button)

### The firmware
* Clone this repository in VisualStudioCode 
* Platformio plugin allow you to build and upload on the arduino micro


# Licences

Software under GPL v3

External component used :
* used and modify ADS123X from https://github.com/HamidSaffari/ADS123X
* used and modify Joystick from https://github.com/MHeironimus/ArduinoJoystickLibrary
* used u8G2
* used EEPROMex

