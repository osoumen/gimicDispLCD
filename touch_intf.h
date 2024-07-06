#ifndef touch_intf_h
#define touch_intf_h

#include <stdint.h>

class TouchIntf {
public:
  virtual bool  getTouch(uint16_t *x, uint16_t *y, uint16_t threshold=600) = 0;
  virtual void  touch_calibrate(uint16_t *calData) = 0;
  virtual void  set_calibrate(const uint16_t *calData) = 0;
};

class TouchNull : public TouchIntf {
public:
  bool  getTouch(uint16_t *x, uint16_t *y, uint16_t threshold) { return false; }
  void  touch_calibrate(uint16_t *calData) {}
  void  set_calibrate(const uint16_t *calData) {}
};

#endif // touch_intf
