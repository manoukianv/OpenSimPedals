#include <Arduino.h>

#include <ADS123X.h>
#include <Joystick.h>
#include <U8g2lib.h>
#include <EEPROMex.h>

// *************************************** FEATURES ***************************************

#define DEBUG false

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
#define ADS_BRAKE_THROTTLE_DOUT   8
#define ADS_BRAKE_THROTTLE_SCLK   9
#define ADS_BRAKE_THROTTLE_PDWN   7
#define ADS_BRAKE_THROTTLE_GAIN0  4
#define ADS_BRAKE_THROTTLE_GAIN1  5
#define ADS_BRAKE_THROTTLE_SPEED  6
#define ADS_BRAKE_THROTTLE_A0     16
#define ADS_BRAKE_THROTTLE_A1     10

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
                    4, 0,
                    true, true, true, 
                    false, false, false,
                    false, false, 
                    false, false, false);

ADS123X throttle_sensor;
ADS123X break_sensor;
ADS123X clutch_sensor;

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


