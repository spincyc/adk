#pragma once

#include "analog.h"

namespace adk {

    // The kit's 10 kilohm NTC thermistor, whose resistance falls as it warms,
    // in a divider with a 10 kilohm resistor: 5 V to one thermistor leg, the
    // other leg to an analog pin (A0-A15), and from that pin the resistor to
    // GND. The warmer it is, the higher the pin's voltage.
    //
    // The pin is read every 100 ms and smoothed, so celsius () is steady and
    // quick to call. A reading of 0 (the thermistor unplugged) or 1023 (its
    // legs shorted) is taken as 1 or 1022, which shows about -77 C or 352 C
    // with the defaults: neither is a real temperature.
    struct Thermistor : Object
    {
        // beta and nominalOhms, the resistance at 25 C, come from the
        // thermistor's datasheet; seriesOhms is the fixed resistor.
        explicit Thermistor (Pin pin, float beta = 3950, float nominalOhms = 10000,
                             float seriesOhms = 10000);

        float celsius    () const;
        float fahrenheit () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        void sample ();

        Smoother smoother_;
        float    beta_;
        float    nominalOhms_;
        float    seriesOhms_;
        float    celsius_;
        Millis   sampledAt_;
        Pin      pin_;
        bool     starting_;
    };
}
