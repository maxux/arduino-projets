#include <FastLED.h>
#include <ArduinoUniqueID.h>

#define STRIPE_PIN   8
#define CLONE_PIN    9
#define PER_LANE     120
#define NUM_LANES    4
#define TOTAL_LEDS   (NUM_LANES * PER_LANE)
#define BRIGHTNESS   200

CRGB leds[TOTAL_LEDS];

void setup() {
  Serial.begin(9600);
  
  FastLED.addLeds<WS2811, STRIPE_PIN, GRB>(leds, TOTAL_LEDS);
  FastLED.addLeds<WS2811, CLONE_PIN, GRB>(leds, TOTAL_LEDS);
  pinMode(LED_BUILTIN, OUTPUT);

  // full blackout
  for(int i = 0; i < TOTAL_LEDS; i++)
    leds[i] = CHSV(0, 0, 0);

  FastLED.show();

  Serial.println("core: magibux light controler ready");
}

char base = 0;
int index = 0;
int brightness = BRIGHTNESS;

void mode_spectre();
void mode_spectre_large();
void mode_full();
void mode_full_segmented();
void mode_blackout();
void mode_forward();
void mode_static_light();

int modeindex = 0;

void (*modes[])() = {
  mode_spectre,
  mode_spectre_large,
  mode_full,
  mode_full_segmented,
  mode_blackout,
  mode_forward,
  mode_static_light,
  NULL
};

// default mode
void (*runmode)() = mode_static_light;

void loop() {
  runmode();

  // checking for new mode
  if (Serial.available() > 0) {
    int newmode = Serial.read();

    if(newmode < '0' || newmode > '5') {
      Serial.println("[-] unknown mode requested");
      return;
    }

    int newindex = newmode - '0';
    Serial.print("[+] switching to mode index: ");
    Serial.println(newindex);
    runmode = modes[newindex];

    // trigger a single time blackout to reset
    mode_blackout();
  }
}

void mode_spectre() {
  digitalWrite(LED_BUILTIN, HIGH);

  char value = base;
  
  for(int i = 0; i < TOTAL_LEDS; i++) {
    leds[i] = CHSV(value++, 255, brightness);
  }

  FastLED.show();
  digitalWrite(LED_BUILTIN, LOW);

  base += 1;

  delay(5);
}

void mode_spectre_large() {
  digitalWrite(LED_BUILTIN, HIGH);

  char value = base;
  
  for(int i = 0; i < TOTAL_LEDS; i += 4) {
    for(int j = 0; j < 4; j++) {
      leds[i + j] = CHSV(value, 255, brightness);
    }

    value++;
  }

  FastLED.show();
  digitalWrite(LED_BUILTIN, LOW);

  base += 1;

  delay(50);
}

void mode_full() {
  digitalWrite(LED_BUILTIN, HIGH);

  for(int i = 0; i < TOTAL_LEDS; i++) {
    leds[i] = CRGB(brightness, brightness, brightness);
  }

  FastLED.show();
  digitalWrite(LED_BUILTIN, LOW);

  delay(50);
}

void mode_full_segmented() {
  digitalWrite(LED_BUILTIN, HIGH);

  for(int i = 0; i < TOTAL_LEDS; i += 24) {
    for(int j = 0; j < 8; j++) {
      leds[i + j] = CRGB(brightness, brightness, brightness);
    }
  }

  FastLED.show();
  digitalWrite(LED_BUILTIN, LOW);

  delay(50);
}

void mode_blackout() {
  digitalWrite(LED_BUILTIN, HIGH);

  for(int i = 0; i < TOTAL_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
  }

  FastLED.show();
  digitalWrite(LED_BUILTIN, LOW);

  delay(50);
}

void mode_forward() {
  digitalWrite(LED_BUILTIN, HIGH);

  leds[index] = CRGB(0, 0, 0);

  index += 1;
  if(index >= TOTAL_LEDS)
    index = 0;

  leds[index] = CRGB(brightness, brightness, brightness);

  FastLED.show();
  digitalWrite(LED_BUILTIN, LOW);

  delay(1);
}

void mode_static_light() {
  digitalWrite(LED_BUILTIN, HIGH);

  for(int index = 0; index < TOTAL_LEDS; index += 32)
    leds[index] = CRGB(brightness, brightness, brightness);

  FastLED.show();
  digitalWrite(LED_BUILTIN, LOW);

  delay(100);
}
