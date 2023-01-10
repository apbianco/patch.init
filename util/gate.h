#ifndef GATE_H
#define GATE_H

#include <daisy_patch_sm.h>
#include <daisy.h>

#include "hid/gatein.h"

class Gate_ {
 public:  
  enum State {
    ON,
    OFF,
    UNKNOWN,
  };
  Gate_(int32_t gate_index) :
    gate_index_(gate_index),
    last_transition_time_(System::GetNow()), state_(OFF),
    initialized_(false), debug_(false) {
    if (gate_index_ < 1 || gate_index_ > 2) {
      LOG_WARN("Gate index %d out of range, defaulting to 1", gate_index_);
      gate_index_ = 1;
    }
  }

  virtual ~Gate_() = default;

  void SetDebug() { debug_ = true; }

  inline State GetState() { return state_; }

  virtual void Print() {}

protected:

  inline State AdjustInternalState(State s) {
    state_ = s;
    last_transition_time_ = System::GetNow();
    if (debug_) {
      Print();
    }
    return state_;
  }
  
  int32_t gate_index_;
  uint32_t last_transition_time_;
  State state_;
  bool initialized_;
  bool debug_;

  const char* StateName(State state) {
    switch (state) {
    case ON:
      return "ON";
    case OFF:
      return "OFF";
    case UNKNOWN:
      return "UNKNOWN";
    }
    return "???";
  }
};

class InGate : public Gate_ {
 public:
  InGate(int gate_index): Gate_(gate_index) {}

  void Init() {
    gate_.Init(gate_index_ == 1 ? 
	       (dsy_gpio_pin *)&DaisyPatchSM::B10 : 
	       (dsy_gpio_pin *)&DaisyPatchSM::B9, true);
    GetState();
    initialized_ = true;
  }

  State GetState() {
    if (!initialized_) {
      LOG_ERROR("Can't get state, not initialized");
      return UNKNOWN;
    }
    return AdjustInternalState(gate_.State() ? ON : OFF);
  }
  
  void Print() {
    LOG_INFO("InGate #%d: state:  %s", gate_index_, StateName(state_));
  }

private:
  GateIn gate_;
};

class OutGate : public Gate_ {
 public:
  OutGate(int gate_index): Gate_(gate_index) {}

  void Init() {
    gate_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_.pull = DSY_GPIO_NOPULL;
    // FIXME: This should be fixed with
    // https://github.com/electro-smith/libDaisy/pull/558 being pulled
    // in.
    gate_.pin = (gate_index_ == 1 ?
		 DaisyPatchSM::B6 : DaisyPatchSM::B5);
    dsy_gpio_init(&gate_);
    SetState(OFF);
    initialized_ = true;
  }
  
  inline void On() { SetState(ON); }
  inline void Off() { SetState(OFF); }

  void Alternate(float delay_ms = 500) {
    if (System::GetNow() - last_transition_time_ > delay_ms) {
      SetState(state_ == ON ? OFF : ON);
    }
  }

  inline void SetState(State state) {
    if (!initialized_) {
      LOG_ERROR("Can't set state, not initialized");
      return;
    }
    dsy_gpio_write(&gate_, state == ON ? true : false);
    AdjustInternalState(state);
  }

  void Print() {
    LOG_INFO("OutGate #%d: state: %s", gate_index_, StateName(state_));
  }

private:
  dsy_gpio gate_;
};

InGate CreateInGate1() { return InGate(1); }

InGate CreateInGate2() { return InGate(2); }

OutGate CreateOutGate1() { return OutGate(1); }

OutGate CreateOutGate2() { return OutGate(2); }

#endif  // GATE_H
