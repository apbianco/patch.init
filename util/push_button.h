#ifndef PUSH_BUTTON_H
#define PUSH_BUTTON_H

#include "util.h"
#include "led.h"

class OnOffPushButton {
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
    State(StateValue state, bool long_press):
      state(state), long_press(long_press) {}
    State() : State(OFF, false) {}
    StateValue state;
    bool long_press;
  };
  
  OnOffPushButton() :
    state_(OFF), internal_led_state_(OFF), just_changed_(false),
    internal_state_(RELEASED),
    long_press_(false),
    long_press_delay_ms_(1000.0), time_pressed_ms_(0.0),
    led_(nullptr), debug_(false) {
    switch_.Init(GetHardware()->B7);
  }

  void SetLED(LED *led) {
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
  }

  bool GetStateIfChanged(State *v) {
    UpdateState();
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
    LOG_INFO("State: %s, long press: %s",
	     StateValueToString(s.state), s.long_press ? "Yes" : "No");
  }

  void Print() {
    LOG_INFO("Button: state: %s, long press: %s",
	     StateValueToString(state_), long_press_ ? "Yes" : "No");
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
  LED *led_;
  bool debug_;

  void LogDebug(const char *prefix) {
    if (debug_) {
      FB(b1); FB(b2);
      LOG_INFO(
        "OnOffPushButton: %s: "
	"state: %s, "
	"internal_led_state: %s, "
	"just_changed: %d",
	prefix,
	StateValueToString(state_),
	StateValueToString(internal_led_state_),
	just_changed_);
      LOG_INFO(
        "  internal_state: %s, "
	"long_press: %d, "
	"long_press_delay_ms: %s, "
	"time_pressed_ms: %s",
	InternalStateToString(internal_state_),
	long_press_,
	f2a(long_press_delay_ms_, b1),
	f2a(time_pressed_ms_, b2));
    }
  }

  const char *StateValueToString(StateValue v) {
    return v == ON ? "On" : "Off";
  }

  const char *InternalStateToString(InternalState s) {
    switch(s) {
    case WAITING:
      return "WAITING";
      break;
    case CHANGED:
      return "CHANGED";
      break;
    case RELEASED:
      return "RELEASED";
      break;
    }
    return "???";
  }

};

#endif //  PUSH_BUTTON_H
