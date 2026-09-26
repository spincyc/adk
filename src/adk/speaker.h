#pragma once

#include "containers.h"
#include "notes.h"
#include "timing.h"

namespace adk {

    // One note of a melody: a pitch in hertz (note::rest for silence) and a
    // length in milliseconds.
    struct Note
    {
        uint16_t hz;
        uint16_t ms;
    };

    // A passive buzzer or small speaker, wired from the pin through a 220 ohm
    // resistor to GND. The kit's passive buzzer has a coil of only about
    // 16 ohm, and the resistor keeps the pin's current safe. It plays tones
    // and whole melodies while the sketch carries on. Tones use Timer 2, so
    // PWM on pins 9 and 10 cannot be used alongside a Speaker.
    //
    // Asking again for the tone or the melody already playing changes
    // nothing, so tone () and play () can be called from every pass of
    // loop (); a different one starts at once.
    struct Speaker : Object
    {
        Speaker (Pin pin);

        // Sound a pitch, for a duration or, if none is given, until stop ().
        // note::rest is a silence for the duration, or stops at once.
        void tone (uint16_t hz, Millis duration = 0);

        // Play a melody: an array, an adk::Array or an adk::Vector of notes,
        // which must last until it has played. Each note sounds for 7/8 of
        // its length, so repeated notes stay separate; a note of 0 ms is
        // skipped. A melody that has finished plays again when asked again.
        void play (Span<const Note> melody);

        // A melody made on the spot would be gone before it had played.
        template <size_t N>
        void play (const Note (&&) [N]) = delete;

        template <size_t N>
        void play (const Array<Note, N>&&) = delete;

        template <size_t Capacity>
        void play (const Vector<Note, Capacity>&&) = delete;

        void stop      () override;
        bool isPlaying () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        bool nextNote ();
        void sound    (uint16_t hz, Millis length);

        Span<const Note> melody_;
        StartTime        started_;
        Millis           length_;     // 0: until stop ()
        size_t           next_;
        uint16_t         hz_;
        Pin              pin_;
        bool             playing_;
        bool             sounding_;
    };
}
