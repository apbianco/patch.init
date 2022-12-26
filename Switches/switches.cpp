#include <daisy_patch_sm.h>
#include <daisysp.h>
#include <daisy.h>

#include "util.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

class RadioButton {
public:
  enum State {
    OFF,
    ON,
  };
  enum InternalState {
    WAITING = 1,
    CHANGED,
    RELEASED,
  };
  RadioButton() : state_(OFF), just_changed_(false),
		  internal_state_(RELEASED),
		  long_press_(false),
		  long_press_delay_ms_(1000.0), time_pressed_ms_(0.0),
		  debug_(false) {
    switch_.Init(GetHardware()->B7);
  }

  void SetDebug() { debug_ = true; }
  
  State UpdateState() {
    switch_.Debounce();
    switch (internal_state_) {
    case RELEASED:
      if (switch_.RisingEdge()) {
	internal_state_ = WAITING;
      }
      break;
    case WAITING:
      if (switch_.Pressed()) {
	time_pressed_ms_ = switch_.TimeHeldMs();
	if (time_pressed_ms_ > 10.0) {
	  internal_state_ = CHANGED;
	}
      } else {
	internal_state_ = RELEASED;
      }
      break;
    case CHANGED:
      if (switch_.Pressed()) {
	time_pressed_ms_ = switch_.TimeHeldMs();
      } else {
	internal_state_ = RELEASED;
	state_ = (state_ == ON ? OFF : ON);
	just_changed_ = true;
	if (time_pressed_ms_  > long_press_delay_ms_) {
	  long_press_ = true;
	}
	time_pressed_ms_ = 0.0;
      }
      break;
    }
    if (debug_) {
      LOG_INFO("state: %d, internal_state: %d, long_press: %d",
	       state_, internal_state_, long_press_);
    }
    return state_;
  }

  State GetState() { return state_; }

  void PrintIfChanged() {
    if (just_changed_ || long_press_) {
      Print();
      just_changed_ = false;
      long_press_ = false;
    }
  }
  
  void Print() {
    LOG_INFO("State: %s, long Press: %s",
	     state_ == ON ? "On" : "Off", long_press_ ? "Yes" : "No");
  }
  
private:
  Switch switch_;
  State state_;
  bool just_changed_;
  InternalState internal_state_;
  bool long_press_;
  float long_press_delay_ms_;
  float time_pressed_ms_;
  bool debug_;
};

int main(void)
{
  InitHardware(true);

  RadioButton radio;
  // radio.SetDebug();
  while(true) {
    radio.UpdateState();
    radio.PrintIfChanged();
    System::Delay(2);
  }
}
