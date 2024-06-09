#include "tftDispSPI.h"
#include "EscSeqParser.h"
#include <Wire.h>

void recv(int len);
void req();

uint8_t button_input=0;
uint8_t button_ipol=0;
volatile bool i2c_reading;
uint8_t i2c_reading_address;
unsigned long myTime;

tftDispSPI tft;
EscSeqParser parser(&tft);

void setup() {
  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire1.setClock(400000);
  Wire1.begin(0x20);
  Wire1.onReceive(recv);
  Wire1.onRequest(req);

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
  attachInterrupt(digitalPinToInterrupt(4), buttonChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(6), buttonChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(7), buttonChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(14), buttonChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(26), buttonChange, CHANGE);

  //Serial.begin(115200);
  Serial1.setPinout(0, 1);
  Serial1.setFIFOSize(4096);
  Serial1.begin(115200);

  tft.init();
}

void loop() {
  while (Serial1.available() > 0) {
    parser.ParseByte(Serial1.read());
  }
  // while (Serial.available() > 0) {
  //   parser.ParseByte(Serial.read());
  // }

  if ((micros() - myTime) > 16666) {
    if (tft.updateContent())
      myTime = millis();
  }

  if (Serial1.overflow()) {
    Serial1.flush();
    tft.move(0,0);
    tft.set_attribute(17);
    tft.puts_("!!Overflowed!!");
  }
}

void buttonChange() {
  int pinInput = 0;
  if (digitalRead(4) == LOW) {
    pinInput |= 1 << 3;
  }
  if (digitalRead(6) == LOW) {
    pinInput |= 1 << 4;
  }
  if (digitalRead(7) == LOW) {
    pinInput |= 1 << 5;
  }
  if (digitalRead(14) == LOW) {
    pinInput |= 1 << 6;
  }
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
  }
}

void req() {
  if (i2c_reading) {
    i2c_reading = false;
    if (i2c_reading_address == 0x09) {
      Wire1.write(button_input);
    }
    else {
      Wire1.write(0);
    }
  }
  else {
    Wire1.write(0);
  }
}
