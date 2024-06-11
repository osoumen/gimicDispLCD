#define BUTTON1_PIN_NO  4
#define BUTTON2_PIN_NO  6
#define BUTTON3_PIN_NO  7
#define BUTTON4_PIN_NO  14
#define BUTTON5_PIN_NO  26

#define ENABLE_MULTI_CORE 1

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
#ifdef ENABLE_MULTI_CORE
}

void setup1() {
#endif

  //Serial.begin(115200);
  pinMode(BUTTON1_PIN_NO, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (BUTTON1_PIN_NO, true);
  pinMode(BUTTON2_PIN_NO, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (BUTTON2_PIN_NO, true);
  pinMode(BUTTON3_PIN_NO, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (BUTTON3_PIN_NO, true);
  pinMode(BUTTON4_PIN_NO, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (BUTTON4_PIN_NO, true);
  pinMode(BUTTON5_PIN_NO, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (BUTTON5_PIN_NO, true);
  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN_NO), buttonChange1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN_NO), buttonChange2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON3_PIN_NO), buttonChange3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON4_PIN_NO), buttonChange4, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON5_PIN_NO), buttonChange5, CHANGE);
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
  tft.move(0, 0);
}

void loop() {
#ifdef ENABLE_MULTI_CORE
}

void loop1() {
#endif
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
  if (digitalRead(BUTTON1_PIN_NO) == LOW) {
    pinInput |= 1 << 3;
  }
  button_input = pinInput;
}

void buttonChange2() {
  int pinInput = button_input & ~(1 << 4);
  if (digitalRead(BUTTON2_PIN_NO) == LOW) {
    pinInput |= 1 << 4;
  }
  button_input = pinInput;
}

void buttonChange3() {
  int pinInput = button_input & ~(1 << 5);
  if (digitalRead(BUTTON3_PIN_NO) == LOW) {
    pinInput |= 1 << 5;
  }
  button_input = pinInput;
}

void buttonChange4() {
  int pinInput = button_input & ~(1 << 6);
  if (digitalRead(BUTTON4_PIN_NO) == LOW) {
    pinInput |= 1 << 6;
  }
  button_input = pinInput;
}

void buttonChange5() {
  int pinInput = button_input & ~(1 << 7);
  if (digitalRead(BUTTON5_PIN_NO) == LOW) {
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
