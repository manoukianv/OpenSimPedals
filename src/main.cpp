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
**/

#include <main.h>

// prototype
void saveEEPROM ();
void loadDataEEPROM();
void drawBox();
void displayData(int16_t throttle, int16_t brake, int16_t clutch);
void drawScreen(const char *title, int titleX, int titleY, 
                const char* message, int messageX, int messageY,
                const char* subMessage = nullptr, int subMessageX = 0, int subMessageY = 0);
void checkMenu();
long readPedal(ADS123X& sensor, byte& range, byte& deadZone, byte idLog);

/********************************  Setup & Main *******************************/

void setup() {
  // Set the range throttle
  Joystick.setXAxisRange (0, INT16_MAX);
  Joystick.setYAxisRange (0, INT16_MAX);
  Joystick.setZAxisRange (0, INT16_MAX);
  #if !DEBUG
    // Initialize Joystick Library
    Joystick.begin(false);
  #else
    // Initialize Serial
    delay(1000);
    Serial.begin(9600);
    delay(1000);
    Serial.println("start");
  #endif

  // Init the eeprom data
  EEPROM.setMemPool(memBase, EEPROMSizeMicro);
  EEPROM.setMaxAllowedWrites(maxAllowedWrites);
  loadDataEEPROM();

  // Init screen
  u8g2.begin(A3, A2, A1);
  drawScreen("WELCOME !", 40, 11, "OpenSimPedals", 0, 40, RELEASE, 50, 53);
  delay(5000);     // Add a welcome screen to display release and wait for a prog connection

  // Init & calibrate the ADC
  drawScreen("WELCOME !", 40, 11, "CALIBRATING", 10, 45);
#if THROTTLE
  Serial.println("THROTTLE");
  throttle_sensor.begin(ADS_THROTTLE_DOUT, ADS_THROTTLE_SCLK, ADS_THROTTLE_PDWN);
  while ( !throttle_sensor.is_ready() ) { delay(1); }
  delay(300);
  throttle_sensor.tare(AIN1, 20, true);
#endif
#if BRAKE
  Serial.println("BRAKE");
  brake_sensor.begin(ADS_BRAKE_DOUT, ADS_BRAKE_SCLK, ADS_BRAKE_PDWN);
  while ( !brake_sensor.is_ready()    ) { delay(1); }
  delay(300);
  brake_sensor.tare(AIN1, 20, true);
#endif
#if CLUTCH
  Serial.println("CLUTCH");
  clutch_sensor.begin(ADS_CLUTCH_DOUT, ADS_CLUTCH_SCLK, ADS_CLUTCH_PDWN);
  while ( !clutch_sensor.is_ready()   ) { delay(1); }
  delay(300);
  clutch_sensor.tare(AIN1, 20, true);
#endif

  drawScreen("RUNNING !", 40, 11, "+- to monitor", 0, 40, "Sel to setup", 25, 53);

}

long old_time = millis();
long time_click = LONG_MIN;
long count = 0;

void loop() {

  // Check if it's time to display menu
  checkMenu();

  // read the load cell value
  long throttle = 0, brake = 0, clutch = 0;
#if THROTTLE
  throttle = readPedal(throttle_sensor, brake_pct_range, brake_pct_deadzone, 1);
  Joystick.setXAxis(throttle);
#endif
#if BRAKE
  brake    = readPedal(brake_sensor, brake_pct_range, brake_pct_deadzone, 2);
  Joystick.setYAxis(brake);
#endif
#if CLUTCH
  clutch   = readPedal(clutch_sensor, brake_pct_range, brake_pct_deadzone, 3);
  Joystick.setZAxis(throttle);
#endif
#if !DEBUG
  Joystick.sendState();
#endif

  // Update screen only each second : priority to trame / s on joystick
  count++;
  if ((millis() - old_time) > 10000 ) {
    #if DEBUG
      Serial.println("Nb sample / 10s :" + String(count));
    #endif
    old_time = millis();
    count = 0;
  }
  if ((millis() - time_click) < 30000 ) {
    displayData((int16_t)throttle, (int16_t)brake, (int16_t)clutch);
  } else if ( time_click == LONG_MIN) {
    return;
  } else {
    time_click = LONG_MIN;
    drawScreen("RUNNING !", 40, 11, "+- to monitor", 0, 40, "Sel to setup", 25, 53);
  }
  
}

/********************************    EEPROM     ********************************/
void loadDataEEPROM() {
  throttle_pct_range_addr     = EEPROM.getAddress(sizeof(byte)); // read in eeprom the throttle param
  throttle_pct_range          = EEPROM.readByte(throttle_pct_range_addr);
  throttle_pct_deadzone_addr  = EEPROM.getAddress(sizeof(byte));
  throttle_pct_deadzone       = EEPROM.readByte(throttle_pct_deadzone_addr);
  brake_pct_range_addr        = EEPROM.getAddress(sizeof(byte)); // read in eeprom the break param
  brake_pct_range             = EEPROM.readByte(brake_pct_range_addr);
  brake_pct_deadzone_addr     = EEPROM.getAddress(sizeof(byte));
  brake_pct_deadzone          = EEPROM.readByte(brake_pct_deadzone_addr);
  clutch_pct_range_addr       = EEPROM.getAddress(sizeof(byte)); // read in eeprom the clutch param
  clutch_pct_range            = EEPROM.readByte(clutch_pct_range_addr);
  clutch_pct_deadzone_addr    = EEPROM.getAddress(sizeof(byte));
  clutch_pct_deadzone         = EEPROM.readByte(clutch_pct_deadzone_addr);

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

/********************************* Read Sendor *********************************/

long readPedal(ADS123X& sensor, byte& range, byte& deadZone, byte idLog) {
  // read the brake position on the loadcell
  long raw_value, result;
  sensor.read(AIN1, raw_value);                   // value read are from +/-10mv on 40mV ADC range, so we read a 22 bits signal
  raw_value -= long(sensor.get_offset(AIN1));
  result = ADC_FITLER(raw_value);                 // keep 15bits and use gain software
  result = JOYSTICK_GAIN(result, range); 
  result = JOYSTICK_DEADZONE(result, deadZone);

  // set the brake prosition on the joystick
  #if DEBUG
    char str[30];
    sprintf(str, "%d] %ld(+%d-%d) <= %ld-%.2f)", idLog, result, range, deadZone, raw_value, trunc(sensor.get_offset(AIN1)));
    Serial.println(str);
  #endif

  return result;
}

/******************************** SCREEN & MENU ********************************/
void checkMenu() {
  keycode = u8g2.getMenuEvent();

  if ( keycode == U8X8_MSG_GPIO_MENU_SELECT ) {

    keycode = 0;
    u8g2.setFont(u8g2_font_courR10_tr);
    u8g2.clearBuffer();
    
    uint8_t current_selection = u8g2.userInterfaceSelectionList("Preferences", 0, parameters_list);
    if (current_selection != 0) {

      u8g2.clearBuffer();

      char title[20];
      title[0] = '\n'; // Add a empty line in the first yellow line
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

      int input = u8g2.userInterfaceInputValue(title, "Percent= ", v, 0, 200, 3, " %");
      if (input == 1) {
        drawScreen("Preferences", 30, 11, "SAVING", 35, 45);
        saveEEPROM();
        delay(2000);
      }
    }
    
    drawScreen("RUNNING !", 40, 11, "+- to monitor", 0, 40, "Sel to setup", 25, 53);
  } else if ( keycode == U8X8_MSG_GPIO_MENU_NEXT || keycode == U8X8_MSG_GPIO_MENU_PREV) {
    time_click = millis();
    drawBox();
  }


}

void drawScreen(const char *title, int titleX, int titleY, 
                const char* message, int messageX, int messageY,
                const char* subMessage, int subMessageX, int subMessageY) {
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, 0, 128, 16);
  u8g2.setFont(u8g2_font_courR08_tr);
  u8g2.setDrawColor(0);
  u8g2.drawStr(titleX, titleY, title);
  u8g2.setDrawColor(1);
  if (subMessage != nullptr) {
    u8g2.drawStr(subMessageX, subMessageY, subMessage);
  }
  u8g2.setFont(u8g2_font_courB12_tr); //ncenB18
  u8g2.drawStr(messageX, messageY, message);
  u8g2.sendBuffer();					        // transfer internal memory to the display
}

void drawBox() {
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, 0, 128, 16);
  u8g2.setFont(u8g2_font_courR08_tr);
  u8g2.setDrawColor(0);
  u8g2.drawStr(27, 11, "LEVEL STATUS");
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_courB12_tr);
  u8g2.drawStr(0, 32, "T");
  u8g2.drawFrame(20, 23, 106, 9);
  u8g2.drawStr(0, 48, "B");
  u8g2.drawFrame(20, 39, 106, 9);
  u8g2.drawStr(0, 64, "C");
  u8g2.drawFrame(20, 55, 106, 9);
  u8g2.sendBuffer();
}

void displayData(int16_t throttle, int16_t brake, int16_t clutch) {
  u8g2.setDrawColor(0);
  u8g2.drawBox(22, 25, 100, 5);
  u8g2.drawBox(22, 41, 100, 5);
  u8g2.drawBox(22, 57, 100, 5);
  u8g2.setDrawColor(1);
  float thro = 100 * (throttle / (INT16_MAX * 1.0));
  float brak = 100 * (brake / (INT16_MAX * 1.0));
  float clut = 100 * (clutch / (INT16_MAX * 1.0));
  u8g2.drawBox(22, 25, thro, 5);
  u8g2.drawBox(22, 41, brak, 5);
  u8g2.drawBox(22, 57, clut, 5);
  u8g2.sendBuffer();
}