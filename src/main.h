#include <Arduino.h>

#include <ADS123X.h>
#include <Joystick.h>
#include <U8g2lib.h>
#include <EEPROMex.h>

#define DEBUG true

// ************************************************************** PINOUT ************************************************************
#define ADS_BRAKE_THROTTLE_DOUT   4
#define ADS_BRAKE_THROTTLE_SCLK   5
#define ADS_BRAKE_THROTTLE_PDWN   6
#define ADS_BRAKE_THROTTLE_GAIN0  7
#define ADS_BRAKE_THROTTLE_GAIN1  8
#define ADS_BRAKE_THROTTLE_SPEED  9
#define ADS_BRAKE_THROTTLE_A0     10
#define ADS_BRAKE_THROTTLE_A1     11

// ************************************************************** EEPROM ************************************************************
const int maxAllowedWrites = 80;
const int memBase          = 350;
byte throttle_pct_range;
byte throttle_pct_deadzone;
byte brake_pct_range;
byte brake_pct_deadzone;
byte clutch_pct_range;
byte clutch_pct_deadzone;
const char *parameters_list = 
  "Throttle Range\n"
  "Throttle Dead Zone\n"
  "Brake Range\n"
  "Brake Dead Zone\n"
  "Clutch Range\n"
  "Clutch Dead Zone";

// ************************************************************ Joystick ************************************************************
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
                    JOYSTICK_TYPE_MULTI_AXIS, 0, 0,
                    false, false, false, false, false, false,
                    false, false, true, true, true);

ADS123X break_throttle_sensor;

// ************************************************************ Screen ************************************************************
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
uint8_t keycode = 0;

