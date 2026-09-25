#pragma once

#include "debouncer.h"
#include "digital.h"

namespace adk {

    // A switch or an on/off sensor module: tilt, reed, PIR motion, obstacle,
    // flame, sound, touch or line-tracking. Changes are debounced, and
    // activated () and deactivated () are true for one update after a change.
    struct Switch : Object
    {
        // Active low turns on the internal pull-up, for a switch wired to GND.
        explicit Switch (Pin pin, Polarity polarity = ActiveLow, uint8_t debounce = 20);

        bool isActive    () const;
        bool activated   () const;
        bool deactivated () const;
        Pin  pin         () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        bool read () const;

        Debouncer debouncer_;
        Pin       pin_;
        uint8_t   debounce_;
        Polarity  polarity_;
        bool      activated_;
        bool      deactivated_;
    };

    // A push button wired from the pin to GND. wasPressed () and
    // wasReleased () are true for one update after the button moves.
    struct Button : Switch
    {
        explicit Button (Pin pin, uint8_t debounce = 20);

        bool isPressed   () const;
        bool wasPressed  () const;
        bool wasReleased () const;
    };
}
