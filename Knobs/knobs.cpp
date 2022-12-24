#include <daisy_patch_sm.h>
#include <daisysp.h>
#include <daisy.h>

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM dpsm;

#define PRECISION 1000.0

#define F2I(f) static_cast<int>(PRECISION * f)

class KnobValue {
public:
  KnobValue(int cv_index) : prev_(0.0f) {
    cv_index_ = cv_index;
    dpsm.ProcessAnalogControls();
    prev_ = dpsm.GetAdcValue(cv_index_);
  }
  
  bool MaybeCaptureNewValue(float *new_value) {
    dpsm.ProcessAnalogControls();
    *new_value = dpsm.GetAdcValue(cv_index_);
    if (F2I(*new_value) != F2I(prev_)) {
      prev_ = *new_value;
      return true;
    }
    return false;
  }

  void PrintIfChange() {
    float value;
    if (MaybeCaptureNewValue(&value)) {
      Print();
    }
  }

  void Print() {
    dpsm.PrintLine("// CV_%d: %d", cv_index_+1, F2I(prev_));
  }
  
private:
  float prev_;
  int cv_index_;
};

int main(void)
{
  dpsm.Init();
  dpsm.StartLog(true);		// Wait for the terminal connection
  dpsm.PrintLine("// Started...");

  KnobValue kv(CV_1);

  while(true) {
    float f;
    if (kv.MaybeCaptureNewValue(&f)) {
      dpsm.WriteCvOut(CV_OUT_2, f*5.0);
      kv.Print();
    }
    System::Delay(10);
  }
}
