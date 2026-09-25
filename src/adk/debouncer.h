#pragma once

#include "object.h"

namespace adk {

    // A mechanical contact bounces for a few milliseconds when it moves. A
    // Debouncer only accepts a new state once raw samples have agreed on it
    // for a whole window.
    struct Debouncer
    {
        explicit Debouncer (bool initial = false);

        // Take one raw sample; true when the stable state has just changed.
        bool sample (bool raw, Millis now, uint8_t window);

        bool stable () const;

      private:
        Millis changedAt_;
        bool   raw_;
        bool   stable_;
    };
}
