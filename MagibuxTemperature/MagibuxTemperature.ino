#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);  
DallasTemperature sensors(&oneWire);

void setup(void) {
  Serial.begin(9600);
  Serial.println("[+] initializing magibux temperature sensors");
  
  sensors.begin();
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop(void) {
  uint8_t address[8];
  char deviceid[32];

  memset(deviceid, 0, sizeof(deviceid));

  uint8_t devices = sensors.getDeviceCount();

  Serial.print("sensors: ");
  Serial.println(devices);
  
  sensors.requestTemperatures();

  for(uint8_t i = 0; i < devices; i++) {
    sensors.getAddress(address, i);

    sprintf(deviceid, "%02x-", address[0]);
    for(int a = 1; a < 8; a++) {
      sprintf(deviceid + 3 + ((a - 1) * 2), "%02x", address[a]);
    }
    
    Serial.print(deviceid);
    Serial.print(": ");
    
    float value = sensors.getTempCByIndex(i);
    Serial.println(value);
  }

  delay(5000);
}
