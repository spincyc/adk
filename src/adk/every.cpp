#include "every.h"

namespace adk {

    Every::Every (Millis period)
        : period_   (period)
        , last_     (0)
        , ticked_   (false)
        , starting_ (true)
    {
    }

    bool Every::ticked () const
    {
        return ticked_;
    }

    // A beat already reported stays reported until the next update.
    void Every::restart ()
    {
        starting_ = true;
    }

    void Every::period (Millis period)
    {
        period_ = period;
    }

    Millis Every::period () const
    {
        return period_;
    }

    void Every::update (Millis now)
    {
        ticked_ = false;

        // The first beat comes one period after the first update.
        if (starting_)
        {
            last_     = now;
            starting_ = false;
            return;
        }

        if (now - last_ >= period_)
        {
            // Keep to the beat, but never try to catch up on missed ones.
            last_   = (now - last_ >= 2 * period_) ? now : last_ + period_;
            ticked_ = true;
        }
    }
}
