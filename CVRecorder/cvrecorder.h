#ifndef CVRECORDER_H
#define CVRECORDER_H

#include "util.h"
#include "knobs.h"
#include "transcaler.h"
#include "cv.h"
#include "led.h"
#include "gate.h"

// An instance of this class is used to keep track of the state of the
// application. A method exists to advance to a new state, remembering
// where we came from (and how long it took us to move to the new
// state) so that this information can be displayed on demand via the
// Report() method.
class State {
public:
  enum StateValue {
    PLAYBACK,
    RECORD_WAITING_PRESS,
    RECORDING,
  };
  
 State() :
  state_(PLAYBACK), prev_(PLAYBACK),
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
    case PLAYBACK:
      return "PLAYBACK";
    case RECORD_WAITING_PRESS:
      return "RECORD_WAITING_PRESS";
    case RECORDING:
      return "RECORDING";
    default:
      return "???";
    }
  }
};

// Sampled CV values held in the SDRAM bss section. This pointer will
// be owned by an instance of the CVRecorder class.
float DSY_SDRAM_BSS *cv_samples_ = nullptr;

// A class to manage recording and playing back sampled CV input on
// the CV output. Recording/playing samples back is managed via a
// circular buffer that can be played forward/backward at various
// speed and amplification levels.
class CVRecorder {
 public:
 CVRecorder(uint32_t seconds=2, uint32_t sample_rate=48000) :
  sample_rate_(sample_rate),
    buffer_size_(sample_rate * seconds),
    record_index_(0),
    play_index_increment_(1), first_sample_to_play_(0), recorded_length_(0),
    amplitude_(1.0f), crossed_(false),
    recording_knob_(CreateKnob1()), amplitude_knob_(CreateKnob2()),
    scrub_knob_(CreateKnob3()), speed_knob_(CreateKnob4()),
    speed_backward_(Transcaler(0.0f, 0.5f, -9.0f, -1.0f)),
    speed_forward_(Transcaler(0.5f, 1.0f, 1.0f, 9.0f)),
    cv_out_(CVOut(Transcaler(0.0f, 1.0f, 0.0f, 5.0f))) {}

  // Prepare the instance to start capturing samples again.
  void Reset() {
    record_index_ = 0;
    play_index_ = 0;
    play_index_increment_ = 1;
    first_sample_to_play_ = 0;
    recorded_length_ = 0;
    amplitude_ = 1.0f;
    crossed_ = false;
    const size_t num_bytes = buffer_size_ * sizeof(float);
    memset(cv_samples_, 0, num_bytes);
  }

  // Post constructor initialization of the instance, which can only
  // be called after the rest of the hardware has been initialized.
  bool Init() {
    // Triggering gate is gate #1
    trigger_ = CreateInGate1();
    trigger_.Init();
    
    const size_t num_bytes = buffer_size_ * sizeof(float);
    cv_samples_ = static_cast<float *>(malloc(num_bytes));
    if (cv_samples_ == nullptr) {
      LOG_ERROR("Can't allocate %d bytes", num_bytes);
      return false;
    }
    Reset();
    return true;
  }

  void SetDebug() {
    debug_ = true;
    trigger_.SetDebug();
  }

  ~CVRecorder() {
    free(cv_samples_);
  }

  // Add a sample read from the recording knob. Keep track of where we
  // are in the circular buffer and how much valid data the buffer
  // actualy holds. The sampled value is emitted on the CV out.
  inline void AddSample() {
    if (cv_samples_ == nullptr) {
      LOG_FATAL("Init() hasn't been called");
    }
    float v = recording_knob_.GetCalibratedValue();
    // This isn't the most defensive but pretty safe unless
    // record_index_ has been accidentaly overwritten.
    cv_samples_[record_index_++] = v;
    if (recorded_length_ < buffer_size_) {
      recorded_length_ += 1;
    }
    if (record_index_ >= buffer_size_) {
      record_index_ = 0;
    }
    // Mirror the value to the CV out.
    cv_out_.SetVoltage(v);
  }

  // Reset the play index if necessary, depending on the direction of
  // playback.
  //
  // Forward playback: if the play_index is above the recorded length,
  // set the play index to be the first sample to play (controlled by
  // the scrub knob.)
  //
  // Backward playback: if the play index becomes negative, set the
  // play index to be last sample that can be played.
  inline void ResetOrForceResetPlayIndex(bool force_reset) {
    if (play_index_ < 0 ||
	(force_reset && play_index_increment_ < 0)) {
      play_index_ =
	recorded_length_ > abs(play_index_increment_) ?
	recorded_length_ - 1 : first_sample_to_play_;
      crossed_ = true;
      return;
    }
    if (play_index_ >= recorded_length_ ||
	(force_reset && play_index_increment_ >= 0)) {
      play_index_ = first_sample_to_play_;
      crossed_ = true;
      return;
    }
  }

  // Emit a sample out and manage the circular buffer of samples,
  // taking its read direction into account.
  void OutSample() {
    if (cv_samples_ == nullptr) {
      LOG_FATAL("Init() hasn't been called");
    }
    // Set the voltage out.
    cv_out_.SetVoltage(amplitude_ * cv_samples_[play_index_]);
    // Point to the next sample to play
    play_index_ += play_index_increment_;
    // Handler over- and under-flows. Set crossed_ to true when we
    // went over or under (so that the LED can be flashed to indicate
    // the start of a sequence.)
    ResetOrForceResetPlayIndex(/* force_reset= */false);
  }

  // Write on the CV out what we're reading on the recording knob.
  void OutputMirrorsInput() {
    cv_out_.SetVoltage(amplitude_ * recording_knob_.GetCalibratedValue());
  }

  // Reads all knobs are adjust the parameters conditionning how the
  // circular buffer data is read. Read the first input gate and when
  // the gate as triggered configure the circular buffer to start
  // playing from the beginning (direction dependent)
  //
  // Flashes  the front pannel LED when:
  //
  // - The speed knob is returned to a position that translates into a
  //   forward read at normal speed.
  // - When we playback routine has positionned the crossed_ field,
  //   indicating that we went over or under the circular buffer
  //   capacity.
  void ReadKnobsAdjustParameters(State::StateValue current_state) {
    float amplitude;
    bool changed = false;
    bool play_index_increment_changed = false;
    // If the amplitude knob has changed, use its value as a multiplier
    if (amplitude_knob_.GetCalibratedValueAndIndicateChange(&amplitude)) {
      amplitude_ = amplitude;
      changed = true;
    }
    // If the scrub knob has change, advance to the next sample that
    // should be played and set the begining of the sequence to that
    // value.
    float scrub;
    if (scrub_knob_.GetCalibratedValueAndIndicateChange(&scrub)) {
      first_sample_to_play_ = scrub * recorded_length_;
      if (first_sample_to_play_ < 0) {
	first_sample_to_play_ = 0;
      }
      if (first_sample_to_play_ >= recorded_length_ - play_index_increment_) {
	first_sample_to_play_ = recorded_length_ - play_index_increment_;
      }
    }

    // If the speed knob has changed, map its value to the number of
    // samples we skip in order to loop faster through the buffer.
    float index_increment;
    if (speed_knob_.GetCalibratedValueAndIndicateChange(&index_increment)) {
      if (index_increment >= 0.5f) {
	play_index_increment_ = speed_forward_.Transcale(index_increment);
      } else {
	play_index_increment_ = speed_backward_.Transcale(index_increment);
      }
      play_index_increment_changed = true;
    }
    // If the trigger was activated and changed state, move the
    // reading head at the beginning of the buffer (which is direction
    // dependent.)
    Gate_::State state;		// FIXME, namespace
    if (trigger_.GetStateIfChange(&state) && state == Gate_::ON) {
      ResetOrForceResetPlayIndex(/* force_reset= */true);
    }
    // If we registered a change, print some debug information and if
    // we moved back to playing at normal forward speed, blink the led
    // twice.)
    if (changed) {
      Print(true);
    }
    if (play_index_increment_ == 1 && play_index_increment_changed) {
      LED led;
      led.BlockBlink(3, 20);
    }
    // if we crossed the end of buffer boundary (direction dependant)
    // during playback, blink the light briefly twice. Only do that
    // when we have data in the buffer.
    if (crossed_ &&
	recorded_length_ > 0 &&
	current_state == State::StateValue::PLAYBACK) {
      LED led;
      led.BlockBlink(1, 25);
    }
    crossed_ = false;
  }

  // FIXME: This returns bogus results. Implement this differently.
  float RecordLengthInMilliseconds() {
    return (1000.0f * recorded_length_) / GetHardware()->AudioSampleRate();
  }

  void Print(bool abreviated=false) {
    LOG_INFO("sample_rate_: %d, buffer_size_: %d, "
	     "record_index_: %d, recorded_length_: %d",
	     sample_rate_, buffer_size_, record_index_, recorded_length_);
    LOG_INFO("play_index_increment_: %d, amplitude_: %s",
	     play_index_increment_, f2a(amplitude_));
    LOG_INFO("first_sample_to_play_: %d", first_sample_to_play_);
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
  const uint32_t sample_rate_;	 // Number of samples we're capturing/sec.
  const int32_t buffer_size_;	 // Buffer size we need for recording N sec.
  int32_t record_index_;	 // Where are in recording
  int play_index_;		 // Next sample to play
  int32_t play_index_increment_; // Offset to the next sample to play
  int32_t first_sample_to_play_; // Offset to the first sample to play
  int32_t recorded_length_;      // Index of the last sampled value
  float amplitude_;              // Amplitute multiplier
  bool crossed_;                 // True when reading went over/under
  
  Knob recording_knob_;          // Knob providing input when recording
  Knob amplitude_knob_;          // Knob to change amplitude during playback

  Knob scrub_knob_;		 // Knob setting the replay offset
  Knob speed_knob_;              // Knob to change playback speed/direction
  Transcaler speed_backward_;    // Map speed knob values to bwd playback speed
  Transcaler speed_forward_;     // Map speed knob values to fwd playback speed
  
  CVOut cv_out_;		 // Where samples will be output
  InGate trigger_;               // When triggered, force play from start.

  bool debug_;			 // Set components in debug mode
};

#endif //  CVRECORDER_H
