#include <printf.h>
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

char *keybuf = "CAR.X.";

#define SPI_CE   9
#define SPI_CSN  10

// const byte car_listen_addr[5] = {'C','A','R','x','A'};
const byte car_listen_addr[5] = {'M','X','E','T','H'};
char message[32];

RF24 radio(SPI_CE, SPI_CSN);

void setup() {
  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);

  radio.begin();
  radio.openWritingPipe(car_listen_addr);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  bool rslt;
  
  if(Serial.available() > 0) {
    int incomingByte = Serial.read();
    
    if(incomingByte >= 'A') {
      keybuf[4] = incomingByte;
    }
  }

  digitalWrite(LED_BUILTIN, HIGH);

  strcpy(message, keybuf);

  Serial.print("Sending: ");
  Serial.print(message);
  
  if(radio.write(&message, strlen(message))) {
    Serial.println(" - ACK");
  } else {
    Serial.println(" - NOK");
  }
  
  digitalWrite(LED_BUILTIN, LOW);
  
  delay(100);
}
