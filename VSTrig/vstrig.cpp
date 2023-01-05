#include <daisy_patch_sm.h>
#include <daisy.h>

#include "util.h"
#include "vstrig.h"

using namespace daisy;
using namespace patch_sm;

int main(void) {
  InitHardware(true);

  VSTrig t(50);
  t.Init();

  while(true) {
    t.Process();
  }
}

