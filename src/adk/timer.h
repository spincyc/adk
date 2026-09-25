#pragma once

#include "object.h"

namespace adk {

    // A countdown. start () sets it going; expired () is true for the one
    // update in which it runs out, like a button's wasPressed ().
    struct Timer : Object
    {
        Timer ();

        void   start     (Millis duration);
        void   stop      ();
        bool   isRunning () const;
        bool   expired   () const;
        Millis remaining () const;

      protected:
        void update (Millis now) override;

      private:
        Millis startedAt_;
        Millis duration_;
        Millis now_;
        bool   running_;
        bool   starting_;
        bool   expired_;
    };

    // Measures time while it runs, as of the latest adk::update (). Stopping
    // and starting again carries on from where it stopped, like a real
    // stopwatch; reset () goes back to zero, and restart () goes back to zero
    // and runs.
    struct Stopwatch : Object
    {
        Stopwatch ();

        void   start     ();
        void   stop      ();
        void   reset     ();
        void   restart   ();
        bool   isRunning () const;
        Millis elapsed   () const;

      protected:
        void update (Millis now) override;

      private:
        Millis startedAt_;
        Millis banked_;
        Millis now_;
        bool   running_;
        bool   starting_;
    };
}
