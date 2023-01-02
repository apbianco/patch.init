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
  case State::StateValue::MAIN_LOOP:
  case State::StateValue::RECORD_WAITING_PRESS:
    global_cvrecorder.OutSample();
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
    LOG_ERROR("Aborting");
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
    global_cvrecorder.ReadKnobsAdjustParameters();
    switch (global_state.GetState()) {
    case State::StateValue::MAIN_LOOP:
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
	  global_state.AdvanceTo(State::StateValue::MAIN_LOOP);
	} else {
	  led.OnHalf();
	  global_cvrecorder.Reset();
	  global_state.AdvanceTo(State::StateValue::RECORDING);
	}
      }
      break;
    case State::StateValue::RECORDING:
      if (button.GetStateIfChanged(&button_state)) {
	global_state.AdvanceTo(State::StateValue::MAIN_LOOP);
	global_cvrecorder.Print();
	led.Off();
      }
      break;
    default:
      LOG_ERROR("Unexpected state value: %d", global_state.GetState());
    }

    global_state.Report();
    if (LOG_INFO_EVERY_MS(1000, "Alive [%s]",
			  global_state.GetStateAsString())) {
      if (report_cpu_load) {
        // get the current load (smoothed value and peak values)
        const float avgLoad = loadMeter.GetAvgCpuLoad();
        const float maxLoad = loadMeter.GetMaxCpuLoad();
        const float minLoad = loadMeter.GetMinCpuLoad();
        // print it to the serial connection (as percentages)
        LOG_INFO("Processing Load %:");
        LOG_INFO("Max: " FLT_FMT3, FLT_VAR3(maxLoad * 100.0f));
        LOG_INFO("Avg: " FLT_FMT3, FLT_VAR3(avgLoad * 100.0f));
        LOG_INFO("Min: " FLT_FMT3, FLT_VAR3(minLoad * 100.0f));
      }
    }
    System::Delay(0);
  }
}
