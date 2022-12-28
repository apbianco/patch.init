#ifndef VISIBLE_LED_H
#define VISIBLE_LED_H

#include "linearizer.h"

// The linearizer maps an input range to a voltage output range that
// make the light coming out of the LED visibly changing throughout
// the range. Otherwise, some values of the range don't really light
// up the LED.
class VisibleLED {
 public:
 VisibleLED() : VisibleLED(0, 0) {}
 VisibleLED(float v_min, float v_max) :
  l_(Linearizer(v_min, v_max, 1.50, 2.83)) {}
  void SetVoltage(float v) {
    GetHardware()->WriteCvOut(CV_OUT_2, l_.Linearize(v));
  }
 private:
  Linearizer l_;
};

#endif  // VISIBLE_LED_H
