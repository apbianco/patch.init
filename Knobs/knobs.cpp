#include <daisy_patch_sm.h>
#include <daisysp.h>
#include <daisy.h>

#include "knobs.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

int main(void)
{
  InitHardware(true);
  Knob kv[4] = {
    CreateKnob1(),
    CreateKnob2(),
    CreateKnob3(),
    CreateKnob4()
  };
  
  while(true) {
    for (int i = 0; i < 4; i++) {
      kv[i].PrintIfChange();
    }
    System::Delay(10);
  }
}
