#ifndef VISIBLE_LED_H
#define VISIBLE_LED_H

#include "linearizer.h"

// The linearizer maps an input range to a voltage output range that
// make the light coming out of the LED visibly changing throughout
// the range. Otherwise, some values of the range don't really light
// up the LED.
class VisibleLED {
public:
  VisibleLED() : VisibleLED(0.0, 5.0) {}
  VisibleLED(float v_min, float v_max) :
    current_voltage_(0.0f),
    memorized_current_voltage_(0.0f),
    l_(Linearizer(v_min, v_max, 1.50, 2.83)) {
    SetVoltage(current_voltage_);
  }
  
  ~VisibleLED() {
    RestoreMemorizedVoltage();
  }
  
  void SetVoltage(float v) {
    current_voltage_ = v;
    GetHardware()->WriteCvOut(CV_OUT_2, l_.Linearize(v));
  }

  float MemorizeVoltage() {
    memorized_current_voltage_ = current_voltage_;
    return current_voltage_;
  }

  float RestoreMemorizedVoltage() {
    SetVoltage(memorized_current_voltage_);
    return memorized_current_voltage_;
  }

  void Print() {
    FB(b1); FB(b2);
    LOG_INFO("LED: current_voltage_: %s, memorized_current_voltage_: %s",
	     f2a(current_voltage_, b1),
	     f2a(memorized_current_voltage_, b2));
  }
  
 private:
  float current_voltage_;
  float memorized_current_voltage_;
  Linearizer l_;
};

#endif  // VISIBLE_LED_H
