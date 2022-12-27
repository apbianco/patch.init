#include <daisy_patch_sm.h>
#include <daisysp.h>
#include <daisy.h>

#include "util.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

static DaisyPatchSM hw_;
static bool debug_ = false;

void InitHardware(bool debug) {
  hw_.Init();
  // Wait for the terminal connection
  debug_ = debug;
  if (debug) {
    hw_.StartLog(true);
  }
  LOG_INFO("Hardware started...");
}

DaisyPatchSM *GetHardware() {
  return &hw_;
}

char *f2a(float f) {
  static char buffer[128];
  snprintf(buffer, 128, FLT_FMT(FLOAT_PRECISION), FLT_VAR(FLOAT_PRECISION, f));
  return buffer;
}
