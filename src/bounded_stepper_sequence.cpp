#include "bounded_stepper_sequence.h"

#include <limits.h>

namespace adk {
    namespace {
        constexpr uint32_t halfRange = UINT32_C (0x80000000);
        constexpr uint8_t  frames[]  = {0x01U, 0x03U, 0x02U, 0x06U,
                                        0x04U, 0x0cU, 0x08U, 0x09U};

        StepperCommand emptyCommand () noexcept
        {
            return {0,           TimePoint (), StepDirection::Stopped, 0,
                    Duration (), false,        StatusCode::Ok};
        }

        StepperSequenceSnapshot emptySnapshot (Status status) noexcept
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

        bool validStatus (Status status) noexcept
        {
            return status.error () >= StatusCode::Ok &&
                   status.error () <= StatusCode::HardwareFailure;
        }

        bool validDirection (StepDirection direction) noexcept
        {
            return direction >= StepDirection::Stopped &&
                   direction <= StepDirection::Reverse;
        }

        bool forwardOrEqual (TimePoint later, TimePoint earlier) noexcept
        {
            return later.elapsedSince (earlier).milliseconds () < halfRange;
        }

        bool commandEqual (const StepperCommand& left,
                           const StepperCommand& right) noexcept
        {
            return left.commandId == right.commandId &&
                   left.issuedAt == right.issuedAt &&
                   left.direction == right.direction &&
                   left.stepCount == right.stepCount &&
                   left.stepInterval == right.stepInterval &&
                   left.cancel == right.cancel && left.status == right.status;
        }

        TimePoint add (TimePoint point, Duration duration) noexcept
        {
            return TimePoint (point.milliseconds () + duration.milliseconds ());
        }

        bool due (TimePoint now, TimePoint deadline) noexcept
        {
            return now.elapsedSince (deadline).milliseconds () < halfRange;
        }

        uint8_t nextIndex (uint8_t index, StepDirection direction) noexcept
        {
            return direction == StepDirection::Forward
                       ? static_cast<uint8_t> ((index + 1U) & 7U)
                       : static_cast<uint8_t> ((index + 7U) & 7U);
        }
    } // namespace

    StepperSequenceConfig::StepperSequenceConfig (Duration minimumStepInterval,
                                                  Duration maximumStepInterval,
                                                  Duration maximumCommandAge,
                                                  int32_t  minimumLogicalPosition,
                                                  int32_t  maximumLogicalPosition,
                                                  bool     holdAtRest) noexcept
        // clang-format off
        : minimumStepInterval     (minimumStepInterval),
          maximumStepInterval     (maximumStepInterval),
          maximumCommandAge       (maximumCommandAge),
          minimumLogicalPosition  (minimumLogicalPosition),
          maximumLogicalPosition  (maximumLogicalPosition), holdAtRest (holdAtRest)
    // clang-format on
    {
    }

    StepperSequencePreview::StepperSequencePreview () noexcept
        // clang-format off
        : snapshot_      (emptySnapshot (StatusCode::NotInitialized)),
          command_       (emptyCommand ()),
          owner_         (nullptr),
          lastUpdateAt_  (),
          generation_    (0),
          phaseIndex_    (0),
          hasCommand_    (false),
          hasIdentity_   (false),
          hasLastUpdate_ (false)
    // clang-format on
    {
    }

    BoundedStepperSequence::BoundedStepperSequence (
        const StepperSequenceConfig& config) noexcept
        // clang-format off
        : config_        (config),
          snapshot_      (emptySnapshot (StatusCode::NotInitialized)),
          command_       (emptyCommand ()),
          lastUpdateAt_  (),
          generation_    (0),
          phaseIndex_    (0),
          hasCommand_    (false),
          hasIdentity_   (false),
          hasLastUpdate_ (false),
          initialized_   (false)
    // clang-format on
    {
    }

    Status BoundedStepperSequence::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const Duration minimum = config_.minimumStepInterval;
        const Duration maximum = config_.maximumStepInterval;
        const Duration age     = config_.maximumCommandAge;

        // clang-format off
        const uint32_t minimumInterval = minimum.milliseconds ();

        const uint32_t maximumInterval = maximum.milliseconds ();

        const uint32_t maximumAge      = age.milliseconds ();
        // clang-format on
        if (minimumInterval == 0 || minimumInterval >= halfRange ||
            maximumInterval == 0 || maximumInterval >= halfRange || maximumAge == 0 ||
            maximumAge >= halfRange || minimumInterval > maximumInterval ||
            config_.minimumLogicalPosition >= config_.maximumLogicalPosition ||
            config_.minimumLogicalPosition > 0 || config_.maximumLogicalPosition < 0)
        {
            snapshot_ = emptySnapshot (StatusCode::InvalidConfiguration);
            return snapshot_.status;
        }

        const int64_t range = static_cast<int64_t> (config_.maximumLogicalPosition) -
                              static_cast<int64_t> (config_.minimumLogicalPosition);
        if (range <= 0 || range > UINT32_MAX)
        {
            snapshot_ = emptySnapshot (StatusCode::InvalidConfiguration);
            return snapshot_.status;
        }

        initialized_ = true;
        reset ();
        return StatusCode::Ok;
    }

    void BoundedStepperSequence::reset () noexcept
    {
        snapshot_ =
            emptySnapshot (initialized_ ? StatusCode::Ok : StatusCode::NotInitialized);
        command_ = emptyCommand ();

        lastUpdateAt_  = TimePoint ();
        phaseIndex_    = 0;
        hasCommand_    = false;
        hasIdentity_   = false;
        hasLastUpdate_ = false;
        ++generation_;
    }

    Status
    BoundedStepperSequence::preview (TimePoint now, const StepperCommand& command,
                                     StepperSequencePreview& candidate) const noexcept
    {
        candidate.owner_         = nullptr;
        candidate.generation_    = generation_;
        candidate.snapshot_      = snapshot_;
        candidate.command_       = command_;
        candidate.lastUpdateAt_  = lastUpdateAt_;
        candidate.phaseIndex_    = phaseIndex_;
        candidate.hasCommand_    = hasCommand_;
        candidate.hasIdentity_   = hasIdentity_;
        candidate.hasLastUpdate_ = hasLastUpdate_;

        if (!initialized_)
        {
            candidate.snapshot_ = emptySnapshot (StatusCode::NotInitialized);
            return candidate.snapshot_.status;
        }
        const bool issuedTimeValid = forwardOrEqual (now, command.issuedAt);
        const bool updateTimeValid =
            !hasLastUpdate_ || forwardOrEqual (now, lastUpdateAt_);
        if (!validDirection (command.direction) || !validStatus (command.status) ||
            command.commandId == 0 ||
            (command.direction == StepDirection::Stopped && command.stepCount != 0) ||
            (command.direction != StepDirection::Stopped && command.stepCount == 0) ||
            command.stepInterval < config_.minimumStepInterval ||
            command.stepInterval > config_.maximumStepInterval || !issuedTimeValid ||
            !updateTimeValid)
        {
            return StatusCode::InvalidArgument;
        }
        candidate.owner_         = this;
        candidate.lastUpdateAt_  = now;
        candidate.hasLastUpdate_ = true;

        const bool live   = hasCommand_ && snapshot_.phase == StepSequencePhase::Moving;
        const bool sameId = hasIdentity_ && command.commandId == command_.commandId;
        if (sameId && !commandEqual (command, command_))
        {
            candidate.owner_ = nullptr;
            return StatusCode::InvalidArgument;
        }
        if (sameId && !live)
        {
            return candidate.snapshot_.status;
        }
        if (snapshot_.phase == StepSequencePhase::Fault)
        {
            candidate.owner_ = nullptr;
            return StatusCode::InvalidArgument;
        }

        bool newCommand = !sameId;
        if (newCommand && hasIdentity_)
        {
            const uint32_t delta = command.commandId - command_.commandId;
            if (delta == 0 || delta >= halfRange)
            {
                candidate.owner_ = nullptr;
                return StatusCode::InvalidArgument;
            }
        }

        const int64_t direction = command.direction == StepDirection::Reverse ? -1 : 1;
        const int64_t endpoint  = static_cast<int64_t> (snapshot_.logicalPosition) +
                                  direction * static_cast<int64_t> (command.stepCount);
        if (newCommand && (endpoint < config_.minimumLogicalPosition ||
                           endpoint > config_.maximumLogicalPosition))
        {
            candidate.owner_ = nullptr;
            return StatusCode::InvalidArgument;
        }

        if (command.cancel)
        {
            if (!live)
            {
                candidate.owner_ = nullptr;
                return StatusCode::InvalidArgument;
            }
            candidate.snapshot_.commandId   = command.commandId;
            candidate.snapshot_.phase       = StepSequencePhase::Cancelled;
            candidate.snapshot_.disposition = StepSequenceDisposition::Cancelled;
            candidate.snapshot_.direction   = StepDirection::Stopped;
            candidate.snapshot_.coilIntent  = 0;
            candidate.snapshot_.phaseSince  = now;
            candidate.snapshot_.hasDeadline = false;
            candidate.snapshot_.status      = StatusCode::Ok;
            candidate.command_              = command;
            candidate.hasCommand_           = false;
            candidate.hasIdentity_          = true;
            return StatusCode::Ok;
        }

        if (!command.status.ok ())
        {
            candidate.snapshot_.commandId      = command.commandId;
            candidate.snapshot_.phase          = StepSequencePhase::Fault;
            candidate.snapshot_.disposition    = StepSequenceDisposition::Rejected;
            candidate.snapshot_.direction      = StepDirection::Stopped;
            candidate.snapshot_.requestedSteps = command.stepCount;
            candidate.snapshot_.completedSteps = 0;
            candidate.snapshot_.coilIntent     = 0;
            candidate.snapshot_.phaseSince     = now;
            candidate.snapshot_.nextStepAt     = TimePoint ();
            candidate.snapshot_.hasDeadline    = false;
            candidate.snapshot_.status         = command.status;
            candidate.command_                 = command;
            candidate.hasCommand_              = false;
            candidate.hasIdentity_             = true;
            return command.status;
        }

        if (newCommand)
        {
            candidate.snapshot_.commandId   = command.commandId;
            candidate.snapshot_.disposition = live ? StepSequenceDisposition::Replaced
                                                   : StepSequenceDisposition::Accepted;
            candidate.snapshot_.direction   = command.direction;
            candidate.snapshot_.requestedSteps = command.stepCount;
            candidate.snapshot_.completedSteps = 0;
            candidate.snapshot_.phaseSince     = now;
            candidate.snapshot_.status         = StatusCode::Ok;
            candidate.command_                 = command;
            candidate.hasCommand_  = command.direction != StepDirection::Stopped;
            candidate.hasIdentity_ = true;

            if (command.direction == StepDirection::Stopped)
            {
                candidate.snapshot_.phase = StepSequencePhase::Holding;
                candidate.snapshot_.coilIntent =
                    config_.holdAtRest ? snapshot_.coilIntent : 0;
                candidate.snapshot_.hasDeadline = false;
                return StatusCode::Ok;
            }

            candidate.snapshot_.phase = StepSequencePhase::Moving;
            candidate.snapshot_.nextStepAt =
                add (command.issuedAt, command.stepInterval);
            candidate.snapshot_.hasDeadline = true;
        }

        if (candidate.hasCommand_ &&
            now.elapsedSince (candidate.command_.issuedAt) > config_.maximumCommandAge)
        {
            candidate.snapshot_.phase       = StepSequencePhase::Fault;
            candidate.snapshot_.direction   = StepDirection::Stopped;
            candidate.snapshot_.coilIntent  = 0;
            candidate.snapshot_.phaseSince  = now;
            candidate.snapshot_.hasDeadline = false;
            candidate.snapshot_.status      = StatusCode::Timeout;
            candidate.hasCommand_           = false;
            return StatusCode::Timeout;
        }

        if (candidate.hasCommand_ && candidate.snapshot_.hasDeadline &&
            due (now, candidate.snapshot_.nextStepAt))
        {
            if (candidate.snapshot_.completedSteps == 0 && snapshot_.coilIntent == 0)
            {
                candidate.phaseIndex_ =
                    candidate.snapshot_.direction == StepDirection::Forward ? 0 : 7;
            }
            else
            {
                candidate.phaseIndex_ =
                    nextIndex (candidate.phaseIndex_, candidate.snapshot_.direction);
            }
            candidate.snapshot_.coilIntent = frames[candidate.phaseIndex_];
            candidate.snapshot_.logicalPosition +=
                candidate.snapshot_.direction == StepDirection::Forward ? 1 : -1;
            ++candidate.snapshot_.completedSteps;
            candidate.snapshot_.nextStepAt =
                add (candidate.snapshot_.nextStepAt, candidate.command_.stepInterval);

            if (candidate.snapshot_.completedSteps ==
                candidate.snapshot_.requestedSteps)
            {
                candidate.snapshot_.phase       = StepSequencePhase::Complete;
                candidate.snapshot_.direction   = StepDirection::Stopped;
                candidate.snapshot_.phaseSince  = now;
                candidate.snapshot_.hasDeadline = false;
                candidate.hasCommand_           = false;
                if (!config_.holdAtRest)
                {
                    candidate.snapshot_.coilIntent = 0;
                }
            }
        }
        return StatusCode::Ok;
    }

    bool BoundedStepperSequence::canCommit (
        const StepperSequencePreview& candidate) const noexcept
    {
        return initialized_ && candidate.owner_ == this &&
               candidate.generation_ == generation_;
    }

    Status
    BoundedStepperSequence::commit (const StepperSequencePreview& candidate) noexcept
    {
        if (!canCommit (candidate))
        {
            return initialized_ ? StatusCode::InvalidArgument
                                : StatusCode::NotInitialized;
        }
        snapshot_      = candidate.snapshot_;
        command_       = candidate.command_;
        lastUpdateAt_  = candidate.lastUpdateAt_;
        phaseIndex_    = candidate.phaseIndex_;
        hasCommand_    = candidate.hasCommand_;
        hasIdentity_   = candidate.hasIdentity_;
        hasLastUpdate_ = candidate.hasLastUpdate_;
        ++generation_;
        return StatusCode::Ok;
    }

    Status BoundedStepperSequence::stop (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (hasLastUpdate_ && !forwardOrEqual (now, lastUpdateAt_))
        {
            return StatusCode::InvalidArgument;
        }
        if (snapshot_.phase != StepSequencePhase::Cancelled &&
            snapshot_.phase != StepSequencePhase::Fault)
        {
            snapshot_.phase       = StepSequencePhase::Cancelled;
            snapshot_.disposition = StepSequenceDisposition::Cancelled;
            snapshot_.direction   = StepDirection::Stopped;
            snapshot_.coilIntent  = 0;
            snapshot_.phaseSince  = now;
            snapshot_.hasDeadline = false;
            snapshot_.status      = StatusCode::Ok;
        }
        hasCommand_    = false;
        lastUpdateAt_  = now;
        hasLastUpdate_ = true;
        ++generation_;
        return StatusCode::Ok;
    }

    bool BoundedStepperSequence::initialized () const noexcept
    {
        return initialized_;
    }

    StepperSequenceSnapshot BoundedStepperSequence::snapshot () const noexcept
    {
        return snapshot_;
    }
} // namespace adk
