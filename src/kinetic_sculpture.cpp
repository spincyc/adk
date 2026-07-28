#include "kinetic_sculpture.h"

namespace adk {
    // clang-format off
    namespace {
        constexpr uint32_t halfRange = UINT32_C (0x80000000);

        InteractionSource emptySource () noexcept
        {
            return {InteractionSourceKind::SyntheticFixture, 0, 0};
        }

        AuthorizationRecord emptyAuthorization () noexcept
        {
            return {0,
                    emptySource (),
                    emptySource (),
                    0,
                    0,
                    InteractionDirection::Neutral,
                    AuthorizationDisposition::None,
                    StatusCode::Ok};
        }

        InteractionIntent emptyInteraction (Status status) noexcept
        {
            return {emptySource (),
                    emptySource (),
                    TimePoint   (),
                    0,
                    0,
                    InteractionDirection::Neutral,
                    0,
                    false,
                    false,
                    false,
                    false,
                    InteractionQuality::Invalid,
                    Duration (),
                    Duration (),
                    false,
                    ContactQuality::Unqualified,
                    status,
                    status,
                    status};
        }

        StepperSequenceSnapshot emptyMotion (Status status) noexcept
        {
            return {0,
                    StepSequencePhase::Inactive,
                    StepSequenceDisposition::None,
                    StepDirection::Stopped,
                    0,
                    0,
                    0,
                    0,
                    TimePoint (),
                    TimePoint (),
                    false,
                    status};
        }

        SculptureLightIntent emptyLights () noexcept
        {
            return {0, false, false, false, false, false};
        }

        SculptureSnapshot emptySnapshot (Status status) noexcept
        {
            return {0,
                    SculpturePhase::Inactive,
                    emptyInteraction (status),
                    emptyMotion      (status),
                    emptyLights      (),
                    emptySource      (),
                    TimePoint        (),
                    0,
                    false,
                    false,
                    false,
                    false,
                    emptyAuthorization (),
                    false,
                    emptyAuthorization (),
                    ContactQuality::Unqualified,
                    status,
                    status,
                    status,
                    0,
                    status};
        }

        StepperCommand emptyCommand () noexcept
        {
            return {0,           TimePoint (), StepDirection::Stopped, 0,
                    Duration (), false,        StatusCode::Ok};
        }

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
        }

        bool validKind (InteractionSourceKind kind) noexcept
        {
            return kind >= InteractionSourceKind::SyntheticFixture &&
                   kind <= InteractionSourceKind::CopiedJoystick;
        }

        bool validStopSource (const InteractionSource& source) noexcept
        {
            return validKind (source.kind) && source.sourceId != 0 &&
                   source.configurationRevision != 0 &&
                   source.kind != InteractionSourceKind::CopiedJoystick;
        }

        bool sameSource (const InteractionSource& left,
                         const InteractionSource& right) noexcept
        {
            return left.kind == right.kind && left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision;
        }

        bool validQuality (ContactQuality quality) noexcept
        {
            return quality >= ContactQuality::Unqualified &&
                   quality <= ContactQuality::TimingFault;
        }

        bool forwardOrRepeat (uint32_t current, uint32_t previous) noexcept
        {
            return current - previous < halfRange;
        }

        bool timeForwardOrRepeat (TimePoint current, TimePoint previous) noexcept
        {
            return current.elapsedSince (previous).milliseconds () < halfRange;
        }

        bool withinAge (TimePoint now, TimePoint observed, Duration maximum) noexcept
        {
            const Duration age = now.elapsedSince (observed);
            return age.milliseconds               () < halfRange && age <= maximum;
        }

        uint32_t separation (TimePoint left, TimePoint right) noexcept
        {
            const uint32_t forward = left.elapsedSince  (right).milliseconds ();
            const uint32_t reverse = right.elapsedSince (left).milliseconds ();
            return forward < reverse ? forward : reverse;
        }

        bool healthy (const InteractionIntent& interaction) noexcept
        {
            return interaction.quality == InteractionQuality::Current &&
                   interaction.status.ok ();
        }

        AuthorizationRecord authorizationFor (uint32_t                 frameId,
                                              const InteractionIntent& interaction,
                                              AuthorizationDisposition disposition,
                                              Status                   status) noexcept
        {
            return {frameId,
                    interaction.contactSource,
                    interaction.directionalSource,
                    interaction.contactSequence,
                    interaction.directionalSequence,
                    interaction.direction,
                    disposition,
                    status};
        }

        bool motifFor (InteractionDirection direction, StepDirection& stepDirection,
                       uint32_t& steps, Duration& interval) noexcept
        {
            switch (direction)
            {
                case InteractionDirection::North:
                    stepDirection = StepDirection::Forward;
                    steps         = 8;
                    interval      = Duration (120);
                    return true;
                case InteractionDirection::NorthEast:
                    stepDirection = StepDirection::Forward;
                    steps         = 12;
                    interval      = Duration (100);
                    return true;
                case InteractionDirection::East:
                    stepDirection = StepDirection::Forward;
                    steps         = 16;
                    interval      = Duration (80);
                    return true;
                case InteractionDirection::SouthEast:
                    stepDirection = StepDirection::Forward;
                    steps         = 12;
                    interval      = Duration (100);
                    return true;
                case InteractionDirection::South:
                    stepDirection = StepDirection::Reverse;
                    steps         = 8;
                    interval      = Duration (120);
                    return true;
                case InteractionDirection::SouthWest:
                    stepDirection = StepDirection::Reverse;
                    steps         = 12;
                    interval      = Duration (100);
                    return true;
                case InteractionDirection::West:
                    stepDirection = StepDirection::Reverse;
                    steps         = 16;
                    interval      = Duration (80);
                    return true;
                case InteractionDirection::NorthWest:
                    stepDirection = StepDirection::Reverse;
                    steps         = 12;
                    interval      = Duration (100);
                    return true;
                case InteractionDirection::Neutral: break;
            }
            return false;
        }

        SculpturePhase phaseFor (const StepperSequenceSnapshot& motion) noexcept
        {
            switch (motion.phase)
            {
                case StepSequencePhase::Moving: return SculpturePhase::Running;
                case StepSequencePhase::Complete: return SculpturePhase::Complete;
                case StepSequencePhase::Fault: return SculpturePhase::Fault;
                case StepSequencePhase::Cancelled: return SculpturePhase::Stopped;
                case StepSequencePhase::Inactive:
                case StepSequencePhase::Holding: return SculpturePhase::Ready;
            }
            return SculpturePhase::Fault;
        }
    } // namespace

    KineticLightSculpture::KineticLightSculpture (
        const InteractionIntentConfig& interactionConfig,
        const StepperSequenceConfig& sequenceConfig, Duration maximumFrameAge,
        Duration maximumSourceSkew) noexcept
        : interaction_          (interactionConfig),
          sequence_                           (sequenceConfig),
          maximumFrameAge_                    (maximumFrameAge),
          maximumSourceSkew_                  (maximumSourceSkew),
          snapshot_                           (emptySnapshot (StatusCode::NotInitialized)),
          motionCommand_                      (emptyCommand ()),
          lastProjectTime_                    (),
          pendingCreatedAt_                   (),
          lastStopObservedAt_                 (),
          lastStopSource_                     (emptySource ()),
          minimumLogicalPosition_             (sequenceConfig.minimumLogicalPosition),
          maximumLogicalPosition_             (sequenceConfig.maximumLogicalPosition),
          lastFrameId_                        (0),
          lastStopSequence_                   (0),
          nextCommandId_                      (1),
          terminalFrameId_                    (0),
          initialized_                        (false),
          hasProjectTime_                     (false),
          hasFrameIdentity_                   (false),
          hasStopEvidence_                    (false),
          hasMotionCommand_                   (false),
          lastStopActive_                     (false),
          lastStopQuality_                    (ContactQuality::Unqualified),
          lastStopStatus_                     (StatusCode::Ok),
          stoppedLatch_                       (false),
          faultLatch_                         (false)
    {
    }

    Status KineticLightSculpture::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }
        const uint32_t frameAge = maximumFrameAge_.milliseconds ();
        if (frameAge == 0 || frameAge > 1000 ||
            maximumSourceSkew_.milliseconds () > frameAge)
        {
            snapshot_ = emptySnapshot (StatusCode::InvalidConfiguration);
            return snapshot_.status;
        }

        const Status interactionStatus = interaction_.initialize ();
        if (!interactionStatus.ok                                ())
        {
            interaction_.reset        ();
            sequence_.reset           ();
            snapshot_ = emptySnapshot (interactionStatus);
            return interactionStatus;
        }
        const Status motionStatus = sequence_.initialize ();
        if (!motionStatus.ok                             ())
        {
            interaction_.reset        ();
            sequence_.reset           ();
            snapshot_ = emptySnapshot (motionStatus);
            return motionStatus;
        }

        initialized_           = true;
        snapshot_              = emptySnapshot (StatusCode::Ok);
        snapshot_.phase        = SculpturePhase::Ready;
        snapshot_.lights.ready = true;
        snapshot_.motion       = sequence_.snapshot ();
        motionCommand_         = emptyCommand       ();
        lastProjectTime_       = TimePoint          ();
        pendingCreatedAt_      = TimePoint          ();
        lastStopObservedAt_    = TimePoint          ();
        lastStopSource_        = emptySource        ();
        lastFrameId_           = 0;
        lastStopSequence_      = 0;
        nextCommandId_         = 1;
        terminalFrameId_       = 0;
        hasProjectTime_        = false;
        hasFrameIdentity_      = false;
        hasStopEvidence_       = false;
        hasMotionCommand_      = false;
        lastStopActive_        = false;
        lastStopQuality_       = ContactQuality::Unqualified;
        lastStopStatus_        = StatusCode::Ok;
        stoppedLatch_          = false;
        faultLatch_            = false;
        return StatusCode::Ok;
    }

    void KineticLightSculpture::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }
        if (snapshot_.hasPendingAuthorization)
        {
            snapshot_.pendingAuthorization.disposition =
                AuthorizationDisposition::Inhibited;
            snapshot_.pendingAuthorization.status  = StatusCode::NotInitialized;
            snapshot_.lastTerminalAuthorization    = snapshot_.pendingAuthorization;
            snapshot_.hasLastTerminalAuthorization = true;
            snapshot_.hasPendingAuthorization      = false;
        }
        if (hasProjectTime_)
        {
            sequence_.stop (lastProjectTime_);
        }
        sequence_.reset    ();
        interaction_.reset ();
        snapshot_.phase             = SculpturePhase::Inactive;
        snapshot_.interaction       = interaction_.snapshot ();
        snapshot_.motion            = sequence_.snapshot    ();
        snapshot_.lights            = emptyLights           ();
        snapshot_.status            = StatusCode::NotInitialized;
        snapshot_.interactionStatus = StatusCode::NotInitialized;
        snapshot_.motionStatus      = StatusCode::NotInitialized;
        motionCommand_              = emptyCommand ();
        hasMotionCommand_           = false;
        initialized_                = false;
        stoppedLatch_               = false;
        faultLatch_                 = false;
    }

    Status KineticLightSculpture::update (const SculptureInput& input) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        const bool stopSourceValid = validStopSource (input.stopSource);
        const bool stopShapeValid =
            stopSourceValid && input.stopSequence != 0 &&
            validQuality (input.stopQuality) && validStatus (input.stopStatus) &&
            (!hasProjectTime_ ||
             timeForwardOrRepeat (input.observedAt, lastProjectTime_)) &&
            withinAge (input.observedAt, input.stopObservedAt, maximumFrameAge_);
        const bool stopDomainChanged =
            hasStopEvidence_ && !sameSource (input.stopSource, lastStopSource_);
        const bool stopChronologyValid =
            !hasStopEvidence_ || stopDomainChanged ||
            (forwardOrRepeat (input.stopSequence, lastStopSequence_) &&
             timeForwardOrRepeat (input.stopObservedAt, lastStopObservedAt_));
        const bool stopRepeatChanged = hasStopEvidence_ && !stopDomainChanged &&
                                       input.stopSequence == lastStopSequence_ &&
                                       (input.stopObservedAt != lastStopObservedAt_ ||
                                        input.stopActive != lastStopActive_ ||
                                        input.stopQuality != lastStopQuality_ ||
                                        input.stopStatus != lastStopStatus_);
        const bool stopValid =
            stopShapeValid && stopChronologyValid && !stopRepeatChanged;

        const bool stopHealthy =
            input.stopQuality == ContactQuality::Valid && input.stopStatus.ok ();
        if (stopValid && stopHealthy && input.stopActive)
        {
            sequence_.stop (input.observedAt);
            if (snapshot_.hasPendingAuthorization)
            {
                snapshot_.pendingAuthorization.disposition =
                    AuthorizationDisposition::Inhibited;
                snapshot_.pendingAuthorization.status  = input.stopStatus;
                snapshot_.lastTerminalAuthorization    = snapshot_.pendingAuthorization;
                snapshot_.hasLastTerminalAuthorization = true;
                snapshot_.hasPendingAuthorization      = false;
                terminalFrameId_                       = input.frameId;
            }
            snapshot_.frameId         = input.frameId;
            snapshot_.phase           = SculpturePhase::Stopped;
            snapshot_.motion          = sequence_.snapshot ();
            snapshot_.lights          = emptyLights        ();
            snapshot_.lights.stopped  = true;
            snapshot_.stopSource      = input.stopSource;
            snapshot_.stopObservedAt  = input.stopObservedAt;
            snapshot_.stopSequence    = input.stopSequence;
            snapshot_.stopActive      = true;
            snapshot_.hasStopIdentity = true;
            snapshot_.stopQuality     = input.stopQuality;
            snapshot_.stopStatus      = input.stopStatus;
            snapshot_.motionStatus    = snapshot_.motion.status;
            snapshot_.interactionStatus =
                !validStatus (input.status)
                    ? Status            (StatusCode::InvalidArgument)
                    : (!input.status.ok ()
                           ? input.status
                           : (!validStatus (input.touchSample.status)
                                  ? Status                        (StatusCode::InvalidArgument)
                                  : (!input.touchSample.status.ok ()
                                         ? input.touchSample.status
                                         : (!validStatus (input.directional.status)
                                                ? Status (StatusCode::InvalidArgument)
                                                : input.directional.status))));
            snapshot_.status    = StatusCode::Ok;
            lastProjectTime_    = input.observedAt;
            lastStopObservedAt_ = input.stopObservedAt;
            lastStopSource_     = input.stopSource;
            lastStopSequence_   = input.stopSequence;
            hasProjectTime_     = true;
            hasStopEvidence_    = true;
            hasMotionCommand_   = false;
            lastStopActive_     = input.stopActive;
            lastStopQuality_    = input.stopQuality;
            lastStopStatus_     = input.stopStatus;
            stoppedLatch_       = true;
            return StatusCode::Ok;
        }

        if (!stopValid)
        {
            if (snapshot_.hasPendingAuthorization)
            {
                snapshot_.pendingAuthorization.disposition =
                    AuthorizationDisposition::Inhibited;
                snapshot_.pendingAuthorization.status  = StatusCode::InvalidArgument;
                snapshot_.lastTerminalAuthorization    = snapshot_.pendingAuthorization;
                snapshot_.hasLastTerminalAuthorization = true;
                snapshot_.hasPendingAuthorization      = false;
            }
            if (hasProjectTime_)
            {
                sequence_.stop (lastProjectTime_);
            }
            else
            {
                sequence_.reset ();
            }
            snapshot_.phase        = SculpturePhase::Fault;
            snapshot_.motion       = sequence_.snapshot ();
            snapshot_.lights       = emptyLights        ();
            snapshot_.lights.fault = true;
            snapshot_.stopStatus   = StatusCode::InvalidArgument;
            snapshot_.stopQuality  = ContactQuality::Unqualified;
            snapshot_.status       = StatusCode::InvalidArgument;
            snapshot_.motionStatus = snapshot_.motion.status;
            snapshot_.stopActive   = false;
            if (!hasStopEvidence_)
            {
                snapshot_.stopSource      = emptySource ();
                snapshot_.stopObservedAt  = TimePoint   ();
                snapshot_.stopSequence    = 0;
                snapshot_.hasStopIdentity = false;
            }
            hasMotionCommand_ = false;
            faultLatch_       = true;
            return StatusCode::InvalidArgument;
        }

        if (faultLatch_)
        {
            return snapshot_.status;
        }

        if (!stopHealthy)
        {
            if (snapshot_.hasPendingAuthorization)
            {
                snapshot_.pendingAuthorization.disposition =
                    AuthorizationDisposition::Inhibited;
                snapshot_.pendingAuthorization.status =
                    input.stopStatus.ok () ? Status (StatusCode::InvalidArgument)
                                           : input.stopStatus;
                snapshot_.lastTerminalAuthorization    = snapshot_.pendingAuthorization;
                snapshot_.hasLastTerminalAuthorization = true;
                snapshot_.hasPendingAuthorization      = false;
                terminalFrameId_                       = input.frameId;
            }
            if (hasProjectTime_)
            {
                sequence_.stop (lastProjectTime_);
            }
            else
            {
                sequence_.reset ();
            }
            snapshot_.frameId         = input.frameId;
            snapshot_.phase           = SculpturePhase::Fault;
            snapshot_.motion          = sequence_.snapshot ();
            snapshot_.lights          = emptyLights        ();
            snapshot_.lights.fault    = true;
            snapshot_.stopSource      = input.stopSource;
            snapshot_.stopObservedAt  = input.stopObservedAt;
            snapshot_.stopSequence    = input.stopSequence;
            snapshot_.stopActive      = input.stopActive;
            snapshot_.hasStopIdentity = true;
            snapshot_.stopQuality     = input.stopQuality;
            snapshot_.stopStatus      = input.stopStatus;
            snapshot_.motionStatus    = snapshot_.motion.status;
            snapshot_.status          = input.stopStatus.ok ()
                                            ? Status (StatusCode::InvalidArgument)
                                            : input.stopStatus;
            lastStopObservedAt_       = input.stopObservedAt;
            lastStopSource_           = input.stopSource;
            lastStopSequence_         = input.stopSequence;
            hasStopEvidence_          = true;
            hasMotionCommand_         = false;
            lastStopActive_           = input.stopActive;
            lastStopQuality_          = input.stopQuality;
            lastStopStatus_           = input.stopStatus;
            faultLatch_               = true;
            return snapshot_.status;
        }

        const bool frameDomainValid =
            input.frameId != 0 && validStatus      (input.status) &&
            (!hasFrameIdentity_ || forwardOrRepeat (input.frameId, lastFrameId_)) &&
            withinAge                              (input.observedAt, input.touchSample.observedAt,
                       maximumFrameAge_) &&
            withinAge (input.observedAt, input.directional.observedAt,
                       maximumFrameAge_);
        const bool skewValid =
            separation (input.stopObservedAt, input.touchSample.observedAt) <=
                maximumSourceSkew_.milliseconds () &&
            separation (input.stopObservedAt, input.directional.observedAt) <=
                maximumSourceSkew_.milliseconds () &&
            separation (input.touchSample.observedAt, input.directional.observedAt) <=
                maximumSourceSkew_.milliseconds ();
        if (!frameDomainValid || !skewValid)
        {
            return StatusCode::InvalidArgument;
        }

        InteractionIntentPreview interactionCandidate;
        const Status             interactionPreview = interaction_.preview (
            input.observedAt, input.touchSource, input.touchSequence, input.touchSample,
            input.directional, interactionCandidate);
        if (!interactionPreview.ok () || !interaction_.canCommit (interactionCandidate))
        {
            return interactionPreview.ok () ? StatusCode::InvalidArgument
                                            : interactionPreview;
        }

        const bool strictlyForward =
            !hasFrameIdentity_ || input.frameId != lastFrameId_;
        AuthorizationRecord consumed    = emptyAuthorization ();
        bool                hasConsumed = false;
        bool                travelLimit = false;

        StepperCommand sequenceCommand  = motionCommand_;
        bool           commandIsPending = false;
        if (strictlyForward && snapshot_.hasPendingAuthorization)
        {
            StepDirection stepDirection = StepDirection::Stopped;
            uint32_t      steps         = 0;
            Duration      interval;
            const bool    motif = motifFor (snapshot_.pendingAuthorization.direction,
                                            stepDirection, steps, interval);
            consumed            = snapshot_.pendingAuthorization;
            hasConsumed         = true;
            if (!motif ||
                input.observedAt.elapsedSince (pendingCreatedAt_) > maximumFrameAge_)
            {
                consumed.disposition = AuthorizationDisposition::Inhibited;
                consumed.status      = motif ? Status (StatusCode::Timeout)
                                             : Status (StatusCode::InvalidArgument);
            }
            else
            {
                sequenceCommand     = {nextCommandId_, input.observedAt, stepDirection,
                                       steps,          interval,         false,
                                       StatusCode::Ok};
                const int64_t delta = stepDirection == StepDirection::Forward
                                          ? static_cast<int64_t> (steps)
                                          : -static_cast<int64_t> (steps);
                const StepperSequenceSnapshot motion = sequence_.snapshot ();
                const int64_t                 endpoint =
                    static_cast<int64_t> (motion.logicalPosition) + delta;
                if (endpoint < minimumLogicalPosition_ ||
                    endpoint > maximumLogicalPosition_)
                {
                    consumed.disposition = AuthorizationDisposition::BoundRejected;
                    consumed.status      = StatusCode::CapacityExceeded;
                    travelLimit          = true;
                    sequenceCommand      = motionCommand_;
                }
                else
                {
                    commandIsPending = true;
                }
            }
        }

        if (!hasMotionCommand_ && !commandIsPending)
        {
            sequenceCommand = {nextCommandId_,         input.observedAt,
                               StepDirection::Stopped, 0,
                               Duration (60),          false,
                               StatusCode::Ok};
        }

        StepperSequencePreview sequenceCandidate;
        const Status           sequencePreview =
            sequence_.preview (input.observedAt, sequenceCommand, sequenceCandidate);
        if (commandIsPending && !sequence_.canCommit (sequenceCandidate))
        {
            consumed.disposition = AuthorizationDisposition::BoundRejected;
            consumed.status      = sequencePreview.ok ()
                                       ? Status (StatusCode::CapacityExceeded)
                                       : sequencePreview;
            travelLimit          = true;
            commandIsPending     = false;
            sequenceCommand      = motionCommand_;
            if (!hasMotionCommand_)
            {
                sequenceCommand = {nextCommandId_,         input.observedAt,
                                   StepDirection::Stopped, 0,
                                   Duration (60),          false,
                                   StatusCode::Ok};
            }
            sequence_.preview        (input.observedAt, sequenceCommand, sequenceCandidate);
            if (!sequence_.canCommit (sequenceCandidate))
            {
                return StatusCode::InvalidArgument;
            }
        }
        else if (!sequence_.canCommit (sequenceCandidate))
        {
            return sequencePreview.ok () ? StatusCode::InvalidArgument
                                         : sequencePreview;
        }

        interaction_.commit                                         (interactionCandidate);
        const InteractionIntent interaction = interaction_.snapshot ();
        snapshot_.interaction               = interaction;
        snapshot_.interactionStatus         = interaction.status;

        if (!healthy (interaction) || !input.status.ok ())
        {
            sequence_.stop (input.observedAt);
            if (snapshot_.hasPendingAuthorization)
            {
                consumed             = snapshot_.pendingAuthorization;
                hasConsumed          = true;
                consumed.disposition = AuthorizationDisposition::Inhibited;
                consumed.status      = !input.status.ok ()
                                           ? input.status
                                           : (interaction.status.ok ()
                                                  ? Status (StatusCode::InvalidArgument)
                                                  : interaction.status);
            }
            snapshot_.hasPendingAuthorization = false;
            snapshot_.phase =
                input.status.ok () &&
                        (interaction.quality == InteractionQuality::Stale ||
                         interaction.quality == InteractionQuality::StuckActive)
                    ? SculpturePhase::Stopped
                    : SculpturePhase::Fault;
            snapshot_.motion         = sequence_.snapshot ();
            snapshot_.lights         = emptyLights        ();
            snapshot_.lights.fault   = snapshot_.phase == SculpturePhase::Fault;
            snapshot_.lights.stopped = snapshot_.phase == SculpturePhase::Stopped;
            snapshot_.status  = input.status.ok () ? interaction.status : input.status;
            hasMotionCommand_ = false;
            if (snapshot_.phase == SculpturePhase::Fault)
            {
                faultLatch_ = true;
            }
        }
        else
        {
            const Status motionCommit = sequence_.commit (sequenceCandidate);
            if (!motionCommit.ok                         ())
            {
                sequence_.stop (input.observedAt);
                snapshot_.phase        = SculpturePhase::Fault;
                snapshot_.lights       = emptyLights ();
                snapshot_.lights.fault = true;
                snapshot_.status       = StatusCode::InternalInvariant;
                return snapshot_.status;
            }
            snapshot_.motion       = sequence_.snapshot ();
            snapshot_.motionStatus = snapshot_.motion.status;
            if (commandIsPending)
            {
                consumed.disposition = AuthorizationDisposition::Accepted;
                consumed.status      = StatusCode::Ok;
                ++snapshot_.acceptedMotifCount;
                ++nextCommandId_;
                if (nextCommandId_ == 0)
                {
                    nextCommandId_ = 1;
                }
                motionCommand_    = sequenceCommand;
                hasMotionCommand_ = true;
            }
            else if (!hasMotionCommand_)
            {
                motionCommand_    = sequenceCommand;
                hasMotionCommand_ = true;
                ++nextCommandId_;
                if (nextCommandId_ == 0)
                {
                    nextCommandId_ = 1;
                }
            }
            snapshot_.phase                    = phaseFor    (snapshot_.motion);
            snapshot_.lights                   = emptyLights ();
            snapshot_.lights.shiftRegisterBits = snapshot_.motion.coilIntent;
            snapshot_.lights.ready       = snapshot_.phase == SculpturePhase::Ready ||
                                           snapshot_.phase == SculpturePhase::Complete;
            snapshot_.lights.running     = snapshot_.phase == SculpturePhase::Running;
            snapshot_.lights.travelLimit = travelLimit;
            snapshot_.status             = snapshot_.motion.status;
        }

        if (hasConsumed)
        {
            snapshot_.lastTerminalAuthorization    = consumed;
            snapshot_.hasLastTerminalAuthorization = true;
            snapshot_.hasPendingAuthorization      = false;
            terminalFrameId_                       = input.frameId;
        }
        else if (snapshot_.hasLastTerminalAuthorization && strictlyForward &&
                 input.frameId != terminalFrameId_)
        {
            snapshot_.lastTerminalAuthorization    = emptyAuthorization ();
            snapshot_.hasLastTerminalAuthorization = false;
        }

        if (healthy (interaction) && input.status.ok () && interaction.touchEvent &&
            strictlyForward && !input.stopActive)
        {
            snapshot_.pendingAuthorization =
                authorizationFor (input.frameId, interaction,
                                  AuthorizationDisposition::Pending, StatusCode::Ok);
            snapshot_.hasPendingAuthorization = true;
            pendingCreatedAt_                 = input.observedAt;
            snapshot_.phase                   = SculpturePhase::Preview;
            snapshot_.lights.ready            = false;
            stoppedLatch_                     = false;
        }
        else if (stoppedLatch_)
        {
            sequence_.stop                                (input.observedAt);
            snapshot_.motion         = sequence_.snapshot ();
            snapshot_.phase          = SculpturePhase::Stopped;
            snapshot_.lights         = emptyLights ();
            snapshot_.lights.stopped = true;
            hasMotionCommand_        = false;
        }

        snapshot_.frameId         = input.frameId;
        snapshot_.stopSource      = input.stopSource;
        snapshot_.stopObservedAt  = input.stopObservedAt;
        snapshot_.stopSequence    = input.stopSequence;
        snapshot_.stopActive      = false;
        snapshot_.hasStopIdentity = true;
        snapshot_.travelLimit     = travelLimit;
        snapshot_.stopQuality     = input.stopQuality;
        snapshot_.stopStatus      = input.stopStatus;
        snapshot_.motion          = sequence_.snapshot ();
        snapshot_.motionStatus    = snapshot_.motion.status;

        lastProjectTime_    = input.observedAt;
        lastStopObservedAt_ = input.stopObservedAt;
        lastStopSource_     = input.stopSource;
        lastFrameId_        = input.frameId;
        lastStopSequence_   = input.stopSequence;
        hasProjectTime_     = true;
        hasFrameIdentity_   = true;
        hasStopEvidence_    = true;
        lastStopActive_     = input.stopActive;
        lastStopQuality_    = input.stopQuality;
        lastStopStatus_     = input.stopStatus;
        return snapshot_.status;
    }

    bool KineticLightSculpture::initialized () const noexcept
    {
        return initialized_;
    }

    SculptureSnapshot KineticLightSculpture::snapshot () const noexcept
    {
        return snapshot_;
    }
    // clang-format on
} // namespace adk
