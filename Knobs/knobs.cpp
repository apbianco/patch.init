#include <daisy_patch_sm.h>
#include <daisysp.h>
#include <daisy.h>

#include "util.h"
#include "linearizer.h"
#include "knobs.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

LinearCalibrationValues kcv_[4] = {
  {-0.01260, 0.46364, 0.99676},
  {-0.01940, 0.46142, 0.99630},
  {-0.01379, 0.46813, 0.99758},
  {-0.01611, 0.46630, 0.99911}};

int main(void)
{
  InitHardware(true);
  Knob kv[4] = {
    Knob(CV_1, kcv_[0]),
    Knob(CV_2, kcv_[1]),
    Knob(CV_3, kcv_[2]),
    Knob(CV_4, kcv_[3]),
  };

  while(true) {
    for (int i = 0; i < 4; i++) {
      kv[i].PrintIfChange();
    }
    System::Delay(10);
  }
}
