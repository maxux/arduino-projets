#include <printf.h>
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

#define SPI_CE   9
#define SPI_CSN  10

int waiting = 0;

const byte gateway_listen_addr[5] = {'M','X','E','T','H'};
char message[32];

RF24 radio(SPI_CE, SPI_CSN);

void setup() {
  Serial.begin(9600);
  printf_begin();

  pinMode(LED_BUILTIN, OUTPUT);
  
  radio.begin();
  radio.openReadingPipe(1, gateway_listen_addr);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  radio.printPrettyDetails();

  Serial.println("[+] waiting wireless frames");
}

void loop() {
  // Serial.print("[");
  // Serial.print(waiting);
  // Serial.println("] waiting frame");
  waiting += 1;
  
  if(radio.available()) {
    Serial.println("Frame received");
    radio.read(&message, sizeof(message));
    
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println(message);
    
    digitalWrite(LED_BUILTIN, LOW);
  }

  delay(100);
}
