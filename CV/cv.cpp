#include <daisy_patch_sm.h>
#include <daisy.h>

#include "control_voltage.h"
#include "knobs.h"
#include "visible_led.h"

using namespace daisy;
using namespace patch_sm;

LinearCalibrationValues kcv_[4] = {
  {-0.97918, -0.01791, 0.99993 },
  {-0.97909, -0.00659, 0.99990 },
  {-0.97711,  0.01214, 0.99990 },
  {-0.96890,  0.01174, 0.99987 },
};

int main(void)
{
  InitHardware(true);
  CVOut cv_out;
  Knob k(CV_1, kcv_[0]);
  
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
