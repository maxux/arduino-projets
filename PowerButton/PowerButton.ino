#include <printf.h>
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>
#include <require_cpp11.h>
#include <MFRC522.h>
#include <deprecated.h>
#include <MFRC522Extended.h>

#define RFID_SPI_SS   10
#define RFID_SPI_RST  9
#define NF_SPI_CE     6
#define NF_SPI_CSN    7

const byte card_on[] = {0x04, 0x9F, 0x2C, 0x1C, 0x10, 0x02, 0x89};
const byte card_off[] = {0x04, 0xEF, 0x63, 0x1C, 0x10, 0x02, 0x89};

const byte gateway_listen_addr[5] = {'M','X','E','T','H'};
char message[32];

MFRC522 rfid(RFID_SPI_SS, RFID_SPI_RST);
MFRC522::MIFARE_Key key;
RF24 radio(NF_SPI_CE, NF_SPI_CSN);

void setup() {
  Serial.begin(9600);
  printf_begin();
  
  Serial.println("[+] initializing rfid module");
  SPI.begin();
  rfid.PCD_Init(RFID_SPI_SS, RFID_SPI_RST);
  rfid.PCD_DumpVersionToSerial();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.print("[+] rfid decryption key:");
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();

  Serial.print("[+] initializing radio module:");
  radio.begin();
  radio.openWritingPipe(gateway_listen_addr);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  radio.printPrettyDetails();
}

void action_up() {
  Serial.print("[+] requesting power up: ");
  strcpy(message, "switch-powerup");
  
  if(radio.write(message, sizeof(message))) {
    Serial.println("ACK");
  } else {
    Serial.println("NO ACK");
  }
}

void action_down() {
  Serial.print("[+] requesting power down: ");
  strcpy(message, "switch-powerdown");
  
  if(radio.write(message, sizeof(message))) {
    Serial.println("ACK");
  } else {
    Serial.println("NO ACK");
  }
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void loop() {
  if(!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial())
    return;

  Serial.print("[+] card detected, uid: ");
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();
  
  // assume both card have same size
  if(rfid.uid.size == sizeof(card_on)) {
    if(memcmp(rfid.uid.uidByte, card_on, rfid.uid.size) == 0) {
      action_up();
    }

    if(memcmp(rfid.uid.uidByte, card_off, rfid.uid.size) == 0) {
      action_down();
    }
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(50);
}
