#include "time.h"

namespace adk {

    Duration::Duration (Raw milliseconds) noexcept
        : milliseconds_ (milliseconds)
    {
    }

    Duration::Raw Duration::milliseconds () const noexcept
    {
        return milliseconds_;
    }

    TimePoint::TimePoint (Raw milliseconds) noexcept
        : milliseconds_ (milliseconds)
    {
    }

    TimePoint::Raw TimePoint::milliseconds () const noexcept
    {
        return milliseconds_;
    }

    Duration TimePoint::elapsedSince (TimePoint earlier) const noexcept
    {
        return Duration (milliseconds_ - earlier.milliseconds_);
    }

    bool operator== (Duration left, Duration right) noexcept
    {
        return left.milliseconds () == right.milliseconds ();
    }

    bool operator!= (Duration left, Duration right) noexcept
    {
        return !(left == right);
    }

    bool operator< (Duration left, Duration right) noexcept
    {
        return left.milliseconds () < right.milliseconds ();
    }

    bool operator<= (Duration left, Duration right) noexcept
    {
        return !(right < left);
    }

    bool operator> (Duration left, Duration right) noexcept
    {
        return right < left;
    }

    bool operator>= (Duration left, Duration right) noexcept
    {
        return !(left < right);
    }

    bool operator== (TimePoint left, TimePoint right) noexcept
    {
        return left.milliseconds () == right.milliseconds ();
    }

    bool operator!= (TimePoint left, TimePoint right) noexcept
    {
        return !(left == right);
    }
}
