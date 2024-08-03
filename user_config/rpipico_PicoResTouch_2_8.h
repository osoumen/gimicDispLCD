/*
使用する前に、TFT_eSPIライブラリの初期設定が必要です。
未インストールの場合は、まずArduino IDEのライブラリマネージャから"TFT_eSPI"をインストールしてください。
インストールされたライブラリ内のファイルを書き換えますので、あらかじめArduinoの環境設定でArduinoフォルダの場所を確認しておいてください。

https://www.waveshare.com/wiki/Pico-ResTouch-LCD-2.8 がLCDの公式マニュアルです。
ページの末尾の方にある
Resource > Demo codes > Examples > Pico-ResTouch-LCD-X_X_Code.zip
をダウンロードしてください。
zipファイル内の Arduino/ResTouch-LCD-2.8/ に入っているファイルをArduinoのライブラリフォルダ内にあるファイルと置き換えます。
TFT_eSPI.h と User_Setup_Select.h は Arduino/libraries/TFT_eSPI に上書きコピーしてください。
Setup23_TTGO_TM.h は、 Arduino/libraries/TFT_eSPI/User_Setups に上書きコピーしてください。
その後、User_Setups/Setup23_TTGO_TM.h を以下のように書き換えてください。

#define TFT_SDA_READ // Read from display, it only provides an SDA pin
↓
// #define TFT_SDA_READ // Read from display, it only provides an SDA pin

#define TFT_BACKLIGHT_ON HIGH  // Level to turn ON back-light (HIGH or LOW)
↓
#define TFT_BACKLIGHT_ON LOW  // Level to turn ON back-light (HIGH or LOW)

//#define TFT_INVERSION_ON
#define TFT_INVERSION_OFF
↓
#define TFT_INVERSION_ON
//#define TFT_INVERSION_OFF

define SPI_FREQUENCY  40000000     // This display also seems to work reliably at 80MHz
//#define SPI_FREQUENCY  80000000
↓
//define SPI_FREQUENCY  40000000     // This display also seems to work reliably at 80MHz
#define SPI_FREQUENCY  80000000

次に、Arduino　IDEのビルド設定を以下のように変更してください。
ボード: Raspberry Pi Pico/RP2040 > Raspberry Pi Pico
Optimize: "Optimize Even More(-O3)"
USB Stack: "Adafruit TinyUSB Host (native)"
USBホスト機能が不要の場合は、このファイル内の ENABLE_USB_HOST をundefしてください。
その場合、USBスタックはデフォルト(Pico SDK)のままで良いです。
他はどの設定でも構いません

デフォルト状態ではPS4コントローラなどの一部のゲームパッドではUSBディスクリプタのバッファが不足するため使用出来ません。
使用したい場合は、
Arduino15/packages/rp2040/hardware/rp2040/xx.xx.xx/libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/rp2040/tusb_config_rp2040.h
ファイルを編集し、
CFG_TUH_ENUMERATION_BUFSIZE のサイズをデフォルトの256から512程度の値に変更してください。

以上で、Pico-ResTouch-LCD-2.8をTFT_eSPIライブラリで使用出来るようになります。
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
#define BUTTON1_PIN_NO  4
#define BUTTON2_PIN_NO  6
#define BUTTON3_PIN_NO  7
#define BUTTON4_PIN_NO  14
#define BUTTON5_PIN_NO  26
#define ENC_A_PIN_NO    27
#define ENC_B_PIN_NO    28
// アナログタッチパネルを使用する場合のみ指定してください
// #define TP_XR_PIN_NO    26  // ADC入力ピン
// #define TP_YD_PIN_NO    27  // ADC入力ピン
// #define TP_XL_PIN_NO    21   // 汎用ピンで良い
// #define TP_YU_PIN_NO    22   // 汎用ピンで良い

// ロータリーエンコーダーのパルスあたりのクリック数を{1,2,4}から指定します
// クリック数=30,パルス数=15のタイプなら2が適切です。ノンクリックのタイプは0または4を指定してください。
// 未定義にすると初回起動時にセットアップを行います。再セットアップする場合はボタン２を押しながら起動します。
// セットアップにはボタン1,2,3,5が必須です。
// #define RENC_CLICKS_PER_PULSE 2

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


#define DO_LCD_WRITE_ANOTHER_CORE 1
// #define DO_TP_UPDATE_ANOTHER_CORE 1

// #define USE_LGFX 1
