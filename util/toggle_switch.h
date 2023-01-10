#ifndef TOGGLE_SWITCH_H
#define TOGGLE_SWITCH_H

#include "hid/switch.h"
#include "util.h"

class ToggleSwitch {
 public:
  // Define a class to inherit a state from
  enum State {
    UP,
    DOWN,
    UNKNOWN,
  };

  ToggleSwitch() = default;

  void Init() {
    s_.Init(GetHardware()->B8);
    GetState();
  }

  State GetState() {
    s_.Debounce();
    state_ = s_.Pressed() ? UP : DOWN;
    return state_;
  }

  void Print() {
    LOG_INFO("ToggleSwitch position: %s", StateName(state_));
  }
  
 private:
  State state_;
  Switch s_;

  const char* StateName(State state) {
    switch (state) {
    case UP:
      return "UP";
    case DOWN:
      return "DOWN";
    case UNKNOWN:
      return "UNKNOWN";
    }
    return "???";
  }
};

#endif //  TOGGLE_SWITCH_H
