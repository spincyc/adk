#include "inert_cue_scheduler.h"

namespace adk {

    namespace {

        constexpr uint32_t maximumUnambiguousDuration = 0x7fffffffu;

        CueSchedulerSnapshot inertSnapshot () noexcept
        {
            return {CueSchedulerPhase::Idle,
                    CueDecision::Waiting,
                    0,
                    0,
                    Duration (),
                    Duration (),
                    StatusCode::NotInitialized};
        }
    } // namespace

    InertCueScheduler::InertCueScheduler (const InertCuePlan& plan,
                                          Duration            confirmationWindow,
                                          CueAuditBuffer&     audit) noexcept
        : plan_ (plan), confirmationWindow_ (confirmationWindow), audit_ (audit),
          snapshot_      (inertSnapshot ()), planStartedAt_ (), confirmationRequestedAt_ (),
          lastUpdatedAt_ (), planStarted_ (false), hasLastUpdate_ (false),
          initialized_   (false)
    {
    }

    InertCueScheduler::~InertCueScheduler () noexcept
    {
        shutdown ();
    }

    Status InertCueScheduler::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!audit_.initialized ())
        {
            return StatusCode::NotInitialized;
        }

        if (!validPlan () || confirmationWindow_.milliseconds () == 0 ||
            confirmationWindow_.milliseconds () > maximumUnambiguousDuration)
        {
            return StatusCode::InvalidArgument;
        }

        snapshot_                = inertSnapshot ();
        snapshot_.status         = StatusCode::Ok;
        planStartedAt_           = TimePoint ();
        confirmationRequestedAt_ = TimePoint ();
        lastUpdatedAt_           = TimePoint ();
        planStarted_             = false;
        hasLastUpdate_           = false;
        initialized_             = true;

        if (!append (TimePoint (), CueAuditEvent::Initialized))
        {
            initialized_ = false;
            snapshot_    = inertSnapshot ();
            return StatusCode::CapacityExceeded;
        }

        return StatusCode::Ok;
    }

    void InertCueScheduler::shutdown () noexcept
    {
        if (initialized_)
        {
            audit_.appendShutdown (hasLastUpdate_ ? lastUpdatedAt_ : TimePoint (),
                                   snapshot_.cue, snapshot_.cueIndex);
        }

        snapshot_      = inertSnapshot ();
        planStarted_   = false;
        hasLastUpdate_ = false;
        initialized_   = false;
    }

    bool InertCueScheduler::initialized () const noexcept
    {
        return initialized_;
    }

    Status InertCueScheduler::update (TimePoint               now,
                                      const CueOperatorInput& input) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (hasLastUpdate_ && now.elapsedSince (lastUpdatedAt_).milliseconds () >
                                  maximumUnambiguousDuration)
        {
            return enterFault (now, StatusCode::InvalidArgument);
        }

        lastUpdatedAt_ = now;
        hasLastUpdate_ = true;
        refreshElapsed (now);

        if (input.cancelPressed)
        {
            if (snapshot_.phase == CueSchedulerPhase::Cancelled)
            {
                return StatusCode::Ok;
            }

            if (!append (now, CueAuditEvent::Cancelled))
            {
                enterHeld (now, StatusCode::CapacityExceeded);
                return StatusCode::CapacityExceeded;
            }

            snapshot_.phase    = CueSchedulerPhase::Cancelled;
            snapshot_.decision = CueDecision::Cancelled;
            snapshot_.status   = StatusCode::Ok;
            return StatusCode::Ok;
        }

        if (snapshot_.phase == CueSchedulerPhase::Cancelled ||
            snapshot_.phase == CueSchedulerPhase::Complete ||
            snapshot_.phase == CueSchedulerPhase::Fault)
        {
            return StatusCode::Ok;
        }

        if (!input.reviewHeld && snapshot_.phase != CueSchedulerPhase::Idle &&
            snapshot_.phase != CueSchedulerPhase::Held)
        {
            if (!append (now, CueAuditEvent::Held))
            {
                enterHeld (now, StatusCode::CapacityExceeded);
                return StatusCode::CapacityExceeded;
            }

            snapshot_.phase    = CueSchedulerPhase::Held;
            snapshot_.decision = CueDecision::Held;
            snapshot_.status   = StatusCode::Ok;
            planStarted_       = false;
            return StatusCode::Ok;
        }

        if (hasChord (input))
        {
            return enterFault (now, StatusCode::InvalidArgument);
        }

        return process (now, input);
    }

    CueSchedulerSnapshot InertCueScheduler::snapshot () const noexcept
    {
        return snapshot_;
    }

    bool InertCueScheduler::validPlan () const noexcept
    {
        if (plan_.count == 0 || plan_.count > InertCuePlan::capacity ||
            plan_.cues[0].offset.milliseconds () != 0)
        {
            return false;
        }

        uint32_t previousEnd = 0;

        for (uint8_t index = 0; index < plan_.count; ++index)
        {
            const InertCue& cue     = plan_.cues[index];
            const uint32_t  offset  = cue.offset.milliseconds     ();
            const uint32_t  visible = cue.visibleFor.milliseconds ();

            if (cue.id >= InertCuePlan::capacity || visible == 0 ||
                offset > maximumUnambiguousDuration ||
                visible > maximumUnambiguousDuration - offset ||
                (index > 0 && offset < previousEnd))
            {
                return false;
            }

            previousEnd = offset + visible;
        }

        return previousEnd <= maximumUnambiguousDuration;
    }

    bool InertCueScheduler::append (TimePoint now, CueAuditEvent event,
                                    Status status) noexcept
    {
        return audit_
            .appendOperational (now, event, snapshot_.cue, snapshot_.cueIndex, status)
            .ok                ();
    }

    void InertCueScheduler::enterHeld (TimePoint now, Status status) noexcept
    {
        audit_.appendCapacityHold (now, snapshot_.cue, snapshot_.cueIndex);
        snapshot_.phase    = CueSchedulerPhase::Held;
        snapshot_.decision = CueDecision::Held;
        snapshot_.status   = status;
        planStarted_       = false;
    }

    Status InertCueScheduler::enterFault (TimePoint now, Status status) noexcept
    {
        if (!append (now, CueAuditEvent::Faulted, status))
        {
            enterHeld (now, StatusCode::CapacityExceeded);
            return StatusCode::CapacityExceeded;
        }

        snapshot_.phase    = CueSchedulerPhase::Fault;
        snapshot_.decision = CueDecision::Held;
        snapshot_.status   = status;
        planStarted_       = false;
        return status;
    }

    Status InertCueScheduler::process (TimePoint               now,
                                       const CueOperatorInput& input) noexcept
    {
        switch (snapshot_.phase)
        {
            case CueSchedulerPhase::Idle: return processIdle       (now, input);
            case CueSchedulerPhase::Review: return processReview   (now, input);
            case CueSchedulerPhase::Waiting: return processWaiting (now, input);
            case CueSchedulerPhase::Confirmation:
                return processConfirmation (now, input);
            case CueSchedulerPhase::Active: return processActive (now, input);
            case CueSchedulerPhase::Held:
                if (input.runPressed || input.confirmPressed || input.skipPressed)
                {
                    return enterFault (now, StatusCode::InvalidArgument);
                }

                if (input.reviewHeld)
                {
                    if (!append (now, CueAuditEvent::Resumed))
                    {
                        enterHeld (now, StatusCode::CapacityExceeded);
                        return StatusCode::CapacityExceeded;
                    }

                    snapshot_.phase    = CueSchedulerPhase::Review;
                    snapshot_.decision = CueDecision::Waiting;
                    snapshot_.status   = StatusCode::Ok;
                }
                return StatusCode::Ok;
            case CueSchedulerPhase::Complete:
            case CueSchedulerPhase::Cancelled:
            case CueSchedulerPhase::Fault: return StatusCode::Ok;
        }

        return enterFault (now, StatusCode::HardwareFailure);
    }

    Status InertCueScheduler::processIdle (TimePoint               now,
                                           const CueOperatorInput& input) noexcept
    {
        if (input.runPressed || input.confirmPressed || input.skipPressed)
        {
            return enterFault (now, StatusCode::InvalidArgument);
        }

        if (input.reviewHeld)
        {
            if (!append (now, CueAuditEvent::ReviewStarted))
            {
                enterHeld (now, StatusCode::CapacityExceeded);
                return StatusCode::CapacityExceeded;
            }

            snapshot_.phase  = CueSchedulerPhase::Review;
            snapshot_.status = StatusCode::Ok;
        }

        return StatusCode::Ok;
    }

    Status InertCueScheduler::processReview (TimePoint               now,
                                             const CueOperatorInput& input) noexcept
    {
        if (input.confirmPressed || input.skipPressed)
        {
            return enterFault (now, StatusCode::InvalidArgument);
        }

        if (!input.runPressed)
        {
            return StatusCode::Ok;
        }

        if (!append (now, CueAuditEvent::RunRequested))
        {
            enterHeld (now, StatusCode::CapacityExceeded);
            return StatusCode::CapacityExceeded;
        }

        snapshot_.cue      = plan_.cues[0].id;
        snapshot_.cueIndex = 0;
        snapshot_.phase    = CueSchedulerPhase::Waiting;
        snapshot_.decision = CueDecision::Waiting;
        return StatusCode::Ok;
    }

    Status InertCueScheduler::processWaiting (TimePoint               now,
                                              const CueOperatorInput& input) noexcept
    {
        if (input.runPressed || input.confirmPressed || input.skipPressed)
        {
            return enterFault (now, StatusCode::InvalidArgument);
        }

        if (!planStarted_ ||
            snapshot_.planElapsed >= plan_.cues[snapshot_.cueIndex].offset)
        {
            const InertCue& cue = plan_.cues[snapshot_.cueIndex];

            if (planStarted_ &&
                snapshot_.planElapsed.milliseconds () >=
                    cue.offset.milliseconds () + cue.visibleFor.milliseconds ())
            {
                if (!append (now, CueAuditEvent::CueSkipped))
                {
                    enterHeld (now, StatusCode::CapacityExceeded);
                    return StatusCode::CapacityExceeded;
                }

                snapshot_.decision = CueDecision::Skipped;
                ++snapshot_.cueIndex;

                if (snapshot_.cueIndex >= plan_.count)
                {
                    snapshot_.phase    = CueSchedulerPhase::Complete;
                    snapshot_.decision = CueDecision::Complete;
                }
                else
                {
                    snapshot_.cue = plan_.cues[snapshot_.cueIndex].id;
                }

                return StatusCode::Ok;
            }

            if (!append (now, CueAuditEvent::ConfirmationRequested))
            {
                enterHeld (now, StatusCode::CapacityExceeded);
                return StatusCode::CapacityExceeded;
            }

            confirmationRequestedAt_ = now;
            snapshot_.phase          = CueSchedulerPhase::Confirmation;
            snapshot_.decision       = CueDecision::ConfirmationRequired;
        }

        return StatusCode::Ok;
    }

    Status
    InertCueScheduler::processConfirmation (TimePoint               now,
                                            const CueOperatorInput& input) noexcept
    {
        if (input.runPressed)
        {
            return enterFault (now, StatusCode::InvalidArgument);
        }

        const Duration  waiting = now.elapsedSince (confirmationRequestedAt_);
        const InertCue& cue     = plan_.cues[snapshot_.cueIndex];

        if (waiting > confirmationWindow_ ||
            (planStarted_ &&
             snapshot_.planElapsed.milliseconds () >=
                 cue.offset.milliseconds () + cue.visibleFor.milliseconds ()))
        {
            if (!append (now, CueAuditEvent::CueSkipped))
            {
                enterHeld (now, StatusCode::CapacityExceeded);
                return StatusCode::CapacityExceeded;
            }

            snapshot_.decision = CueDecision::Skipped;
            ++snapshot_.cueIndex;

            if (snapshot_.cueIndex >= plan_.count)
            {
                snapshot_.phase    = CueSchedulerPhase::Complete;
                snapshot_.decision = CueDecision::Complete;
            }
            else
            {
                snapshot_.cue   = plan_.cues[snapshot_.cueIndex].id;
                snapshot_.phase = CueSchedulerPhase::Waiting;
            }

            return StatusCode::Ok;
        }

        if (input.skipPressed)
        {
            if (!append (now, CueAuditEvent::CueSkipped))
            {
                enterHeld (now, StatusCode::CapacityExceeded);
                return StatusCode::CapacityExceeded;
            }

            snapshot_.decision = CueDecision::Skipped;
            ++snapshot_.cueIndex;

            if (snapshot_.cueIndex >= plan_.count)
            {
                snapshot_.phase    = CueSchedulerPhase::Complete;
                snapshot_.decision = CueDecision::Complete;
            }
            else
            {
                snapshot_.cue   = plan_.cues[snapshot_.cueIndex].id;
                snapshot_.phase = CueSchedulerPhase::Waiting;
            }

            return StatusCode::Ok;
        }

        if (!input.confirmPressed)
        {
            return StatusCode::Ok;
        }

        if (!audit_.canAppendOperational (2))
        {
            enterHeld (now, StatusCode::CapacityExceeded);
            return StatusCode::CapacityExceeded;
        }

        append (now, CueAuditEvent::Confirmed);
        append (now, CueAuditEvent::CueShown);

        if (!planStarted_)
        {
            planStartedAt_        = now;
            planStarted_          = true;
            snapshot_.planElapsed = Duration ();
        }

        snapshot_.phase    = CueSchedulerPhase::Active;
        snapshot_.decision = CueDecision::Active;
        snapshot_.cueElapsed =
            snapshot_.planElapsed.milliseconds () >=
                    plan_.cues[snapshot_.cueIndex].offset.milliseconds ()
                ? Duration (snapshot_.planElapsed.milliseconds () -
                            plan_.cues[snapshot_.cueIndex].offset.milliseconds ())
                : Duration ();
        return StatusCode::Ok;
    }

    Status InertCueScheduler::processActive (TimePoint               now,
                                             const CueOperatorInput& input) noexcept
    {
        if (input.runPressed || input.confirmPressed || input.skipPressed)
        {
            return enterFault (now, StatusCode::InvalidArgument);
        }

        const InertCue& cue = plan_.cues[snapshot_.cueIndex];

        if (snapshot_.planElapsed.milliseconds () <
            cue.offset.milliseconds () + cue.visibleFor.milliseconds ())
        {
            return StatusCode::Ok;
        }

        if (!append (now, CueAuditEvent::CueHidden))
        {
            enterHeld (now, StatusCode::CapacityExceeded);
            return StatusCode::CapacityExceeded;
        }

        ++snapshot_.cueIndex;

        if (snapshot_.cueIndex >= plan_.count)
        {
            snapshot_.phase    = CueSchedulerPhase::Complete;
            snapshot_.decision = CueDecision::Complete;
            return StatusCode::Ok;
        }

        snapshot_.cue        = plan_.cues[snapshot_.cueIndex].id;
        snapshot_.phase      = CueSchedulerPhase::Waiting;
        snapshot_.decision   = CueDecision::Waiting;
        snapshot_.cueElapsed = Duration ();
        return StatusCode::Ok;
    }

    void InertCueScheduler::refreshElapsed (TimePoint now) noexcept
    {
        if (planStarted_)
        {
            snapshot_.planElapsed = now.elapsedSince (planStartedAt_);

            if (snapshot_.phase == CueSchedulerPhase::Active)
            {
                const uint32_t offset =
                    plan_.cues[snapshot_.cueIndex].offset.milliseconds ();
                snapshot_.cueElapsed =
                    snapshot_.planElapsed.milliseconds () >= offset
                        ? Duration (snapshot_.planElapsed.milliseconds () - offset)
                        : Duration ();
            }
        }
    }

    bool InertCueScheduler::hasChord (const CueOperatorInput& input) const noexcept
    {
        uint8_t edgeCount = static_cast<uint8_t> (input.runPressed);

        edgeCount = static_cast<uint8_t> (
            edgeCount + static_cast<uint8_t> (input.confirmPressed));
        edgeCount = static_cast<uint8_t> (
            edgeCount + static_cast<uint8_t> (input.skipPressed));

        return edgeCount > 1;
    }
} // namespace adk
