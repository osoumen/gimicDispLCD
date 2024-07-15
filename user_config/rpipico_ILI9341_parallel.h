/*
使用する前に、TFT_eSPIライブラリの初期設定が必要です。
未インストールの場合は、まずArduino IDEのライブラリマネージャから"TFT_eSPI"をインストールしてください。
インストールされたライブラリ内のファイルを書き換えますので、あらかじめArduinoの環境設定でArduinoフォルダの場所を確認しておいてください。

ILI9341(パラレル8bit)用のセットアップファイルが用意されていますので、それを使用するようにUser_Setup_Select.hを書き換えます。
Arduino/libraries/TFT_eSPI/User_Setup_Select.h を編集し、
コメントアウトされた大量のセットアップファイルが並んでいる箇所で、
#include <User_Setups/Setup102_RP2040_ILI9341_parallel.h>
のコメントを外し、それ以外はすべてコメントアウトされた状態にしてください。
その後、User_Setups/Setup102_RP2040_ILI9341_parallel.h を以下のように書き換えてください。

#define TFT_DC   28    // Data Command control pin
#define TFT_RST   2    // Reset pin
↓
#define TFT_DC   7    // Data Command control pin
#define TFT_RST   4    // Reset pin

#define TFT_WR   22
↓
#define TFT_WR   6

#define TFT_D0    6
#define TFT_D1    7
#define TFT_D2    8
#define TFT_D3    9
#define TFT_D4   10
#define TFT_D5   11
#define TFT_D6   12
#define TFT_D7   13
↓
#define TFT_D0    8
#define TFT_D1    9
#define TFT_D2   10
#define TFT_D3   11
#define TFT_D4   12
#define TFT_D5   13
#define TFT_D6   14
#define TFT_D7   15

このファイル内で設定したピン番号の通りに液晶ユニットとボードを繋いでください。
ピンを変更する場合は、TFT_D0-D7のピンが必ず連続するようにしてください。

バックライトを制御するピンが足りませんが、ボード裏面のTP5は、LEDが点灯する時にONになりますので、
これを引き出し、MOSFET等を用いて制御しても良いと思います。

次に、Arduino IDEのビルド設定を以下のように変更してください。
ボード: Raspberry Pi Pico/RP2040 > Raspberry Pi Pico
Optimize: "Optimize Even More(-O3)"
USB Stack: "Adafruit TinyUSB Host (native)"
USBホスト機能が不要の場合は、このファイル内の ENABLE_USB_HOST をundefしてください。
その場合、USBスタックはデフォルト(Pico SDK)のままで良いです。
他はどの設定でも構いません

デフォルト状態ではPS4コントローラなどの一部のゲームパッドではUSBディスクリプタのバッファが不足するため使用出来ません。
これを使用する場合、
Arduino15/packages/rp2040/hardware/rp2040/xx.xx.xx/libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/rp2040/tusb_config_rp2040.h
ファイルを編集し、
CFG_TUH_ENUMERATION_BUFSIZE のサイズをデフォルトの256から512程度の値に変更してください。

以上で、ILI9341をTFT_eSPIライブラリで使用出来るようになります。
一度TFT_eSPIのデモプログラムの動作を確認する事をおすすめします。

USB Stack: "Adafruit TinyUSB Host (native)"に設定して書き込んだ後は、ボードのプログラムを書き換える際は、BOOTSELボタンを押しながらPCに接続してください。

*/

// G.I.M.I.Cと接続するUART, I2Cのポート指定(必須)
#define TO_GIMIC_SERIAL Serial1
#define TO_GIMIC_I2C    Wire1
#define GIMIC_IF_TX_PIN     0
#define GIMIC_IF_RX_PIN     1
#define GIMIC_IF_SDA_PIN    2
#define GIMIC_IF_SCL_PIN    3
// G.I.M.I.Cの、EX I/Fコネクタと以下のように接続してください。
// TXD(4) => GIMIC_IF_RX_PIN
// RXD(2) => GIMIC_IF_TX_PIN
// SDA(3) => GIMIC_IF_SDA_PIN
// SCL(5) => GIMIC_IF_SCL_PIN
// GND(6) => GND
// +5V => VBUS

// メインの操作ボタンのピン割り当てを設定します。
// ボタンの端子のもう片方をGNDに接続してください。
// 使用しないピンはundefする事でピンを空ける事ができます
#define BUTTON1_PIN_NO  5
#define BUTTON2_PIN_NO  18
#define BUTTON3_PIN_NO  19
#define BUTTON4_PIN_NO  20
#define BUTTON5_PIN_NO  28
#define ENC_A_PIN_NO    16
#define ENC_B_PIN_NO    17
// アナログタッチパネルを使用する場合のみ指定してください
#define TP_XR_PIN_NO    26  // ADC入力ピン
#define TP_YD_PIN_NO    27  // ADC入力ピン
#define TP_XL_PIN_NO    21   // 汎用ピンで良い
#define TP_YU_PIN_NO    22   // 汎用ピンで良い

// ロータリーエンコーダーのパルスあたりのクリック数を{1,2,4}から指定します
// クリック数=30,パルス数=15のタイプなら2が適切です。ノンクリックのタイプは0または4を指定してください。
#define RENC_CLICKS_PER_PULSE 2

// USBゲームパッドのボタン割り当てを変更できます
#define BTN4_JOYPAD_BTN   1 // A btn
#define BS_JOYPAD_BTN     2 // B btn
#define BTN5_JOYPAD_BTN   3 // X btn
#define ENTER_JOYPAD_BTN  4 // Y btn
#define PAGE_UP_JOYPAD_BTN 5    // L btn
#define PAGE_DOWN_JOYPAD_BTN 6  // R btn
#define MONITOR_MODE_JOYPAD_BTN 7   // SELECT btn
#define STOP_JOYPAD_BTN 8   // START btn

// キーボード、ゲームパッドの一部のボタンのキーリピート設定です(10ms単位)
#define KEY_REPEAT_DELAY  25
#define KEY_REPEAT_RATE  2

// タッチパネルの誤反応が多い場合は数値を上げてください
#define TOUCH_THRESHOLD 300

// タッチパネルとマウスのダブルクリック判定時間です(ミリ秒)
#define DOUBLE_CLICK_TIME_MS  330

// G.I.M.I.Cとシリアル通信する速度です。動作が不安定な場合は低いレートを試してみてください
// #define UART_MAX_BAUD_RATE  115200
// #define UART_MAX_BAUD_RATE  230400
// #define UART_MAX_BAUD_RATE  460800
#define UART_MAX_BAUD_RATE  921600

// 画面の転置設定です。逆さまや横向きになる場合に変更してください。
#define SCREEN_ROTATION 1

// USBホスト機能を有効化してゲームパッド、マウス、キーボードを使用出来るようにします。不要な場合はundefしてください。
// 現在、RP2040のみ対応しています。
#define ENABLE_USB_HOST 1


// #define DO_LCD_WRITE_ANOTHER_CORE 1
#define DO_TP_UPDATE_ANOTHER_CORE 1

// #define USE_LGFX 1
