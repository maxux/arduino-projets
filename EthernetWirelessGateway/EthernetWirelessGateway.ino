#include <printf.h>
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

#define SPI_CE   6
#define SPI_CSN  7

const byte gateway_listen_addr[5] = {'M','X','E','T','H'};
char message[32];

RF24 radio(SPI_CE, SPI_CSN);

void setup() {
  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);
  
  radio.begin();
  radio.openReadingPipe(1, gateway_listen_addr);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if(radio.available()) {
    radio.read(&message, sizeof(message));
    
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println(message);

    /*
    if(strncmp(message, "CAR.", 4) == 0) {
      Serial.println(message[4]);
    }
    */
    
    digitalWrite(LED_BUILTIN, LOW);
  }

  delay(100);
}
