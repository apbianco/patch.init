#include <daisy_patch_sm.h>
#include <daisy.h>

#include "cvrecorder.h"

#include "cv.h"
#include "knobs.h"
#include "led.h"
#include "push_button.h"

using namespace daisy;
using namespace patch_sm;

// The of the application is global so that it can be queried
// anywhere, including in the audio callback.
State global_state;
CVRecorder global_cvrecorder;
CpuLoadMeter loadMeter;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size) {
  loadMeter.OnBlockStart();
  switch (global_state.GetState()) {
  case State::StateValue::PLAYBACK:
    global_cvrecorder.OutSample();
    break;
    
  case State::StateValue::RECORD_WAITING_PRESS:
    global_cvrecorder.OutputMirrorsInput();
    break;
    
  case State::StateValue::RECORDING:
    global_cvrecorder.AddSample();
    break;
  }
  loadMeter.OnBlockEnd();
}

int main(void) {
  InitHardware(true);
  if (!global_cvrecorder.Init()) {
    LOG_FATAL("Initialization failed.");
    return -1;
  }

  // A blocksize of one ensures that AudioCallback is called at 48kHz
  // GetHardware()->SetAudioBlockSize(1);
  loadMeter.Init(GetHardware()->AudioSampleRate(),
		 GetHardware()->AudioBlockSize());
  GetHardware()->StartAudio(AudioCallback);
  global_cvrecorder.Print();

  OnOffPushButton button;
  OnOffPushButton::State button_state;

  LED led;
  led.BlockBlink(3);
  button.SetLED(&led);

  bool report_cpu_load = false;
  
  while(true) {
    auto state = global_state.GetState();
    global_cvrecorder.ReadKnobsAdjustParameters(state);
    switch (state) {
    case State::StateValue::PLAYBACK:
      if (button.GetStateIfChanged(&button_state)) {
	if (button_state.state == OnOffPushButton::StateValue::ON &&
	    button_state.long_press) {
	  global_state.AdvanceTo(State::StateValue::RECORD_WAITING_PRESS);
	}
      }
      break;
    case State::StateValue::RECORD_WAITING_PRESS:
      led.Alternate();
      if (button.GetStateIfChanged(&button_state)) {
	if (button_state.long_press) {
	  global_state.AdvanceTo(State::StateValue::PLAYBACK);
	} else {
	  led.OnHalf();
	  global_cvrecorder.Reset();
	  global_state.AdvanceTo(State::StateValue::RECORDING);
	}
      }
      break;
    case State::StateValue::RECORDING:
      if (button.GetStateIfChanged(&button_state)) {
	global_state.AdvanceTo(State::StateValue::PLAYBACK);
	global_cvrecorder.Print();
	led.Off();
      }
      break;
    default:
      LOG_ERROR("Unexpected state value: %d", global_state.GetState());
    }

    // Report changes (only when they happened)
    global_state.Report();
    // Every second, indicate we're alive and the state we're in.
    if (LOG_INFO_EVERY_MS(1000, "Alive [%s]",
			  global_state.GetStateAsString())) {
      if (report_cpu_load) {
        const float avg = loadMeter.GetAvgCpuLoad();
        const float max = loadMeter.GetMaxCpuLoad();
        const float min = loadMeter.GetMinCpuLoad();
	FB(b1); FB(b2); FB(b3);
	LOG_INFO("Load max: %s, avg: %s, min: %s",
		 f2a(max * 100.0f, b1),
		 f2a(avg * 100.0f, b2),
		 f2a(min * 100.0f, b3));
      }
    }
    System::Delay(0);
  }
}
