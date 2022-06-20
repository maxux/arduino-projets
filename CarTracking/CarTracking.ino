#include <SoftwareSerial.h>

#define PIN_TX     10
#define PIN_RX     11
#define PIN_PWR    9

#define LED_RED    6
#define LED_GREEN  5
#define LED_BLUE   3

SoftwareSerial simSerial(PIN_TX, PIN_RX);

bool modem_initialized = false;
bool session_created = false;
bool reset_ongoing = false;
bool waitip = false;
long lastserial = 0;
long countreset = 0;

char buffer[256];
char gprmc[128];
char gpgga[128];
int state_gprmc = 0;
int state_gpgga = 0;
int bufidx = 0;

void led_set(int red, int green, int blue) {
  analogWrite(LED_RED, red);
  analogWrite(LED_GREEN, green);
  analogWrite(LED_BLUE, blue);
}

void reset_state() {
  serial_read_reset();
  state_gprmc = 0;
  state_gpgga = 0;
  modem_initialized = false;
  lastserial = millis() + 15000;
  countreset += 1;
}

void reset_board_hard() {
  asm volatile (" jmp 0");  
}

void reset_board() {
  Serial.println("[!] === RESET BOARD ===");
  for(int i = 0; i < 20; i++) {
    led_set(20, 0, 0);
    delay(50);
    led_set(0, 20, 0);
    delay(50);
  }
  
  reset_ongoing = true;
  
  reset_state();
  reset_modem();

  if(countreset > 15) {
    reset_board_hard();
  }
}

void reset_modem() {
  led_set(20, 0, 0);

  /*
  memset(buffer, 0, sizeof(buffer));
  modem_initialized = false;
  state_gprmc = 0;
  state_gpgga = 0;
  bufidx = 0;
  lastserial = millis();
  */

  Serial.println("[+] force power down modem");
  digitalWrite(PIN_PWR, HIGH);
  delay(3000);
  digitalWrite(PIN_PWR, LOW);

  delay(600);

  Serial.println("[+] starting up modem");
  digitalWrite(PIN_PWR, HIGH);
  delay(1000);
  digitalWrite(PIN_PWR, LOW);

  led_set(20, 20, 0);
}

void setup()   {
  // leds pin
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  // power pin
  pinMode(PIN_PWR, OUTPUT);

  // serial
  Serial.println("[+] initializing serial lines");
  Serial.begin(9600);
  simSerial.begin(9600);

  reset_modem();
}

void serial_read_reset() {
  memset(buffer, 0, sizeof(buffer));
  bufidx = 0;
}

bool debug_input(char *str, bool value) {
  Serial.print("[>>] ");
  Serial.println(str);
  return value;  
}

bool serial_wait_cip_ready() {
  char input[64];
  int id = 0;
  
  while(1) {
    if(simSerial.available()) {
      char reader = simSerial.read();
      input[id] = reader;
      input[id + 1] = '\0';
      id += 1;

      // > confirm input
      if(strcmp(input, "\r\n> ") == 0) {
        return debug_input(input, true);
      }

      // not ready to send data
      if(strcmp(input, "\r\nERROR\r\n") == 0) {
        return debug_input(input, false);
      }

      // something went wrong
      if(id > 32) {
        return debug_input(input, false);
      }
    }
  }
}

bool serial_wait_send_ok() {
  serial_read_reset();
  serial_read_line();
  
  while(!startswith(buffer, "SEND OK")) {
    if(startswith(buffer, "ERROR")) {
      Serial.println("--- ERROR ---");
      return false;
    }

    if(reset_ongoing) {
      return false;
    }
    
    serial_read_reset();
    serial_read_line();
  }

  serial_read_reset();
  return true;
}

bool serial_read() {
  if(millis() > lastserial + 10000) {
    Serial.println("<< no response from serial line for 10 seconds");
    reset_board();
    return true;
  }
    
  if(simSerial.available()) {
    lastserial = millis();
    
    buffer[bufidx] = simSerial.read();
      
    if(buffer[bufidx] == '\n') {
      buffer[bufidx + 1] = '\0';
      return true;
    }
      
    bufidx += 1;
  }

  return false;
}

void serial_read_line() {
  while(!serial_read()) {
    // waiting for line
  }

  Serial.print("[<<] ");
  Serial.print(buffer);
}

void serial_write_raw(const char *msg) {
  // dump line to serial monitor
  Serial.print("<< ");
  Serial.println(msg);

  // send line to modem
  simSerial.write(msg);
}

void serial_write(const char *msg) {
  serial_write_raw(msg);
  simSerial.write("\n");
}

void serial_write_wait(const char *msg) {
  serial_write(msg);
  serial_read_reset();
  
  while(1) {
    serial_read_line();

    if(reset_ongoing) {
      return;
    }

    // keep going if we just got empty line
    if(buffer[0] != '\n') {
      return;
    }
  }
}

void serial_read_wait_ok() {
  serial_read_reset();
  serial_read_line();
  
  while(!startswith(buffer, "OK")) {
    if(startswith(buffer, "ERROR")) {
      Serial.println("--- ERROR ---");
      return;
    }

    if(reset_ongoing) {
      return;
    }
    
    serial_read_reset();
    serial_read_line();
  }

  serial_read_reset();
}

void serial_write_wait_ok(const char *msg) {
  serial_write(msg);
  serial_read_wait_ok();
}



bool startswith(char *str, char *msg) {
  return (strncmp(str, msg, strlen(msg)) == 0);
}

void parse_line_uninitialized(char *str) {
  if(startswith(str, "SMS Ready")) {
    led_set(0, 20, 0);
    
    Serial.println("[+] modem initialized, ready");
    modem_initialized = true;

    // disable AT command echo
    serial_write_wait("ATE0");
    serial_read_line();
    serial_write_wait_ok("ATE0");

    serial_write_wait_ok("AT+CMEE=2");
    
    Serial.println("[+] connecting internet connection");
    serial_write_wait_ok("AT+CREG=1");
    serial_write_wait_ok("AT+CREG?");
    led_set(0, 0, 5);
    delay(1000);
    
    serial_write_wait_ok("AT+CSTT=\"CMNET\"");
    serial_write_wait_ok("AT+CIICR");
    serial_write_wait("AT+CIFSR");

    waitip = true;
  }
}

void parse_line_initialized(char *str) {
  char sender[64];
  
  if(waitip == true && strchr(str, '.')) {
    waitip = false;
    led_set(0, 20, 5);
    
    Serial.println("[+] opening udp transfert");
    serial_write_wait_ok("AT+CIPSTART=\"UDP\",\"home.maxux.net\",\"60942\"");
  }
  
  if(startswith(str, "CONNECT OK")) {
    Serial.println("[+] udp connection established, creating new session");

    if(session_created == false) {
      char *creator = "NEW SESSION\r\n";
      sprintf(sender, "AT+CIPSEND=%d", strlen(creator));
      serial_write(sender);
      delay(100);
      serial_write_raw(creator);
      delay(200);

      session_created = true;
      
    } else {
      Serial.println("[+] new session skipped, already created");
    }

    led_set(20, 10, 5);

    Serial.println("[+] starting gps module");
    serial_write_wait_ok("AT+CGPSPWR=1");
    serial_write_wait("AT+CGPSSTATUS?");
  }

  // waiting GPS signal to be ready
  if(startswith(str, "+CGPSSTATUS: Location Unknown") || startswith(str, "+CGPSSTATUS: Location Not") || startswith(str, "+CGPSSTATUS: Location 2D")) {
    led_set(0, 0, 5);
    delay(500);

    led_set(15, 10, 5);
    delay(500);
    
    serial_read_wait_ok();
    serial_write_wait("AT+CGPSSTATUS?");
  }

  if(startswith(str, "+CGPSSTATUS: Location 3D Fix")) {
    // led_set(LED_GPS, 0, 20, 0);
    
    // GPRMC | GPGGA
    serial_write_wait_ok("AT+CGPSOUT=34");
    
    // led_state = STATE_RDY;
  }

  if(startswith(str, "$GPGGA")) {
    strcpy(gpgga, str);
    state_gpgga = 1;
  }

  if(startswith(str, "$GPRMC")) {
    strcpy(gprmc, str);
    state_gprmc = 1;
  }

  if(state_gprmc == 1 && state_gpgga == 1) {
    Serial.println("[+] sending batch gps data");
    
    led_set(20, 10, 5);
    delay(50);
    led_set(20, 10, 0);
    
    int length = strlen(gprmc) + strlen(gpgga);
    
    sprintf(sender, "AT+CIPSEND=%d", length);
    serial_write(sender);
    if(serial_wait_cip_ready() == false) {
      reset_board();
    }
    
    serial_write_raw(gprmc);
    serial_write_raw(gpgga);
    serial_wait_send_ok();
    
    state_gprmc = 0;
    state_gpgga = 0;
  }
  
  if(startswith(str, "$GP")) {
    // sprintf(sender, "AT+CIPSEND=%d", strlen(str));
    // serial_write(sender);
    // serial_write_raw(str);
  } 
}

void parse_line(char *str) {
  Serial.print(">> ");
  Serial.print(buffer);

  if(startswith(str, "RDY")) {
    Serial.println("[?] modem seems to have reset, let's reset our state");
    reset_state();
  }

  if(modem_initialized) {
    parse_line_initialized(str);
    
  } else {
    parse_line_uninitialized(str);
  }

  serial_read_reset();
}

void loop() {
  if(serial_read()) {
    parse_line(buffer); 
  }

  if(Serial.available()) {
    while(Serial.available()) {
      simSerial.write(Serial.read());
    }
    simSerial.print("\n");
  }

  if(reset_ongoing) {
    reset_ongoing = false;
  }
}
