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
  while(true) {
    OnOffPushButton::State s;
    if (button.GetStateIfChanged(&s)) {
      button.PrintState(s);
    }
    System::Delay(0);
  }
}
