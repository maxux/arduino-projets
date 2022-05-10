#define FASTLED_ALLOW_INTERRUPTS 1
#include <FastLED.h>
#include <SoftwareSerial.h>

#define PIN_TX     10
#define PIN_RX     11
#define PIN_PWR    9

#define LEDS_LEN   8
#define LEDS_DATA  3

#define LED_PWR    0
#define LED_NET    1
#define LED_GPS    2
// #define LED_SIG    3

SoftwareSerial simSerial(PIN_TX, PIN_RX);
CRGB leds[LEDS_LEN];

bool modem_initialized = false;
char buffer[256];
char gprmc[128];
char gpgga[128];
int state_gprmc = 0;
int state_gpgga = 0;
int bufidx = 0;

void led_blink_wait(int led, int ms, int timewait) {
  int loop = ms / (timewait * 2);
  
  for(int i = 0; i < loop; i++) {
    leds[led].setRGB(0, 0, 20);
    // FastLED.show();
    delay(timewait);
    
    leds[led].setRGB(0, 0, 0);
    // FastLED.show();
    delay(timewait);
  }
}

void led_quick_blink(int led) {
  leds[led].setRGB(0, 0, 20);
  // FastLED.show();
  delay(50);
    
  leds[led].setRGB(0, 0, 0);
  // FastLED.show();
}

void led_set_boot() {
  leds[LED_PWR].setRGB(20, 0, 0);
  leds[LED_NET].setRGB(20, 0, 0);
  leds[LED_GPS].setRGB(20, 0, 0);
  FastLED.show();
}

void led_set(int led, int red, int green, int blue) {
  leds[led].setRGB(red, green, blue);
  FastLED.show();
}

void setup()   {
  FastLED.addLeds<NEOPIXEL, LEDS_DATA>(leds, LEDS_LEN);
  led_set_boot();
  
  // power pin
  pinMode(PIN_PWR, OUTPUT);
  
  Serial.begin(9600);
  memset(buffer, 0, sizeof(buffer));

  Serial.println("[+] opening modem serial");
  simSerial.begin(9600);

  Serial.println("[+] force power down modem");
  digitalWrite(PIN_PWR, HIGH);
  led_blink_wait(LED_PWR, 3000, 200);
  digitalWrite(PIN_PWR, LOW);

  led_blink_wait(LED_PWR, 600, 100);

  Serial.println("[+] starting up modem");
  digitalWrite(PIN_PWR, HIGH);
  led_blink_wait(LED_PWR, 1000, 50);
  digitalWrite(PIN_PWR, LOW);

  led_set(LED_PWR, 0, 20, 0);
  // led_state = STATE_NET;
}



void serial_read_reset() {
  memset(buffer, 0, sizeof(buffer));
  bufidx = 0;
}

bool serial_read() {
  // led_status_update();
  
  if(simSerial.available()) {
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
    Serial.println("[+] modem initialized, ready");
    modem_initialized = true;

    // disable AT command echo
    serial_write_wait("ATE0");
    serial_read_line();
    serial_write_wait_ok("ATE0");
    
    Serial.println("[+] connecting internet connection");
    serial_write_wait_ok("AT+CREG=1");
    serial_write_wait_ok("AT+CREG?");
    led_set(LED_NET, 0, 0, 20);
    led_blink_wait(LED_NET, 1000, 100);
    
    serial_write_wait_ok("AT+CSTT=\"CMNET\"");
    serial_write_wait_ok("AT+CIICR");
    serial_write_wait("AT+CIFSR");
  }
}

void parse_line_initialized(char *str) {
  char sender[64];

  if(startswith(str, "10.")) {
    Serial.println("[+] opening udp transfert");
    serial_write_wait_ok("AT+CIPSTART=\"UDP\",\"home.maxux.net\",\"60942\"");
  }
  
  if(startswith(str, "CONNECT OK")) {
    Serial.println("[+] udp connection established, creating new session");
    
    char *creator = "NEW SESSION\r\n";
    sprintf(sender, "AT+CIPSEND=%d", strlen(creator));
    serial_write(sender);
    delay(100);
    serial_write_raw(creator);
    delay(200);

    led_set(LED_NET, 0, 20, 0);
    led_set(LED_GPS, 0, 0, 20);
    // led_state = STATE_GPS;

    Serial.println("[+] starting gps module");
    serial_write_wait_ok("AT+CGPSPWR=1");
    serial_write_wait("AT+CGPSSTATUS?");
  }

  // waiting GPS signal to be ready
  if(startswith(str, "+CGPSSTATUS: Location Unknown") || startswith(str, "+CGPSSTATUS: Location Not") || startswith(str, "+CGPSSTATUS: Location 2D")) {
    led_blink_wait(LED_GPS, 1000, 100);
    serial_read_wait_ok();
    serial_write_wait("AT+CGPSSTATUS?");
  }

  if(startswith(str, "+CGPSSTATUS: Location 3D Fix")) {
    led_set(LED_GPS, 0, 20, 0);
    
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
    Serial.println("Sending batch GPS data");
    int length = strlen(gprmc) + strlen(gpgga);
    
    sprintf(sender, "AT+CIPSEND=%d", length);
    serial_write(sender);
    serial_write_raw(gprmc);
    serial_write_raw(gpgga);
    
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

  if(modem_initialized) {
    parse_line_initialized(str);
    
  } else {
    parse_line_uninitialized(str);
  }

  serial_read_reset();
}

#if 0
void led_status_update() {
  if(led_time > millis() - 100 || led_state == STATE_RDY) {
    return;
  }
  
  led_time = millis();  
  led_level = led_level ? 20 : 0;
  
  if(led_state == STATE_NET) {
    led_set(LED_NET, 0, 0, led_level);
  }
  
  if(led_state == STATE_GPS) {
    led_set(LED_GPS, 0, 0, led_level);
  }
}
#endif

void loop() {
  if(serial_read()) {
    // led_quick_blink(LED_SIG);
    parse_line(buffer);
  }

  if(Serial.available()) {
    while(Serial.available()) {
      simSerial.write(Serial.read());
    }
    simSerial.print("\n");
  }
}
