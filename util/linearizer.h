// Linear interpolation from a [start, end] interval to a [start', end']
// interval.
//
// Use this to make potentiometer or CV in a little easier to use.
// NOTE: Use this if you can assume that your hardware works
// linearily. It probably doesn't but for small intervals, this might
// work.

#ifndef LINEARIZER_H
#define LINEARIZER_H

#include "util.h"

struct LinearCalibrationValues {
  float true_min;
  float true_med;
  float true_max;
};

class Linearizer {
public:
  Linearizer(float s, float e, float S, float E) :
    a_((S-E)/(s-e)), b_(0.5 * ((S+E) - a_ * (s+e))) {}
  
  float Linearize(float x) { return (a_*x + b_); }
  
  void Print() {
    LOG_INFO("a=%s, b=%s", f2a(a_), f2a(b_));
  }
   
 private:
  float a_, b_, factor_;
};

#endif  // LINEARIZER_H
