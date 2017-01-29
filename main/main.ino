#include <FastLED.h>
#include <math.h>

#define BRIGHTNESS 180
#define SATURATION 140
#define NUM_LEDS 150

// LED pointers, global brightness and hue
CRGB led[NUM_LEDS];
int brightness = 0;
int hue = 0;

// Midi input pointers and variables
const int D2 = 2;
const int D3 = 3;
const int D4 = 4;
int btn2, btn3, btn4;
int sensorValueA0 = 0;
int sensorValueA1 = 0;

// mode variables
int mode = 0;
bool modeSetup = true;
bool d3High = false;
bool d3Low = false;

// int arrays (reuse for limited RAM)
int bright[NUM_LEDS];
int directions[NUM_LEDS];

// alternating pattern variables
bool flip = false;
int dir = 1;
int color = 135;
int offset = 65;


void loop()
{
  // get button and potentiometer values
  readMidiValues();

  // set global brightness and hue
  setBrightnessAndHue();
      
  // switch modes on button press
  detectModeSwitch();

  // choose routine based on mode
  modeStateMachine();

  // apply changes and delay 5ms
  FastLED.show();
  delay(5);
}

/*
 * Looks for btn D3 to go high then low
 * Increments mode or resets to 0
 */
void detectModeSwitch(){
  if(!d3High && !d3Low && btn3 == 1)
    d3High = true;
  else if(d3High && !d3Low && btn3 == 0){
    d3Low = true;
    d3High = false;
  }
  else if(d3Low){
    switch(mode){
      case 0:
        mode = 1;
        break;
      case 1:
        mode = 2;
        break;
      case 2:
        mode = 3;
        break;
      case 3:
        mode = 4;
        break;
      case 4:
        mode = 5;
        break;
      case 5:
        mode = 0;
        break;
    }
    d3Low = false;
    modeSetup = true;
  }
}

/*
 * Called each iteration to perform
 * the correct routine based on mode
 */
void modeStateMachine(){
  switch(mode){
    case 0:
      mode0();
      break;
    case 1:
      mode1();
      break;
    case 2:
      mode2();
      break;
    case 3:
      mode3();
      break;
    case 4:
      mode4();
      break;
    case 5:
      mode5();
      break;
  }
}

/* 
 *  Mode 0
 *  Unmoving solid color
 */
void mode0(){
  for(int i=0; i<150; i++)
    led[i] = CHSV(hue,255,BRIGHTNESS);
}

/*
 *  Mode 1
 *  Moving solid color
 */
void mode1(){
  if(modeSetup){
    bright[0] = 0;
    modeSetup = false;
  }
  for(int i=0; i<150; i++)
    led[i] = CHSV(hue+bright[0],255,BRIGHTNESS);
  bright[0]++;
  if(bright[0]==255)
    bright[0] = 0;
  delay(5);
}

/* 
 *  Mode 2
 *  Unmoving Gradient
 */
void mode2(){
  for(int i=0; i<150; i++)
    led[i] = CHSV(i+hue,255,BRIGHTNESS);
}

/* 
 *  Mode 3
 *  Moving gradient
 */
void mode3(){
  if(modeSetup){
    for(int i=0; i<150; i++)
      bright[i] = i;
    modeSetup = false;
  }
  for(int i=0; i<150; i++){
    led[i] = CHSV(bright[i]+hue, 255, BRIGHTNESS);
    bright[i]++;
    if(bright[i]==255){
      bright[i]=0;
    }
  }
  delay(5);
}

/* 
 *  Mode 4
 *  Random pulses
 */
void mode4(){
  if(modeSetup){
    for(int i=0; i<150; i++){
      bright[i] = random(50,BRIGHTNESS);
  
      if(random(0,2)==0)
        directions[i] = 1;
      else
        directions[i] = -1;
    }
    modeSetup = false;
  }
  for(int i=0; i<150; i++){
    bright[i] += random(1,8) * directions[i];
    
    if(bright[i] > BRIGHTNESS)
      bright[i] = BRIGHTNESS;
    if(bright[i] < 50)
      bright[i]=50;
    if(bright[i]==50 || bright[i]==BRIGHTNESS)
      directions[i] *= -1;
      
    led[i] = CHSV(hue,100,bright[i]);
  }
  delay(10);
}

/* 
 *  Mode 5
 *  alternating each two LEDs, pulsing
 */
void mode5(){
  if(modeSetup){
    flip = 0;
    bright[0] = 100;
    dir = 1;
    for(int i=0; i<150; i++){
      if(flip==0){
        directions[i] = 0;
        directions[i+1] = 0;
        flip=1;
      }
      else{
        directions[i] = 1;
        directions[i+1] = 1;
        flip=0;
      }
      i++;
    }
    modeSetup = false;
  }
  if(flip){
    for(int i=0; i<150; i++){
      if(directions[i] == 0){
        directions[i] = 1;
        led[i] = CHSV(color+hue, SATURATION, bright[0]);
      }
      else{
        directions[i] = 0;
        led[i] = CHSV(color+offset+hue, SATURATION, bright[0]);
      }
    }
    flip = false;
  }
  else{
    for(int i=0; i<150; i++)
      if(directions[i] == 0)
        led[i] = CHSV(color+hue, SATURATION, bright[0]);
      else
        led[i] = CHSV(color+offset+hue, SATURATION, bright[0]);
  }
  
  if(dir == 1)
    bright[0]+=2;
  else
    bright[0]-=2;
  
  if(bright[0]>=252){
    dir = 0;
    flip = true;
  }
  else if(bright[0]<=180){
    dir = 1;
    flip = true;
  }
  delay(5);
}

/*
 * Gets dial values and button states
 */
void readMidiValues(){
  // get potentiometer values
  sensorValueA0 = analogRead(A0);
  sensorValueA1 = analogRead(A1);

  // get button states
  btn2 = digitalRead(D2);
  btn3 = digitalRead(D3);
  btn4 = digitalRead(D4);
}

/*
 * Scales A0 and A1 from 1023 to 255
 * Call after readMidiValues() to set 
 * global brightness and hue
 */
void setBrightnessAndHue(){
  // recalculate brightness based on Dial A0 scaled to 255
  brightness = int((1.0 * 255 / 1023) * sensorValueA0);
  FastLED.setBrightness(brightness);

  // recalculate hue based on Dial A1 scaled to 255
  hue = int((1.0 * 255 / 1023) * sensorValueA1);
}

void setup() {
  delay(1000);
  FastLED.addLeds<NEOPIXEL, 12>(led, NUM_LEDS);
  Serial.begin(9600); 
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4 , INPUT);
  
  // get button and potentiometer values
  readMidiValues();

  // set global brightness and hue
  setBrightnessAndHue();

  // set initial state based on calculated brightness and hue
  for(int i=0; i<150; i++){
    led[i] = CHSV(hue,SATURATION,brightness);
  }

  FastLED.show();
}

