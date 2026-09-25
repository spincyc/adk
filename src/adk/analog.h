#pragma once

#include "object.h"

namespace adk {

    // A voltage from 0 V to 5 V on A0-A15, read as 0-1023.
    struct AnalogInput : Object
    {
        AnalogInput (Pin pin);

        uint16_t read () const;

        // The reading scaled to a range, like map (): 0 becomes low, 1023 high.
        long read (long low, long high) const;

        Pin pin () const;

      protected:
        void setup () override;

      private:
        Pin pin_;
    };

    // A pulse-width-modulated output: 0 is always low, 255 always high, and
    // values between switch fast enough to dim an LED or slow a motor.
    struct PwmOutput : Object
    {
        PwmOutput (Pin pin);

        void    write (uint8_t duty);
        uint8_t duty  () const;
        Pin     pin   () const;

      protected:
        void setup () override;
        void stop  () override;

      private:
        Pin     pin_;
        uint8_t duty_;
    };

    // Smooths a jumpy reading. Each sample moves the value 1/2^shift of the
    // way towards itself, so a larger shift is smoother and slower.
    struct Smoother
    {
        explicit Smoother (uint8_t shift = 3);

        uint16_t add   (uint16_t sample);
        uint16_t value () const;

      private:
        uint32_t scaled_;
        uint8_t  shift_;
        bool     primed_;
    };
}
