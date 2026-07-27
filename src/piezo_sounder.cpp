#include "piezo_sounder.h"

#include <Arduino.h>

namespace adk {

    PiezoSounder::PiezoSounder (ResourceRegistry& resources, PinId pin) noexcept
        : resources_   (&resources)
        , pinClaim_    ()
        , timerClaim_  ()
        , pin_         (pin)
        , frequency_   (0)
        , duration_    ()
        , startedAt_   ()
        , initialized_ (false)
        , sounding_    (false)
    {
    }

    PiezoSounder::~PiezoSounder () noexcept
    {
        shutdown ();
    }

    Status PiezoSounder::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (pin_ >= NUM_DIGITAL_PINS)
        {
            return StatusCode::InvalidPin;
        }

        const ResourceId pinResource = {ResourceKind::Pin, pin_};
        Status           status      = resources_->claim (pinResource, pinClaim_);

        if (!status.ok ())
        {
            return status;
        }

        const ResourceId timerResource = {ResourceKind::Timer, toneTimer};
        status = resources_->claim (timerResource, timerClaim_);

        if (!status.ok ())
        {
            pinClaim_.release ();
            return status;
        }

        pinMode (pin_, INPUT);
        initialized_ = true;
        return StatusCode::Ok;
    }

    void PiezoSounder::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        stop                ();
        timerClaim_.release ();
        pinClaim_  .release ();
        initialized_ = false;
    }

    Status PiezoSounder::play (
        Frequency frequency,
        Duration  duration,
        TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (frequency < minimumFrequencyHz ||
            frequency > maximumFrequencyHz ||
            duration.milliseconds () == 0 ||
            duration.milliseconds () > maximumDurationMs)
        {
            return StatusCode::InvalidArgument;
        }

        tone (pin_, frequency);

        frequency_ = frequency;
        duration_  = duration;
        startedAt_ = now;
        sounding_  = true;
        return StatusCode::Ok;
    }

    void PiezoSounder::stop () noexcept
    {
        if (!initialized_ || !sounding_)
        {
            return;
        }

        noTone  (pin_);
        pinMode (pin_, INPUT);

        frequency_ = 0;
        duration_  = Duration ();
        sounding_  = false;
    }

    void PiezoSounder::update (TimePoint now) noexcept
    {
        if (sounding_ && now.elapsedSince (startedAt_) >= duration_)
        {
            stop ();
        }
    }

    PinId PiezoSounder::pin () const noexcept
    {
        return pin_;
    }

    PiezoSounder::Frequency PiezoSounder::frequency () const noexcept
    {
        return frequency_;
    }

    bool PiezoSounder::initialized () const noexcept
    {
        return initialized_;
    }

    bool PiezoSounder::sounding () const noexcept
    {
        return sounding_;
    }
}
