#include "sampled_signal.h"

namespace adk {

    LinearCalibration::LinearCalibration (const LinearCalibrationConfig& config) noexcept
        : config_ (config)
        , valid_  (config.observedMinimum < config.observedMaximum)
    {
    }

    bool LinearCalibration::valid () const noexcept
    {
        return valid_;
    }

    Result<uint16_t> LinearCalibration::map (uint16_t sample) const noexcept
    {
        if (!valid ())
        {
            return Result<uint16_t> (StatusCode::InvalidArgument, 0);
        }

        if (!config_.clamp
            && (sample < config_.observedMinimum || sample > config_.observedMaximum))
        {
            return Result<uint16_t> (StatusCode::InvalidArgument, 0);
        }

        uint16_t boundedSample = sample;
        if (boundedSample < config_.observedMinimum)
        {
            boundedSample = config_.observedMinimum;
        }
        if (boundedSample > config_.observedMaximum)
        {
            boundedSample = config_.observedMaximum;
        }

        const uint32_t inputSpan =
            static_cast<uint32_t> (config_.observedMaximum - config_.observedMinimum);
        const uint32_t inputOffset =
            static_cast<uint32_t> (boundedSample - config_.observedMinimum);
        const bool     ascending = config_.mappedAtMinimum <= config_.mappedAtMaximum;
        const uint32_t outputSpan = ascending
            ? static_cast<uint32_t> (config_.mappedAtMaximum
                                     - config_.mappedAtMinimum)
            : static_cast<uint32_t> (config_.mappedAtMinimum
                                     - config_.mappedAtMaximum);
        const uint32_t mappedOffset =
            (inputOffset * outputSpan + inputSpan / 2U) / inputSpan;
        const uint16_t mapped = ascending
            ? static_cast<uint16_t> (config_.mappedAtMinimum + mappedOffset)
            : static_cast<uint16_t> (config_.mappedAtMinimum - mappedOffset);

        return Result<uint16_t> (StatusCode::Ok, mapped);
    }

    MovingAverage::MovingAverage (uint8_t windowSize) noexcept
        : samples_     ()
        , sum_         (0)
        , windowSize_  (windowSize)
        , sampleCount_ (0)
        , nextIndex_   (0)
    {
    }

    void MovingAverage::reset () noexcept
    {
        sum_         = 0;
        sampleCount_ = 0;
        nextIndex_   = 0;
    }

    bool MovingAverage::valid () const noexcept
    {
        return windowSize_ > 0 && windowSize_ <= maximumWindowSize;
    }

    bool MovingAverage::hasValue () const noexcept
    {
        return sampleCount_ > 0;
    }

    uint8_t MovingAverage::sampleCount () const noexcept
    {
        return sampleCount_;
    }

    uint8_t MovingAverage::windowSize () const noexcept
    {
        return windowSize_;
    }

    Result<uint16_t> MovingAverage::addSample (uint16_t sample) noexcept
    {
        if (!valid ())
        {
            return Result<uint16_t> (StatusCode::InvalidArgument, 0);
        }

        if (sampleCount_ < windowSize_)
        {
            samples_[nextIndex_] = sample;
            sum_ += sample;
            ++sampleCount_;
        }
        else
        {
            sum_ -= samples_[nextIndex_];
            samples_[nextIndex_] = sample;
            sum_ += sample;
        }

        nextIndex_ = static_cast<uint8_t> ((nextIndex_ + 1U) % windowSize_);

        return value ();
    }

    Result<uint16_t> MovingAverage::value () const noexcept
    {
        if (!valid ())
        {
            return Result<uint16_t> (StatusCode::InvalidArgument, 0);
        }
        if (!hasValue ())
        {
            return Result<uint16_t> (StatusCode::NotInitialized, 0);
        }

        const uint16_t average =
            static_cast<uint16_t> ((sum_ + sampleCount_ / 2U) / sampleCount_);
        return Result<uint16_t> (StatusCode::Ok, average);
    }

    Deadband::Deadband (uint16_t width) noexcept
        : width_    (width)
        , value_    (0)
        , hasValue_ (false)
    {
    }

    void Deadband::reset () noexcept
    {
        value_    = 0;
        hasValue_ = false;
    }

    bool Deadband::hasValue () const noexcept
    {
        return hasValue_;
    }

    uint16_t Deadband::width () const noexcept
    {
        return width_;
    }

    uint16_t Deadband::addSample (uint16_t sample) noexcept
    {
        if (!hasValue_)
        {
            value_    = sample;
            hasValue_ = true;
            return value_;
        }

        const uint16_t difference =
            sample > value_ ? sample - value_ : value_ - sample;
        if (difference >= width_)
        {
            value_ = sample;
        }

        return value_;
    }

    Result<uint16_t> Deadband::value () const noexcept
    {
        if (!hasValue_)
        {
            return Result<uint16_t> (StatusCode::NotInitialized, 0);
        }

        return Result<uint16_t> (StatusCode::Ok, value_);
    }
}
