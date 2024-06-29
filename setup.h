#ifndef setup_h
#define setup_h

// UART, I2Cのポート指定(必須)
#define TO_GIMIC_SERIAL Serial1
#define TO_GIMIC_I2C    Wire1
#define GIMIC_IF_TX_PIN     0
#define GIMIC_IF_RX_PIN     1
#define GIMIC_IF_SDA_PIN    2
#define GIMIC_IF_SCL_PIN    3

// 使用しないピンはundefする事でピンを空ける事ができます
#define BUTTON1_PIN_NO  4
#define BUTTON2_PIN_NO  6
#define BUTTON3_PIN_NO  7
#define BUTTON4_PIN_NO  14
#define BUTTON5_PIN_NO  26
#define ENC_A_PIN_NO    27
#define ENC_B_PIN_NO    28

// A btn
#define BTN4_JOYPAD_BTN   1
// B btn
#define BS_JOYPAD_BTN     2
// X btn
#define BTN5_JOYPAD_BTN   3
// Y btn
#define ENTER_JOYPAD_BTN  4
// L btn
#define PAGE_UP_JOYPAD_BTN 5
// R btn
#define PAGE_DOWN_JOYPAD_BTN 6
// SELECT btn
#define MONITOR_MODE_JOYPAD_BTN 7
// START btn
#define STOP_JOYPAD_BTN 8

#define KEY_REPEAT_DELAY  25
#define KEY_REPEAT_RATE  2

#define TOUCH_THRESHOLD 300
// #define UART_MAX_BAUD_RATE  115200
// #define UART_MAX_BAUD_RATE  230400
// #define UART_MAX_BAUD_RATE  460800
#define UART_MAX_BAUD_RATE  921600
#define ENABLE_CURSOR_POINTER 1
#define SINGLEBYTEGLYPH_TO_RAM 1
#define SCREEN_ROTATION 1
#define INVERT_DISPLAY true
// #define BG_BUFF_NUM 1

#if defined(ARDUINO_ARCH_RP2040)
#define ENABLE_MULTI_CORE 1
#endif

#endif // setup_h