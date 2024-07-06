#ifndef touch_m5_h
#define touch_m5_h

#include "setup.h"
#include "touch_intf.h"

class TouchM5 : public TouchIntf {
public:
  void  updateTouch(uint16_t threshold);
  bool  getTouch(uint16_t *x, uint16_t *y) const;
  void  touch_calibrate(uint16_t *calData) {}
  void  set_calibrate(const uint16_t *calData) {}

private:
};

#endif // touch_m5_h