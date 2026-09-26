#pragma once

#include "object.h"

namespace adk {

    // A sound sensor module on an analog pin: the Mega kit's, or the 37-in-1
    // kit's big and small ones. The microphone's output wobbles round the
    // middle of its range, further the louder the sound, so level () is how
    // far it swung in the last 50 ms: near 0 in a quiet room, hundreds for a
    // clap nearby.
    //
    //   AO -> an analog pin, + -> 5 V, G -> GND
    //   DO -> not connected, or a Switch: it goes high above the loudness
    //         the module's knob sets
    //
    // It reads the pin four times each update, about half a millisecond, so
    // it hears best when loop () comes round often.
    struct SoundSensor : Object
    {
        explicit SoundSensor (Pin pin);

        // How far the sound swung, from 0 to 1023.
        uint16_t level () const;

        // A new level, every 50 ms.
        bool measured () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        Millis   windowAt_;
        uint16_t level_;
        uint16_t lowest_;
        uint16_t highest_;
        Pin      pin_;
        bool     starting_;
        bool     measured_;
    };
}
