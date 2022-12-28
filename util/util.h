// Utility functions to:
//
// - Initialize the hardware and access the hardware global variable
// - Convert float string
// - Log info, warning or error (conditionally compiled)

#ifndef UTIL_H
#define UTIL_H

#include <daisy_patch_sm.h>

using namespace daisy;
using namespace patch_sm;

// Precision used to print floats
#define FLOAT_PRECISION 5
// Declare a standard buffer to print floats
#define FB(X) char X[128]

// Hardware initialization with the choice of issuing debug
// statements. Once initialize, the hardware is accessible via
// the GetHardware() function.
void InitHardware(bool debug);
DaisyPatchSM *GetHardware();

// Format a float to a static buffer and return the address of that
// buffer. The buffer is not duplicated so if you use this function
// twice on a LOG_* invocation for instance, the result is undefined.
char *f2a(float f);
// Same thing into a declared buffer.
char *f2a(float f, char buffer[128]);
  
// Right now, this can't be moved into the C++ file, the template
// instancation won't work.
#ifdef LOG_ON
template <typename... VA>
static void LOG_(const char *format, const char *what, VA... va) {
  char new_format[128];
  GetHardware()->PrintLine(strcat(strcpy(new_format, what), format), va...);
}
#else
template <typename... VA>
static void LOG_(const char* format, const char *what, VA... va) {}
#endif //  LOG_ON

// Log with //, !! or ** prefixes.
template <typename... VA>
void LOG_INFO(const char *format, VA... va) {
  LOG_(format, "// ", va...);
}
template <typename... VA>
void LOG_WARN(const char *format, VA... va) {
    LOG_(format, "!! ", va...);
}
template <typename... VA>
void LOG_ERROR(const char *format, VA... va) {
  LOG_(format, "** ", va...);
}

template <typename... VA>
void LOG_EVERY_MS_INFO(size_t delay_ms, const char *format, VA... va) {
  static uint32_t last = System::GetNow();
  uint32_t now = System::GetNow();
  if (now - last > delay_ms) {
    last = now;
    LOG_INFO(format, va...);
  }
}

#endif
