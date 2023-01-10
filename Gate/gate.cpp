#include <daisy_patch_sm.h>
#include <daisy.h>

#include "util.h"
#include "gate.h"

int main(void) {
  InitHardware(true);

  auto out_gate = CreateOutGate1();
  auto in_gate = CreateInGate1();
  
  out_gate.SetDebug();
  out_gate.Init();
  in_gate.SetDebug();
  in_gate.Init();
  
  while(true) {
    out_gate.Alternate(500);
    out_gate.Print();
    in_gate.GetState();
    System::Delay(50);
  }
}  
