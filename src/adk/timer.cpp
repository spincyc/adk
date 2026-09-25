#include "timer.h"

namespace adk {

    Timer::Timer ()
        : startedAt_ (0)
        , duration_  (0)
        , now_       (0)
        , running_   (false)
        , starting_  (false)
        , expired_   (false)
    {
    }

    void Timer::start (Millis duration)
    {
        duration_ = duration;
        running_  = true;
        starting_ = true;
        expired_  = false;
    }

    void Timer::stop ()
    {
        running_ = false;
        expired_ = false;
    }

    bool Timer::isRunning () const
    {
        return running_;
    }

    bool Timer::expired () const
    {
        return expired_;
    }

    Millis Timer::remaining () const
    {
        if (!running_)
        {
            return 0;
        }

        return starting_ ? duration_ : duration_ - (now_ - startedAt_);
    }

    void Timer::update (Millis now)
    {
        now_     = now;
        expired_ = false;

        if (!running_)
        {
            return;
        }

        // The countdown starts on the first update after start (), so it runs
        // its full length whatever the sketch did in between.
        if (starting_)
        {
            startedAt_ = now;
            starting_  = false;
        }

        if (now - startedAt_ >= duration_)
        {
            running_ = false;
            expired_ = true;
        }
    }

    Stopwatch::Stopwatch ()
        : startedAt_ (0)
        , banked_    (0)
        , now_       (0)
        , running_   (false)
        , starting_  (false)
    {
    }

    void Stopwatch::start ()
    {
        if (!running_)
        {
            running_  = true;
            starting_ = true;
        }
    }

    void Stopwatch::stop ()
    {
        banked_   = elapsed ();
        running_  = false;
        starting_ = false;
    }

    void Stopwatch::reset ()
    {
        banked_    = 0;
        startedAt_ = now_;
    }

    bool Stopwatch::isRunning () const
    {
        return running_;
    }

    Millis Stopwatch::elapsed () const
    {
        if (!running_ || starting_)
        {
            return banked_;
        }

        return banked_ + (now_ - startedAt_);
    }

    void Stopwatch::update (Millis now)
    {
        now_ = now;

        if (starting_)
        {
            startedAt_ = now;
            starting_  = false;
        }
    }
}
