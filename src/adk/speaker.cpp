#include "speaker.h"

#include <Arduino.h>

namespace adk {

    Speaker::Speaker (Pin pin)
        : melody_    (nullptr)
        , noteStart_ (0)
        , note_      ({0, 0})
        , length_    (0)
        , next_      (0)
        , pin_       (pin)
        , playing_   (false)
        , sounding_  (false)
        , starting_  (false)
    {
    }

    void Speaker::setup ()
    {
        if (claimOutput (pin_))
        {
            claimTimer (2, pin_);
        }
    }

    void Speaker::tone (uint16_t hz, Millis duration)
    {
        melody_ = nullptr;
        length_ = 0;
        start ({hz, static_cast<uint16_t> (duration)});
    }

    void Speaker::play (Span<const Note> melody)
    {
        if (melody.empty ())
        {
            stop ();
            return;
        }

        melody_ = melody.begin ();
        length_ = static_cast<uint8_t> (min (melody.size (), size_t {255}));
        next_   = 1;
        start (melody[0]);
    }

    void Speaker::stop ()
    {
        ::noTone (pin_);
        melody_   = nullptr;
        length_   = 0;
        playing_  = false;
        sounding_ = false;
        starting_ = false;
    }

    bool Speaker::isPlaying () const
    {
        return playing_;
    }

    void Speaker::update (Millis now)
    {
        if (!playing_ || note_.ms == 0)
        {
            return;
        }

        if (starting_)
        {
            noteStart_ = now;
            starting_  = false;
        }

        Millis elapsed = now - noteStart_;

        // A melody note falls silent for the last eighth of its length.
        if (melody_ && sounding_ && elapsed >= note_.ms - note_.ms / 8u)
        {
            ::noTone (pin_);
            sounding_ = false;
        }

        if (elapsed < note_.ms)
        {
            return;
        }

        if (next_ < length_)
        {
            start (melody_[next_++]);
            noteStart_ = now;
            starting_  = false;
        }
        else
        {
            stop ();
        }
    }

    void Speaker::start (Note note)
    {
        if (note.hz == note::rest)
        {
            ::noTone (pin_);
        }
        else
        {
            ::tone (pin_, note.hz);
        }

        note_     = note;
        playing_  = true;
        sounding_ = note.hz != note::rest;
        starting_ = true;
    }
}
