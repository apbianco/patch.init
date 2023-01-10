#include <daisy_patch_sm.h>
#include <daisy.h>

#include "util.h"
#include "toggle_switch.h"

int main(void) {
  InitHardware(true);

  ToggleSwitch t;
  t.Init();

  while(true) {
    t.GetState();
    t.Print();
    System::Delay(50);
  }
}  
