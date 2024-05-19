int sensorPin = A3;
float R1 = 11000.0;
float R2 = 2200.0;

void setup() {
  Serial.begin(9600);
  Serial.println("init: magibux hardware monitoring");

  // 5v power hack for the other side of the board
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);

  pinMode(LED_BUILTIN, OUTPUT);
}

float readVoltage() {
   float sensorValue = analogRead(sensorPin);
   float vout = (sensorValue * 5.0) / 1024.0; 
   float vin = vout / (R2/(R1+R2));   
   return vin;
}

int pins[] = {A0, A1};
char *names[] = {"press0", "press1"};  //, "press2", "press3", "press4", "press5"}

void pressure(char *name, int pin) {
  int a = analogRead(pin);
  float bar = a / 1024.0;

  Serial.print(name);
  Serial.print(": ");
  Serial.print(bar * 10);
  Serial.println(" bar");
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  
  // float voltage = readVoltage();
  // Serial.print("voltage: ");
  // Serial.print(voltage);
  // Serial.println(" v");

  for(int i = 0; i < sizeof(pins) / sizeof(int); i++) {
    pressure(names[i], pins[i]);
  }

  delay(100);
}