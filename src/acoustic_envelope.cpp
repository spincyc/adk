#include "acoustic_envelope.h"

// clang-format off
namespace adk {
    namespace {
        constexpr uint32_t halfRange = 0x80000000UL;
        constexpr uint16_t adcMaximum = 1023;

        bool validLevel (Level level) noexcept
        {
            return level == Level::Low || level == Level::High;
        }

        bool validDuration (Duration duration) noexcept
        {
            return duration.milliseconds () != 0 &&

                   duration.milliseconds () < halfRange;
        }

        AcousticObservation initialObservation () noexcept
        {
            return {TimePoint (),
                    0,
                    0,
                    0,
                    0,
                    0,
                    false,
                    false,
                    false,
                    TimePoint (),

                    Duration (),
                    AcousticPhase::Calibrating,
                    AcousticQuality::Unqualified,
                    StatusCode::NotInitialized};
        }
    } // namespace

    AcousticEnvelopeConfig::AcousticEnvelopeConfig (
        bool hasThresholdValue, Level thresholdActiveLevelValue,
        uint16_t railMarginValue, uint16_t attackAboveBaselineValue,
        uint16_t releaseAboveBaselineValue, uint8_t baselineShiftValue,
        Duration calibrationValue, Duration eventWindowValue,
        Duration quietToCloseValue, Duration refractoryValue) noexcept
        : hasThreshold        (hasThresholdValue)

        , thresholdActiveLevel (thresholdActiveLevelValue)

        , railMargin          (railMarginValue)

        , attackAboveBaseline (attackAboveBaselineValue)

        , releaseAboveBaseline (releaseAboveBaselineValue)

        , baselineShift       (baselineShiftValue)

        , calibration         (calibrationValue)

        , eventWindow         (eventWindowValue)

        , quietToClose        (quietToCloseValue)

        , refractory          (refractoryValue)
    {
    }

    AcousticEnvelope::AcousticEnvelope (
        const AcousticEnvelopeConfig& config) noexcept
        : config_                       (config)

        , snapshot_                     (initialObservation ())

        , lastSample_                   ()

        , calibrationStartedAt_         ()

        , eventStartedAt_               ()

        , quietStartedAt_               ()

        , refractoryStartedAt_          ()

        , disagreementStartedAt_        ()

        , completedStartedAt_            ()

        , completedDuration_             ()

        , completedPeak_                 (0)

        , completedIntensity_            (0)

        , initialized_                  (false)

        , hasSample_                    (false)

        , calibrationStarted_           (false)

        , quietCandidate_               (false)

        , disagreementCandidate_        (false)

        , hasCompleted_                 (false)
    {
    }

    Status AcousticEnvelope::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!validConfig ())
        {
            return StatusCode::InvalidArgument;
        }

        initialized_ = true;
        reset ();

        return StatusCode::Ok;
    }

    void AcousticEnvelope::reset () noexcept
    {
        snapshot_              = initialObservation ();
        hasSample_             = false;
        calibrationStarted_    = false;
        quietCandidate_        = false;
        disagreementCandidate_ = false;
        lastSample_            = AcousticSample ();

        calibrationStartedAt_  = TimePoint ();

        eventStartedAt_        = TimePoint ();

        quietStartedAt_        = TimePoint ();

        refractoryStartedAt_   = TimePoint ();

        disagreementStartedAt_ = TimePoint ();

        completedStartedAt_    = TimePoint ();

        completedDuration_     = Duration ();
        completedPeak_         = 0;
        completedIntensity_    = 0;
        hasCompleted_          = false;

        if (initialized_)
        {
            snapshot_.status = StatusCode::Ok;
        }
    }

    Status AcousticEnvelope::update (const AcousticSample& sample) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (hasSample_)
        {
            const Duration elapsed = sample.observedAt.elapsedSince (
                lastSample_.observedAt);

            if (elapsed.milliseconds () == 0)
            {
                if (sameSample (sample))
                {
                    return snapshot_.status;
                }

                return enterFault (sample, AcousticQuality::TimingFault,
                                   StatusCode::InvalidArgument, true);
            }

            if (elapsed.milliseconds () >= halfRange)
            {
                return enterFault (sample, AcousticQuality::TimingFault,
                                   StatusCode::InvalidArgument, true);
            }
        }

        const bool completedEventWasPublished =
            snapshot_.phase == AcousticPhase::Refractory &&
            snapshot_.eventCompleted;

        snapshot_.eventStarted   = false;
        snapshot_.eventCompleted = false;

        if (completedEventWasPublished)
        {
            clearCurrentEvent ();
        }

        if (snapshot_.phase == AcousticPhase::Fault)
        {
            rememberSample (sample);
            return snapshot_.status;
        }

        if (config_.hasThreshold && sample.hasThreshold &&
            !validLevel (sample.thresholdLevel))
        {
            const uint16_t previousRaw = snapshot_.raw;
            const bool previousRawThresholdActive =
                snapshot_.rawThresholdActive;

            const Status status =
                enterFault (sample, AcousticQuality::SourceFault,
                            StatusCode::InvalidArgument, false);

            snapshot_.raw                = previousRaw;
            snapshot_.rawThresholdActive = previousRawThresholdActive;

            return status;
        }

        if (!sample.analogStatus.ok ())
        {
            return enterFault (sample, AcousticQuality::SourceFault,
                               sample.analogStatus, false);
        }

        if (config_.hasThreshold != sample.hasThreshold)
        {
            return enterFault (sample, AcousticQuality::SourceFault,
                               StatusCode::HardwareFailure, false);
        }

        if (config_.hasThreshold && !sample.thresholdStatus.ok ())
        {
            return enterFault (sample, AcousticQuality::SourceFault,
                               sample.thresholdStatus, false);
        }

        if (sample.raw > adcMaximum)
        {
            return enterFault (sample, AcousticQuality::Unqualified,
                               StatusCode::InvalidArgument, false);
        }

        snapshot_.observedAt         = sample.observedAt;
        snapshot_.raw                = sample.raw;
        snapshot_.rawThresholdActive =
            config_.hasThreshold &&
            sample.thresholdLevel == config_.thresholdActiveLevel;

        if (sample.raw <= config_.railMargin)
        {
            return enterFault (sample, AcousticQuality::ClippedLow,
                               StatusCode::InvalidArgument, false);
        }

        if (sample.raw >= adcMaximum - config_.railMargin)
        {
            return enterFault (sample, AcousticQuality::ClippedHigh,
                               StatusCode::InvalidArgument, false);
        }

        if (!calibrationStarted_)
        {
            calibrationStarted_   = true;
            calibrationStartedAt_ = sample.observedAt;
            snapshot_.baseline    = sample.raw;
            snapshot_.amplitude   = 0;
            snapshot_.phase       = AcousticPhase::Calibrating;
            snapshot_.quality     = AcousticQuality::Unqualified;
            snapshot_.status      = StatusCode::Ok;
            rememberSample (sample);
            return snapshot_.status;
        }

        const uint16_t amplitude = amplitudeFrom (sample.raw);
        snapshot_.amplitude     = amplitude;

        if (snapshot_.phase == AcousticPhase::Calibrating)
        {
            updateBaseline (sample.raw);

            snapshot_.amplitude = amplitudeFrom (sample.raw);

            if (sample.observedAt.elapsedSince (calibrationStartedAt_) >=
                config_.calibration)
            {
                if (!validHeadroom ())
                {
                    return enterFault (sample, AcousticQuality::Unqualified,
                                       StatusCode::InvalidArgument, false);
                }

                snapshot_.phase   = AcousticPhase::Quiet;
                snapshot_.quality = AcousticQuality::ValidQuiet;
            }

            rememberSample (sample);
            return snapshot_.status;
        }

        const Status thresholdStatus = checkThreshold (sample, amplitude);

        if (!thresholdStatus.ok ())
        {
            return thresholdStatus;
        }

        if (snapshot_.phase == AcousticPhase::EventOpen)
        {
            if (amplitude > snapshot_.peakAmplitude)
            {
                snapshot_.peakAmplitude = amplitude;
            }

            if (amplitude <= config_.releaseAboveBaseline)
            {
                if (!quietCandidate_)
                {
                    quietCandidate_ = true;
                    quietStartedAt_ = sample.observedAt;
                }
            }
            else
            {
                quietCandidate_ = false;
            }

            const bool quietComplete =
                quietCandidate_ &&
                sample.observedAt.elapsedSince (quietStartedAt_) >=
                    config_.quietToClose;
            const bool windowComplete =
                sample.observedAt.elapsedSince (eventStartedAt_) >=
                config_.eventWindow;

            if (quietComplete || windowComplete)
            {
                completeEvent (sample);
            }

            rememberSample (sample);
            return snapshot_.status;
        }

        if (snapshot_.phase == AcousticPhase::Refractory)
        {
            if (sample.observedAt.elapsedSince (refractoryStartedAt_) <
                config_.refractory)
            {
                snapshot_.quality = AcousticQuality::ValidQuiet;
                snapshot_.status  = StatusCode::Ok;
                rememberSample (sample);
                return snapshot_.status;
            }

            snapshot_.phase   = AcousticPhase::Quiet;
            snapshot_.quality = AcousticQuality::ValidQuiet;
        }

        if (amplitude >= config_.attackAboveBaseline)
        {
            openEvent (sample, amplitude);
        }
        else
        {
            updateBaseline (sample.raw);

            if (!validHeadroom ())
            {
                return enterFault (sample, AcousticQuality::Unqualified,
                                   StatusCode::InvalidArgument, false);
            }

            snapshot_.phase     = AcousticPhase::Quiet;
            snapshot_.quality   = AcousticQuality::ValidQuiet;
            snapshot_.status    = StatusCode::Ok;
        }

        rememberSample (sample);
        return snapshot_.status;
    }

    bool AcousticEnvelope::initialized () const noexcept
    {
        return initialized_;
    }

    AcousticObservation AcousticEnvelope::snapshot () const noexcept
    {
        return snapshot_;
    }

    bool AcousticEnvelope::validConfig () const noexcept
    {
        const uint16_t usableSpan =
            static_cast<uint16_t> (adcMaximum - 2U * config_.railMargin);

        return config_.railMargin >= 1 && config_.railMargin <= 511 &&
               config_.releaseAboveBaseline < config_.attackAboveBaseline &&
               config_.attackAboveBaseline != 0 &&
               config_.attackAboveBaseline < usableSpan &&
               config_.baselineShift >= 1 && config_.baselineShift <= 8 &&
               validDuration (config_.calibration) &&

               validDuration (config_.eventWindow) &&

               validDuration (config_.quietToClose) &&

               validDuration (config_.refractory) &&
               config_.quietToClose <= config_.eventWindow &&
               validLevel (config_.thresholdActiveLevel) &&
               (config_.hasThreshold ||
                config_.thresholdActiveLevel == Level::Low);
    }

    bool AcousticEnvelope::sameSample (
        const AcousticSample& sample) const noexcept
    {
        return sample.observedAt == lastSample_.observedAt &&
               sample.raw == lastSample_.raw &&
               sample.hasThreshold == lastSample_.hasThreshold &&
               sample.thresholdLevel == lastSample_.thresholdLevel &&
               sample.analogStatus == lastSample_.analogStatus &&
               sample.thresholdStatus == lastSample_.thresholdStatus;
    }

    uint16_t AcousticEnvelope::amplitudeFrom (uint16_t raw) const noexcept
    {
        return raw >= snapshot_.baseline
                   ? static_cast<uint16_t> (raw - snapshot_.baseline)
                   : static_cast<uint16_t> (snapshot_.baseline - raw);
    }

    bool AcousticEnvelope::validHeadroom () const noexcept
    {
        const uint16_t low =
            static_cast<uint16_t> (snapshot_.baseline - config_.railMargin);
        const uint16_t high = static_cast<uint16_t> (
            adcMaximum - config_.railMargin - snapshot_.baseline);
        const uint16_t headroom = low > high ? low : high;

        return snapshot_.baseline > config_.railMargin &&
               snapshot_.baseline < adcMaximum - config_.railMargin &&
               headroom > config_.attackAboveBaseline;
    }

    void AcousticEnvelope::clearCurrentEvent () noexcept
    {
        snapshot_.peakAmplitude    = 0;
        snapshot_.relativeIntensity = 0;
        snapshot_.eventStartedAt   = TimePoint ();

        snapshot_.eventDuration    = Duration ();
    }

    void AcousticEnvelope::rememberSample (
        const AcousticSample& sample) noexcept
    {
        lastSample_ = sample;
        hasSample_  = true;
    }

    Status AcousticEnvelope::enterFault (const AcousticSample& sample,
                                         AcousticQuality      quality,
                                         Status               status,
                                         bool preserveEvidence) noexcept
    {
        snapshot_.observedAt         = sample.observedAt;
        snapshot_.raw                = sample.raw;
        snapshot_.rawThresholdActive =
            config_.hasThreshold && sample.hasThreshold &&
            sample.thresholdLevel == config_.thresholdActiveLevel;
        snapshot_.eventStarted   = false;
        snapshot_.eventCompleted = false;
        snapshot_.phase          = AcousticPhase::Fault;
        snapshot_.quality        = quality;
        snapshot_.status         = status;

        if (!preserveEvidence)
        {
            snapshot_.amplitude = 0;
            clearCurrentEvent ();
        }
        else if (hasCompleted_)
        {
            snapshot_.peakAmplitude     = completedPeak_;
            snapshot_.relativeIntensity = completedIntensity_;
            snapshot_.eventStartedAt    = completedStartedAt_;
            snapshot_.eventDuration     = completedDuration_;
        }
        else
        {
            clearCurrentEvent ();
        }

        rememberSample (sample);
        return snapshot_.status;
    }

    Status AcousticEnvelope::checkThreshold (const AcousticSample& sample,
                                             uint16_t amplitude) noexcept
    {
        if (!config_.hasThreshold)
        {
            disagreementCandidate_ = false;
            return StatusCode::Ok;
        }

        const bool analogActive = amplitude >= config_.attackAboveBaseline;

        if (snapshot_.rawThresholdActive == analogActive)
        {
            disagreementCandidate_ = false;
            return StatusCode::Ok;
        }

        if (!disagreementCandidate_)
        {
            disagreementCandidate_ = true;
            disagreementStartedAt_ = sample.observedAt;
            return StatusCode::Ok;
        }

        if (sample.observedAt.elapsedSince (disagreementStartedAt_) >=
            config_.quietToClose)
        {
            return enterFault (sample, AcousticQuality::ThresholdDisagreement,
                               StatusCode::HardwareFailure, false);
        }

        return StatusCode::Ok;
    }

    void AcousticEnvelope::updateBaseline (uint16_t raw) noexcept
    {
        const int32_t difference =
            static_cast<int32_t> (raw) - static_cast<int32_t> (snapshot_.baseline);
        const int32_t divisor = static_cast<int32_t> (
            static_cast<uint16_t> (1U << config_.baselineShift));
        const int32_t updated =
            static_cast<int32_t> (snapshot_.baseline) + difference / divisor;

        snapshot_.baseline = static_cast<uint16_t> (updated);
    }

    void AcousticEnvelope::openEvent (const AcousticSample& sample,
                                      uint16_t amplitude) noexcept
    {
        snapshot_.phase             = AcousticPhase::EventOpen;
        snapshot_.quality           = AcousticQuality::ValidEvent;
        snapshot_.status            = StatusCode::Ok;
        snapshot_.peakAmplitude     = amplitude;
        snapshot_.relativeIntensity = 0;
        snapshot_.eventStarted      = true;
        snapshot_.eventCompleted    = false;
        snapshot_.eventStartedAt    = sample.observedAt;
        snapshot_.eventDuration     = Duration ();
        eventStartedAt_             = sample.observedAt;
        quietCandidate_             = false;
    }

    void AcousticEnvelope::completeEvent (
        const AcousticSample& sample) noexcept
    {
        snapshot_.phase             = AcousticPhase::Refractory;
        snapshot_.quality           = AcousticQuality::ValidEvent;
        snapshot_.status            = StatusCode::Ok;
        snapshot_.eventCompleted    = true;
        snapshot_.eventStartedAt    = eventStartedAt_;
        snapshot_.eventDuration     =
            sample.observedAt.elapsedSince (eventStartedAt_);

        snapshot_.relativeIntensity = completedIntensity ();
        refractoryStartedAt_        = sample.observedAt;
        quietCandidate_             = false;
        completedStartedAt_         = snapshot_.eventStartedAt;
        completedDuration_          = snapshot_.eventDuration;
        completedPeak_              = snapshot_.peakAmplitude;
        completedIntensity_         = snapshot_.relativeIntensity;
        hasCompleted_               = true;
    }

    uint16_t AcousticEnvelope::completedIntensity () const noexcept
    {
        const uint16_t low =
            static_cast<uint16_t> (snapshot_.baseline - config_.railMargin);
        const uint16_t high = static_cast<uint16_t> (
            adcMaximum - config_.railMargin - snapshot_.baseline);
        const uint16_t headroom = low > high ? low : high;
        const uint16_t span =
            static_cast<uint16_t> (headroom - config_.attackAboveBaseline);
        uint16_t excess = snapshot_.peakAmplitude > config_.attackAboveBaseline
                              ? static_cast<uint16_t> (
                                    snapshot_.peakAmplitude -
                                    config_.attackAboveBaseline)
                              : 0;

        if (excess > span)
        {
            excess = span;
        }

        return static_cast<uint16_t> (
            (static_cast<uint32_t> (excess) * 1000U) / span);
    }
} // namespace adk
// clang-format on
