#ifndef setup_h
#define setup_h

#define M5_UNIFIED 1

// UART, I2Cのポート指定(必須)
#if defined(M5_UNIFIED)
#define TO_GIMIC_SERIAL Serial2
#define TO_GIMIC_I2C    Wire
#define GIMIC_IF_TX_PIN     TXD2
#define GIMIC_IF_RX_PIN     RXD2
#define GIMIC_IF_SDA_PIN    M5.Ex_I2C.getSDA()
#define GIMIC_IF_SCL_PIN    M5.Ex_I2C.getSCL()
#else
#define TO_GIMIC_SERIAL Serial1
#define TO_GIMIC_I2C    Wire1
#define GIMIC_IF_TX_PIN     0
#define GIMIC_IF_RX_PIN     1
#define GIMIC_IF_SDA_PIN    2
#define GIMIC_IF_SCL_PIN    3
#endif


// 使用しないピンはundefする事でピンを空ける事ができます
#define BUTTON1_PIN_NO  4
#define BUTTON2_PIN_NO  6
#define BUTTON3_PIN_NO  7
#define BUTTON4_PIN_NO  14
#define BUTTON5_PIN_NO  26
#define ENC_A_PIN_NO    27
#define ENC_B_PIN_NO    28

// ジョイパッド用のボタン設定
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
#define DOUBLE_CLICK_TIME_MS  330
// #define UART_MAX_BAUD_RATE  115200
// #define UART_MAX_BAUD_RATE  230400
// #define UART_MAX_BAUD_RATE  460800
#define UART_MAX_BAUD_RATE  921600
#define STORE_SINGLEBYTEGLYPH_TO_RAM 1
#define SCREEN_ROTATION 1
#define INVERT_DISPLAY true
// #define BG_BUFF_NUM 1
#define ENABLE_USB_HOST
#define ENABLE_TFT_DMA 1

#if defined(ARDUINO_ARCH_RP2040)
#define ENABLE_MULTI_CORE 1
// #define DO_LCD_WRITE_ANOTHER_CORE 1
#elif defined(ARDUINO_ARCH_ESP32)
// #define ENABLE_MULTI_CORE 1
// #define DO_LCD_WRITE_ANOTHER_CORE 1
#define gpio_set_input_hysteresis_enabled(x,y) (NULL)
#else
#define gpio_set_input_hysteresis_enabled(x,y) (NULL)
#endif

#endif // setup_h
