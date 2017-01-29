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
#define BRIGHTNESS 30

CRGB leds[NUM_LEDS];
CHSV hues[NUM_LEDS];

CHSV purple = CHSV(200, 255, 50);

int positions[] = {0,15,30,45,60,75,90,105,120,135};

int hue=0;


void loop()
{
  

  for (int i=0; i<10; i++){
    positions[i]++;
    if(positions[i] > 149){
      positions[i] = 0;
    }
    leds[positions[i]] = CRGB::White;
  }

  for(int i=0; i<150; i++){
    int inList = 0;
    for(int j=0; j<10; j++){
      if(positions[j] == i){
        inList = 1;
      }
    }
    if(inList == 0){
      leds[i] = CHSV(hue, 200, 70);
    }
  }
  hue++;

  if(hue==255){
    hue=0;
  }

    
  FastLED.show();
  FastLED.delay(30);
    
}

void setup() {
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  for(int i=0; i<150; i++){
    //hues[i] = CHSV(hue2++,255,60);
    leds[i] = purple;
  }
}

