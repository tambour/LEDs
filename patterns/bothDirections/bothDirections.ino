#include <FastLED.h>
#include <math.h>

#define BRIGHTNESS 140
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

// animation variables
int start = 50;
int counter = 2;
bool off1 = false;
bool off2 = false;
int startPos = 70;



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
  //delay(5);
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
  }
}

/* 
 *  Mode 1
 *  Unmoving solid color
 */
void mode1(){
  for(int i=0; i<150; i++)
    led[i] = CHSV(hue,255,BRIGHTNESS);
}

/* 
 *  Mode 0
 *  Unmoving Gradient
 */
void mode0(){
  if(modeSetup){
    start = startPos;
    led[start] = CRGB::White;
    counter = 2;
    modeSetup = false;

    for(int i=0; i<150; i++){
      directions[i] = 0;
      bright[i] = hue;
    }
  }

  for(int i=1; i<counter; i++){
    if(start + i < 150){
      bright[start + i]+=2;
      led[start + i] = CHSV(bright[start+i],255,BRIGHTNESS);
      directions[start + i]++;
    }
    if(start - i >= 0){
      bright[start - i]+=2;
      led[start - i] = CHSV(bright[start-i],255,BRIGHTNESS);
      directions[start - i]++;
    }
  }

  
  for(int i=1; i<150; i++){
    if(start + i < 150){
      if(directions[start + i] == 0 || directions[start + i] > 20){
        led[start + i] = CRGB::Black;
      }
    }
    if(start - i >= 0){
      if(directions[start - i] == 0 || directions[start - i] > 20){
        led[start - i] = CRGB::Black;
      }
    }
  }
  if(counter > 0)
    counter++;
  if(counter > 100){
    counter = 0;
    modeSetup = true;
    startPos += dir * 5;
    if(startPos > 100 || startPos < 50){
      dir *= -1;
    }
  }
    

  //delay(10);
  
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
    //led[i] = CHSV(hue,SATURATION,brightness);
  }

  FastLED.show();
}

