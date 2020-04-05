/**
    OpenSimPedals : https://github.com/manoukianv/OpenSimPedals
    Copyright (C) 2020  Vincent Manoukian

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>

 ***************************************************************************************
 * Benchmark (Trame per second) :
 *    3 loadcells : 26,7 tps
 *    2 loadcells : 40,1 tps
 *    1 loadcell  : 80,1 tps
 * 
 **/


#include <Arduino.h>

#include <ADS123X.h>
#include <Joystick.h>
#include <U8g2lib.h>
#include <EEPROMex.h>
#include <AS5600.h>

// *************************************** FEATURES ***************************************

#define DEBUG     false
#define THROTTLE  true
#define BRAKE     false
#define CLUTCH    false

// ************************************** NOT TOUCH ***************************************

#define RELEASE "1.0.0"

// check if a is in [0-100], else return b
#define CHECK_RANGE(value, defval)  ( ( ((value) < 0) || ((value) > 100) ) ? (defval): (value) )
// filter ADC signal and apply the apps gain
// at 80SPS, with 44nV imprecision (17bit/24) => result is on 15bits, ok for the int16
#define ADC_FITLER(value)  (value >> 7)
// Apply the gain and max the output if overload
#define JOYSTICK_GAIN(value, gain) (((value) * 100)/(gain)  > INT16_MAX) ? INT16_MAX : ((value) * 100)/(gain)
#define JOYSTICK_DEADZONE(value, deadzone) ( ((value) / 32767.0) < ((deadzone) * 0.01) ) ? 0 : (value)

// **************************************** PINOUT ****************************************

#define ADS_THROTTLE_PDWN   4
#define ADS_THROTTLE_DOUT   5
#define ADS_THROTTLE_SCLK   6

#define ADS_BRAKE_PDWN      7
#define ADS_BRAKE_DOUT      8
#define ADS_BRAKE_SCLK      9

#define ADS_CLUTCH_PDWN     14
#define ADS_CLUTCH_DOUT     16
#define ADS_CLUTCH_SCLK     10

// **************************************** EEPROM ****************************************
const int maxAllowedWrites = 80;
const int memBase          = 350;

byte throttle_pct_range;
byte throttle_pct_deadzone;
byte brake_pct_range;
byte brake_pct_deadzone;
byte clutch_pct_range;
byte clutch_pct_deadzone;

int throttle_pct_range_addr;
int throttle_pct_deadzone_addr;
int brake_pct_range_addr;
int brake_pct_deadzone_addr;
int clutch_pct_range_addr;
int clutch_pct_deadzone_addr;

// **************************************** Joystick ****************************************
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK, 
                    0, 0,
                    true, true, true, 
                    false, false, false,
                    false, false, 
                    false, false, false);

#ifdef THROTTLE
AMS_5600 throttle_sensor;
#endif
#ifdef BRAKE
ADS123X brake_sensor;
#endif
#ifdef CLUTCH
ADS123X clutch_sensor;
#endif

// **************************************** Screen ****************************************
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
uint8_t keycode = 0;

const char *parameters_list = 
  "Throttle Range\n"
  "Throttle Dead Zone\n"
  "Brake Range\n"
  "Brake Dead Zone\n"
  "Clutch Range\n"
  "Clutch Dead Zone";