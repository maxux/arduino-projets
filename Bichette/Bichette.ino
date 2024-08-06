void setup() {
  Serial.begin(9600);
  Serial.println("core: waiting for input");

  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
}

void loop() {
  // checking for new mode
  if (Serial.available() > 0) {
    int newmode = Serial.read();

    if(newmode < '0' || newmode > '5') {
      Serial.println("[-] unknown mode requested");
      return;
    }

    if(newmode == '1') {
      Serial.println("ENABNLING");
      digitalWrite(8, LOW);
    }

    if(newmode == '2') {
      Serial.println("DISABLING");
      digitalWrite(8, HIGH);
    }
  }
}