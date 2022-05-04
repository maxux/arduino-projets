#include <printf.h>
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

#define MOTOR_LEFT_1    7
#define MOTOR_LEFT_2    6
#define MOTOR_RIGHT_1   5
#define MOTOR_RIGHT_2   4

#define SPI_CE   9
#define SPI_CSN  10

const byte car_listen_addr[5] = {'C','A','R','x','A'};
char message[32];

RF24 radio(SPI_CE, SPI_CSN);

void setup() {
  Serial.begin(9600);
  
  pinMode(LED_BUILTIN, OUTPUT);
  
  pinMode(MOTOR_LEFT_1, OUTPUT);
  pinMode(MOTOR_LEFT_2, OUTPUT);
  pinMode(MOTOR_RIGHT_1, OUTPUT);
  pinMode(MOTOR_RIGHT_2, OUTPUT);

  radio.begin();
  radio.openReadingPipe(1, car_listen_addr);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void setMotor(int l1, int l2, int r1, int r2) {
  digitalWrite(LED_BUILTIN, HIGH);
  
  digitalWrite(MOTOR_LEFT_1, l1);
  digitalWrite(MOTOR_LEFT_2, l2);
  digitalWrite(MOTOR_RIGHT_1, r1);
  digitalWrite(MOTOR_RIGHT_2, r2);

  delay(20);
  digitalWrite(LED_BUILTIN, LOW);
}

void drive_stop() {
  setMotor(LOW, LOW, LOW, LOW);  
}

void drive_forward() {
  setMotor(HIGH, LOW, HIGH, LOW);
}

void drive_backward() {
  setMotor(LOW, HIGH, LOW, HIGH);
}

void turn_left() {
  setMotor(LOW, HIGH, HIGH, LOW);
}

void turn_right() {
  setMotor(HIGH, LOW, LOW, HIGH);
}

void remote_parse(char input) {
  Serial.println(input);
  
  if(input == 'Z')
    drive_forward();

   if(input == 'S')
    drive_backward();

   if(input == 'Q')
    turn_left();

   if(input == 'D')
    turn_right();

   if(input == 'X')
     drive_stop();
}

void loop() {
  if(radio.available()) {
    radio.read(&message, sizeof(message));
    
    digitalWrite(LED_BUILTIN, HIGH);
    
    if(strncmp(message, "CAR.", 4) == 0) {
      remote_parse(message[4]);
    }
    
    digitalWrite(LED_BUILTIN, LOW);
  }
}
