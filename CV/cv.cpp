#include <daisy_patch_sm.h>
#include <daisy.h>

#include "cv.h"
#include "knobs.h"
#include "led.h"

using namespace daisy;
using namespace patch_sm;

int main(void)
{
  InitHardware(true);
  CVOut cv_out;
  Knob k = CreateKnob1();
  
  while(true) {
    k.PrintIfChange();
    float f = k.GetCalibratedScaledValue();
    if (f <= 0.0f) {
      f = 1.0f;
    }
    cv_out.SetVoltage(0.0);
    System::Delay(f);
    cv_out.SetVoltage(5.0);
    System::Delay(f);
  }
}
