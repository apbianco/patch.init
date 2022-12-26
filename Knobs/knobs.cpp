#include <daisy_patch_sm.h>
#include <daisysp.h>
#include <daisy.h>

#include "util.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

struct KnobCalibrationValues {
  float true_min;
  float true_med;
  float true_max;
};

class KnobValue {
public:
  KnobValue(int cv_index, KnobCalibrationValues kcv, int factor=1000) :
    prev_(0.0f), cv_index_(cv_index),
    true_min_(kcv.true_min), true_med_(kcv.true_med),
    correction_low_(0.5f/(kcv.true_med - kcv.true_min)),
    correction_high_(0.5f/(kcv.true_max - kcv.true_med)),
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

  float true_min_, true_med_;
  float correction_low_, correction_high_;
  float calibrate_min_, calibrate_max_;

  int factor_;

  bool debug_;

  float GetValue() {
    GetHardware()->ProcessAnalogControls();
    return GetHardware()->GetAdcValue(cv_index_);
  }

  float CalibratedValue(float f) {
    if (f < true_med_) {
      return (f - true_min_)*correction_low_;
    } else {
      return 0.5 + (f - true_med_)*correction_high_;
    }
  }
};

int main(void)
{
  KnobCalibrationValues kcv[4] = {
    {-0.01260, 0.46364, 0.99676},
    {-0.01940, 0.46142, 0.99630},
    {-0.01379, 0.46813, 0.99758},
    {-0.01611, 0.46630, 0.99911}};
      
  InitHardware(true);
  KnobValue kv[4] = {
    KnobValue(CV_1, kcv[0]),
    KnobValue(CV_2, kcv[1]),
    KnobValue(CV_3, kcv[2]),
    KnobValue(CV_4, kcv[3]),
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
