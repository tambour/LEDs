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
#define BRIGHTNESS 130

CRGB leds[NUM_LEDS];
CHSV hues[NUM_LEDS];

CHSV color1 = CHSV(205, 255, BRIGHTNESS);
CHSV color2 = CHSV(185, 255, BRIGHTNESS);
CHSV color3 = CHSV(170, 255, BRIGHTNESS);
CHSV color4 = CHSV(145, 255, BRIGHTNESS);
CHSV color5 = CHSV(170, 255, BRIGHTNESS);
CHSV color6 = CHSV(185, 255, BRIGHTNESS);

int group1[25];
int group2[25];
int group3[25];
int group4[25];
int group5[25];
int group6[25];

void loop()
{

  
  for (int i=0; i<25; i++){
    group1[i]++;
    group2[i]++;
    group3[i]++;
    group4[i]++;
    group5[i]++;
    group6[i]++;

    if(group1[i] > 149){
      group1[i] = 0;
    }
    if(group2[i] > 149){
      group2[i] = 0;
    }
    if(group3[i] > 149){
      group3[i] = 0;
    }
    if(group4[i] > 149){
      group4[i] = 0;
    }
    if(group5[i] > 149){
      group5[i] = 0;
    }
    if(group6[i] > 149){
      group6[i] = 0;
    }

    leds[group1[i]] = color1;
    leds[group2[i]] = color2;
    leds[group3[i]] = color3;
    leds[group4[i]] = color4;
    leds[group5[i]] = color5;
    leds[group6[i]] = color6;
  }

//  for(int i=0; i<150; i++){
//    leds[i].s++;
//
//    if(leds[i].s > 255){
//      leds[i].s = 0;
//    }
//  }
  
 

    
  FastLED.show();
  FastLED.delay(10);
    
}

void setup() {
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  for(int i=0; i<150; i++){
    if(i<25){
      group1[i] = i;
    }
    else if(i>=25 && i<50){
      group2[i-25] = i;
    }
    else if(i>=50 && i<75){
      group3[i-50] = i;
    }
    else if(i>=75 && i<100){
      group4[i-75] = i;
    }
    else if(i>=100 && i<125){
      group5[i-100] = i;
    }
    else{
      group6[i-125] = i;
    }
  }
}

