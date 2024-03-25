#include <w5100.h>
#include <printf.h>
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

#define SPI_CE   6
#define SPI_CSN  7

#define MONIETH_TYPE_DS18X20    0x03
#define MONIETH_TYPE_SWITCH     0x07

// routinx lan mac address
const byte srvaddr[] = {
  0x34, 0x97, 0xf6, 0x3f, 0x99, 0x97
};

// custom device mac address
// last byte will be set by deviceid
byte macaddr[] = {
  0xA2, 0x42, 0x42, 0x42, 0x42, 0x00
};

const uint8_t netdevid = 0x10;
Wiznet5100 w5100;
uint8_t netbuffer[512];
uint16_t netlength = 32;

struct moth_ds18_t {
    uint8_t deviceid[8];
    int32_t temperature;
};

struct moth_switch_t {
    uint8_t id;
};

struct moth_ds18_t mothdevice;
struct moth_switch_t mothswitch;

const byte gateway_listen_addr[5] = {'M','X','E','T','H'};
char message[32];

RF24 radio(SPI_CE, SPI_CSN);

void setup() {
  Serial.begin(9600);

  Serial.println("[+] initializing network");
  macaddr[5] = netdevid;

  // destination mac address
  memcpy(netbuffer, srvaddr, 6);

  // source mac address
  memcpy(netbuffer + 6, macaddr, 6);

  // ethernet type
  netbuffer[12] = 0x42;
  netbuffer[13] = 0xF0;

  // init network
  w5100.begin(macaddr);

  // initialize default device
  memset(&mothdevice.deviceid, 0x00, 8);
  mothdevice.deviceid[0] = 0x88;

  pinMode(LED_BUILTIN, OUTPUT);
  
  radio.begin();
  radio.openReadingPipe(1, gateway_listen_addr);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  Serial.println("[+] waiting wireless frames");
}

struct moth_ds18_t *frame_to_ds18(struct moth_ds18_t *target, char *message) {
  char *value = strchr(message, ' ');
  if(value == NULL) {
    return NULL;
  }

  memset(&mothdevice.deviceid, 0x00, 8);
  target->deviceid[0] = 0x88;

  int temp = atoi(value + 1);
  target->temperature = __builtin_bswap32(temp);

  // default to unknown device
  target->deviceid[7] = 0xFF;

  if(strncmp(message, "fridge-drink-top", 16) == 0) {
    target->deviceid[7] = 0x10;
  }

  if(strncmp(message, "fridge-drink-bottom", 19) == 0) {
    target->deviceid[7] = 0x20;
  }

  return target;
}

struct moth_ds18_t *frame_to_generic_ds18(struct moth_ds18_t *target, char *message) {
  char *value = strchr(message, ' ');
  if(value == NULL) {
    return NULL;
  }

  int temp = atoi(value + 1);
  target->temperature = __builtin_bswap32(temp);

  char parser[4];
  memset(parser, 0, sizeof(parser));

  strncpy(parser, message, 2);
  Serial.print(parser);
  target->deviceid[0] = strtoul(parser, NULL, 16);

  for(int i = 1; i < 8; i++) {
    strncpy(parser, message + 1 + (i * 2), 2);
    Serial.print(parser);
    target->deviceid[i] = strtoul(parser, NULL, 16);
  }

  Serial.print(": ");
  Serial.println(temp);

  return target;
}

struct moth_switch_t *frame_to_switch(struct moth_switch_t *target, char *message) {
  if(strncmp(message, "switch-powerdown", 16) == 0) {
    target->id = 1;
  }

  if(strncmp(message, "switch-powerup", 14) == 0) {
    target->id = 2;
  }
  
  return target;
}

void loop() {
  if(radio.available()) {
    radio.read(&message, sizeof(message));
    
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println(message);

    if(strncmp(message, "fridge-", 7) == 0) {
      if(!frame_to_ds18(&mothdevice, message)) {
        Serial.println("[-] invalid ds18 frame received");
        return;
      }
  
      memcpy(netbuffer + 15, &mothdevice, sizeof(struct moth_ds18_t));
      netbuffer[14] = MONIETH_TYPE_DS18X20;
      w5100.sendFrame(netbuffer, netlength);      
    }

    if(strncmp(message, "ss ", 3) == 0) {
      if(!frame_to_generic_ds18(&mothdevice, message + 3)) {
        Serial.println("[-] invalid generic ds18 frame received");
        return;
      }
  
      memcpy(netbuffer + 15, &mothdevice, sizeof(struct moth_ds18_t));
      netbuffer[14] = MONIETH_TYPE_DS18X20;
      w5100.sendFrame(netbuffer, netlength);      
    }

    if(strncmp(message, "switch-", 7) == 0) {
      if(!frame_to_switch(&mothswitch, message)) {
        Serial.println("[-] invalid switch frame received");
        return;
      }
  
      memcpy(netbuffer + 15, &mothswitch, sizeof(struct moth_switch_t));
      netbuffer[14] = MONIETH_TYPE_SWITCH;
      w5100.sendFrame(netbuffer, netlength);
    }
    
    digitalWrite(LED_BUILTIN, LOW);
  }

  delay(100);
}
