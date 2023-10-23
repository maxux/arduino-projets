void setup() {
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
}

void loop() {
  int val = analogRead(A0);
  digitalWrite(7, (val > 200) ? LOW : HIGH);

  val = analogRead(A1);
  digitalWrite(8, (val > 200) ? LOW : HIGH);

  val = analogRead(A2);
  digitalWrite(9, (val > 200) ? LOW : HIGH);
  
  delay(100);
}
