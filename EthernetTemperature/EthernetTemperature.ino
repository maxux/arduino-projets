#include <OneWire.h>
#include <DallasTemperature.h>
#include <w5100.h>

#define ONE_WIRE_BUS 8
#define POWER_SWITCH_PIN 9

OneWire oneWire(ONE_WIRE_BUS);  
DallasTemperature sensors(&oneWire);
DeviceAddress sensaddr[8];

// routinx lan mac address
const byte srvaddr[] = {
  0x34, 0x97, 0xf6, 0x3f, 0x99, 0x97
};

// custom device mac address
// last byte will be set by deviceid
byte macaddr[] = {
  0xA2, 0x42, 0x42, 0x42, 0x42, 0x00
};

const uint8_t netdevid = 0x22;
Wiznet5100 w5100;
uint8_t netbuffer[512];
uint16_t netlength = 32;

struct moth_ds18_t {
    uint8_t deviceid[8];
    int32_t temperature;

} __attribute__((packed));

struct moth_ds18_t *dallas = (struct moth_ds18_t *)((uint8_t *) netbuffer + 15);

void setup() {
  Serial.begin(9600);
  Serial.println("[+] initializing temperature debug");

  Serial.println("[+] powering on");
  pinMode(POWER_SWITCH_PIN, OUTPUT);
  digitalWrite(POWER_SWITCH_PIN, HIGH);
  
  delay(1000);

  Serial.println("[+] initializing sensors");
  sensors.begin();

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
  w5100.begin(macaddr, srvaddr);
  
  Serial.print("[+] ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices found");

  for(int i = 0; i < sensors.getDeviceCount(); i++) {
    if(!sensors.getAddress(sensaddr[i], i)) {
      Serial.print("[-] could not retreive address of sensor: ");
      Serial.print(i, DEC);
      Serial.println("");
    }
  }
}



void loop() {
  uint8_t devices = sensors.getDeviceCount();

  Serial.print("[+] devices found: ");
  Serial.println(devices);

  sensors.requestTemperatures();

  for(uint8_t i = 0; i < devices; i++) {
    float temp;
    int32_t convert;

    char deviceid[32];
    memset(deviceid, 0, sizeof(deviceid));

    //
    // ds18b20
    //
    memset(netbuffer + 14, 0x00, sizeof(netbuffer) - 14);

    sprintf(deviceid, "%02x-", sensaddr[i][0]);
    for(int a = 1; a < 8; a++) {
      sprintf(deviceid + 3 + ((a - 1) * 2), "%02x", sensaddr[i][a]);
    }
    
    Serial.print("[+] device: ");
    Serial.print(deviceid);
    Serial.print(": ");

    temp = sensors.getTempC(sensaddr[i]);
    Serial.println(temp);

    convert = temp * 1000;
    dallas->temperature = __builtin_bswap32(convert);
    memcpy(dallas->deviceid, sensaddr[i], 8);

    Serial.println("[+] sending network frame");
    netbuffer[14] = 0x03;
    w5100.sendFrame(netbuffer, netlength);
  }

  delay(5000);
}
