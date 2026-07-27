#include "moisture_sensor.h"

namespace adk {

    MoistureSensor::MoistureSensor (
        AnalogInput&               input,
        const MoistureCalibration& calibration) noexcept
        : input_       (&input)
        , calibration_ (calibration)
        , sample_      ({0, 0, TimePoint (), MoistureSampleState::Unavailable})
        , initialized_ (false)
    {
    }

    MoistureSensor::~MoistureSensor () noexcept
    {
        shutdown ();
    }

    Status MoistureSensor::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!validCalibration ())
        {
            return StatusCode::InvalidArgument;
        }

        const Status status = input_->initialize ();

        if (!status.ok ())
        {
            return status;
        }

        sample_      = {0, input_->read (), TimePoint (),
                        MoistureSampleState::Unavailable};
        initialized_ = true;
        return StatusCode::Ok;
    }

    void MoistureSensor::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        input_->shutdown ();
        sample_      = {0, sample_.rawReading, sample_.observedAt,
                        MoistureSampleState::Unavailable};
        initialized_ = false;
    }

    Status MoistureSensor::update (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        input_->update ();
        deriveSample   (input_->read (), now);
        return StatusCode::Ok;
    }

    MoistureSample MoistureSensor::sample (
        TimePoint now,
        Duration  staleAfter) const noexcept
    {
        MoistureSample result = sample_;

        if (result.state == MoistureSampleState::Valid
            && now.elapsedSince (result.observedAt) > staleAfter)
        {
            result.state = MoistureSampleState::Stale;
        }

        return result;
    }

    bool MoistureSensor::initialized () const noexcept
    {
        return initialized_;
    }

    bool MoistureSensor::validCalibration () const noexcept
    {
        if (calibration_.dryReading > AnalogInput::maximumReading
            || calibration_.wetReading > AnalogInput::maximumReading
            || calibration_.faultMargin > 64
            || calibration_.dryReading == calibration_.wetReading)
        {
            return false;
        }

        const uint16_t span = calibration_.dryReading < calibration_.wetReading
            ? static_cast<uint16_t> (
                  calibration_.wetReading - calibration_.dryReading)
            : static_cast<uint16_t> (
                  calibration_.dryReading - calibration_.wetReading);

        return span > static_cast<uint16_t> (2U * calibration_.faultMargin);
    }

    void MoistureSensor::deriveSample (
        AnalogInput::Reading reading,
        TimePoint            now) noexcept
    {
        const int32_t dry    = calibration_.dryReading;
        const int32_t wet    = calibration_.wetReading;
        const int32_t margin = calibration_.faultMargin;
        const int32_t low    = dry < wet ? dry : wet;
        const int32_t high   = dry < wet ? wet : dry;

        sample_.rawReading       = reading;
        sample_.observedAt       = now;
        sample_.moisturePermille = 0;

        if (static_cast<int32_t> (reading) < low - margin)
        {
            sample_.state = MoistureSampleState::InputBelowRange;
            return;
        }

        if (static_cast<int32_t> (reading) > high + margin)
        {
            sample_.state = MoistureSampleState::InputAboveRange;
            return;
        }

        int32_t bounded = reading;

        if (bounded < low)
        {
            bounded = low;
        }
        if (bounded > high)
        {
            bounded = high;
        }

        const uint32_t span = static_cast<uint32_t> (high - low);
        const uint32_t offset = dry < wet
            ? static_cast<uint32_t> (bounded - dry)
            : static_cast<uint32_t> (dry - bounded);

        sample_.moisturePermille =
            static_cast<uint16_t> ((offset * 1000U + span / 2U) / span);
        sample_.state = MoistureSampleState::Valid;
    }
}
