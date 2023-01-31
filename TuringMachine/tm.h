#ifndef TM_H
#define TM_H

#include "util.h"

class ShiftRegister {
 public:
 ShiftRegister(uint32_t size): length_(size), bits_(nullptr),
    initialized_(false) {}

  bool Init() {
    auto num_bytes = length_ * sizeof(bool);
    bits_ = static_cast<bool *>(malloc(num_bytes));
    if (bits_ == nullptr) {
      LOG_ERROR("Can't allocate %d bytes", num_bytes);
    } else {
      initialized_ = true;
    }
    return initialized_;
  }

 private:
  uint32_t length_;
  bool *bits_;			// No efficient but it doesn't matter
  bool initialized_;
};

#endif  // TM_H
