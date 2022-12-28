#ifndef CONTROL_VOLTAGE_H
#define CONTROL_VOLTAGE_H

#include "cv_.h"

class CVIn : public CV_ {
 public:
 CVIn(int cv_index, LinearCalibrationValues kcv, int factor=1000) :
  CV_(cv_index, factor) {
    l1_ = Transcaler(kcv.true_min, kcv.true_med, -5.0f, 0.0f);
    l2_ = Transcaler(kcv.true_med, kcv.true_max, 0.0f, 5.0f);
  }
};

class CVOut {
 public:
 CVOut() : debug_(false) {}

  void SetVoltage(float v) {
    GetHardware()->WriteCvOut(CV_OUT_1, v);
    if (debug_) {
      LOG_INFO("CV_OUT_1: %s", f2a(v));
    }
  }

  void SetDebug() { debug_ = true; }
  
 private:
  bool debug_;
};

#endif  // CONTROL_VOLTAGE_H
