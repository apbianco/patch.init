#include <daisy_patch_sm.h>
#include <daisy.h>

#include "util.h"
#include "vstrig.h"

using namespace daisy;
using namespace patch_sm;

int main(void) {
  InitHardware(true);

  VSTrig t;
  t.Init();

  while(true) {
    t.Process();
    LOG_INFO_EVERY_MS(1000, "Alive...");
  }
}
