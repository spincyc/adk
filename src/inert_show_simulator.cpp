#include "inert_show_simulator.h"

namespace adk {

    namespace {

        constexpr uint32_t maximumUnambiguousDuration = 0x7fffffffu;
        constexpr uint32_t fnvOffsetBasis             = 2166136261u;
        constexpr uint32_t fnvPrime                   = 16777619u;

        bool validObservation (InertObservation observation) noexcept
        {
            switch (observation)
            {
                case InertObservation::Open:
                case InertObservation::Closed:
                case InertObservation::ShortSimulated:
                case InertObservation::Unavailable: return true;
            }

            return false;
        }

        InertShowSnapshot inertSnapshot () noexcept
        {
            const InertChannelAssessment channel  = {0, InertChannelState::Unavailable,
                                                     TimePoint ()};
            const CueSchedulerSnapshot   schedule = {CueSchedulerPhase::Idle,
                                                     CueDecision::Waiting,
                                                     0,
                                                     0,
                                                     Duration (),
                                                     Duration (),
                                                     StatusCode::NotInitialized,
                                                     false};

            return {InertShowState::Startup,
                    InertShowFault::None,
                    channel,
                    schedule,
                    0,
                    fnvOffsetBasis,
                    StatusCode::NotInitialized,
                    false};
        }

        bool sameOperatorInput (const CueOperatorInput& left,
                                const CueOperatorInput& right) noexcept
        {
            return left.reviewHeld == right.reviewHeld &&
                   left.runPressed == right.runPressed &&
                   left.confirmPressed == right.confirmPressed &&
                   left.skipPressed == right.skipPressed &&
                   left.cancelPressed == right.cancelPressed;
        }
    } // namespace

    InertShowSimulator::InertShowSimulator (const InertCueChannelMap& map,
                                            InertChannelAssessor&     assessor,
                                            InertCueScheduler&        scheduler,
                                            CueAuditBuffer&           audit) noexcept
        : map_ (map), assessor_ (assessor), scheduler_ (scheduler), audit_ (audit),
          snapshot_         (inertSnapshot ()), lastInput_{}, lastUpdatedAt_ (),
          traceDigest_      (fnvOffsetBasis), hasLastFrame_ (false),
          ownsAuditSession_ (false), initialized_ (false)
    {
        for (uint8_t channel = 0; channel < InertChannelAssessor::capacity; ++channel)
        {
            lastObservations_[channel] = {channel, InertObservation::Unavailable,
                                          InertObservation::Unavailable, TimePoint ()};
        }
    }

    InertShowSimulator::~InertShowSimulator () noexcept
    {
        shutdown ();
    }

    Status InertShowSimulator::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!validMap () || assessor_.initialized () || scheduler_.initialized ())
        {
            return StatusCode::InvalidConfiguration;
        }

        if (audit_.initialized ())
        {
            if (!ownsAuditSession_)
            {
                return StatusCode::ResourceBusy;
            }

            audit_.shutdown ();
            ownsAuditSession_ = false;
        }

        const Status assessorStatus = assessor_.initialize ();

        if (!assessorStatus.ok ())
        {
            return assessorStatus;
        }

        const Status schedulerStatus = scheduler_.initialize ();

        if (!schedulerStatus.ok ())
        {
            assessor_.shutdown ();
            return schedulerStatus;
        }

        snapshot_ = inertSnapshot ();

        snapshot_.schedule = scheduler_.snapshot ();

        snapshot_.auditSequence = audit_.count ();
        snapshot_.status        = StatusCode::Ok;
        traceDigest_            = fnvOffsetBasis;
        snapshot_.traceDigest   = traceDigest_;
        lastInput_              = {};
        lastUpdatedAt_          = TimePoint ();
        hasLastFrame_           = false;
        ownsAuditSession_       = true;
        initialized_            = true;
        return StatusCode::Ok;
    }

    void InertShowSimulator::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        scheduler_.shutdown ();
        assessor_. shutdown ();

        snapshot_ = inertSnapshot ();

        snapshot_.auditSequence = audit_.count ();
        snapshot_.traceDigest   = traceDigest_;
        hasLastFrame_           = false;
        initialized_            = false;
    }

    bool InertShowSimulator::initialized () const noexcept
    {
        return initialized_;
    }

    Status InertShowSimulator::update (TimePoint             now,
                                       const InertShowInput& input) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        InertChannelObservation canonical[InertChannelAssessor::capacity];
        const Status            frameStatus = canonicalizeFrame (now, input, canonical);

        if (!frameStatus.ok ())
        {
            if (input.operatorInput.cancelPressed &&
                (!hasLastFrame_ || now != lastUpdatedAt_))
            {
                const CueOperatorInput cancel = {false, false, false, false, true};
                const Status           cancelStatus = scheduler_.update (now, cancel);

                refreshSnapshot (now);
                return cancelStatus;
            }

            return frameStatus;
        }

        if (hasLastFrame_ && now == lastUpdatedAt_)
        {
            if (sameFrame (canonical, input.operatorInput))
            {
                return StatusCode::Ok;
            }

            enterInputFault (now, input.operatorInput, StatusCode::InvalidArgument);

            for (uint8_t index = 0; index < InertChannelAssessor::capacity; ++index)
            {
                lastObservations_[index] = canonical[index];
            }

            lastInput_     = input.operatorInput;
            lastUpdatedAt_ = now;
            updateDigest (now, canonical, input.operatorInput);
            return StatusCode::InvalidArgument;
        }

        if (hasLastFrame_ && now.elapsedSince (lastUpdatedAt_).milliseconds () >
                                 maximumUnambiguousDuration)
        {
            return StatusCode::InvalidArgument;
        }

        const Status assessmentStatus =
            assessor_.update (now, canonical, InertChannelAssessor::capacity);

        if (!assessmentStatus.ok ())
        {
            enterInputFault (now, input.operatorInput, assessmentStatus);
            return assessmentStatus;
        }

        const CueSchedulerSnapshot before = scheduler_.snapshot ();
        CueEvidenceGate gate = {CueEvidenceDisposition::Permit, StatusCode::Ok};
        bool            gateSelected =
            before.hasCue ||
            (before.phase == CueSchedulerPhase::Held && before.cueIndex < map_.count) ||
            (before.phase == CueSchedulerPhase::Review &&
             input.operatorInput.runPressed);

        if (gateSelected)
        {
            const uint8_t        cueIndex = before.hasCue ? before.cueIndex
                                            : before.phase == CueSchedulerPhase::Held
                                                ? before.cueIndex
                                                : 0;
            const InertChannelId channel  = map_.channels[cueIndex];
            const Result<InertChannelAssessment> selected =
                assessor_.assessment (channel, now);

            if (!selected.ok ())
            {
                enterInputFault (now, input.operatorInput, selected.status ());

                return selected.status ();
            }

            gate = evidenceGate (selected.value ());
        }

        const Status schedulerStatus =
            scheduler_.update (now, input.operatorInput, gate);

        for (uint8_t index = 0; index < InertChannelAssessor::capacity; ++index)
        {
            lastObservations_[index] = canonical[index];
        }

        lastInput_     = input.operatorInput;
        lastUpdatedAt_ = now;
        hasLastFrame_  = true;

        refreshSnapshot (now);

        if (schedulerStatus.error () == StatusCode::CapacityExceeded)
        {
            snapshot_.fault = InertShowFault::AuditFull;
        }

        updateDigest (now, canonical, input.operatorInput);
        return schedulerStatus;
    }

    InertShowSnapshot InertShowSimulator::snapshot () const noexcept
    {
        return snapshot_;
    }

    bool InertShowSimulator::validMap () const noexcept
    {
        if (map_.count == 0 || map_.count != scheduler_.cueCount ())
        {
            return false;
        }

        for (uint8_t index = 0; index < map_.count; ++index)
        {
            if (map_.channels[index] >= InertChannelAssessor::capacity ||
                !scheduler_.cue (index).ok ())
            {
                return false;
            }
        }

        return true;
    }

    Status InertShowSimulator::canonicalizeFrame (
        TimePoint now, const InertShowInput& input,
        InertChannelObservation (
            &canonical)[InertChannelAssessor::capacity]) const noexcept
    {
        if (input.observations == nullptr ||
            input.observationCount != InertChannelAssessor::capacity)
        {
            return StatusCode::InvalidArgument;
        }

        bool seen[InertChannelAssessor::capacity] = {};

        for (uint8_t index = 0; index < input.observationCount; ++index)
        {
            const InertChannelObservation& observation = input.observations[index];

            if (observation.channel >= InertChannelAssessor::capacity ||
                seen[observation.channel] || !validObservation (observation.primary) ||
                !                             validObservation (observation.redundant))
            {
                return StatusCode::InvalidArgument;
            }

            const uint32_t age =
                now.elapsedSince (observation.observedAt).milliseconds ();

            if (age > maximumUnambiguousDuration)
            {
                return StatusCode::InvalidArgument;
            }

            seen[observation.channel]      = true;
            canonical[observation.channel] = observation;
        }

        return StatusCode::Ok;
    }

    bool InertShowSimulator::sameFrame (
        const InertChannelObservation (&canonical)[InertChannelAssessor::capacity],
        const CueOperatorInput& input) const noexcept
    {
        if (!sameOperatorInput (input, lastInput_))
        {
            return false;
        }

        for (uint8_t index = 0; index < InertChannelAssessor::capacity; ++index)
        {
            const InertChannelObservation& left  = canonical[index];
            const InertChannelObservation& right = lastObservations_[index];

            if (left.channel != right.channel || left.primary != right.primary ||
                left.redundant != right.redundant ||
                left.observedAt != right.observedAt)
            {
                return false;
            }
        }

        return true;
    }

    CueEvidenceGate InertShowSimulator::evidenceGate (
        const InertChannelAssessment& assessment) const noexcept
    {
        switch (assessment.state)
        {
            case InertChannelState::Closed:
                return {CueEvidenceDisposition::Permit, StatusCode::Ok};
            case InertChannelState::Open:
            case InertChannelState::ShortSimulated:
            case InertChannelState::Stale:
            case InertChannelState::Unavailable:
                return {CueEvidenceDisposition::Hold, StatusCode::Ok};
            case InertChannelState::Contradictory:
                return {CueEvidenceDisposition::Fault,
                        StatusCode::InvalidConfiguration};
        }

        return {CueEvidenceDisposition::Fault, StatusCode::InternalInvariant};
    }

    void InertShowSimulator::refreshSnapshot (TimePoint now) noexcept
    {
        snapshot_.schedule = scheduler_.snapshot ();

        snapshot_.auditSequence      = audit_.count ();
        snapshot_.status             = snapshot_.schedule.status;
        snapshot_.fault              = InertShowFault::None;
        snapshot_.hasSelectedChannel = false;

        if (snapshot_.schedule.hasCue && snapshot_.schedule.cueIndex < map_.count)
        {
            const InertChannelId channel = map_.channels[snapshot_.schedule.cueIndex];
            const Result<InertChannelAssessment> selected =
                assessor_.assessment (channel, now);

            if (selected.ok ())
            {
                snapshot_.selectedChannel    = selected.value ();
                snapshot_.hasSelectedChannel = true;
            }
        }

        switch (snapshot_.schedule.phase)
        {
            case CueSchedulerPhase::Idle:
                snapshot_.state = InertShowState::Startup;
                break;
            case CueSchedulerPhase::Review:
                snapshot_.state = InertShowState::Review;
                break;
            case CueSchedulerPhase::Waiting:
            case CueSchedulerPhase::Confirmation:
                snapshot_.state = InertShowState::Ready;
                break;
            case CueSchedulerPhase::Active:
                snapshot_.state = InertShowState::Running;
                break;
            case CueSchedulerPhase::Held:
                snapshot_.state = InertShowState::Held;

                if (snapshot_.schedule.status.error () == StatusCode::CapacityExceeded)
                {
                    snapshot_.fault = InertShowFault::AuditFull;
                }
                break;
            case CueSchedulerPhase::Complete:
                snapshot_.state = InertShowState::Complete;
                break;
            case CueSchedulerPhase::Cancelled:
                snapshot_.state = InertShowState::Cancelled;
                break;
            case CueSchedulerPhase::Fault:
                snapshot_.state = InertShowState::Fault;
                snapshot_.fault = snapshot_.schedule.status.error () ==
                                          StatusCode::InvalidConfiguration
                                      ? InertShowFault::ObservationContradictory
                                      : InertShowFault::InvalidInput;
                break;
        }
    }

    void InertShowSimulator::enterInputFault (TimePoint               now,
                                              const CueOperatorInput& input,
                                              Status                  status) noexcept
    {
        const CueEvidenceGate fault           = {CueEvidenceDisposition::Fault, status};
        const Status          schedulerStatus = scheduler_.update (now, input, fault);

        refreshSnapshot (now);
        snapshot_.state  = InertShowState::Fault;
        snapshot_.fault  = schedulerStatus.error () == StatusCode::CapacityExceeded
                               ? InertShowFault::AuditFull
                               : InertShowFault::InvalidInput;
        snapshot_.status = status;
    }

    void InertShowSimulator::updateDigest (
        TimePoint now,
        const InertChannelObservation (&canonical)[InertChannelAssessor::capacity],
        const CueOperatorInput& input) noexcept
    {
        digestByte (1);
        digestWord (now.milliseconds ());

        for (uint8_t index = 0; index < InertChannelAssessor::capacity; ++index)
        {
            digestByte (canonical[index].channel);
            digestByte (static_cast<uint8_t> (canonical[index].primary));
            digestByte (static_cast<uint8_t> (canonical[index].redundant));
            digestWord (canonical[index].observedAt.milliseconds ());
        }

        digestByte (static_cast<uint8_t> (input.reviewHeld));
        digestByte (static_cast<uint8_t> (input.runPressed));
        digestByte (static_cast<uint8_t> (input.confirmPressed));
        digestByte (static_cast<uint8_t> (input.skipPressed));
        digestByte (static_cast<uint8_t> (input.cancelPressed));
        digestByte (static_cast<uint8_t> (snapshot_.state));
        digestByte (static_cast<uint8_t> (snapshot_.fault));
        digestByte (static_cast<uint8_t> (snapshot_.schedule.phase));
        digestByte (static_cast<uint8_t> (snapshot_.schedule.decision));
        digestByte (static_cast<uint8_t> (snapshot_.schedule.hasCue));
        digestByte (snapshot_.schedule.cue);
        digestByte (snapshot_.schedule.cueIndex);
        digestWord (snapshot_.schedule.planElapsed.milliseconds ());
        digestWord (snapshot_.schedule.cueElapsed.milliseconds ());
        digestByte (static_cast<uint8_t> (snapshot_.schedule.status.error ()));
        digestByte (static_cast<uint8_t> (snapshot_.hasSelectedChannel));

        if (snapshot_.hasSelectedChannel)
        {
            digestByte (snapshot_.selectedChannel.channel);
            digestByte (static_cast<uint8_t> (snapshot_.selectedChannel.state));
            digestWord (snapshot_.selectedChannel.observedAt.milliseconds ());
        }

        digestWord (snapshot_.auditSequence);
        digestByte (static_cast<uint8_t> (snapshot_.status.error ()));
        snapshot_.traceDigest = traceDigest_;
    }

    void InertShowSimulator::digestByte (uint8_t value) noexcept
    {
        traceDigest_ ^= value;
        traceDigest_ *= fnvPrime;
    }

    void InertShowSimulator::digestWord (uint32_t value) noexcept
    {
        for (uint8_t shift = 0; shift < 32; shift = static_cast<uint8_t> (shift + 8))
        {
            digestByte (static_cast<uint8_t> (value >> shift));
        }
    }
} // namespace adk
