#include <daisy_patch_sm.h>
#include <daisysp.h>
#include <daisy.h>

#include "util.h"
#include "linearizer.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

LinearCalibrationValues kcv_[4] = {
  {-0.01260, 0.46364, 0.99676},
  {-0.01940, 0.46142, 0.99630},
  {-0.01379, 0.46813, 0.99758},
  {-0.01611, 0.46630, 0.99911}};

class KnobValue {
public:
  KnobValue(int cv_index) : KnobValue(cv_index, kcv_[cv_index]) {}

  KnobValue(int cv_index, LinearCalibrationValues kcv, int factor=1000) :
    prev_(0.0f), cv_index_(cv_index),
    l1_(Linearizer(kcv.true_min, kcv.true_med, 0.0f, 0.5f)),
    l2_(Linearizer(kcv.true_med, kcv.true_max, 0.5f, 1.0f)),
    true_med_(kcv.true_med),
    factor_(factor),
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
    if (value < calibrate_min_) {
      calibrate_min_ = value;
    }
    if (value > calibrate_max_) {
      calibrate_max_ = value;
    }
    LOG_INFO("CV_%d: min=%s", cv_index_+1, f2a(calibrate_min_));
    LOG_INFO("CV_%d: max=%s", cv_index_+1, f2a(calibrate_max_));
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

  void PrintCalibrated() {
    LOG_INFO("CV_%d: %d", cv_index_+1,
	     static_cast<int>(prev_ * factor_));
  }
  
private:
  float prev_;
  int cv_index_;
  Linearizer l1_, l2_;
  float true_med_;
  int factor_;
  float calibrate_min_, calibrate_max_;
  bool debug_;

  float GetValue() {
    GetHardware()->ProcessAnalogControls();
    return GetHardware()->GetAdcValue(cv_index_);
  }

  float CalibratedValue(float f) {
    if (f <= true_med_) {
      return l1_.Linearize(f);
    } else {
      return l2_.Linearize(f);
    }
  }
};

int main(void)
{
  InitHardware(true);
  KnobValue kv[4] = {
    KnobValue(CV_1),
    KnobValue(CV_2),
    KnobValue(CV_3),
    KnobValue(CV_4),
  };

  while(true) {
#if 1
    for (int i = 0; i < 4; i++) {
      kv[i].PrintIfChange();
    }
#else
    kv.Calibrate();
#endif    
    System::Delay(10);
  }
}
