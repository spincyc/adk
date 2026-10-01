#pragma once

#include "timing.h"

namespace adk {

    // A countdown. start () sets it going; expired () is true for the one
    // update in which it runs out, like a button's wasPressed (), even if
    // the sketch starts it again in that update.
    struct Timer : Object
    {
        Timer ();

        void start (Millis duration);

        // Cancel the countdown, so it never expires. adk::stop () stops
        // every Timer too, so none goes off after it.
        void stop () override;

        bool   isRunning () const;
        bool   expired   () const;
        Millis remaining () const;

      protected:
        void update (Millis now) override;

      private:
        StartTime started_;
        Millis    duration_;
        Millis    elapsed_;     // as of the latest update
        bool      running_;
        bool      expired_;
    };

    // Measures time while it runs, as of the latest adk::update (). Stopping
    // and starting again carries on from where it stopped, like a real
    // stopwatch; reset () goes back to zero, and restart () goes back to zero
    // and runs.
    struct Stopwatch : Object
    {
        Stopwatch ();

        void start ();

        // Hold the time it has. adk::stop () stops every Stopwatch too.
        void stop () override;

        void   reset     ();
        void   restart   ();
        bool   isRunning () const;
        Millis elapsed   () const;

      protected:
        void update (Millis now) override;

      private:
        StartTime started_;
        Millis    banked_;      // the time before the latest start
        Millis    now_;
        bool      running_;
    };
}
