#include "setup.h"
#if defined(M5_UNIFIED)
#include <M5Unified.h>
#endif
#include "EscSeqParser.h"
#include "tftDispSPI.h"
#ifdef ENABLE_USB_HOST
#include "hid_host.h"
#include "Adafruit_TinyUSB.h"
#endif
#include <Wire.h>
#include <EEPROM.h>

// #define ENABLE_SERIAL_OUT 1

uint8_t button_input=0;
uint8_t joypad_input=0;
#if defined(M5_UNIFIED)
uint8_t m5_tp_btn=0;
#endif
uint8_t arrowkey_hold_time[4] = {0};
uint8_t inputkey_hold_time[256] = {0};
uint8_t button_ipol=0;
volatile bool i2c_reading;
uint8_t i2c_reading_address;
uint32_t prev_joypad=0;
uint32_t inputUpdateTime;
uint32_t lastTpDownTime;
uint16_t tp_X, tp_Y;
bool tp_On=false;
uint16_t mouse_X, mouse_Y;
uint32_t mouse_buttons=0;
bool backLightOn=false;
volatile int rotary_inc=0;
volatile int rotary_phase=0;
int to_hide_cursor=0;

#ifdef ENABLE_USB_HOST
Adafruit_USBH_Host USBHost;
#endif
tftDispSPI tft;
EscSeqParser parser(&tft);

void i2c_recv(int len);
void i2c_req();

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
#if defined(BUTTON1_PIN_NO)
  for (int i=0; i<5; ++i) {
    if(digitalRead(BUTTON1_PIN_NO) == LOW) {
      ++onCount;
    }
  }
#endif
  EEPROM.begin(256);
  uint8_t calData[11];
  if (onCount >= 3) {
    tft.touch_calibrate((uint16_t*)calData);
    calData[10] = SCREEN_ROTATION;
    // キャリブレーション値をFlash領域に保存
    for (int i=0; i<11; ++i) {
      EEPROM.write(i, calData[i]);
    }
    EEPROM.commit();
  }
  else {
    int initialCnt = 0;
    // キャリブレーション値をFlash領域から読み出す
    for (int i=0; i<11; ++i) {
      calData[i] = EEPROM.read(i);
      if (calData[i] == 0xff) {
        ++initialCnt;
      }
    }
    // スクリーンの向きが変わったらキャリブレーションを実行する
    if (calData[10] != SCREEN_ROTATION) {
      initialCnt = 11;
    }
    // 初期状態ならキャリブレーションを実行する
    if (initialCnt == 11) {
      tft.touch_calibrate((uint16_t*)calData);
      calData[10] = SCREEN_ROTATION;
      for (int i=0; i<11; ++i) {
        EEPROM.write(i, calData[i]);
      }
      EEPROM.commit();
    }
  }
  EEPROM.end();
  tft.set_calibrate((uint16_t*)calData);
}

void setup() {
#if defined(M5_UNIFIED)
  auto cfg = M5.config();
  M5.begin(cfg);
#endif

#if defined(ARDUINO_ARCH_RP2040)
  TO_GIMIC_I2C.setSDA(GIMIC_IF_SDA_PIN);
  TO_GIMIC_I2C.setSCL(GIMIC_IF_SCL_PIN);
#elif defined(ARDUINO_ARCH_ESP32)
  TO_GIMIC_I2C.setPins(GIMIC_IF_SDA_PIN, GIMIC_IF_SCL_PIN);
#else
#error Unknown Platform
#endif
  TO_GIMIC_I2C.setClock(400000);
  TO_GIMIC_I2C.begin(0x20);
  TO_GIMIC_I2C.onReceive(i2c_recv);
  TO_GIMIC_I2C.onRequest(i2c_req);
  
#ifdef BUTTON1_PIN_NO
  pinMode(BUTTON1_PIN_NO, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (BUTTON1_PIN_NO, true);
  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN_NO), buttonChange1, CHANGE);
#endif
#ifdef BUTTON2_PIN_NO
  pinMode(BUTTON2_PIN_NO, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (BUTTON2_PIN_NO, true);
  attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN_NO), buttonChange2, CHANGE);
#endif
#ifdef BUTTON3_PIN_NO
  pinMode(BUTTON3_PIN_NO, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (BUTTON3_PIN_NO, true);
  attachInterrupt(digitalPinToInterrupt(BUTTON3_PIN_NO), buttonChange3, CHANGE);
#endif
#ifdef BUTTON4_PIN_NO
  pinMode(BUTTON4_PIN_NO, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (BUTTON4_PIN_NO, true);
  attachInterrupt(digitalPinToInterrupt(BUTTON4_PIN_NO), buttonChange4, CHANGE);
#endif
#ifdef BUTTON5_PIN_NO
  pinMode(BUTTON5_PIN_NO, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled (BUTTON5_PIN_NO, true);
  attachInterrupt(digitalPinToInterrupt(BUTTON5_PIN_NO), buttonChange5, CHANGE);
#endif
#if defined(ENC_A_PIN_NO) && defined(ENC_B_PIN_NO)
  pinMode(ENC_A_PIN_NO, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled(ENC_A_PIN_NO, true);
  pinMode(ENC_B_PIN_NO, INPUT_PULLUP);
  gpio_set_input_hysteresis_enabled(ENC_B_PIN_NO, true);
  attachInterrupt(digitalPinToInterrupt(ENC_A_PIN_NO), rotary_enc, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B_PIN_NO), rotary_enc, CHANGE);
#endif

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
#ifdef ENABLE_USB_HOST
  USBHost.begin(0);
#endif

#if defined(ARDUINO_ARCH_RP2040)
  TO_GIMIC_SERIAL.setPinout(GIMIC_IF_TX_PIN, GIMIC_IF_RX_PIN);
  TO_GIMIC_SERIAL.setFIFOSize(4096);
  TO_GIMIC_SERIAL.begin(115200);
#elif defined(ARDUINO_ARCH_ESP32)
  TO_GIMIC_SERIAL.setRxBufferSize(4096);
  TO_GIMIC_SERIAL.begin(115200, SERIAL_8N1, GIMIC_IF_RX_PIN, GIMIC_IF_TX_PIN);
#else
#error Unknown Platform
#endif

  tft.init();

  doTPCallibration();

#ifdef TFT_BL
  digitalWrite(TFT_BL, LOW);
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
#ifdef ENABLE_USB_HOST
  USBHost.task();
#endif
  if (backLightOn) {  
    while (TO_GIMIC_SERIAL.available() > 0) {
      parser.ParseByte(TO_GIMIC_SERIAL.read());
      draw = true;
    }
    // while (Serial.available() > 0) {
    //   parser.ParseByte(Serial.read());
    // }

    if ((millis() - inputUpdateTime) > 10) {
      inputUpdateTime = millis();
#if defined(M5_UNIFIED)
      M5.update();
#endif
      draw = TouchPanelTask(draw);
#ifdef ENABLE_USB_HOST
      draw = MouseTask(draw);
      JoypadTask();
      KeyboardTask();
#endif
    }

    while (rotary_inc != 0) {
      if (rotary_inc > 0) {
        TO_GIMIC_SERIAL.write("\x1b@993y");  // KEY_JOG_CW
        --rotary_inc;
      }
      if (rotary_inc < 0) {
        TO_GIMIC_SERIAL.write("\x1b@994y");  // KEY_JOG_CCW
        ++rotary_inc;
      }
    }

    if (to_hide_cursor > 0) {
      to_hide_cursor = 0;
      tft.hideCursorPointer();
    }

#if defined(ARDUINO_ARCH_RP2040)
    if (TO_GIMIC_SERIAL.overflow()) {
      TO_GIMIC_SERIAL.flush();
      tft.move(0,0);
      tft.set_attribute(17);
      tft.puts_("!!Overflowed!!");
      draw = true;
    }
#endif

    if (draw) {
      tft.updateContent();
#ifdef ENABLE_SERIAL_OUT
      Serial.println(millis() - updateStartTime);
#endif
    }
  }
  else {
    delay(10);
  }
}

#ifdef ENABLE_USB_HOST
void KeyboardTask()
{
  static uint8_t input_keys[6] = {0};
  uint8_t new_input_keys[6];
  uint8_t key_rep = 0;
  if (getKeyboardKey(new_input_keys)) {
    // 入力キーが変化した
    for (int i=0; i<6; ++i) {
      if (new_input_keys[i] != 0) {
        bool hold = false;
        for (int j=0; j<6; ++j) {
          if (input_keys[j] == new_input_keys[i]) {
            hold = true;
            break;
          }
        }
        if (hold == false) {
          inputkey_hold_time[new_input_keys[i]] = 0;
          key_rep = new_input_keys[i];  // 同時に押されたキーは１つしか受け付けないがリピートはされるので許容する
        }
      }
    }
    memcpy(input_keys, new_input_keys, 6);
  }
  for (int i=0; i<6; ++i) {
    uint8_t key = input_keys[i];
    if (key != 0) {
      inputkey_hold_time[key]++;
      if (inputkey_hold_time[key] > KEY_REPEAT_DELAY) {
        if (((inputkey_hold_time[key] - KEY_REPEAT_DELAY) % KEY_REPEAT_RATE) == 0) {
          key_rep = key;
        }
      }
    }
    if (key_rep >= KEY_CURSOR_UP) {
      uint8_t cmd[] = {0x1b, '[', 'A'};
      switch (key_rep) {
        case KEY_CURSOR_UP:
          cmd[2] = 'A';
      		TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_CURSOR_DOWN:
          cmd[2] = 'B';
      		TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_CURSOR_LEFT:
          cmd[2] = 'D';
      		TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_CURSOR_RIGHT:
          cmd[2] = 'C';
      		TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    // case KEY_RET:
        //   break;
		    case KEY_BS:
          TO_GIMIC_SERIAL.write(0x08); // BS
          break;
		    case KEY_HOME:
          cmd[2] = '1';
  		    TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_END:
          cmd[2] = '4';
		      TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_PAGE_UP:
          cmd[2] = '5';
		      TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_PAGE_DOWN:
          cmd[2] = '6';
		      TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_FUNC_F1:
          cmd[1] = 'O';
          cmd[2] = 'P';
          TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_FUNC_F2:
          cmd[1] = 'O';
          cmd[2] = 'Q';
          TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_FUNC_F3:
          cmd[1] = 'O';
          cmd[2] = 'R';
          TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_FUNC_F4:
          cmd[1] = 'O';
          cmd[2] = 'S';
          TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_FUNC_F5:
          cmd[1] = 'O';
          cmd[2] = 'T';
          TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_FUNC_F6:
          cmd[1] = 'O';
          cmd[2] = 'U';
          TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_FUNC_F7:
          cmd[1] = 'O';
          cmd[2] = 'V';
          TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_FUNC_F8:
          cmd[1] = 'O';
          cmd[2] = 'W';
          TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_FUNC_F9:
          cmd[1] = 'O';
          cmd[2] = 'X';
          TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    case KEY_FUNC_F10:
          cmd[1] = 'O';
          cmd[2] = 'Y';
          TO_GIMIC_SERIAL.write(cmd, 3);
          break;
		    // case KEY_FUNC_F11:
        //   break;
		    // case KEY_FUNC_F12:
        //   break;
      }
    }
    else {
      if (key_rep != 0) {
        TO_GIMIC_SERIAL.write(key_rep);
      }
    }
  }
}

void JoypadTask()
{
  const uint32_t press = getJoypadButtons();
  const uint32_t change = press ^ prev_joypad;
  const uint32_t up = change & prev_joypad;
  const uint32_t down = change & press;
  prev_joypad = press;
  uint32_t btn_state = 0;
  uint8_t arrow_key_bit = 1 << 3;
  uint8_t cmd[] = {0x1b, '[', 'A'};
  const uint8_t cmd_end[] = {'D', 'A', 'B', 'C'};

  // 十字キーはキーリピートに対応する
  for (int i=0; i<4; ++i) {
    if (up & arrow_key_bit) {
      arrowkey_hold_time[i] = 0;
    }
    if (down & arrow_key_bit) {
      cmd[2] = cmd_end[i];
      TO_GIMIC_SERIAL.write(cmd,3);
    }
    if (press & arrow_key_bit) {
      arrowkey_hold_time[i]++;
      if (arrowkey_hold_time[i] > KEY_REPEAT_DELAY) {
        if (((arrowkey_hold_time[i] - KEY_REPEAT_DELAY) % KEY_REPEAT_RATE) == 0) {
          cmd[2] = cmd_end[i];
          TO_GIMIC_SERIAL.write(cmd,3);
				}
      }
    }
    arrow_key_bit <<= 1;
  }

  if (press & (1 << (BTN4_JOYPAD_BTN+6))) {
    btn_state |= 1 << 6;  // ボタン４
  }
  if (down & (1 << (BS_JOYPAD_BTN+6))) {
    TO_GIMIC_SERIAL.write(0x08);  // BS
  }
  if (press & (1 << (BTN5_JOYPAD_BTN+6))) {
    btn_state |= 1 << 7;  // ボタン５
  }
  if (down & (1 << (ENTER_JOYPAD_BTN+6))) {
    TO_GIMIC_SERIAL.write('\r');  // Enter
  }
  if (down & (1 << (PAGE_UP_JOYPAD_BTN+6))) {
    cmd[2] = '5';
		TO_GIMIC_SERIAL.write(cmd, 3);  // KEY_PAGE_UP
  }
  if (down & (1 << (PAGE_DOWN_JOYPAD_BTN+6))) {
    cmd[2] = '6';
		TO_GIMIC_SERIAL.write(cmd, 3);  // KEY_PAGE_DOWN
  }
  if (down & (1 << (MONITOR_MODE_JOYPAD_BTN+6))) {
    cmd[1] = 'O';
		cmd[2] = 'P';
		TO_GIMIC_SERIAL.write(cmd, 3);  // F1(MONOTOR MODE)
  }
  if (down & (1 << (STOP_JOYPAD_BTN+6))) {
    cmd[1] = 'O';
		cmd[2] = 'Q';
		TO_GIMIC_SERIAL.write(cmd, 3);  // F2(STOP)
  }
  joypad_input = btn_state;
}

bool MouseTask(bool draw)
{
  int x,y,wheel;
  const uint32_t press = getMouseMove(&x, &y, &wheel);
  const uint32_t change = press ^ mouse_buttons;
  const uint32_t up = change & mouse_buttons;
  const uint32_t down = change & press;
  clearMouseMove();
  mouse_buttons = press;

  int btnState = 0;
  if (up & 1) {
    TO_GIMIC_SERIAL.printf("\x1b[<%d;%d;%dM", MOUSE_BTN1_REL | IS_TP, mouse_X, mouse_Y);
    draw = true;
  }
  if (down & 1) {
    btnState |= MOUSE_BTN1;
    if ((millis() - lastTpDownTime) < DOUBLE_CLICK_TIME_MS) {
      btnState |= MOUSE_BTN_DBL_CLK;
      lastTpDownTime -= DOUBLE_CLICK_TIME_MS;
    }
    else {
      lastTpDownTime = millis();
    }
  }
  if (down & 2) {
    btnState |= MOUSE_BTN2;
  }
  if ((x != 0) || (y != 0) || (btnState != 0)) {
    x += mouse_X;
    if (x < 0) x = 0;
    if (x >= 320) x = 320 - 1;
    y += mouse_Y;
    if (y < 0) y = 0;
    if (y >= 240) y = 240 - 1;
    if (press) {
      TO_GIMIC_SERIAL.printf("\x1b[<%d;%d;%dM", btnState | IS_TP, x, y);
    }
    mouse_X = x;
    mouse_Y = y;
    tft.setCursorPointer(x, y);
    draw = true;
  }
  // wheelの処理
  while (wheel > 0) {
    TO_GIMIC_SERIAL.write("\x1b@995y");  // KEY_MOUSEWHEEL_UP
    --wheel;
  }
  while (wheel < 0) {
    TO_GIMIC_SERIAL.write("\x1b@996y");  // KEY_MOUSEWHEEL_DOWN
    ++wheel;
  }
  return draw;
}
#endif

bool TouchPanelTask(bool draw)
{
  uint16_t x,y;
#ifdef TOUCH_CS
  bool isOn = tft.getTouch(&x, &y, TOUCH_THRESHOLD);
#else
  bool isOn = false;
#if defined(M5_UNIFIED)
  auto t = M5.Touch.getDetail();
  isOn = t.isPressed();
  if (isOn && t.y >= 240) {
    const uint8_t touch_btn[] = {1 << 3, 1 << 4, 1 << 7, 1 << 5, 1 << 6};
    m5_tp_btn = touch_btn[(t.x * 5) / 320];   // TODO: マルチタッチができないか検討
    isOn = false;
  }
  else {
    m5_tp_btn = 0;
  }
  if (isOn) {
    x = t.x;
    y = t.y;
  }
#endif
#endif
  if (isOn == false && tp_On == true) {
    TO_GIMIC_SERIAL.printf("\x1b[<%d;%d;%dM", MOUSE_BTN1_REL | IS_TP, tp_X, tp_Y);
    // Serial.printf("\x1b[<%d;%d;%dM\n", MOUSE_BTN1_REL | IS_TP, tp_X, tp_Y);
    tp_On = false;
    // tft.hideCursorPointer();
    draw = true;
  }
  else if (isOn) {
    int btnState = IS_TP;
    if (tp_On == false) {
      btnState |= MOUSE_BTN1;
      if ((millis() - lastTpDownTime) < DOUBLE_CLICK_TIME_MS) {
        int distance_x = (tp_X - x);
        int distance_y = (tp_Y - y);
        if ((distance_x * distance_x + distance_y * distance_y) < 16*16) {
          btnState |= MOUSE_BTN_DBL_CLK;
          lastTpDownTime -= DOUBLE_CLICK_TIME_MS;
        }
      }
      else {
        lastTpDownTime = millis();
      }
      tp_X = tp_Y = 9999;
      tp_On = true;
      to_hide_cursor++;
    }
    if ((x != tp_X) || (y != tp_Y)) {
      TO_GIMIC_SERIAL.printf("\x1b[<%d;%d;%dM", btnState, x, y);
      // Serial.printf("\x1b[<%d;%d;%dM\n", btnState, x, y);
      tp_X = x;
      tp_Y = y;
    }
    // tft.setCursorPointer(x, y);
    draw = true;
  }
  return draw;
}

#ifdef BUTTON1_PIN_NO
void buttonChange1() {
  int pinInput = button_input & ~(1 << 3);
  if (digitalRead(BUTTON1_PIN_NO) == LOW) {
    pinInput |= 1 << 3;
  }
  button_input = pinInput;
  ++to_hide_cursor;
}
#endif

#ifdef BUTTON2_PIN_NO
void buttonChange2() {
  int pinInput = button_input & ~(1 << 4);
  if (digitalRead(BUTTON2_PIN_NO) == LOW) {
    pinInput |= 1 << 4;
  }
  button_input = pinInput;
  ++to_hide_cursor;
}
#endif

#ifdef BUTTON3_PIN_NO
void buttonChange3() {
  int pinInput = button_input & ~(1 << 5);
  if (digitalRead(BUTTON3_PIN_NO) == LOW) {
    pinInput |= 1 << 5;
  }
  button_input = pinInput;
  ++to_hide_cursor;
}
#endif

#ifdef BUTTON4_PIN_NO
void buttonChange4() {
  int pinInput = button_input & ~(1 << 6);
  if (digitalRead(BUTTON4_PIN_NO) == LOW) {
    pinInput |= 1 << 6;
  }
  button_input = pinInput;
  ++to_hide_cursor;
}
#endif

#ifdef BUTTON5_PIN_NO
void buttonChange5() {
  int pinInput = button_input & ~(1 << 7);
  if (digitalRead(BUTTON5_PIN_NO) == LOW) {
    pinInput |= 1 << 7;
  }
  button_input = pinInput;
  ++to_hide_cursor;
}
#endif

#if defined(ENC_A_PIN_NO) && defined(ENC_B_PIN_NO)
void rotary_enc() {
  int a_level = digitalRead(ENC_A_PIN_NO) ? (1 << 2) : 0;
  int b_level = digitalRead(ENC_B_PIN_NO) ? (1 << 3) : 0;
  rotary_phase = a_level | b_level | (rotary_phase >> 2);
  switch (rotary_phase) {
    case (1 + (3 << 2)):
      ++rotary_inc;
      break;
    case (3 + (1 << 2)):
      --rotary_inc;
      break;
  }
}
#endif

void i2c_recv(int len) {
  if (len == 1) {
    i2c_reading = true;
    i2c_reading_address = (uint8_t)TO_GIMIC_I2C.read();
  }
  else if (len == 2) {
    uint8_t address = (uint8_t)TO_GIMIC_I2C.read();
    uint8_t data = TO_GIMIC_I2C.read();
    if (address == 0x01) {
      button_ipol = data; // 読み取るだけ
    }
    else if (address == 0x09) {
      if ((data & 0x01) == 0) {
        // リセット要求に応える
#if defined(ARDUINO_ARCH_RP2040)
        watchdog_enable(1, 1);
        while(1);
#elif defined(ARDUINO_ARCH_ESP32)
        delay(1);
        ESP.restart();
        while(1);
#else
#warning The reset action will not be performed
#endif
      }
#ifdef LED_BUILTIN
      digitalWrite(LED_BUILTIN, (data & 0x02)?LOW:HIGH);  // led g
#endif
#ifdef TFT_BL
      digitalWrite(TFT_BL, (data & 0x02)?LOW:HIGH);
#endif
      backLightOn = (data & 0x02)?false:true;
    }
  }
}

void i2c_req() {
  if (i2c_reading) {
    i2c_reading = false;
    if (i2c_reading_address == 0x09) {
#if defined(M5_UNIFIED)
      TO_GIMIC_I2C.write(button_input | joypad_input | m5_tp_btn);
#else
      TO_GIMIC_I2C.write(button_input | joypad_input);
#endif
    }
    else {
      TO_GIMIC_I2C.write(0);
    }
  }
  else {
    TO_GIMIC_I2C.write(0);
  }
}
