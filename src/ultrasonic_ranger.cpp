#include "ultrasonic_ranger.h"

namespace adk {

    static constexpr uint32_t roundTripDivisor = 2000;

    UltrasonicRanger::UltrasonicRanger (const UltrasonicRangerConfig& config) noexcept
        : config_ (config), pulse_ ({config.echoTimeout, config.maximumEchoDuration}),
          reading_     ({RangeState::Idle, 0, MicrosecondDuration (), false}),
          initialized_ (false)
    {
    }

    Status UltrasonicRanger::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!validConfig ())
        {
            return StatusCode::InvalidArgument;
        }

        const Status status = pulse_.initialize ();

        if (!status.ok ())
        {
            return status;
        }

        reset ();
        initialized_ = true;
        return StatusCode::Ok;
    }

    void UltrasonicRanger::reset () noexcept
    {
        pulse_.reset                                         ();
        reading_ = {RangeState::Idle, 0, MicrosecondDuration (), false};
    }

    Status UltrasonicRanger::startMeasurement (MicrosecondTimePoint now,
                                               bool                 echoHigh) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        const Status status = pulse_.arm (now, echoHigh);

        if (status.ok ())
        {
            reading_ = {RangeState::AwaitingEcho, 0, MicrosecondDuration (), false};
        }

        return status;
    }

    Status UltrasonicRanger::update (MicrosecondTimePoint now, bool echoHigh) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        const Status status = pulse_.update (now, echoHigh);

        if (!status.ok ())
        {
            return status;
        }

        updateReading ();
        return StatusCode::Ok;
    }

    RangeReading UltrasonicRanger::reading () const noexcept
    {
        return reading_;
    }

    bool UltrasonicRanger::initialized () const noexcept
    {
        return initialized_;
    }

    bool UltrasonicRanger::validConfig () const noexcept
    {
        return config_.minimumDistanceMm > 0 &&
               config_.minimumDistanceMm < config_.maximumDistanceMm &&
               config_.soundSpeedMicrometersPerMicrosecond > 0;
    }

    uint16_t
    UltrasonicRanger::distanceFromEcho (MicrosecondDuration echoDuration) const noexcept
    {
        const uint64_t numerator =
            static_cast<uint64_t> (echoDuration.microseconds ()) *
                config_.soundSpeedMicrometersPerMicrosecond +
            roundTripDivisor / 2;
        const uint64_t distance = numerator / roundTripDivisor;

        return distance <= 0xffffu ? static_cast<uint16_t> (distance) : 0xffffu;
    }

    void UltrasonicRanger::updateReading () noexcept
    {
        const PulseInputSnapshot pulse = pulse_.snapshot ();

        if (pulse.state == PulseInputState::AwaitingLow ||
            pulse.state == PulseInputState::AwaitingRise)
        {
            reading_.state = RangeState::AwaitingEcho;
            return;
        }

        if (pulse.state == PulseInputState::MeasuringHigh)
        {
            reading_.state = RangeState::Measuring;
            return;
        }

        if (pulse.state == PulseInputState::Timeout)
        {
            reading_ = {RangeState::Timeout, 0, MicrosecondDuration (), false};
            return;
        }

        if (pulse.state != PulseInputState::Complete)
        {
            return;
        }

        const uint16_t distance = distanceFromEcho (pulse.highDuration);
        const bool     valid    = distance >= config_.minimumDistanceMm &&
                                  distance <= config_.maximumDistanceMm;

        reading_ = {valid ? RangeState::Valid : RangeState::OutOfRange, distance,
                    pulse.highDuration, valid};
    }
} // namespace adk
