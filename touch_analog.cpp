/*! TFT_eSPI v2.5.43 | MIT License | https://github.com/Bodmer/TFT_eSPI */

#include "setup.h"

#ifdef USE_ANALOG_TOUCH_PANEL

#include "touch_analog.h"
#include <TFT_eSPI.h>

#ifndef Z_THRESHOLD
  #define Z_THRESHOLD 350 // Touch pressure threshold for validating touches
#endif

void  TouchAnalog::touch_calibrate(uint16_t *calData)
{
  uint8_t calDataOK = 0;

  mTft->fillScreen(TFT_BLACK);
  mTft->setCursor(20, 0);
  mTft->setTextFont(2);
  mTft->setTextSize(1);
  mTft->setTextColor(TFT_WHITE, TFT_BLACK);

  mTft->println("Touch corners as indicated");

  mTft->setTextFont(1);
  mTft->println();

  calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

  mTft->fillScreen(TFT_BLACK);
  
  mTft->setTextColor(TFT_GREEN, TFT_BLACK);
  mTft->println("Calibration complete!");
}

#define _RAWERR 20 // Deadband error allowed in successive position samples
uint8_t TouchAnalog::validTouch(uint16_t *x, uint16_t *y, uint16_t threshold){
  uint16_t x_tmp, y_tmp, x_tmp2, y_tmp2;
  TSPoint p = mTs.getPoint();

  // Wait until pressure stops increasing to debounce pressure
  uint16_t z1 = 1;
  uint16_t z2 = 0;
  while (z1 > z2)
  {
    z2 = z1;
    z1 = p.z;
    delay(1);
  }

  //  Serial.print("Z = ");Serial.println(z1);

  if (z1 <= threshold) return false;
  
  x_tmp = p.x;
  y_tmp = p.y;

  //  Serial.print("Sample 1 x,y = "); Serial.print(x_tmp);Serial.print(",");Serial.print(y_tmp);
  //  Serial.print(", Z = ");Serial.println(z1);

  delay(1); // Small delay to the next sample
  p = mTs.getPoint();
  if (p.z <= threshold) return false;

  delay(2); // Small delay to the next sample
  p = mTs.getPoint();
  x_tmp2 = p.x;
  y_tmp2 = p.y;
  
  //  Serial.print("Sample 2 x,y = "); Serial.print(x_tmp2);Serial.print(",");Serial.println(y_tmp2);
  //  Serial.print("Sample difference = ");Serial.print(abs(x_tmp - x_tmp2));Serial.print(",");Serial.println(abs(y_tmp - y_tmp2));

  if (abs(x_tmp - x_tmp2) > _RAWERR) return false;
  if (abs(y_tmp - y_tmp2) > _RAWERR) return false;
  
  *x = x_tmp;
  *y = y_tmp;
  
  return true;
}

void TouchAnalog::updateTouch(uint16_t threshold)
{
  uint16_t x_tmp, y_tmp;
  
  if (threshold<20) threshold = 20;
  if (_pressTime > millis()) threshold=20;

  uint8_t n = 5;
  uint8_t valid = 0;
  while (n--)
  {
    if (validTouch(&x_tmp, &y_tmp, threshold)) valid++;;
  }

  if (valid<1) { _pressTime = 0; _press = false; return; }
  
  _pressTime = millis() + 50;

  convertRawXY(&x_tmp, &y_tmp);

  if (x_tmp >= VIEW_WIDTH || y_tmp >= VIEW_HEIGHT) { _press = false; return; }

  _pressX = x_tmp;
  _pressY = y_tmp;
  _press = true;
}

bool TouchAnalog::getTouch(uint16_t *x, uint16_t *y) const
{
  if (_press) {
    *x = _pressX;
    *y = _pressY;
  }
  return _press;
}

void TouchAnalog::convertRawXY(uint16_t *x, uint16_t *y)
{
  uint16_t x_tmp = *x, y_tmp = *y, xx, yy;

  if(!touchCalibration_rotate){
    xx=(x_tmp-touchCalibration_x0)*VIEW_WIDTH/touchCalibration_x1;
    yy=(y_tmp-touchCalibration_y0)*VIEW_HEIGHT/touchCalibration_y1;
    if(touchCalibration_invert_x)
      xx = VIEW_WIDTH - xx;
    if(touchCalibration_invert_y)
      yy = VIEW_HEIGHT - yy;
  } else {
    xx=(y_tmp-touchCalibration_x0)*VIEW_WIDTH/touchCalibration_x1;
    yy=(x_tmp-touchCalibration_y0)*VIEW_HEIGHT/touchCalibration_y1;
    if(touchCalibration_invert_x)
      xx = VIEW_WIDTH - xx;
    if(touchCalibration_invert_y)
      yy = VIEW_HEIGHT - yy;
  }
  *x = xx;
  *y = yy;
}

void TouchAnalog::calibrateTouch(uint16_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size)
{
  int16_t values[] = {0,0,0,0,0,0,0,0};
  uint16_t x_tmp, y_tmp;



  for(uint8_t i = 0; i<4; i++){
    mTft->fillRect(0, 0, size+1, size+1, color_bg);
    mTft->fillRect(0, VIEW_HEIGHT-size-1, size+1, size+1, color_bg);
    mTft->fillRect(VIEW_WIDTH-size-1, 0, size+1, size+1, color_bg);
    mTft->fillRect(VIEW_WIDTH-size-1, VIEW_HEIGHT-size-1, size+1, size+1, color_bg);

    if (i == 5) break; // used to clear the arrows
    
    switch (i) {
      case 0: // up left
        mTft->drawLine(0, 0, 0, size, color_fg);
        mTft->drawLine(0, 0, size, 0, color_fg);
        mTft->drawLine(0, 0, size , size, color_fg);
        break;
      case 1: // bot left
        mTft->drawLine(0, VIEW_HEIGHT-size-1, 0, VIEW_HEIGHT-1, color_fg);
        mTft->drawLine(0, VIEW_HEIGHT-1, size, VIEW_HEIGHT-1, color_fg);
        mTft->drawLine(size, VIEW_HEIGHT-size-1, 0, VIEW_HEIGHT-1 , color_fg);
        break;
      case 2: // up right
        mTft->drawLine(VIEW_WIDTH-size-1, 0, VIEW_WIDTH-1, 0, color_fg);
        mTft->drawLine(VIEW_WIDTH-size-1, size, VIEW_WIDTH-1, 0, color_fg);
        mTft->drawLine(VIEW_WIDTH-1, size, VIEW_WIDTH-1, 0, color_fg);
        break;
      case 3: // bot right
        mTft->drawLine(VIEW_WIDTH-size-1, VIEW_HEIGHT-size-1, VIEW_WIDTH-1, VIEW_HEIGHT-1, color_fg);
        mTft->drawLine(VIEW_WIDTH-1, VIEW_HEIGHT-1-size, VIEW_WIDTH-1, VIEW_HEIGHT-1, color_fg);
        mTft->drawLine(VIEW_WIDTH-1-size, VIEW_HEIGHT-1, VIEW_WIDTH-1, VIEW_HEIGHT-1, color_fg);
        break;
      }

    // user has to get the chance to release
    if(i>0) delay(1000);

    for(uint8_t j= 0; j<8; j++){
      // Use a lower detect threshold as corners tend to be less sensitive
      while(!validTouch(&x_tmp, &y_tmp, Z_THRESHOLD/2));
      values[i*2  ] += x_tmp;
      values[i*2+1] += y_tmp;
      }
    values[i*2  ] /= 8;
    values[i*2+1] /= 8;
  }


  // from case 0 to case 1, the y value changed. 
  // If the measured delta of the touch x axis is bigger than the delta of the y axis, the touch and TFT axes are switched.
  touchCalibration_rotate = false;
  if(abs(values[0]-values[2]) > abs(values[1]-values[3])){
    touchCalibration_rotate = true;
    touchCalibration_x0 = (values[1] + values[3])/2; // calc min x
    touchCalibration_x1 = (values[5] + values[7])/2; // calc max x
    touchCalibration_y0 = (values[0] + values[4])/2; // calc min y
    touchCalibration_y1 = (values[2] + values[6])/2; // calc max y
  } else {
    touchCalibration_x0 = (values[0] + values[2])/2; // calc min x
    touchCalibration_x1 = (values[4] + values[6])/2; // calc max x
    touchCalibration_y0 = (values[1] + values[5])/2; // calc min y
    touchCalibration_y1 = (values[3] + values[7])/2; // calc max y
  }

  // in addition, the touch screen axis could be in the opposite direction of the TFT axis
  touchCalibration_invert_x = false;
  if(touchCalibration_x0 > touchCalibration_x1){
    values[0]=touchCalibration_x0;
    touchCalibration_x0 = touchCalibration_x1;
    touchCalibration_x1 = values[0];
    touchCalibration_invert_x = true;
  }
  touchCalibration_invert_y = false;
  if(touchCalibration_y0 > touchCalibration_y1){
    values[0]=touchCalibration_y0;
    touchCalibration_y0 = touchCalibration_y1;
    touchCalibration_y1 = values[0];
    touchCalibration_invert_y = true;
  }

  // pre calculate
  touchCalibration_x1 -= touchCalibration_x0;
  touchCalibration_y1 -= touchCalibration_y0;

  if(touchCalibration_x0 == 0) touchCalibration_x0 = 1;
  if(touchCalibration_x1 == 0) touchCalibration_x1 = 1;
  if(touchCalibration_y0 == 0) touchCalibration_y0 = 1;
  if(touchCalibration_y1 == 0) touchCalibration_y1 = 1;

  // export parameters, if pointer valid
  if(parameters != NULL){
    parameters[0] = touchCalibration_x0;
    parameters[1] = touchCalibration_x1;
    parameters[2] = touchCalibration_y0;
    parameters[3] = touchCalibration_y1;
    parameters[4] = touchCalibration_rotate | (touchCalibration_invert_x <<1) | (touchCalibration_invert_y <<2);
  }
}

void TouchAnalog::set_calibrate(const uint16_t *parameters){
  touchCalibration_x0 = parameters[0];
  touchCalibration_x1 = parameters[1];
  touchCalibration_y0 = parameters[2];
  touchCalibration_y1 = parameters[3];

  if(touchCalibration_x0 == 0) touchCalibration_x0 = 1;
  if(touchCalibration_x1 == 0) touchCalibration_x1 = 1;
  if(touchCalibration_y0 == 0) touchCalibration_y0 = 1;
  if(touchCalibration_y1 == 0) touchCalibration_y1 = 1;

  touchCalibration_rotate = parameters[4] & 0x01;
  touchCalibration_invert_x = parameters[4] & 0x02;
  touchCalibration_invert_y = parameters[4] & 0x04;
}

#endif