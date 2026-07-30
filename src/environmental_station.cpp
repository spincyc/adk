#include "environmental_station.h"

#include <limits.h>

namespace adk {

    namespace {

        constexpr uint32_t maximumUnambiguousDuration =
            static_cast<uint32_t> (INT32_MAX);

        EnvironmentalRecord emptyRecord () noexcept
        {
            const ClimateSample sample = {0, 0, TimePoint (0),
                                          ClimateSampleState::Unavailable};

            return {sample, EnvironmentalHealth::Starting,
                    StatusCode::NotInitialized,
                    TimePoint (0), 0};
        }
    } // namespace

    EnvironmentalStation::EnvironmentalStation (
        ClimateSensor& sensor, const EnvironmentalStationConfig& config) noexcept
        : sensor_ (&sensor), config_ (config),
          snapshot_ (
              {emptyRecord (), TimePoint (0), StatusCode::NotInitialized, false,
               false}),
          lastUpdateAt_ (0), initialized_ (false), hasUpdated_ (false)
    {
    }

    EnvironmentalStation::~EnvironmentalStation () noexcept
    {
        shutdown ();
    }

    Status EnvironmentalStation::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!configValid ())
        {
            clearState (StatusCode::InvalidArgument);
            return snapshot_.status;
        }

        const Status status = sensor_->initialize ();

        if (!status.ok ())
        {
            sensor_->shutdown ();
            clearState        (status);
            return snapshot_.status;
        }

        clearState (StatusCode::Ok);
        initialized_ = true;

        return StatusCode::Ok;
    }

    void EnvironmentalStation::shutdown () noexcept
    {
        sensor_->shutdown ();
        clearState        (StatusCode::NotInitialized);
    }

    bool EnvironmentalStation::initialized () const noexcept
    {
        return initialized_;
    }

    Status EnvironmentalStation::reset () noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        sensor_->shutdown ();

        const Status status = sensor_->initialize ();

        if (!status.ok ())
        {
            sensor_->shutdown ();
            clearState        (status);
            return status;
        }

        clearState (StatusCode::Ok);
        initialized_ = true;

        return StatusCode::Ok;
    }

    Status EnvironmentalStation::update (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        snapshot_.recordReady = false;

        if (hasUpdated_ && now.elapsedSince (lastUpdateAt_).milliseconds () >
                               maximumUnambiguousDuration)
        {
            snapshot_.record.health       = EnvironmentalHealth::TimingFault;
            snapshot_.record.sensorStatus = StatusCode::InvalidArgument;
            snapshot_.record.recordedAt   = now;
            snapshot_.status              = StatusCode::InvalidArgument;
            snapshot_.hasDeadline         = false;
            return snapshot_.status;
        }

        lastUpdateAt_ = now;
        hasUpdated_   = true;

        if (!sampleDue (now))
        {
            return snapshot_.status;
        }

        const Status        sensorStatus = sensor_->update (now);
        const ClimateSample sample       = sensor_->sample (now, config_.staleAfter);

        ++snapshot_.record.sequence;
        snapshot_.record.sample       = sample;
        snapshot_.record.health       = chooseHealth (sample, sensorStatus);
        snapshot_.record.sensorStatus = sensorStatus;
        snapshot_.record.recordedAt   = now;
        snapshot_.nextSampleAt =
            TimePoint (now.milliseconds () + config_.samplePeriod.milliseconds ());
        snapshot_.status      = sensorStatus;
        snapshot_.hasDeadline = true;
        snapshot_.recordReady = true;

        return snapshot_.status;
    }

    EnvironmentalSnapshot EnvironmentalStation::snapshot () const noexcept
    {
        return snapshot_;
    }

    bool EnvironmentalStation::configValid () const noexcept
    {
        return config_.samplePeriod.milliseconds () != 0 &&
               config_.samplePeriod.milliseconds () <=
                   maximumUnambiguousDuration &&
               config_.staleAfter.milliseconds  () <=
                   maximumUnambiguousDuration;
    }

    bool EnvironmentalStation::sampleDue (TimePoint now) const noexcept
    {
        return !snapshot_.hasDeadline ||
               now.elapsedSince (snapshot_.nextSampleAt).milliseconds () <=
                   maximumUnambiguousDuration;
    }

    EnvironmentalHealth
    EnvironmentalStation::chooseHealth (const ClimateSample& sample,
                                        Status sensorStatus) const noexcept
    {
        // A sensor reports every non-valid sample through one InvalidArgument
        // status, so the sample state is the only evidence that distinguishes
        // a timing problem from a reading the sensor itself judged out of
        // range. Classify from the state first; the status is a fallback for
        // a sensor that reports a timing failure without a sample state.
        if (sample.state == ClimateSampleState::TemperatureOutOfRange ||
            sample.state == ClimateSampleState::HumidityOutOfRange)
        {
            return EnvironmentalHealth::SensorFault;
        }

        if (sample.state == ClimateSampleState::InvalidTiming ||
            sensorStatus.error () == StatusCode::InvalidArgument)
        {
            return EnvironmentalHealth::TimingFault;
        }

        if (sample.state == ClimateSampleState::Stale)
        {
            return EnvironmentalHealth::Stale;
        }

        if (sensorStatus.ok () &&
            sample.state == ClimateSampleState::Unavailable)
        {
            return EnvironmentalHealth::Starting;
        }

        if (sensorStatus.ok () && sample.state == ClimateSampleState::Valid)
        {
            return EnvironmentalHealth::Healthy;
        }

        return EnvironmentalHealth::SensorFault;
    }

    void EnvironmentalStation::clearState (Status status) noexcept
    {
        snapshot_.record       = emptyRecord ();

        snapshot_.nextSampleAt = TimePoint (0);
        snapshot_.status       = status;
        snapshot_.hasDeadline  = false;
        snapshot_.recordReady  = false;
        lastUpdateAt_          = TimePoint (0);
        initialized_           = false;
        hasUpdated_            = false;
    }
} // namespace adk
