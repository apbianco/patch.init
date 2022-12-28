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
  CVIn cv(CV_8, kcv_[3]);
  CVOut cv_out;
  // Create an LED that is going to react to -5.9/5.0 V
  VisibleLED led(0.0, 1.0);
  cv_out.SetDebug();
  Knob k(CV_1);
  
  while(true) {
    float f = 0.0f;
    if (k.CaptureValueIndicateChange(&f)) {
      k.Print();
    }
    f += 1.0f;
    cv_out.SetVoltage(0.0);
    System::Delay(1/f);
    cv_out.SetVoltage(5.0);
    System::Delay(1/f);
  }
}
