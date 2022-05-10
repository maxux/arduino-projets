
int coolup = 0;
int cooldown = 0;
int lastup = LOW;
int lastdown = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
}

void action_up(int up) {
  if(coolup > millis() - 1000)
    return;
      
  if(lastup == LOW) {
    Serial.println("UP Pressed !");
  }

  lastup = up;
  coolup = millis(); 
}

void action_down(int down) {
  if(cooldown > millis() - 1000)
    return;

  if(lastdown == LOW) {
    Serial.println("DOWN Pressed !");
  }
  
  lastdown = down;
  cooldown = millis();
}

void loop() {
  int up = digitalRead(2);
  int down = digitalRead(3);

  if(lastup != up) {
    action_up(up);
  }

  if(lastdown != down) {
    action_down(down);
  }

  // Serial.print(up);
  // Serial.println(down);
  delay(50);
}
