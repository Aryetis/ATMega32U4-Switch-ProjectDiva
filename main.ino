#include <QuickMpr121.h>
#include <FastLED.h>
#include "LUFAConfig.h"
#include <LUFA.h>
#include "Joystick.h"
#define BOUNCE_WITH_PROMPT_DETECTION
#include <Bounce2.h>

#define MILLIDEBOUNCE 1 //Debounce time in milliseconds

//Debug Timer

unsigned long previousMillis = 0;
unsigned long interval = 30;
bool right = true;
int currentSlide1 = 0;
int currentSlide2 = 0;

bool buttonStartBefore;
bool buttonSelectBefore;
byte buttonStatus[16] = {0};
int buttonSwitchModeTimer = 100;

/*
  0x4000,
  0x8000,*/
#define CAPTURE_MASK_ON 0x2000
#define R3_MASK_ON 0x800
#define L3_MASK_ON 0x400
 
#define DPAD_UP_MASK_ON 0x00
#define DPAD_UPRIGHT_MASK_ON 0x01
#define DPAD_RIGHT_MASK_ON 0x02
#define DPAD_DOWNRIGHT_MASK_ON 0x03
#define DPAD_DOWN_MASK_ON 0x04
#define DPAD_DOWNLEFT_MASK_ON 0x05
#define DPAD_LEFT_MASK_ON 0x06
#define DPAD_UPLEFT_MASK_ON 0x07
#define DPAD_NOTHING_MASK_ON 0x08
#define A_MASK_ON 0x04
#define B_MASK_ON 0x02
#define X_MASK_ON 0x08
#define Y_MASK_ON 0x01
#define LB_MASK_ON 0x10
#define RB_MASK_ON 0x20
#define ZL_MASK_ON 0x40
#define ZR_MASK_ON 0x80
#define START_MASK_ON 0x200
#define SELECT_MASK_ON 0x100
#define HOME_MASK_ON 0x1000

#define BUTTONUP 0
#define BUTTONDOWN 1
#define BUTTONLEFT 2
#define BUTTONRIGHT 3
#define BUTTONA 4
#define BUTTONB 5
#define BUTTONX 7
#define BUTTONY 6
#define BUTTONLB 8
#define BUTTONRB 9
#define BUTTONLT 10
#define BUTTONRT 11
#define BUTTONSTART 12
#define BUTTONSELECT 13
#define BUTTONHOME 14
#define SWITCHMODEPIN 15

bool PS4 = false;

//Slider
#define NUM_MPRS 3
#define SLIDER_DEBUG true
#define ANALOG_OUTPUT true
#define PROXIMITY_ENABLE false
#define NUM_SENSORS 36
#define MPR_THRESHOLD_TOUCH 12  // Sensitivity (default 15 - 10) (Optimised 10 - 9)  (perfect 9, 6)
#define MPR_THRESHOLD_RELEASE 10
short sensors[36];
short sensorsConfirmed[36];
short sensorsConfirmed2[36];
bool sensorsTouched[36];
bool sensorTouched;

// create the mpr121 instances
// these will have addresses set automatically
mpr121 mprs[NUM_MPRS];
bool calibrated = false;



typedef enum {
  GAMEPLAY,
  NAVIGATION,
  CHUNITHM,
  ARCADE,
  MENU,
  CALIBRATE
} SliderMode;
SliderMode sliderMode = GAMEPLAY;
SliderMode sliderModeChange = GAMEPLAY;

// Slider Key mapping
int sensorsSwapPS4[3] {0, 1, 2};
int sensorsArcade[4] {6, 7, 8, 9};
int sensorsDemo[4] {13, 14, 15, 16};
int sensorsNav[4] {20, 21, 22, 23};
int sensorsGame[5] {27, 28, 29, 30, 31};



// LEDs
#define NUM_LEDS_PER_STRIP 32
#define LED_TYPE WS2812B
#define LED_DATA_PIN 21
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS_PER_STRIP];
//bool idleLeds = true;


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
Bounce buttonLT = Bounce();
Bounce buttonRT = Bounce();
Bounce buttonSTART = Bounce();
Bounce buttonSELECT = Bounce();
Bounce buttonHOME = Bounce();
Bounce switchModePin = Bounce();

typedef enum {
  ANALOG_MODE,
  DIGITAL,
  SMASH
} State_t;
State_t state = DIGITAL;

void checkModeChange(){
   if (buttonStatus[BUTTONSTART] && buttonStatus[BUTTONSELECT]){
    if(buttonStartBefore == 0 && buttonSelectBefore ==0){
        switch(state)
        {
           case DIGITAL:
              state=ANALOG_MODE;
           break;

           case ANALOG_MODE:
              state=SMASH;
           break;

           case SMASH:
              state=DIGITAL;
           break;
        }
        buttonStartBefore = 1;
        buttonSelectBefore = 1;
    }
  }
  else {buttonSelectBefore = 0;buttonStartBefore = 0;}
}
void setupPins(){
  //  You'll not have enough pins on a Pro Micro, please change the pins to the correspondig one.
  //  You can use Navgation mode of the Slider to have virtual buttons.
    joystickUP.attach(255,INPUT_PULLUP);
    joystickDOWN.attach(255,INPUT_PULLUP);
    joystickLEFT.attach(1,INPUT_PULLUP);
    joystickRIGHT.attach(0,INPUT_PULLUP);
    buttonA.attach(4,INPUT_PULLUP);
    buttonB.attach(5,INPUT_PULLUP);
    buttonX.attach(7,INPUT_PULLUP);
    buttonY.attach(6,INPUT_PULLUP);
    buttonLB.attach(9,INPUT_PULLUP);
    buttonRB.attach(8,INPUT_PULLUP);
    buttonLT.attach(16,INPUT_PULLUP);
    buttonRT.attach(10,INPUT_PULLUP);
    buttonSTART.attach(15,INPUT_PULLUP);
    buttonSELECT.attach(14,INPUT_PULLUP);
    buttonHOME.attach(18,INPUT_PULLUP);
    switchModePin.attach(19, INPUT_PULLUP);
  
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
    buttonLT.interval(MILLIDEBOUNCE);
    buttonRT.interval(MILLIDEBOUNCE);
    buttonSTART.interval(MILLIDEBOUNCE);
    buttonSELECT.interval(MILLIDEBOUNCE);
    buttonHOME.interval(MILLIDEBOUNCE);
    switchModePin.interval(MILLIDEBOUNCE);
    
}

void sensorsInitialization(){
  
    for (int i = 0; i < NUM_MPRS; i++) {
    // this special line makes `mpr` the same as typing `mprs[i]`
    mpr121 &mpr = mprs[i];

    
    // `mpr.begin()` sets up the Wire library
    // mpr121 can run in 400kHz mode; if you have issues with it or want 100kHz speed, use `mpr121.begin(100000)`
    // (or use `Wire.begin` and/or `Wire.setClock` directly instead of this)
    mpr.begin();

    // set autoconfig charge level based on 3.2V
    // without this, it will assume 1.8V (a safe default, but not always ideal)
    mpr.autoConfigUSL = 256L * (3200 - 700) / 3200;

    // enable proximity sensing if it's set to true
    if (PROXIMITY_ENABLE)
      mpr.proxEnable = MPR_ELEPROX_0_TO_11;
    else
      mpr.proxEnable = MPR_ELEPROX_DISABLED;

      // Adjusting...
      mpr.FFI = MPR_FFI_10;
      mpr.SFI = MPR_SFI_6;

      // Sensitivity (default 15 - 10) (Optimised 10 - 9)  (perfect 9, 6)
      mpr.setAllThresholds(MPR_THRESHOLD_TOUCH, MPR_THRESHOLD_RELEASE, false);

    // start sensing
    mpr.start(12);

  
  }
}

void calibrateSensors(){
    leds[0] = CRGB::White;
    leds[1] = CRGB::White;
    leds[2] = CRGB::White;
    calibrated = false;
    for (int i = 0; i < NUM_MPRS; i++) {
      mpr121 &mpr = mprs[i];
      mpr.softReset();
    }
    sliderModeChange = GAMEPLAY;
    sensorsInitialization();
  }

void ledsInitialization(){
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS_PER_STRIP);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 400);
  }

void setup() {
  sensorsInitialization();
  buttonStartBefore = false;
  buttonSelectBefore = false;
  setupPins();
  ledsInitialization();
  SetupHardware();
  GlobalInterruptEnable();
  
}


void loop() {
    buttonRead();
    checkSensors();
    if (switchModePin.fell())
      sliderModeChange = MENU;
      
      bool swapmode = true;
      if(sliderMode != sliderModeChange){
        for (bool sensor : sensors){
          if(sensor && swapmode)  swapmode = false;
          }
      }
        
        if(swapmode) sliderMode = sliderModeChange;
        
    switch(sliderMode)
    {
      case GAMEPLAY: // Slider act like the HORI controller's Slider with Dedicated Controller Mode (For Mega39's / MegaMix / Future Tone (International version not compatible)
      sliderGameplay();
      break;
      case NAVIGATION: // Slider act like a controller with navigations buttons if you lack of buttons on your controller (customize the code below)
      sliderNavigation();
      break;
      case CHUNITHM: // Slider CHUNITHM 8 KEYS
      sliderGameplay();
      break;
      case ARCADE: // Slider act like a Keyboard that you can hook up to Project Diva Arcade Future Tone (TO DO)
      sliderMenu();
      break;
      case MENU: // Slider is in the menu selection mode
      sliderMenu();
      break;
      case CALIBRATE: // Calibrate slider
      calibrateSensors();
      break;
    }
    //checkModeChange();
    processButtons();
    HID_Task();
    // We also need to run the main USB management task.
    USB_USBTask();
    FastLED.show();

    
}



void checkSensors(){
  int numElectrodes = 12;
  int sensorCount = 0;
  int touchedSensors = 0;
  
  for (int i = 0; i < NUM_MPRS; i++) {
    mpr121 &mpr = mprs[i];
    for (int j = 0; j < numElectrodes; j++) {
      short touching = mpr.readTouchState(j);
      //sensors[sensorCount] = touching;
      if(touching){
        if(sensorsConfirmed[sensorCount] && sensorsConfirmed2[sensorCount])
          sensors[sensorCount] = touching;
        else if(sensorsConfirmed[sensorCount])
          sensorsConfirmed2[sensorCount] = touching;
        else
          sensorsConfirmed[sensorCount] = touching;
      }
      else{
        sensors[sensorCount] = touching;
        sensorsConfirmed[sensorCount] = touching;
        sensorsConfirmed2[sensorCount] = touching;
        }
        
      //if(touching)
        //touchedSensors++;
      sensorCount++;
    }
    if(!calibrated){
      short value = mpr.readElectrodeData(0); 
      byte baseline = mpr.readElectrodeBaseline(0);
      value -= ((short)baseline << 2);
      if(value > 0 && value < 5) calibrated = true;
      }
  }

  // Check if at least one sensor is touched
  if(touchedSensors > 0){
    sensorTouched = true;
    }
    
}

bool sensorJustTouched(int x){
  if(sensors[x] && !sensorsTouched[x])
    {
    sensorsTouched[x] = true;  
    return true;
    }
    return false;
  }


void sliderMenu(){
   // Default Stick
  long resultBits;
  int32_t sliderBits = 0;
  resultBits = sliderBits ^ 0x80808080;
  // SENDING TO CONTROLLER THE RESULTED VALUES
  ReportData.RY = (resultBits >> 24) & 0xFF;
  ReportData.RX = (resultBits >> 16) & 0xFF;
  ReportData.LY = (resultBits >> 8) & 0xFF;
  ReportData.LX = (resultBits) & 0xFF;
   
    //LOGIC
  if (!buttonStatus[SWITCHMODEPIN])
  {    
    if(sensors[sensorsArcade[0]] || sensors[sensorsArcade[1]] || sensors[sensorsArcade[2]] || sensors[sensorsArcade[3]])
      sliderModeChange = ARCADE;
      else if(sensors[sensorsDemo[0]] || sensors[sensorsDemo[1]] || sensors[sensorsDemo[2]] || sensors[sensorsDemo[3]])
      sliderModeChange = CHUNITHM;
      else if(sensors[sensorsNav[0]] || sensors[sensorsNav[1]] || sensors[sensorsNav[2]] || sensors[sensorsNav[3]])
      sliderModeChange = NAVIGATION;
      else if(sensors[sensorsGame[0]] || sensors[sensorsGame[1]] || sensors[sensorsGame[2]] || sensors[sensorsGame[3]] || sensors[sensorsGame[4]])
      sliderModeChange = GAMEPLAY;
      else if(sensors[sensorsSwapPS4[0]] || sensors[sensorsSwapPS4[1]] || sensors[sensorsSwapPS4[2]])
      sliderModeChange = CALIBRATE;
    
  }
   //LEDS
for (CRGB &led : leds){
        led = CRGB::Black;
        }
   
    if(!PS4)
      leds[0] = leds[1] = leds[2] = CRGB::DarkRed;
    else
      leds[0] = leds[1] = leds[2] = CRGB::Aqua;
      
    leds[sensorsArcade[0]] = leds[sensorsArcade[1]] = leds[sensorsArcade[2]] = leds[sensorsArcade[3]] = CRGB(0, 255, 153);
    leds[sensorsDemo[0]] = leds[sensorsDemo[1]] = leds[sensorsDemo[2]] = leds[sensorsDemo[3]] = CRGB(255, 0, 242);
    leds[sensorsNav[0]] = leds[sensorsNav[1]] = leds[sensorsNav[2]] = leds[sensorsNav[3]] = CRGB(0, 128, 255);
    leds[sensorsGame[0]] = leds[sensorsGame[1]] = leds[sensorsGame[2]] = leds[sensorsGame[3]] = leds[sensorsGame[4]] = CRGB::Red;
}


void sliderGameplay(){
  long resultBits;
  long noTouchBits;
  int bit_count = 31; // Number of sensors for your slider

  int32_t sliderBits = 0;
  noTouchBits = sliderBits ^ 0x80808080;
  for (bool sensor : sensors)
  {
    if(bit_count >= 0){
    // Check current Sensor state here
    //if((sensor & 0x8000) != 0)
    if(sensor)
      sliderBits |= (1l << bit_count);

    bit_count -= 1;
    }
  }

  resultBits = sliderBits ^ 0x80808080;

  // GAMEPLAY
  // SENDING TO CONTROLLER THE RESULTED VALUES
  if(sliderMode == GAMEPLAY){
    ReportData.RY = (resultBits >> 24) & 0xFF;
    ReportData.RX = (resultBits >> 16) & 0xFF;
    ReportData.LY = (resultBits >> 8) & 0xFF;
    ReportData.LX = (resultBits) & 0xFF;

    // LEDS
    
  //if(!sensorTouched)
  if(!calibrated){
    bool lightUp = true;
      for (CRGB &led : leds){
        if(lightUp){
          led = CRGB::White;
          lightUp = false;
          }
          else{
            led = CRGB::Black;
            lightUp = true;
            }
            //led = CRGB::White;
        }
  /*}else if(resultBits == noTouchBits){
    bool lightUp = true;
      for (CRGB &led : leds){
        if(lightUp){
          led = CRGB::White;
          lightUp = false;
          }
          else{
            lightUp = true;
            }
        }*/
    }else{
    for (CRGB &led : leds){
        led = CRGB::Black;
        }
    for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
      if(sensors[i]){
        leds[i] = CRGB::White;
        }
      }
    }
  }
  else if(sliderMode == CHUNITHM){

    // KEYS
    if(sensors[0] || sensors[1] || sensors[2] || sensors[3]) buttonStatus[BUTTONA] = true; else buttonStatus[BUTTONA] = false;
    if(sensors[4] || sensors[5] || sensors[6] || sensors[7]) buttonStatus[BUTTONB] = true; else buttonStatus[BUTTONB] = false;
    if(sensors[8] || sensors[9] || sensors[10] || sensors[11]) buttonStatus[BUTTONX] = true; else buttonStatus[BUTTONX] = false;
    if(sensors[12] || sensors[13] || sensors[14] || sensors[15]) buttonStatus[BUTTONY] = true; else buttonStatus[BUTTONY] = false;
    if(sensors[16] || sensors[17] || sensors[18] || sensors[19]) buttonStatus[BUTTONRB] = true; else buttonStatus[BUTTONRB] = false;
    if(sensors[20] || sensors[21] || sensors[22] || sensors[23]) buttonStatus[BUTTONLB] = true; else buttonStatus[BUTTONLB] = false;
    if(sensors[24] || sensors[25] || sensors[26] || sensors[27]) buttonStatus[BUTTONRT] = true; else buttonStatus[BUTTONRT] = false;
    if(sensors[28] || sensors[29] || sensors[30] || sensors[31]) buttonStatus[BUTTONLT] = true; else buttonStatus[BUTTONLT] = false;
    
    // LEDS
  if(!calibrated){
    bool lightUp = true;
      for (CRGB &led : leds){
        if(lightUp){
          led = CRGB::Yellow;
          lightUp = false;
          }
          else{
            led = CRGB::Black;
            lightUp = true;
            }
            //led = CRGB::White;
        }
  /*}else if(resultBits == noTouchBits){
    bool lightUp = true;
      for (CRGB &led : leds){
        if(lightUp){
          led = CRGB::White;
          lightUp = false;
          }
          else{
            lightUp = true;
            }
        }*/
    }else{
    for (CRGB &led : leds){
        led = CRGB::Yellow;
        }
    for(int i = 0; i < NUM_LEDS_PER_STRIP; i++){
      if(sensors[i]){
        leds[i] = CRGB::Purple;
        if(leds[i+1]) leds[i+1] = CRGB::Purple;
        if(leds[i-1]) leds[i-1] = CRGB::Purple;
        }
      }
    }
  }
  
  
  
  }

void sliderNavigation(){

  // Default Stick
  long resultBits;
  int32_t sliderBits = 0;
  resultBits = sliderBits ^ 0x80808080;
  // SENDING TO CONTROLLER THE RESULTED VALUES
  ReportData.RY = (resultBits >> 24) & 0xFF;
  ReportData.RX = (resultBits >> 16) & 0xFF;
  ReportData.LY = (resultBits >> 8) & 0xFF;
  ReportData.LX = (resultBits) & 0xFF;
  
  // sensors 0-2 & 29-31 : L1 / R1 | Blue
  // sensors 3-5 & 26-28 : L2 / R2 | Darker blue
  // sensors 6 & 25 : L3 / R3 | darkest blue
  // sensors 9-11 : Up | Green
  // sensors 13-15 : Down | Blue
  // sensors 17-19 : Left | Pink
  // sensors 21-23 : Right | Red

  // Bumpers/Triggers
  if(sensors[0] || sensors[1] || sensors[2]) {buttonStatus[BUTTONLB] = true;}
  else {buttonStatus[BUTTONLB] = false;}
  if(sensors[29] || sensors[30] || sensors[31]) {buttonStatus[BUTTONRB] = true;}
  else {buttonStatus[BUTTONRB] = false;}
  if(sensors[3] || sensors[4] || sensors[5]) {buttonStatus[BUTTONLT] = true;}
  else {buttonStatus[BUTTONLT] = false;}
  if(sensors[26] || sensors[27] || sensors[28]) {buttonStatus[BUTTONRT] = true;}
  else {buttonStatus[BUTTONRT] = false;}
  //if(sensors[6]) {buttonStatus[BUTTONL3] = true;}
  //if(sensors[25]) {buttonStatus[BUTTONR3] = true;}

  // DPAD
  if (sensors[9] || sensors[10] || sensors[11]) {buttonStatus[BUTTONUP] = true;}
  else {buttonStatus[BUTTONUP] = false;}
  if (sensors[13] || sensors[14] || sensors[15]) {buttonStatus[BUTTONDOWN] = true;}
  else {buttonStatus[BUTTONDOWN] = false;}
  if (sensors[17] || sensors[18] || sensors[19]) {buttonStatus[BUTTONLEFT] = true;}
  else {buttonStatus[BUTTONLEFT] = false;}
  if (sensors[21] || sensors[22] || sensors[23]) {buttonStatus[BUTTONRIGHT] = true;}
  else {buttonStatus[BUTTONRIGHT] = false;}

  // LEDS
  for (CRGB &led : leds){
        led = CRGB::Black;
        }
  leds[0] = leds[1] = leds[2] = leds[29] = leds[30] = leds[31] = CRGB::Blue; // L1 / R1
  leds[3] = leds[4] = leds[5] = leds[26] = leds[27] = leds[28] = CRGB::MediumBlue; // L2 / R2
  leds[6] = leds[25] = CRGB::CRGB::DarkBlue; // L3 / R3
  leds[8] = leds[9] = leds[10] = CRGB(0, 255, 153); // UP
  leds[12] = leds[13] = leds[14] = CRGB(0, 128, 255); // DOWN
  leds[17] = leds[18] = leds[19] = CRGB(255, 0, 242); // LEFT
  leds[21] = leds[22] = leds[23] = CRGB::Red; // RIGHT
  

  
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
  if (buttonLT.update()) {buttonStatus[BUTTONLT] = buttonLT.fell();}
  if (buttonRT.update()) {buttonStatus[BUTTONRT] = buttonRT.fell();}
  if (buttonSTART.update()) {buttonStatus[BUTTONSTART] = buttonSTART.fell();}
  if (buttonSELECT.update()) {buttonStatus[BUTTONSELECT] = buttonSELECT.fell();}
  if (buttonHOME.update()) {buttonStatus[BUTTONHOME] = buttonHOME.fell();}
  if (switchModePin.update()) {buttonStatus[SWITCHMODEPIN] = switchModePin.fell();}
  
  //if (buttonSwitchMenu.update()) {buttonStatus[BUTTONSWITCHMENU] = buttonSwitchMenu.fell();}
}


void processDPAD(){
    /*ReportData.LX = 128;
    ReportData.LY = 128;
    ReportData.RX = 128;
    ReportData.RY = 128;*/
 
    if ((buttonStatus[BUTTONUP]) && (buttonStatus[BUTTONRIGHT])){ReportData.HAT = DPAD_UPRIGHT_MASK_ON;}
    else if ((buttonStatus[BUTTONDOWN]) && (buttonStatus[BUTTONRIGHT])) {ReportData.HAT = DPAD_DOWNRIGHT_MASK_ON;} 
    else if ((buttonStatus[BUTTONDOWN]) && (buttonStatus[BUTTONLEFT])) {ReportData.HAT = DPAD_DOWNLEFT_MASK_ON;}
    else if ((buttonStatus[BUTTONUP]) && (buttonStatus[BUTTONLEFT])){ReportData.HAT = DPAD_UPLEFT_MASK_ON;}
    else if (buttonStatus[BUTTONUP]) {ReportData.HAT = DPAD_UP_MASK_ON;}
    else if (buttonStatus[BUTTONDOWN]) {ReportData.HAT = DPAD_DOWN_MASK_ON;}
    else if (buttonStatus[BUTTONLEFT]) {ReportData.HAT = DPAD_LEFT_MASK_ON;}
    else if (buttonStatus[BUTTONRIGHT]) {ReportData.HAT = DPAD_RIGHT_MASK_ON;}
    else{ReportData.HAT = DPAD_NOTHING_MASK_ON;}
}

void processLANALOG(){
    ReportData.HAT = DPAD_NOTHING_MASK_ON;
    ReportData.RX = 128;
    ReportData.RY = 128;

    if ((buttonStatus[BUTTONUP]) && (buttonStatus[BUTTONRIGHT])){ReportData.LY = 0;ReportData.LX = 255;}
    else if ((buttonStatus[BUTTONDOWN]) && (buttonStatus[BUTTONRIGHT])) {ReportData.LY = 255;ReportData.LX = 255;}
    else if ((buttonStatus[BUTTONDOWN]) && (buttonStatus[BUTTONLEFT])) {ReportData.LY = 255;ReportData.LX = 0;}
    else if ((buttonStatus[BUTTONUP]) && (buttonStatus[BUTTONLEFT])){ReportData.LY = 0;ReportData.LX = 0;}
    else if (buttonStatus[BUTTONUP]) {ReportData.LY = 0;ReportData.LX = 128;}
    else if (buttonStatus[BUTTONDOWN]) {ReportData.LY = 255;ReportData.LX = 128;}
    else if (buttonStatus[BUTTONLEFT]) {ReportData.LX = 0;ReportData.LY = 128;}
    else if (buttonStatus[BUTTONRIGHT]) {ReportData.LX = 255;ReportData.LY = 128;}
    else {ReportData.LX = 128;ReportData.LY = 128;}
}
void processLANALOGSmash(){
    ReportData.HAT = DPAD_NOTHING_MASK_ON;
    ReportData.RX = 128;
    ReportData.RY = 128;
    
    if ((buttonStatus[BUTTONUP]) && (buttonStatus[BUTTONRIGHT])){ReportData.LY = 64;ReportData.LX = 192;}
    else if ((buttonStatus[BUTTONDOWN]) && (buttonStatus[BUTTONRIGHT])) {ReportData.LY = 192;ReportData.LX = 192;}
    else if ((buttonStatus[BUTTONDOWN]) && (buttonStatus[BUTTONLEFT])) {ReportData.LY = 192;ReportData.LX = 64;}  
    else if ((buttonStatus[BUTTONUP]) && (buttonStatus[BUTTONLEFT])){ReportData.LY = 64;ReportData.LX = 64;}
    else if (buttonStatus[BUTTONUP]) {ReportData.LY = 64;ReportData.LX = 128;}
    else if (buttonStatus[BUTTONDOWN]) {ReportData.LY = 192;ReportData.LX = 128;}
    else if (buttonStatus[BUTTONLEFT]) {ReportData.LX = 64;ReportData.LY = 128;}
    else if (buttonStatus[BUTTONRIGHT]) {ReportData.LX = 192;ReportData.LY = 128;}
    else{ReportData.LX = 128;ReportData.LY = 128;}
}
void processRANALOG(){
    ReportData.HAT = 0x08;
    ReportData.LX = 128;
    ReportData.LY = 128;
    
    if ((buttonStatus[BUTTONUP]) && (buttonStatus[BUTTONRIGHT])){ReportData.RY = 0;ReportData.RX = 255;}
    else if ((buttonStatus[BUTTONUP]) && (buttonStatus[BUTTONLEFT])){ReportData.RY = 0;ReportData.RX = 0;}
    else if ((buttonStatus[BUTTONDOWN]) && (buttonStatus[BUTTONRIGHT])) {ReportData.RY = 255;ReportData.RX = 255;}
    else if ((buttonStatus[BUTTONDOWN]) && (buttonStatus[BUTTONLEFT])) {ReportData.RY = 255;ReportData.RX = 0;}
    else if (buttonStatus[BUTTONUP]) {ReportData.RY = 0;ReportData.RX = 128;}
    else if (buttonStatus[BUTTONDOWN]) {ReportData.RY = 255;ReportData.RX = 128;}
    else if (buttonStatus[BUTTONLEFT]) {ReportData.RX = 0;ReportData.RY = 128;}
    else if (buttonStatus[BUTTONRIGHT]) {ReportData.RX = 255;ReportData.RY = 128;}
    else {ReportData.RX = 128;ReportData.RY = 128;}
    
}
void processButtons(){
  //state gets set with checkModeChange
  switch (state)
  {
    case DIGITAL:
        processDPAD();
        buttonProcessing();
    break;

    case ANALOG_MODE:   
       if(buttonStatus[BUTTONLT]){processRANALOG();}
       else{processLANALOG();}
       buttonProcessing();
    break;

    case SMASH:
       if(buttonStatus[BUTTONB]){processLANALOGSmash();}
       else{processLANALOG();}
       buttonProcessingSmash();
    break;
  }
}
void buttonProcessing(){
  if (buttonStatus[BUTTONA]) {ReportData.Button |= A_MASK_ON;}
  if (buttonStatus[BUTTONB]) {ReportData.Button |= B_MASK_ON;}
  if (buttonStatus[BUTTONX]) {ReportData.Button |= X_MASK_ON;}
  if (buttonStatus[BUTTONY]) {ReportData.Button |= Y_MASK_ON;}
  if (buttonStatus[BUTTONLB]) {ReportData.Button |= LB_MASK_ON;}
  if (buttonStatus[BUTTONRB]) {ReportData.Button |= RB_MASK_ON;}
  if (buttonStatus[BUTTONLT]) {ReportData.Button |= ZL_MASK_ON;}
  if (buttonStatus[BUTTONRT]) {ReportData.Button |= ZR_MASK_ON;}
  if (buttonStatus[BUTTONSTART]){ReportData.Button |= START_MASK_ON;}
  if (buttonStatus[BUTTONSELECT]){ReportData.Button |= SELECT_MASK_ON;}
  if (buttonStatus[BUTTONHOME]){ReportData.Button |= HOME_MASK_ON;}
}
void buttonProcessingSmash(){
  if (buttonStatus[BUTTONA]) {ReportData.Button |= X_MASK_ON;}
  if (buttonStatus[BUTTONB]) {}
  if (buttonStatus[BUTTONX]) {ReportData.Button |= A_MASK_ON;}
  if (buttonStatus[BUTTONY]) {ReportData.Button |= B_MASK_ON;}
  if (buttonStatus[BUTTONLB]) {ReportData.Button |= LB_MASK_ON;}
  if (buttonStatus[BUTTONRB]) {ReportData.Button |= ZR_MASK_ON;}
  if (buttonStatus[BUTTONLT]) {ReportData.Button |= ZL_MASK_ON;}
  if (buttonStatus[BUTTONRT]) {ReportData.Button |= RB_MASK_ON;}
  if (buttonStatus[BUTTONSTART]){ReportData.Button |= START_MASK_ON;}
  if (buttonStatus[BUTTONSELECT]){ReportData.Button |= SELECT_MASK_ON;}
  if (buttonStatus[BUTTONHOME]){ReportData.Button |= HOME_MASK_ON;}
}

/*bool testBoolArray(bool[] boolArray, bool any = false){
  if(!any){
    int boolTrue = 0;
    for(bool val : boolArray){
            if(val)
              boolTrue++;
      }
      if(boolTrue >= (sizeof(boolArray)/sizeof(boolArray[0]))
        return true;
      else
        return false;  
    }
    else{
      for(bool val : boolArray){
            if(val)
              return true;
      }
      return false;
      }
  }*/
