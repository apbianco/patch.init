#ifndef GATE_H
#define GATE_H

class GateOut {
  enum State {
    ON,
    OFF
  };
 public:
  GateOut(int gate_index):
    gate_index_(gate_index), state_(OFF),
    last_transition_time_(System::GetNow()), debug_(false) {}

  void Init() {
    gate_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_.pull = DSY_GPIO_NOPULL;
    if (gate_index_ < 1 || gate_index_ > 2) {
      LOG_WARN("Gate index %d out of range", gate_index_);
    }
    // FIXME: This should be fixed with
    // https://github.com/electro-smith/libDaisy/pull/558 being pulled
    // in.
    gate_.pin = (gate_index_ == 1 ?
		 DaisyPatchSM::B6 : DaisyPatchSM::B5);
    dsy_gpio_init(&gate_);
    SetState(OFF);
  }
  
  void On() {
    SetState(ON);
  }

  void Off() {
    SetState(OFF);
  }

  State GetState() {
    return state_;
  }

  void Alternate(float delay_ms = 500) {
    if (System::GetNow() - last_transition_time_ > delay_ms) {
      SetState(state_ == ON ? OFF : ON);
    }
  }

  void Print() {
    LOG_INFO("Gate #%d: state: %s", gate_index_,
	     state_ == ON ? "ON" : "OFF");
  }

  inline void SetState(State state) {
    dsy_gpio_write(&gate_, state == ON ? true : false);
    last_transition_time_ = System::GetNow();
    state_ = state;
    if (debug_) {
      Print();
    }
  }

  void SetDebug() { debug_ = true; }
  
 private:
  int32_t gate_index_;
  dsy_gpio gate_;
  State state_;
  uint32_t last_transition_time_;
  bool debug_;
};

GateOut CreateGateOut1() { return GateOut(1); }

GateOut CreateGateOut2() { return GateOut(2); }

#endif
