#include "presence_model.h"

namespace adk {
    namespace {
        constexpr uint32_t halfRange  = 0x80000000UL;
        constexpr uint8_t  allSources = 0x0FU;

        constexpr uint8_t initializedFlag          = 1U << 0;
        constexpr uint8_t hasUpdateFlag            = 1U << 1;
        constexpr uint8_t candidateFlag            = 1U << 2;
        constexpr uint8_t candidateValueFlag       = 1U << 3;
        constexpr uint8_t disagreementFlag         = 1U << 2;
        constexpr uint8_t beamPassageCandidateFlag = 1U << 3;
        constexpr uint8_t passageEventFlag         = 1U << 4;
        constexpr uint8_t timingFaultFlag          = 1U << 5;
        constexpr uint8_t suppressBeamEventFlag    = 1U << 6;
        constexpr uint8_t suppressGuardEventFlag   = 1U << 7;

        bool hasFlag (uint8_t state, uint8_t flag) noexcept
        {
            return (state & flag) != 0;
        }

        void setFlag (uint8_t& state, uint8_t flag, bool value) noexcept
        {
            if (value)
            {
                state = static_cast<uint8_t> (state | flag);
            }
            else
            {
                state = static_cast<uint8_t> (state & static_cast<uint8_t> (~flag));
            }
        }

        bool validLevel (Level level) noexcept
        {
            return level == Level::Low || level == Level::High;
        }

        bool validTime (TimePoint later, TimePoint earlier) noexcept
        {
            return later.elapsedSince (earlier).milliseconds () < halfRange;
        }

        bool validDuration (Duration duration) noexcept
        {
            return duration.milliseconds () < halfRange;
        }

        bool equalProvenance (const OpticalProvenance& left,
                              const OpticalProvenance& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.calibrationRevision == right.calibrationRevision &&
                   left.observedAt == right.observedAt;
        }

        bool equalPirSample (const PirSample& left, const PirSample& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.observedAt == right.observedAt &&
                   left.rawLevel == right.rawLevel && left.status == right.status;
        }

        bool equalPirObservation (const PirObservation& left,
                                  const PirObservation& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.observedAt == right.observedAt &&
                   left.rawLevel == right.rawLevel && left.phase == right.phase &&
                   left.motionEvent == right.motionEvent &&
                   left.clearEvent == right.clearEvent &&
                   left.stableFor == right.stableFor && left.status == right.status;
        }

        bool equalBeam (const BeamObservation& left,
                        const BeamObservation& right) noexcept
        {
            return equalProvenance (left.provenance, right.provenance) &&
                   left.rawLevel == right.rawLevel &&
                   left.interrupted == right.interrupted &&
                   left.interruptionEvent == right.interruptionEvent &&
                   left.restorationEvent == right.restorationEvent &&
                   left.stableFor == right.stableFor && left.quality == right.quality &&
                   left.status == right.status;
        }

        bool equalReflective (const ReflectiveObservation& left,
                              const ReflectiveObservation& right) noexcept
        {
            return equalProvenance (left.provenance, right.provenance) &&
                   left.raw == right.raw && left.darkReference == right.darkReference &&
                   left.lightReference == right.lightReference &&
                   left.normalizedPermille == right.normalizedPermille &&
                   left.markerActive == right.markerActive &&
                   left.activationEvent == right.activationEvent &&
                   left.deactivationEvent == right.deactivationEvent &&
                   left.stableFor == right.stableFor && left.quality == right.quality &&
                   left.status == right.status;
        }

        bool equalRangeReading (const RangeReading& left,
                                const RangeReading& right) noexcept
        {
            return left.state == right.state && left.distanceMm == right.distanceMm &&
                   left.echoDuration.microseconds () ==
                       right.echoDuration.microseconds () &&
                   left.valid == right.valid;
        }

        bool equalRange (const TimedRangeEvidence& left,
                         const TimedRangeEvidence& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.startedAt == right.startedAt &&
                   left.completedAt == right.completedAt &&
                   left.measurementStartedAt.microseconds () ==
                       right.measurementStartedAt.microseconds () &&
                   left.measurementLatency.microseconds () ==
                       right.measurementLatency.microseconds () &&
                   equalRangeReading (left.reading, right.reading) &&
                   left.status == right.status;
        }

        bool equalInput (const PresenceInput& left, const PresenceInput& right) noexcept
        {
            return left.observedAt == right.observedAt &&
                   left.pir.present == right.pir.present &&
                   (!left.pir.present ||
                    equalPirObservation (left.pir.value, right.pir.value)) &&
                   left.beam.present == right.beam.present &&
                   (!left.beam.present ||
                    equalBeam (left.beam.value, right.beam.value)) &&
                   left.finishGuard.present == right.finishGuard.present &&
                   (!left.finishGuard.present ||
                    equalReflective (left.finishGuard.value,
                                     right.finishGuard.value)) &&
                   left.range.present == right.range.present &&
                   (!left.range.present ||
                    equalRange (left.range.value, right.range.value));
        }

        PirObservation emptyPir (Status status) noexcept
        {
            return {0,     TimePoint (), Level::Low,  PirPhase::Warming,
                    false, false,        Duration (), status};
        }

        TimedRangeEvidence emptyRange () noexcept
        {
            return {0,
                    TimePoint                                 (),
                    TimePoint                                 (),
                    MicrosecondTimePoint                      (),
                    MicrosecondDuration                       (),
                    {RangeState::Idle, 0, MicrosecondDuration (), false},
                    StatusCode::Ok};
        }

        PresenceInput emptyInput () noexcept
        {
            const BeamObservation       beam       = {{0, 0, TimePoint ()},
                                                      Level::Low,
                                                      false,
                                                      false,
                                                      false,
                                                      Duration (),
                                                      OpticalQuality::Unqualified,
                                                      StatusCode::Ok};
            const ReflectiveObservation reflective = {{0, 0, TimePoint ()},
                                                      0,
                                                      0,
                                                      0,
                                                      0,
                                                      false,
                                                      false,
                                                      false,
                                                      Duration (),
                                                      OpticalQuality::Unqualified,
                                                      StatusCode::Ok};
            return {TimePoint (),
                    {false, emptyPir (StatusCode::Ok)},
                    {false, beam},
                    {false, reflective},
                    {false, emptyRange ()}};
        }

        PresenceSnapshot emptySnapshot (Status status) noexcept
        {
            const OpticalPresenceState optical = {false,
                                                  {0, 0, TimePoint ()},
                                                  OpticalQuality::Unqualified,
                                                  Duration (),
                                                  false,
                                                  false,
                                                  false,
                                                  false,
                                                  false,
                                                  StatusCode::Ok};
            return {{false, emptyPir (StatusCode::Ok), Duration (), false, false},
                    optical,
                    optical,
                    {false, emptyRange (), Duration (), false, false, false},
                    false,
                    false,
                    false,
                    Duration (),
                    PresenceQuality::Unqualified,
                    status};
        }

        bool canonicalInputAbsences (const PresenceInput& input) noexcept
        {
            const PresenceInput empty = emptyInput ();
            return                                 (input.pir.present ||
                    equalPirObservation (input.pir.value, empty.pir.value)) &&
                   (input.beam.present ||
                    equalBeam (input.beam.value, empty.beam.value)) &&
                   (input.finishGuard.present ||
                    equalReflective (input.finishGuard.value,
                                     empty.finishGuard.value)) &&
                   (input.range.present ||
                    equalRange (input.range.value, empty.range.value));
        }

        bool validPirPhase (PirPhase phase) noexcept
        {
            return phase == PirPhase::Warming || phase == PirPhase::ReadyClear ||
                   phase == PirPhase::Motion || phase == PirPhase::StuckMotion ||
                   phase == PirPhase::Fault;
        }

        bool validOpticalQuality (OpticalQuality quality) noexcept
        {
            return quality == OpticalQuality::Unqualified ||
                   quality == OpticalQuality::Valid ||
                   quality == OpticalQuality::BelowQualifiedRange ||
                   quality == OpticalQuality::AboveQualifiedRange ||
                   quality == OpticalQuality::DegenerateCalibration ||
                   quality == OpticalQuality::SourceFault ||
                   quality == OpticalQuality::TimingFault;
        }

        bool validRangeState (RangeState state) noexcept
        {
            return state == RangeState::Idle || state == RangeState::AwaitingEcho ||
                   state == RangeState::Measuring || state == RangeState::Valid ||
                   state == RangeState::Timeout || state == RangeState::OutOfRange;
        }

        uint8_t bitCount (uint8_t bits) noexcept
        {
            uint8_t count = 0;
            while (bits != 0)
            {
                count += static_cast<uint8_t> (bits & 1U);
                bits = static_cast<uint8_t> (bits >> 1U);
            }
            return count;
        }
    } // namespace

    PirObservationPolicy::PirObservationPolicy (
        const PirObservationConfig& config) noexcept
        : config_ (config), observation_ (emptyPir (StatusCode::NotInitialized)),
          lastSample_    ({0, TimePoint (), Level::Low, StatusCode::Ok}),
          warmupStarted_ (), candidateStarted_ (), stableStarted_ (), state_ (0)
    {
    }

    bool PirObservationPolicy::validConfig () const noexcept
    {
        return validLevel (config_.motionLevel) && validDuration (config_.warmup) &&
               validDuration                      (config_.qualifyMotion) &&
               validDuration                      (config_.qualifyClear) &&
               validDuration                      (config_.stuckMotion) &&
               config_.warmup.milliseconds        () > 0 &&
               config_.qualifyMotion.milliseconds () > 0 &&
               config_.qualifyClear.milliseconds  () > 0 &&
               config_.stuckMotion >= config_.qualifyMotion;
    }

    Status PirObservationPolicy::initialize () noexcept
    {
        if (initialized ())
        {
            return StatusCode::Ok;
        }
        if (!validConfig ())
        {
            observation_ = emptyPir (StatusCode::InvalidConfiguration);
            return observation_.status;
        }
        setFlag (state_, initializedFlag, true);
        reset   ();
        return StatusCode::Ok;
    }

    void PirObservationPolicy::reset () noexcept
    {
        state_ = initialized () ? initializedFlag : 0;
        observation_ =
            emptyPir (initialized () ? StatusCode::Ok : StatusCode::NotInitialized);
        observation_.sourceId = config_.sourceId;
    }

    Status PirObservationPolicy::update (const PirSample& sample) noexcept
    {
        if (!initialized ())
        {
            observation_.status = StatusCode::NotInitialized;
            return observation_.status;
        }
        if (observation_.phase == PirPhase::Fault)
        {
            return observation_.status;
        }

        if (hasFlag (state_, hasUpdateFlag) &&
            sample.observedAt == lastSample_.observedAt)
        {
            if (equalPirSample (sample, lastSample_))
            {
                return observation_.status;
            }
            observation_.phase       = PirPhase::Fault;
            observation_.motionEvent = false;
            observation_.clearEvent  = false;
            observation_.status      = StatusCode::InvalidArgument;
            return observation_.status;
        }

        if (sample.sourceId != config_.sourceId || !validLevel (sample.rawLevel) ||
            (hasFlag (state_, hasUpdateFlag) &&
             !validTime (sample.observedAt, lastSample_.observedAt)))
        {
            observation_.phase       = PirPhase::Fault;
            observation_.motionEvent = false;
            observation_.clearEvent  = false;
            observation_.status      = StatusCode::InvalidArgument;
            return observation_.status;
        }

        if (!sample.status.ok ())
        {
            observation_ = {sample.sourceId, sample.observedAt,
                            sample.rawLevel, PirPhase::Fault,
                            false,           false,
                            Duration (),     sample.status};
            lastSample_  = sample;
            setFlag (state_, hasUpdateFlag, true);
            return observation_.status;
        }

        observation_.sourceId    = sample.sourceId;
        observation_.observedAt  = sample.observedAt;
        observation_.rawLevel    = sample.rawLevel;
        observation_.motionEvent = false;
        observation_.clearEvent  = false;
        observation_.status      = StatusCode::Ok;

        if (!hasFlag (state_, hasUpdateFlag))
        {
            warmupStarted_ = sample.observedAt;
            stableStarted_ = sample.observedAt;
        }

        if (observation_.phase == PirPhase::Warming &&
            sample.observedAt.elapsedSince (warmupStarted_) < config_.warmup)
        {
            observation_.stableFor = sample.observedAt.elapsedSince (warmupStarted_);
            lastSample_            = sample;
            setFlag (state_, hasUpdateFlag, true);
            return StatusCode::Ok;
        }

        const bool motion        = sample.rawLevel == config_.motionLevel;
        const bool currentMotion = observation_.phase == PirPhase::Motion ||
                                   observation_.phase == PirPhase::StuckMotion;
        if (observation_.phase == PirPhase::Warming)
        {
            if (!hasFlag (state_, candidateFlag) ||
                hasFlag (state_, candidateValueFlag) != motion)
            {
                setFlag (state_, candidateFlag, true);
                setFlag (state_, candidateValueFlag, motion);
                candidateStarted_ = sample.observedAt;
            }
        }
        else if (motion == currentMotion)
        {
            setFlag (state_, candidateFlag, false);
        }
        else if (!hasFlag (state_, candidateFlag) ||
                 hasFlag (state_, candidateValueFlag) != motion)
        {
            setFlag (state_, candidateFlag, true);
            setFlag (state_, candidateValueFlag, motion);
            candidateStarted_ = sample.observedAt;
        }

        if (hasFlag (state_, candidateFlag))
        {
            const bool     candidateMotion = hasFlag (state_, candidateValueFlag);
            const Duration required =
                candidateMotion ? config_.qualifyMotion : config_.qualifyClear;
            if (sample.observedAt.elapsedSince (candidateStarted_) >= required)
            {
                observation_.phase =
                    candidateMotion ? PirPhase::Motion : PirPhase::ReadyClear;
                observation_.motionEvent = candidateMotion;
                observation_.clearEvent  = !candidateMotion;
                stableStarted_           = sample.observedAt;
                setFlag (state_, candidateFlag, false);
            }
        }

        observation_.stableFor = sample.observedAt.elapsedSince (stableStarted_);
        if (observation_.phase == PirPhase::Motion &&
            observation_.stableFor >= config_.stuckMotion)
        {
            observation_.phase = PirPhase::StuckMotion;
        }

        lastSample_ = sample;
        setFlag (state_, hasUpdateFlag, true);
        return StatusCode::Ok;
    }

    PirObservation PirObservationPolicy::snapshot () const noexcept
    {
        return observation_;
    }

    bool PirObservationPolicy::initialized () const noexcept
    {
        return hasFlag (state_, initializedFlag);
    }

    PresenceModel::PresenceModel (const PresenceModelConfig& config) noexcept
        : config_ (config), evidence_ (emptyInput ()), disagreementStarted_ (),
          beamInterruptedAt_ (), state_ (0)
    {
    }

    bool PresenceModel::validConfig () const noexcept
    {
        return (config_.requiredSources & static_cast<uint8_t> (~allSources)) == 0 &&
               (config_.agreementSources &
                static_cast<uint8_t> (~config_.requiredSources)) == 0 &&
               (config_.agreementSources == 0 ||
                bitCount (config_.agreementSources) >= 2) &&
               (config_.agreementSources == 0 ||
                config_.agreementWindow.milliseconds () > 0) &&
               validDuration                          (config_.pirMaximumAge) &&
               validDuration                          (config_.beamMaximumAge) &&
               validDuration                          (config_.finishGuardMaximumAge) &&
               validDuration                          (config_.rangeMaximumAge) &&
               validDuration                          (config_.beamPassageWindow) &&
               validDuration                          (config_.agreementWindow) &&
               config_.beamPassageWindow.milliseconds () > 0 &&
               config_.approachMinimumMm > 0 &&
               config_.approachMinimumMm <= config_.approachMaximumMm;
    }

    Status PresenceModel::initialize () noexcept
    {
        if (initialized ())
        {
            return StatusCode::Ok;
        }
        if (!validConfig ())
        {
            return StatusCode::InvalidConfiguration;
        }
        state_ = initializedFlag;
        reset ();
        return StatusCode::Ok;
    }

    void PresenceModel::reset () noexcept
    {
        state_    = initialized () ? initializedFlag : 0;
        evidence_ = emptyInput  ();
    }

    Status PresenceModel::update (const PresenceInput& input) noexcept
    {
        if (!initialized ())
        {
            return StatusCode::NotInitialized;
        }

        if (hasFlag (state_, hasUpdateFlag) && input.observedAt == evidence_.observedAt)
        {
            if (equalInput (input, evidence_))
            {
                return snapshot ().status;
            }
            setFlag (state_, timingFaultFlag, true);
            setFlag (state_, passageEventFlag, false);
            return StatusCode::InvalidArgument;
        }
        if (hasFlag (state_, hasUpdateFlag) &&
            !validTime (input.observedAt, evidence_.observedAt))
        {
            setFlag (state_, timingFaultFlag, true);
            setFlag (state_, passageEventFlag, false);
            return StatusCode::InvalidArgument;
        }

        bool malformed          = !canonicalInputAbsences (input);
        bool suppressBeamEvent  = false;
        bool suppressGuardEvent = false;
        if (input.pir.present)
        {
            malformed = malformed || !validPirPhase (input.pir.value.phase) ||
                        !validLevel (input.pir.value.rawLevel) ||
                        !validTime  (input.observedAt, input.pir.value.observedAt);
        }
        if (input.beam.present)
        {
            malformed =
                malformed || !validLevel (input.beam.value.rawLevel) ||
                !validOpticalQuality     (input.beam.value.quality) ||
                !validTime               (input.observedAt, input.beam.value.provenance.observedAt);
            const PresenceInput empty = emptyInput ();
            if (hasFlag                            (state_, hasUpdateFlag) &&
                !equalBeam (evidence_.beam.value, empty.beam.value) &&
                input.beam.value.provenance.sourceId ==
                    evidence_.beam.value.provenance.sourceId &&
                input.beam.value.provenance.calibrationRevision ==
                    evidence_.beam.value.provenance.calibrationRevision)
            {
                const bool sameEpoch =
                    input.beam.value.provenance.observedAt ==
                    evidence_.beam.value.provenance.observedAt;
                malformed =
                    malformed ||
                    (!sameEpoch &&
                     !validTime (input.beam.value.provenance.observedAt,
                                 evidence_.beam.value.provenance.observedAt)) ||
                    (sameEpoch &&
                     !equalBeam (input.beam.value, evidence_.beam.value));
                suppressBeamEvent =
                    sameEpoch && equalBeam (input.beam.value, evidence_.beam.value) &&
                    (input.beam.value.interruptionEvent ||
                     input.beam.value.restorationEvent);
            }
        }
        if (input.finishGuard.present)
        {
            malformed = malformed ||
                        !validOpticalQuality (input.finishGuard.value.quality) ||
                        !validTime           (input.observedAt,
                                    input.finishGuard.value.provenance.observedAt);
            const PresenceInput empty = emptyInput ();
            if (hasFlag                            (state_, hasUpdateFlag) &&
                !equalReflective (evidence_.finishGuard.value,
                                  empty.finishGuard.value) &&
                input.finishGuard.value.provenance.sourceId ==
                    evidence_.finishGuard.value.provenance.sourceId &&
                input.finishGuard.value.provenance.calibrationRevision ==
                    evidence_.finishGuard.value.provenance.calibrationRevision)
            {
                const bool sameEpoch =
                    input.finishGuard.value.provenance.observedAt ==
                    evidence_.finishGuard.value.provenance.observedAt;
                malformed =
                    malformed ||
                    (!sameEpoch &&
                     !validTime (
                         input.finishGuard.value.provenance.observedAt,
                         evidence_.finishGuard.value.provenance.observedAt)) ||
                    (sameEpoch &&
                     !equalReflective (input.finishGuard.value,
                                       evidence_.finishGuard.value));
                suppressGuardEvent =
                    sameEpoch &&
                    equalReflective (input.finishGuard.value,
                                     evidence_.finishGuard.value) &&
                    (input.finishGuard.value.activationEvent ||
                     input.finishGuard.value.deactivationEvent);
            }
        }
        if (input.range.present)
        {
            const TimedRangeEvidence& range = input.range.value;
            const uint32_t latency          = range.measurementLatency.microseconds ();
            const uint32_t echo = range.reading.echoDuration.microseconds           ();
            bool rangeShape     = validRangeState                                   (range.reading.state) &&
                                  validTime (input.observedAt, range.completedAt) &&
                                  validTime (range.completedAt, range.startedAt) &&
                                  latency < halfRange;
            if (!range.status.ok ())
            {
                rangeShape = rangeShape && range.reading.state == RangeState::Idle &&
                             range.reading.distanceMm == 0 && echo == 0 &&
                             latency == 0 && !range.reading.valid &&
                             range.startedAt == range.completedAt &&
                             range.measurementStartedAt.microseconds () == 0;
            }
            else if (range.reading.state == RangeState::Valid)
            {
                rangeShape = rangeShape && range.reading.valid &&
                             range.reading.distanceMm > 0 && echo > 0 &&
                             echo <= latency;
            }
            else if (range.reading.state == RangeState::Timeout)
            {
                rangeShape = rangeShape && !range.reading.valid &&
                             range.reading.distanceMm == 0 && echo == 0 && latency > 0;
            }
            else if (range.reading.state == RangeState::OutOfRange)
            {
                rangeShape =
                    rangeShape && !range.reading.valid && echo <= latency &&
                    (latency > 0 || (range.reading.distanceMm == 0 && echo == 0));
            }
            else
            {
                rangeShape = false;
            }
            malformed = malformed || !rangeShape;
        }
        if (malformed)
        {
            setFlag (state_, timingFaultFlag, true);
            setFlag (state_, passageEventFlag, false);
            return StatusCode::InvalidArgument;
        }

        evidence_.observedAt          = input.observedAt;
        evidence_.pir.present         = input.pir.present;
        evidence_.beam.present        = input.beam.present;
        evidence_.finishGuard.present = input.finishGuard.present;
        evidence_.range.present       = input.range.present;
        if (input.pir.present)
        {
            evidence_.pir.value = input.pir.value;
        }
        if (input.beam.present)
        {
            evidence_.beam.value = input.beam.value;
        }
        if (input.finishGuard.present)
        {
            evidence_.finishGuard.value = input.finishGuard.value;
        }
        if (input.range.present)
        {
            evidence_.range.value = input.range.value;
        }
        setFlag (state_, hasUpdateFlag, true);
        setFlag (state_, timingFaultFlag, false);
        setFlag (state_, passageEventFlag, false);
        setFlag (state_, suppressBeamEventFlag, suppressBeamEvent);
        setFlag (state_, suppressGuardEventFlag, suppressGuardEvent);

        PresenceSnapshot next     = makeSnapshot ();
        const bool healthy = next.quality == PresenceQuality::Valid ||
                             next.quality == PresenceQuality::Disagreement;
        bool             mismatch = false;
        if (healthy && config_.agreementSources != 0)
        {
            const bool claims[4] = {
                next.pir.available &&
                    (next.pir.evidence.phase == PirPhase::Motion ||
                     next.pir.evidence.phase == PirPhase::StuckMotion),
                next.beam.active, next.finishGuard.active, next.range.approachValid};
            bool firstSet = false;
            bool first    = false;
            for (uint8_t index = 0; index < 4; ++index)
            {
                if ((config_.agreementSources & static_cast<uint8_t> (1U << index)) ==
                    0)
                {
                    continue;
                }
                if (!firstSet)
                {
                    firstSet = true;
                    first    = claims[index];
                }
                else if (first != claims[index])
                {
                    mismatch = true;
                }
            }
        }

        if (mismatch)
        {
            if (!hasFlag (state_, disagreementFlag))
            {
                disagreementStarted_ = input.observedAt;
                setFlag (state_, disagreementFlag, true);
            }
        }
        else
        {
            setFlag (state_, disagreementFlag, false);
        }

        next = makeSnapshot ();
        if (next.quality == PresenceQuality::Valid && !next.disagreement &&
            next.beam.available && next.beam.valid && !next.beam.stale)
        {
            if (next.beam.activationEvent)
            {
                beamInterruptedAt_ = next.beam.provenance.observedAt;
                setFlag (state_, beamPassageCandidateFlag, true);
            }
            else if (next.beam.deactivationEvent &&
                     hasFlag (state_, beamPassageCandidateFlag))
            {
                const Duration elapsed =
                    next.beam.provenance.observedAt.elapsedSince (beamInterruptedAt_);
                setFlag (state_, passageEventFlag,
                         elapsed.milliseconds () > 0 &&
                             elapsed <= config_.beamPassageWindow);
                setFlag (state_, beamPassageCandidateFlag, false);
            }
        }
        else
        {
            setFlag (state_, beamPassageCandidateFlag, false);
        }
        return makeSnapshot ().status;
    }

    PresenceSnapshot PresenceModel::makeSnapshot () const noexcept
    {
        if (!initialized ())
        {
            return emptySnapshot (validConfig () ? StatusCode::NotInitialized
                                                 : StatusCode::InvalidConfiguration);
        }
        if (hasFlag (state_, timingFaultFlag) &&
            !hasFlag (state_, hasUpdateFlag))
        {
            PresenceSnapshot fault = emptySnapshot (StatusCode::InvalidArgument);
            fault.quality          = PresenceQuality::TimingFault;
            return fault;
        }
        if (!hasFlag (state_, hasUpdateFlag))
        {
            return emptySnapshot (StatusCode::Ok);
        }

        PresenceSnapshot result = emptySnapshot (StatusCode::Ok);
        if (evidence_.pir.present)
        {
            result.pir.available = true;
            result.pir.evidence  = evidence_.pir.value;
            result.pir.age =
                evidence_.observedAt.elapsedSince (evidence_.pir.value.observedAt);
            result.pir.valid = evidence_.pir.value.status.ok () &&
                               (evidence_.pir.value.phase == PirPhase::ReadyClear ||
                                evidence_.pir.value.phase == PirPhase::Motion ||
                                evidence_.pir.value.phase == PirPhase::StuckMotion);
            result.pir.stale =
                result.pir.valid && result.pir.age >= config_.pirMaximumAge;
            result.pirEligible = result.pir.valid && !result.pir.stale &&
                                 evidence_.pir.value.phase == PirPhase::Motion;
        }
        if (evidence_.beam.present)
        {
            const BeamObservation& value = evidence_.beam.value;
            result.beam                  = {
                true,
                value.provenance,
                value.quality,
                evidence_.observedAt.elapsedSince (value.provenance.observedAt),
                value.status.ok                   () && value.quality == OpticalQuality::Valid,
                false,
                value.interrupted,
                value.interruptionEvent &&
                    !hasFlag (state_, suppressBeamEventFlag),
                value.restorationEvent &&
                    !hasFlag (state_, suppressBeamEventFlag),
                value.status};
            result.beam.stale =
                result.beam.valid && result.beam.age >= config_.beamMaximumAge;
        }
        if (evidence_.finishGuard.present)
        {
            const ReflectiveObservation& value = evidence_.finishGuard.value;
            result.finishGuard                 = {
                true,
                value.provenance,
                value.quality,
                evidence_.observedAt.elapsedSince (value.provenance.observedAt),
                value.status.ok                   () && value.quality == OpticalQuality::Valid,
                false,
                value.markerActive,
                value.activationEvent &&
                    !hasFlag (state_, suppressGuardEventFlag),
                value.deactivationEvent &&
                    !hasFlag (state_, suppressGuardEventFlag),
                value.status};
            result.finishGuard.stale =
                result.finishGuard.valid &&
                result.finishGuard.age >= config_.finishGuardMaximumAge;
        }
        if (evidence_.range.present)
        {
            result.range.available = true;
            result.range.evidence  = evidence_.range.value;
            result.range.age =
                evidence_.observedAt.elapsedSince (evidence_.range.value.completedAt);
            result.range.valid =
                evidence_.range.value.status.ok () &&
                evidence_.range.value.reading.state == RangeState::Valid &&
                evidence_.range.value.reading.valid;
            result.range.stale =
                result.range.valid && result.range.age >= config_.rangeMaximumAge;
            result.range.approachValid =
                result.range.valid && !result.range.stale &&
                evidence_.range.value.reading.state == RangeState::Valid &&
                evidence_.range.value.reading.valid &&
                evidence_.range.value.reading.distanceMm >= config_.approachMinimumMm &&
                evidence_.range.value.reading.distanceMm <= config_.approachMaximumMm;
        }

        const uint8_t present = static_cast<uint8_t> (
            (evidence_.pir.present ? 1U : 0U) | (evidence_.beam.present ? 2U : 0U) |
            (evidence_.finishGuard.present ? 4U : 0U) |
            (evidence_.range.present ? 8U : 0U));
        const bool missing =
            (present & config_.requiredSources) != config_.requiredSources;
        Status sourceStatus = StatusCode::Ok;
        if (evidence_.pir.present && !evidence_.pir.value.status.ok ())
        {
            sourceStatus = evidence_.pir.value.status;
        }
        else if (evidence_.beam.present && !evidence_.beam.value.status.ok ())
        {
            sourceStatus = evidence_.beam.value.status;
        }
        else if (evidence_.finishGuard.present &&
                 !evidence_.finishGuard.value.status.ok ())
        {
            sourceStatus = evidence_.finishGuard.value.status;
        }
        else if (evidence_.range.present && !evidence_.range.value.status.ok ())
        {
            sourceStatus = evidence_.range.value.status;
        }
        const bool invalid =
            ((config_.requiredSources & 1U) != 0 && result.pir.available &&
             !result.pir.valid) ||
            ((config_.requiredSources & 2U) != 0 && result.beam.available &&
             !result.beam.valid) ||
            ((config_.requiredSources & 4U) != 0 &&
             result.finishGuard.available && !result.finishGuard.valid);
        const bool stale =
            ((config_.requiredSources & 1U) != 0 && result.pir.stale) ||
            ((config_.requiredSources & 2U) != 0 && result.beam.stale) ||
            ((config_.requiredSources & 4U) != 0 && result.finishGuard.stale) ||
            ((config_.requiredSources & 8U) != 0 && result.range.stale);
        const bool rangeUnqualified =
            (config_.requiredSources & 8U) != 0 && result.range.available &&
            !result.range.valid && evidence_.range.value.status.ok ();

        if (hasFlag (state_, timingFaultFlag))
        {
            result.quality = PresenceQuality::TimingFault;
            result.status  = StatusCode::InvalidArgument;
        }
        else if (!sourceStatus.ok () || invalid)
        {
            result.quality = PresenceQuality::SourceFault;
            result.status =
                sourceStatus.ok () ? StatusCode::InvalidArgument : sourceStatus;
        }
        else if (stale)
        {
            result.quality = PresenceQuality::Stale;
        }
        else if (missing || rangeUnqualified)
        {
            result.quality = PresenceQuality::Unqualified;
        }
        else if (hasFlag (state_, disagreementFlag))
        {
            result.disagreementFor =
                evidence_.observedAt.elapsedSince (disagreementStarted_);
            result.disagreement = true;
            result.quality =
                result.disagreementFor >= config_.agreementWindow
                    ? PresenceQuality::Disagreement
                    : PresenceQuality::Valid;
        }
        else
        {
            result.quality = PresenceQuality::Valid;
        }
        result.passageEvent = hasFlag (state_, passageEventFlag);
        return result;
    }

    PresenceSnapshot PresenceModel::snapshot () const noexcept
    {
        return makeSnapshot ();
    }

    bool PresenceModel::initialized () const noexcept
    {
        return hasFlag (state_, initializedFlag);
    }
} // namespace adk
