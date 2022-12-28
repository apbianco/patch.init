#include <daisy_patch_sm.h>
#include <daisy.h>

#include "cv.h"
#include "knobs.h"
#include "led.h"

using namespace daisy;
using namespace patch_sm;

int main(void) {
  InitHardware(true);
  
  while(true) {
    System::Delay(5);
  }
}
