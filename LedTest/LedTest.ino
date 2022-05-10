#include <FastLED.h>

#define NUM_LEDS 8
#define DATA_PIN 3

// Define the array of leds
CRGB leds[NUM_LEDS];

void setup() {
    // Uncomment/edit one of the following lines for your leds arrangement.
    // ## Clockless types ##
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}

void loop() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB(20, 0, 0);
    FastLED.show();
    delay(100);
  }

  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB(0, 20, 0);
    FastLED.show();
    delay(100);
  }

  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB(0, 0, 20);
    FastLED.show();
    delay(100);
  }

  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB(0, 0, 0);
  }

  FastLED.show();
  delay(1000);
}
