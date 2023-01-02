#ifndef LED_H
#define LED_H

#include "transcaler.h"

// The linearizer maps an input range to a voltage output range that
// make the light coming out of the LED visibly changing throughout
// the range. Otherwise, some values of the range don't really light
// up the LED.
class LED {
public:
  LED() : LED(0.0, 5.0) {}
  LED(float v_min, float v_max) :
    v_max_(v_max), current_voltage_(0.0f),
    memorized_current_voltage_(0.0f), debug_(false),
    l_(Transcaler(v_min, v_max, 1.50, 2.83)) {
    SetVoltage(current_voltage_);
  }
  
  ~LED() {
    RestoreMemorizedVoltage();
  }

  void SetDebug() { debug_ = true; }
  
  void SetVoltage(float v) {
    current_voltage_ = v;
    GetHardware()->WriteCvOut(CV_OUT_2, l_.Transcale(v));
    last_transition_time_ = System::GetNow();
    if (debug_) {
      Print();
    }
  }

  void On() {
    SetVoltage(v_max_);
  }

  void OnHalf() {
    SetVoltage(v_max_/2.0f);
  }

  void Off() {
    SetVoltage(0.0f);
  }

  void Alternate(float delay_ms = 500) {
    if (System::GetNow() - last_transition_time_ > delay_ms) {
      current_voltage_ == v_max_ ? Off() : On();
    }
  }

  float MemorizeVoltage() {
    memorized_current_voltage_ = current_voltage_;
    return current_voltage_;
  }

  float RestoreMemorizedVoltage() {
    SetVoltage(memorized_current_voltage_);
    return memorized_current_voltage_;
  }

  void BlockBlink(int n, float delay=250) {
    MemorizeVoltage();
    while(n-- > 0) {
      SetVoltage(v_max_);
      System::Delay(delay);
      SetVoltage(0.0f);
      System::Delay(delay);
    }
    RestoreMemorizedVoltage();
  }

  void Print() {
    FB(b1); FB(b2); FB(b3);
    LOG_INFO("LED: current_voltage_: %s, memorized_current_voltage_: %s, "
	     "v_max_: %s, last_transition_time_: %d",
	     f2a(current_voltage_, b1),
	     f2a(memorized_current_voltage_, b2),
	     f2a(v_max_, b3), last_transition_time_);
  }
  
 private:
  float v_max_;
  float current_voltage_;
  float memorized_current_voltage_;
  uint32_t last_transition_time_;
  bool debug_;
  Transcaler l_;
};

#endif  // LED_H
