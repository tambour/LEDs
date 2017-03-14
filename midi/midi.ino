#include <SoftwareSerial.h>
#include <MIDI.h>
#include <FastLED.h>
#include <math.h>

// macros
#define BRIGHTNESS 255
#define SATURATION 255
#define NUM_LEDS 80
#define TRIG 9
#define ECHO 10

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

// LED pointers, global brightness, hue, delay
CRGB led[NUM_LEDS];
int brightness = 200;
int hue = 120;
int waitTime = 5;

// mode vars
int mode = 5;
bool modeSetup = true;
bool action = false;

// gradient vars
int hueStep = 8;
int hueStart = 128;
int hueStop = 192;
int hueDirection = 1;

// sonar vars
int targetBrightness = 180;
int brightnessStep = 3;
int descending = 0;

// make adjustments via sonar reading
// setting sonar toggle makes timing unreliable!!
int sonarToggle = 0;

// int arrays 
int bright[NUM_LEDS];
int directions[NUM_LEDS];
int colors[NUM_LEDS];

// alternating pattern vars
bool flip = false;
int dir = 1;

/*
 * Arduino Loop
 */
void loop()
{
  // get MIDI
  MIDI.read();

  // set sonar brightness
  if(sonarToggle == 1)
    setSonarBrightness();
    
  // set burst brightness
  if(descending != 0)
    setBurstBrightness();

  // choose routine based on mode
  modeStateMachine();

  // apply changes and delay
  FastLED.show();
  delay(waitTime);
}

void setGradientHue(){
  hue += hueStep * hueDirection;
  if(hue >= hueStop){
    hueDirection = -1;
  }
  else if(hue <= hueStart){
    hueDirection = 1;
  }
}

void setBurstBrightness(){
  if(descending > 0){
    brightness -= descending;
    if(brightness <= 0){
      descending = 0;
      brightness = 0;
    }
  }
  else if(descending < 0){
    brightness -= descending;
    if(brightness == 128){
      descending = 0;
      brightness = 128 ;
    }
  }
}

void setSonarBrightness(){
  if(brightness == targetBrightness){
    long duration, distance;
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    duration = pulseIn(ECHO, HIGH);
    distance = (duration/2) / 29.1;
    if(distance <= 50 && distance != 0){
      targetBrightness = int(5*distance + 5);
      if(targetBrightness > 255){
        targetBrightness = 255;
      }
      else if(targetBrightness < 0){
        targetBrightness = 0;
      }
    }    
  }
  if(brightness != targetBrightness){
      if(targetBrightness > brightness){
        brightnessStep = int((targetBrightness - brightness)/32);
        if(brightnessStep == 0)
          brightnessStep = 1;
        brightness += brightnessStep;
        if(brightness > targetBrightness)
          brightness = targetBrightness;
      }
      else{
        brightnessStep = int((brightness - targetBrightness)/32);
        if(brightnessStep == 0)
          brightnessStep = 1;
        brightness -= brightnessStep;
        if(brightness < targetBrightness)
          brightness = targetBrightness;
      }
    }
}

void HandleNoteOn(byte channel, byte pitch, byte velocity) {
  // brightness bursts (C5, C#5, D5) variable speed
  switch(pitch){
    case 60: // increment mode
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
          mode = 6;
          break;
        case 6:
          mode = 7;
          break;
        case 7:
          mode = 8;
          break;
        case 8:
          mode = 9;
          break;
        case 9:
          mode = 0;
          break;
      }
      modeSetup = true;
      break;
    case 62:
      hue+=50;
      break;
    case 64:
      brightness-=20;
      if(brightness < 0)
        brightness = 0;
      break;
    case 65:
      brightness+=20;
      if(brightness > 255)
        brightness = 255;
      break;
    case 67:
      // waitTime -= 5;
      // if(waitTime < 0){
      //   waitTime = 0;
      // }
      break;
    case 69:
      // waitTime += 5;
      break;
    case 71:
      action = true;
      break;
    case 72: // slow burst
      brightness = BRIGHTNESS;
      setGradientHue();
      descending = 1;
      break;
    case 73: // medium burst
      brightness = BRIGHTNESS;
      setGradientHue();
      descending = 2;
      break;
    case 74: // fast burst
      brightness = BRIGHTNESS;
      setGradientHue();
      descending = 7;
      break;
    case 75: // slow swell
      brightness = 0;
      setGradientHue();
      descending = -1;
      break;
    case 76: // stop burst/swell
      descending = 0;
      brightness = BRIGHTNESS;
      break;
    case 77: // color set 1 (ROY)
      hue = 0;
      hueStart = 0;
      hueStop = 50;
      hueStep = 6;
      modeSetup = true;
      break;
    case 78: // color set 2
      hue = 64;
      hueStart = 64;
      hueStop = 128;
      hueStep = 8;
      modeSetup = true;
      break;
    case 79: // color set 3
      hue = 128;
      hueStart = 128;
      hueStop = 192;
      hueStep = 8;
      modeSetup = true;
      break;
    case 80: // color set 4
      hue = 192;
      hueStart = 192;
      hueStop = 255;
      hueStep = 8;
      modeSetup = true;
      break;
    case 81: // color set 5 (ROYGBIV)
      hue = 0;
      hueStart = 0;
      hueStop = 255;
      hueStep = 16;
      modeSetup = true;
      break;
    case 82: // sonar toggle
      if(sonarToggle == 1){
        sonarToggle = 0;
        brightness = BRIGHTNESS;
      }
      else {
        sonarToggle = 1;
      }
      break;
    // mode changes (C7+)
    case 96: // mode 0
      mode = 0;
      modeSetup = true;
      break;
    case 97: // mode 1
      mode = 1;
      modeSetup = true;
      break;
    case 98: // mode 2
      mode = 2;
      modeSetup = true;
      break;
    case 99: // mode 3
      mode = 3;
      modeSetup = true;
      break;
    case 100: // mode 4
      mode = 4;
      modeSetup = true;
      break;
    case 101: // mode 5
      mode = 5;
      modeSetup = true;
      break;
    case 102: // mode 6
      mode = 6;
      modeSetup = true;
      break;
    case 103: // mode 7
      mode = 7;
      modeSetup = true;
      break;
    case 104: // mode 8
      mode = 8;
      modeSetup = true;
      break;
    case 105: // mode 9
      mode = 9;
      modeSetup = true;
      break;
    case 106: // mode 10
      mode = 10;
      modeSetup = true;
      break;
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
    case 6:
      mode6();
      break;
    case 7:
      mode7();
      break;
    case 8:
      mode8();
      break;
    case 9:
      mode9();
      break;
    case 10:
      mode10();
      break;
  }
}

/* 
 *  Mode 0
 *  Unmoving solid color
 */
void mode0(){
  for(int i=0; i<NUM_LEDS; i++)
    led[i] = CHSV(hue,SATURATION,brightness);
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
    led[i] = CHSV(hue+bright[0],SATURATION,brightness);
  bright[0]+=2;
  if(bright[0]==255)
    bright[0] = 0;
}

/* 
 *  Mode 2
 *  Unmoving ROYGBIV Gradient
 */
void mode2(){
  for(int i=0; i<150; i++)
    led[i] = CHSV(i+hue,SATURATION,brightness);
}

/* 
 *  Mode 3
 *  Moving ROYGBIV gradient
 */
void mode3(){
  if(modeSetup){
    for(int i=0; i<150; i++)
      bright[i] = i;
    modeSetup = false;
    descending = 0;
  }
  for(int i=0; i<150; i++){
    led[i] = CHSV(bright[i]+hue, SATURATION, brightness);
    bright[i]++;
    if(bright[i]==255){
      bright[i]=0;
    }
  }
}

/*
 * Mode 4
 * Palette restricted gradient
 * action key SHOULD change direction
 */
void mode4(){
  if(modeSetup){
    descending = 0;
    int adder = 1;
    dir = 1;
    for(int i=0; i<NUM_LEDS; i++){
      bright[i] = hueStart + adder*dir;
      adder += dir;
      if(adder == (hueStop-hueStart)){
        dir = -1;
      }
      else if(adder == 0){
        dir = 1;
      }
    }
    dir = 1;
    modeSetup = false;
  }
  for(int i=NUM_LEDS-1; i>=0; i--){
    if(i == 0){
      if(bright[1] == hueStart){
        dir = 1;
      }
      else if(bright[1] == hueStop){
        dir = -1;
      }
      bright[0] = bright[1] + dir;
    }
    else{
      bright[i] = bright[i-1];
    }
    led[i] = CHSV(bright[i],SATURATION,brightness);
  }
}

/* 
 *  Mode 5
 *  Random pulses
 */
void mode5(){
  if(modeSetup){
    descending = 0;
    for(int i=0; i<NUM_LEDS; i++){
      bright[i] = random(30,brightness);
  
      if(random(0,2)==0)
        directions[i] = 1;
      else
        directions[i] = -1;
    }
    modeSetup = false;
  }
  for(int i=0; i<NUM_LEDS; i++){
    bright[i] += random(1,4) * directions[i];
    
    if(bright[i] > brightness)
      bright[i] = brightness;
    if(bright[i] < 30)
      bright[i]=30;
    if(bright[i]==30 || bright[i]==brightness)
      directions[i] *= -1;
      
    led[i] = CHSV(hue,150,bright[i]);
  }
  delay(5);
}

/* 
 *  Mode 6
 *  alternating each two LEDs, pulsing
 */
void mode6(){
  if(modeSetup){
    descending = 0;
    flip = 0;
    bright[0] = 255;
    dir = 1;
    for(int i=0; i<NUM_LEDS; i++){
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
    bright[0] = 255;
    for(int i=0; i<NUM_LEDS; i++){
      if(directions[i] == 0){
        directions[i] = 1;
        led[i] = CHSV(hueStart, SATURATION, bright[0]);
      }
      else{
        directions[i] = 0;
        led[i] = CHSV(hueStop, SATURATION, bright[0]);
      }
    }
    flip = false;
  }
  else{
    for(int i=0; i<NUM_LEDS; i++)
      if(directions[i] == 0)
        led[i] = CHSV(hueStart, SATURATION, bright[0]);
      else
        led[i] = CHSV(hueStop, SATURATION, bright[0]);
  }

  if(action){
    flip = true;
    action = false;
  }

  if(bright[0] > 180)
    bright[0]-=2;
}

/*
 * Mode 7
 * Maximum Rainbow Power
 * action key advances
 */
 void mode7(){
  if(modeSetup){
    for(int i=0; i<NUM_LEDS; i++){
      colors[i] = random(0, 256);
      bright[i] = random(100, 256);
      if(random(0,2)==0)
        directions[i] = 1;
      else
        directions[i] = -1;
      led[i] = CHSV(colors[i], SATURATION, bright[i]);
    }
    modeSetup = false;
  }
  else{
    for(int i=0; i<NUM_LEDS; i++){
      colors[i] += random(4,10);
      bright[i] += random(1,8)*directions[i];
      if(bright[i] > 255){
        directions[i] = -1;
        bright[i] = 255;
      }
      else if(bright[i] < 100){
        directions[i] = 1;
        bright[i] = 160;
      }
      led[i] = CHSV(colors[i], SATURATION, bright[i]);
    }
    if(action){
      action = false;
      for(int i=NUM_LEDS-1; i>=0; i--){
        if(i!=0){
          colors[i] = colors[i-1];
          directions[i] = directions[i-1];
          bright[i] = bright[i-1];
        }
        else{
          colors[0] = random(0, 256);
          bright[0] = random(100, 256);
          if(random(0,2)==0)
            directions[0] = 1;
          else
            directions[0] = -1;
        }
      }
    }
  }
 }

 /*
  * Mode 8
  * Looping 15 apart
  * action SHOULD change direction
  */
void mode8(){
  if(modeSetup){
    descending = 0;
    int adder = 1;
    dir = 1;
    directions[0] = 1;
    for(int i=0; i<NUM_LEDS; i++){
      // moving lights
      directions[i] = 0;
      if(i%15 == 0)
        directions[i] = 1;
        
      bright[i] = hueStart + adder*dir;
      adder += dir;
      if(adder == (hueStop-hueStart)){
        dir = -1;
      }
      else if(adder == 0){
        dir = 1;
      }
    }
    dir = 1;
    modeSetup = false;
  }
  for(int i=NUM_LEDS-1; i>=0; i--){
    if(i == 0){
      if(bright[1] == hueStart){
        dir = 1;
      }
      else if(bright[1] == hueStop){
        dir = -1;
      }
      bright[0] = bright[1] + dir;
    }
    else{
      bright[i] = bright[i-1];
    }
    if(directions[i] == 0)
      led[i] = CHSV(bright[i],SATURATION,brightness);
    else{
      led[i] = CRGB::White;
      directions[i] = 0;
      directions[i+1] = 1;
      if(i==15)
        directions[0] = 1;
    }
  }
  delay(5);
}

/*
 * Mode 9
 * Strobe
 * action key flashes
 */
void mode9(){
  if(modeSetup){
    dir = 0;
    modeSetup = false;
  }
  if(!action){
    for(int i=0; i<NUM_LEDS; i++){
      led[i] = CHSV(0,0,brightness);
    }
    delay(20);
    dir = 1;
  }
  else{
    action = false;
    for(int i=0; i<NUM_LEDS; i++){
      led[i] = CHSV(0,0,0);
    }
    dir = 0;
    delay(100);
  }
}

/*
 * Mode 10
 * Walking Strobe
 * action key advances
 */
void mode10(){
  if(modeSetup){
    dir = 0;
    bright[0] = 0;
    modeSetup = false;
  }
  if(!action){
    for(int i=0; i<NUM_LEDS; i++){
      if((i+bright[0])%2==0 && i<256)
        led[i] = CHSV(0,0,brightness);
    }
    delay(5);
    dir = 1;

    if(bright[0] == 1)
      bright[0] = 0;
    else
      bright[0] = 1;
  }
  else{
    action = false;
    for(int i=0; i<NUM_LEDS; i++){
        led[i] = CHSV(0,0,0);
    }
    dir = 0;
    delay(80);
  }
}

void setup() {
  delay(200);
  FastLED.addLeds<NEOPIXEL, 8>(led, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(HandleNoteOn);

  // sonar
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  for(int i=0; i<NUM_LEDS; i++){
    led[i] = CHSV(hue,SATURATION,brightness);
  }

  FastLED.show();
}

