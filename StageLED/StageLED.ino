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
#define SEG_PER_LANE 12
#define LED_PER_SEG  120
#define PER_LANE     (SEG_PER_LANE * LED_PER_SEG)
#define NUM_LANES    2
#define TOTAL_LEDS   (NUM_LANES * PER_LANE)

bool netstate = false;
bool linkinit = false;

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
  Serial.println("[+] initializing stage-led controler");

  // Ethernet.setDHCPEnabled(false);
  Ethernet.setHostname("ledstage");
  Ethernet.begin();
  Serial.println("[+] network initialized");
  
  FastLED.setDither(0);
  FastLED.addLeds<NUM_LANES, WS2811, STRIPE_PIN, RGB>(leds, PER_LANE);
  FastLED.setMaxRefreshRate(30);
  Serial.println("[+] led initialized");

  pinMode(LED_BUILTIN, OUTPUT);

  // initializing default light
  for(int i = 0; i < TOTAL_LEDS; i++)
    leds[i] = CHSV(200, 255, 10);

  FastLED.show();

  udp.beginWithReuse(1111);
  mainstats.state = 1;

  Serial.println("[+] server listening (port 1111)");
}

void ethernet_status() {
  uint8_t mac[6];
  Ethernet.macAddress(mac);

  Serial.printf("[+] mac address: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  bool state = Ethernet.linkState();
  int speed = Ethernet.linkSpeed();

  Serial.print("[+] link state: ");
  Serial.println(state);

  Serial.print("[+] link speed: ");
  Serial.print(speed);
  Serial.println(" Mbps");
}

int received = 0;
uint32_t lastcheck = 0;

void loop() {
  bool linkstate = Ethernet.linkState();
  if(!linkstate && !linkinit) {
    waiting_network();
  }

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

    uint8_t *data = (uint8_t *) udp.data();
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
    mainstats.time_current = millis();
    mainstats.fps = mainstats.frames - mainstats.old_frames;

    // FIXME
    udp.send("10.241.0.51", 1111, (uint8_t *) &mainstats, sizeof(mainstats));

    lastcheck = millis();
    mainstats.old_frames = mainstats.frames;
  }
}

//
// no network debug state
//
char base = 0;
uint32_t lastnetcheck = 0;

void waiting_network() {
  int stripes = NUM_LANES * SEG_PER_LANE;
  char value = base;

  for(int i = 0; i < TOTAL_LEDS; i++)
    leds[i] = CRGB(0, 0, 0);

  for(int i = 0; i < stripes; i++) {
    int index = i * LED_PER_SEG;

    for(int stripe = 0; stripe <= i; stripe++) {
      leds[index + stripe] = CHSV(value++, 255, 10);
    }
  }

  base += 1;

  FastLED.show();
  FastLED.delay(100);

  Serial.println("[+] sending debug colors, waiting for network");

  if(millis() > lastnetcheck + 10000) {
    lastnetcheck = millis();
    Serial.printf("[+] still waiting for network link [uptime: %u sec]\n", millis() / 1000);
  }
}