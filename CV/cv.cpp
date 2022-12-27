#include <daisy_patch_sm.h>
#include <daisy.h>

#include "util.h"

using namespace daisy;
using namespace patch_sm;

int main(void)
{
  InitHardware(true);
  while(true) {
    GetHardware()->ProcessAllControls();
    float value = GetHardware()->GetAdcValue(CV_5);
    LOG_INFO("CV5: %s", f2a(value));
    System::Delay(10);
  }
}
