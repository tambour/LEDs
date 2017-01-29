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

int color1 = 135;
int color2 = 200;

int bright = 140;
int dir = 1;
int flip = 0;
int colors[150];
int directions[150];
int hue=0;

void loop()
{

  for(int i=0; i<150; i++){
    colors[i] += random(1,8) * directions[i];
    if(colors[i] > BRIGHTNESS){
      colors[i] = BRIGHTNESS;
    }
    if(colors[i] < 0){
      colors[i]=0;
    }
    if(colors[i]==0 || colors[i]==BRIGHTNESS){
      directions[i] *= -1;
    }
    leds[i] = CHSV(hue,100,colors[i]);
  }

  hue++;
  if(hue==BRIGHTNESS){
    hue=0;
  }
  
  
  FastLED.show();
  delay(8);
    
}

void setup() {
  delay( 1000 ); // power-up safety delay
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  for(int i=0; i<150; i++){
    colors[i] = random(0,BRIGHTNESS);

    if(random(0,2)==0){
      directions[i] = 1;
    }
    else{
      directions[i] = -1;
    }
    
  }

  for(int i=0; i<150; i++){
    leds[i] = CRGB::White;
  }


  
  FastLED.show();
}

