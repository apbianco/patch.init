#include <daisy_patch_sm.h>
#include <daisysp.h>
#include <daisy.h>

#include "util.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

class KnobValue {
public:
  KnobValue(int cv_index,
	    float true_min, float true_max, int factor=1000) :
    prev_(0.0f), cv_index_(cv_index),
    true_min_(true_min),
    correction_(1.0f/(true_max - true_min)),
    calibrate_min_(10.0), calibrate_max_(-10.0),
    factor_(factor), debug_(false) {
    prev_ = CalibratedValue(GetValue());
  }

  void SetDebug() { debug_ = true; }

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
      if (debug_) {
	LOG_INFO("Found different: %d/%d", value, prev);
      }
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

  float true_min_, correction_;
  float calibrate_min_, calibrate_max_;

  int factor_;

  bool debug_;

  float GetValue() {
    GetHardware()->ProcessAnalogControls();
    return GetHardware()->GetAdcValue(cv_index_);
  }

  float CalibratedValue(float f) {
    return (f - true_min_)*correction_;
  }
};

int main(void)
{
  InitHardware(true);
  KnobValue kv(CV_2, -0.01940, 0.99630);

  while(true) {
#if 0
    kv.PrintIfChange();
#else
    kv.Calibrate();
#endif    
    System::Delay(10);
  }
}
