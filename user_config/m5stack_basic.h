/*
TFT_eSPIライブラリは使用せず、M5Unifiedを使用します。
未インストールの場合は、まずArduino IDEのライブラリマネージャから"M5Unified"をインストールしてください。

Arduino IDEのビルド設定を以下のように変更してください。
ボード: M5Stack > M5Core
PSRAM: "Disabled"
他はデフォルト設定で構いません。

もし画面に何も表示されない場合は、
G.I.M.I.C SETTINGSで、Startup When Powered をオフにして手動で電源を入れる設定に変更してみてください。

*/

// G.I.M.I.Cと接続するUART, I2Cのポート指定(必須)
#define TO_GIMIC_SERIAL Serial2
#define TO_GIMIC_I2C    Wire1
#define GIMIC_IF_TX_PIN     TXD2
#define GIMIC_IF_RX_PIN     RXD2
#define GIMIC_IF_SDA_PIN    M5.Ex_I2C.getSDA()
#define GIMIC_IF_SCL_PIN    M5.Ex_I2C.getSCL()
// G.I.M.I.Cの、EX I/Fコネクタと以下のように接続してください。
// TXD(4) => RXD2
// RXD(2) => TXD2
// SDA(3) => SDA
// SCL(5) => SCL
// GND(6) => GND
// +5V => 5V

// メインの操作ボタンのピン割り当てを設定します。
// ボタンの端子のもう片方をGNDに接続してください。
// 使用しないピンはundefする事でピンを空ける事ができます
#define BUTTON1_PIN_NO  15
#define BUTTON2_PIN_NO  12
#define BUTTON3_PIN_NO  13
#define BUTTON4_PIN_NO  2
#define BUTTON5_PIN_NO  5
#define ENC_A_PIN_NO    35  // 要プルアップ
#define ENC_B_PIN_NO    36  // 要プルアップ
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
// #define ENABLE_USB_HOST 1


#define M5_UNIFIED 1

// #define USE_LGFX 1
