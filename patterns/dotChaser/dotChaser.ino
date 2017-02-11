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


#define NUM_LEDS 150
#define DATA_PIN 8
#define BRIGHTNESS 180

CRGB leds[NUM_LEDS];
CHSV hues[NUM_LEDS];

int current=0;
int previous=0;
int dir = 1;
int randNum = random(0,150);
int randFound = 1;
int hue = 0;
int hue2 = 0;

void loop()
{
  if(randFound == 1){
    leds[randNum] = CRGB::Black;
    randNum = random(0,150);
    randFound = 0;
    dir = dir * -1;

    while(dir == 1 && randNum < current || randNum == current){
      if(current==148 || current==149){
        current = 147;
      }
      randNum = random(current+1,150);
    }
    while(dir == -1 && randNum > current || randNum == current){
      if(current==0 || current==1){
        current = 2;
      }
      randNum = random(0,current-1);
    }
    leds[randNum] = CRGB::White;
  }

  current = previous + dir;

  // stay in range
  if(current > 149){
    current = 0;
  }
  else if(current < 0){
    current = 149;
  }

  // detect collision
  if(current == randNum){
    randFound = 1;
  }

  // activate non-chaser lights
  for(int i=0; i<150; i++){
    hues[i].h += 1;
    if(hues[i].h > 255){
      hues[i].h = 0;
    }
    if(i != randNum && i != current){
      leds[i] = hues[i];
    }
    
    
  }
    
  leds[current] = CHSV(hue++,255,255);
  FastLED.show();
  FastLED.delay(10);

  previous = current;
    
}

void setup() {
  delay( 1000 ); // power-up safety delay
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  for(int i=0; i<150; i++){
    hues[i] = CHSV(hue2++,255,60);
  }
}

