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
uint16_t netlength = 32;

#define REFRESH    60
#define SWITCH_PIN 7

void setup() {
  Serial.begin(9600);
  
  Serial.println("[+] initializing network");
  macaddr[5] = netdevid;
  w5100.begin(macaddr);

  // set initial switch off
  pinMode(SWITCH_PIN, OUTPUT);
  digitalWrite(SWITCH_PIN, HIGH);

  Serial.println("[+] sending initial frame");
  prepare_frame();
  w5100.sendFrame(netbuffer, netlength);
}

void opendoor() {
  Serial.println("[+] open door message receive");

  digitalWrite(SWITCH_PIN, LOW);
  delay(400);
  digitalWrite(SWITCH_PIN, HIGH);
  delay(800);
  digitalWrite(SWITCH_PIN, LOW);
  delay(400);
  digitalWrite(SWITCH_PIN, HIGH);
}

void prepare_frame() {
  memset(netbuffer, 0x00, sizeof(netbuffer));

  // destination mac address
  memcpy(netbuffer, srvaddr, 6);

  // source mac address
  memcpy(netbuffer + 6, macaddr, 6);

  // ethernet type
  netbuffer[12] = 0x42;
  netbuffer[13] = 0xF1;

  char *x = "HEY I'M ALIVE";
  memcpy(netbuffer + 14, x, strlen(x));
}

const char *doormsg = "OPEN DOOR";
unsigned long lastsend = 0;

void loop() {
  unsigned long time = micros();

  if(time > lastsend + (REFRESH * 1000000)) {
    Serial.println("[+] sending ping frame");

    lastsend = time;

    prepare_frame();
    w5100.sendFrame(netbuffer, netlength);
  }

  uint16_t readlen = w5100.readFrame(netbuffer, sizeof(netbuffer));
  if(readlen == 0)
    return;

  if(memcmp(netbuffer, macaddr, sizeof(macaddr)) == 0) {
    if(memcmp(netbuffer + 14, doormsg, strlen(doormsg)) == 0) {
      opendoor();
    }
  } 
}
