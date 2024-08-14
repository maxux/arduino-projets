void setup() {
  Serial.begin(9600);
  Serial.println("init: magibux pressure monitoring");

  pinMode(LED_BUILTIN, OUTPUT);
}

int pins[] = {A15, A14, A13, A12, A11, A4, A3, A2, A1, A0};
float pinbar[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

float pressure(int pin) {
  int a = analogRead(pin);
  float bar = a / 1024.0;

  return bar;
}

void loop() {
  //
  // fetch pressure
  //
  digitalWrite(LED_BUILTIN, HIGH);

  for(int i = 0; i < sizeof(pins) / sizeof(int); i++) {
    pinbar[i] = pressure(pins[i]);
  } 

  digitalWrite(LED_BUILTIN, LOW);

  //
  // dump values
  //
  Serial.print("pressure: ");
  for(int i = 0; i < sizeof(pins) / sizeof(int); i++) {
    Serial.print(pinbar[i] * 10);
    Serial.print(" ");
  }

  Serial.println("bar");
  delay(100);
}