// DIVA CONTROLLER by CyberKevin
// Firmware version: v514 - 02/09/2023

#include <QuickMpr121.h>
#include <FastLED.h>
#include "LUFAConfig.h"
#include <LUFA.h>
#include "Joystick.h"
#include <Bounce2.h>
#include <EEPROM.h>
#include "ButtonsMapping.h"
#include "Settings.h"
#include "SDVX.h"

#define BOUNCE_WITH_PROMPT_DETECTION
#define MILLIDEBOUNCE 1 //Debounce time in milliseconds

// Buttons values
byte buttonStatus[17] = {0};

// BOUNCE BUTTONS
Bounce joystickUP = Bounce();
Bounce joystickDOWN = Bounce();
Bounce joystickLEFT = Bounce();
Bounce joystickRIGHT = Bounce();
Bounce buttonA = Bounce();
Bounce buttonB = Bounce();
Bounce buttonX = Bounce();
Bounce buttonY = Bounce();
Bounce buttonLB = Bounce();
Bounce buttonRB = Bounce();
Bounce buttonSTART = Bounce();
Bounce buttonSELECT = Bounce();

// Buttons pins setup
void setupPins()
{
  //  You'll not have enough pins on a Pro Micro, please change the pins to the correspondig one.
  //  You can use Navgation mode of the Slider to have virtual buttons.
  joystickUP.attach(PIN_UP,INPUT_PULLUP);    // Unused
  joystickDOWN.attach(PIN_DOWN,INPUT_PULLUP);  // Unsued
  joystickLEFT.attach(PIN_LEFT,INPUT_PULLUP);
  joystickRIGHT.attach(PIN_RIGHT,INPUT_PULLUP);
  buttonA.attach(PIN_A,INPUT_PULLUP);
  buttonB.attach(PIN_B,INPUT_PULLUP);
  buttonX.attach(PIN_X,INPUT_PULLUP);
  buttonY.attach(PIN_Y,INPUT_PULLUP);
  buttonLB.attach(PIN_LB,INPUT_PULLUP);
  buttonRB.attach(PIN_RB,INPUT_PULLUP);
  buttonSTART.attach(PIN_START,INPUT_PULLUP);
  buttonSELECT.attach(PIN_SELECT,INPUT_PULLUP);

  joystickUP.interval(MILLIDEBOUNCE);
  joystickDOWN.interval(MILLIDEBOUNCE);
  joystickLEFT.interval(MILLIDEBOUNCE);
  joystickRIGHT.interval(MILLIDEBOUNCE);
  buttonA.interval(MILLIDEBOUNCE);
  buttonB.interval(MILLIDEBOUNCE);
  buttonX.interval(MILLIDEBOUNCE);
  buttonY.interval(MILLIDEBOUNCE);
  buttonLB.interval(MILLIDEBOUNCE);
  buttonRB.interval(MILLIDEBOUNCE);
  buttonSTART.interval(MILLIDEBOUNCE);
  buttonSELECT.interval(MILLIDEBOUNCE);
}

//Arduino Setup Process...
void setup()
{
  setupPins();                  // Setup Arduino Pins...
  SetupHardware();              // Setup Hardware...
  GlobalInterruptEnable();
}

// Arduino Loop process...
void loop() 
{
  buttonRead();
  processButtons();

  // output Default Stick values to not trigger false positive in game
  long resultBits;
  int32_t sliderBits = 0;
  resultBits = sliderBits ^ 0x80808080;
  // SENDING TO CONTROLLER THE RESULTED VALUES
  ReportData.RY = (resultBits >> 24) & 0xFF;
  ReportData.RX = (resultBits >> 16) & 0xFF;
  ReportData.LY = (resultBits >> 8) & 0xFF;
  ReportData.LX = (resultBits) & 0xFF;

  HID_Task();
  USB_USBTask();
}

void buttonRead()
{
    if (joystickUP.update()) {buttonStatus[BUTTONUP] = joystickUP.fell();}
    if (joystickDOWN.update()) {buttonStatus[BUTTONDOWN] = joystickDOWN.fell();}
    if (joystickLEFT.update()) {buttonStatus[BUTTONLEFT] = joystickLEFT.fell();}
    if (joystickRIGHT.update()) {buttonStatus[BUTTONRIGHT] = joystickRIGHT.fell();}
    if (buttonA.update()) {buttonStatus[BUTTONA] = buttonA.fell();}
    if (buttonB.update()) {buttonStatus[BUTTONB] = buttonB.fell();}
    if (buttonX.update()) {buttonStatus[BUTTONX] = buttonX.fell();}
    if (buttonY.update()) {buttonStatus[BUTTONY] = buttonY.fell();}
    if (buttonLB.update()) {buttonStatus[BUTTONLB] = buttonLB.fell();}
    if (buttonRB.update()) {buttonStatus[BUTTONRB] = buttonRB.fell();}
    if (buttonSTART.update()) {buttonStatus[BUTTONSTART] = buttonSTART.fell();}
    if (buttonSELECT.update()) {buttonStatus[BUTTONSELECT] = buttonSELECT.fell();}
}

void processButtons()
{  
  processDPAD();
  buttonProcessing();
}

void processDPAD()
{
  if ((buttonStatus[BUTTONUP]) && (buttonStatus[BUTTONRIGHT])){ReportData.HAT = DPAD_UPRIGHT_MASK_ON;}
  else if ((buttonStatus[BUTTONDOWN]) && (buttonStatus[BUTTONRIGHT])) {ReportData.HAT = DPAD_DOWNRIGHT_MASK_ON;} 
  else if ((buttonStatus[BUTTONDOWN]) && (buttonStatus[BUTTONLEFT])) {ReportData.HAT = DPAD_DOWNLEFT_MASK_ON;}
  else if ((buttonStatus[BUTTONUP]) && (buttonStatus[BUTTONLEFT])){ReportData.HAT = DPAD_UPLEFT_MASK_ON;}
  else if (buttonStatus[BUTTONUP]) {ReportData.HAT = DPAD_UP_MASK_ON;}
  else if (buttonStatus[BUTTONDOWN]) {ReportData.HAT = DPAD_DOWN_MASK_ON;}
  else if (buttonStatus[BUTTONLEFT]) {ReportData.HAT = DPAD_LEFT_MASK_ON;}
  else if (buttonStatus[BUTTONRIGHT]) {ReportData.HAT = DPAD_RIGHT_MASK_ON;}
  else {ReportData.HAT = DPAD_NOTHING_MASK_ON;}  
}

void buttonProcessing()
{
  if (buttonStatus[BUTTONA]) {ReportData.Button |= A_MASK_ON;}
  if (buttonStatus[BUTTONB]) {ReportData.Button |= B_MASK_ON;}
  if (buttonStatus[BUTTONX]) {ReportData.Button |= X_MASK_ON;}
  if (buttonStatus[BUTTONY]) {ReportData.Button |= Y_MASK_ON;}
  if (buttonStatus[BUTTONLB]) {ReportData.Button |= LB_MASK_ON; ReportData.Button |= ZL_MASK_ON;}
  if (buttonStatus[BUTTONRB]) {ReportData.Button |= RB_MASK_ON; ReportData.Button |= ZR_MASK_ON;}
  if (buttonStatus[BUTTONSTART]) {ReportData.Button |= START_MASK_ON;}
  if (buttonStatus[BUTTONSELECT]) {ReportData.Button |= SELECT_MASK_ON;}
}

extern "C"
{
  byte ReadEEPROM(int i)
  { 
    return EEPROM.read(i);
  }

  void WriteEEPROM(int i, byte value)
  {
    EEPROM.update(i, value);
  }

  void PDAC_PC_RELOAD()
  { // Reload settings
  }

  void PDAC_PC_CALIBRATE()
  { // Calibrate slider with new settings
  }

  void PDAC_PC_COLORTEST_CHANGECOLOR(int r, int g, int b)
  {
  }

  void PDAC_PC_TRAILTEST_RESETLINE()
  {
  }

  void PDAC_PC_TRAILTEST_CHANGECOLOR(int r, int g, int b)
  {
  }

  void PDAC_PC_NOTOUCHCOLORTEST(int r, int g, int b)
  {
  }

  void PDAC_PC_TOUCHCOLORTEST(int r, int g, int b)
  {
  }

  void PDAC_PC_SERVICE()
  {
  }

  byte PDAC_PC_DEBUG(int i)
  {
  }

  byte PDAC_PC_SENSORSDEBUG_DATA(int sensor)
  {
  }

  byte PDAC_PC_SENSORSDEBUG_BASELINE(int sensor)
  {
  }
} // extern "C"
