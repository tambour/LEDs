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
#define DATA_PIN 12
#define BRIGHTNESS 100

CRGB leds[NUM_LEDS];
CHSV hues[NUM_LEDS];

CHSV purple = CHSV(200, 255, 50);

int color1 = 135;
int color2 = 200;

int bright = 140;
int dir = 1;
int flip = 0;
int colors[150];

void loop()
{

  if(flip==32){
    for(int i=0; i<150; i++){
      if(colors[i] == 0){
        colors[i] = 1;
        leds[i] = CHSV(color1, bright, BRIGHTNESS);
      }
      else{
        colors[i] = 0;
        leds[i] = CHSV(color2, bright, BRIGHTNESS);
      }
    }
    flip = 0;
  }
  else{
    for(int i=0; i<150; i++){
      if(colors[i] ==0){
        leds[i] = CHSV(color1, bright, BRIGHTNESS);
      }
      else{
        leds[i] = CHSV(color2, bright, BRIGHTNESS);

      }
    }
  }
  flip = flip + 1;

  if(dir == 1){
    bright+=4;
  }
  else{
    bright-=4;
  }

  if(bright>=252){
    dir = 0;
  }
  if(bright<=100){
    dir = 1;
  }

  
  
  
  FastLED.show();
  delay(4);
    
}

void setup() {
  delay( 1000 ); // power-up safety delay
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  int up = 0;
  for(int i=0; i<150; i++){
    if(up==0){
      colors[i] = 0;
      colors[i+1] = 0;
      up=1;
    }
    else{
      colors[i] = 1;
      colors[i+1] = 1;
      up=0;
    }
    i++;
    
  }

  for(int i=0; i<150; i++){
    leds[i] = CRGB::Black;
  }


  
  FastLED.show();
}

