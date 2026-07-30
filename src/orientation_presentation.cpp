#include "orientation_presentation.h"

#include "piezo_sounder.h"

#include <limits.h>

namespace adk {
    namespace {
        constexpr int32_t  cordicErrorMilliDegrees = 4;
        constexpr int32_t  maximumCordicAngle      = 180000;
        constexpr uint16_t permilleMaximum         = 1000;
        constexpr uint32_t modularHalfRange        = 0x80000000UL;
        constexpr int64_t  normalizationScale      = INT64_C (1) << 30;
        constexpr int32_t  cordicAngles[]          = {
            45000000, 26565051, 14036243, 7125016, 3576334, 1789911, 895174, 447614,
            223811,   111906,   55953,    27976,   13988,   6994,    3497,   1749};

        OrientationEstimate emptyEstimate (Status status) noexcept
        {
            return {0, 0, OrientationQuality::Invalid, status};
        }

        BalanceLightIntent emptyLight () noexcept
        {
            return {0, 0, 0, true};
        }

        BalancePresentation emptyPresentation (Status status) noexcept
        {
            return {BalanceDirection::None, emptyLight (), {false, 0, 0}, status};
        }

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
        }

        bool validFrame (const BoardFrame& frame) noexcept
        {
            return validSignedAxisMapping ({frame.right, frame.forward, frame.up});
        }

        int64_t absolute64 (int64_t value) noexcept
        {
            return value < 0 ? -value : value;
        }

        uint64_t square (int64_t value) noexcept
        {
            return static_cast<uint64_t> (value * value);
        }

        uint64_t integerSquareRoot (uint64_t value) noexcept
        {
            uint64_t result = 0;
            uint64_t bit    = UINT64_C (1) << 62;

            while (bit > value)
            {
                bit /= 4;
            }

            while (bit != 0)
            {
                if (value >= result + bit)
                {
                    value -= result + bit;
                    result = result / 2 + bit;
                }
                else
                {
                    result /= 2;
                }
                bit /= 4;
            }

            return result;
        }

        int32_t roundMicrodegrees (int64_t value) noexcept
        {
            if (value >= 0)
            {
                return static_cast<int32_t> ((value + 500) / 1000);
            }
            return static_cast<int32_t> ((value - 500) / 1000);
        }

        int32_t atan2MilliDegrees (int64_t y, int64_t x) noexcept
        {
            if (y == 0)
            {
                return x < 0 ? 180000 : 0;
            }
            if (x == 0)
            {
                return y > 0 ? 90000 : -90000;
            }

            const int64_t magnitude =
                absolute64 (x) > absolute64 (y) ? absolute64 (x) : absolute64 (y);
            x = (x * normalizationScale) / magnitude;
            y = (y * normalizationScale) / magnitude;

            int64_t angle = 0;
            if (x < 0)
            {
                const bool upper = y >= 0;
                x                = -x;
                y                = -y;
                angle            = upper ? 180000000 : -180000000;
            }

            for (uint8_t index = 0; index < 16 && y != 0; ++index)
            {
                const int64_t divisor = INT64_C (1) << index;
                const int64_t oldX    = x;
                const int64_t oldY    = y;

                if (oldY > 0)
                {
                    x = oldX + oldY / divisor;
                    y = oldY - oldX / divisor;
                    angle += cordicAngles[index];
                }
                else
                {
                    x = oldX - oldY / divisor;
                    y = oldY + oldX / divisor;
                    angle -= cordicAngles[index];
                }
            }

            while (angle > 180000000)
            {
                angle -= 360000000;
            }
            while (angle < -180000000)
            {
                angle += 360000000;
            }
            return roundMicrodegrees (angle);
        }

        bool validQuality (InertialSampleQuality quality) noexcept
        {
            return quality >= InertialSampleQuality::Invalid &&
                   quality <= InertialSampleQuality::Saturated;
        }

        bool validSource (const InertialSource& source) noexcept
        {
            const bool recognizedKind =
                source.kind >= InertialSourceKind::SyntheticFixture &&
                source.kind <= InertialSourceKind::Qmi8658Adapter;
            const bool recognizedModel =
                source.model >= InertialModel::Synthetic &&
                source.model <= InertialModel::Qmi8658UnknownRevision;
            if (!recognizedKind || !recognizedModel || source.sourceId == 0 ||
                source.configurationRevision == 0 || source.calibrationRevision == 0 ||
                source.accelerationRangeMicroG == 0 ||
                source.accelerationRangeMicroG > static_cast<uint32_t> (INT32_MAX) ||
                source.angularRateRangeMilliDegreesPerSecond == 0 ||
                source.angularRateRangeMilliDegreesPerSecond >
                    static_cast<uint32_t> (INT32_MAX))
            {
                return false;
            }

            return (source.kind == InertialSourceKind::SyntheticFixture &&
                    source.model == InertialModel::Synthetic) ||
                   (source.kind == InertialSourceKind::Mpu6050Adapter &&
                    source.model == InertialModel::Mpu6050) ||
                   (source.kind == InertialSourceKind::Qmi8658Adapter &&
                    source.model == InertialModel::Qmi8658UnknownRevision);
        }

        bool withinRange (const InertialVector& vector, uint32_t range) noexcept
        {
            return static_cast<uint64_t> (absolute64 (vector.x)) < range &&
                   static_cast<uint64_t> (absolute64 (vector.y)) < range &&
                   static_cast<uint64_t> (absolute64 (vector.z)) < range;
        }

        bool beyondRate (const InertialVector& rate, int32_t maximum) noexcept
        {
            return absolute64 (rate.x) > maximum || absolute64 (rate.y) > maximum ||
                   absolute64 (rate.z) > maximum;
        }

        int32_t absoluteAngle (int32_t value) noexcept
        {
            return value < 0 ? -value : value;
        }

        bool validPresentationAngle (int32_t value) noexcept
        {
            return value >= -maximumCordicAngle && value <= maximumCordicAngle;
        }

        bool validLight (const BalanceLightIntent& light) noexcept
        {
            return light.redPermille <= permilleMaximum &&
                   light.greenPermille <= permilleMaximum &&
                   light.bluePermille <= permilleMaximum;
        }

        BalanceDirection directionFor (const OrientationEstimate& estimate) noexcept
        {
            const int32_t pitch = absoluteAngle (estimate.pitchMilliDegrees);
            const int32_t roll  = absoluteAngle (estimate.rollMilliDegrees);
            const bool    pitchWins =
                pitch >= roll ||
                static_cast<int64_t> (roll) - pitch <= 2 * cordicErrorMilliDegrees;

            if (pitchWins)
            {
                return estimate.pitchMilliDegrees >= 0 ? BalanceDirection::Forward
                                                       : BalanceDirection::Backward;
            }
            return estimate.rollMilliDegrees >= 0 ? BalanceDirection::Right
                                                  : BalanceDirection::Left;
        }

        BalanceLightIntent directionLight (const BalancePresentationConfig& config,
                                           BalanceDirection direction) noexcept
        {
            switch (direction)
            {
                case BalanceDirection::Forward: return config.forward;
                case BalanceDirection::Backward: return config.backward;
                case BalanceDirection::Left: return config.left;
                case BalanceDirection::Right: return config.right;
                default: return config.invalid;
            }
        }

        uint16_t tiltIntensity (int32_t angle, uint16_t sensitivity,
                                const BalancePresentationConfig& config) noexcept
        {
            uint64_t scaled =
                static_cast<uint64_t> (absoluteAngle (angle)) * sensitivity;
            scaled /= static_cast<uint32_t> (config.fullScaleAngleMilliDegrees);
            if (scaled < config.minimumTiltIntensityPermille)
            {
                scaled = config.minimumTiltIntensityPermille;
            }
            if (scaled > config.maximumTiltIntensityPermille)
            {
                scaled = config.maximumTiltIntensityPermille;
            }
            return static_cast<uint16_t> (scaled);
        }

        BalanceLightIntent scaledLight (BalanceLightIntent light,
                                        uint16_t           intensity) noexcept
        {
            light.redPermille = static_cast<uint16_t> (
                (static_cast<uint32_t> (light.redPermille) * intensity) /
                permilleMaximum);
            light.greenPermille = static_cast<uint16_t> (
                (static_cast<uint32_t> (light.greenPermille) * intensity) /
                permilleMaximum);
            light.bluePermille = static_cast<uint16_t> (
                (static_cast<uint32_t> (light.bluePermille) * intensity) /
                permilleMaximum);
            return light;
        }
    } // namespace

    Status validateOrientationConfig (const OrientationConfig& config) noexcept
    {
        if (!validFrame (config.boardFrame) || config.minimumGravityMicroG <= 0 ||
            config.maximumGravityMicroG < config.minimumGravityMicroG ||
            config.maximumStationaryRateMilliDegreesPerSecond <= 0 ||
            config.levelThresholdMilliDegrees < 0 ||
            config.maximumPresentationAngleMilliDegrees <=
                config.levelThresholdMilliDegrees ||
            config.maximumPresentationAngleMilliDegrees > maximumCordicAngle)
        {
            return StatusCode::InvalidConfiguration;
        }
        return StatusCode::Ok;
    }

    Status
    validateBalancePresentationConfig (const BalancePresentationConfig& config) noexcept
    {
        const bool validLights =
            validLight (config.level) && validLight (config.forward) &&
            validLight (config.backward) && validLight (config.left) &&
            validLight (config.right) && validLight (config.unsteadyPhaseA) &&
            validLight (config.unsteadyPhaseB) && validLight (config.beyondRange) &&
            validLight (config.invalid);
        const bool toneDisabled = config.directionChangeFrequencyHertz == 0 &&
                                  config.directionChangeDurationMilliseconds == 0;
        const bool toneEnabled =
            config.directionChangeFrequencyHertz >= PiezoSounder::minimumFrequencyHz &&
            config.directionChangeFrequencyHertz <= PiezoSounder::maximumFrequencyHz &&
            config.directionChangeDurationMilliseconds > 0 &&
            config.directionChangeDurationMilliseconds <=
                PiezoSounder::maximumDurationMs;

        if (!validLights || config.fullScaleAngleMilliDegrees <= 0 ||
            config.fullScaleAngleMilliDegrees > maximumCordicAngle ||
            config.minimumTiltIntensityPermille == 0 ||
            config.minimumTiltIntensityPermille > config.maximumTiltIntensityPermille ||
            config.maximumTiltIntensityPermille > permilleMaximum ||
            (!toneDisabled && !toneEnabled))
        {
            return StatusCode::InvalidConfiguration;
        }
        return StatusCode::Ok;
    }

    PreparedOrientationEstimate::PreparedOrientationEstimate () noexcept
        : result_ (), owner_ (nullptr), generation_ (0)
    {
    }

    const OrientationEstimate&
    PreparedOrientationEstimate::result () const noexcept
    {
        return result_;
    }

    OrientationPolicy::OrientationPolicy (const OrientationConfig& config) noexcept
        : config_ (config), estimate_ (), generation_ (0), initialized_ (false)
    {
        estimate_ = emptyEstimate (StatusCode::NotInitialized);
    }

    Status OrientationPolicy::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const Status status =
            validateOrientationConfig (config_);
        if (!status.ok ())
        {
            estimate_ = emptyEstimate (status);
            return estimate_.status;
        }

        initialized_ = true;
        reset ();
        return StatusCode::Ok;
    }

    void OrientationPolicy::reset () noexcept
    {
        estimate_ =
            emptyEstimate (initialized_ ? StatusCode::Ok : StatusCode::NotInitialized);
        ++generation_;
    }

    Status OrientationPolicy::preview (const InertialObservation& input,
                                       PreparedOrientationEstimate& prepared) const
        noexcept
    {
        prepared.owner_               = this;
        prepared.generation_          = generation_;
        OrientationEstimate& estimate = prepared.result_;
        if (!initialized_)
        {
            estimate        = emptyEstimate (StatusCode::NotInitialized);
            prepared.owner_ = nullptr;
            return estimate.status;
        }

        const uint32_t maximumAge = input.maximumAge.milliseconds ();
        if (maximumAge == 0 || maximumAge >= modularHalfRange ||
            input.freshnessContractRevision == 0 ||
            !validSource (input.sample.source) || !input.sample.dataReady ||
            input.sample.saturation < InertialSaturation::None ||
            input.sample.saturation > InertialSaturation::Both)
        {
            estimate        = emptyEstimate (StatusCode::InvalidArgument);
            prepared.owner_ = nullptr;
            return estimate.status;
        }

        if (!validStatus (input.status) || !validStatus (input.sample.status) ||
            !validQuality (input.quality))
        {
            estimate        = emptyEstimate (StatusCode::InvalidArgument);
            prepared.owner_ = nullptr;
            return estimate.status;
        }
        if (!input.status.ok ())
        {
            estimate = emptyEstimate (input.status);
            return estimate.status;
        }
        if (!input.sample.status.ok ())
        {
            estimate = emptyEstimate (input.sample.status);
            return estimate.status;
        }

        if (input.quality != InertialSampleQuality::Current)
        {
            estimate = emptyEstimate (StatusCode::InvalidArgument);
            return estimate.status;
        }

        if (!input.sample.dataReady ||
            input.sample.saturation != InertialSaturation::None ||
            !withinRange (input.sample.accelerationMicroG,
                          input.sample.source.accelerationRangeMicroG) ||
            !withinRange (input.sample.angularRateMilliDegreesPerSecond,
                          input.sample.source.angularRateRangeMilliDegreesPerSecond) ||
            input.age.milliseconds () > maximumAge || !input.latestDataReady)
        {
            estimate        = emptyEstimate (StatusCode::InvalidArgument);
            prepared.owner_ = nullptr;
            return estimate.status;
        }

        const InertialVector& acceleration = input.sample.accelerationMicroG;
        const uint64_t        magnitudeSquared =
            square (acceleration.x) + square (acceleration.y) + square (acceleration.z);
        const uint64_t gravity = integerSquareRoot (magnitudeSquared);
        if (gravity < static_cast<uint32_t> (config_.minimumGravityMicroG) ||
            gravity > static_cast<uint32_t> (config_.maximumGravityMicroG) ||
            beyondRate (input.sample.angularRateMilliDegreesPerSecond,
                        config_.maximumStationaryRateMilliDegreesPerSecond))
        {
            estimate = {0, 0, OrientationQuality::Unsteady, StatusCode::Ok};
            return StatusCode::Ok;
        }

        int32_t rightValue;
        int32_t forwardValue;
        int32_t upValue;
        if (!mapSignedAxes ({config_.boardFrame.right, config_.boardFrame.forward,
                             config_.boardFrame.up},
                            acceleration.x, acceleration.y, acceleration.z, rightValue,
                            forwardValue, upValue))
        {
            estimate        = emptyEstimate (StatusCode::InvalidArgument);
            prepared.owner_ = nullptr;
            return estimate.status;
        }
        const int64_t right   = rightValue;
        const int64_t forward = forwardValue;
        const int64_t up      = upValue;
        const uint64_t horizontal =
            integerSquareRoot (square (right) + square (up));

        estimate.pitchMilliDegrees =
            atan2MilliDegrees (forward, static_cast<int64_t> (horizontal));
        estimate.rollMilliDegrees = atan2MilliDegrees (right, up);
        estimate.status           = StatusCode::Ok;

        const int32_t pitch = absoluteAngle (estimate.pitchMilliDegrees);
        const int32_t roll  = absoluteAngle (estimate.rollMilliDegrees);
        if (static_cast<int64_t> (pitch) + cordicErrorMilliDegrees <=
                config_.levelThresholdMilliDegrees &&
            static_cast<int64_t> (roll) + cordicErrorMilliDegrees <=
                config_.levelThresholdMilliDegrees)
        {
            estimate.quality = OrientationQuality::Level;
        }
        else if (static_cast<int64_t> (pitch) + cordicErrorMilliDegrees >
                     config_.maximumPresentationAngleMilliDegrees ||
                 static_cast<int64_t> (roll) + cordicErrorMilliDegrees >
                     config_.maximumPresentationAngleMilliDegrees)
        {
            estimate.quality = OrientationQuality::BeyondPresentationRange;
        }
        else
        {
            estimate.quality = OrientationQuality::Tilted;
        }
        return StatusCode::Ok;
    }

    bool OrientationPolicy::canCommit (
        const PreparedOrientationEstimate& prepared) const noexcept
    {
        return initialized_ && prepared.owner_ == this &&
               prepared.generation_ == generation_;
    }

    Status OrientationPolicy::commit (
        const PreparedOrientationEstimate& prepared) noexcept
    {
        if (!canCommit (prepared))
        {
            return initialized_ ? StatusCode::InvalidArgument :
                                  StatusCode::NotInitialized;
        }
        estimate_ = prepared.result_;
        ++generation_;
        return StatusCode::Ok;
    }

    Status OrientationPolicy::update (const InertialObservation& input) noexcept
    {
        PreparedOrientationEstimate prepared;
        const Status status =
            preview (input, prepared);
        if (canCommit (prepared))
        {
            static_cast<void> (commit (prepared));
        }
        else
        {
            estimate_ = prepared.result_;
            ++generation_;
        }
        return status;
    }

    OrientationEstimate OrientationPolicy::snapshot () const noexcept
    {
        return estimate_;
    }

    bool OrientationPolicy::initialized () const noexcept
    {
        return initialized_;
    }

    PreparedBalancePresentation::PreparedBalancePresentation () noexcept
        : result_ (), owner_ (nullptr), generation_ (0)
    {
    }

    const BalancePresentation&
    PreparedBalancePresentation::result () const noexcept
    {
        return result_;
    }

    BalancePresentationPolicy::BalancePresentationPolicy (
        const BalancePresentationConfig& config) noexcept
        : config_ (config), presentation_ (), previousDirection_ (),
          generation_ (0), initialized_ (false)
    {
        presentation_      = emptyPresentation (StatusCode::NotInitialized);
        previousDirection_ = BalanceDirection::None;
    }

    Status BalancePresentationPolicy::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const Status status =
            validateBalancePresentationConfig (config_);
        if (!status.ok ())
        {
            presentation_ = emptyPresentation (status);
            return presentation_.status;
        }

        initialized_ = true;
        reset ();
        return StatusCode::Ok;
    }

    void BalancePresentationPolicy::reset () noexcept
    {
        presentation_ = emptyPresentation (initialized_ ? StatusCode::Ok
                                                        : StatusCode::NotInitialized);
        previousDirection_ = BalanceDirection::None;
        ++generation_;
    }

    Status BalancePresentationPolicy::preview (
        const OrientationEstimate& estimate, uint16_t sensitivityPermille,
        bool diagnosticPhase, PreparedBalancePresentation& prepared) const noexcept
    {
        prepared.owner_                   = this;
        prepared.generation_              = generation_;
        BalancePresentation& presentation = prepared.result_;
        if (!initialized_)
        {
            presentation    = emptyPresentation (StatusCode::NotInitialized);
            prepared.owner_ = nullptr;
            return presentation.status;
        }
        if (sensitivityPermille == 0 || sensitivityPermille > permilleMaximum ||
            !validStatus (estimate.status) ||
            estimate.quality < OrientationQuality::Invalid ||
            estimate.quality > OrientationQuality::BeyondPresentationRange ||
            !validPresentationAngle (estimate.pitchMilliDegrees) ||
            !validPresentationAngle (estimate.rollMilliDegrees))
        {
            presentation    = emptyPresentation (StatusCode::InvalidArgument);
            prepared.owner_ = nullptr;
            return presentation.status;
        }

        presentation.direction = BalanceDirection::None;
        presentation.tone      = {false, 0, 0};
        presentation.status    = estimate.status;

        if (!estimate.status.ok () || estimate.quality == OrientationQuality::Invalid)
        {
            presentation.light = config_.invalid;
            if (presentation.status.ok ())
            {
                presentation.status = StatusCode::InvalidArgument;
            }
            return presentation.status;
        }
        if (estimate.quality == OrientationQuality::Unsteady)
        {
            presentation.light =
                diagnosticPhase ? config_.unsteadyPhaseB : config_.unsteadyPhaseA;
            return StatusCode::Ok;
        }
        if (estimate.quality == OrientationQuality::BeyondPresentationRange)
        {
            presentation.light = config_.beyondRange;
            return StatusCode::Ok;
        }
        if (estimate.quality == OrientationQuality::Level)
        {
            presentation.light = config_.level;
            return StatusCode::Ok;
        }

        const BalanceDirection direction = directionFor (estimate);
        const int32_t  dominant = direction == BalanceDirection::Forward ||
                                          direction == BalanceDirection::Backward
                                      ? estimate.pitchMilliDegrees
                                      : estimate.rollMilliDegrees;
        const uint16_t intensity =
            tiltIntensity (dominant, sensitivityPermille, config_);

        presentation.direction = direction;
        presentation.light =
            scaledLight (directionLight (config_, direction), intensity);
        if (previousDirection_ != BalanceDirection::None &&
            previousDirection_ != direction &&
            config_.directionChangeFrequencyHertz != 0)
        {
            presentation.tone = {true, config_.directionChangeFrequencyHertz,
                                 config_.directionChangeDurationMilliseconds};
        }
        return StatusCode::Ok;
    }

    bool BalancePresentationPolicy::canCommit (
        const PreparedBalancePresentation& prepared) const noexcept
    {
        return initialized_ && prepared.owner_ == this &&
               prepared.generation_ == generation_;
    }

    Status BalancePresentationPolicy::commit (
        const PreparedBalancePresentation& prepared) noexcept
    {
        if (!canCommit (prepared))
        {
            return initialized_ ? StatusCode::InvalidArgument :
                                  StatusCode::NotInitialized;
        }
        presentation_      = prepared.result_;
        previousDirection_ = prepared.result_.direction;
        ++generation_;
        return StatusCode::Ok;
    }

    Status BalancePresentationPolicy::update (const OrientationEstimate& estimate,
                                              uint16_t sensitivityPermille,
                                              bool     diagnosticPhase) noexcept
    {
        PreparedBalancePresentation prepared;
        const Status                status =
            preview (estimate, sensitivityPermille, diagnosticPhase, prepared);
        if (canCommit (prepared))
        {
            static_cast<void> (commit (prepared));
        }
        else
        {
            presentation_      = prepared.result_;
            previousDirection_ = BalanceDirection::None;
            ++generation_;
        }
        return status;
    }

    BalancePresentation BalancePresentationPolicy::snapshot () const noexcept
    {
        return presentation_;
    }

    bool BalancePresentationPolicy::initialized () const noexcept
    {
        return initialized_;
    }
} // namespace adk
