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
#define BRIGHTNESS 200

CRGB leds[NUM_LEDS];
CHSV hues[NUM_LEDS];

CHSV purple = CHSV(200, 255, 50);

int bright = 0;

int col = random(0,256);

void loop()
{
  for(int i=0; i<150; i++){
    leds[i] = CHSV(col, 255, bright);
  }

  bright+=3;
  if(bright>=185){
    bright=25;
    col = random(0,255);
  }
    
  FastLED.show();
  delay(8);
}

void setup() {
  delay( 1000 ); // power-up safety delay
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  for(int i=0; i<150; i++){
    //hues[i] = CHSV(hue2++,255,60);
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

