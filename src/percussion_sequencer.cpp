#include "percussion_sequencer.h"

#include <limits.h>

namespace adk {

    static constexpr uint32_t maximumDuration = 0x7fffffffu;

    static bool statusDefined (Status status) noexcept
    {
        return status.error () >= StatusCode::Ok &&
               status.error () <= StatusCode::HardwareFailure;
    }

    PercussionSequencerConfig::PercussionSequencerConfig (
        uint8_t stepCount, uint16_t minimumTempo, uint16_t maximumTempo,
        Duration simultaneous, Duration associationTimeout) noexcept
        : steps (stepCount), minimumTempoBpm (minimumTempo),
          maximumTempoBpm            (maximumTempo), simultaneousWindow (simultaneous),
          acousticAssociationTimeout (associationTimeout)
    {
    }

    PercussionAcousticCompletion::PercussionAcousticCompletion () noexcept
        : present (false), eventStartedAt (TimePoint (0)), eventDuration (Duration (0)),
          intensity (0)
    {
    }

    PercussionSequencerInput::PercussionSequencerInput () noexcept
        : observedAt (TimePoint (0)), attackMask (0),
          surfaceStatus{StatusCode::Ok, StatusCode::Ok, StatusCode::Ok, StatusCode::Ok},
          acousticStatus (StatusCode::Ok), acousticCompletion (), tempoPosition (0),
          playEvent      (false), clearEvent (false)
    {
    }

    PercussionSequencer::PercussionSequencer (
        const PercussionSequencerConfig& config) noexcept
        : config_ (config), snapshot_{PercussionMode::Recording,
                                      0,
                                      0,
                                      0,
                                      0,
                                      false,
                                      false,
                                      false,
                                      false,
                                      PercussionFaultSource::None,
                                      PercussionAssociation::None,
                                      {0, 0, 0, 0, PercussionAssociation::None},
                                      {0, 0, {0, 0, 0, 0}, 0, Duration (0), false},
                                      StatusCode::NotInitialized},
          hits_{}, timeoutAssociations_ (0), lastInput_{}, lastUpdate_ (TimePoint (0)),
          recordingEpoch_               (TimePoint (0)), pendingFirstAttack_ (TimePoint (0)),
          playbackDeadline_             (TimePoint (0)), recordingTempoBpm_ (0),
          playbackStepCount_            (0), pendingSurfaces_{false, false, false, false},
          initialized_                  (false), hasLastUpdate_ (false), hasRecordingEpoch_ (false),
          pending_                      (false), pendingClosed_ (false)
    {
    }

    Status PercussionSequencer::initialize () noexcept
    {
        if (initialized_ && snapshot_.mode != PercussionMode::Fault)
        {
            return StatusCode::Ok;
        }

        if (!configValid ())
        {
            initialized_          = false;
            hasLastUpdate_        = false;
            snapshot_.mode        = PercussionMode::Fault;
            snapshot_.faultSource = PercussionFaultSource::Input;
            snapshot_.status      = StatusCode::InvalidConfiguration;
            return snapshot_.status;
        }

        initialized_       = true;
        hasLastUpdate_     = false;
        hasRecordingEpoch_ = false;
        pending_           = false;
        pendingClosed_     = false;
        recordingTempoBpm_ = 0;
        playbackStepCount_ = 0;
        snapshot_.mode = snapshot_.hitCount == maximumHits ? PercussionMode::Full
                                                           : PercussionMode::Recording;
        snapshot_.tempoBpm        = config_.minimumTempoBpm;
        snapshot_.currentStep     = 0;
        snapshot_.hitAccepted     = false;
        snapshot_.hitSuppressed   = false;
        snapshot_.patternFull     = snapshot_.hitCount == maximumHits;
        snapshot_.frameValid      = false;
        snapshot_.faultSource     = PercussionFaultSource::None;
        snapshot_.lastAssociation = PercussionAssociation::None;
        snapshot_.frame           = {0, 0, {0, 0, 0, 0}, 0, Duration (0), false};
        snapshot_.status          = StatusCode::Ok;

        for (uint8_t surface = 0; surface < 4; ++surface)
        {
            pendingSurfaces_[surface] = false;
        }

        return snapshot_.status;
    }

    void PercussionSequencer::shutdown () noexcept
    {
        initialized_              = false;
        hasLastUpdate_            = false;
        hasRecordingEpoch_        = false;
        pending_                  = false;
        pendingClosed_            = false;
        recordingTempoBpm_        = 0;
        playbackStepCount_        = 0;
        snapshot_.mode            = PercussionMode::Recording;
        snapshot_.tempoBpm        = 0;
        snapshot_.currentStep     = 0;
        snapshot_.hitAccepted     = false;
        snapshot_.hitSuppressed   = false;
        snapshot_.patternFull     = snapshot_.hitCount == maximumHits;
        snapshot_.frameValid      = false;
        snapshot_.faultSource     = PercussionFaultSource::None;
        snapshot_.lastAssociation = PercussionAssociation::None;
        snapshot_.frame           = {0, 0, {0, 0, 0, 0}, 0, Duration (0), false};
        snapshot_.status          = StatusCode::NotInitialized;

        for (uint8_t surface = 0; surface < 4; ++surface)
        {
            pendingSurfaces_[surface] = false;
        }
    }

    Status PercussionSequencer::update (const PercussionSequencerInput& input) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (hasLastUpdate_)
        {
            const uint32_t elapsed =
                input.observedAt.elapsedSince (lastUpdate_).milliseconds ();

            if (input.observedAt == lastUpdate_)
            {
                if (inputEqual (input, lastInput_))
                {
                    return snapshot_.status;
                }

                enterFault (StatusCode::InvalidArgument, PercussionFaultSource::Timing);
                return snapshot_.status;
            }

            if (elapsed >= UINT32_C (0x80000000))
            {
                enterFault (StatusCode::InvalidArgument, PercussionFaultSource::Timing);
                return snapshot_.status;
            }
        }

        Status                failure;
        PercussionFaultSource faultSource;

        if (!inputEvidenceValid (input, failure, faultSource))
        {
            enterFault (failure, faultSource);
            return snapshot_.status;
        }

        if (input.tempoPosition > 1000)
        {
            enterFault (StatusCode::InvalidArgument, PercussionFaultSource::Tempo);
            return snapshot_.status;
        }

        clearTransient ();
        lastInput_     = input;
        lastUpdate_    = input.observedAt;
        hasLastUpdate_ = true;

        if (snapshot_.mode == PercussionMode::Fault)
        {
            return snapshot_.status;
        }

        const uint16_t tempoBpm = mapTempo (input.tempoPosition);

        if (snapshot_.mode != PercussionMode::Playing)
        {
            snapshot_.tempoBpm = tempoBpm;
        }

        if (input.clearEvent)
        {
            clearPattern ();
            snapshot_.tempoBpm = tempoBpm;
            return snapshot_.status;
        }

        if (pending_ && input.observedAt.elapsedSince (pendingFirstAttack_) >=
                            config_.acousticAssociationTimeout)
        {
            finalizePending (0, PercussionAssociation::AssociationTimeout);
        }
        else if (pending_ && pendingClosed_ && input.acousticCompletion.present &&
                 acousticContainsAttack (input.acousticCompletion))
        {
            finalizePending (input.acousticCompletion.intensity,
                             PercussionAssociation::AcousticCompletion);
        }

        if (input.playEvent)
        {
            togglePlayback (input.observedAt, tempoBpm);
        }

        if (snapshot_.mode == PercussionMode::Recording ||
            snapshot_.mode == PercussionMode::Full)
        {
            if (pending_ && !pendingClosed_ &&
                input.observedAt.elapsedSince (pendingFirstAttack_) >
                    config_.simultaneousWindow)
            {
                pendingClosed_ = true;
            }

            collectAttacks (input);

            if (pending_ && input.observedAt != pendingFirstAttack_ &&
                input.observedAt.elapsedSince (pendingFirstAttack_) >=
                    config_.simultaneousWindow)
            {
                pendingClosed_ = true;
            }
        }

        if (snapshot_.mode == PercussionMode::Playing)
        {
            applyPlayback (input.observedAt, tempoBpm);
        }

        return snapshot_.status;
    }

    void PercussionSequencer::clear () noexcept
    {
        if (initialized_ && snapshot_.mode == PercussionMode::Fault)
        {
            return;
        }

        clearPattern ();
    }

    bool PercussionSequencer::initialized () const noexcept
    {
        return initialized_;
    }

    PercussionSequencerSnapshot PercussionSequencer::snapshot () const noexcept
    {
        return snapshot_;
    }

    Result<PercussionHit> PercussionSequencer::hit (uint8_t index) const noexcept
    {
        if (index >= snapshot_.hitCount)
        {
            return {StatusCode::InvalidArgument,
                    {0, 0, 0, 0, PercussionAssociation::None}};
        }

        const StoredHit&            stored = hits_[index];
        const PercussionAssociation association =
            (timeoutAssociations_ & (UINT32_C (1) << index)) != 0
                ? PercussionAssociation::AssociationTimeout
                : PercussionAssociation::AcousticCompletion;

        return {StatusCode::Ok,
                {stored.surface, stored.step, stored.intensity, stored.ordinal,
                 association}};
    }

    bool PercussionSequencer::configValid () const noexcept
    {
        const uint32_t simultaneous = config_.simultaneousWindow.milliseconds        ();
        const uint32_t association = config_.acousticAssociationTimeout.milliseconds ();

        return config_.steps >= 4 && config_.steps <= maximumSteps &&
               config_.minimumTempoBpm >= 30 && config_.maximumTempoBpm <= 240 &&
               config_.minimumTempoBpm <= config_.maximumTempoBpm && simultaneous > 0 &&
               simultaneous <= maximumDuration && association > simultaneous &&
               association <= maximumDuration;
    }

    bool PercussionSequencer::inputEqual (
        const PercussionSequencerInput& left,
        const PercussionSequencerInput& right) const noexcept
    {
        if (left.observedAt != right.observedAt ||
            left.attackMask != right.attackMask ||
            left.tempoPosition != right.tempoPosition ||
            left.playEvent != right.playEvent || left.clearEvent != right.clearEvent ||
            left.acousticStatus != right.acousticStatus ||
            left.acousticCompletion.present != right.acousticCompletion.present ||
            left.acousticCompletion.eventStartedAt !=
                right.acousticCompletion.eventStartedAt ||
            left.acousticCompletion.eventDuration !=
                right.acousticCompletion.eventDuration ||
            left.acousticCompletion.intensity != right.acousticCompletion.intensity)
        {
            return false;
        }

        for (uint8_t surface = 0; surface < 4; ++surface)
        {
            if (left.surfaceStatus[surface] != right.surfaceStatus[surface])
            {
                return false;
            }
        }

        return true;
    }

    bool PercussionSequencer::inputEvidenceValid (
        const PercussionSequencerInput& input, Status& failure,
        PercussionFaultSource& source) const noexcept
    {
        if ((input.attackMask & UINT8_C (0xf0)) != 0)
        {
            failure = StatusCode::InvalidArgument;
            source  = PercussionFaultSource::Input;
            return false;
        }

        for (uint8_t surface = 0; surface < 4; ++surface)
        {
            if (!statusDefined (input.surfaceStatus[surface]))
            {
                failure = StatusCode::InternalInvariant;
                source  = static_cast<PercussionFaultSource> (
                    static_cast<uint8_t> (PercussionFaultSource::Surface0) + surface);
                return false;
            }

            if (!input.surfaceStatus[surface].ok ())
            {
                failure = input.surfaceStatus[surface];
                source  = static_cast<PercussionFaultSource> (
                    static_cast<uint8_t> (PercussionFaultSource::Surface0) + surface);
                return false;
            }
        }

        if (!statusDefined (input.acousticStatus))
        {
            failure = StatusCode::InternalInvariant;
            source  = PercussionFaultSource::Acoustic;
            return false;
        }

        if (!input.acousticStatus.ok ())
        {
            failure = input.acousticStatus;
            source  = PercussionFaultSource::Acoustic;
            return false;
        }

        if (input.acousticCompletion.present &&
            (input.acousticCompletion.eventDuration.milliseconds () == 0 ||
             input.acousticCompletion.eventDuration.milliseconds () > maximumDuration))
        {
            failure = StatusCode::InvalidArgument;
            source  = PercussionFaultSource::Acoustic;
            return false;
        }

        if (input.acousticCompletion.present)
        {
            const uint32_t completionAge =
                input.observedAt.elapsedSince (input.acousticCompletion.eventStartedAt)
                    .milliseconds ();

            if (completionAge > maximumDuration ||
                input.acousticCompletion.eventDuration.milliseconds () > completionAge)
            {
                failure = StatusCode::InvalidArgument;
                source  = PercussionFaultSource::Acoustic;
                return false;
            }
        }

        if (!input.acousticCompletion.present &&
            (input.acousticCompletion.eventStartedAt != TimePoint (0) ||
             input.acousticCompletion.eventDuration != Duration (0) ||
             input.acousticCompletion.intensity != 0))
        {
            failure = StatusCode::InvalidArgument;
            source  = PercussionFaultSource::Acoustic;
            return false;
        }

        source = PercussionFaultSource::None;
        return true;
    }

    bool PercussionSequencer::acousticContainsAttack (
        const PercussionAcousticCompletion& completion) const noexcept
    {
        return pendingFirstAttack_.elapsedSince (completion.eventStartedAt) <=
               completion.eventDuration;
    }

    uint16_t PercussionSequencer::mapTempo (uint16_t position) const noexcept
    {
        const uint32_t range =
            static_cast<uint32_t> (config_.maximumTempoBpm) - config_.minimumTempoBpm;

        return static_cast<uint16_t> (config_.minimumTempoBpm +
                                      (range * position) / 1000u);
    }

    uint32_t PercussionSequencer::stepMilliseconds (uint16_t tempoBpm) const noexcept
    {
        return 60000u / tempoBpm / 4u;
    }

    uint8_t PercussionSequencer::quantizedStep (TimePoint attackAt) const noexcept
    {
        const uint32_t step = stepMilliseconds (recordingTempoBpm_);
        const uint32_t elapsed =
            attackAt.elapsedSince (recordingEpoch_).milliseconds ();

        return static_cast<uint8_t> (((elapsed + step / 2u) / step) % config_.steps);
    }

    void PercussionSequencer::clearTransient () noexcept
    {
        snapshot_.hitAccepted     = false;
        snapshot_.hitSuppressed   = false;
        snapshot_.lastAssociation = PercussionAssociation::None;
    }

    void PercussionSequencer::clearPattern () noexcept
    {
        snapshot_.mode            = PercussionMode::Recording;
        snapshot_.currentStep     = 0;
        snapshot_.hitCount        = 0;
        snapshot_.nextOrdinal     = 0;
        snapshot_.hitAccepted     = false;
        snapshot_.hitSuppressed   = false;
        snapshot_.patternFull     = false;
        snapshot_.frameValid      = false;
        snapshot_.faultSource     = PercussionFaultSource::None;
        snapshot_.lastAssociation = PercussionAssociation::None;
        snapshot_.lastHit         = {0, 0, 0, 0, PercussionAssociation::None};
        snapshot_.frame           = {0, 0, {0, 0, 0, 0}, 0, Duration (0), false};
        hasRecordingEpoch_        = false;
        pending_                  = false;
        pendingClosed_            = false;
        recordingTempoBpm_        = 0;
        playbackStepCount_        = 0;
        timeoutAssociations_      = 0;

        if (initialized_)
        {
            snapshot_.status = StatusCode::Ok;
        }
    }

    void PercussionSequencer::beginPending (TimePoint attackAt) noexcept
    {
        if (!hasRecordingEpoch_)
        {
            recordingEpoch_    = attackAt;
            recordingTempoBpm_ = snapshot_.tempoBpm;
            hasRecordingEpoch_ = true;
        }

        pendingFirstAttack_ = attackAt;
        pending_            = true;
        pendingClosed_      = false;

        for (uint8_t surface = 0; surface < 4; ++surface)
        {
            pendingSurfaces_[surface] = false;
        }
    }

    void
    PercussionSequencer::collectAttacks (const PercussionSequencerInput& input) noexcept
    {
        bool hasAttack = false;

        for (uint8_t surface = 0; surface < 4; ++surface)
        {
            hasAttack =
                hasAttack ||
                (input.attackMask & static_cast<uint8_t> (UINT8_C (1) << surface)) != 0;
        }

        if (!hasAttack)
        {
            return;
        }

        if (snapshot_.mode == PercussionMode::Full || pendingClosed_)
        {
            snapshot_.hitSuppressed = true;
            return;
        }

        if (!pending_)
        {
            beginPending (input.observedAt);
        }

        for (uint8_t surface = 0; surface < 4; ++surface)
        {
            if ((input.attackMask & static_cast<uint8_t> (UINT8_C (1) << surface)) == 0)
            {
                continue;
            }

            if (pendingSurfaces_[surface])
            {
                snapshot_.hitSuppressed = true;
                continue;
            }

            pendingSurfaces_[surface] = true;
            snapshot_.hitAccepted     = true;
        }
    }

    void
    PercussionSequencer::finalizePending (uint16_t              intensity,
                                          PercussionAssociation association) noexcept
    {
        const uint8_t step                = quantizedStep (pendingFirstAttack_);
        uint8_t       acceptedSurfaces[4] = {0, 0, 0, 0};
        uint8_t       acceptedCount       = 0;

        for (uint8_t surface = 0; surface < 4; ++surface)
        {
            if (!pendingSurfaces_[surface])
            {
                continue;
            }

            bool duplicate = false;

            for (uint8_t index = 0; index < snapshot_.hitCount; ++index)
            {
                duplicate = duplicate || (hits_[index].surface == surface &&
                                          hits_[index].step == step);
            }

            if (duplicate)
            {
                snapshot_.hitSuppressed = true;
                continue;
            }

            acceptedSurfaces[acceptedCount] = surface;
            ++acceptedCount;
        }

        if (acceptedCount > static_cast<uint8_t> (maximumHits - snapshot_.hitCount))
        {
            snapshot_.hitSuppressed = true;
            pending_                = false;
            pendingClosed_          = false;
            return;
        }

        for (uint8_t accepted = 0; accepted < acceptedCount; ++accepted)
        {
            const uint8_t surface = acceptedSurfaces[accepted];

            StoredHit hit       = {surface, step, intensity, snapshot_.nextOrdinal};
            uint8_t   insertion = snapshot_.hitCount;

            while (insertion > 0)
            {
                const StoredHit& previous = hits_[insertion - 1];
                const bool       after =
                    previous.step > hit.step ||
                    (previous.step == hit.step && previous.surface > hit.surface) ||
                    (previous.step == hit.step && previous.surface == hit.surface &&
                     previous.ordinal > hit.ordinal);

                if (!after)
                {
                    break;
                }

                hits_[insertion] = previous;
                --insertion;
            }

            const uint32_t lowerMask =
                insertion == 0 ? 0 : (UINT32_C (1) << insertion) - 1u;

            timeoutAssociations_ = (timeoutAssociations_ & lowerMask) |
                                   ((timeoutAssociations_ & ~lowerMask) << 1u);
            if (association == PercussionAssociation::AssociationTimeout)
            {
                timeoutAssociations_ |= UINT32_C (1) << insertion;
            }

            hits_[insertion] = hit;
            ++snapshot_.hitCount;
            snapshot_.lastHit     = {hit.surface, hit.step, hit.intensity, hit.ordinal,
                                     association};
            snapshot_.hitAccepted = true;
            snapshot_.lastAssociation = association;

            if (snapshot_.nextOrdinal != UINT32_MAX)
            {
                ++snapshot_.nextOrdinal;
            }
        }

        pending_       = false;
        pendingClosed_ = false;

        if (snapshot_.hitCount == maximumHits)
        {
            snapshot_.mode        = PercussionMode::Full;
            snapshot_.patternFull = true;
        }
    }

    void PercussionSequencer::togglePlayback (TimePoint now, uint16_t tempoBpm) noexcept
    {
        if (snapshot_.mode == PercussionMode::Playing)
        {
            snapshot_.mode        = snapshot_.hitCount == maximumHits
                                        ? PercussionMode::Full
                                        : PercussionMode::Recording;
            snapshot_.frameValid  = false;
            snapshot_.currentStep = 0;
            playbackStepCount_    = 0;
            return;
        }

        if (snapshot_.hitCount == 0)
        {
            return;
        }

        snapshot_.mode        = PercussionMode::Playing;
        snapshot_.tempoBpm    = tempoBpm;
        snapshot_.currentStep = 0;
        playbackStepCount_    = 0;
        playbackDeadline_ =
            TimePoint (now.milliseconds () + stepMilliseconds (tempoBpm));
        publishFrame ();
    }

    void PercussionSequencer::applyPlayback (TimePoint now, uint16_t tempoBpm) noexcept
    {
        const uint32_t untilBoundary =
            now.elapsedSince (playbackDeadline_).milliseconds ();

        if (untilBoundary >= UINT32_C (0x80000000))
        {
            return;
        }

        const uint32_t duration = stepMilliseconds (tempoBpm);
        const uint32_t crossed  = 1u + untilBoundary / duration;

        const uint32_t playbackCycle = static_cast<uint32_t> (config_.steps) * 8u;

        playbackStepCount_ = (playbackStepCount_ + crossed) % playbackCycle;
        snapshot_.tempoBpm = tempoBpm;
        snapshot_.currentStep =
            static_cast<uint8_t> (playbackStepCount_ % config_.steps);
        playbackDeadline_ =
            TimePoint (playbackDeadline_.milliseconds () + crossed * duration);
        publishFrame ();
    }

    void PercussionSequencer::publishFrame () noexcept
    {
        snapshot_.frame = {snapshot_.currentStep, 0,
                           {0, 0, 0, 0},          0,
                           Duration (0),          (playbackStepCount_ / 4u) % 2u != 0};

        uint16_t chosenIntensity = 0;
        uint8_t  chosenSurface   = 0;
        bool     hasTone         = false;

        for (uint8_t index = 0; index < snapshot_.hitCount; ++index)
        {
            const StoredHit& hit = hits_[index];

            if (hit.step != snapshot_.currentStep)
            {
                continue;
            }

            snapshot_.frame.surfaceMask = static_cast<uint8_t> (
                snapshot_.frame.surfaceMask | static_cast<uint8_t> (1u << hit.surface));
            snapshot_.frame.intensity[hit.surface] = hit.intensity;

            if (!hasTone || hit.intensity > chosenIntensity ||
                (hit.intensity == chosenIntensity && hit.surface < chosenSurface))
            {
                chosenIntensity = hit.intensity;
                chosenSurface   = hit.surface;
                hasTone         = true;
            }
        }

        if (hasTone)
        {
            static constexpr uint16_t frequencies[4] = {262, 330, 392, 523};
            const uint32_t halfStep = stepMilliseconds (snapshot_.tempoBpm) / 2u;

            snapshot_.frame.frequencyHz  = frequencies[chosenSurface];
            snapshot_.frame.toneDuration = Duration (halfStep < 60u ? halfStep : 60u);
        }

        snapshot_.frameValid = true;
    }

    void PercussionSequencer::enterFault (Status                status,
                                          PercussionFaultSource source) noexcept
    {
        snapshot_.mode            = PercussionMode::Fault;
        snapshot_.status          = status;
        snapshot_.frameValid      = false;
        snapshot_.faultSource     = source;
        snapshot_.lastAssociation = PercussionAssociation::None;
        snapshot_.frame           = {snapshot_.currentStep, 0,    {0, 0, 0, 0}, 0,
                                     Duration (0),          false};
        snapshot_.hitAccepted     = false;
        snapshot_.hitSuppressed   = false;
        pending_                  = false;
        pendingClosed_            = false;
    }
} // namespace adk
