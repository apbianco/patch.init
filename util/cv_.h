#ifndef CV__H
#define CV__H

#include "util.h"
#include "transcaler.h"

using namespace daisy;
using namespace patch_sm;

// Operations supported on CV inputs (CV inputs are either
// potentiometers or sampled input jacks):
//
// - Calibration: print the current value emitted (potentiometer) or
//   found on the input jack (CV). Use this to determine the offset
//   values for min/mid/max to be provided as input to a
//   LinearCalibrationValues instance.
//
// - SetDebug(): call to have extra information logged. Off by
//   default.
//
// - GetCalibratedValueAndIndicateChange: a method loading the current
//   calibrated value to a pointed location, returning true when the
//   value has changed for some definition of changed: crude rounding
//   obtained taking the integer value of the input multiplied by a
//   factor we call sensitivity.
//
// - Print(): print raw, calibrated and scaled value of the CV.
//
// - GetRawValue(): read the current raw value value.
//
// - GetCalibratedValue(): read the calibrated current raw value.
//
// TODO:
//
//  - Make it an option to avoid calling ProcessAnalogControl. Process()
//    can be invoked directly on the select control

class CV_ {
public:
  // Basic CV corrected with two correction segments.
  CV_(int cv_index, int factor=1000, int sensitivity=100) :
    cv_index_(cv_index),
    factor_(factor), sensitivity_(sensitivity),
    last_(0.0f),
    l1_(Transcaler(0.0f, 0.5f, 0.0f, 0.5f)),
    l2_(Transcaler(0.5f, 1.0f, 0.5f, 1.0f)),
    true_med_(0.5f),
    debug_(false) {
    if (sensitivity_ >  factor_) {
      LOG_ERROR("sensitivity_ >  factor_, sensitivity_ reset to %d",
		factor_);
      sensitivity_ = factor_;
    }
    last_ = CalibrateValue(GetRawValue());
  }
  virtual ~CV_() = default;

  void SetDebug() {
    debug_ = true;
    LOG_INFO("L1:");
    l1_.Print();
    LOG_INFO("L2:");
    l2_.Print();
  }

  void Calibrate() {
    float value = GetRawValue();
    LOG_INFO("CV_%d: min=%s", cv_index_+1, f2a(value));
  }
  
  bool GetCalibratedValueAndIndicateChange(float *new_value) {
    float fvalue = CalibrateValue(GetRawValue());
    int value = static_cast<int>(fvalue * sensitivity_);
    int prev = static_cast<int>(last_ * sensitivity_);
    if (debug_) {
      LOG_INFO("fvalue=%s last_=%s value=%d prev=%d",
	       f2a(fvalue), f2a(last_), value, prev);
    }
    // Always update with the new value if one was provided
    last_ = fvalue;
    if (new_value != nullptr) {
      *new_value = last_;
    }
    // Return true when we estimate a change happened. sensitivity_
    // was used to truncate the result: the lower sensitivity_ is, the
    // more resilient we are to detecting a change.
    if (value != prev) {
      return true;
    }
    return false;
  }

  void PrintIfChange() {
    float value;
    if (GetCalibratedValueAndIndicateChange(&value)) {
      Print();
    }
  }

  virtual void Print() {
    FB(b1); FB(b2);
    float r = GetRawValue();
    LOG_INFO("CV_%d: raw=%s, cal=%s, scaled=%d",
	     cv_index_+1, f2a(r, b1), f2a(last_, b2),
	     static_cast<int>(last_ * factor_));
  }

  inline float GetRawValue() {
    // FIXME: we can optimize this by just returning the value of
    // GetHardware()->controls[cv_index_].Process();
    GetHardware()->ProcessAnalogControls();
    return GetHardware()->GetAdcValue(cv_index_);
  }

  inline float GetCalibratedValue() {
    return CalibrateValue(GetRawValue());
  }

  float GetCalibratedScaledValue() {
    return GetCalibratedValue() * factor_;
  }

protected:
  int cv_index_;	        // The device index
  int factor_;			// Factor that brings input to an integer
  int sensitivity_;		// How sensitive we are to a change
  float last_;			// Last value read
  Transcaler l1_, l2_;		// Two linearizing segments
  float true_med_;		// value < true_med_: use l1_, l2_ otherwise.
  
private:
  bool debug_;			// Control printing additional info.

  inline float CalibrateValue(float f) {
    if (f <= true_med_) {
      return l1_.Transcale(f);
    } else {
      return l2_.Transcale(f);
    }
  }
};

#endif //  CV__H
