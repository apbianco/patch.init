#include <daisy_patch_sm.h>
#include <daisysp.h>
#include <daisy.h>

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM dpsm;

#define PRECISION 10000.0

#define F2I(f) static_cast<int>(PRECISION * f)
#define I2F(i) static_cast<float>(i / PRECISION)

class KnobValue {
public:
  KnobValue(int cv_index, float min_value, float max_value) :
    prev_(0.0f),
    threshold_(min_value),
    factor_(1/(max_value - min_value)) {
    assert (max_value - min_value != 0.0);
    cv_index_ = cv_index;
    prev_ = GetValue();
  }

  void Calibrate() {
    dpsm.PrintLine("CV_%d: %d", cv_index_+1, F2I(GetRawValue()));
  }
  
  bool CaptureValueIndicateChange(float *new_value) {
    *new_value = GetValue();
    int new_int_value = F2I(*new_value);
    if (new_int_value < 0) {
      new_int_value = 0;
      *new_value = 0.0;
    }
    int delta = new_int_value - F2I(prev_);
    if (delta <= -2 || delta >= 2) {
      prev_ = *new_value;
      return true;
    }
    return false;
  }

  void PrintIfChange() {
    float value;
    if (CaptureValueIndicateChange(&value)) {
      Print();
    }
  }

  void Print() {
    dpsm.PrintLine("// CV_%d: %d", cv_index_+1, F2I(prev_));
  }
  
private:
  float prev_;
  float threshold_;
  float factor_;
  int cv_index_;

  float GetRawValue() {
    dpsm.ProcessAnalogControls();
    return dpsm.GetAdcValue(cv_index_);
  }

  float GetValue() {
    return (GetRawValue() - threshold_) * factor_;
  }
};

int main(void)
{
  dpsm.Init();
  dpsm.StartLog(true);		// Wait for the terminal connection
  dpsm.PrintLine("// Started...");

  KnobValue kv(CV_2, I2F(-192), I2F(9961));

  while(true) {
    float f;
    if (kv.CaptureValueIndicateChange(&f)) {
      dpsm.WriteCvOut(CV_OUT_2, f*5.0);
      kv.Print();
    }
    System::Delay(10);
  }
}
