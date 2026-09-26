#pragma once

#include "object.h"

namespace adk {

    // When something timed began: a blink, a beep, a fade, a note. Time
    // enters a part only through update (now), so a command cannot know it:
    // the command calls restart (), and the next update's now becomes the
    // start. A new StartTime waits for an update too, so a part that keeps a
    // schedule, such as a sensor, counts from its first update.
    struct StartTime
    {
        StartTime ();

        // Start at the next update.
        void restart ();

        // Start at now, from inside update ().
        void restart (Millis now);

        // Milliseconds since the start. While it waits for an update, the
        // start becomes now and this is 0.
        Millis elapsed (Millis now);

      private:
        Millis start_;
        bool   waiting_;
    };

    // Where a glide or a fade has got to: elapsed / length of the way from
    // one value to another, and the second value once elapsed reaches
    // length. It stays exact to 1 part in 65536 however long the length.
    uint16_t interpolate (uint16_t from, uint16_t to, Millis elapsed, Millis length);
}
