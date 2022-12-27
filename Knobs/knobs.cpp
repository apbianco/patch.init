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
  KnobValue kv[4] = {
    KnobValue(CV_1, kcv_[0]),
    KnobValue(CV_2, kcv_[1]),
    KnobValue(CV_3, kcv_[2]),
    KnobValue(CV_4, kcv_[3]),
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
