#include "climate_sensor.h"

#include <limits.h>

namespace adk {

    namespace {

        ClimateSample unavailableSample () noexcept
        {
            return {0, 0, TimePoint (0), ClimateSampleState::Unavailable};
        }

        constexpr uint32_t maximumUnambiguousDuration =
            static_cast<uint32_t> (INT32_MAX);

        bool limitsValid (const ClimateSampleLimits& limits) noexcept
        {
            return limits.minimumTemperatureCentiCelsius <=
                       limits.maximumTemperatureCentiCelsius &&
                   limits.maximumHumidityPermille <= 1000U;
        }
    } // namespace

    ClimateSample validateClimateSample (int16_t   temperatureCentiCelsius,
                                         uint16_t  humidityPermille,
                                         TimePoint observedAt,
                                         const ClimateSampleLimits& limits) noexcept
    {
        ClimateSampleState state = ClimateSampleState::Valid;

        if (!limitsValid (limits))
        {
            state = ClimateSampleState::InvalidLimits;
        }
        else if (temperatureCentiCelsius < limits.minimumTemperatureCentiCelsius ||
                 temperatureCentiCelsius > limits.maximumTemperatureCentiCelsius)
        {
            state = ClimateSampleState::TemperatureOutOfRange;
        }
        else if (humidityPermille > limits.maximumHumidityPermille)
        {
            state = ClimateSampleState::HumidityOutOfRange;
        }

        return {temperatureCentiCelsius, humidityPermille, observedAt, state};
    }

    ClimateSensor::~ClimateSensor () noexcept = default;

    RecordedClimateSensor::RecordedClimateSensor (
        const RecordedClimateFrame* frames,
        size_t                      frameCount) noexcept
        : frames_       (frames)
        , frameCount_   (frameCount)
        , frameIndex_   (0)
        , sample_       (unavailableSample ())
        , lastUpdateAt_ (0)
        , initialized_  (false)
        , hasUpdated_   (false)
    {
    }

    RecordedClimateSensor::~RecordedClimateSensor () noexcept
    {
        shutdown ();
    }

    Status RecordedClimateSensor::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!traceValid ())
        {
            return StatusCode::InvalidArgument;
        }

        frameIndex_   = 0;
        sample_       = unavailableSample ();
        lastUpdateAt_ = TimePoint         (0);
        initialized_  = true;
        hasUpdated_   = false;

        return StatusCode::Ok;
    }

    void RecordedClimateSensor::shutdown () noexcept
    {
        frameIndex_   = 0;
        sample_       = unavailableSample ();
        lastUpdateAt_ = TimePoint         (0);
        initialized_  = false;
        hasUpdated_   = false;
    }

    bool RecordedClimateSensor::initialized () const noexcept
    {
        return initialized_;
    }

    Status RecordedClimateSensor::update (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (hasUpdated_ && now.elapsedSince (lastUpdateAt_).milliseconds () >
                               maximumUnambiguousDuration)
        {
            return StatusCode::InvalidArgument;
        }

        lastUpdateAt_ = now;
        hasUpdated_   = true;

        // Equal-time frames are consumed in trace order; the final status wins.
        Status status;

        while (frameIndex_ < frameCount_ &&
               frameDue (now, frames_[frameIndex_].availableAt))
        {
            sample_ = frames_[frameIndex_].sample;
            status  = frames_[frameIndex_].updateStatus;
            ++frameIndex_;
        }

        return status;
    }

    ClimateSample RecordedClimateSensor::sample (TimePoint now,
                                                 Duration  staleAfter) const noexcept
    {
        ClimateSample current = sample_;

        if (current.state != ClimateSampleState::Valid)
        {
            return current;
        }

        const uint32_t age = now.elapsedSince (current.observedAt).milliseconds ();

        if (staleAfter.milliseconds () > maximumUnambiguousDuration ||
            age > maximumUnambiguousDuration)
        {
            current.state = ClimateSampleState::InvalidTiming;
        }
        else if (age > staleAfter.milliseconds ())
        {
            current.state = ClimateSampleState::Stale;
        }

        return current;
    }

    size_t RecordedClimateSensor::frameIndex () const noexcept
    {
        return frameIndex_;
    }

    bool RecordedClimateSensor::traceValid () const noexcept
    {
        if (frameCount_ != 0 && frames_ == nullptr)
        {
            return false;
        }

        for (size_t index = 1; index < frameCount_; ++index)
        {
            const uint32_t distance = frames_[index].availableAt.elapsedSince (
                frames_[index - 1].availableAt).milliseconds ();

            if (distance > maximumUnambiguousDuration)
            {
                return false;
            }
        }

        return true;
    }

    bool RecordedClimateSensor::frameDue (TimePoint now,
                                          TimePoint availableAt) const noexcept
    {
        return now.elapsedSince (availableAt).milliseconds () <=
               maximumUnambiguousDuration;
    }
} // namespace adk
