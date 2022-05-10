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
 
MFRC522 rfid(RFID_SPI_SS, RFID_SPI_RST);
MFRC522::MIFARE_Key key;
RF24 radio(NF_SPI_CE, NF_SPI_CSN);

const byte gateway_listen_addr[5] = {'M','X','E','T','H'};
char message[32];

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

  Serial.println("[+] initializing radio module");
  radio.begin();
  radio.openWritingPipe(gateway_listen_addr);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  radio.printPrettyDetails();
}
 
void loop() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if(!rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial())
    return;
   
  Serial.print(F("UID: "));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
