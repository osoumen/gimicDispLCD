#pragma once

#define LGFX_USE_V1

#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI       _bus_instance;
  lgfx::Light_PWM     _light_instance;
  lgfx::Touch_XPT2046 _touch_instance;

  public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host   = VSPI_HOST;
      cfg.spi_mode   = 0;
      cfg.freq_write = 80000000;
      cfg.freq_read  = 16000000;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk   = 18;
      cfg.pin_miso   = 19;
      cfg.pin_mosi   = 23;
      cfg.pin_dc     = 2;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs       = 15;
      cfg.pin_rst      = 4;
      cfg.panel_width  = 240;
      cfg.panel_height = 320;
      cfg.offset_x     = 0;
      cfg.offset_y     = 0;
      cfg.invert       = false;
      cfg.rgb_order    = false;
      cfg.offset_rotation = 0;
      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();
      cfg.pin_bl      = 25;
      // cfg.pwm_channel = 1;
      // cfg.freq   = 44100;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    { // タッチスクリーン制御の設定を行います。（必要なければ削除）
      auto cfg = _touch_instance.config();

      cfg.x_min      = 0;    // タッチスクリーンから得られる最小のX値(生の値)
      cfg.x_max      = 4095;  // タッチスクリーンから得られる最大のX値(生の値)
      cfg.y_min      = 0;    // タッチスクリーンから得られる最小のY値(生の値)
      cfg.y_max      = 4095;  // タッチスクリーンから得られる最大のY値(生の値)
      // cfg.pin_int    = 26;   // INTが接続されているピン番号
      cfg.bus_shared = false; // 画面と共通のバスを使用している場合 trueを設定
      cfg.offset_rotation = 3;// 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定

// SPI接続の場合
      cfg.spi_host = HSPI_HOST;// 使用するSPIを選択 (HSPI_HOST or VSPI_HOST)
      cfg.freq = 2500000;     // SPIクロックを設定
      cfg.pin_sclk = 14;     // SCLKが接続されているピン番号
      cfg.pin_mosi = 13;     // MOSIが接続されているピン番号
      cfg.pin_miso = 12;     // MISOが接続されているピン番号
      cfg.pin_cs   = 5;     //   CSが接続されているピン番号

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);  // タッチスクリーンをパネルにセットします。
    }

    setPanel(&_panel_instance);
  }
};
