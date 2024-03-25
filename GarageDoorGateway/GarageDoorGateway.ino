#include <w5100.h>

// custom device mac address
// last byte will be set by deviceid
byte macaddr[] = {
  0xA2, 0x42, 0x42, 0x42, 0x42, 0x00
};

const byte srvaddr[] = {
  0x34, 0x97, 0xf6, 0x3f, 0x99, 0x97
};

const uint8_t netdevid = 0xA0;
Wiznet5100 w5100;

uint8_t netbuffer[512];
uint16_t netlength = 48;

#define REFRESH    60
#define SWITCH_LEFT_PIN  6
#define SWITCH_RIGHT_PIN 7

void setup() {
  Serial.begin(9600);
  
  Serial.println("[+] initializing network");
  macaddr[5] = netdevid;
  w5100.begin(macaddr);

  // set initial switch off
  pinMode(SWITCH_LEFT_PIN, OUTPUT);
  digitalWrite(SWITCH_LEFT_PIN, HIGH);

  pinMode(SWITCH_RIGHT_PIN, OUTPUT);
  digitalWrite(SWITCH_RIGHT_PIN, HIGH);

  Serial.println("[+] sending initial frame");
  prepare_send_frame("HEY I'M ALIVE");
}

void opendoor(int pin) {
  // Serial.println("SIMULATE DOOR OPEN XXXX");
  // return;

  digitalWrite(pin, LOW);
  delay(700);

  digitalWrite(pin, HIGH);
  delay(300);

  digitalWrite(pin, LOW);
  delay(700);

  digitalWrite(pin, HIGH);
  delay(300);

  digitalWrite(pin, LOW);
  delay(1200);

  digitalWrite(pin, HIGH);
}

void prepare_frame(char *payload) {
  memset(netbuffer, 0x00, sizeof(netbuffer));

  // destination mac address
  memcpy(netbuffer, srvaddr, 6);

  // source mac address
  memcpy(netbuffer + 6, macaddr, 6);

  // ethernet type
  netbuffer[12] = 0x42;
  netbuffer[13] = 0xF1;

  memcpy(netbuffer + 14, payload, strlen(payload));
}

void prepare_send_frame(char *payload) {
  prepare_frame(payload);

  Serial.print("[+] sending network frame: ");
  Serial.println(payload);

  w5100.sendFrame(netbuffer, netlength);
}

const char *doormsgl = "OPEN DOOR LEFT";
const char *doormsgr = "OPEN DOOR RIGHT";
unsigned long lastsend = 0;

void loop() {
  unsigned long time = micros();

  // fix overflow
  if(time < lastsend)
    lastsend = 0;

  if(time > lastsend + (REFRESH * 1000000)) {
    Serial.println("[+] sending ping frame");

    lastsend = time;

    prepare_send_frame("HEY I'M ALIVE");
  }

  uint16_t readlen = w5100.readFrame(netbuffer, sizeof(netbuffer));
  if(readlen == 0)
    return;

  if(memcmp(netbuffer, macaddr, sizeof(macaddr)) == 0) {
    if(memcmp(netbuffer + 14, doormsgl, strlen(doormsgl)) == 0) {
      Serial.println("[+] open door (left) message received");
      prepare_send_frame("LEFT DOOR OPEN ACK");
      opendoor(SWITCH_LEFT_PIN);
    }

    if(memcmp(netbuffer + 14, doormsgr, strlen(doormsgr)) == 0) {
      Serial.println("[+] open door (right) message received");
      prepare_send_frame("RIGHT DOOR OPEN ACK");
      opendoor(SWITCH_RIGHT_PIN);
    }
  } 
}
