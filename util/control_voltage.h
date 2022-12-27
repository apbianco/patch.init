#ifndef CONTROL_VOLTAGE_H
#define CONTROL_VOLTAGE_H

#include "knobs.h"

class CVIn : public KnobValue {
 public:
 CVIn(int cv_index, LinearCalibrationValues kcv, int factor=1000) :
  KnobValue(cv_index, kcv, factor) {
    l1_ = Linearizer(kcv.true_min, kcv.true_med, -5.0f, 0.0f);
    l2_ = Linearizer(kcv.true_med, kcv.true_max, 0.0f, 5.0f);
  }
  void PrintCalibrated() {
    LOG_INFO("CV_%d: %s", cv_index_+1, f2a(prev_));
  }
};

#endif  // CONTROL_VOLTAGE_H
