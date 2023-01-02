#include <daisy_patch_sm.h>
#include <daisy.h>

#include "cv.h"
#include "knobs.h"
#include "led.h"
#include "push_button.h"

using namespace daisy;
using namespace patch_sm;

class State {
public:
  enum StateValue {
    MAIN_LOOP,
    RECORD_WAITING_PRESS,
    RECORDING,
  };
  
  State() : state_(MAIN_LOOP), prev_(MAIN_LOOP), reported_(false) {}

  void AdvanceTo(StateValue v) {
    prev_ = state_;
    state_ = v;
    reported_ = false;
  }

  StateValue GetState() { return state_; }

  const char *GetStateAsString() {
    return StateValueToString(state_);
  }

  void Report() {
    if (! reported_) {
      LOG_INFO("%s --> %s",
	       StateValueToString(prev_), StateValueToString(state_));
      reported_ = true;
    }
  }

private:
  StateValue state_;
  StateValue prev_;
  bool reported_;

  const char *StateValueToString(StateValue v) {
    switch(v) {
    case MAIN_LOOP:
      return "MAIN_LOOP";
    case RECORD_WAITING_PRESS:
      return "RECORD_WAITING_PRESS";
    case RECORDING:
      return "RECORDING";
    default:
      return "???";
    }
  }
};

// The of the application is global so that it can be queried
// anywhere, including in the audio callback.
State global_state;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size) {
  switch (global_state.GetState()) {
  case State::StateValue::MAIN_LOOP:
  case State::StateValue::RECORD_WAITING_PRESS:
    // Play the samples on the first CV out
    break;
  case State::StateValue::RECORDING:
    // This is it, we're sampling CV in and storing the values
    break;
  }
}

int main(void) {
  InitHardware(true);

  OnOffPushButton button;
  OnOffPushButton::State button_state;

  LED led;
  led.BlockBlink(3);
  button.SetLED(&led);
  
  while(true) {
    switch (global_state.GetState()) {
    case State::StateValue::MAIN_LOOP:
      if (button.GetStateIfChanged(&button_state)) {
	if (button_state.state == OnOffPushButton::StateValue::ON &&
	    button_state.long_press) {
	  led.On();
	  global_state.AdvanceTo(State::StateValue::RECORD_WAITING_PRESS);
	}
      }
      break;
    case State::StateValue::RECORD_WAITING_PRESS:
      led.Alternate();
      if (button.GetStateIfChanged(&button_state)) {
	if (button_state.long_press) {
	  global_state.AdvanceTo(State::StateValue::MAIN_LOOP);
	} else {
	  global_state.AdvanceTo(State::StateValue::RECORDING);
	}
      }
      break;
    case State::StateValue::RECORDING:
      // Set the sample rate at 48kHz
      break;
    default:
      LOG_ERROR("Unexpected state value: %d", global_state.GetState());
    }

    global_state.Report();
    LOG_INFO_EVERY_MS(1000, "Alive [%s]", global_state.GetStateAsString());
    System::Delay(0);
  }
}



