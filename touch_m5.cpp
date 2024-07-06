#include "setup.h"

#if defined(M5_UNIFIED)

#include "touch_m5.h"
#include <M5Unified.h>

void  TouchM5::updateTouch(uint16_t threshold)
{
  M5.update();
}

bool  TouchM5::getTouch(uint16_t *x, uint16_t *y) const
{
  auto t = M5.Touch.getDetail();
  bool isOn = t.isPressed();
  if (isOn) {
    *x = t.x;
    *y = t.y;
  }
  return isOn;
}

#endif