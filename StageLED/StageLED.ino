#include <FastLED.h>
#include <OctoWS2811.h>
#include <ArduinoUniqueID.h>
#include <QNEthernet.h>

typedef struct __attribute__ ((packed)) server_stats_t {
  uint64_t state;
  uint64_t old_frames;
  uint64_t frames;
  uint64_t fps;
  uint64_t time_last_frame;
  uint64_t time_current;

} server_stats_t;

using namespace qindesign::network;
EthernetUDP udp(32);

#define STRIPE_PIN   1
#define PER_LANE     (12 * 120)
#define NUM_LANES    2
#define TOTAL_LEDS   (NUM_LANES * PER_LANE)
#define BRIGHTNESS   80

CRGB leds[TOTAL_LEDS];
server_stats_t mainstats = {
  .state = 0,
  .old_frames = 0,
  .frames = 0,
  .fps = 0,
  .time_last_frame = 0,
  .time_current = 0,
};

void setup() {
  Serial.begin(9600);

  // Ethernet.setDHCPEnabled(false);
  Ethernet.setHostname("ledstage");
  Ethernet.begin();
  
  FastLED.setDither(0);
  FastLED.addLeds<NUM_LANES, WS2811, STRIPE_PIN, RGB>(leds, PER_LANE);
  FastLED.setMaxRefreshRate(30);

  pinMode(LED_BUILTIN, OUTPUT);

  // initializing light
  for(int i = 0; i < TOTAL_LEDS; i++)
    leds[i] = CHSV(200, 255, 50);
  
  #if 0
  for(int i = 0; i < TOTAL_LEDS; i++) {
    /*
    if(i > 800 && i < 1440) {
      leds[i] = CHSV(100, 255, BRIGHTNESS / 2);
      continue;
    }
    
    if(i > 2000) {
      leds[i] = CHSV(100, 255, BRIGHTNESS / 2);
      continue;
    }
    */

    leds[i] = CHSV(0, 0, 0);
  }
  #endif

  FastLED.show();

  udp.beginWithReuse(1111);
  mainstats.state = 1;

  Serial.println("[+] server listening");
}

void ethernet_status() {
  uint8_t mac[6];
  Ethernet.macAddress(mac);

  // Serial.printf("[+] mac address: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  bool state = Ethernet.linkState();
  int speed = Ethernet.linkSpeed();

  Serial.print("[+] link state: ");
  Serial.println(state);

  Serial.print("[+] link speed: ");
  Serial.print(speed);
  Serial.println(" Mbps");
}

char base = 0;
int received = 0;
int lastcheck = 0;

void loopx() {
  digitalWrite(LED_BUILTIN, HIGH);

  char value = base;
  
  for(int i = 0; i < TOTAL_LEDS; i++) {
    leds[i] = CHSV(value++, 255, BRIGHTNESS);
  }

  FastLED.show();
  digitalWrite(LED_BUILTIN, LOW);

  base += 1;
}

void loop() {
  int packetsize = udp.parsePacket();

  if(packetsize >= 0) {
    mainstats.state = 2; // frame received

    /*
    Serial.print("size: ");
    Serial.print(packetsize);
    Serial.print(", received: ");
    Serial.println(received);
    */

    digitalWrite(LED_BUILTIN, HIGH);

    uint8_t *data = udp.data();
    int led = 0;

    for(int i = 0; i < packetsize; i += 3) {
      // Serial.printf("setting led %d: %d,%d,%d\n", led, data[i], data[i + 1], data[i + 2]);
      leds[led] = CRGB(data[i], data[i + 1], data[i + 2]);
      led += 1;
    }

    FastLED.show();
    FastLED.delay(10);

    digitalWrite(LED_BUILTIN, LOW);

    mainstats.frames += 1;
    mainstats.time_last_frame = millis();
    received += 1;
  }

  if(millis() > lastcheck + 1000) {
    // ethernet_status();
    
    mainstats.time_current = millis();
    mainstats.fps = mainstats.frames - mainstats.old_frames;

    udp.send("10.241.0.51", 1111, (char *) &mainstats, sizeof(mainstats));

    lastcheck = millis();
    mainstats.old_frames = mainstats.frames;
  }
}
