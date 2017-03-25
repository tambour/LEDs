#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <FastLED.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>
#include <math.h>

#define NUM_LEDS 150
#define DATA_PIN 12
#define BRIGHTNESS 180
#define SATURATION 140

const int D2 = 2;
const int D3 = 3;
const int D4 = 4;

int sensorValueA0 = 0;
int sensorValueA1 = 0;

CRGB leds[NUM_LEDS];

int brightness = 0;
int hue = 0;
int hueOffset = 0;

int mode = 0;

int btn2, btn3, btn4;
bool d3High = false;
bool d3Low = false;
bool modeSetup = true;

// random glow variables
int bright[NUM_LEDS];
int directions[NUM_LEDS];

// alternating variables
bool flip = false;
int dir = 1;
int color1 = 135;
int color2 = 200;


void loop()
{
  // get potentiometer states
  sensorValueA0 = analogRead(A0);
  sensorValueA1 = analogRead(A1);

  // get button states
  btn2 = digitalRead(D2);
  btn3 = digitalRead(D3);
  btn4 = digitalRead(D4);
  Serial.print("Button D3: ");
  Serial.println(btn3);


  // recalculate brightness
  brightness = int((1.0 * 255 / 1023) * sensorValueA0);
  FastLED.setBrightness(brightness);

  // recalculate hues
  hueOffset = int((1.0 * 255 / 1023) * sensorValueA1);
  
  if(mode == 0)
    for(int i=0; i<150; i++)
      leds[i] = CHSV(hueOffset,255,BRIGHTNESS);
  else if(mode == 1)
    for(int i=0; i<150; i++)
      leds[i] = CHSV(i+hueOffset,255,BRIGHTNESS);
  else if(mode == 2){
      if(modeSetup){
        for(int i=0; i<150; i++)
          bright[i] = i;
        modeSetup = false;
      }
      for(int i=0; i<150; i++){
        leds[i] = CHSV(bright[i]+hueOffset, 255, BRIGHTNESS);
        bright[i]++;
        if(bright[i]==255){
          bright[i]=0;
        }
      }
      
  }
  else if(mode == 3){
    if(modeSetup){
      for(int i=0; i<150; i++){
        bright[i] = random(0,BRIGHTNESS);
    
        if(random(0,2)==0)
          directions[i] = 1;
        else
          directions[i] = -1;
      }
      modeSetup = false;
    }
    for(int i=0; i<150; i++){
        bright[i] += random(1,4) * directions[i];
        
        if(bright[i] > BRIGHTNESS)
          bright[i] = BRIGHTNESS;
        if(bright[i] < 50)
          bright[i]=50;
        if(bright[i]==50 || bright[i]==BRIGHTNESS)
          directions[i] *= -1;
          
        leds[i] = CHSV(hueOffset,120,bright[i]);
      }
      delay(15);
  }
  else if(mode == 4){
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
          leds[i] = CHSV(color1, SATURATION, bright[0]);
        }
        else{
          directions[i] = 0;
          leds[i] = CHSV(color2, SATURATION, bright[0]);
        }
      }
      flip = false;
    }
    else{
      for(int i=0; i<150; i++)
        if(directions[i] == 0)
          leds[i] = CHSV(color1+hueOffset, SATURATION, bright[0]);
        else
          leds[i] = CHSV(color2+hueOffset, SATURATION, bright[0]);
    }
    
    if(dir == 1)
      bright[0]+=4;
    else
      bright[0]-=4;
  
    if(bright[0]>=252)
      dir = 0;
    else if(bright[0]<=180){
      dir = 1;
      flip = true;
    }
  }

  // switch modes on button press
  if(!d3High && !d3Low && btn3 == 1)
    d3High = true;
  else if(d3High && !d3Low && btn3 == 0){
    d3Low = true;
    d3High = false;
  }
  else if(d3Low){
    if(mode == 0)
      mode = 1;
    else if(mode == 1)
      mode = 2;
    else if(mode == 2)
      mode = 3;
    else if(mode == 3)
      mode = 4;
    else if(mode == 4)
      mode = 0;
    
    d3Low = false;
    modeSetup = true;
  }
  
  
  
  FastLED.show();
  delay(5);
    
}

void setup() {
  delay(1000); // power-up safety delay
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4 , INPUT);
  Serial.begin(9600); 
  
  //set initial brightness from potentiometer
  sensorValueA0 = analogRead(A0);
  sensorValueA1 = analogRead(A1);
  brightness = int((1.0 * 255 / 1023) * sensorValueA0);
  hueOffset = int((1.0 * 255 / 1023) * sensorValueA1);
  FastLED.setBrightness(brightness);
  
  for(int i=0; i<150; i++){
    leds[i] = CHSV(hueOffset,SATURATION,brightness);
  }


  
  FastLED.show();
}

