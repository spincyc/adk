#include "balance_table_instrument.h"

#include <limits.h>

namespace adk {
    namespace {
        constexpr uint32_t halfRange = 0x80000000UL;

        CompactInertialEvidence emptyProvenance () noexcept
        {
            return {{InertialSourceKind::SyntheticFixture, InertialModel::Synthetic, 0,
                     0, 0, 0, 0},
                    TimePoint (),
                    0,
                    InertialSampleQuality::Invalid,
                    Duration (),
                    0,
                    InertialSaturation::None,
                    false,
                    false,
                    StatusCode::NotInitialized};
        }

        BalanceMeasurementEvidence emptyEvidence () noexcept
        {
            return {emptyProvenance (),
                    {0, 0, OrientationQuality::Invalid, StatusCode::NotInitialized},
                    false};
        }

        BalancePresentation emptyPresentation (Status status) noexcept
        {
            return {BalanceDirection::None, {0, 0, 0, true}, {false, 0, 0}, status};
        }

        BalanceInstrumentOutput emptyOutput () noexcept
        {
            return {BalanceInstrumentMode::AwaitingFrame,
                    emptyEvidence     (),
                    emptyEvidence     (),
                    emptyPresentation (StatusCode::NotInitialized),
                    0,
                    0,
                    TimePoint (),
                    StatusCode::NotInitialized,
                    StatusCode::NotInitialized,
                    StatusCode::NotInitialized,
                    StatusCode::NotInitialized};
        }

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
        }

        bool validSensitivityEvent (SensitivityEvent event) noexcept
        {
            return event >= SensitivityEvent::None &&
                   event <= SensitivityEvent::Contradictory;
        }

        bool validQuality (InertialSampleQuality quality) noexcept
        {
            return quality >= InertialSampleQuality::Invalid &&
                   quality <= InertialSampleQuality::Saturated;
        }

        bool validSaturation (InertialSaturation saturation) noexcept
        {
            return saturation >= InertialSaturation::None &&
                   saturation <= InertialSaturation::Both;
        }

        bool validDirection (BalanceDirection direction) noexcept
        {
            return direction >= BalanceDirection::None &&
                   direction <= BalanceDirection::Right;
        }

        bool validCanonicalPresentation (const BalancePresentation& presentation,
                                         bool                       requireFault,
                                         Status requiredStatus) noexcept
        {
            return validDirection (presentation.direction) &&
                   presentation.direction == BalanceDirection::None &&
                   presentation.status == requiredStatus &&
                   presentation.light.redPermille <= 1000 &&
                   presentation.light.greenPermille <= 1000 &&
                   presentation.light.bluePermille <= 1000 &&
                   presentation.light.fault == requireFault &&
                   !presentation.tone.enabled &&
                   presentation.tone.frequencyHertz == 0 &&
                   presentation.tone.durationMilliseconds == 0;
        }

        BalancePresentation
        canonicalPresentation (const BalancePresentation& configured,
                               Status                     status) noexcept
        {
            BalancePresentation result = configured;
            result.status              = status;
            result.tone                = {false, 0, 0};
            return result;
        }

        bool validSource (const InertialSource& source) noexcept
        {
            return source.kind == InertialSourceKind::SyntheticFixture &&
                   source.model == InertialModel::Synthetic && source.sourceId != 0 &&
                   source.configurationRevision != 0 &&
                   source.calibrationRevision != 0 &&
                   source.accelerationRangeMicroG != 0 &&
                   source.accelerationRangeMicroG <=
                       static_cast<uint32_t> (INT32_MAX) &&
                   source.angularRateRangeMilliDegreesPerSecond != 0 &&
                   source.angularRateRangeMilliDegreesPerSecond <=
                       static_cast<uint32_t> (INT32_MAX);
        }

        uint32_t magnitude (int32_t value) noexcept
        {
            const int64_t widened = value;
            return static_cast<uint32_t> (widened < 0 ? -widened : widened);
        }

        bool beyondRange (const InertialVector& vector, uint32_t range) noexcept
        {
            return magnitude (vector.x) > range || magnitude (vector.y) > range ||
                   magnitude (vector.z) > range;
        }

        bool atRange (const InertialVector& vector, uint32_t range) noexcept
        {
            return magnitude (vector.x) == range || magnitude (vector.y) == range ||
                   magnitude (vector.z) == range;
        }

        InertialSaturation measuredSaturation (const InertialSample& sample) noexcept
        {
            const uint8_t acceleration = atRange (sample.accelerationMicroG,
                                                  sample.source.accelerationRangeMicroG)
                                             ? 1U
                                             : 0U;
            const uint8_t angularRate =
                atRange (sample.angularRateMilliDegreesPerSecond,
                         sample.source.angularRateRangeMilliDegreesPerSecond)
                    ? 2U
                    : 0U;
            return static_cast<InertialSaturation> (acceleration | angularRate);
        }

        bool sameSource (const InertialSource& left,
                         const InertialSource& right) noexcept
        {
            return left.kind == right.kind && left.model == right.model &&
                   left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.calibrationRevision == right.calibrationRevision &&
                   left.accelerationRangeMicroG == right.accelerationRangeMicroG &&
                   left.angularRateRangeMilliDegreesPerSecond ==
                       right.angularRateRangeMilliDegreesPerSecond;
        }

        bool sameVector (const InertialVector& left,
                         const InertialVector& right) noexcept
        {
            return left.x == right.x && left.y == right.y && left.z == right.z;
        }

        bool sameSample (const InertialSample& left,
                         const InertialSample& right) noexcept
        {
            return sameSource (left.source, right.source) &&
                   sameVector (left.accelerationMicroG, right.accelerationMicroG) &&
                   sameVector (left.angularRateMilliDegreesPerSecond,
                               right.angularRateMilliDegreesPerSecond) &&
                   left.observedAt == right.observedAt &&
                   left.sequence == right.sequence &&
                   left.dataReady == right.dataReady &&
                   left.saturation == right.saturation && left.status == right.status;
        }

        bool sameObservation (const InertialObservation& left,
                              const InertialObservation& right) noexcept
        {
            return sameSample (left.sample, right.sample) &&
                   left.quality == right.quality &&
                   left.latestDataReady == right.latestDataReady &&
                   left.age == right.age && left.maximumAge == right.maximumAge &&
                   left.freshnessContractRevision == right.freshnessContractRevision &&
                   left.sequenceGap == right.sequenceGap && left.status == right.status;
        }

        bool sameJoystick (const BalanceJoystickObservation& left,
                           const BalanceJoystickObservation& right) noexcept
        {
            return left.xPermille == right.xPermille &&
                   left.yPermille == right.yPermille && left.event == right.event &&
                   left.observedAt == right.observedAt &&
                   left.sequence == right.sequence && left.status == right.status;
        }

        bool sameButton (const BalanceButtonObservation& left,
                         const BalanceButtonObservation& right) noexcept
        {
            return left.pressed == right.pressed &&
                   left.pressEvent == right.pressEvent &&
                   left.releaseEvent == right.releaseEvent &&
                   left.observedAt == right.observedAt &&
                   left.sequence == right.sequence && left.status == right.status;
        }

        bool sameFrame (const BalanceInstrumentInput& left,
                        const BalanceInstrumentInput& right) noexcept
        {
            return sameObservation (left.inertial, right.inertial) &&
                   sameJoystick (left.joystick, right.joystick) &&
                   sameButton   (left.freezeButton, right.freezeButton) &&
                   left.frameAt == right.frameAt &&
                   left.frameSequence == right.frameSequence;
        }

        bool forwardDelta (uint32_t current, uint32_t previous) noexcept
        {
            const uint32_t delta = current - previous;
            return delta != 0 && delta < halfRange;
        }

        SensitivityEvent
        canonicalEvent (const BalanceJoystickObservation& joystick) noexcept
        {
            const bool positive = joystick.xPermille > 0 || joystick.yPermille > 0;
            const bool negative = joystick.xPermille < 0 || joystick.yPermille < 0;
            if (positive && negative)
            {
                return SensitivityEvent::Contradictory;
            }
            if (positive)
            {
                return SensitivityEvent::Increase;
            }
            if (negative)
            {
                return SensitivityEvent::Decrease;
            }
            return SensitivityEvent::None;
        }

        bool validButton (const BalanceButtonObservation& button) noexcept
        {
            if (button.pressEvent)
            {
                return button.pressed && !button.releaseEvent;
            }
            if (button.releaseEvent)
            {
                return !button.pressed;
            }
            return true;
        }

        Status validateConfig (const BalanceInstrumentConfig& config) noexcept
        {
            if (config.minimumSensitivityPermille == 0 ||
                config.minimumSensitivityPermille > config.maximumSensitivityPermille ||
                config.maximumSensitivityPermille > 1000 ||
                config.sensitivityStepPermille == 0 ||
                config.inertialMaximumAge.milliseconds () == 0 ||
                config.inertialMaximumAge.milliseconds () >= halfRange ||
                config.inertialFreshnessContractRevision == 0 ||
                config.maximumInputSkew.milliseconds () <=
                    config.inertialMaximumAge.milliseconds () ||
                config.maximumInputSkew.milliseconds () >= halfRange ||
                config.diagnosticPhase.milliseconds  () == 0 ||
                config.diagnosticPhase.milliseconds  () >= halfRange ||
                !validCanonicalPresentation          (config.awaitingFramePresentation, false,
                                             StatusCode::Ok) ||
                !validCanonicalPresentation (config.recoveringPresentation, false,
                                             StatusCode::Ok) ||
                !validCanonicalPresentation (config.faultPresentation, true,
                                             StatusCode::HardwareFailure) ||
                !validCanonicalPresentation (config.shutdownPresentation, false,
                                             StatusCode::NotInitialized))
            {
                return StatusCode::InvalidConfiguration;
            }
            return StatusCode::Ok;
        }

        Status validateControls (const BalanceInstrumentInput& input) noexcept
        {
            if (!validStatus (input.joystick.status) ||
                !validStatus           (input.freezeButton.status) ||
                !validSensitivityEvent (input.joystick.event) ||
                input.joystick.sequence == 0 || input.freezeButton.sequence == 0 ||
                input.joystick.xPermille < -1000 || input.joystick.xPermille > 1000 ||
                input.joystick.yPermille < -1000 || input.joystick.yPermille > 1000 ||
                input.joystick.event != canonicalEvent (input.joystick) ||
                !validButton                           (input.freezeButton))
            {
                return StatusCode::InvalidArgument;
            }
            return StatusCode::Ok;
        }

        Status validateSampleStructure (const InertialSample& sample) noexcept
        {
            if (!validSource (sample.source) || sample.sequence == 0 ||
                !sample.dataReady || !validStatus (sample.status) ||
                !validSaturation                  (sample.saturation) ||
                beyondRange                       (sample.accelerationMicroG,
                             sample.source.accelerationRangeMicroG) ||
                beyondRange (sample.angularRateMilliDegreesPerSecond,
                             sample.source.angularRateRangeMilliDegreesPerSecond) ||
                sample.saturation != measuredSaturation (sample))
            {
                return StatusCode::InvalidArgument;
            }
            return StatusCode::Ok;
        }

        Status canonicalizeObservation (const BalanceInstrumentInput&  input,
                                        const BalanceInstrumentConfig& config,
                                        const BalanceFrameStorage&     replayStorage,
                                        InertialObservation& canonical) noexcept
        {
            canonical = input.inertial;
            if (!validQuality (input.inertial.quality) ||
                !validStatus (input.inertial.status))
            {
                return StatusCode::InvalidArgument;
            }

            const Status sampleStatus = validateSampleStructure (input.inertial.sample);

            if (!sampleStatus.ok ())
            {
                return sampleStatus;
            }

            if (input.inertial.maximumAge != config.inertialMaximumAge ||
                input.inertial.freshnessContractRevision !=
                    config.inertialFreshnessContractRevision)
            {
                return StatusCode::InvalidConfiguration;
            }

            const uint32_t age =
                input.frameAt.elapsedSince (input.inertial.sample.observedAt)
                    .milliseconds ();
            if (age >= halfRange)
            {
                return StatusCode::InvalidArgument;
            }

            uint32_t sequenceGap = 0;
            if (replayStorage.available)
            {
                const InertialSample& previous = replayStorage.previous.inertial.sample;
                if (sameSource (input.inertial.sample.source, previous.source))
                {
                    const uint32_t delta =
                        input.inertial.sample.sequence - previous.sequence;
                    if (delta == 0)
                    {
                        if (!sameSample (input.inertial.sample, previous))
                        {
                            return StatusCode::InvalidArgument;
                        }
                    }
                    else if (delta < halfRange)
                    {
                        sequenceGap = delta - 1;
                    }
                    else
                    {
                        return StatusCode::InvalidArgument;
                    }
                }
            }

            canonical.age         = Duration (age);
            canonical.sequenceGap = sequenceGap;
            if (!input.inertial.status.ok ())
            {
                canonical.quality = InertialSampleQuality::Invalid;
                canonical.status  = input.inertial.status;
            }
            else if (!input.inertial.sample.status.ok ())
            {
                canonical.quality = InertialSampleQuality::Invalid;
                canonical.status  = input.inertial.sample.status;
            }
            else
            {
                canonical.status = StatusCode::Ok;
                if (input.inertial.sample.saturation != InertialSaturation::None)
                {
                    canonical.quality = InertialSampleQuality::Saturated;
                }
                else if (!input.inertial.latestDataReady ||
                         age > config.inertialMaximumAge.milliseconds ())
                {
                    canonical.quality = InertialSampleQuality::Stale;
                }
                else
                {
                    canonical.quality = InertialSampleQuality::Current;
                }
            }

            if (canonical.quality != input.inertial.quality ||
                canonical.age != input.inertial.age ||
                canonical.sequenceGap != input.inertial.sequenceGap ||
                canonical.status != input.inertial.status)
            {
                return StatusCode::InvalidArgument;
            }
            return StatusCode::Ok;
        }

        CompactInertialEvidence
        evidenceFor (const InertialObservation& observation) noexcept
        {
            return {
                observation.sample.source,     observation.sample.observedAt,
                observation.sample.sequence,   observation.quality,
                observation.maximumAge,        observation.freshnessContractRevision,
                observation.sample.saturation, observation.sample.dataReady,
                observation.latestDataReady,   observation.status};
        }

        bool eligible (const OrientationEstimate& estimate) noexcept
        {
            return estimate.status.ok () &&
                   (estimate.quality == OrientationQuality::Level ||
                    estimate.quality == OrientationQuality::Tilted);
        }

        bool phaseFor (TimePoint frameAt, TimePoint epoch, Duration phase) noexcept
        {
            return ((frameAt.elapsedSince (epoch).milliseconds () /
                     phase.milliseconds ()) &
                    1U) != 0;
        }

        uint16_t initialSensitivity (const BalanceInstrumentConfig& config) noexcept
        {
            return static_cast<uint16_t> (config.minimumSensitivityPermille +
                                          (config.maximumSensitivityPermille -
                                           config.minimumSensitivityPermille) /
                                              2U);
        }

        uint16_t changedSensitivity (uint16_t current, SensitivityEvent event,
                                     const BalanceInstrumentConfig& config) noexcept
        {
            if (event == SensitivityEvent::Increase)
            {
                const uint32_t next =
                    static_cast<uint32_t> (current) + config.sensitivityStepPermille;
                return next > config.maximumSensitivityPermille
                           ? config.maximumSensitivityPermille
                           : static_cast<uint16_t> (next);
            }
            if (event == SensitivityEvent::Decrease)
            {
                return current <= static_cast<uint32_t> (
                                      config.minimumSensitivityPermille) +
                                      config.sensitivityStepPermille
                           ? config.minimumSensitivityPermille
                           : static_cast<uint16_t> (current -
                                                    config.sensitivityStepPermille);
            }
            return current;
        }

        Status policyConfigurationStatus (
            const OrientationConfig&         orientation,
            const BalancePresentationConfig& presentation) noexcept
        {
            const Status orientationStatus = validateOrientationConfig (orientation);

            return orientationStatus.ok ()
                       ? validateBalancePresentationConfig (presentation)
                       : orientationStatus;
        }
    } // namespace

    BalanceInstrument::BalanceInstrument (
        const BalanceInstrumentConfig&   config,
        const OrientationConfig&         orientationConfig,
        const BalancePresentationConfig& presentationConfig,
        BalanceFrameStorage&             replayStorage) noexcept
        : config_ (config), orientation_ (orientationConfig),
          presentation_              (presentationConfig),
          policyConfigurationStatus_ (
              policyConfigurationStatus (orientationConfig, presentationConfig)),
          replayStorage_             (&replayStorage), output_ (emptyOutput ()),
          initialized_               (false), hasEpoch_ (false), latestFrameHealthy_ (false),
          recoveryNeedsForwardFrame_ (false), diagnosticEpoch_ ()
    {
    }

    Status BalanceInstrument::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const Status configStatus = validateConfig (config_);

        if (!configStatus.ok ())
        {
            output_.status = configStatus;
            return output_.status;
        }
        if (!policyConfigurationStatus_.ok ())
        {
            output_.status = policyConfigurationStatus_;
            return output_.status;
        }

        Status status = orientation_.initialize ();

        if (!status.ok ())
        {
            output_.status = status;
            return output_.status;
        }
        status = presentation_.initialize ();

        if (!status.ok ())
        {
            orientation_.reset ();
            output_.status = status;
            return output_.status;
        }

        initialized_ = true;
        shutdown ();
        initialized_                = true;
        output_.mode                = BalanceInstrumentMode::AwaitingFrame;
        output_.sensitivityPermille = initialSensitivity (config_);
        output_.status              = StatusCode::Ok;
        output_.presentation =
            canonicalPresentation (config_.awaitingFramePresentation, StatusCode::Ok);
        output_.inertialStatus = StatusCode::Ok;
        output_.joystickStatus = StatusCode::Ok;
        output_.buttonStatus   = StatusCode::Ok;
        return StatusCode::Ok;
    }

    void BalanceInstrument::shutdown () noexcept
    {
        orientation_.reset  ();
        presentation_.reset ();
        replayStorage_->available = false;
        output_                   = emptyOutput           ();
        output_.presentation      = canonicalPresentation (config_.shutdownPresentation,
                                                           StatusCode::NotInitialized);
        initialized_              = false;
        hasEpoch_                 = false;
        latestFrameHealthy_       = false;
        recoveryNeedsForwardFrame_ = false;
        diagnosticEpoch_           = TimePoint ();
    }

    Status BalanceInstrument::acknowledgeFault () noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (output_.mode != BalanceInstrumentMode::Fault || !latestFrameHealthy_)
        {
            return StatusCode::InvalidArgument;
        }

        output_.mode = BalanceInstrumentMode::Recovering;
        output_.presentation =
            canonicalPresentation (config_.recoveringPresentation, StatusCode::Ok);
        output_.status             = StatusCode::Ok;
        recoveryNeedsForwardFrame_ = true;
        return StatusCode::Ok;
    }

    Status BalanceInstrument::update (const BalanceInstrumentInput& input) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (replayStorage_->available &&
            input.frameSequence == replayStorage_->previous.frameSequence)
        {
            if (input.frameAt == replayStorage_->previous.frameAt &&
                sameFrame (input, replayStorage_->previous))
            {
                return output_.status;
            }
            return StatusCode::InvalidArgument;
        }

        if (input.frameSequence == 0 ||
            (replayStorage_->available &&
             !forwardDelta (input.frameSequence,
                            replayStorage_->previous.frameSequence)))
        {
            return StatusCode::InvalidArgument;
        }

        if (replayStorage_->available &&
            input.frameAt.elapsedSince (replayStorage_->previous.frameAt)
                    .milliseconds () >= halfRange)
        {
            return StatusCode::InvalidArgument;
        }
        if (replayStorage_->available &&
            input.frameAt == replayStorage_->previous.frameAt)
        {
            return StatusCode::InvalidArgument;
        }

        const uint32_t inertialAge =
            input.frameAt.elapsedSince (input.inertial.sample.observedAt)
                .milliseconds ();
        const uint32_t joystickAge =
            input.frameAt.elapsedSince (input.joystick.observedAt).milliseconds ();
        const uint32_t buttonAge =
            input.frameAt.elapsedSince (input.freezeButton.observedAt).milliseconds ();
        if (inertialAge >= halfRange || joystickAge >= halfRange ||
            buttonAge >= halfRange)
        {
            return StatusCode::InvalidArgument;
        }

        Status status = validateControls (input);

        if (!status.ok ())
        {
            return status;
        }

        bool joystickEventIsNew = true;
        bool buttonEventIsNew   = true;
        if (replayStorage_->available)
        {
            const BalanceJoystickObservation& previousJoystick =
                replayStorage_->previous.joystick;
            const uint32_t joystickDelta =
                input.joystick.sequence - previousJoystick.sequence;
            if (joystickDelta == 0)
            {
                if (!sameJoystick (input.joystick, previousJoystick))
                {
                    return StatusCode::InvalidArgument;
                }
                joystickEventIsNew = false;
            }
            else if (joystickDelta >= halfRange)
            {
                return StatusCode::InvalidArgument;
            }

            const BalanceButtonObservation& previousButton =
                replayStorage_->previous.freezeButton;
            const uint32_t buttonDelta =
                input.freezeButton.sequence - previousButton.sequence;
            if (buttonDelta == 0)
            {
                if (!sameButton (input.freezeButton, previousButton))
                {
                    return StatusCode::InvalidArgument;
                }
                buttonEventIsNew = false;
            }
            else if (buttonDelta >= halfRange)
            {
                return StatusCode::InvalidArgument;
            }
        }

        InertialObservation canonical = input.inertial;
        status = canonicalizeObservation (input, config_, *replayStorage_, canonical);

        if (!status.ok ())
        {
            return status;
        }

        const bool skewFault = inertialAge > config_.maximumInputSkew.milliseconds () ||
                               joystickAge > config_.maximumInputSkew.milliseconds () ||
                               buttonAge > config_.maximumInputSkew.milliseconds   ();

        PreparedOrientationEstimate preparedOrientation;
        status = orientation_.preview (canonical, preparedOrientation);

        if (!status.ok () && canonical.quality == InertialSampleQuality::Current &&
            canonical.status.ok ())
        {
            return status;
        }
        const OrientationEstimate  liveEstimate = preparedOrientation.result ();
        BalanceMeasurementEvidence live = {evidenceFor                       (canonical), liveEstimate, true};

        BalanceInstrumentOutput next = output_;
        next.liveEvidence            = live;
        next.acceptedFrameSequence   = input.frameSequence;
        next.acceptedFrameAt         = input.frameAt;
        next.inertialStatus          = canonical.status;
        next.joystickStatus          = input.joystick.status;
        next.buttonStatus            = input.freezeButton.status;

        const Status producerFault =
            !canonical.status.ok            ()            ? canonical.status
            : !input.freezeButton.status.ok () ? input.freezeButton.status
            : !input.joystick.status.ok     ()     ? input.joystick.status
                                               : Status (StatusCode::Ok);
        const bool dominatingFault = !producerFault.ok () || skewFault;
        const bool fullyHealthy =
            !dominatingFault && canonical.quality == InertialSampleQuality::Current &&
            canonical.latestDataReady &&
            canonical.sample.saturation == InertialSaturation::None &&
            eligible (liveEstimate) &&
            input.joystick.event != SensitivityEvent::Contradictory;

        bool suppressControls = output_.mode == BalanceInstrumentMode::Fault ||
                                output_.mode == BalanceInstrumentMode::Recovering;
        if (dominatingFault)
        {
            next.mode   = BalanceInstrumentMode::Fault;
            next.status = !producerFault.ok () ? producerFault
                                               : Status (StatusCode::InvalidArgument);
            suppressControls = true;
        }
        else if (output_.mode == BalanceInstrumentMode::Fault)
        {
            next.mode        = BalanceInstrumentMode::Fault;
            next.status      = output_.status;
            suppressControls = true;
        }
        else if (output_.mode == BalanceInstrumentMode::Recovering)
        {
            if (fullyHealthy && recoveryNeedsForwardFrame_)
            {
                next.mode   = BalanceInstrumentMode::Live;
                next.status = StatusCode::Ok;
            }
            else
            {
                next.mode   = BalanceInstrumentMode::Recovering;
                next.status = StatusCode::Ok;
            }
            suppressControls = true;
        }
        else
        {
            next.mode   = next.mode == BalanceInstrumentMode::AwaitingFrame
                              ? BalanceInstrumentMode::Live
                              : next.mode;
            next.status = StatusCode::Ok;
        }

        if (!suppressControls && fullyHealthy && buttonEventIsNew &&
            input.freezeButton.pressEvent)
        {
            if (next.mode == BalanceInstrumentMode::Frozen)
            {
                next.mode = BalanceInstrumentMode::Live;
            }
            else
            {
                next.mode           = BalanceInstrumentMode::Frozen;
                next.frozenEvidence = live;
            }
        }

        if (!suppressControls && fullyHealthy && joystickEventIsNew)
        {
            next.sensitivityPermille = changedSensitivity (
                next.sensitivityPermille, input.joystick.event, config_);
        }

        const OrientationEstimate selected =
            next.mode == BalanceInstrumentMode::Frozen && next.frozenEvidence.available
                ? next.frozenEvidence.estimate
                : liveEstimate;

        const TimePoint nextEpoch = hasEpoch_ ? diagnosticEpoch_ : input.frameAt;
        const bool      diagnosticPhase =
            phaseFor (input.frameAt, nextEpoch, config_.diagnosticPhase);

        OrientationEstimate presented = selected;
        if (next.mode == BalanceInstrumentMode::Fault)
        {
            presented = {0, 0, OrientationQuality::Invalid, next.status};
        }
        else if (next.mode == BalanceInstrumentMode::Recovering)
        {
            presented = {0, 0, OrientationQuality::Invalid, StatusCode::Ok};
        }

        PreparedBalancePresentation preparedPresentation;
        status = presentation_.preview (presented, next.sensitivityPermille,
                                        diagnosticPhase, preparedPresentation);
        if (!status.ok () && eligible (presented))
        {
            return status;
        }
        next.presentation = preparedPresentation.result ();
        if (next.mode == BalanceInstrumentMode::Fault)
        {
            next.presentation =
                canonicalPresentation (config_.faultPresentation, next.status);
        }
        else if (next.mode == BalanceInstrumentMode::Recovering)
        {
            next.presentation =
                canonicalPresentation (config_.recoveringPresentation, StatusCode::Ok);
        }
        else if (!eligible (presented))
        {
            next.presentation.tone = {false, 0, 0};
        }

        if (!orientation_.canCommit (preparedOrientation) ||
            !presentation_.canCommit (preparedPresentation))
        {
            return StatusCode::InternalInvariant;
        }
        status = orientation_.commit (preparedOrientation);
        if (!status.ok               ())
        {
            return StatusCode::InternalInvariant;
        }
        status = presentation_.commit (preparedPresentation);
        if (!status.ok                ())
        {
            return StatusCode::InternalInvariant;
        }
        output_             = next;
        hasEpoch_           = true;
        diagnosticEpoch_    = nextEpoch;
        latestFrameHealthy_ = fullyHealthy;
        if (output_.mode != BalanceInstrumentMode::Recovering)
        {
            recoveryNeedsForwardFrame_ = false;
        }
        replayStorage_->previous  = input;
        replayStorage_->available = true;
        return output_.status;
    }

    BalanceInstrumentOutput BalanceInstrument::snapshot () const noexcept
    {
        return output_;
    }

    bool BalanceInstrument::initialized () const noexcept
    {
        return initialized_;
    }
} // namespace adk
