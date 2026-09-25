#include "debouncer.h"

namespace adk {

    Debouncer::Debouncer (bool initial)
        : changedAt_ (0)
        , raw_       (initial)
        , stable_    (initial)
    {
    }

    bool Debouncer::sample (bool raw, Millis now, uint8_t window)
    {
        if (raw != raw_)
        {
            raw_       = raw;
            changedAt_ = now;
            return false;
        }

        if (raw != stable_ && now - changedAt_ >= window)
        {
            stable_ = raw;
            return true;
        }

        return false;
    }

    bool Debouncer::stable () const
    {
        return stable_;
    }
}
