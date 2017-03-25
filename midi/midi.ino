#include <SoftwareSerial.h>
#include <FastLED.h>
#include <MIDI.h>
#include <math.h>

#define BRIGHTNESS 255
#define IDLE_BRIGHTNESS 100
#define SATURATION 255
#define NUM_LEDS 150
#define DATA_PIN 13 //8
#define TRIG 9
#define ECHO 10

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

// LED pointers, global brightness, hue, delay
CRGB led[NUM_LEDS];
int brightness = IDLE_BRIGHTNESS;
int hue = 120;
int waitTime = 5;

// qwerty input
bool keyControl = true;
int key = 0;

// mode vars
int mode = 4;
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
bool sonar = false;

// general purpose vars
int bright[NUM_LEDS];
int directions[NUM_LEDS];
int colors[NUM_LEDS];
int dir = 1;
int counter = 0;
int value = 0;

/*
 * Arduino Loop
 */
void loop()
{
  // get MIDI
  MIDI.read();

  // get qwerty keys
  if(keyControl)
    getSerial();

  // set sonar brightness
  if(sonar)
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
    if(brightness >= IDLE_BRIGHTNESS){
      descending = 0;
      brightness = IDLE_BRIGHTNESS ;
    }
  }
}

void setGradientHue(){
  hue += hueStep * hueDirection;
  if(hue >= hueStop)
    hueDirection = -1;
  else if(hue <= hueStart)
    hueDirection = 1;
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
      if(targetBrightness > 255)
        targetBrightness = 255;
      else if(targetBrightness < 0)
        targetBrightness = 0;
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
    value = 0;
    modeSetup = false;
  }
  for(int i=0; i<NUM_LEDS; i++)
    led[i] = CHSV(hue+value,SATURATION,brightness);
  value+=2;
  if(value==255)
    value = 0;
}

/* 
 *  Mode 2
 *  Unmoving ROYGBIV Gradient
 */
void mode2(){
  for(int i=0; i<NUM_LEDS; i++)
    led[i] = CHSV(i+hue,SATURATION,brightness);
}

/* 
 *  Mode 3
 *  Moving ROYGBIV gradient
 */
void mode3(){
  if(modeSetup){
    for(int i=0; i<NUM_LEDS; i++)
      bright[i] = i;
    modeSetup = false;
    descending = 0;
  }
  for(int i=0; i<NUM_LEDS; i++){
    led[i] = CHSV(bright[i]+hue, SATURATION, brightness);
    bright[i]++;
    if(bright[i]==255)
      bright[i]=0;
  }
}

/*
 * Mode 4
 * Palette restricted gradient
 */
void mode4(){
  if(modeSetup){
    descending = 0;
    int adder = 1;
    dir = 1;
    for(int i=0; i<NUM_LEDS; i++){
      bright[i] = hueStart + adder*dir;
      adder += dir;
      if(adder == (hueStop-hueStart))
        dir = -1;
      else if(adder == 0)
        dir = 1;
    }
    dir = 1;
    modeSetup = false;
  }
  for(int i=NUM_LEDS-1; i>=0; i--){
    if(i == 0){
      if(bright[1] == hueStart)
        dir = 1;
      else if(bright[1] == hueStop)
        dir = -1;
      bright[0] = bright[1] + dir;
    }
    else
      bright[i] = bright[i-1];

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
    for(int i=0; i<150; i++){
      bright[i] = random(50,brightness);
  
      if(random(0,2)==0)
        directions[i] = 1;
      else
        directions[i] = -1;
    }
    modeSetup = false;
  }
  for(int i=0; i<150; i++){
    bright[i] += random(1,4) * directions[i];
    
    if(bright[i] > brightness)
      bright[i] = brightness;
    if(bright[i] < 50)
      bright[i]=50;
    if(bright[i]==50 || bright[i]==brightness)
      directions[i] *= -1;
      
    led[i] = CHSV(hue,100,bright[i]);
  }
  delay(8);
}

/* 
 *  Mode 6
 *  alternating each two LEDs, pulsing
 */
void mode6(){
  if(modeSetup){
    descending = 0;
    value = 160;
    dir = 1;
    for(int i=0; i<150; i++){
      if(dir==0){
        directions[i] = 0;
        directions[i+1] = 0;
        dir=1;
      }
      else{
        directions[i] = 1;
        directions[i+1] = 1;
        dir=0;
      }
      i++;
    }
    dir=1;
    modeSetup = false;
  }
  if(action){
    for(int i=0; i<150; i++){
      if(directions[i] == 0){
        directions[i] = 1;
        led[i] = CHSV(hueStart, SATURATION, value);
      }
      else{
        directions[i] = 0;
        led[i] = CHSV(hueStop, SATURATION, value);
      }
    }
    action = false;
  }
  else{
    for(int i=0; i<150; i++)
      if(directions[i] == 0)
        led[i] = CHSV(hueStart, SATURATION, value);
      else
        led[i] = CHSV(hueStop, SATURATION, value);
  }
  
  if(dir == 1)
    value+=2;
  else
    value-=2;
  
  if(value>=252)
    dir = 0;
  else if(value<=160)
    dir = 1;
}

/*
 * Mode 7
 * Maximum Rainbow Power
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
    counter = 0;
    modeSetup = false;
  }
  else{
    counter++;
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
      counter = 0;
    }
  }
  
 }

 /*
  * Mode 8
  * Looping 15 apart
  */
void mode8(){
  if(modeSetup){
    descending = 0;
    int adder = 1;
    dir = 1;
    directions[0] = 1;
    for(int i=0; i<NUM_LEDS; i++){
      // mark every 15th light
      directions[i] = 0;
      if(i%15 == 0)
        directions[i] = 1;
        
      bright[i] = hueStart + adder*dir;
      adder += dir;
      if(adder == (hueStop-hueStart))
        dir = -1;
      else if(adder == 0)
        dir = 1;
    }
    dir = 1;
    value = 1;
    modeSetup = false;
  }
  if(value){
    for(int i=NUM_LEDS-1; i>=0; i--){
      if(i == 0){
        if(bright[1] == hueStart)
          dir = 1;
        else if(bright[1] == hueStop)
          dir = -1;
        bright[0] = bright[1] + dir;
      }
      else
        bright[i] = bright[i-1];

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
  }
  else{
    for(int i=0; i<NUM_LEDS-1; i++){
      if(i == 0){
        if(bright[1] == hueStart)
          dir = 1;
        else if(bright[1] == hueStop)
          dir = -1;
        bright[0] = bright[1] + dir;
      }
      else
        bright[i] = bright[i-1];

      if(directions[i] == 0)
        led[i] = CHSV(bright[i],SATURATION,brightness);
      else{
        led[i] = CRGB::White;
        directions[i] = 0;
        directions[i+1] = 1;
        if(i==NUM_LEDS-15)
          directions[NUM_LEDS-1] = 1;
      }
    }
  }
  delay(20);
}

/*
 * Mode 9
 * Strobe
 */
void mode9(){
  if(modeSetup){
    dir = 0;
    modeSetup = false;
  }

  if(dir==0){
    for(int i=0; i<NUM_LEDS; i++)
      led[i] = CHSV(0,0,0);
    delay(15);
    dir = 1;
  }
  else{
    for(int i=0; i<NUM_LEDS; i++)
      led[i] = CHSV(0,0,brightness);
    dir = 0;
    delay(100);
  }
}


void setup() {
  delay(200);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(led, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(HandleNoteOn);

  // qwerty to midi
  if(keyControl)
    Serial.begin(9600);

  // sonar
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  for(int i=0; i<NUM_LEDS; i++)
    led[i] = CHSV(hue,SATURATION,brightness);

  FastLED.show();
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
  }
}

/*
 * Get serial readings for simulating
 * MIDI via qwerty keyboard
 */
void getSerial(){
  if(Serial.available()){
    key = Serial.read();
    Serial.println(key);
    byte pitch = 0;

    switch(key){
      case 113:
        pitch = 71;
        break;
      case 119:
        pitch = 72;
        break;
      case 101:
        pitch = 73;
        break;
      case 114:
        pitch = 74;
        break;
      case 116:
        pitch = 75;
        break;
      case 121:
        pitch = 76;
        break;
      case 117:
        pitch = 77;
        break;
      case 105:
        pitch = 78;
        break;
      case 111:
        pitch = 79;
        break;
      case 112:
        pitch = 80;
        break;
      case 91:
        pitch = 81;
        break;
      // modes
      case 97:
        pitch = 96;
        break;
      case 115:
        pitch = 97;
        break;
      case 100:
        pitch = 98;
        break;
      case 102:
        pitch = 99;
        break;
      case 103:
        pitch = 100;
        break;
      case 104:
        pitch = 101;
        break;
      case 106:
        pitch = 102;
        break;
      case 107:
        pitch = 103;
        break;
      case 108:
        pitch = 104;
        break;
      case 59:
        pitch = 105;
        break;
      case 39:
        pitch = 106;
        break;
    }
    key = -1;
    HandleNoteOn(pitch,pitch,pitch);
  }
}

void HandleNoteOn(byte channel, byte pitch, byte velocity) {
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
      waitTime -= 5;
      if(waitTime < 0)
        waitTime = 0;
    case 69:
      waitTime += 5;
      break;
    case 71: // action key
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
      brightness = IDLE_BRIGHTNESS;
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
      if(sonar){
        sonar = false;
        brightness = BRIGHTNESS;
      }
      else
        sonar = true;
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
  }
}

