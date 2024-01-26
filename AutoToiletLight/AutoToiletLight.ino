#include <FastLED.h>

#define DST_TRIGGER_PIN 7
#define DST_ECHO_PIN 6
#define DST_POWER_PIN 12

#define PHOTO_PIN 2 // analog
#define BRIGHT_THRESHOLD 900
#define LOW_LIGHT 0
#define HIGH_LIGHT 1

#define STATUS_RED 10
#define STATUS_GREEN 9
#define STATUS_BLUE 11

#define LED_PIN 3
#define TOTAL_LEDS 300

#define FADE_DELAY 6
#define COOLDOWN_TIME 5000
#define DISTANCE_THRESHOLD 900
#define MEASURE_TIMEOUT 25000 // 25ms = ~8m @ 340m/s
#define SOUND_SPEED 340.0 / 1000

int leds_maxval = 0;
CRGB leds[TOTAL_LEDS];
CRGB statecolor = CRGB(CRGB::Blue);

void setup() {
  Serial.begin(9600);
  status_update();

  pinMode(DST_TRIGGER_PIN, OUTPUT);
  pinMode(DST_ECHO_PIN, INPUT);
  pinMode(DST_POWER_PIN, OUTPUT);  // hack to use a pin as power
  pinMode(LED_BUILTIN, OUTPUT);

  // distance sensor trigger must be low
  digitalWrite(DST_TRIGGER_PIN, LOW);

  digitalWrite(DST_POWER_PIN, HIGH);

  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, TOTAL_LEDS);

  // initial color
  for(int i = 0; i < TOTAL_LEDS; i++)
    leds[i] = CRGB(0, 0, 0);

  FastLED.show();
}

float distance() {
  digitalWrite(DST_TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(DST_TRIGGER_PIN, LOW);

  long measure = pulseIn(DST_ECHO_PIN, HIGH, MEASURE_TIMEOUT);

  float mm = measure / 2.0 * SOUND_SPEED;

  return mm;
}

int state = 0;

void fade_in(CRGB target, int intensity) {
  int maxval = (intensity == HIGH_LIGHT) ? 20 : 255;
  CRGB current = CRGB(CRGB::Black);

  for(int value = 0; value < maxval; value++) {
    current.r = scale8(target.r, value);
    current.g = scale8(target.g, value);
    current.b = scale8(target.b, value);

    for(int i = 0; i < TOTAL_LEDS; i++)
      leds[i] = current;
    
    FastLED.show();

    delay(FADE_DELAY);
  }

  leds_maxval = maxval;
}

void fade_out() {
  CRGB initial = leds[0];
  CRGB current = leds[0];

  for(int value = 255; value > 0; value -= 10) {
    current.r = scale8(initial.r, value);
    current.g = scale8(initial.g, value);
    current.b = scale8(initial.b, value);

    for(int i = 0; i < TOTAL_LEDS; i++)
      leds[i] = current;
    
    FastLED.show();

    delay(FADE_DELAY);
  }

  for(int i = 0; i < TOTAL_LEDS; i++)
    leds[i] = CRGB(CRGB::Black);

  FastLED.show();
}

int intensity_raw() {
  int value = analogRead(PHOTO_PIN);
  return value;
}

int intensity() {
  int value = intensity_raw();
  return (value < BRIGHT_THRESHOLD) ? LOW_LIGHT : HIGH_LIGHT;
}

void status_light(CRGB color) {
  statecolor = color;
  status_update();
}

void status_update() {
  int intens = intensity();
  int divider = 1;

  if(intens == HIGH_LIGHT)
    divider = 18;

  analogWrite(STATUS_RED, statecolor.r / divider);
  analogWrite(STATUS_GREEN, statecolor.g / divider);
  analogWrite(STATUS_BLUE, statecolor.b / divider);
}

void loopx() {
  float mm = distance();
  int intens = intensity_raw();

  Serial.print("[+] distance: ");
  Serial.println(mm);
}

void loop() {
  status_update();

  float mm = distance();

  // ignore initializing value
  if(mm < 1) {
    delay(100);
    return;
  }

  int intens = intensity_raw();
  Serial.print("[+] distance: ");
  Serial.print(mm);
  Serial.print(", intensity: ");
  Serial.println(intens, DEC);

  if(state == 0 && mm < DISTANCE_THRESHOLD) {
    int intens = intensity();

    Serial.print("[+] presence detected, switching on, intensity: ");
    Serial.println((intens == LOW_LIGHT) ? "low" : "high");

    status_light(CRGB(CRGB::DarkOrange));
    fade_in(CRGB(Tungsten40W), intens);

    status_light(CRGB(CRGB::Red));
    state = 1;
  }
  
  if(state == 1 && mm > DISTANCE_THRESHOLD) {
    Serial.println("[+] presence not detected anymore, checking average...");
    // status_light(STATUS_BLUE);

    unsigned long initial = micros();
    unsigned long timeout = initial + (COOLDOWN_TIME * 1000ul);
    double detected = 0;
    double checked = 0;

    while(micros() < timeout) {
      float mm = distance();

      Serial.print("[+] staging distance: ");
      Serial.print(mm);

      if(mm > DISTANCE_THRESHOLD) {
        Serial.print(" - adding");
        detected += 1;
      }
      
      Serial.println("");
      checked += 1;

      delay(50);
    }

    Serial.print("[+] checking average to avoid glitch: ");
    Serial.print(detected);
    Serial.print(" / ");
    Serial.println(checked);

    if((detected / checked) < 0.95) {
      Serial.println("[+] abort, presence still there");
      status_light(CRGB(CRGB::Red));
      return;
    }
    
    Serial.println("[+] powering off...");
    fade_out();
    status_light(CRGB(CRGB::Green));

    state = 0;
  }

  delay(60);
}