#include <daisy_patch_sm.h>
#include <daisysp.h>
#include <daisy.h>

#include "util.h"
#include "linearizer.h"

using namespace daisy;
using namespace patch_sm;

class KnobValue {
public:
  KnobValue(int cv_index, LinearCalibrationValues kcv, int factor=1000) :
    cv_index_(cv_index),
    factor_(factor),
    prev_(0.0f),
    l1_(Linearizer(kcv.true_min, kcv.true_med, 0.0f, 0.5f)),
    l2_(Linearizer(kcv.true_med, kcv.true_max, 0.5f, 1.0f)),
    true_med_(kcv.true_med),
    calibrate_min_(10.0), calibrate_max_(-10.0),
    debug_(false) {
    prev_ = CalibratedValue(GetValue());
  }

  void SetDebug() {
    debug_ = true;
    LOG_INFO("L1:");
    l1_.Print();
    LOG_INFO("L2:");
    l2_.Print();
  }

  void Calibrate() {
    float value = GetValue();
    LOG_INFO("CV_%d: min=%s", cv_index_+1, f2a(value));
  }
  
  bool CaptureValueIndicateChange(float *new_value) {
    float fvalue = CalibratedValue(GetValue());
    int value = static_cast<int>(fvalue * factor_);
    int prev = static_cast<int>(prev_ * factor_);
    if (debug_) {
      LOG_INFO("fvalue=%s prev_=%s value=%d prev=%d",
	       f2a(fvalue), f2a(prev_), value, prev);
    }
    if (value != prev) {
      *new_value = prev_ = fvalue;
      return true;
    }
    return false;
  }

  void PrintIfChange() {
    float value;
    if (CaptureValueIndicateChange(&value)) {
      PrintCalibrated();
    }
  }

  virtual void PrintCalibrated() {
    LOG_INFO("CV_%d: %d", cv_index_+1,
	     static_cast<int>(prev_ * factor_));
  }

  float GetValue() {
    GetHardware()->ProcessAnalogControls();
    return GetHardware()->GetAdcValue(cv_index_);
  }

protected:
  int cv_index_;
  int factor_;
  float prev_;
  Linearizer l1_, l2_;
  
private:
  float true_med_;
  float calibrate_min_, calibrate_max_;
  bool debug_;

  float CalibratedValue(float f) {
    if (f <= true_med_) {
      return l1_.Linearize(f);
    } else {
      return l2_.Linearize(f);
    }
  }
};
