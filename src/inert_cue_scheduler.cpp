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
                    StatusCode::NotInitialized,
                    false};
        }
    } // namespace

    InertCueScheduler::InertCueScheduler (const InertCueSchedulerConfig& config,
                                          CueAuditBuffer&                audit) noexcept
        : plan_ (config.plan), confirmationWindow_ (config.confirmationWindow),
          audit_ (audit), snapshot_ (inertSnapshot ()), planStartedAt_ (),

          cueShownAt_ (), lastUpdatedAt_ (), lastInput_{}, planStarted_ (false),

          hasLastUpdate_ (false), initialized_ (false)
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

        if (audit_.initialized ())
        {
            return StatusCode::ResourceBusy;
        }

        if (!validPlan () || confirmationWindow_.milliseconds () == 0 ||
            confirmationWindow_.milliseconds () > maximumUnambiguousDuration)
        {
            return StatusCode::InvalidConfiguration;
        }

        const Status auditStatus = audit_.initialize ();

        if (!auditStatus.ok ())
        {
            return auditStatus;
        }

        snapshot_        = inertSnapshot ();
        snapshot_.status = StatusCode::Ok;
        planStartedAt_   = TimePoint ();

        cueShownAt_      = TimePoint ();

        lastUpdatedAt_   = TimePoint ();
        lastInput_       = {};
        planStarted_     = false;
        hasLastUpdate_   = false;
        initialized_     = true;

        if (!append (TimePoint (), CueAuditEvent::Initialized))
        {
            initialized_ = false;
            snapshot_    = inertSnapshot ();

            audit_.shutdown ();
            return StatusCode::CapacityExceeded;
        }

        return StatusCode::Ok;
    }

    void InertCueScheduler::shutdown () noexcept
    {
        if (initialized_)
        {
            audit_.appendShutdown (hasLastUpdate_ ? lastUpdatedAt_ : TimePoint ());
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

            lastUpdatedAt_       = now;
            lastInput_           = input;
            hasLastUpdate_       = true;
            snapshot_.phase      = CueSchedulerPhase::Cancelled;
            snapshot_.decision   = CueDecision::Cancelled;
            snapshot_.status     = StatusCode::Ok;
            snapshot_.hasCue     = false;
            snapshot_.cueElapsed = Duration ();
            planStarted_         = false;
            return StatusCode::Ok;
        }

        if (snapshot_.phase == CueSchedulerPhase::Cancelled ||
            snapshot_.phase == CueSchedulerPhase::Complete ||
            snapshot_.phase == CueSchedulerPhase::Fault)
        {
            return StatusCode::Ok;
        }

        if (hasLastUpdate_ && now == lastUpdatedAt_)
        {
            if (sameInput (input, lastInput_))
            {
                return StatusCode::Ok;
            }

            return enterFault (now, StatusCode::InvalidArgument);
        }

        if (hasLastUpdate_ && now.elapsedSince (lastUpdatedAt_).milliseconds () >
                                  maximumUnambiguousDuration)
        {
            return enterFault (now, StatusCode::InvalidArgument);
        }

        lastUpdatedAt_ = now;
        lastInput_     = input;
        hasLastUpdate_ = true;
        refreshElapsed (now);

        if (hasChord (input))
        {
            return enterFault (now, StatusCode::InvalidArgument);
        }

        if (!input.reviewHeld && snapshot_.phase != CueSchedulerPhase::Idle &&
            snapshot_.phase != CueSchedulerPhase::Held)
        {
            if (!append (now, CueAuditEvent::Held))
            {
                enterHeld (now, StatusCode::CapacityExceeded);
                return StatusCode::CapacityExceeded;
            }

            if (snapshot_.phase == CueSchedulerPhase::Active)
            {
                ++snapshot_.cueIndex;
            }

            snapshot_.phase      = CueSchedulerPhase::Held;
            snapshot_.decision   = CueDecision::Held;
            snapshot_.status     = StatusCode::Ok;
            snapshot_.hasCue     = false;
            snapshot_.cueElapsed = Duration ();
            planStarted_         = false;
            return StatusCode::Ok;
        }

        return process (now, input);
    }

    CueSchedulerSnapshot InertCueScheduler::snapshot () const noexcept
    {
        return snapshot_;
    }

    bool InertCueScheduler::validPlan () const noexcept
    {
        if (plan_.count == 0 || plan_.count > InertCuePlan::capacity)
        {
            return false;
        }

        uint32_t previousOffset = 0;
        uint32_t previousEnd    = 0;

        for (uint8_t index = 0; index < plan_.count; ++index)
        {
            const InertCue& cue     = plan_.cues[index];
            const uint32_t  offset  = cue.offset.milliseconds ();

            const uint32_t  visible = cue.visibleFor.milliseconds ();

            if (cue.id >= InertCuePlan::capacity || visible == 0 ||
                offset > maximumUnambiguousDuration ||
                visible > maximumUnambiguousDuration ||
                visible > maximumUnambiguousDuration - offset ||
                (index > 0 && (offset <= previousOffset || offset < previousEnd)))
            {
                return false;
            }

            for (uint8_t earlier = 0; earlier < index; ++earlier)
            {
                if (plan_.cues[earlier].id == cue.id)
                {
                    return false;
                }
            }

            previousOffset = offset;
            previousEnd    = offset + visible;
        }

        return true;
    }

    bool InertCueScheduler::append (TimePoint now, CueAuditEvent event, Status status,
                                    bool hasCue, uint8_t cueIndex) noexcept
    {
        const InertCueId cue =
            hasCue && cueIndex < plan_.count ? plan_.cues[cueIndex].id : 0;

        return audit_.appendOperational (now, event, cue, cueIndex, status, hasCue)
            .ok ();
    }

    void InertCueScheduler::enterHeld (TimePoint now, Status status) noexcept
    {
        audit_.appendCapacityHold (now, snapshot_.cue, snapshot_.cueIndex);
        snapshot_.phase      = CueSchedulerPhase::Held;
        snapshot_.decision   = CueDecision::Held;
        snapshot_.status     = status;
        snapshot_.hasCue     = false;
        snapshot_.cueElapsed = Duration ();
        planStarted_         = false;
    }

    Status InertCueScheduler::enterFault (TimePoint now, Status status) noexcept
    {
        if (!append (now, CueAuditEvent::Faulted, status))
        {
            enterHeld (now, StatusCode::CapacityExceeded);
            return StatusCode::CapacityExceeded;
        }

        snapshot_.phase      = CueSchedulerPhase::Fault;
        snapshot_.decision   = CueDecision::Held;
        snapshot_.status     = status;
        snapshot_.hasCue     = false;
        snapshot_.cueElapsed = Duration ();
        planStarted_         = false;
        return status;
    }

    Status InertCueScheduler::process (TimePoint               now,
                                       const CueOperatorInput& input) noexcept
    {
        switch (snapshot_.phase)
        {
            case CueSchedulerPhase::Idle: return processIdle (now, input);

            case CueSchedulerPhase::Review: return processReview (now, input);

            case CueSchedulerPhase::Waiting: return processWaiting (now, input);
            case CueSchedulerPhase::Confirmation:
                return processConfirmation (now, input);
            case CueSchedulerPhase::Active: return processActive (now, input);
            case CueSchedulerPhase::Held:
                if (input.confirmPressed || input.skipPressed)
                {
                    return enterFault (now, StatusCode::InvalidArgument);
                }

                if (!input.runPressed)
                {
                    return StatusCode::Ok;
                }

                if (!input.reviewHeld)
                {
                    return enterFault (now, StatusCode::InvalidArgument);
                }

                if (snapshot_.cueIndex >= plan_.count)
                {
                    if (!audit_.canAppendOperational (2))
                    {
                        enterHeld (now, StatusCode::CapacityExceeded);
                        return StatusCode::CapacityExceeded;
                    }

                    append (now, CueAuditEvent::Resumed);

                    append (now, CueAuditEvent::Completed);
                    snapshot_.phase    = CueSchedulerPhase::Complete;
                    snapshot_.decision = CueDecision::Complete;
                    snapshot_.status   = StatusCode::Ok;
                    snapshot_.hasCue   = false;
                    return StatusCode::Ok;
                }

                if (!audit_.canAppendOperational (2))
                {
                    enterHeld (now, StatusCode::CapacityExceeded);
                    return StatusCode::CapacityExceeded;
                }

                append (now, CueAuditEvent::Resumed);

                append (now, CueAuditEvent::ConfirmationRequested, StatusCode::Ok, true,
                        snapshot_.cueIndex);
                planStartedAt_ =
                    TimePoint (now.milliseconds () -
                               plan_.cues[snapshot_.cueIndex].offset.milliseconds ());
                planStarted_          = true;
                snapshot_.planElapsed = plan_.cues[snapshot_.cueIndex].offset;
                snapshot_.cue         = plan_.cues[snapshot_.cueIndex].id;
                snapshot_.phase       = CueSchedulerPhase::Confirmation;
                snapshot_.decision    = CueDecision::ConfirmationRequired;
                snapshot_.status      = StatusCode::Ok;
                snapshot_.hasCue      = true;
                return StatusCode::Ok;
            case CueSchedulerPhase::Complete:
            case CueSchedulerPhase::Cancelled:
            case CueSchedulerPhase::Fault: return StatusCode::Ok;
        }

        return enterFault (now, StatusCode::InternalInvariant);
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

        planStartedAt_                     = now;
        planStarted_                       = true;
        snapshot_.planElapsed              = Duration ();
        snapshot_.cue                      = plan_.cues[0].id;
        snapshot_.cueIndex                 = 0;
        snapshot_.phase                    = CueSchedulerPhase::Waiting;
        snapshot_.decision                 = CueDecision::Waiting;
        snapshot_.hasCue                   = true;
        const CueOperatorInput elapsedOnly = {true, false, false, false, false};
        return processWaiting (now, elapsedOnly);
    }

    Status InertCueScheduler::processWaiting (TimePoint               now,
                                              const CueOperatorInput& input) noexcept
    {
        if (input.runPressed || input.confirmPressed || input.skipPressed)
        {
            return enterFault (now, StatusCode::InvalidArgument);
        }

        const InertCue& cue = plan_.cues[snapshot_.cueIndex];

        if (snapshot_.planElapsed < cue.offset)
        {
            return StatusCode::Ok;
        }

        if (snapshot_.planElapsed.milliseconds () - cue.offset.milliseconds () >
            confirmationWindow_.milliseconds ())
        {
            return finishCue (now, CueAuditEvent::CueSkipped, StatusCode::Timeout);
        }

        if (!append (now, CueAuditEvent::ConfirmationRequested, StatusCode::Ok, true,
                     snapshot_.cueIndex))
        {
            enterHeld (now, StatusCode::CapacityExceeded);
            return StatusCode::CapacityExceeded;
        }

        snapshot_.phase    = CueSchedulerPhase::Confirmation;
        snapshot_.decision = CueDecision::ConfirmationRequired;
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

        const InertCue& cue = plan_.cues[snapshot_.cueIndex];

        if (snapshot_.planElapsed.milliseconds () - cue.offset.milliseconds () >
            confirmationWindow_.milliseconds ())
        {
            return finishCue (now, CueAuditEvent::CueSkipped, StatusCode::Timeout);
        }

        if (input.skipPressed)
        {
            return finishCue (now, CueAuditEvent::CueSkipped, StatusCode::Ok);
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

        append (now, CueAuditEvent::Confirmed, StatusCode::Ok, true,
                snapshot_.cueIndex);
        append (now, CueAuditEvent::CueShown, StatusCode::Ok, true, snapshot_.cueIndex);
        cueShownAt_          = now;
        snapshot_.phase      = CueSchedulerPhase::Active;
        snapshot_.decision   = CueDecision::Active;
        snapshot_.cueElapsed = Duration ();
        return StatusCode::Ok;
    }

    Status InertCueScheduler::processActive (TimePoint               now,
                                             const CueOperatorInput& input) noexcept
    {
        if (input.runPressed || input.confirmPressed || input.skipPressed)
        {
            return enterFault (now, StatusCode::InvalidArgument);
        }

        if (snapshot_.cueElapsed < plan_.cues[snapshot_.cueIndex].visibleFor)
        {
            return StatusCode::Ok;
        }

        return finishCue (now, CueAuditEvent::CueHidden, StatusCode::Ok);
    }

    Status InertCueScheduler::finishCue (TimePoint now, CueAuditEvent firstEvent,
                                         Status firstStatus) noexcept
    {
        const uint8_t firstIndex = snapshot_.cueIndex;
        uint8_t       nextIndex  = static_cast<uint8_t> (firstIndex + 1U);
        uint8_t       expired    = 0;

        while (nextIndex < plan_.count &&
               snapshot_.planElapsed >= plan_.cues[nextIndex].offset &&
               snapshot_.planElapsed.milliseconds () -
                       plan_.cues[nextIndex].offset.milliseconds () >
                   confirmationWindow_.milliseconds ())
        {
            ++expired;
            ++nextIndex;
        }

        const bool complete = nextIndex >= plan_.count;
        const bool request =
            !complete && snapshot_.planElapsed >= plan_.cues[nextIndex].offset;
        const uint8_t recordCount = static_cast<uint8_t> (
            1U + expired + static_cast<uint8_t> (complete || request));

        if (!audit_.canAppendOperational (recordCount))
        {
            enterHeld (now, StatusCode::CapacityExceeded);
            return StatusCode::CapacityExceeded;
        }

        append (now, firstEvent, firstStatus, true, firstIndex);

        for (uint8_t index = static_cast<uint8_t> (firstIndex + 1U); index < nextIndex;
             ++index)
        {
            append (now, CueAuditEvent::CueSkipped, StatusCode::Timeout, true, index);
        }

        snapshot_.cueIndex   = nextIndex;
        snapshot_.cueElapsed = Duration ();

        if (complete)
        {
            append (now, CueAuditEvent::Completed);
            snapshot_.phase    = CueSchedulerPhase::Complete;
            snapshot_.decision = CueDecision::Complete;
            snapshot_.status   = StatusCode::Ok;
            snapshot_.hasCue   = false;
            return StatusCode::Ok;
        }

        snapshot_.cue    = plan_.cues[nextIndex].id;
        snapshot_.hasCue = true;
        snapshot_.status = StatusCode::Ok;

        if (request)
        {
            append (now, CueAuditEvent::ConfirmationRequested, StatusCode::Ok, true,
                    nextIndex);
            snapshot_.phase    = CueSchedulerPhase::Confirmation;
            snapshot_.decision = CueDecision::ConfirmationRequired;
        }
        else
        {
            snapshot_.phase    = CueSchedulerPhase::Waiting;
            snapshot_.decision = CueDecision::Waiting;
        }

        return StatusCode::Ok;
    }

    void InertCueScheduler::refreshElapsed (TimePoint now) noexcept
    {
        if (planStarted_)
        {
            snapshot_.planElapsed = now.elapsedSince (planStartedAt_);
        }

        if (snapshot_.phase == CueSchedulerPhase::Active)
        {
            snapshot_.cueElapsed = now.elapsedSince (cueShownAt_);
        }
        else
        {
            snapshot_.cueElapsed = Duration ();
        }
    }

    bool InertCueScheduler::hasChord (const CueOperatorInput& input) const noexcept
    {
        uint8_t edgeCount = static_cast<uint8_t> (input.runPressed);

        edgeCount = static_cast<uint8_t> (edgeCount +
                                          static_cast<uint8_t> (input.confirmPressed));
        edgeCount =
            static_cast<uint8_t> (edgeCount + static_cast<uint8_t> (input.skipPressed));

        return edgeCount > 1;
    }

    bool InertCueScheduler::sameInput (const CueOperatorInput& left,
                                       const CueOperatorInput& right) const noexcept
    {
        return left.reviewHeld == right.reviewHeld &&
               left.runPressed == right.runPressed &&
               left.confirmPressed == right.confirmPressed &&
               left.skipPressed == right.skipPressed &&
               left.cancelPressed == right.cancelPressed;
    }
} // namespace adk
