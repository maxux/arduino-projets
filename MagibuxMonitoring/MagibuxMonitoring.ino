int vdc30_pin = A2;
int vdc15_pin = A3;

float vdc30_r1 = 110000.0;  // 30 VDC max.
float vdc30_r2 = 22000.0;

float vdc15_r1 = 200000.0;  // 15 VDC max.
float vdc15_r2 = 100000.0;

void setup() {
  Serial.begin(9600);
  Serial.println("init: magibux hardware monitoring");

  pinMode(LED_BUILTIN, OUTPUT);
}

float voltage_read(int source, float r1, float r2) {
   float value = analogRead(source);
   float vout = (value * 5.0) / 1024.0; 
   float vin = vout / (r2 / (r1 + r2));

   return vin;
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  
  float voltage30 = voltage_read(vdc30_pin, vdc30_r1, vdc30_r2);
  Serial.print("input 30v: ");
  Serial.print(voltage30);
  Serial.println(" v");

  float voltage15 = voltage_read(vdc15_pin, vdc15_r1, vdc15_r2);
  Serial.print("input 15v: ");
  Serial.print(voltage15);
  Serial.println(" v");

  digitalWrite(LED_BUILTIN, LOW);

  delay(100);
}