#pragma once

#include <stdint.h>

namespace adk {

    struct Duration
    {
        using Raw = uint32_t;

        explicit Duration (Raw milliseconds = 0) noexcept;

        Raw milliseconds () const noexcept;

      private:
        Raw milliseconds_;
    };

    struct TimePoint
    {
        using Raw = uint32_t;

        explicit TimePoint (Raw milliseconds = 0) noexcept;

        Raw      milliseconds () const noexcept;
        Duration elapsedSince (TimePoint earlier) const noexcept;

      private:
        Raw milliseconds_;
    };

    bool operator== (Duration left, Duration right) noexcept;
    bool operator!= (Duration left, Duration right) noexcept;
    bool operator<  (Duration left, Duration right) noexcept;
    bool operator<= (Duration left, Duration right) noexcept;
    bool operator>  (Duration left, Duration right) noexcept;
    bool operator>= (Duration left, Duration right) noexcept;

    bool operator== (TimePoint left, TimePoint right) noexcept;
    bool operator!= (TimePoint left, TimePoint right) noexcept;
}
