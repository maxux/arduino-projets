void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT_PULLUP);
}

void loop() {
  int but = digitalRead(2);
  Serial.println(but);

  delay(200);
}
