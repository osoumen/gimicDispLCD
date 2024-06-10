#include "tftDispSPI.h"
#include "EscSeqParser.h"
#include <Wire.h>

void recv(int len);
void req();

uint8_t button_input=0;
uint8_t button_ipol=0;
volatile bool i2c_reading;
uint8_t i2c_reading_address;
uint32_t updateTime;
uint32_t keepAliveTime;
bool  connected=false;

tftDispSPI tft;
EscSeqParser parser(&tft);

void setup() {
  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire1.setClock(400000);
  Wire1.begin(0x20);
  Wire1.onReceive(recv);
  Wire1.onRequest(req);
  
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif
}

void setup1() {
  pinMode(4, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (4, true);
  pinMode(6, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (6, true);
  pinMode(7, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (7, true);
  pinMode(14, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (14, true);
  pinMode(26, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (26, true);
  attachInterrupt(digitalPinToInterrupt(4), buttonChange1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(6), buttonChange2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(7), buttonChange3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(14), buttonChange4, CHANGE);
  attachInterrupt(digitalPinToInterrupt(26), buttonChange5, CHANGE);

  //Serial.begin(115200);
  Serial1.setPinout(0, 1);
  Serial1.setFIFOSize(4096);
  Serial1.begin(115200);

  tft.init();

  showStartupScreen();
}

void showStartupScreen() {
  tft.init_disp();
  tft.set_charsize(2);
  tft.move(7, 6);
  tft.puts_("G.I.M.I.C\x82\xAA\x90\xDA\x91\xB1\x82\xB3\x82\xEA\x82\xC4\x82\xA2\x82\xDC\x82\xB9\x82\xF1\x81\x42");
}

void loop() {
}

void loop1() {
  uint32_t updateStartTime = millis();
  while (Serial1.available() > 0) {
    parser.ParseByte(Serial1.read());
    connected = true;
  }
  // while (Serial.available() > 0) {
  //   parser.ParseByte(Serial.read());
  // }

  if ((millis() - updateTime) > 10) {
    if (tft.updateContent())
      updateTime = millis();
  }

  if ((millis() - keepAliveTime) > 3500) {
    if (connected) {
      showStartupScreen();
      connected = false;
    }
  }
  else {
    connected = true;
  }

  if (Serial1.overflow()) {
    Serial1.flush();
    tft.move(0,0);
    tft.set_attribute(17);
    tft.puts_("!!Overflowed!!");
  }
}

void buttonChange1() {
  int pinInput = button_input & ~(1 << 3);
  if (digitalRead(4) == LOW) {
    pinInput |= 1 << 3;
  }
  button_input = pinInput;
}

void buttonChange2() {
  int pinInput = button_input & ~(1 << 4);
  if (digitalRead(6) == LOW) {
    pinInput |= 1 << 4;
  }
  button_input = pinInput;
}

void buttonChange3() {
  int pinInput = button_input & ~(1 << 5);
  if (digitalRead(7) == LOW) {
    pinInput |= 1 << 5;
  }
  button_input = pinInput;
}

void buttonChange4() {
  int pinInput = button_input & ~(1 << 6);
  if (digitalRead(14) == LOW) {
    pinInput |= 1 << 6;
  }
  button_input = pinInput;
}

void buttonChange5() {
  int pinInput = button_input & ~(1 << 7);
  if (digitalRead(26) == LOW) {
    pinInput |= 1 << 7;
  }
  button_input = pinInput;
}

void recv(int len) {
  if (len == 1) {
    i2c_reading = true;
    i2c_reading_address = (uint8_t)Wire1.read();
  }
  else if (len == 2) {
    uint8_t address = (uint8_t)Wire1.read();
    uint8_t data = Wire1.read();
    if (address == 0x01) {
      button_ipol = data; // 読み取るだけ
    }
    else if (address == 0x09) {
#ifdef LED_BUILTIN
      digitalWrite(LED_BUILTIN, (data & 0x02)?LOW:HIGH);  // led g
#endif
    }
  }
}

void req() {
  if (i2c_reading) {
    i2c_reading = false;
    if (i2c_reading_address == 0x09) {
      Wire1.write(button_input);
      keepAliveTime = millis();
    }
    else {
      Wire1.write(0);
    }
  }
  else {
    Wire1.write(0);
  }
}
