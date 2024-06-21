#define BUTTON1_PIN_NO  4
#define BUTTON2_PIN_NO  6
#define BUTTON3_PIN_NO  7
#define BUTTON4_PIN_NO  14
#define BUTTON5_PIN_NO  26

#define LCD_BACKLIGHT_PIN 13

#define ENABLE_MULTI_CORE 1
// #define ENABLE_SERIAL_OUT 1
#define TOUCH_THRESHOLD 300

#include "tftDispSPI.h"
#include "EscSeqParser.h"
#include <Wire.h>
#include <EEPROM.h>

void recv(int len);
void req();

uint8_t button_input=0;
uint8_t button_ipol=0;
volatile bool i2c_reading;
uint8_t i2c_reading_address;
uint32_t tpUpdateTime;
uint32_t lastTpDownTime;
uint16_t tp_X, tp_Y;
bool tp_On=false;
bool backLightOn=false;

tftDispSPI tft;
EscSeqParser parser(&tft);

enum MouseButtons {
	MOUSE_BTN1			= (1 << 0),
	MOUSE_BTN2			= (1 << 1),
	MOUSE_BTN3			= (1 << 2),
	MOUSE_BTN_RELEASE	= (1 << 3),
	MOUSE_BTN1_REL		= (MOUSE_BTN1 | MOUSE_BTN_RELEASE),
	MOUSE_BTN_DBL_CLK	= (1 << 4),
  IS_TP             = (1 << 5)
};

void doTPCallibration() {
  int onCount = 0;
  for (int i=0; i<5; ++i) {
    if(digitalRead(BUTTON1_PIN_NO) == LOW) {
      ++onCount;
    }
  }
  EEPROM.begin(256);
  uint8_t calData[10];
  if (onCount >= 3) {
    tft.touch_calibrate((uint16_t*)calData);
    // キャリブレーション値をFlash領域に保存
    for (int i=0; i<10; ++i) {
      EEPROM.write(i, calData[i]);
    }
    EEPROM.commit();
  }
  else {
    int initialCnt = 0;
    // キャリブレーション値をFlash領域から読み出す
    for (int i=0; i<10; ++i) {
      calData[i] = EEPROM.read(i);
      if (calData[i] == 0xff) {
        ++initialCnt;
      }
    }
    // 初期状態ならキャリブレーションを実行する
    if (initialCnt == 10) {
      tft.touch_calibrate((uint16_t*)calData);
      for (int i=0; i<10; ++i) {
        EEPROM.write(i, calData[i]);
      }
      EEPROM.commit();
    }
  }
  EEPROM.end();
  tft.set_calibrate((uint16_t*)calData);
}

void setup() {
  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire1.setClock(400000);
  Wire1.begin(0x20);
  Wire1.onReceive(recv);
  Wire1.onRequest(req);
  
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

#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif
#ifdef ENABLE_MULTI_CORE
}

void setup1() {
#endif

#ifdef ENABLE_SERIAL_OUT
  Serial.begin(115200);
#endif
  Serial1.setPinout(0, 1);
  Serial1.setFIFOSize(4096);
  Serial1.begin(115200);

  tft.init();

  doTPCallibration();

#ifdef LCD_BACKLIGHT_PIN
  digitalWrite(LCD_BACKLIGHT_PIN, LOW);
#endif
  tft.updateContent();
}

void loop() {
#ifdef ENABLE_MULTI_CORE
  tft.lcdPushProc();
}

void loop1() {
#endif
  uint32_t updateStartTime = millis();
  bool draw = false;
  while (Serial1.available() > 0) {
    parser.ParseByte(Serial1.read());
    draw = true;
  }
  // while (Serial.available() > 0) {
  //   parser.ParseByte(Serial.read());
  // }

  if ((millis() - tpUpdateTime) > 10) {
    tpUpdateTime = millis();
    if (backLightOn) {
      draw = TouchPanelTask(draw);
    }
  }

  if (Serial1.overflow()) {
    Serial1.flush();
    tft.move(0,0);
    tft.set_attribute(17);
    tft.puts_("!!Overflowed!!");
    draw = true;
  }
  if (draw) {
    tft.updateContent();
#ifdef ENABLE_SERIAL_OUT
    Serial.println(millis() - updateStartTime);
#endif
  }
}

bool TouchPanelTask(bool draw)
{
  uint16_t x,y;
  bool isOn = tft.getTouch(&x, &y, TOUCH_THRESHOLD);
  if (isOn == false && tp_On == true) {
    Serial1.printf("\x1b[<%d;%d;%dM", MOUSE_BTN1_REL | IS_TP, tp_X, tp_Y);
    // Serial.printf("\x1b[<%d;%d;%dM\n", MOUSE_BTN1_REL | IS_TP, tp_X, tp_Y);
    tp_On = false;
    tft.hideCursorPointer();
    draw = true;
  }
  else if (isOn) {
    int btnState = IS_TP;
    if (tp_On == false) {
      btnState |= MOUSE_BTN1;
      if ((millis() - lastTpDownTime) < 500) {   // TODO: ダブルタップ時間を調整
        int distance_x = (tp_X - x);
        int distance_y = (tp_Y - y);
        if ((distance_x * distance_x + distance_y * distance_y) < 16*16) {
          btnState |= MOUSE_BTN_DBL_CLK;
        }
      }
      tp_X = tp_Y = 9999;
      tp_On = true;
      lastTpDownTime = millis();
    }
    if ((x != tp_X) || (y != tp_Y)) {
      Serial1.printf("\x1b[<%d;%d;%dM", btnState, x, y);
      // Serial.printf("\x1b[<%d;%d;%dM\n", btnState, x, y);
      tp_X = x;
      tp_Y = y;
    }
    tft.setCursorPointer(x, y);
    draw = true;
  }
  return draw;
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
      if ((data & 0x01) == 0) {
        // リセット要求に応える
        watchdog_enable(1, 1);
        while(1);
      }
#ifdef LED_BUILTIN
      digitalWrite(LED_BUILTIN, (data & 0x02)?LOW:HIGH);  // led g
#endif
#ifdef LCD_BACKLIGHT_PIN
      digitalWrite(LCD_BACKLIGHT_PIN, (data & 0x02)?LOW:HIGH);
#endif
      backLightOn = (data & 0x02)?false:true;
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
