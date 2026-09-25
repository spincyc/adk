#pragma once

#include "notes.h"
#include "object.h"

namespace adk {

    // One note of a melody: a pitch in hertz (note::rest for silence) and a
    // length in milliseconds.
    struct Note
    {
        uint16_t hz;
        uint16_t ms;
    };

    // A passive buzzer or small speaker between the pin and GND. It plays
    // tones and whole melodies while the sketch carries on. Tones use Timer 2,
    // so PWM on pins 9 and 10 cannot be used alongside a Speaker.
    struct Speaker : Object
    {
        explicit Speaker (Pin pin);

        // Sound a pitch, for a duration or, if none is given, until stop ().
        void tone (uint16_t hz, Millis duration = 0);

        // Play a melody. Each note sounds for 7/8 of its length, so repeated
        // notes stay separate.
        template <uint8_t Length>
        void play (const Note (&melody)[Length])
        {
            play (melody, Length);
        }

        void play (const Note* melody, uint8_t length);

        void stop      () override;
        bool isPlaying () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        void start (Note note);

        const Note* melody_;
        Millis      noteStart_;
        Note        note_;
        uint8_t     length_;
        uint8_t     next_;
        Pin         pin_;
        bool        playing_;
        bool        sounding_;
        bool        starting_;
    };
}
