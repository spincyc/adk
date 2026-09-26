#pragma once

#include "object.h"

namespace adk {

    // A mechanical contact bounces for a few milliseconds when it moves. A
    // Debouncer only accepts a new state once raw samples have agreed on it
    // for a whole window. The state is a bool, pressed or not, unless told
    // otherwise, as the keypad's is the number of the key held.
    template <typename State = bool>
    struct Debouncer
    {
        explicit Debouncer (State initial = State {})
            : changedAt_ (0)
            , raw_       (initial)
            , stable_    (initial)
        {
        }

        // Take one raw sample; true when the stable state has just changed.
        bool sample (State raw, Millis now, uint8_t window)
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

        State stable () const { return stable_; }

      private:
        Millis changedAt_;
        State  raw_;
        State  stable_;
    };
}
