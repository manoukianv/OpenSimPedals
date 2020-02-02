#include <main.h>

// check if a is in [0-100], else return b
#define CHECK_RANGE(value, defval)  ( ( ((value) < 0) || ((value) > 100) ) ? (defval): (value) )
// filter ADC signal and apply the apps gain
// at 80SPS, with 44nV imprecision (17bit/24) => result is on 15bits, ok for the int16
#define ADC_FITLER(value)  (value >> 7)
// Apply the gain and max the output if overload
#define JOYSTICK_GAIN(value, gain) (((value) * 100)/(gain)  > INT16_MAX) ? INT16_MAX : ((value) * 100)/(gain)

int throttle_pc = 100;
int throttle_deadzone_pc = 1;

// prototype
void drawBox();
void displayData(int16_t throttle, int16_t brake, int16_t clutch);

void loadDataEEPROM() {
  long address;
  address               = EEPROM.getAddress(sizeof(byte)); // read in eeprom the throttle param
  throttle_pct_range    = EEPROM.readByte(address);
  address               = EEPROM.getAddress(sizeof(byte));
  throttle_pct_deadzone = EEPROM.readByte(address);
  address               = EEPROM.getAddress(sizeof(byte)); // read in eeprom the break param
  brake_pct_range       = EEPROM.readByte(address);
  address               = EEPROM.getAddress(sizeof(byte));
  brake_pct_deadzone    = EEPROM.readByte(address);
  address               = EEPROM.getAddress(sizeof(byte)); // read in eeprom the clutch param
  clutch_pct_range      = EEPROM.readByte(address);
  address               = EEPROM.getAddress(sizeof(byte));
  clutch_pct_deadzone   = EEPROM.readByte(address);

  throttle_pct_range    = CHECK_RANGE(throttle_pct_range, 100);
  brake_pct_range       = CHECK_RANGE(brake_pct_range, 100);
  clutch_pct_range      = CHECK_RANGE(clutch_pct_range, 100);
  throttle_pct_deadzone = CHECK_RANGE(throttle_pct_range, 2);
  brake_pct_deadzone    = CHECK_RANGE(brake_pct_range, 2);
  clutch_pct_deadzone   = CHECK_RANGE(clutch_pct_range, 2);
}

void setup() {
  // Initialize Joystick Library
  #if !DEBUG
    Joystick.begin();
    // Set the range throttle
    Joystick.setThrottleRange (0, INT16_MAX);
    Joystick.setBrakeRange    (0, INT16_MAX);
    Joystick.setRzAxisRange   (0, INT16_MAX);
  #else
    delay(2000);
    Serial.begin(9600);
    delay(1000);
    Serial.println("test");
  #endif

  // Init the eeprom data
  EEPROM.setMemPool(memBase, EEPROMSizeMicro);
  EEPROM.setMaxAllowedWrites(maxAllowedWrites);
  loadDataEEPROM();

  // Init screen
  u8g2.begin(/*Select=*/ 14, /*Right/Next=*/ A1, /*Left/Prev=*/ A2, /*Up=*/ A0, /*Down=*/ A3, /*Home/Cancel=*/ 15);
  drawBox();
  displayData(random(0, INT16_MAX), random(0, INT16_MAX), random(0, INT16_MAX));
  
  // Init the ADC
  long value_long;
  break_throttle_sensor.begin(ADS_BRAKE_THROTTLE_DOUT, ADS_BRAKE_THROTTLE_SCLK, ADS_BRAKE_THROTTLE_PDWN, ADS_BRAKE_THROTTLE_GAIN0, ADS_BRAKE_THROTTLE_GAIN1, ADS_BRAKE_THROTTLE_SPEED, ADS_BRAKE_THROTTLE_A0, ADS_BRAKE_THROTTLE_A1, GAIN128, FAST);
  //while ( !break_throttle_sensor.is_ready() ) {}       // Wait until the ADS is ready
  break_throttle_sensor.read(AIN1, value_long, true);  // Read a first value with a calibration process to remove spring compression offset
  break_throttle_sensor.read(AIN2, value_long, true);  // Read a first value with a calibration process to remove spring compression offset
}

void loop() {

  long raw_value, raw_brake, raw_throttle;

  //if ( keycode == 0 ) {
    keycode = u8g2.getMenuEvent();
    Serial.println(keycode);
  //}

  // if press the select menu,
  if ( keycode == U8X8_MSG_GPIO_MENU_HOME ) {
    u8g2.setFont(u8g2_font_courR10_tr);
    u8g2.clearBuffer();
    
    uint8_t current_selection = u8g2.userInterfaceSelectionList("Parameters", current_selection, parameters_list);
    if (current_selection==0) return;

    uint8_t v=50;
    u8g2.clearBuffer();
    u8g2.userInterfaceInputValue(  "yrdt", 
                                    "Percent= ", &v, 0, 100, 3, " %");

    drawBox();

    keycode = 0;
  }



  // read the throttle position on the loadcell
  break_throttle_sensor.read(AIN1, raw_value);          // value read are from +/-10mv on 40mV ADC range, so we read a 22 bits signal
  raw_throttle = ADC_FITLER(raw_value);                 // keep 15bits and use gain software
  raw_value = JOYSTICK_GAIN(raw_throttle, throttle_pct_range); 
  // set the throttle prosition on the joystick
  Joystick.setThrottle(raw_value);
  #if DEBUG
    Serial.print("Throttle : " + raw_throttle);
    Serial.print("(+" + throttle_pct_range);
    Serial.println("%) => " + raw_value);
  #endif

  // read the brake position on the loadcell
  break_throttle_sensor.read(AIN2, raw_value);          // value read are from +/-10mv on 40mV ADC range, so we read a 22 bits signal
  raw_brake = ADC_FITLER(raw_value);                    // keep 15bits and use gain software
  raw_value = JOYSTICK_GAIN(raw_brake, brake_pct_range); 
  // set the brake prosition on the joystick
  Joystick.setBrake(raw_value);
  #if DEBUG
    Serial.print("Brake : " + raw_brake);
    Serial.print("(+" + brake_pct_range);
    Serial.println("%) => " + raw_value);
  #endif

  //displayData((int16_t)raw_throttle, (int16_t)raw_brake, (int16_t)0);

  delay(10);
}

void drawBox() {
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, 0, 128, 16);
  u8g2.setFont(u8g2_font_courR08_tr);
  u8g2.setDrawColor(0);
  u8g2.drawStr(30, 11, "LEVEL STATUS");
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_courB14_tr); //ncenB18
  u8g2.drawStr(0, 32, "T");
  u8g2.drawFrame(20, 21, 56, 7);
  u8g2.drawStr(0, 48, "B");
  u8g2.drawFrame(20, 37, 56, 7);
  u8g2.drawStr(0, 64, "C");
  u8g2.drawFrame(20, 53, 56, 7);
  u8g2.sendBuffer();					        // transfer internal memory to the display
}

void displayData(int16_t throttle, int16_t brake, int16_t clutch) {
  u8g2.setDrawColor(0);
  u8g2.drawBox(22, 23, 50, 3);
  u8g2.drawBox(22, 39, 50, 3);
  u8g2.drawBox(22, 55, 50, 3);
  u8g2.setDrawColor(1);
  float thro = 50 * (throttle / (INT16_MAX * 1.0));
  float brak = 50 * (brake / (INT16_MAX * 1.0));
  float clut = 50 * (clutch / (INT16_MAX * 1.0));
  Serial.println(thro);
  Serial.println(brak);
  Serial.println(clut);
  u8g2.drawBox(22, 23, thro, 3);
  u8g2.drawBox(22, 39, brak, 3);
  u8g2.drawBox(22, 55, clut, 3);
  u8g2.sendBuffer();
}