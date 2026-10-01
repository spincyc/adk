#include "timer.h"

namespace adk {

    Timer::Timer ()
        : started_  ()
        , duration_ (0)
        , elapsed_  (0)
        , running_  (false)
        , expired_  (false)
    {
    }

    // Neither start () nor stop () clears expired (): an event lasts until
    // the next update, whatever the sketch does after reading it. The
    // countdown starts on the next update, so it runs its full length
    // whatever the sketch did in between.
    void Timer::start (Millis duration)
    {
        duration_ = duration;
        elapsed_  = 0;
        running_  = true;
        started_.restart ();
    }

    void Timer::stop ()
    {
        running_ = false;
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
        return running_ ? duration_ - elapsed_ : 0;
    }

    void Timer::update (Millis now)
    {
        expired_ = false;

        if (!running_)
        {
            return;
        }

        started_.start (now);
        elapsed_ = started_.elapsed (now);

        if (elapsed_ >= duration_)
        {
            running_ = false;
            expired_ = true;
        }
    }

    Stopwatch::Stopwatch ()
        : started_ ()
        , banked_  (0)
        , now_     (0)
        , running_ (false)
    {
    }

    // Like every command that starts something timed, start (), reset ()
    // and restart () count from the next update.
    void Stopwatch::start ()
    {
        if (!running_)
        {
            running_ = true;
            started_.restart ();
        }
    }

    void Stopwatch::stop ()
    {
        banked_  = elapsed ();
        running_ = false;
    }

    void Stopwatch::reset ()
    {
        banked_ = 0;
        started_.restart ();
    }

    void Stopwatch::restart ()
    {
        reset ();
        running_ = true;
    }

    bool Stopwatch::isRunning () const
    {
        return running_;
    }

    Millis Stopwatch::elapsed () const
    {
        return running_ ? banked_ + started_.elapsed (now_) : banked_;
    }

    void Stopwatch::update (Millis now)
    {
        now_ = now;
        started_.start (now);
    }
}
