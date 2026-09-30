#include "every.h"

namespace adk {

    Every::Every (Millis period)
        : beat_   ()
        , period_ (period)
        , ticked_ (false)
    {
    }

    bool Every::ticked () const
    {
        return ticked_;
    }

    // A beat already reported stays reported until the next update.
    void Every::restart ()
    {
        beat_.restart ();
    }

    void Every::period (Millis period)
    {
        period_ = period;
    }

    Millis Every::period () const
    {
        return period_;
    }

    // The first beat comes one period after the first update.
    void Every::update (Millis now)
    {
        ticked_ = beat_.beat (now, period_);
    }
}
