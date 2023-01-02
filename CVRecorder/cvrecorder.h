#ifndef CVRECORDER_H
#define CVRECORDER_H

#include "util.h"
#include "knobs.h"
#include "transcaler.h"
#include "cv.h"
#include "led.h"

class State {
public:
  enum StateValue {
    MAIN_LOOP,
    RECORD_WAITING_PRESS,
    RECORDING,
  };
  
 State() :
  state_(MAIN_LOOP), prev_(MAIN_LOOP),
    reported_(false), prev_clock_(System::GetNow()) {}

  void AdvanceTo(StateValue v) {
    prev_ = state_;
    prev_clock_ = System::GetNow() - prev_clock_;
    state_ = v;
    reported_ = false;
  }

  StateValue GetState() { return state_; }

  const char *GetStateAsString() {
    return StateValueToString(state_);
  }

  void Report() {
    if (! reported_) {
      LOG_INFO("%s (%s ms)--> %s",
	       StateValueToString(prev_),
	       f2a(prev_clock_),
	       StateValueToString(state_));
      reported_ = true;
    }
  }

private:
  StateValue state_;
  StateValue prev_;
  bool reported_;
  float prev_clock_;

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

float DSY_SDRAM_BSS *cv_samples_ = nullptr;

class CVRecorder {
 public:
 CVRecorder(uint32_t seconds=2, uint32_t sample_rate=48000) :
    sample_rate_(sample_rate),
    buffer_size_(sample_rate * seconds),
    record_index_(0),
    play_index_increment_(1), recorded_length_(0),
    amplitude_(1.0f), 
    recording_knob_(CreateKnob1()), amplitude_knob_(CreateKnob2()),
    speed_knob_(CreateKnob4()),
    speed_backward_(Transcaler(0.0f, 0.5f, -9.0f, -1.0f)),
    speed_forward_(Transcaler(0.5f, 1.0f, 1.0f, 9.0f)),
    cv_out_(CVOut(Transcaler(0.0f, 1.0f, 0.0f, 5.0f))) {}

  void Reset() {
    record_index_ = 0;
    play_index_ = 0;
    play_index_increment_ = 1;
    const size_t num_bytes = buffer_size_ * sizeof(float);
    memset(cv_samples_, 0, num_bytes);
  }
    
  bool Init() {
    const size_t num_bytes = buffer_size_ * sizeof(float);
    cv_samples_ = static_cast<float *>(malloc(num_bytes));
    if (cv_samples_ == nullptr) {
      LOG_ERROR("Can't allocate %d bytes", num_bytes);
      return false;
    }
    Reset();
    return true;
  }

  ~CVRecorder() {
    free(cv_samples_);
  }

  // Add a sample and retur true when we've started to write again
  // at the beginning of the buffer.
  inline void AddSample() {
    float v = recording_knob_.GetCalibratedValue();
    // This isn't the most defensive but pretty safe unless
    // record_index_ has been accidentaly overwritten.
    cv_samples_[record_index_++] = v;
    if (recorded_length_ < buffer_size_) {
      recorded_length_ += 1;
    }
    if (record_index_ >= buffer_size_) {
      record_index_ = 0;
      LOG_INFO("Reset");
    }
    // Mirror the value to the CV out.
    cv_out_.SetVoltage(v);
  }

  void OutSample() {
    if (cv_samples_ == nullptr) {
      cv_out_.SetVoltage(0.0f);
      return;
    }
    cv_out_.SetVoltage(amplitude_ * cv_samples_[play_index_]);
    play_index_ += play_index_increment_;
    if (play_index_ < 0) {
      play_index_ =
	recorded_length_ > abs(play_index_increment_) ?
	recorded_length_ - 1 : 0;
    } else {
      if (play_index_ >= recorded_length_) {
	play_index_ = 0;
      }
    }
  }

  void OutputMirrorsInput() {
    cv_out_.SetVoltage(amplitude_ * recording_knob_.GetCalibratedValue());
  }

  void ReadKnobsAdjustParameters() {
    float amplitude;
    bool changed = false;
    // If the amplitude knob has changed, use its value as a multiplier
    if (amplitude_knob_.GetCalibratedValueAndIndicateChange(&amplitude)) {
      amplitude_ = amplitude;
      changed = true;
    }
    float index_increment;
    if (speed_knob_.GetCalibratedValueAndIndicateChange(&index_increment)) {
      if (index_increment >= 0.5f) {
	play_index_increment_ = speed_forward_.Transcale(index_increment);
      } else {
	play_index_increment_ = speed_backward_.Transcale(index_increment);
      }
      changed = true;
    }
    if (changed) {
      Print(true);
      if (play_index_increment_ == 1) {
	LED led;
	led.BlockBlink(2, 50);
      }
    }

  }

  float RecordLengthInMilliseconds() {
    return (1000.0f * recorded_length_) / GetHardware()->AudioSampleRate();
  }

  void Print(bool abreviated=false) {
    LOG_INFO("sample_rate_: %d, buffer_size_: %d, "
	     "record_index_: %d, recorded_length_: %d",
	     sample_rate_, buffer_size_, record_index_, recorded_length_);
    LOG_INFO("play_index_increment_; %d, amplitude_: %s",
	     play_index_increment_, f2a(amplitude_));
    LOG_INFO("time recorded: %s [ms]", f2a(RecordLengthInMilliseconds()));
    if (abreviated) {
      return;
    }
    for (uint32_t i = 0; i < 20; i += 4) {
      FB(b1); FB(b2); FB(b3); FB(b4);
      LOG_INFO("%02d: %s | %02d: %s | %02d: %s | %02d: %s",
	       i+0, f2a(cv_samples_[i+0], b1),
	       i+1, f2a(cv_samples_[i+1], b2),
	       i+2, f2a(cv_samples_[i+2], b3),
	       i+3, f2a(cv_samples_[i+4], b4));
    }
  }

 private:
  const uint32_t sample_rate_;
  const int32_t buffer_size_;
  int32_t record_index_;
  int play_index_;
  int32_t play_index_increment_;
  int32_t recorded_length_;
  float amplitude_;
  Knob recording_knob_;
  Knob amplitude_knob_;
  Knob speed_knob_;
  Transcaler speed_backward_;
  Transcaler speed_forward_;
  CVOut cv_out_;
};

#endif //  CVRECORDER_H
