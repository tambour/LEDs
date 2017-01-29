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
#define BRIGHTNESS 255

CRGB leds[NUM_LEDS];
CHSV hues[NUM_LEDS];

CHSV purple = CHSV(200, 255, 50);


int index = 0;
int index2 = 75;

int hue=0;
int hue2=255;


void loop()
{
  for(int i=0; i<150; i++){
    if(i==index){
      leds[i] = CHSV(hue,255,255);
    }
    else if(i==index2){
      leds[i] = CHSV(hue2,255,255);
    }
    else{
      leds[i] = CHSV(0,0,40);
    }
  }

  index++;
  if(index>150){
    index=0;
  }

  index2++;
  if(index2>150){
    index2=0;
  }

  hue++;
  if(hue>255){
    hue=0;
  }

  hue2--;
  if(hue2<0){
    hue2=255;
  }

  FastLED.show();  
  delay(1);
    
}

void setup() {
  delay( 1000 ); // power-up safety delay
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  for(int i=0; i<150; i++){
    //hues[i] = CHSV(hue2++,255,60);
    leds[i] = purple;
  }
  FastLED.show();
}

