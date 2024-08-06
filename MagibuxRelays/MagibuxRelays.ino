#define CHANNELS 8

char mainstate[CHANNELS] = {0, 0, 0, 0, 0, 0, 0, 0};
char pingroups[CHANNELS][2] = {
  {22, 24}, // channel 0
  {26, 28}, // channel 1
  {30, 32}, // channel 2
  {34, 36}, // channel 3
  {38, 40}, // channel 4
  {42, 44}, // channel 5
  {46, 48}, // channel 6
  {50, 52}, // channel 7
};
uint32_t lastcheck = 0;

void setup() {
  Serial.begin(9600);

  for(int i = 0; i < CHANNELS; i++) {
    pinMode(pingroups[i][0], OUTPUT);
    digitalWrite(pingroups[i][0], HIGH);

    pinMode(pingroups[i][1], OUTPUT);
    digitalWrite(pingroups[i][1], HIGH);
  }
}

void loop() {
  //
  // check for state update
  //
  if (Serial.available() > 0) {
    int ctrl = Serial.read();
    int id = Serial.read();
    int eof = Serial.read();

    if(ctrl != 'E' && ctrl != 'D') {
      Serial.println("error: invalid control code");
      return;
    }

    int port = id - 48;

    if(port < 0 || port > 7) {
      Serial.println("error: port out of range");
      return;
    }

    int mode = (ctrl == 'E') ? LOW : HIGH;
    mainstate[port] = !mode;
    
    digitalWrite(pingroups[port][0], mode);
    digitalWrite(pingroups[port][1], mode);

    // reset lastcheck so we force to dump new state
    lastcheck = 0;
  }

  //
  // dump current state
  //
  if(millis() > lastcheck + 1000) {
    Serial.print("state: ");

    for(int i = 0; i < 8; i++) {
      Serial.print(mainstate[i], DEC);
      Serial.print(" ");
    }

    Serial.println("");

    lastcheck = millis();
  }

  // millis overflow
  if(millis() < lastcheck)
    lastcheck = 0;

  delay(50);
}
