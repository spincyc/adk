#include "sound_sensor.h"

#include <Arduino.h>

namespace adk {

    namespace {

        constexpr Millis  Window  = 50;         // longer than a low note's wave
        constexpr uint8_t Samples = 4;
    }

    SoundSensor::SoundSensor (Pin pin)
        : level_    (0)
        , lowest_   (1023)
        , highest_  (0)
        , pin_      (pin)
        , measured_ (false)
    {
    }

    void SoundSensor::setup ()
    {
        claimAnalog (pin_);
        window_.restart ();
    }

    uint16_t SoundSensor::level () const
    {
        return level_;
    }

    bool SoundSensor::measured () const
    {
        return measured_;
    }

    void SoundSensor::update (Millis now)
    {
        measured_ = false;

        for (uint8_t sample = 0; sample < Samples; ++sample)
        {
            uint16_t reading = static_cast<uint16_t> (analogRead (pin_));
            lowest_          = min (lowest_, reading);
            highest_         = max (highest_, reading);
        }

        if (window_.elapsed (now) >= Window)
        {
            level_    = highest_ >= lowest_ ? static_cast<uint16_t> (highest_ - lowest_) : 0;
            lowest_   = 1023;
            highest_  = 0;
            measured_ = true;
            window_.restart (now);
        }
    }
}
