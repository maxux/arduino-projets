#define LED_PWR  2
#define LED_NET  3
#define LED_GPS  4

void setup()  {
  pinMode(LED_PWR, OUTPUT);
  pinMode(LED_NET, OUTPUT);
  pinMode(LED_GPS, OUTPUT);
}


void loop()  {
  digitalWrite(LED_PWR, HIGH);
  delay(200);
  digitalWrite(LED_PWR, LOW);
  delay(200);

  digitalWrite(LED_NET, HIGH);
  delay(200);
  digitalWrite(LED_NET, LOW);
  delay(200);

  digitalWrite(LED_GPS, HIGH);
  delay(200);
  digitalWrite(LED_GPS, LOW);
  delay(200);
}
