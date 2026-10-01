#pragma once

#include "timing.h"

namespace adk {

    // A steady beat. ticked () is true for one update each period, so a loop
    // can do something every second without stopping to wait for it.
    struct Every : Object
    {
        explicit Every (Millis period);

        bool ticked () const;

        // Begin the beat again, the next tick one period after the next
        // update. A tick already reported stays reported until then.
        void restart ();

        // Stop the beat: no tick comes until restart (). adk::stop () stops
        // every Every too, so a loop that waits for ticked () stays still.
        void stop () override;

        void   period (Millis period);
        Millis period () const;

      protected:
        void update (Millis now) override;

      private:
        StartTime beat_;
        Millis    period_;
        bool      ticked_;
        bool      running_;
    };
}
