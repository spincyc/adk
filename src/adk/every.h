#pragma once

#include "object.h"

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

        void   period (Millis period);
        Millis period () const;

      protected:
        void update (Millis now) override;

      private:
        Millis period_;
        Millis last_;
        bool   ticked_;
        bool   starting_;
    };
}
