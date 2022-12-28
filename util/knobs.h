// This should be defining the CV baseclass. Knobs and CVIn should
// inherit from that base class.
//
// Let's see how taking the gate into account brings...

#ifndef KNOBS_H
#define KNOBS_H

#include <daisy_patch_sm.h>
#include <daisysp.h>
#include <daisy.h>

#include "util.h"
#include "linearizer.h"

using namespace daisy;
using namespace patch_sm;

// Operations supported on CV inputs (CV inputs are either
// potentiometers or sampled input jacks):
//
// - Calibration: print the current value. Use this to determine the
//   offset values for min/mid/max to be provided as input to a
//   LinearCalibrationValues instance.
//
// - SetDebug(): call to have extra information logged. Debugging
//   is off by default.
//
// - GetCalibratedValueAndIndicateChange: a method loading the current
//   calibrated value to a pointed location, returning true when the
//   value has changed for some definition of changed: crude rounding
//   multiplying by a factor.
//
// - Print(): print raw, calibrated and scaled value of the CV.
//
// - GetRawValue(): read the current raw value value.
//
// - GetCalibratedValue(): read the calibrated current raw value.

class Knob {
public:
  // Basic: perfect CV: min/mid/max have ideal values normalized
  // values.
  Knob(int cv_index) : Knob(cv_index, {0, 0.5, 1.0}) {}
  
  // Corrected CV: corrected with two correction segments.
  Knob(int cv_index, LinearCalibrationValues kcv, int factor=1000) :
    cv_index_(cv_index),
    factor_(factor),
    last_(0.0f),
    l1_(Linearizer(kcv.true_min, kcv.true_med, 0.0f, 0.5f)),
    l2_(Linearizer(kcv.true_med, kcv.true_max, 0.5f, 1.0f)),
    true_med_(kcv.true_med),
    debug_(false) {
    last_ = CalibrateValue(GetRawValue());
  }
  virtual ~Knob() = default;

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
    int value = static_cast<int>(fvalue * factor_);
    int prev = static_cast<int>(last_ * factor_);
    if (debug_) {
      LOG_INFO("fvalue=%s last_=%s value=%d prev=%d",
	       f2a(fvalue), f2a(last_), value, prev);
    }
    // Always update with the new value
    *new_value = last_ = fvalue;
    // Return true when we estimate a change happened. factor_
    // implements some simple truncation.
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

  float GetRawValue() {
    GetHardware()->ProcessAnalogControls();
    return GetHardware()->GetAdcValue(cv_index_);
  }

  float GetCalibratedValue() {
    return CalibrateValue(GetRawValue());
  }

protected:
  int cv_index_;	        // The device index
  int factor_;			// Factor that brings input to an integer
  float last_;			// Last value read
  Linearizer l1_, l2_;		// Two linearizing segments
  
private:
  float true_med_;		// value < true_med_: use l1_, l2_ otherwise.
  bool debug_;			// Control printing additional info.

  float CalibrateValue(float f) {
    if (f <= true_med_) {
      return l1_.Linearize(f);
    } else {
      return l2_.Linearize(f);
    }
  }
};

#endif  // KNOB_H
