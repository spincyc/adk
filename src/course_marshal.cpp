#include "course_marshal.h"

#include <limits.h>

namespace adk {
    namespace {
        constexpr uint32_t halfRange = 0x80000000UL;

        bool validInterval (Duration value) noexcept
        {
            return value.milliseconds () != 0U && value.milliseconds () < halfRange;
        }

        bool elapsedIsValid (TimePoint later, TimePoint earlier) noexcept
        {
            return later.elapsedSince (earlier).milliseconds () < halfRange;
        }

        bool samePirState (const PirPresenceState& left,
                           const PirPresenceState& right) noexcept
        {
            return left.available == right.available &&
                   left.evidence.sourceId == right.evidence.sourceId &&
                   left.evidence.observedAt == right.evidence.observedAt &&
                   left.evidence.rawLevel == right.evidence.rawLevel &&
                   left.evidence.phase == right.evidence.phase &&
                   left.evidence.motionEvent == right.evidence.motionEvent &&
                   left.evidence.clearEvent == right.evidence.clearEvent &&
                   left.evidence.stableFor == right.evidence.stableFor &&
                   left.evidence.status == right.evidence.status &&
                   left.age == right.age && left.valid == right.valid &&
                   left.stale == right.stale;
        }

        bool sameStartInput (const CourseStartInput& left,
                             const CourseStartInput& right) noexcept
        {
            return left.observedAt == right.observedAt &&
                   left.buttonSourceId == right.buttonSourceId &&
                   left.buttonPressEvent == right.buttonPressEvent &&
                   samePirState (left.pir, right.pir);
        }

        bool sameStartEvent (const CourseStartEvent& left,
                             const CourseStartEvent& right) noexcept
        {
            return left.present == right.present && left.source == right.source &&
                   left.buttonSourceId == right.buttonSourceId &&
                   left.observedAt == right.observedAt &&
                   samePirState (left.pir, right.pir) && left.status == right.status;
        }

        bool canonicalAbsentStart (const CourseStartEvent& event) noexcept
        {
            return sameStartEvent (event, CourseStartEvent ());
        }

        bool sameOpticalState (const OpticalPresenceState& left,
                               const OpticalPresenceState& right) noexcept
        {
            return left.available == right.available &&
                   left.provenance.sourceId == right.provenance.sourceId &&
                   left.provenance.calibrationRevision ==
                       right.provenance.calibrationRevision &&
                   left.provenance.observedAt == right.provenance.observedAt &&
                   left.quality == right.quality && left.age == right.age &&
                   left.valid == right.valid && left.stale == right.stale &&
                   left.active == right.active &&
                   left.activationEvent == right.activationEvent &&
                   left.deactivationEvent == right.deactivationEvent &&
                   left.status == right.status;
        }

        bool sameRangeState (const RangePresenceState& left,
                             const RangePresenceState& right) noexcept
        {
            return left.available == right.available &&
                   left.evidence.sourceId == right.evidence.sourceId &&
                   left.evidence.startedAt == right.evidence.startedAt &&
                   left.evidence.completedAt == right.evidence.completedAt &&
                   left.evidence.measurementStartedAt.microseconds () ==
                       right.evidence.measurementStartedAt.microseconds () &&
                   left.evidence.measurementLatency.microseconds () ==
                       right.evidence.measurementLatency.microseconds () &&
                   left.evidence.reading.state == right.evidence.reading.state &&
                   left.evidence.reading.distanceMm ==
                       right.evidence.reading.distanceMm &&
                   left.evidence.reading.echoDuration.microseconds () ==
                       right.evidence.reading.echoDuration.microseconds () &&
                   left.evidence.reading.valid == right.evidence.reading.valid &&
                   left.evidence.status == right.evidence.status &&
                   left.age == right.age && left.valid == right.valid &&
                   left.stale == right.stale &&
                   left.approachValid == right.approachValid;
        }

        bool samePresence (const PresenceSnapshot& left,
                           const PresenceSnapshot& right) noexcept
        {
            return samePirState (left.pir, right.pir) &&
                   sameOpticalState (left.beam, right.beam) &&
                   sameOpticalState (left.finishGuard, right.finishGuard) &&
                   sameRangeState   (left.range, right.range) &&
                   left.pirEligible == right.pirEligible &&
                   left.passageEvent == right.passageEvent &&
                   left.disagreement == right.disagreement &&
                   left.disagreementFor == right.disagreementFor &&
                   left.quality == right.quality && left.status == right.status;
        }

        bool sameEvent (const CheckpointEvent& left,
                        const CheckpointEvent& right) noexcept
        {
            return left.checkpointId.value == right.checkpointId.value &&
                   left.sourceKind == right.sourceKind &&
                   left.provenance.sourceId == right.provenance.sourceId &&
                   left.provenance.calibrationRevision ==
                       right.provenance.calibrationRevision &&
                   left.provenance.observedAt == right.provenance.observedAt &&
                   left.quality == right.quality && left.status == right.status;
        }

        bool eventLess (const CheckpointEvent& left,
                        const CheckpointEvent& right) noexcept
        {
            if (left.checkpointId.value != right.checkpointId.value)
            {
                return left.checkpointId.value < right.checkpointId.value;
            }
            if (left.sourceKind != right.sourceKind)
            {
                return static_cast<uint8_t> (left.sourceKind) <
                       static_cast<uint8_t> (right.sourceKind);
            }
            if (left.provenance.sourceId != right.provenance.sourceId)
            {
                return left.provenance.sourceId < right.provenance.sourceId;
            }
            if (left.provenance.calibrationRevision !=
                right.provenance.calibrationRevision)
            {
                return left.provenance.calibrationRevision <
                       right.provenance.calibrationRevision;
            }
            return left.provenance.observedAt.milliseconds () <
                   right.provenance.observedAt.milliseconds ();
        }

        bool knownSourceKind (OpticalSourceKind kind) noexcept
        {
            return kind == OpticalSourceKind::ReflectiveAnalog ||
                   kind == OpticalSourceKind::InterruptedDigital;
        }

        bool knownPirPhase (PirPhase phase) noexcept
        {
            return phase == PirPhase::Warming || phase == PirPhase::ReadyClear ||
                   phase == PirPhase::Motion || phase == PirPhase::StuckMotion ||
                   phase == PirPhase::Fault;
        }

        bool validCheckpoint (const CheckpointEvent& event) noexcept
        {
            return event.checkpointId.value != 0U &&
                   knownSourceKind                                           (event.sourceKind) &&
                   event.quality == OpticalQuality::Valid && event.status.ok ();
        }

        RecordedCheckpoint recordedCheckpoint (const CheckpointEvent& event) noexcept
        {
            return {event.provenance.observedAt,
                    event.provenance.calibrationRevision,
                    event.checkpointId,
                    event.sourceKind,
                    event.provenance.sourceId,
                    event.quality,
                    event.status};
        }

        bool sameRecorded (const RecordedCheckpoint& accepted,
                           const CheckpointEvent&    event) noexcept
        {
            return accepted.observedAt == event.provenance.observedAt &&
                   accepted.calibrationRevision ==
                       event.provenance.calibrationRevision &&
                   accepted.checkpointId.value == event.checkpointId.value &&
                   accepted.sourceKind == event.sourceKind &&
                   accepted.sourceId == event.provenance.sourceId &&
                   accepted.quality == event.quality && accepted.status == event.status;
        }

        CoursePresentationPhase presentationPhase (MarshalPhase phase) noexcept
        {
            switch (phase)
            {
                case MarshalPhase::Disarmed:
                case MarshalPhase::Arming: return CoursePresentationPhase::Starting;
                case MarshalPhase::Ready: return CoursePresentationPhase::Ready;
                case MarshalPhase::Running: return CoursePresentationPhase::Running;
                case MarshalPhase::Finished: return CoursePresentationPhase::Finished;
                case MarshalPhase::Rejected: return CoursePresentationPhase::Rejected;
                case MarshalPhase::Fault: return CoursePresentationPhase::Fault;
            }
            return CoursePresentationPhase::Fault;
        }

        bool sameSnapshot (const CourseMarshalSnapshot& left,
                           const CourseMarshalSnapshot& right) noexcept
        {
            return left.phase == right.phase &&
                   left.checkpointCount == right.checkpointCount &&
                   left.expectedSlot == right.expectedSlot &&
                   left.expectedCheckpointId.value ==
                       right.expectedCheckpointId.value &&
                   left.acceptedCheckpointCount == right.acceptedCheckpointCount &&
                   left.elapsed == right.elapsed && left.hasRecord == right.hasRecord &&
                   left.recordSequence == right.recordSequence &&
                   left.disposition == right.disposition && left.status == right.status;
        }

        bool validMarshalPhase (MarshalPhase phase) noexcept
        {
            return phase == MarshalPhase::Disarmed || phase == MarshalPhase::Arming ||
                   phase == MarshalPhase::Ready || phase == MarshalPhase::Running ||
                   phase == MarshalPhase::Finished || phase == MarshalPhase::Rejected ||
                   phase == MarshalPhase::Fault;
        }

        bool validDisposition (RunDisposition disposition) noexcept
        {
            return disposition >= RunDisposition::None &&
                   disposition <= RunDisposition::EvidenceFault;
        }

        bool validSnapshot (const CourseMarshalSnapshot& snapshot) noexcept
        {
            if (!validMarshalPhase (snapshot.phase) ||
                !validDisposition (snapshot.disposition) ||
                snapshot.checkpointCount == 0U || snapshot.checkpointCount > 4U ||
                snapshot.acceptedCheckpointCount > snapshot.checkpointCount ||
                snapshot.expectedSlot > snapshot.checkpointCount)
            {
                return false;
            }
            const bool terminal = snapshot.phase == MarshalPhase::Finished ||
                                  snapshot.phase == MarshalPhase::Rejected;
            if (snapshot.acceptedCheckpointCount != snapshot.expectedSlot ||
                (snapshot.expectedSlot == snapshot.checkpointCount &&
                 snapshot.expectedCheckpointId.value != 0U) ||
                (snapshot.expectedSlot < snapshot.checkpointCount &&
                 snapshot.expectedCheckpointId.value == 0U) ||
                snapshot.hasRecord != terminal ||
                (!snapshot.hasRecord &&
                 (snapshot.recordSequence != 0U ||
                  snapshot.disposition != RunDisposition::None)) ||
                (snapshot.hasRecord &&
                 (snapshot.recordSequence == 0U ||
                  snapshot.disposition == RunDisposition::None)))
            {
                return false;
            }
            if ((snapshot.phase == MarshalPhase::Finished &&
                 snapshot.disposition != RunDisposition::Accepted) ||
                (snapshot.phase == MarshalPhase::Rejected &&
                 (snapshot.disposition == RunDisposition::None ||
                  snapshot.disposition == RunDisposition::Accepted)) ||
                (snapshot.phase == MarshalPhase::Fault && snapshot.status.ok ()) ||
                ((snapshot.phase == MarshalPhase::Disarmed ||
                  snapshot.phase == MarshalPhase::Arming ||
                  snapshot.phase == MarshalPhase::Ready ||
                  snapshot.phase == MarshalPhase::Fault) &&
                 snapshot.expectedSlot != 0U) ||
                (!terminal && snapshot.phase != MarshalPhase::Running &&
                 snapshot.elapsed != Duration ()))
            {
                return false;
            }
            return snapshot.status.ok () || snapshot.phase == MarshalPhase::Fault ||
                   snapshot.disposition == RunDisposition::EvidenceFault;
        }

        uint8_t elapsedDigit (Duration elapsed, Duration quantum, uint8_t cell) noexcept
        {
            uint32_t value = elapsed.milliseconds () / quantum.milliseconds ();
            if (value > 9999U)
            {
                value = 9999U;
            }
            while (cell-- != 0U)
            {
                value /= 10U;
            }
            return static_cast<uint8_t> (value % 10U);
        }

        PresenceSnapshot selectedPresence (
            RunTriggerKind kind, const PresenceSnapshot& source) noexcept
        {
            if (kind == RunTriggerKind::PresenceFault)
            {
                return source;
            }
            PresenceSnapshot selected = PresenceSnapshot ();
            if (kind == RunTriggerKind::FinishGuard)
            {
                selected.finishGuard = source.finishGuard;
                selected.range       = source.range;
            }
            else if (kind == RunTriggerKind::Range)
            {
                selected.range = source.range;
            }
            return selected;
        }
    } // namespace

    CourseStartPolicy::CourseStartPolicy (uint8_t buttonSourceId) noexcept
        : buttonSourceId_ (buttonSourceId), event_ (), lastInput_ (),
          initialized_ (false), hasInput_ (false)
    {
        event_.status = StatusCode::NotInitialized;
    }

    Status CourseStartPolicy::initialize () noexcept
    {
        if (initialized_)
        {
            return Status ();
        }
        if (buttonSourceId_ == 0U)
        {
            event_.status = StatusCode::InvalidConfiguration;
            return event_.status;
        }
        initialized_ = true;
        reset         ();
        return Status ();
    }

    void CourseStartPolicy::reset () noexcept
    {
        event_        = CourseStartEvent ();
        lastInput_    = CourseStartInput ();
        hasInput_     = false;
        event_.status = initialized_ ? Status () : Status (StatusCode::NotInitialized);
    }

    Status CourseStartPolicy::update (const CourseStartInput& input) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (hasInput_)
        {
            const Duration elapsed =
                input.observedAt.elapsedSince (lastInput_.observedAt);
            if (elapsed.milliseconds () == 0U)
            {
                if (sameStartInput (input, lastInput_))
                {
                    return event_.status;
                }
                event_.status = StatusCode::InvalidArgument;
                return event_.status;
            }
            if (elapsed.milliseconds () >= halfRange)
            {
                event_.status = StatusCode::InvalidArgument;
                return event_.status;
            }
        }

        event_ = CourseStartEvent ();
        if (input.buttonSourceId != buttonSourceId_ ||
            !knownPirPhase (input.pir.evidence.phase) ||
            (input.pir.evidence.rawLevel != Level::Low &&
             input.pir.evidence.rawLevel != Level::High) ||
            input.pir.age.milliseconds () >= halfRange ||
            (!input.pir.available &&
             !samePirState (input.pir, PirPresenceState ())) ||
            (input.pir.available &&
             (!elapsedIsValid (input.observedAt, input.pir.evidence.observedAt) ||
              input.pir.age !=
                  input.observedAt.elapsedSince (input.pir.evidence.observedAt))))
        {
            event_.status = StatusCode::InvalidArgument;
        }
        else if (!input.pir.evidence.status.ok ())
        {
            event_.status = input.pir.evidence.status;
        }
        else if (input.buttonPressEvent && input.pir.available && input.pir.valid &&
                 !input.pir.stale && input.pir.evidence.phase == PirPhase::Motion)
        {
            event_.present        = true;
            event_.source         = CourseStartSource::ExplicitButtonWithPirEligibility;
            event_.buttonSourceId = buttonSourceId_;
            event_.observedAt     = input.observedAt;
            event_.pir            = input.pir;
        }
        lastInput_ = input;
        hasInput_  = true;
        return event_.status;
    }

    CourseStartEvent CourseStartPolicy::snapshot () const noexcept
    {
        return event_;
    }

    bool CourseStartPolicy::initialized () const noexcept
    {
        return initialized_;
    }

    CourseMarshal::CourseMarshal (const CourseMarshalConfig&    config,
                                  CourseRunStorage&             runStorage,
                                  CourseTriggerStorage&         triggerStorage,
                                  CourseTriggerPresenceStorage& triggerPresenceStorage,
                                  CourseReplayFrameStorage&     replayFrameStorage,
                                  CourseReplayPresenceStorage&  replayPresenceStorage,
                                  CourseReplayEventStorage& replayEventStorage) noexcept
        : config_ (config), runStorage_ (&runStorage),
          triggerStorage_         (&triggerStorage),
          triggerPresenceStorage_ (&triggerPresenceStorage),
          replayFrameStorage_     (&replayFrameStorage),
          replayPresenceStorage_  (&replayPresenceStorage),
          replayEventStorage_     (&replayEventStorage), lastFrameAt_ (), startedAt_ (),
          nextSequence_           (1U), expectedSlot_ (0U), phase_ (MarshalPhase::Disarmed),
          status_                 (StatusCode::NotInitialized), initialized_ (false), hasFrame_ (false),
          hasRecord_              (false), sequenceExhausted_ (false)
    {
        runStorage_->record               = CourseRunRecord          ();
        triggerStorage_->trigger          = CourseTriggerRecord      ();
        triggerPresenceStorage_->presence = PresenceSnapshot         ();
        *replayFrameStorage_              = CourseReplayFrameStorage ();
        replayPresenceStorage_->presence  = PresenceSnapshot         ();
        *replayEventStorage_              = CourseReplayEventStorage ();
    }

    bool CourseMarshal::validConfig () const noexcept
    {
        if (config_.checkpointCount == 0U || config_.checkpointCount > 4U ||
            config_.buttonSourceId == 0U ||
            !validInterval (config_.checkpointEventMaximumAge) ||
            !validInterval (config_.checkpointSimultaneityWindow) ||
            !validInterval (config_.finishAgreementWindow) ||
            !validInterval (config_.maximumRunDuration))
        {
            return false;
        }
        for (uint8_t index = 0; index < 4U; ++index)
        {
            const CheckpointBinding& binding = config_.orderedCheckpoints[index];
            if (index >= config_.checkpointCount)
            {
                if (binding.checkpointId.value != 0U || binding.sourceId != 0U ||
                    binding.calibrationRevision != 0U ||
                    binding.sourceKind != OpticalSourceKind::ReflectiveAnalog)
                {
                    return false;
                }
                continue;
            }
            if (binding.checkpointId.value == 0U ||
                !knownSourceKind (binding.sourceKind))
            {
                return false;
            }
            for (uint8_t prior = 0; prior < index; ++prior)
            {
                const CheckpointBinding& other = config_.orderedCheckpoints[prior];
                if (binding.checkpointId.value == other.checkpointId.value ||
                    (binding.sourceKind == other.sourceKind &&
                     binding.sourceId == other.sourceId))
                {
                    return false;
                }
            }
        }
        return true;
    }

    Status CourseMarshal::initialize () noexcept
    {
        if (initialized_)
        {
            return Status ();
        }
        if (!validConfig ())
        {
            status_ = StatusCode::InvalidConfiguration;
            return status_;
        }
        initialized_ = true;
        reset         ();
        return Status ();
    }

    void CourseMarshal::reset () noexcept
    {
        expectedSlot_ = 0U;
        phase_        = MarshalPhase::Disarmed;
        status_       = initialized_ ? Status () : Status (StatusCode::NotInitialized);
        hasFrame_     = false;
        hasRecord_    = false;
        runStorage_->record               = CourseRunRecord          ();
        triggerStorage_->trigger          = CourseTriggerRecord      ();
        triggerPresenceStorage_->presence = PresenceSnapshot         ();
        *replayFrameStorage_              = CourseReplayFrameStorage ();
        replayPresenceStorage_->presence  = PresenceSnapshot         ();
        *replayEventStorage_              = CourseReplayEventStorage ();
    }

    void CourseMarshal::acknowledgeRecord () noexcept
    {
        if (!hasRecord_)
        {
            return;
        }
        hasRecord_                        = false;
        expectedSlot_                     = 0U;
        phase_                            = MarshalPhase::Disarmed;
        status_                           = Status              ();
        runStorage_->record               = CourseRunRecord     ();
        triggerStorage_->trigger          = CourseTriggerRecord ();
        triggerPresenceStorage_->presence = PresenceSnapshot    ();
    }

    Status CourseMarshal::update (const CourseMarshalInputView& input) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (hasRecord_)
        {
            return status_;
        }
        if (input.eventCount > 4U)
        {
            status_ = StatusCode::CapacityExceeded;
            return status_;
        }
        if (input.presence == nullptr ||
            (input.eventCount != 0U && input.events == nullptr))
        {
            status_ = StatusCode::InvalidArgument;
            return status_;
        }
        if (hasFrame_)
        {
            const Duration elapsed = input.observedAt.elapsedSince (lastFrameAt_);
            if (elapsed.milliseconds                               () == 0U)
            {
                bool identical =
                    replayFrameStorage_->present &&
                    replayFrameStorage_->observedAt == input.observedAt &&
                    replayFrameStorage_->eventCount == input.eventCount &&
                    sameStartEvent (replayFrameStorage_->start, input.start) &&
                    input.presence != nullptr &&
                    samePresence (replayPresenceStorage_->presence, *input.presence);
                for (uint8_t index = 0; identical && index < input.eventCount; ++index)
                {
                    identical = input.events != nullptr &&
                                sameEvent (replayEventStorage_->events[index],
                                           input.events[index]);
                }
                if (identical)
                {
                    return status_;
                }
                phase_  = MarshalPhase::Fault;
                status_ = StatusCode::InvalidArgument;
                return status_;
            }
            if (elapsed.milliseconds () >= halfRange)
            {
                phase_  = MarshalPhase::Fault;
                status_ = StatusCode::InvalidArgument;
                return status_;
            }
        }
        if (hasRecord_)
        {
            return status_;
        }

        status_ = Status ();

        uint8_t invalidEvents[4] = {};
        uint8_t invalidSlots[4]  = {};
        uint8_t invalidCount     = 0U;
        for (uint8_t index = 0; index < input.eventCount; ++index)
        {
            const CheckpointEvent& event = input.events[index];
            bool bindingMatches          = false;
            uint8_t matchedSlot           = 4U;
            for (uint8_t slot = 0; slot < config_.checkpointCount; ++slot)
            {
                const CheckpointBinding& binding = config_.orderedCheckpoints[slot];
                const bool matches =
                    event.checkpointId.value == binding.checkpointId.value &&
                    event.sourceKind == binding.sourceKind &&
                    event.provenance.sourceId == binding.sourceId &&
                    event.provenance.calibrationRevision ==
                        binding.calibrationRevision;
                if (matches)
                {
                    bindingMatches = true;
                    matchedSlot    = slot;
                }
            }
            if (!validCheckpoint   (event) || !bindingMatches ||
                !elapsedIsValid               (input.observedAt,
                                               event.provenance.observedAt) ||
                input.observedAt.elapsedSince (event.provenance.observedAt) >=
                    config_.checkpointEventMaximumAge)
            {
                invalidEvents[invalidCount] = index;
                invalidSlots[invalidCount]  = matchedSlot;
                ++invalidCount;
            }
        }
        for (uint8_t left = 0; left < invalidCount; ++left)
        {
            for (uint8_t right = left + 1U; right < invalidCount; ++right)
            {
                const bool rightFirst =
                    invalidSlots[right] < invalidSlots[left] ||
                    (invalidSlots[right] == invalidSlots[left] &&
                     eventLess (input.events[invalidEvents[right]],
                                input.events[invalidEvents[left]]));
                if (rightFirst)
                {
                    const uint8_t eventSwap = invalidEvents[left];
                    invalidEvents[left]     = invalidEvents[right];
                    invalidEvents[right]    = eventSwap;
                    const uint8_t slotSwap  = invalidSlots[left];
                    invalidSlots[left]      = invalidSlots[right];
                    invalidSlots[right]     = slotSwap;
                }
            }
        }
        if (invalidCount != 0U)
        {
            const CheckpointEvent& first = input.events[invalidEvents[0]];
            status_ = first.status.ok () ? Status (StatusCode::InvalidArgument)
                                         : first.status;
            if (phase_ == MarshalPhase::Running)
            {
                CourseRunRecord& failed  = runStorage_->record;
                failed.finishedAt        = input.observedAt;
                failed.elapsed           = input.observedAt.elapsedSince (startedAt_);
                failed.disposition       = RunDisposition::EvidenceFault;
                failed.status            = status_;
                triggerStorage_->trigger = CourseTriggerRecord ();
                triggerStorage_->trigger.kind       = RunTriggerKind::Checkpoint;
                triggerStorage_->trigger.observedAt = input.observedAt;
                triggerStorage_->trigger.checkpointCount = invalidCount;
                triggerStorage_->trigger.status = status_;
                for (uint8_t index = 0; index < invalidCount; ++index)
                {
                    triggerStorage_->trigger.checkpoints[index] =
                        recordedCheckpoint (input.events[invalidEvents[index]]);
                }
                phase_     = MarshalPhase::Rejected;
                hasRecord_ = true;
            }
            else
            {
                phase_ = MarshalPhase::Fault;
            }
            return status_;
        }

        const bool malformedStart =
            (!input.start.present && !canonicalAbsentStart (input.start)) ||
            (input.start.present &&
             (!knownPirPhase (input.start.pir.evidence.phase) ||
              input.start.source !=
                  CourseStartSource::ExplicitButtonWithPirEligibility ||
              input.start.buttonSourceId != config_.buttonSourceId ||
              input.start.observedAt != input.observedAt ||
              !input.start.status.ok () || !input.start.pir.available ||
              !input.start.pir.valid || input.start.pir.stale ||
              !input.start.pir.evidence.status.ok () ||
              input.start.pir.evidence.phase != PirPhase::Motion ||
              !samePirState (input.start.pir, input.presence->pir)));
        if (malformedStart)
        {
            status_ = StatusCode::InvalidArgument;
            runStorage_->record                 = CourseRunRecord     ();
            triggerStorage_->trigger            = CourseTriggerRecord ();
            triggerPresenceStorage_->presence   = PresenceSnapshot    ();
            runStorage_->record.sequence        = nextSequence_;
            runStorage_->record.finishedAt      = input.observedAt;
            runStorage_->record.start           = input.start;
            runStorage_->record.disposition     = RunDisposition::EvidenceFault;
            runStorage_->record.status          = status_;
            triggerStorage_->trigger.kind       = RunTriggerKind::Start;
            triggerStorage_->trigger.observedAt = input.observedAt;
            triggerStorage_->trigger.start      = input.start;
            triggerStorage_->trigger.status     = status_;
            phase_                              = MarshalPhase::Rejected;
            hasRecord_                          = true;
            if (nextSequence_ == UINT32_MAX)
            {
                sequenceExhausted_ = true;
            }
            else
            {
                ++nextSequence_;
            }
            return status_;
        }

        if (!input.presence->status.ok () ||
            input.presence->quality != PresenceQuality::Valid)
        {
            status_ = input.presence->status.ok ()
                          ? Status (StatusCode::InvalidArgument)
                          : input.presence->status;
            if (phase_ == MarshalPhase::Running)
            {
                CourseRunRecord& failed  = runStorage_->record;
                failed.finishedAt        = input.observedAt;
                failed.elapsed           = input.observedAt.elapsedSince (startedAt_);
                failed.disposition       = RunDisposition::EvidenceFault;
                failed.status            = status_;
                failed.sequenceExhausted = sequenceExhausted_;
                triggerStorage_->trigger = CourseTriggerRecord ();
                triggerStorage_->trigger.kind       = RunTriggerKind::PresenceFault;
                triggerStorage_->trigger.observedAt = input.observedAt;
                triggerStorage_->trigger.status     = status_;
                triggerPresenceStorage_->presence =
                    selectedPresence (RunTriggerKind::PresenceFault, *input.presence);
                phase_                              = MarshalPhase::Rejected;
                hasRecord_                          = true;
            }
            else
            {
                phase_ = MarshalPhase::Fault;
            }
            return status_;
        }

        lastFrameAt_                     = input.observedAt;
        hasFrame_                        = true;
        replayFrameStorage_->observedAt  = input.observedAt;
        replayFrameStorage_->start       = input.start;
        replayFrameStorage_->eventCount  = input.eventCount;
        replayFrameStorage_->present     = true;
        replayPresenceStorage_->presence = *input.presence;
        for (uint8_t index = 0; index < 4U; ++index)
        {
            replayEventStorage_->events[index] =
                index < input.eventCount ? input.events[index] : CheckpointEvent ();
        }

        if (phase_ != MarshalPhase::Running)
        {
            phase_ = input.presence->pirEligible ? MarshalPhase::Ready
                                                 : MarshalPhase::Arming;
            if (input.start.present)
            {
                if (phase_ != MarshalPhase::Ready ||
                    input.start.source !=
                        CourseStartSource::ExplicitButtonWithPirEligibility ||
                    input.start.buttonSourceId != config_.buttonSourceId ||
                    input.start.observedAt != input.observedAt ||
                    !input.start.status.ok () || !input.start.pir.available ||
                    !input.start.pir.valid || input.start.pir.stale ||
                    input.start.pir.evidence.phase != PirPhase::Motion ||
                    !samePirState (input.start.pir, input.presence->pir))
                {
                    phase_  = MarshalPhase::Fault;
                    status_ = StatusCode::InvalidArgument;
                    return status_;
                }
                if (sequenceExhausted_)
                {
                    return StatusCode::CapacityExceeded;
                }
                startedAt_                        = input.observedAt;
                expectedSlot_                     = 0U;
                phase_                            = MarshalPhase::Running;
                runStorage_->record               = CourseRunRecord     ();
                triggerStorage_->trigger          = CourseTriggerRecord ();
                triggerPresenceStorage_->presence = PresenceSnapshot    ();
                runStorage_->record.sequence      = nextSequence_;
                runStorage_->record.startedAt     = startedAt_;
                runStorage_->record.start         = input.start;
                if (nextSequence_ == UINT32_MAX)
                {
                    sequenceExhausted_ = true;
                }
                else
                {
                    ++nextSequence_;
                }
            }
            return status_;
        }

        CourseRunRecord& recordValue = runStorage_->record;
        const Duration   runElapsed  = input.observedAt.elapsedSince (startedAt_);
        if (runElapsed.milliseconds                                  () >= halfRange ||
            runElapsed > config_.maximumRunDuration)
        {
            recordValue.finishedAt              = input.observedAt;
            recordValue.elapsed                 = runElapsed;
            recordValue.disposition             = RunDisposition::TimedOut;
            recordValue.sequenceExhausted       = sequenceExhausted_;
            triggerStorage_->trigger.kind       = RunTriggerKind::RunTimeout;
            triggerStorage_->trigger.observedAt = input.observedAt;
            phase_                              = MarshalPhase::Rejected;
            hasRecord_                          = true;
            return status_;
        }

        uint8_t eventSlots[4] = {};
        uint8_t eventOrder[4] = {};
        bool    consumed[4]   = {};
        for (uint8_t index = 0; index < input.eventCount; ++index)
        {
            const CheckpointEvent& event = input.events[index];
            if (!validCheckpoint (event) ||
                !elapsedIsValid               (input.observedAt, event.provenance.observedAt) ||
                input.observedAt.elapsedSince (event.provenance.observedAt) >=
                    config_.checkpointEventMaximumAge)
            {
                status_ = event.status.ok () ? Status (StatusCode::InvalidArgument)
                                             : event.status;
                recordValue.finishedAt              = input.observedAt;
                recordValue.elapsed                 = runElapsed;
                recordValue.disposition             = RunDisposition::EvidenceFault;
                recordValue.status                  = status_;
                recordValue.sequenceExhausted       = sequenceExhausted_;
                triggerStorage_->trigger            = CourseTriggerRecord ();
                triggerStorage_->trigger.kind       = RunTriggerKind::Checkpoint;
                triggerStorage_->trigger.observedAt = input.observedAt;
                triggerStorage_->trigger.checkpointCount = 1U;
                triggerStorage_->trigger.checkpoints[0]  = recordedCheckpoint (event);
                phase_                                   = MarshalPhase::Rejected;
                hasRecord_                               = true;
                return status_;
            }
            uint8_t slot = 4U;
            for (uint8_t candidate = 0; candidate < config_.checkpointCount;
                 ++candidate)
            {
                const CheckpointBinding& binding =
                    config_.orderedCheckpoints[candidate];
                if (event.checkpointId.value == binding.checkpointId.value &&
                    event.sourceKind == binding.sourceKind &&
                    event.provenance.sourceId == binding.sourceId &&
                    event.provenance.calibrationRevision == binding.calibrationRevision)
                {
                    slot = candidate;
                }
            }
            if (slot == 4U)
            {
                status_                             = StatusCode::InvalidArgument;
                recordValue.finishedAt              = input.observedAt;
                recordValue.elapsed                 = runElapsed;
                recordValue.disposition             = RunDisposition::EvidenceFault;
                recordValue.status                  = status_;
                triggerStorage_->trigger            = CourseTriggerRecord ();
                triggerStorage_->trigger.kind       = RunTriggerKind::Checkpoint;
                triggerStorage_->trigger.observedAt = input.observedAt;
                triggerStorage_->trigger.checkpointCount = 1U;
                triggerStorage_->trigger.checkpoints[0]  = recordedCheckpoint (event);
                phase_                                   = MarshalPhase::Rejected;
                hasRecord_                               = true;
                return status_;
            }
            eventSlots[index] = slot;
            eventOrder[index] = index;
            if (slot < expectedSlot_ &&
                sameRecorded (recordValue.acceptedCheckpoints[slot], event))
            {
                consumed[index] = true;
            }
        }
        for (uint8_t left = 0; left < input.eventCount; ++left)
        {
            for (uint8_t right = left + 1U; right < input.eventCount; ++right)
            {
                if (eventSlots[eventOrder[right]] < eventSlots[eventOrder[left]])
                {
                    const uint8_t swap = eventOrder[left];
                    eventOrder[left]   = eventOrder[right];
                    eventOrder[right]  = swap;
                }
            }
        }

        for (uint8_t left = 0; left < input.eventCount; ++left)
        {
            for (uint8_t right = left + 1U; right < input.eventCount; ++right)
            {
                const TimePoint leftTime  = input.events[left].provenance.observedAt;
                const TimePoint rightTime = input.events[right].provenance.observedAt;
                const Duration  separation =
                    leftTime.elapsedSince (rightTime).milliseconds () < halfRange
                        ? leftTime.elapsedSince  (rightTime)
                        : rightTime.elapsedSince (leftTime);
                if (eventSlots[left] != eventSlots[right] && !consumed[left] &&
                    !consumed[right] &&
                    separation <= config_.checkpointSimultaneityWindow)
                {
                    recordValue.disposition = RunDisposition::SimultaneousCheckpoints;
                    phase_                  = MarshalPhase::Rejected;
                    hasRecord_              = true;
                }
            }
        }
        if (hasRecord_)
        {
            recordValue.finishedAt              = input.observedAt;
            recordValue.elapsed                 = runElapsed;
            triggerStorage_->trigger.kind       = RunTriggerKind::Checkpoint;
            triggerStorage_->trigger.observedAt = input.observedAt;
            for (uint8_t order = 0; order < input.eventCount; ++order)
            {
                const uint8_t index = eventOrder[order];
                if (!consumed[index])
                {
                    triggerStorage_->trigger
                        .checkpoints[triggerStorage_->trigger.checkpointCount++] =
                        recordedCheckpoint (input.events[index]);
                }
            }
            return status_;
        }

        for (uint8_t order = 0; order < input.eventCount; ++order)
        {
            const uint8_t index = eventOrder[order];
            if (consumed[index])
            {
                continue;
            }
            const uint8_t slot = eventSlots[index];
            if (slot < expectedSlot_)
            {
                recordValue.disposition = RunDisposition::DuplicateCheckpoint;
            }
            else if (slot > expectedSlot_)
            {
                recordValue.disposition = RunDisposition::SkippedCheckpoint;
            }
            else if (order + 1U < input.eventCount)
            {
                const uint8_t next = eventOrder[order + 1U];
                if (!consumed[next] &&
                    input.events[next]
                            .provenance.observedAt
                            .elapsedSince (input.events[index].provenance.observedAt)
                            .milliseconds () >= halfRange)
                {
                    recordValue.disposition = RunDisposition::ReversedCheckpoint;
                }
            }
            if (recordValue.disposition == RunDisposition::None)
            {
                recordValue.acceptedCheckpoints[expectedSlot_] =
                    recordedCheckpoint (input.events[index]);
                ++expectedSlot_;
                recordValue.acceptedCheckpointCount = expectedSlot_;
                continue;
            }
            recordValue.finishedAt                   = input.observedAt;
            recordValue.elapsed                      = runElapsed;
            triggerStorage_->trigger.kind            = RunTriggerKind::Checkpoint;
            triggerStorage_->trigger.observedAt      = input.observedAt;
            triggerStorage_->trigger.checkpointCount = 1U;
            triggerStorage_->trigger.checkpoints[0] =
                recordedCheckpoint (input.events[index]);
            phase_     = MarshalPhase::Rejected;
            hasRecord_ = true;
            return status_;
        }

        if (input.presence->finishGuard.activationEvent)
        {
            const bool finishValid = input.presence->finishGuard.available &&
                                     input.presence->finishGuard.valid &&
                                     !input.presence->finishGuard.stale;
            const bool rangeValid =
                input.presence->range.available && input.presence->range.valid &&
                !input.presence->range.stale && input.presence->range.approachValid;
            const uint32_t guardAge = input.presence->finishGuard.age.milliseconds ();
            const uint32_t rangeAge = input.presence->range.age.milliseconds       ();
            const uint32_t separation =
                guardAge > rangeAge ? guardAge - rangeAge : rangeAge - guardAge;
            if ((input.presence->finishGuard.available && !finishValid) ||
                (input.presence->range.available &&
                 (!input.presence->range.valid || input.presence->range.stale ||
                  !input.presence->range.evidence.status.ok ())))
            {
                status_ = !input.presence->finishGuard.status.ok ()
                              ? input.presence->finishGuard.status
                              : (!input.presence->range.evidence.status.ok ()
                                     ? input.presence->range.evidence.status
                                     : Status (StatusCode::InvalidArgument));
                recordValue.disposition       = RunDisposition::EvidenceFault;
                recordValue.status            = status_;
                phase_                        = MarshalPhase::Rejected;
                triggerStorage_->trigger.kind = !finishValid
                                                    ? RunTriggerKind::PresenceFault
                                                    : RunTriggerKind::Range;
                triggerStorage_->trigger.status = status_;
            }
            else if (expectedSlot_ == config_.checkpointCount && finishValid &&
                     rangeValid &&
                     separation <= config_.finishAgreementWindow.milliseconds ())
            {
                recordValue.disposition = RunDisposition::Accepted;
                phase_                  = MarshalPhase::Finished;
            }
            else
            {
                recordValue.disposition = RunDisposition::FinishTooEarly;
                phase_                  = MarshalPhase::Rejected;
            }
            recordValue.finishedAt        = input.observedAt;
            recordValue.elapsed           = runElapsed;
            recordValue.sequenceExhausted = sequenceExhausted_;
            if (triggerStorage_->trigger.kind == RunTriggerKind::None)
            {
                triggerStorage_->trigger.kind = RunTriggerKind::FinishGuard;
            }
            triggerStorage_->trigger.observedAt = input.observedAt;
            triggerPresenceStorage_->presence =
                selectedPresence (triggerStorage_->trigger.kind, *input.presence);
            hasRecord_                          = true;
        }
        return status_;
    }

    CourseMarshalSnapshot CourseMarshal::snapshot () const noexcept
    {
        CourseMarshalSnapshot value = CourseMarshalSnapshot ();
        value.phase                 = phase_;
        value.checkpointCount       = config_.checkpointCount;
        value.expectedSlot          = expectedSlot_;
        if (expectedSlot_ < config_.checkpointCount)
        {
            value.expectedCheckpointId =
                config_.orderedCheckpoints[expectedSlot_].checkpointId;
        }
        value.acceptedCheckpointCount = expectedSlot_;
        value.elapsed = phase_ == MarshalPhase::Running && hasFrame_
                            ? lastFrameAt_.elapsedSince                            (startedAt_)
                            : (hasRecord_ ? runStorage_->record.elapsed : Duration ());
        value.hasRecord      = hasRecord_;
        value.recordSequence = hasRecord_ ? runStorage_->record.sequence : 0U;
        value.disposition =
            hasRecord_ ? runStorage_->record.disposition : RunDisposition::None;
        value.status = status_;
        return value;
    }

    const CourseRunRecord& CourseMarshal::record () const noexcept
    {
        return runStorage_->record;
    }

    const CourseTriggerRecord& CourseMarshal::trigger () const noexcept
    {
        return triggerStorage_->trigger;
    }

    const PresenceSnapshot& CourseMarshal::triggerPresence () const noexcept
    {
        return triggerPresenceStorage_->presence;
    }

    bool CourseMarshal::initialized () const noexcept
    {
        return initialized_;
    }

    CourseMarshalPresenter::CourseMarshalPresenter (Duration displayQuantum,
                                                    Duration heartbeatInterval) noexcept
        : displayQuantum_ (displayQuantum), heartbeatInterval_ (heartbeatInterval),
          intent_    (), lastSnapshot_ (), epoch_ (), lastUpdate_ (), initialized_ (false),
          hasUpdate_ (false)
    {
        intent_.status = StatusCode::NotInitialized;
    }

    Status CourseMarshalPresenter::initialize () noexcept
    {
        if (initialized_)
        {
            return Status ();
        }
        if (!validInterval (displayQuantum_) || !validInterval (heartbeatInterval_))
        {
            intent_.status = StatusCode::InvalidConfiguration;
            return intent_.status;
        }
        initialized_ = true;
        reset         ();
        return Status ();
    }

    void CourseMarshalPresenter::reset () noexcept
    {
        intent_        = CoursePresentationIntent ();
        lastSnapshot_  = CourseMarshalSnapshot    ();
        epoch_         = TimePoint                ();
        lastUpdate_    = TimePoint                ();
        hasUpdate_     = false;
        intent_.phase  = CoursePresentationPhase::Starting;
        intent_.status = initialized_ ? Status () : Status (StatusCode::NotInitialized);
    }

    Status
    CourseMarshalPresenter::update (TimePoint                    now,
                                    const CourseMarshalSnapshot& snapshot) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (!validSnapshot (snapshot))
        {
            intent_.status = StatusCode::InvalidArgument;
            intent_.phase  = CoursePresentationPhase::Fault;
            return intent_.status;
        }
        if (hasUpdate_)
        {
            const Duration elapsed = now.elapsedSince (lastUpdate_);
            if (elapsed.milliseconds                  () == 0U)
            {
                if (sameSnapshot (snapshot, lastSnapshot_))
                {
                    return intent_.status;
                }
                intent_.status = StatusCode::InvalidArgument;
                intent_.phase  = CoursePresentationPhase::Fault;
                return intent_.status;
            }
            if (elapsed.milliseconds () >= halfRange)
            {
                intent_.status = StatusCode::InvalidArgument;
                intent_.phase  = CoursePresentationPhase::Fault;
                return intent_.status;
            }
            intent_.displayCell =
                static_cast<uint8_t> ((intent_.displayCell + 1U) % 4U);
        }
        else
        {
            epoch_              = now;
            intent_.displayCell = 0U;
        }

        intent_.observedAt = now;
        intent_.phase      = presentationPhase (snapshot.phase);
        intent_.acceptedMask =
            static_cast<uint8_t> ((1U << snapshot.acceptedCheckpointCount) - 1U);
        intent_.expectedSlot   = snapshot.expectedSlot;
        intent_.hasRecord      = snapshot.hasRecord;
        intent_.recordSequence = snapshot.hasRecord ? snapshot.recordSequence : 0U;
        intent_.elapsed = snapshot.phase == MarshalPhase::Running || snapshot.hasRecord
                              ? snapshot.elapsed
                              : Duration ();
        intent_.allRed  = intent_.phase == CoursePresentationPhase::Rejected ||
                          intent_.phase == CoursePresentationPhase::Fault;
        intent_.heartbeat = ((now.elapsedSince (epoch_).milliseconds () /
                              heartbeatInterval_.milliseconds ()) %
                             2U) != 0U;
        switch (intent_.phase)
        {
            case CoursePresentationPhase::Starting: intent_.displayValue = 0U; break;
            case CoursePresentationPhase::Ready:
                intent_.displayValue =
                    static_cast<uint8_t> (snapshot.expectedSlot + 1U);
                break;
            case CoursePresentationPhase::Running:
            case CoursePresentationPhase::Finished:
                intent_.displayValue = elapsedDigit (intent_.elapsed, displayQuantum_,
                                                     intent_.displayCell);
                break;
            case CoursePresentationPhase::Rejected: intent_.displayValue = 0x0eU; break;
            case CoursePresentationPhase::Fault: intent_.displayValue = 0x0fU; break;
        }
        intent_.status = snapshot.status;
        lastSnapshot_  = snapshot;
        lastUpdate_    = now;
        hasUpdate_     = true;
        return intent_.status;
    }

    CoursePresentationIntent CourseMarshalPresenter::intent () const noexcept
    {
        return intent_;
    }

    bool CourseMarshalPresenter::initialized () const noexcept
    {
        return initialized_;
    }
} // namespace adk
