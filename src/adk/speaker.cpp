#include "speaker.h"

#include <Arduino.h>

namespace adk {

    Speaker::Speaker (Pin pin)
        : melody_   (nullptr, 0)
        , started_  ()
        , length_   (0)
        , next_     (0)
        , hz_       (note::rest)
        , pin_      (pin)
        , playing_  (false)
        , sounding_ (false)
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
        if (playing_ && melody_.empty () && hz == hz_ && duration == length_)
        {
            return;
        }

        if (hz == note::rest && duration == 0)
        {
            stop ();
            return;
        }

        melody_ = {nullptr, 0};
        sound (hz, duration);
        started_.restart ();
    }

    void Speaker::play (Span<const Note> melody)
    {
        bool same = !melody.empty () && melody.begin () == melody_.begin ()
                 && melody.size () == melody_.size ();

        if (playing_ && same)
        {
            return;
        }

        melody_ = melody;
        next_   = 0;

        if (nextNote ())
        {
            started_.restart ();
        }
        else
        {
            stop ();
        }
    }

    void Speaker::stop ()
    {
        ::noTone (pin_);
        melody_   = {nullptr, 0};
        playing_  = false;
        sounding_ = false;
    }

    bool Speaker::isPlaying () const
    {
        return playing_;
    }

    void Speaker::update (Millis now)
    {
        if (!playing_)
        {
            return;
        }

        Millis elapsed = started_.elapsed (now);

        // A melody note falls silent for the last eighth of its length.
        if (!melody_.empty () && sounding_ && elapsed >= length_ - length_ / 8)
        {
            ::noTone (pin_);
            sounding_ = false;
        }

        if (length_ == 0 || elapsed < length_)
        {
            return;
        }

        if (nextNote ())
        {
            started_.restart (now);
        }
        else
        {
            stop ();
        }
    }

    // Sound the melody's next note that has a length; false at its end.
    bool Speaker::nextNote ()
    {
        while (next_ < melody_.size ())
        {
            Note note = melody_[next_++];

            if (note.ms != 0)
            {
                sound (note.hz, note.ms);
                return true;
            }
        }

        return false;
    }

    void Speaker::sound (uint16_t hz, Millis length)
    {
        if (hz == note::rest)
        {
            ::noTone (pin_);
        }
        else
        {
            ::tone (pin_, hz);
        }

        hz_       = hz;
        length_   = length;
        playing_  = true;
        sounding_ = hz != note::rest;
    }
}
