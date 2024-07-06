/*! TFT_eSPI v2.5.43 | MIT License | https://github.com/Bodmer/TFT_eSPI */

#ifndef touch_analog_h
#define touch_analog_h

#include "setup.h"
#include "touch_intf.h"
#include "TouchScreen.h"

class TFT_eSPI;

class TouchAnalog : public TouchIntf {
public:
// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
  TouchAnalog() : mTs(TP_YU_PIN_NO, TP_XR_PIN_NO, TP_YD_PIN_NO, TP_XL_PIN_NO, 600) {}
  void  setTft(TFT_eSPI *tft) { mTft = tft; }
  bool  getTouch(uint16_t *x, uint16_t *y, uint16_t threshold);
  void  touch_calibrate(uint16_t *calData);
  void  set_calibrate(const uint16_t *calData);

private:
  static const int	VIEW_WIDTH = 320;
  static const int	VIEW_HEIGHT = 240;

  TouchScreen mTs;
  TFT_eSPI *mTft;

  uint16_t touchCalibration_x0 = 300, touchCalibration_x1 = 3600, touchCalibration_y0 = 300, touchCalibration_y1 = 3600;
  uint8_t  touchCalibration_rotate = 1, touchCalibration_invert_x = 2, touchCalibration_invert_y = 0;
  uint32_t _pressTime;        // Press and hold time-out
  uint16_t _pressX, _pressY;  // For future use (last sampled calibrated coordinates)

  uint8_t validTouch(uint16_t *x, uint16_t *y, uint16_t threshold);
  void convertRawXY(uint16_t *x, uint16_t *y);
  void calibrateTouch(uint16_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size);
};

#endif // touch_analog_h