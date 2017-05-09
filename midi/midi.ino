#include <SoftwareSerial.h>
#include <FastLED.h>
#include <MIDI.h>
#include <math.h>

#define BRIGHTNESS 255
#define IDLE_BRIGHTNESS 100
#define SATURATION 255
#define NUM_LEDS 150
#define DATA_PIN 8 //13
#define TRIG 9
#define ECHO 10
#define SOUND_PIN 4

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

// LED pointers, global brightness, hue, delay
CRGB led[NUM_LEDS];
int brightness = IDLE_BRIGHTNESS;
int saturation = SATURATION;
int hue = 120;
int waitTime = 5;

// qwerty input
bool keyControl = true;
int key = 0;

// local sound reading
int soundValue = HIGH;
int soundCooldown = 10;

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
int brightnessStep = 3;
int descending = 0;
bool sonar = false;

// fade brightness / saturation
bool fadeBrightness = false;
bool fadeSaturation = false;
int targetBrightness = 128;
int targetSaturation = 255;
int fadeRate = 1;

// general purpose vars
int bright[NUM_LEDS];
int directions[NUM_LEDS];
int colors[NUM_LEDS];
int dir = 1;
int counter = 0;
int value = 0;
int target = 0;
int index = 0;
int color = 0;

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
  //if(sonar)
    //setSonarBrightness();

  // read sound sensor
  checkForSound();
    
  // set burst brightness
  if(descending != 0)
    setBurstBrightness();

  if(fadeBrightness || fadeSaturation)
    doFade();

  // choose routine based on mode
  modeStateMachine();

  // apply changes and delay
  FastLED.show();
  delay(waitTime);
}

void checkForSound(){
  soundValue = analogRead(SOUND_PIN);
  if(soundValue > 400)
    Serial.println(soundValue);

  if(soundValue > 400){
    brightness = BRIGHTNESS;
    action = true;
    if(mode != 3)
      setGradientHue();
    descending = 224;
    fadeBrightness = false;
  }
  else{
    if(soundCooldown > 0)
      soundCooldown--;
  }
  
}

void setBurstBrightness(){
  if(descending == 224){
    // magic number for kick drum burst
    brightness -= 3;
    if(brightness <= targetBrightness || brightness < 0){
      descending = 0;
      brightness = targetBrightness;
    }
  }
  else if(descending > 0){
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

void doFade(){

  if(targetBrightness < brightness){
    brightness -= fadeRate;
    if(brightness <= targetBrightness){
      fadeBrightness = false;
      brightness = targetBrightness;
    }
  }
  else{
    brightness += fadeRate;
    if(brightness >= targetBrightness){
      fadeBrightness = false;
      brightness = targetBrightness;
    }
  }

  if(targetSaturation < saturation){
    saturation -= fadeRate;
    if(saturation <= targetSaturation){
      fadeSaturation = false;
      saturation = targetSaturation;
    }
  }
  else{
    saturation += fadeRate;
    if(saturation >= targetSaturation){
      fadeSaturation = false;
      saturation = targetSaturation;
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
  if(modeSetup){
    hue = hueStart;
    dir = 1;
    modeSetup = false;
  }
  if(action){
    hue += 8*dir;
    if(hue >= hueStop){
      dir = -1;
      hue = hueStop;
    }
    else if(hue <= hueStart){
      dir = 1;
      hue = hueStart;
    }
    action = false;
  }
  for(int i=0; i<NUM_LEDS; i++)
    led[i] = CHSV(hue,saturation,brightness);
}

/*
 *  Mode 1
 *  Moving solid color
 */
void mode1(){
  if(modeSetup){
    dir = 1;
    color=0;
    target = hueStart + dir;
    modeSetup = false;
  }
  if(hueStart == 0 && hueStop == 255){
    if(action){
      dir *= -1;
      action = false;
    }
    color += dir;

    for(int i=0; i<NUM_LEDS; i++)
      led[i] = CHSV(color,saturation,brightness);
  }
  else{
    if(action){
    if(dir == 1){
      dir = -1;
    }
    else{
      dir = 1;
    }
    action = false;
    }
    target += dir;
    if(target >= hueStop){
      target = hueStop;
      dir = -1;
    }
    else if(target <= hueStart){
      target = hueStart;
      dir = 1;
    }
    for(int i=0; i<NUM_LEDS; i++)
      led[i] = CHSV(target,saturation,brightness);
  }

  delay(10);
}

/* 
 *  Mode 2
 *  Static Gradient
 */
void mode2(){
  if(modeSetup){
    target = hueStart;
    value = 0;
    directions[0] = 1;
    modeSetup = false;
  }
  else{
    if(action){
      target += 8*directions[0];
      if(target >= hueStop){
        target = hueStop;
        directions[0] = -1;
      }
      else if(target <= hueStart){
        target = hueStart;
        directions[0] = 1;
      }
      action = false;
    }
    color = target;
    dir = 1;
    for(int i=0; i<NUM_LEDS; i++){
      if(dir==1){
        if(color >= hueStop){
          dir = -1;
          led[i] = CHSV(color--,saturation,brightness);
        }
        else{
          led[i] = CHSV(color++,saturation,brightness);
        }
      }
      else{
        if(color <= hueStart){
          dir = 1;
          led[i] = CHSV(color++,saturation,brightness);
        }
        else{
          led[i] = CHSV(color--,saturation,brightness);
        }
      }
    }
  }
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
    value = brightness;
  }
  if(action){
    fadeBrightness = true;
    fadeRate = 3;
    targetBrightness = value-32;
    if(targetBrightness < 0) 
      targetBrightness = 0;
    
    action = false;
  }
  if(brightness == value-32){
    fadeBrightness = true;
    targetBrightness = value;
  }
  for(int i=0; i<NUM_LEDS; i++){
    led[i] = CHSV(bright[i]+hue, saturation, brightness);
    bright[i]++;
    if(bright[i]==255)
      bright[i]=0;
  }
  // slower moving gradient is sexy
  delay(20);
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
      if(adder == (hueStop-hueStart)){
        dir = -1;
        adder = 1;
      }
      else if(adder == 0){
        dir = 1;
        adder = 1;
      }
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

    led[i] = CHSV(bright[i],saturation,brightness);
  }
}

/*
 * looping 15 fix?
 */
void mode5(){
  if(modeSetup){
    descending = 0;
    target = hueStart;
    value = 1;
    modeSetup = false;

    counter = 0;
    for(int i=0; i<NUM_LEDS; i++){
      // mark every 15th light
      directions[i] = 0;
      if(i==0)
        directions[i] = 1;
      if(counter == 15){
        directions[i] = 1;
        counter = 0;
      }

      counter++;
    }
  }
  else{
    if(action){
      target += 8*value;
      if(target >= hueStop){
        target = hueStop;
        value = -1;
      }
      else if(target <= hueStart){
        target = hueStart;
        value = 1;
      }
      action = false;
    }
    color = target;
    dir = 1;
    for(int i=NUM_LEDS-1; i>=0; i--){
      if(dir==1){
        if(color >= hueStop){
          dir = -1;
          led[i] = CHSV(color--,saturation,brightness);
        }
        else{
          led[i] = CHSV(color++,saturation,brightness);
        }
      }
      else{
        if(color <= hueStart){
          dir = 1;
          led[i] = CHSV(color++,saturation,brightness);
        }
        else{
          led[i] = CHSV(color--,saturation,brightness);
        }
      }

      if(directions[i] == 1){\
        led[i] = CHSV(0,0,255);

        directions[i] = 0;

        if(i!=NUM_LEDS-1)
          directions[i+1] = 1;

        if(i == 15)
          directions[0] = 1;
      }
    }
  }
  delay(10);
}
 /*
  * Mode 5
  * Looping 15 apart
  */
void mode15(){
  if(modeSetup){
    int adder = 1;
    for(int i=0; i<NUM_LEDS; i++){
      // mark every 15th light
      directions[i] = 0;
      if(i==0){
        directions[i] = 1;
      }
      if(i%15 == 0)
        directions[i] = 1;
        
      bright[i] = hueStart + adder*dir;
      adder += dir;
      if(adder == (hueStop-hueStart)){
        dir = -1;
        adder = 1;
      }
      else if(adder == 0){
        dir = 1;
        adder = 1;
      }
    }
    
    dir = 1;
    value = 1;
    target = 1;
    descending = 0;
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

    if(action){
      if(target == 1)
        target = 0;
      else
        target = 1;
      action = false;
    }
    
    if(directions[i] == 0){
      led[i] = CHSV(bright[i],saturation,brightness);
    }
    else{
      if(target == 1)
        led[i] = CHSV(0,0,255);
      else
        led[i] = CHSV(bright[i],saturation,16);
      
      directions[i] = 0;
      directions[i+1] = 1;
      if(i==15)
        directions[0] = 1;
    }
  }
 
  delay(35);
}

/*
 * Mode 6
 * Random glow-points
 */
void mode6(){
  if(modeSetup){
    int adder = 1;
    descending = 0;
    counter = 0;
    dir = 1;
    for(int i=0; i<NUM_LEDS; i++){
      directions[i] = 0;
      colors[i] = 0;
      bright[i] = hueStart + adder*dir;
      adder += dir;
      if(adder == (hueStop-hueStart)){
        dir = -1;
        adder = 1;
      }
      else if(adder == 0){
        dir = 1;
        adder = 1;
      }
    }
    int pos = random(0,NUM_LEDS);
    directions[pos] = 1;
    colors[pos] = 0;
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

    led[i] = CHSV(bright[i],saturation,brightness);

    if(directions[i] == 1 || directions[i] == -1){
      colors[i] = colors[i] + directions[i];
      if(colors[i] > brightness){
        colors[i] = brightness;
        directions[i] = -1;
      }
      else if(colors[i] < 0){
        colors[i] = 0;
        directions[i] = 0;
      }
      led[i] = CHSV(bright[i], 0, colors[i]);
    }
  }
  counter++;
  if(action){
    counter = 16;
    action = false;
  }
  while(counter > 15){
    int pos = random(0, NUM_LEDS);
    if(directions[pos]!=1){
      directions[pos] = 1;
      colors[pos] = 0;
      counter = 0;
    }
  }
  delay(5);
}

/*
 * Mode 7
 * Palette gradient with runner
 */
void mode7(){
  if(modeSetup){
    int adder = 1;
    descending = 0;
    counter = 0;
    value = 1;
    dir = 1;
    for(int i=0; i<NUM_LEDS; i++){
      directions[i] = -1;
      bright[i] = hueStart + adder*dir;
      adder += dir;
      if(adder == (hueStop-hueStart)){
        dir = -1;
        adder = 1;
      }
      else if(adder == 0){
        dir = 1;
        adder = 1;
      }
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

    bool active = false;
    for(int k=0; k<16; k++){
      if(directions[k]==i)
        active = true;
    }

    if(!active)
      led[i] = CHSV(bright[i],saturation,brightness);

    directions[i] = -1;

    if(i == counter){
      if(value == 1){
        led[i] = CHSV(bright[i], 0, brightness);

        for(int j=0; j<8; j++){
          if(i-j >= 0){
            directions[j] = i-j;
            led[i-j] = CHSV(bright[i], 0, brightness);
          }
        }

        for(int j=8; j<16; j++){
          if(i-j >= 0){
            directions[j] = i-j;
            if(brightness/(j+1) > 0)
              led[i-j] = CHSV(bright[i], 0, brightness/(j+1));
            else
              led[i-j] = CHSV(bright[i], 0, 0);
          }
        }
      }
      else{
        led[i] = CHSV(bright[i], 0, brightness);

        for(int j=0; j<8; j++){
          if(i+j < NUM_LEDS-1){
            directions[j] = i+j;
            led[i+j] = CHSV(bright[i], 0, brightness);
          }
        }

        for(int j=8; j<16; j++){
          if(i+j < NUM_LEDS-1){
            directions[j] = i+j;
            if(brightness/(j+1) > 0)
              led[i+j] = CHSV(bright[i], 0, brightness/(j+1));
            else
              led[i+j] = CHSV(bright[i], 0, 0);
          }
        }
      }
    }
  }
  if(counter >= NUM_LEDS-1){
    counter = NUM_LEDS-1;
    value = -1;
  }
  else if(counter <= 0){
    counter = 0;
    value = 1;
  }
  counter = counter + value;
  if(action){
    for(int i=0; i<12; i++)
      directions[i] = -1;
    if(value == 1){
      value = -1;
    }
    else{
      value = 1;
    }
    action = false;
  }
  delay(5);
}

/* 
 *  Mode 8
 *  Random pulses
 */
void mode8(){
  if(modeSetup){
    descending = 0;
    hue = hueStart;
    dir = 1;
    for(int i=0; i<150; i++){
      bright[i] = random(20,brightness);
  
      if(random(0,2)==0)
        directions[i] = 1;
      else
        directions[i] = -1;
    }
    modeSetup = false;
  }
  if(action){
    hue += 8 * dir;
    if(hue >= hueStop){
      hue = hueStop;
      dir = -1;
    }
    else if(hue <= hueStart){
      hue = hueStart;
      dir = 1;
    }
    action = false;
  }
  for(int i=0; i<150; i++){
    bright[i] += random(1,3) * directions[i];
    
    if(bright[i] > brightness)
      bright[i] = brightness;
    if(bright[i] < 20)
      bright[i]=20;
    if(bright[i]==20 || bright[i]==brightness)
      directions[i] *= -1;

    // rainbow hue yields white glows
    if(hueStart==0 && hueStop==255)
      led[i] = CHSV(0,0,bright[i]);
    else
      led[i] = CHSV(hue,130,bright[i]);
  }
  delay(8);
}

/* 
 *  Mode 9
 *  alternating each two LEDs, pulsing
 */
void mode9(){
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
        led[i] = CHSV(hueStart, saturation, value);
      }
      else{
        directions[i] = 0;
        led[i] = CHSV(hueStop, saturation, value);
      }
    }
    action = false;
  }
  else{
    for(int i=0; i<150; i++)
      if(directions[i] == 0)
        led[i] = CHSV(hueStart, saturation, value);
      else
        led[i] = CHSV(hueStop, saturation, value);
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
 * Mode 10
 * Strobe
 */
void mode10(){
  if(modeSetup){
    dir = 0;
    value = 0;
    modeSetup = false;
  }

  if(!value && action){
    value = 1;
    action = false;
  }

  if(!value){
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
  else{
    if(action){
      if(dir==0){
        for(int i=0; i<NUM_LEDS; i++)
          led[i] = CHSV(0,0,0);
        delay(150);
        dir = 1;
      }
      else{
        for(int i=0; i<NUM_LEDS; i++)
          led[i] = CHSV(0,0,brightness);
        dir = 0;
        action = false;
        delay(15);
      }
    }
    else{
      for(int i=0; i<NUM_LEDS; i++)
          led[i] = CHSV(0,0,0);
    }
  }
}

/*
 * Mode 11
 * Maximum Rainbow Power
 */
 void mode11(){
  if(modeSetup){
    for(int i=0; i<NUM_LEDS; i++){
      colors[i] = random(0, 256);
      bright[i] = random(100, 256);
      if(random(0,2)==0)
        directions[i] = 1;
      else
        directions[i] = -1;
      led[i] = CHSV(colors[i], saturation, bright[i]);
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
      led[i] = CHSV(colors[i], saturation, bright[i]);
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


void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(led, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(HandleNoteOn);

  // qwerty to midi
  if(keyControl)
    Serial.begin(9600);

  // sonar
  //pinMode(TRIG, OUTPUT);
  //pinMode(ECHO, INPUT);

  for(int i=0; i<NUM_LEDS; i++)
    led[i] = CHSV(hue,saturation,brightness);

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
    case 10:
      mode10();
      break;
    case 11:
      mode11();
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

      //actions
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
      case 122:
        pitch = 107;
        break;

      // brightness
      case 32:
        targetBrightness = 0;
        fadeBrightness = true;
        break;
      case 33:
        targetBrightness = 51;
        fadeBrightness = true;
        break;
      case 34:
        targetBrightness = 102;
        fadeBrightness = true;
        break;
      case 35:
        targetBrightness = 153;
        fadeBrightness = true;
        break;
      case 36:
        targetBrightness = 204;
        fadeBrightness = true;
        break;
      case 37:
        targetBrightness = 255;
        fadeBrightness = true;
        break;

      // saturation
      case 38:
        targetSaturation = 0;
        fadeSaturation = true;
        break;
      case 40:
        targetSaturation = 51;
        fadeSaturation = true;
        break;
      case 41:
        targetSaturation = 102;
        fadeSaturation = true;
        break;
      case 42:
        targetSaturation = 153;
        fadeSaturation = true;
        break;
      case 43:
        targetSaturation = 204;
        fadeSaturation = true;
        break;
      case 44:
        targetSaturation = 255;
        fadeSaturation = true;
        break;
    }
    key = -1;
    HandleNoteOn(pitch,pitch,pitch);
  }
}

void HandleNoteOn(byte channel, byte pitch, byte velocity) {
  switch(pitch){
    case 71: // action
      action = true;
      break;
    case 72: // slow burst
      brightness = BRIGHTNESS;
      if(mode != 3)
        setGradientHue();
      descending = 2;
      fadeBrightness = false;
      break;
    case 73: // medium burst
      brightness = BRIGHTNESS;
      if(mode != 3)
        setGradientHue();
      descending = 3;
      fadeBrightness = false;
      break;
    case 74: // fast burst
      brightness = BRIGHTNESS;
      if(mode != 3)
        setGradientHue();
      descending = 6;
      fadeBrightness = false;
      break;
    case 75: // slow swell
      brightness = 0;
      if(mode != 3)
        setGradientHue();
      descending = -1;
      fadeBrightness = false;
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
    case 106: // mode 10
      mode = 10;
      modeSetup = true;
      break;
    case 107: // mode 11
      mode = 11;
      modeSetup = true;
      break;
  }
}


