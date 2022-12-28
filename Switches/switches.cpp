#include <daisy_patch_sm.h>
#include <daisy.h>

#include "push_button.h"

using namespace daisy;
using namespace patch_sm;

int main(void)
{
  InitHardware(true);

  LED l;
  l.SetVoltage(1.5);
  OnOffPushButton button;
  button.SetLED(&l);
  // button.SetDebug();
  while(true) {
    OnOffPushButton::State s;
    button.UpdateState();
    if (button.GetStateIfChanged(&s)) {
      button.PrintState(s);
    }
    System::Delay(2);
  }
}
