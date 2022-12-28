#include <daisy_patch_sm.h>
#include <daisy.h>

#include "util.h"
#include "visible_led.h"

using namespace daisy;
using namespace patch_sm;

class RadioButton {
public:
  enum StateValue {
    OFF,
    ON,
  };
  enum InternalState {
    WAITING = 1,
    CHANGED,
    RELEASED,
  };
  struct State {
    State() : state(OFF), long_press(false) {}
    StateValue state;
    bool long_press;
  };
  RadioButton() : state_(OFF), internal_led_state_(OFF), just_changed_(false),
		  internal_state_(RELEASED),
		  long_press_(false),
		  long_press_delay_ms_(1000.0), time_pressed_ms_(0.0),
		  led_(nullptr),
		  debug_(false) {
    switch_.Init(GetHardware()->B7);
  }

  void SetLED(VisibleLED *led) {
    led_ = led;
    led_->MemorizeVoltage();
  }
  void SetDebug() { debug_ = true; }
  
  void UpdateState() {
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
	if (switch_.TimeHeldMs() > 10.0) {
	  internal_state_ = CHANGED;
	}
      } else {
	internal_state_ = RELEASED;
      }
      break;
    case CHANGED:
      if (switch_.Pressed()) {
	time_pressed_ms_ = switch_.TimeHeldMs();
	// If we're getting into a long press and if we've chosen to,
	// blink the LED every 20 cycles to indicate that.
	if (led_ != nullptr) {
	  if (time_pressed_ms_  > long_press_delay_ms_) {
	    if (static_cast<int>(time_pressed_ms_) % 20 == 0) {
	      internal_led_state_ = internal_led_state_ == OFF ? ON : OFF;
	      led_->SetVoltage(internal_led_state_ == ON ? 2.5 : 0.0);
	    }
	  }
	}
      } else {
	internal_state_ = RELEASED;
	state_ = state_ == ON ? OFF : ON;
	just_changed_ = true;
	if (time_pressed_ms_  > long_press_delay_ms_) {
	  long_press_ = true;
	}
	time_pressed_ms_ = 0.0;
	if (led_ != nullptr) {
	  led_->RestoreMemorizedVoltage();
	}
      }
      break;
    }
    if (debug_) {
      LOG_INFO("state: %d, internal_state: %d, long_press: %d",
	       state_, internal_state_, long_press_);
    }
  }

  bool GetStateIfChanged(State *v) {
    if (just_changed_ || long_press_) {
      v->state = state_;
      v->long_press = long_press_;
      just_changed_ = false;
      long_press_ = false;
      return true;
    }
    return false;
  }

  void PrintState(const State& s) {
    LOG_INFO("State: %s, long Press: %s",
	     s.state == ON ? "On" : "Off", s.long_press ? "Yes" : "No");
  }
  
private:
  Switch switch_;
  StateValue state_;
  StateValue internal_led_state_;
  bool just_changed_;
  InternalState internal_state_;
  bool long_press_;
  float long_press_delay_ms_;
  float time_pressed_ms_;
  VisibleLED *led_;
  bool debug_;
};

int main(void)
{
  InitHardware(true);

  VisibleLED l;
  l.SetVoltage(1.5);
  RadioButton radio;
  radio.SetLED(&l);
  // radio.SetDebug();
  while(true) {
    RadioButton::State s;
    radio.UpdateState();
    if (radio.GetStateIfChanged(&s)) {
      radio.PrintState(s);
    }
    System::Delay(2);
  }
}
