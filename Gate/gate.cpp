#include <daisy_patch_sm.h>
#include <daisy.h>

#include "util.h"
#include "gate.h"

int main(void) {
  InitHardware(true);

  auto gate = CreateGateOut1();
  gate.SetDebug();
  gate.Init();
  
  while(true) {
    gate.Alternate(500);
  }
}  
