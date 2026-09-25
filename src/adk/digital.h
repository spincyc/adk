#pragma once

#include "object.h"

namespace adk {

    // Which level means "on". Most parts are active high; a button wired to
    // GND, a common-anode LED, or many relay modules are active low.
    enum Polarity : uint8_t
    {
        ActiveHigh,
        ActiveLow
    };

    // A pin driven high (5 V) or low (0 V).
    struct DigitalOutput : Object
    {
        DigitalOutput (Pin pin);

        void write  (bool high);
        void toggle ();
        bool isHigh () const;
        Pin  pin    () const;

      protected:
        void setup () override;
        void stop  () override;

      private:
        Pin  pin_;
        bool high_;
    };

    // A pin read as high or low, optionally held high by the internal pull-up.
    struct DigitalInput : Object
    {
        DigitalInput (Pin pin, bool pullUp = false);

        bool read () const;
        Pin  pin  () const;

      protected:
        void setup () override;

      private:
        Pin  pin_;
        bool pullUp_;
    };
}
