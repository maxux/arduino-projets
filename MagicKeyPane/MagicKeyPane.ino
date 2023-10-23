#include <FastLED.h>

#define LED_PIN     7
#define TOTAL_LEDS  300
#define NUM_LEDS    300
#define BRIGHTNESS  200

CRGB leds[TOTAL_LEDS];

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, TOTAL_LEDS);
  pinMode(LED_BUILTIN, OUTPUT);

  // full blackout
  for(int i = 0; i < TOTAL_LEDS; i++)
    leds[i] = CHSV(0, 0, 0);

  // initial test
  for(int i = 0; i < NUM_LEDS; i++)
    leds[i] = CHSV(0, 255, 10);

  FastLED.show();
}

int cmin = 160;
int cmax = 250;

int base = cmin;
int bdir = 1;

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);

  int value = base;
  int cdir = 1;
  
  for(int i = 0; i < NUM_LEDS; i++) {
    value += cdir;
    
    if(value > cmax) {
      cdir = -1;
    }
    
    if(value < cmin) {
      cdir = 1;
    }

    leds[i] = CHSV(value, 255, BRIGHTNESS);
  }
  
  FastLED.show();

  base += bdir;
  
  if(base > cmax - 1) {
    bdir = -1;
  }

  if(base < cmin + 1) {
    bdir = 1;
  }

  delay(10);
}
