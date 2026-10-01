#include "timing.h"

namespace adk {

    StartTime::StartTime ()
        : start_   (0)
        , waiting_ (true)
    {
    }

    void StartTime::restart ()
    {
        waiting_ = true;
    }

    void StartTime::restart (Millis now)
    {
        start_   = now;
        waiting_ = false;
    }

    void StartTime::start (Millis now)
    {
        if (waiting_)
        {
            restart (now);
        }
    }

    Millis StartTime::elapsed (Millis now) const
    {
        return waiting_ ? 0 : now - start_;
    }

    bool StartTime::beat (Millis now, Millis period)
    {
        if (waiting_)
        {
            restart (now);
            return false;
        }

        Millis gone = now - start_;

        if (gone < period)
        {
            return false;
        }

        start_ = (gone - period >= period) ? now : start_ + period;
        return true;
    }

    bool StartTime::due (Millis now, Millis period)
    {
        if (!waiting_ && now - start_ < period)
        {
            return false;
        }

        restart (now);
        return true;
    }

    bool StartTime::passed (Millis now, Millis period)
    {
        start (now);

        if (now - start_ < period)
        {
            return false;
        }

        restart (now);
        return true;
    }

    uint16_t interpolate (uint16_t from, uint16_t to, Millis elapsed, Millis length)
    {
        if (elapsed >= length)
        {
            return to;
        }

        // Halving both keeps their ratio. Once length fits in 16 bits, the
        // distance, 16 bits at most, times elapsed fits in 32.
        while (length > 0xFFFF)
        {
            length  >>= 1;
            elapsed >>= 1;
        }

        uint16_t distance = static_cast<uint16_t> (to > from ? to - from : from - to);
        uint16_t moved    = static_cast<uint16_t> (distance * elapsed / length);

        return static_cast<uint16_t> (to > from ? from + moved : from - moved);
    }
}
