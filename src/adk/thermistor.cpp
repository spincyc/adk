#include "thermistor.h"

#include <Arduino.h>
#include <math.h>

namespace adk {

    namespace {

        const Millis SamplePeriod  = 100;
        const float  NominalKelvin = 298.15f;
        const float  ZeroCelsius   = 273.15f;
    }

    Thermistor::Thermistor (Pin pin, float beta, float nominalOhms, float seriesOhms)
        : smoother_    ()
        , beta_        (beta)
        , nominalOhms_ (nominalOhms)
        , seriesOhms_  (seriesOhms)
        , celsius_     (0)
        , sampledAt_   (0)
        , pin_         (pin)
        , starting_    (false)
    {
    }

    void Thermistor::setup ()
    {
        if (claimAnalog (pin_))
        {
            smoother_ = Smoother ();
            starting_ = true;
            sample ();
        }
    }

    void Thermistor::update (Millis now)
    {
        // setup () took the first sample; the next comes a period after the
        // first update.
        if (starting_)
        {
            sampledAt_ = now;
            starting_  = false;
            return;
        }

        if (now - sampledAt_ >= SamplePeriod)
        {
            sampledAt_ = now;
            sample ();
        }
    }

    float Thermistor::celsius () const
    {
        return celsius_;
    }

    float Thermistor::fahrenheit () const
    {
        return celsius_ * 9 / 5 + 32;
    }

    // The thermistor's resistance from the divider, then its temperature from
    // the beta equation: 1/T = 1/T25 + ln (R / R25) / beta, in kelvin.
    void Thermistor::sample ()
    {
        uint16_t reading = smoother_.add (static_cast<uint16_t> (analogRead (pin_)));

        // At 0 and 1023 the resistance would be infinite or zero.
        float level  = (reading < 1) ? 1.0f : (reading > 1022) ? 1022.0f : reading;
        float ohms   = seriesOhms_ * (1023 / level - 1);
        float kelvin = 1 / (1 / NominalKelvin + log (ohms / nominalOhms_) / beta_);

        celsius_ = kelvin - ZeroCelsius;
    }
}
