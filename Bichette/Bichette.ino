#define PUMP_SWITCH_PIN   8

void setup() {
  Serial.begin(9600);
  Serial.println("core: waiting for input");

  pinMode(PUMP_SWITCH_PIN, OUTPUT);
  digitalWrite(PUMP_SWITCH_PIN, HIGH);
}

int current_mode = 0;
unsigned long change_time = 0;

void pump_enable() {
  Serial.println("ENABNLING");
  digitalWrite(PUMP_SWITCH_PIN, LOW);

  current_mode = 1;
  change_time = millis();
}

void pump_disable() {
  Serial.println("DISABLING");
  digitalWrite(PUMP_SWITCH_PIN, HIGH);

  current_mode = 0;
  change_time = millis();
}

void loop() {
  unsigned long now = millis();

  // check for roll overs
  if(now < change_time) {
    // force disable, let's not risk days of pumping.
    // this will probably never happen
    // but we all know things which should never happen... happens.
    pump_disable();
  }

  unsigned long difftime = now - change_time;
  if(current_mode == 1) {
    // let's automatically stop pump after 10 seconds as
    // watchdog to avoid flooding the appartment (again)
    if(difftime > 10000) {
      Serial.println("WATCH TIMEOUT REACHED -- EMERGENCY STOP");
      pump_disable();
    }
  }

  // checking for new mode
  if (Serial.available() > 0) {
    int newmode = Serial.read();

    if(newmode < '0' || newmode > '2') {
      Serial.println("[-] unknown mode requested");
      return;
    }

    if(newmode == '1') {
      pump_enable();
    }

    if(newmode == '2') {
      pump_disable();
    }
  }
}