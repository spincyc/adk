#include "servo_calibration.h"

namespace adk {

    namespace {
        const uint16_t minimumSupportedPulseUs = 544;
        const uint16_t maximumSupportedPulseUs = 2400;
    }

    ServoCalibration::ServoCalibration (
        const ServoCalibrationConfig& config) noexcept
        : config_ (config)
        , valid_  (
              config.minimumPosition < config.maximumPosition &&
              config.minimumPulseUs >= minimumSupportedPulseUs &&
              config.minimumPulseUs <= maximumSupportedPulseUs &&
              config.maximumPulseUs >= minimumSupportedPulseUs &&
              config.maximumPulseUs <= maximumSupportedPulseUs &&
              config.minimumPulseUs < config.maximumPulseUs)
    {
    }

    bool ServoCalibration::valid () const noexcept
    {
        return valid_;
    }

    Result<uint16_t> ServoCalibration::pulseFor (
        uint16_t position) const noexcept
    {
        if (!valid_ ||
            position < config_.minimumPosition ||
            position > config_.maximumPosition)
        {
            return {StatusCode::InvalidArgument, 0};
        }

        const int32_t positionOffset =
            static_cast<int32_t> (position - config_.minimumPosition);
        const int32_t positionSpan =
            static_cast<int32_t> (
                config_.maximumPosition - config_.minimumPosition);
        const int32_t pulseSpan =
            static_cast<int32_t> (config_.maximumPulseUs) -
            static_cast<int32_t> (config_.minimumPulseUs);
        const int32_t pulse =
            static_cast<int32_t> (config_.minimumPulseUs) +
            (positionOffset * pulseSpan) / positionSpan;

        return {StatusCode::Ok, static_cast<uint16_t> (pulse)};
    }

    BoundedServo::BoundedServo (const BoundedServoConfig& config) noexcept
        : config_      (config)
        , calibration_ (config.calibration)
        , intent_      (BoundedServoIntent::Inactive)
        , status_      (StatusCode::NotInitialized)
        , position_    (config.safePosition)
        , pulseUs_     (0)
        , initialized_ (false)
    {
    }

    Status BoundedServo::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const Result<uint16_t> safePulse =
            calibration_.pulseFor (config_.safePosition);

        if (!safePulse.ok ())
        {
            status_ = safePulse.status ();
            return status_;
        }

        intent_      = BoundedServoIntent::Inactive;
        status_      = StatusCode::Ok;
        position_    = config_.safePosition;
        pulseUs_     = safePulse.value ();
        initialized_ = true;
        return status_;
    }

    Status BoundedServo::command (uint16_t position) noexcept
    {
        if (!initialized_)
        {
            status_ = StatusCode::NotInitialized;
            return status_;
        }

        const Result<uint16_t> pulse = calibration_.pulseFor (position);

        if (!pulse.ok ())
        {
            status_ = pulse.status ();
            return status_;
        }

        intent_   = BoundedServoIntent::Position;
        status_   = StatusCode::Ok;
        position_ = position;
        pulseUs_  = pulse.value ();
        return status_;
    }

    void BoundedServo::shutdown () noexcept
    {
        intent_      = BoundedServoIntent::Inactive;
        status_      = StatusCode::NotInitialized;
        position_    = config_.safePosition;
        pulseUs_     = 0;
        initialized_ = false;
    }

    BoundedServoSnapshot BoundedServo::snapshot () const noexcept
    {
        return {intent_, status_, position_, pulseUs_};
    }
}
