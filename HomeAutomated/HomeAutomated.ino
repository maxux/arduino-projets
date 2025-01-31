#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp>

#define PUMP_SWITCH_PIN   8

int current_mode = 0;
unsigned long change_time = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("core: initializing");

  pinMode(PUMP_SWITCH_PIN, OUTPUT);
  digitalWrite(PUMP_SWITCH_PIN, HIGH);

  Serial.print("core: infrared pin: ");
  Serial.println(IR_SEND_PIN);

  IrSender.begin();

  Serial.println("core: waiting for user input");
}

void pump_enable() {
  Serial.println("input: enable sparkling");
  digitalWrite(PUMP_SWITCH_PIN, LOW);

  current_mode = 1;
  change_time = millis();
}

void pump_disable() {
  Serial.println("input: disable sparkling");
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
      Serial.println("security: prevent flooding");
      pump_disable();
    }
  }

  // checking for new mode
  if (Serial.available() > 0) {
    int newmode = Serial.read();

    if(newmode < '0' || newmode > '6') {
      Serial.println("input: unknown mode");
      return;
    }

    if(newmode == '1') {
      pump_enable();
    }

    if(newmode == '2') {
      pump_disable();
    }

    if(newmode == '3') {
      Serial.println("input: sending projector power on");
      IrSender.sendNEC(0x32, 0x02, 0);  // Power ON
    }

    if(newmode == '4') {
      Serial.println("input: sending projector power off");
      IrSender.sendNEC(0x32, 0x2E, 0);  // Power OFF
      delay(200);
      IrSender.sendNEC(0x32, 0x2E, 0);  // Power OFF (confirmation)
    }

    if(newmode == '5') {
      Serial.println("input: sending projector hdmi-1");
      IrSender.sendNEC(0x32, 0x16, 0);  // HDMI1
    }

    if(newmode == '6') {
      Serial.println("input: sending projector hdmi-2");
      IrSender.sendNEC(0x32, 0x30, 0);  // HDMI2
    }
  }
}