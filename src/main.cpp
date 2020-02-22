#include <main.h>

// check if a is in [0-100], else return b
#define CHECK_RANGE(value, defval)  ( ( ((value) < 0) || ((value) > 100) ) ? (defval): (value) )
// filter ADC signal and apply the apps gain
// at 80SPS, with 44nV imprecision (17bit/24) => result is on 15bits, ok for the int16
#define ADC_FITLER(value)  (value >> 7)
// Apply the gain and max the output if overload
#define JOYSTICK_GAIN(value, gain) (((value) * 100)/(gain)  > INT16_MAX) ? INT16_MAX : ((value) * 100)/(gain)
#define JOYSTICK_DEADZONE(value, deadzone) ( ((value) / 32767.0) < ((deadzone) * 0.01) ) ? 0 : (value)

// prototype
void drawBox();
void drawWelcome();
void drawSaving();
void displayData(int16_t throttle, int16_t brake, int16_t clutch);

void loadDataEEPROM() {
  throttle_pct_range_addr = EEPROM.getAddress(sizeof(byte)); // read in eeprom the throttle param
  throttle_pct_range    = EEPROM.readByte(throttle_pct_range_addr);
  throttle_pct_deadzone_addr = EEPROM.getAddress(sizeof(byte));
  throttle_pct_deadzone = EEPROM.readByte(throttle_pct_deadzone_addr);
  brake_pct_range_addr  = EEPROM.getAddress(sizeof(byte)); // read in eeprom the break param
  brake_pct_range       = EEPROM.readByte(brake_pct_range_addr);
  brake_pct_deadzone_addr = EEPROM.getAddress(sizeof(byte));
  brake_pct_deadzone    = EEPROM.readByte(brake_pct_deadzone_addr);
  clutch_pct_range_addr = EEPROM.getAddress(sizeof(byte)); // read in eeprom the clutch param
  clutch_pct_range      = EEPROM.readByte(clutch_pct_range_addr);
  clutch_pct_deadzone_addr = EEPROM.getAddress(sizeof(byte));
  clutch_pct_deadzone   = EEPROM.readByte(clutch_pct_deadzone_addr);

  throttle_pct_range    = CHECK_RANGE(throttle_pct_range, 100);
  brake_pct_range       = CHECK_RANGE(brake_pct_range, 100);
  clutch_pct_range      = CHECK_RANGE(clutch_pct_range, 100);
  throttle_pct_deadzone = CHECK_RANGE(throttle_pct_deadzone, 2);
  brake_pct_deadzone    = CHECK_RANGE(brake_pct_deadzone, 2);
  clutch_pct_deadzone   = CHECK_RANGE(clutch_pct_deadzone, 2);
}

void saveEEPROM () {
  EEPROM.writeByte(throttle_pct_range_addr, throttle_pct_range);
  while (!EEPROM.isReady()) delay(1);
  EEPROM.writeByte(throttle_pct_deadzone_addr, throttle_pct_deadzone);
  while (!EEPROM.isReady()) delay(1);
  EEPROM.writeByte(brake_pct_range_addr, brake_pct_range);
  while (!EEPROM.isReady()) delay(1);
  EEPROM.writeByte(brake_pct_deadzone_addr, brake_pct_deadzone);
  while (!EEPROM.isReady()) delay(1);
  EEPROM.writeByte(clutch_pct_range_addr, clutch_pct_range);
  while (!EEPROM.isReady()) delay(1);
  EEPROM.writeByte(clutch_pct_deadzone_addr, clutch_pct_deadzone);
  while (!EEPROM.isReady()) delay(1);
}

void setup() {
  // Initialize Joystick Library
  #if !DEBUG
    // Set the range throttle
    Joystick.setXAxisRange (0, INT16_MAX);
    Joystick.setYAxisRange (0, INT16_MAX);
    Joystick.setZAxisRange (0, INT16_MAX);
    Joystick.begin();
  #else
    delay(500);
    Serial.begin(9600);
    delay(500);
    Serial.println("test");
  #endif

  // Init the eeprom data
  EEPROM.setMemPool(memBase, EEPROMSizeMicro);
  EEPROM.setMaxAllowedWrites(maxAllowedWrites);
  loadDataEEPROM();

  // Init screen
  u8g2.begin(/*Select=*/ 14, /*Right/Next=*/ A1, /*Left/Prev=*/ A2, /*Up=*/ A0, /*Down=*/ A3, /*Home/Cancel=*/ 15);
  drawWelcome();
  
  // Init the ADC
  break_sensor.begin(ADS_BRAKE_THROTTLE_DOUT, ADS_BRAKE_THROTTLE_SCLK, ADS_BRAKE_THROTTLE_PDWN, ADS_BRAKE_THROTTLE_GAIN0, ADS_BRAKE_THROTTLE_GAIN1, ADS_BRAKE_THROTTLE_SPEED, ADS_BRAKE_THROTTLE_A0, ADS_BRAKE_THROTTLE_A1, GAIN128, FAST);
  while ( !break_sensor.is_ready() ) {}       // Wait until the ADS is ready
  Serial.println("end wait init");
  delay(1000);
  break_sensor.tare(AIN1, 20, true);

  drawBox();
}

void loop() {

  long raw_value, raw_brake, raw_throttle;

  keycode = u8g2.getMenuEvent();
  //Serial.println(keycode);

  // if press the select menu,
  if ( keycode == U8X8_MSG_GPIO_MENU_HOME ) {

    keycode = 0;

    u8g2.setFont(u8g2_font_courR10_tr);
    u8g2.clearBuffer();
    
    uint8_t current_selection = u8g2.userInterfaceSelectionList("Parameters", current_selection, parameters_list);
    if (current_selection != 0) {

      u8g2.clearBuffer();

      char title[20];
      title[0] = '\n'; 
      u8x8_CopyStringLine(title+1, current_selection-1, parameters_list );

      byte *v;
      switch (current_selection-1)
      {
      case 0:
        v = &throttle_pct_range;
        break;
      case 1:
        v = &throttle_pct_deadzone;
        break;
      case 2:
        v = &brake_pct_range;
        break;
      case 3:
        v = &brake_pct_deadzone;
        break;
      case 4:
        v = &clutch_pct_range;
        break;
      case 5:
        v = &clutch_pct_deadzone;
        break;
      
      default:
        break;
      } 

      int input = u8g2.userInterfaceInputValue( title, "Percent= ", v, 0, 200, 3, " %");
      if (input==1) {
        drawSaving();
        saveEEPROM();
        delay(2000);
      }
    }
    
    drawBox();
  }



  // read the throttle position on the loadcell
  /*
  break_throttle_sensor.read(AIN1, raw_value);          // value read are from +/-10mv on 40mV ADC range, so we read a 22 bits signal
  raw_value -= long(break_throttle_sensor.get_offset(AIN1));
  raw_throttle = ADC_FITLER(raw_value);                 // keep 15bits and use gain software
  raw_throttle = JOYSTICK_GAIN(raw_throttle, throttle_pct_range); 
  raw_throttle = JOYSTICK_DEADZONE(raw_throttle, throttle_pct_deadzone);
  // set the throttle prosition on the joystick
  Joystick.setXAxis(raw_throttle);
  #if DEBUG
    Serial.print("Throttle : " + String(raw_brake));
    Serial.print("(+" + String(throttle_pct_range));
    Serial.print("%|" + String(throttle_pct_deadzone));
    Serial.println("%) <= " + String(raw_value) + "|" + String(break_throttle_sensor.get_offset(AIN1)));
  #endif*/

  // read the brake position on the loadcell
  break_sensor.read(AIN1, raw_value);          // value read are from +/-10mv on 40mV ADC range, so we read a 22 bits signal
  raw_value -= long(break_sensor.get_offset(AIN1));
  raw_brake = ADC_FITLER(raw_value);                    // keep 15bits and use gain software
  raw_brake = JOYSTICK_GAIN(raw_brake, brake_pct_range); 
  raw_brake = JOYSTICK_DEADZONE(raw_brake, brake_pct_deadzone);
  // set the brake prosition on the joystick
  Joystick.setYAxis(raw_brake);
  #if DEBUG
    Serial.print("Brake : " + String(raw_brake));
    Serial.print("(+" + String(brake_pct_range));
    Serial.print("%|" + String(brake_pct_deadzone));
    Serial.println("%) <= " + String(raw_value) + "|" + String(break_sensor.get_offset(AIN1)));
  #endif

  displayData((int16_t)raw_throttle, (int16_t)raw_brake, (int16_t)0);
}

void drawWelcome() {
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, 0, 128, 16);
  u8g2.setFont(u8g2_font_courR08_tr);
  u8g2.setDrawColor(0);
  u8g2.drawStr(30, 11, "WELCOME !");
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_courB14_tr); //ncenB18
  u8g2.drawStr(0, 40, "CALIBRATION");
  u8g2.sendBuffer();					        // transfer internal memory to the display
}

void drawSaving() {
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, 0, 128, 16);
  u8g2.setFont(u8g2_font_courR08_tr);
  u8g2.setDrawColor(0);
  u8g2.drawStr(30, 11, "Preferences");
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_courB14_tr); //ncenB18
  u8g2.drawStr(0, 40, "SAVING...");
  u8g2.sendBuffer();					        // transfer internal memory to the display
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
  u8g2.drawFrame(20, 21, 106, 7);
  u8g2.drawStr(0, 48, "B");
  u8g2.drawFrame(20, 37, 106, 7);
  u8g2.drawStr(0, 64, "C");
  u8g2.drawFrame(20, 53, 106, 7);
  u8g2.sendBuffer();					        // transfer internal memory to the display
}

void displayData(int16_t throttle, int16_t brake, int16_t clutch) {
  u8g2.setDrawColor(0);
  u8g2.drawBox(22, 23, 100, 3);
  u8g2.drawBox(22, 39, 100, 3);
  u8g2.drawBox(22, 55, 100, 3);
  u8g2.setDrawColor(1);
  float thro = 100 * (throttle / (INT16_MAX * 1.0));
  float brak = 100 * (brake / (INT16_MAX * 1.0));
  float clut = 100 * (clutch / (INT16_MAX * 1.0));
  u8g2.drawBox(22, 23, thro, 3);
  u8g2.drawBox(22, 39, brak, 3);
  u8g2.drawBox(22, 55, clut, 3);
  u8g2.sendBuffer();
}