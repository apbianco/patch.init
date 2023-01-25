#include <daisy_patch_sm.h>
#include <daisy.h>

#include "util.h"
#include "gate.h"

// Define ALTERNATE to generate a square wave of a period of 20ms (5ms
// up) or 100 Hz. Otherwise, you get a 20ms pulse with a rest time of
// 1ms
//
// #define ALTERNATE

int main(void) {
  InitHardware(true);

  auto out_gate = CreateOutGate1();
  auto in_gate = CreateInGate1();
  
  out_gate.SetDebug();
  out_gate.Init();
  in_gate.Init();

  bool driver_off = true;
  
  while(true) {
#ifdef ALTERNATE
    out_gate.Alternate(20);
    // in_gate in debug mode so the state is printed...
    in_gate.GetState();
#else
    if (in_gate.GetState() == Gate_::ON && driver_off) {
      out_gate.ArmPulse(20);
      driver_off = false;
    }
    if (in_gate.GetState() == Gate_::OFF) {
      driver_off = true;
    }
    out_gate.UpdatePulse();
#endif
  }
}  
