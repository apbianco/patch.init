#include <daisy_patch_sm.h>
#include <daisy.h>

#include "cv.h"
#include "knobs.h"
#include "led.h"
#include "push_button.h"

using namespace daisy;
using namespace patch_sm;

int main(void) {
  InitHardware(true);

  OnOffPushButton button;
  OnOffPushButton::State button_state;

  LED led;
  led.Blink(3);
  button.SetLED(&led);
  while(true) {
    if (button.GetStateIfChanged(&button_state)) {
      button.Print();
      if (button_state.state == OnOffPushButton::StateValue::ON &&
	  button_state.long_press) {
	LOG_INFO("Start recording");
      }
    }
    LOG_INFO_EVERY_MS(2000, "Alive...");
    System::Delay(0);
  }
}
