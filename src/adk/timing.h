#pragma once

#include "object.h"

namespace adk {

    // When something timed began: a blink, a beep, a fade, a note. Time
    // enters a part only through update (now), so a command cannot know it:
    // the command calls restart (), and the part's next update takes its
    // now as the start. A new StartTime waits for an update too, so a part
    // that keeps a schedule, such as a sensor, counts from its first update.
    //
    // Inside update (), a part that times one thing calls start (now),
    // then reads elapsed (now). A part that acts each period calls beat (),
    // due () or passed (), which take the start themselves.
    struct StartTime
    {
        StartTime ();

        // Start at the next update.
        void restart ();

        // Start at now, from inside update ().
        void restart (Millis now);

        // From inside update (): start at now if it waits for an update,
        // and otherwise change nothing.
        void start (Millis now);

        // Milliseconds since the start, or 0 while it waits for an update.
        Millis elapsed (Millis now) const;

        // Whether a whole period has passed since the start: a beat, as an
        // Every ticks. The start then moves on by one period, so the beat
        // keeps time however late the update that notices it, except after
        // a gap of two periods or more, which it never tries to catch up
        // on: it starts again from now. While it waits for an update, it
        // starts at now, and no period has passed.
        bool beat (Millis now, Millis period);

        // Whether it is time to act again: at the first update after a
        // restart, and then once a period has passed since it last said so.
        // Each time, it starts again from now, so a late update puts the
        // next one back, as a multiplexed display or a scroll wants.
        bool due (Millis now, Millis period);

        // As due (), except that the first comes a period after the first
        // update, not at it: a sensor's readings, each at least a period
        // after the last.
        bool passed (Millis now, Millis period);

      private:
        Millis start_;
        bool   waiting_;
    };

    // Where a glide or a fade has got to: elapsed / length of the way from
    // one value to another, and the second value once elapsed reaches
    // length. It stays exact to 1 part in 65536 however long the length.
    uint16_t interpolate (uint16_t from, uint16_t to, Millis elapsed, Millis length);
}
