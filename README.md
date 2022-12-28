# patch.init
 Electrosmith's patch.Init() projects

## util/

- `util.h`: `LOG_{INFO,WARN,ERROR}` helper functions. Logging can be
  entirely disabled by not defining the `LOG_ON` macro
  (default). `f2a` methods help convert a float to a string for
  logging, with `FLOAT_PRECISION` digits.

- `knobs.h`: Create individual `Knob` class instances with range
   compentation. `PrintIfChange` method allows you to obtain a reading
   of the knobs and can be used to adjust compensation values to your
   own device (use the raw values for the knob fully CCW, at mid
   course and fully CW.)

- `led.h`: Use to control the visible LED
  (`CV_OUT_2`/`C1`). `MemorizeVoltage()` and
  `RestoreMemorizedVoltage()` methods can be used to share the LED
  between implemented functionalities. The LED can for instance be
  used to meter a value (via the emitted light intensitty) then
  temporarily made to flash to indicate that a button is pressed and
  restored to its previous light intensity value.

  The input range (which can be choosen at instanciation time) is
  translated to a voltage that actually lights to LED from dimmest to
  brighest. Compensation values for your device can vary but are easy
  to measure.

- `transcaler.h`: Used to translate a scale a value from [v1, v2] to
  [V1, V2] in a linear fashion. Extremely handy to rectify the
  pentiometer values that are never exactly at 0.0 CCW, 1.0 CW and 0.5
  at their middile position or to properly drive the `LED` device
  described above.

- `cv.h`: Define the `CVIn` and `CVOut` classes. `CVOut` instances are
  compensated for their offset to truely map from -5.0v to +5.0V.



