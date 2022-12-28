// This should be defining the CV baseclass. Knobs and CVIn should
// inherit from that base class.
//
// Let's see how taking the gate into account brings...

#ifndef KNOBS_H
#define KNOBS_H

#include <daisy_patch_sm.h>
#include <daisysp.h>
#include <daisy.h>

#include "cv_.h"
#include "transcaler.h"

using namespace daisy;
using namespace patch_sm;

class Knob : public CV_ {
public:
  Knob(int cv_index, LinearCalibrationValues kcv, int factor=1000) :
    CV_(cv_index, factor) {
      l1_ = Transcaler(kcv.true_min, kcv.true_med, 0.0f, 0.5f);
      l2_ = Transcaler(kcv.true_med, kcv.true_max, 0.5f, 1.0f);
      true_med_ = kcv.true_med;
    }
};

// Direct constructor for the knobs 1 to 4, properly compensated

Knob CreateKnob1() {
  return Knob(CV_1, {-0.01260, 0.46364, 0.99676});
}

Knob CreateKnob2() {
  return Knob(CV_2, {-0.01940, 0.46142, 0.99630});
}

Knob CreateKnob3() {
  return Knob(CV_3, {-0.01379, 0.46813, 0.99758});
}

Knob CreateKnob4() {
  return Knob(CV_4, {-0.01611, 0.46630, 0.99911});
}

#endif  // KNOB_H
