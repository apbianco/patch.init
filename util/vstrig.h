#ifndef VSTRIG_H
#define VSTRIG_H

#include <daisy_patch_sm.h>
#include <daisy.h>

#include "hid/gatein.h"

using namespace daisy;
using namespace patch_sm;

class VSTrig {
 public:
  VSTrig(int32_t low_state_time_ms=-1):
    low_state_time_ms_(low_state_time_ms),
    low_state_time_start_ms_(0.0f),
    v_trigger_state_(VD),
    s_trigger_state_(SU),
    debug_(false) {}

  void Init() {
    v_trigger_.Init((dsy_gpio_pin *)&DaisyPatchSM::B9, true);
    s_trigger_.mode = DSY_GPIO_MODE_ANALOG;
    s_trigger_.pull = DSY_GPIO_PULLDOWN;
    s_trigger_.pin = DaisyPatchSM::B6;
    dsy_gpio_init(&s_trigger_);
    dsy_gpio_write(&s_trigger_, s_trigger_state_);
  }

  void Print() {
    if (debug_) {
      LOG_INFO_EVERY_MS(20, "v_trigger: %s, s_tringger: %s",
			StateToString(v_trigger_state_),
			StateToString(s_trigger_state_));
    }
  }

  inline void UpdateState() {
    Print();
    switch (s_trigger_state_) {
    case VD:
      break;
    case SD:
      dsy_gpio_write(&s_trigger_, false);
      break;
    case VU:
      break;
    case SU:
      dsy_gpio_write(&s_trigger_, true);
      break;
    }
  }

  void Process() {
    // VD|SU + V down --> VU|SD
    if (v_trigger_state_ == VD && s_trigger_state_ == SU) {
      if (v_trigger_.State() == true) {
	v_trigger_state_ = VU;
	s_trigger_state_ = SD;
	low_state_time_start_ms_ = System::GetNow();
      }
      goto done;
    }
    // 1- VU|SD + V down     --> VD|SU
    // 2- VU|SD + Δt if ∃ Δt --> VU|SU
    if (v_trigger_state_ == VU && s_trigger_state_ == SD) {
      if (v_trigger_.State() == false) {
	v_trigger_state_ = VD;
	s_trigger_state_ = SU;
      }
      if (low_state_time_ms_ > 0 &&
	  System::GetNow() - low_state_time_start_ms_ > low_state_time_ms_) {	
	s_trigger_state_ = SU;
      }
      goto done;
    }
    // VU|SU + V down --> VD|SU
    if (v_trigger_state_ == VU && s_trigger_state_ == SU) {
      if (v_trigger_.State() == false) {
	v_trigger_state_ = VD;
      }
      goto done;
    }
    // VU|SU is not attainable.
    if (v_trigger_state_ == VU && s_trigger_state_ == SU) {
      LOG_ERROR("VU|SU can't exist");
      goto done;
    }
  done:
    UpdateState();	   
  }

 private:

  enum State {
    VD,			 // V-trigger down,
    SD,			 // S-trigger down,
    VU,			 // V-trigger up,
    SU,			 // S-trigger up
  };

  const char *StateToString(State s) {
    switch(s) {
    case VD: return "VD"; break;
    case SD: return "SD"; break;
    case VU: return "VU"; break;
    case SU: return "SU"; break;
    }
    return "???";
  }

  int32_t low_state_time_ms_;
  float low_state_time_start_ms_;
  
  State v_trigger_state_;
  State s_trigger_state_;

  GateIn v_trigger_;
  dsy_gpio s_trigger_;
  bool debug_;
};

#endif //  VSTRIG_H
