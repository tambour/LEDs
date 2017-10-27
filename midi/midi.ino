#include <SoftwareSerial.h>
#include <FastLED.h>
#include <MIDI.h>
#include <math.h>

#define BRIGHTNESS 180
#define IDLE_BRIGHTNESS 100
#define SATURATION 255
#define NUM_LEDS 78 //108
#define DATA_PIN 8 //13
#define TRIG 9
#define ECHO 10
#define SOUND_PIN 4
#define SOUND_THRESHOLD 400
#define SOUND_SENSOR 224
#define DRUM_THRESHOLD 1023
#define PRINT_THRESHOLD 512
#define PIANO_FADE 3
#define PIANO_THRESHOLD 100
#define PIANO_BRIGHTNESS 200

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

// LED pointers, global brightness, hue, delay
CRGB led[NUM_LEDS];
int brightness = IDLE_BRIGHTNESS;
int saturation = SATURATION;
int hue = 120;
int wait_time = 5;

// piano input
bool piano_control = true;
int piano[NUM_LEDS];
int piano_counter = 0;

// qwerty input
bool key_control = true;
int key = 0;

// local sound reading
int sound_sensor = false;
int sound_value = HIGH;

// piezo drum sensor
bool drum_sensor = true;
int drum_value = HIGH;

// direct midi input
bool midi_input = false;

// mode vars
int mode = 0;
bool mode_setup = true;
bool action = false;

// gradient vars
int hue_step = 8;
int hue_start = 128;
int hue_stop = 192;
int hue_direction = 1;

// sonar vars
bool sonar_sensor = false;
int brightness_step = 3;
int descending = 0;

// fade brightness / saturation
bool fade_brightness = false;
bool fade_saturation = false;
int target_brightness = IDLE_BRIGHTNESS;
int target_saturation = SATURATION;
int fade_rate = 1;

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

// debug print helpers
int print_count = 0;
int max_print = 0;
int print_dir = 1;


void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(led, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  if(midi_input){
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.setHandleNoteOn(HandleNoteOn);
  }

  // qwerty to midi
  if(key_control)
    Serial.begin(9600);

  // sonar
  if(sonar_sensor){
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
  }
  
  // initial LED programming
  for(int i=0; i<NUM_LEDS; i++)
    led[i] = CHSV(hue,saturation,brightness);

  FastLED.show();
}

void loop(){
  // get MIDI
  MIDI.read();

  // get qwerty keys
  if(key_control)
    getSerial();

  // poll drum sensor
  if(drum_sensor)
    listenForDrums();

  // poll sound sensor
  if(sound_sensor)
    listenForSound();

  // poll sonar sensor
  if(sonar_sensor)
    listenForSonar();

  // increment fade/swell
  if(descending != 0)
    setBurstBrightness();

  if(fade_brightness || fade_saturation)
    doFade();

  // choose routine based on mode
  modeStateMachine();

  // poll for piano
  if(piano_control)
    applyPiano();

  // apply changes and delay
  FastLED.show();
  delay(wait_time);
}

void listenForDrums(){
  drum_value = analogRead(0);

  if(drum_value >= PRINT_THRESHOLD){

    // lots of debug printing since
    // we have to calibrate this
    if(key_control){

      if(drum_value > max_print)
        max_print = drum_value;

      if(drum_value < 1000)
        Serial.print(" ");

      Serial.print(drum_value);
      Serial.print(" ");

      for(int i=0; i<print_count; i++)
        Serial.print("-");

      if(drum_value >= DRUM_THRESHOLD){
        for(int i=0; i<18-print_count; i++)
          Serial.print(" ");
        Serial.print(" !! - ");
        if(drum_value < 1000)
          Serial.print(" ");
        Serial.print(drum_value);
      }

      Serial.println("");
      print_count += print_dir;
      if(print_count >= 16)
        print_dir=-1;
      if(print_count <= 0)
        print_dir=1;
    }

    // drums trigger mode actions
    action = true;
  }
}

void listenForSound(){

  // poll for measurement [0-1023]
  sound_value = analogRead(SOUND_PIN);

  if(sound_value > SOUND_THRESHOLD){

    // print measurement if debugging
    if(key_control)
      Serial.println(sound_value);

    // set max brightness for fade
    brightness = BRIGHTNESS;

    // do action for mode
    action = true;

    // advance hue
    setGradientHue();

    // set magic number for sound-triggered descent
    descending = SOUND_SENSOR;

    // cancel any active brightness fade
    fade_brightness = false;
  } 
}

void setBurstBrightness(){
  // sensor-triggered fade to idle brightness
  if(descending == SOUND_SENSOR){
    brightness -= 3;
    if(brightness <= target_brightness || brightness < 0){
      descending = 0;
      brightness = target_brightness;
    }
  }
  // brightness fade to 0
  else if(descending > 0){
    brightness -= descending;
    if(brightness <= 0){
      descending = 0;
      brightness = 0;
    }
  }
  // brightness swell to idle
  else if(descending < 0){
    brightness -= descending;
    if(brightness >= IDLE_BRIGHTNESS){
      descending = 0;
      brightness = IDLE_BRIGHTNESS ;
    }
  }
}

void doFade(){

  // fade brightness
  if(target_brightness < brightness){
    brightness -= fade_rate;
    if(brightness <= target_brightness){
      fade_brightness = false;
      brightness = target_brightness;
    }
  }
  else{
    brightness += fade_rate;
    if(brightness >= target_brightness){
      fade_brightness = false;
      brightness = target_brightness;
    }
  }

  // fade saturation
  if(target_saturation < saturation){
    saturation -= fade_rate;
    if(saturation <= target_saturation){
      fade_saturation = false;
      saturation = target_saturation;
    }
  }
  else{
    saturation += fade_rate;
    if(saturation >= target_saturation){
      fade_saturation = false;
      saturation = target_saturation;
    }
  }
}

void setGradientHue(){
  // advance hue
  if(mode == 3)
    return;
  hue += hue_step * hue_direction;
  if(hue >= hue_stop)
    hue_direction = -1;
  else if(hue <= hue_start)
    hue_direction = 1;
}

void listenForSonar(){
  // set brightness based on sonar distance reading
  // BAD: makes timing unreliable
  if(brightness == target_brightness){
    long duration, distance;
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    duration = pulseIn(ECHO, HIGH);
    distance = (duration/2) / 29.1;
    if(distance <= 50 && distance != 0){
      target_brightness = int(5*distance + 5);
      if(target_brightness > 255)
        target_brightness = 255;
      else if(target_brightness < 0)
        target_brightness = 0;
    }    
  }
  if(brightness != target_brightness){
    if(target_brightness > brightness){
      brightness_step = int((target_brightness - brightness)/32);
      if(brightness_step == 0)
        brightness_step = 1;
      brightness += brightness_step;
      if(brightness > target_brightness)
        brightness = target_brightness;
    }
    else{
      brightness_step = int((brightness - target_brightness)/32);
      if(brightness_step == 0)
        brightness_step = 1;
      brightness -= brightness_step;
      if(brightness < target_brightness)
        brightness = target_brightness;
    }
  }
}

/* 
 *  Mode 0
 *  Unmoving solid color
 */
void mode0(){
  if(mode_setup){
    hue = hue_start;
    dir = 1;
    mode_setup = false;
  }
  if(action){
    hue += 8*dir;
    if(hue >= hue_stop){
      dir = -1;
      hue = hue_stop;
    }
    else if(hue <= hue_start){
      dir = 1;
      hue = hue_start;
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
  if(mode_setup){
    dir = 1;
    color=0;
    target = hue_start + dir;
    mode_setup = false;
  }
  if(hue_start == 0 && hue_stop == 255){
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
    if(target >= hue_stop){
      target = hue_stop;
      dir = -1;
    }
    else if(target <= hue_start){
      target = hue_start;
      dir = 1;
    }
    for(int i=0; i<NUM_LEDS; i++)
      led[i] = CHSV(target,saturation,brightness);
  }

  delay(20);
}

/* 
 *  Mode 2
 *  Static Gradient
 */
void mode2(){
  if(mode_setup){
    target = hue_start;
    value = 0;
    directions[0] = 1;
    mode_setup = false;
  }
  else{
    if(action){
      target += 8*directions[0];
      if(target >= hue_stop){
        target = hue_stop;
        directions[0] = -1;
      }
      else if(target <= hue_start){
        target = hue_start;
        directions[0] = 1;
      }
      action = false;
    }
    color = target;
    dir = 1;
    for(int i=0; i<NUM_LEDS; i++){
      if(dir==1){
        if(color >= hue_stop){
          dir = -1;
          led[i] = CHSV(color--,saturation,brightness);
        }
        else{
          led[i] = CHSV(color++,saturation,brightness);
        }
      }
      else{
        if(color <= hue_start){
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
  if(mode_setup){
    for(int i=0; i<NUM_LEDS; i++)
      bright[i] = i;
    mode_setup = false;
    descending = 0;
    value = brightness;
  }
  if(action){
    fade_brightness = true;
    fade_rate = 3;
    target_brightness = value-32;
    if(target_brightness < 0) 
      target_brightness = 0;
    
    action = false;
  }
  if(brightness == value-32){
    fade_brightness = true;
    target_brightness = value;
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
  if(mode_setup){
    descending = 0;
    int adder = 1;
    dir = 1;
    for(int i=0; i<NUM_LEDS; i++){
      bright[i] = hue_start + adder*dir;
      adder += dir;
      if(adder == (hue_stop-hue_start)){
        dir = -1;
        adder = 1;
      }
      else if(adder == 0){
        dir = 1;
        adder = 1;
      }
    }
    dir = 1;
    mode_setup = false;
  }
  for(int i=NUM_LEDS-1; i>=0; i--){
    if(i == 0){
      if(bright[1] == hue_start)
        dir = 1;
      else if(bright[1] == hue_stop)
        dir = -1;
      bright[0] = bright[1] + dir;
    }
    else
      bright[i] = bright[i-1];

    led[i] = CHSV(bright[i],saturation,brightness);
  }
  delay(20);
}

/*
 * Mode 5
 * Looping 15 apart
 */
void mode5(){
  if(mode_setup){
    descending = 0;
    target = hue_start;
    value = 1;
    mode_setup = false;

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
      if(target >= hue_stop){
        target = hue_stop;
        value = -1;
      }
      else if(target <= hue_start){
        target = hue_start;
        value = 1;
      }
      action = false;
    }
    color = target;
    dir = 1;
    for(int i=NUM_LEDS-1; i>=0; i--){
      if(dir==1){
        if(color >= hue_stop){
          dir = -1;
          led[i] = CHSV(color--,saturation,brightness);
        }
        else{
          led[i] = CHSV(color++,saturation,brightness);
        }
      }
      else{
        if(color <= hue_start){
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
  delay(20);
}

/*
 * Mode 6
 * Random glow-points
 */
void mode6(){
  if(mode_setup){
    int adder = 1;
    descending = 0;
    counter = 0;
    dir = 1;
    for(int i=0; i<NUM_LEDS; i++){
      directions[i] = 0;
      colors[i] = 0;
      bright[i] = hue_start + adder*dir;
      adder += dir;
      if(adder == (hue_stop-hue_start)){
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
    mode_setup = false;
  }
  for(int i=NUM_LEDS-1; i>=0; i--){
    if(i == 0){
      if(bright[1] == hue_start)
        dir = 1;
      else if(bright[1] == hue_stop)
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
  delay(10);
}

/*
 * Mode 7
 * Palette gradient with runner
 */
void mode7(){
  if(mode_setup){
    int adder = 1;
    descending = 0;
    counter = 0;
    value = 1;
    dir = 1;
    for(int i=0; i<NUM_LEDS; i++){
      directions[i] = -1;
      bright[i] = hue_start + adder*dir;
      adder += dir;
      if(adder == (hue_stop-hue_start)){
        dir = -1;
        adder = 1;
      }
      else if(adder == 0){
        dir = 1;
        adder = 1;
      }
    }
    dir = 1;
    mode_setup = false;
  }
  for(int i=NUM_LEDS-1; i>=0; i--){
    if(i == 0){
      if(bright[1] == hue_start)
        dir = 1;
      else if(bright[1] == hue_stop)
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
  delay(4);
}

/* 
 *  Mode 8
 *  Random pulses
 */
void mode8(){
  if(mode_setup){
    descending = 0;
    hue = hue_start;
    dir = 1;
    for(int i=0; i<150; i++){
      bright[i] = random(20,brightness);
  
      if(random(0,2)==0)
        directions[i] = 1;
      else
        directions[i] = -1;
    }
    mode_setup = false;
  }
  if(action){
    hue += 8 * dir;
    if(hue >= hue_stop){
      hue = hue_stop;
      dir = -1;
    }
    else if(hue <= hue_start){
      hue = hue_start;
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
    if(hue_start==0 && hue_stop==255)
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
  if(mode_setup){
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
    mode_setup = false;
  }
  if(action){
    for(int i=0; i<150; i++){
      if(directions[i] == 0){
        directions[i] = 1;
        led[i] = CHSV(hue_start, saturation, value);
      }
      else{
        directions[i] = 0;
        led[i] = CHSV(hue_stop, saturation, value);
      }
    }
    action = false;
  }
  else{
    for(int i=0; i<150; i++)
      if(directions[i] == 0)
        led[i] = CHSV(hue_start, saturation, value);
      else
        led[i] = CHSV(hue_stop, saturation, value);
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
  if(mode_setup){
    dir = 0;
    value = 0;
    mode_setup = false;
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
  if(mode_setup){
    for(int i=0; i<NUM_LEDS; i++){
      colors[i] = random(0, BRIGHTNESS);
      bright[i] = random(100, BRIGHTNESS);
      if(random(0,2)==0)
        directions[i] = 1;
      else
        directions[i] = -1;
      led[i] = CHSV(colors[i], saturation, bright[i]);
    }
    counter = 0;
    mode_setup = false;
  }
  else{
    counter++;
    for(int i=0; i<NUM_LEDS; i++){
      colors[i] += random(4,10);
      bright[i] += random(1,8)*directions[i];
      if(bright[i] > BRIGHTNESS){
        directions[i] = -1;
        bright[i] = BRIGHTNESS;
      }
      else if(bright[i] < IDLE_BRIGHTNESS){
        directions[i] = 1;
        bright[i] = IDLE_BRIGHTNESS;
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
          colors[0] = random(0, BRIGHTNESS);
          bright[0] = random(100, BRIGHTNESS);
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
 * Apply piano to LEDs
 */
void applyPiano(){
    for(int i=0; i<NUM_LEDS; i++)
        if(piano[i] >= 1){
            led[i] = CHSV(0,0,piano[i]);
            piano[i] -= PIANO_FADE;
            if (piano[i] <= PIANO_THRESHOLD)
                piano[i] = 0;
        }
}

/*
 * Handle piano note down/up
 */
void handlePiano(int value, bool down){
    
    if(down){            
        // incoming values are [230-255]
        // normalize to get led index
        value -= 230;
            
        // enable led
        piano[value] = PIANO_BRIGHTNESS;
    }
    else{
        // incoming values are [60-85]
        // normalize to get led index
        value -= 60;

        // bump led down on midi off
        piano[value] -= 10;
    }
}   

/*
 * Choose correct routine based on mode
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
    
    // handle piano messages
    if((key >= 60 && key <= 85) || (key >= 230 && key <= 255)){
        handlePiano(key, (key >= 230 && key <= 255));
        return;
    }

    // handle all other messages
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
        target_brightness = 0;
        fade_brightness = true;
        break;
      case 33:
        target_brightness = 51;
        fade_brightness = true;
        break;
      case 34:
        target_brightness = 102;
        fade_brightness = true;
        break;
      case 35:
        target_brightness = 153;
        fade_brightness = true;
        break;
      case 36:
        target_brightness = 204;
        fade_brightness = true;
        break;
      case 37:
        target_brightness = 255;
        fade_brightness = true;
        break;

      // saturation
      case 38:
        target_saturation = 0;
        fade_saturation = true;
        break;
      case 40:
        target_saturation = 51;
        fade_saturation = true;
        break;
      case 41:
        target_saturation = 102;
        fade_saturation = true;
        break;
      case 42:
        target_saturation = 153;
        fade_saturation = true;
        break;
      case 43:
        target_saturation = 204;
        fade_saturation = true;
        break;
      case 44:
        target_saturation = 255;
        fade_saturation = true;
        break;
    }
    key = -1;
    HandleNoteOn(pitch,pitch,pitch);
  }
}

void HandleNoteOn(byte channel, byte pitch, byte velocity) {
  // update state based on received midi byte
  switch(pitch){
    case 71: // action
      action = true;
      break;
    case 72: // slow burst
      brightness = BRIGHTNESS;
      setGradientHue();
      descending = 2;
      fade_brightness = false;
      break;
    case 73: // medium burst
      brightness = BRIGHTNESS;
      setGradientHue();
      descending = 3;
      fade_brightness = false;
      break;
    case 74: // fast burst
      brightness = BRIGHTNESS;
      setGradientHue();
      descending = 6;
      fade_brightness = false;
      break;
    case 75: // slow swell
      brightness = 0;
      setGradientHue();
      descending = -1;
      fade_brightness = false;
      break;
    case 76: // stop burst/swell
      descending = 0;
      brightness = IDLE_BRIGHTNESS;
      break;
    case 77: // color set 1 (ROY)
      hue = 0;
      hue_start = 0;
      hue_stop = 50;
      hue_step = 6;
      mode_setup = true;
      break;
    case 78: // color set 2
      hue = 64;
      hue_start = 64;
      hue_stop = 128;
      hue_step = 8;
      mode_setup = true;
      break;
    case 79: // color set 3
      hue = 128;
      hue_start = 128;
      hue_stop = 192;
      hue_step = 8;
      mode_setup = true;
      break;
    case 80: // color set 4
      hue = 192;
      hue_start = 192;
      hue_stop = 255;
      hue_step = 8;
      mode_setup = true;
      break;
    case 81: // color set 5 (ROYGBIV)
      hue = 0;
      hue_start = 0;
      hue_stop = 255;
      hue_step = 16;
      mode_setup = true;
      break;
    case 82: // sonar toggle
      if(sonar_sensor){
        sonar_sensor = false;
        brightness = BRIGHTNESS;
      }
      else
        sonar_sensor = true;
      break;
    // mode changes (C7+)
    case 96: // mode 0
      mode = 0;
      mode_setup = true;
      break;
    case 97: // mode 1
      mode = 1;
      mode_setup = true;
      break;
    case 98: // mode 2
      mode = 2;
      mode_setup = true;
      break;
    case 99: // mode 3
      mode = 3;
      mode_setup = true;
      break;
    case 100: // mode 4
      mode = 4;
      mode_setup = true;
      break;
    case 101: // mode 5
      mode = 5;
      mode_setup = true;
      break;
    case 102: // mode 6
      mode = 6;
      mode_setup = true;
      break;
    case 103: // mode 7
      mode = 7;
      mode_setup = true;
      break;
    case 104: // mode 8
      mode = 8;
      mode_setup = true;
      break;
    case 105: // mode 9
      mode = 9;
      mode_setup = true;
      break;
    case 106: // mode 10
      mode = 10;
      mode_setup = true;
      break;
    case 107: // mode 11
      mode = 11;
      mode_setup = true;
      break;
  }
}

