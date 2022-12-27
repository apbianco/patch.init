#include <daisy_patch_sm.h>
#include <daisy.h>

#include "util.h"
#include "control_voltage.h"

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
  Linearizer l_led(-5, 5.0, 1.64, 2.83);
  
  while(true) {
#if 1
    float v;
    if (cv.CaptureValueIndicateChange(&v)) {
      GetHardware()->WriteCvOut(CV_OUT_2, l_led.Linearize(v));
    }
#else
    cv.Calibrate();
#endif    
    System::Delay(10);
  }
}
