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
  CVOut(Transcaler l): l_(l), debug_(false) {}
  CVOut() : CVOut(Transcaler(0.0f, 1.0f, 0.0f, 1.0f)) {}

  void SetVoltage(float v) {
    GetHardware()->WriteCvOut(CV_OUT_1, l_.Transcale(v));
    if (debug_) {
      LOG_INFO("CV_OUT_1: %s", f2a(v));
    }
  }

  void SetDebug() { debug_ = true; }
  
 private:
  Transcaler l_;
  bool debug_;
};

CVIn CreateCVIn1() {
  return CVIn(CV_1, {-0.97918, -0.01791, 0.99993 });
}

CVIn CreateCVIn2() {
  return CVIn(CV_2, {-0.97909, -0.00659, 0.99990 });
}

CVIn CreateCVIn3() {
  return CVIn(CV_3, {-0.97711,  0.01214, 0.99990 });
}

CVIn CreateCVIn4() {
  return CVIn(CV_4, {-0.96890,  0.01174, 0.99987 });
}

#endif  // CONTROL_VOLTAGE_H
