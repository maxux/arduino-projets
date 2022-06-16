#include <OneWire.h>
#include <DallasTemperature.h>
#include <printf.h>
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

#define ONE_WIRE_BUS 2

#define SPI_CE   9
#define SPI_CSN  10

const byte temp_listen_addr[5] = {'M','X','E','T','H'};

RF24 radio(SPI_CE, SPI_CSN);
OneWire oneWire(ONE_WIRE_BUS);  
DallasTemperature sensors(&oneWire);

void setup(void) {
  Serial.begin(9600);
  Serial.println("[+] initializing generic wireless sensors module");
  printf_begin();
  
  sensors.begin();
  
  pinMode(LED_BUILTIN, OUTPUT);

  radio.begin();
  radio.openWritingPipe(temp_listen_addr);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  // radio.printPrettyDetails();
}

void sendstr(char *str) {
  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.print("[+] sending frame: ");
  Serial.print(str);
  
  if(radio.write(str, strlen(str))) {
    Serial.println(" - ACK");
  } else {
    Serial.println(" - NO ACK");
  }
  
  digitalWrite(LED_BUILTIN, LOW);
}

void loop(void) {
  char frame[64];
  uint8_t address[8];
  char deviceid[32];

  memset(deviceid, 0, sizeof(deviceid));

  uint8_t devices = sensors.getDeviceCount();

  Serial.print("[+] devices found: ");
  Serial.println(devices);
  
  sensors.requestTemperatures();

  for(uint8_t i = 0; i < devices; i++) {
    sensors.getAddress(address, i);

    sprintf(deviceid, "%02x-", address[0]);
    for(int a = 1; a < 8; a++) {
      sprintf(deviceid + 3 + ((a - 1) * 2), "%02x", address[a]);
    }
    
    Serial.print("[+] device: ");
    Serial.print(deviceid);
    Serial.print(": ");
    
    float value = sensors.getTempCByIndex(i);
    Serial.println(value);

    int valuei = value * 1000;
    
    sprintf(frame, "ss %s %d", deviceid, valuei);
    sendstr(frame);
  }

  delay(5000);
}
