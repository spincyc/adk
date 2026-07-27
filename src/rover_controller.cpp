#include "rover_controller.h"

namespace adk {

    static constexpr uint32_t maximumDuration = 0x7fffffffu;

    static RangeReading emptyRange () noexcept
    {
        const RangeReading reading = {
            RangeState::Idle,
            0,
            MicrosecondDuration (0),
            false};

        return reading;
    }

    RouteStep::RouteStep (RouteAction selectedAction,
                          uint16_t    selectedTargetEdges,
                          Duration    selectedMaximumDuration,
                          uint8_t     selectedDuty) noexcept
        : action          (selectedAction)
        , targetEdges     (selectedTargetEdges)
        , maximumDuration (selectedMaximumDuration)
        , duty            (selectedDuty)
    {
    }

    RoverWheelObservation::RoverWheelObservation (uint32_t observedEdges,
                                                  Status   observedStatus) noexcept
        : totalEdges (observedEdges)
        , status     (observedStatus)
    {
    }

    RoverInput::RoverInput (const RangeReading&          observedRange,
                            TimePoint                    observedRangeAt,
                            const RoverWheelObservation& observedLeftWheel,
                            const RoverWheelObservation& observedRightWheel,
                            bool                         observedStartEvent,
                            bool                         observedStopRequest,
                            TimePoint                    observationTime) noexcept
        : range           (observedRange)
        , rangeObservedAt (observedRangeAt)
        , leftWheel       (observedLeftWheel)
        , rightWheel      (observedRightWheel)
        , startEvent      (observedStartEvent)
        , stopRequested   (observedStopRequest)
        , observedAt      (observationTime)
    {
    }

    RoverController::RoverController (const RoverControllerConfig& config,
                                      const RouteStep*              route,
                                      uint8_t                       routeLength) noexcept
        : config_          (config)
        , route_           ()
        , routeLength_     (routeLength)
        , leftMotor_       ()
        , rightMotor_      ()
        , mode_            (RoverMode::Inactive)
        , stopReason_      (RoverStopReason::None)
        , status_          (StatusCode::NotInitialized)
        , range_           (emptyRange ())
        , rangeObservedAt_ ()
        , leftWheel_       ()
        , rightWheel_      ()
        , modeSince_       ()
        , lastUpdate_      ()
        , mismatchSince_   ()
        , leftStepStart_   (0)
        , rightStepStart_  (0)
        , leftStepEdges_   (0)
        , rightStepEdges_  (0)
        , transitionCount_ (0)
        , routeIndex_      (0)
        , initialized_     (false)
        , hasLastUpdate_   (false)
        , mismatchActive_  (false)
        , routeAdvanced_   (false)
        , stopDominated_   (false)
    {
        if (route != nullptr && routeLength <= routeCapacity)
        {
            for (uint8_t index = 0; index < routeLength; ++index)
            {
                route_[index] = route[index];
            }
        }
    }

    RoverController::~RoverController () noexcept
    {
        shutdown ();
    }

    Status RoverController::initialize () noexcept
    {
        if (initialized_)
        {
            return status_;
        }

        shutdown ();

        if (!configValid () || !routeValid ())
        {
            status_ = StatusCode::InvalidArgument;
            mode_   = RoverMode::Inactive;
            return status_;
        }

        initialized_     = true;
        hasLastUpdate_   = false;
        mismatchActive_  = false;
        routeAdvanced_   = false;
        stopDominated_   = false;
        mode_            = RoverMode::Ready;
        stopReason_      = RoverStopReason::None;
        status_          = StatusCode::Ok;
        routeIndex_      = 0;
        leftStepStart_   = 0;
        rightStepStart_  = 0;
        leftStepEdges_   = 0;
        rightStepEdges_  = 0;
        transitionCount_ = 0;

        return status_;
    }

    void RoverController::shutdown () noexcept
    {
        commandStop ();
        initialized_     = false;
        hasLastUpdate_   = false;
        mismatchActive_  = false;
        routeAdvanced_   = false;
        stopDominated_   = true;
        mode_            = RoverMode::Inactive;
        stopReason_      = RoverStopReason::None;
        status_          = StatusCode::NotInitialized;
        routeIndex_      = 0;
        leftStepEdges_   = 0;
        rightStepEdges_  = 0;
    }

    Status RoverController::update (const RoverInput& input) noexcept
    {
        routeAdvanced_ = false;
        stopDominated_ = false;

        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        const bool stationary = wheelsStationary (input);

        if (!wheelsProgressValid (input))
        {
            observe  (input);
            stopWith (RoverMode::MotionFault,
                      RoverStopReason::EncoderFault,
                      StatusCode::HardwareFailure,
                      input.observedAt,
                      true);
            return status_;
        }

        observe (input);

        if (input.stopRequested)
        {
            stopWith (RoverMode::Stopped,
                      RoverStopReason::StopRequested,
                      StatusCode::Ok,
                      input.observedAt,
                      true);
            return status_;
        }

        if (!timeValid (input.observedAt))
        {
            stopWith (RoverMode::MotionFault,
                      RoverStopReason::InvalidTime,
                      StatusCode::InvalidArgument,
                      input.observedAt,
                      true);
            return status_;
        }

        lastUpdate_    = input.observedAt;
        hasLastUpdate_ = true;

        if (!input.leftWheel.status.ok () || !input.rightWheel.status.ok ())
        {
            stopWith (RoverMode::MotionFault,
                      RoverStopReason::EncoderFault,
                      StatusCode::HardwareFailure,
                      input.observedAt,
                      true);
            return status_;
        }

        if (!rangeUsable (input))
        {
            const RoverStopReason reason = rangeFresh (input)
                                               ? RoverStopReason::RangeInvalid
                                               : RoverStopReason::RangeStale;

            stopWith (RoverMode::SensorFault,
                      reason,
                      StatusCode::HardwareFailure,
                      input.observedAt,
                      true);
            return status_;
        }

        if ((mode_ == RoverMode::Ready ||
             mode_ == RoverMode::Stopped ||
             mode_ == RoverMode::Paused ||
             mode_ == RoverMode::ObstacleHold ||
             mode_ == RoverMode::RouteComplete ||
             mode_ == RoverMode::SensorFault ||
             mode_ == RoverMode::MotionFault) &&
            !stationary)
        {
            stopWith (RoverMode::MotionFault,
                      RoverStopReason::UnexpectedMotion,
                      StatusCode::HardwareFailure,
                      input.observedAt,
                      true);
            return status_;
        }

        if ((mode_ == RoverMode::Stopped ||
             mode_ == RoverMode::SensorFault ||
             mode_ == RoverMode::MotionFault) &&
            input.startEvent && stationary)
        {
            resetRoute (input);
            beginStep  (input.observedAt);
            return status_;
        }

        if (mode_ == RoverMode::Ready)
        {
            commandStop ();
            status_ = StatusCode::Ok;

            if (input.startEvent)
            {
                resetRoute (input);
                beginStep  (input.observedAt);
            }

            return status_;
        }

        if (mode_ == RoverMode::RouteComplete)
        {
            commandStop ();
            status_ = StatusCode::Ok;
            return status_;
        }

        if (input.range.distanceMm <= config_.minimumClearanceMm)
        {
            stopWith (RoverMode::ObstacleHold,
                      RoverStopReason::None,
                      StatusCode::Ok,
                      input.observedAt,
                      true);
            return status_;
        }

        if (mode_ == RoverMode::ObstacleHold)
        {
            commandStop ();

            if (input.range.distanceMm < config_.resumeClearanceMm)
            {
                return status_;
            }

            leftStepStart_  = input.leftWheel.totalEdges;
            rightStepStart_ = input.rightWheel.totalEdges;
            modeSince_      = input.observedAt;
            beginStep (input.observedAt);
            return status_;
        }

        if (mode_ == RoverMode::Paused)
        {
            commandStop ();

            if (stepTimedOut (input.observedAt))
            {
                advanceRoute (input.observedAt);
            }

            return status_;
        }

        if (stepTimedOut (input.observedAt))
        {
            stopWith (RoverMode::MotionFault,
                      RoverStopReason::RouteTimeout,
                      StatusCode::HardwareFailure,
                      input.observedAt,
                      true);
            return status_;
        }

        if (stepComplete ())
        {
            advanceRoute (input.observedAt);
            return status_;
        }

        if (leftStepEdges_ == 0 && rightStepEdges_ == 0 &&
            motionTimedOut (input.observedAt))
        {
            stopWith (RoverMode::MotionFault,
                      RoverStopReason::MotionDidNotStart,
                      StatusCode::HardwareFailure,
                      input.observedAt,
                      true);
            return status_;
        }

        if (wheelMismatch ())
        {
            if (!mismatchActive_)
            {
                mismatchActive_ = true;
                mismatchSince_  = input.observedAt;
            } else if (input.observedAt.elapsedSince (mismatchSince_) >=
                       config_.wheelMismatchTimeout)
            {
                stopWith (RoverMode::MotionFault,
                          RoverStopReason::WheelMismatch,
                          StatusCode::HardwareFailure,
                          input.observedAt,
                          true);
                return status_;
            }
        } else
        {
            mismatchActive_ = false;
        }

        commandForStep ();
        status_ = StatusCode::Ok;
        return status_;
    }

    bool RoverController::initialized () const noexcept
    {
        return initialized_;
    }

    RoverControllerSnapshot RoverController::snapshot () const noexcept
    {
        Duration duration (0);

        if (initialized_ && routeIndex_ < routeLength_)
        {
            duration = route_[routeIndex_].maximumDuration;
        }

        const bool hasDeadline = initialized_ &&
                                 (mode_ == RoverMode::Driving ||
                                  mode_ == RoverMode::Turning ||
                                  mode_ == RoverMode::Paused);
        const RoverControllerSnapshot result = {
            leftMotor_,
            rightMotor_,
            mode_,
            stopReason_,
            status_,
            range_,
            rangeObservedAt_,
            leftWheel_,
            rightWheel_,
            modeSince_,
            TimePoint (modeSince_.milliseconds () + duration.milliseconds ()),
            leftStepEdges_,
            rightStepEdges_,
            routeIndex_,
            hasDeadline,
            routeAdvanced_,
            stopDominated_,
            transitionCount_};

        return result;
    }

    bool RoverController::configValid () const noexcept
    {
        return config_.minimumClearanceMm               < config_.resumeClearanceMm &&
               config_.rangeMaximumAge.milliseconds      () > 0 &&
               config_.rangeMaximumAge.milliseconds      () <= maximumDuration &&
               config_.motionStartTimeout.milliseconds   () > 0 &&
               config_.motionStartTimeout.milliseconds   () <= maximumDuration &&
               config_.wheelMismatchTimeout.milliseconds () > 0 &&
               config_.wheelMismatchTimeout.milliseconds () <= maximumDuration &&
               config_.maximumMotorDuty > 0;
    }

    bool RoverController::routeValid () const noexcept
    {
        if (routeLength_ == 0 || routeLength_ > routeCapacity)
        {
            return false;
        }

        for (uint8_t index = 0; index < routeLength_; ++index)
        {
            const RouteStep& step = route_[index];

            if (step.action == RouteAction::Finish)
            {
                if (index + 1 != routeLength_ ||
                    step.targetEdges != 0 ||
                    step.maximumDuration.milliseconds () != 0 ||
                    step.duty != 0)
                {
                    return false;
                }

                continue;
            }

            if (step.maximumDuration.milliseconds () == 0 ||
                step.maximumDuration.milliseconds () > maximumDuration)
            {
                return false;
            }

            if (step.action == RouteAction::Pause)
            {
                if (step.targetEdges != 0 || step.duty != 0)
                {
                    return false;
                }
            } else if (step.targetEdges == 0 ||
                       step.duty == 0 ||
                       step.duty > config_.maximumMotorDuty)
            {
                return false;
            }
        }

        return route_[routeLength_ - 1].action == RouteAction::Finish;
    }

    bool RoverController::timeValid (TimePoint now) const noexcept
    {
        return !hasLastUpdate_ ||
               now.elapsedSince (lastUpdate_).milliseconds () <= maximumDuration;
    }

    bool RoverController::rangeFresh (const RoverInput& input) const noexcept
    {
        return input.observedAt.elapsedSince (input.rangeObservedAt) <=
               config_.rangeMaximumAge;
    }

    bool RoverController::rangeUsable (const RoverInput& input) const noexcept
    {
        return input.range.state == RangeState::Valid &&
               input.range.valid &&
               rangeFresh (input);
    }

    bool RoverController::wheelsStationary (const RoverInput& input) const noexcept
    {
        return !hasLastUpdate_ ||
               (input.leftWheel.totalEdges == leftWheel_.totalEdges &&
                input.rightWheel.totalEdges == rightWheel_.totalEdges);
    }

    bool RoverController::wheelsProgressValid (const RoverInput& input) const noexcept
    {
        return !hasLastUpdate_ ||
               (input.leftWheel.totalEdges - leftWheel_.totalEdges <= maximumDuration &&
                input.rightWheel.totalEdges - rightWheel_.totalEdges <= maximumDuration);
    }

    bool RoverController::stepTimedOut (TimePoint now) const noexcept
    {
        return now.elapsedSince (modeSince_) >=
               route_[routeIndex_].maximumDuration;
    }

    bool RoverController::motionTimedOut (TimePoint now) const noexcept
    {
        return now.elapsedSince (modeSince_) >= config_.motionStartTimeout;
    }

    bool RoverController::stepComplete () const noexcept
    {
        const uint16_t target = route_[routeIndex_].targetEdges;

        return leftStepEdges_ >= target && rightStepEdges_ >= target;
    }

    bool RoverController::wheelMismatch () const noexcept
    {
        const uint32_t difference = leftStepEdges_ >= rightStepEdges_
                                        ? leftStepEdges_ - rightStepEdges_
                                        : rightStepEdges_ - leftStepEdges_;

        return difference > config_.maximumWheelEdgeDifference;
    }

    void RoverController::resetRoute (const RoverInput& input) noexcept
    {
        routeIndex_      = 0;
        leftStepStart_   = input.leftWheel.totalEdges;
        rightStepStart_  = input.rightWheel.totalEdges;
        leftStepEdges_   = 0;
        rightStepEdges_  = 0;
        mismatchActive_  = false;
        stopReason_      = RoverStopReason::None;
        status_          = StatusCode::Ok;
        routeAdvanced_   = false;
        stopDominated_   = false;
    }

    void RoverController::beginStep (TimePoint now) noexcept
    {
        const RouteStep& step = route_[routeIndex_];

        modeSince_ = now;

        switch (step.action)
        {
            case RouteAction::Drive:     enterMode (RoverMode::Driving, now); break;
            case RouteAction::TurnLeft:
            case RouteAction::TurnRight: enterMode (RoverMode::Turning, now); break;
            case RouteAction::Pause:     enterMode (RoverMode::Paused, now); break;
            case RouteAction::Finish:
                enterMode (RoverMode::RouteComplete, now);
                break;
        }

        commandForStep ();
    }

    void RoverController::advanceRoute (TimePoint now) noexcept
    {
        if (routeIndex_ + 1 < routeLength_)
        {
            ++routeIndex_;
        }

        routeAdvanced_  = true;
        leftStepStart_  = leftWheel_.totalEdges;
        rightStepStart_ = rightWheel_.totalEdges;
        leftStepEdges_  = 0;
        rightStepEdges_ = 0;
        mismatchActive_ = false;
        beginStep (now);
    }

    void RoverController::commandForStep () noexcept
    {
        const RouteStep& step = route_[routeIndex_];

        switch (step.action)
        {
            case RouteAction::Drive:
                leftMotor_  = MotorCommand (MotorDirection::Forward, step.duty);
                rightMotor_ = MotorCommand (MotorDirection::Forward, step.duty);
                break;

            case RouteAction::TurnLeft:
                leftMotor_  = MotorCommand (MotorDirection::Reverse, step.duty);
                rightMotor_ = MotorCommand (MotorDirection::Forward, step.duty);
                break;

            case RouteAction::TurnRight:
                leftMotor_  = MotorCommand (MotorDirection::Forward, step.duty);
                rightMotor_ = MotorCommand (MotorDirection::Reverse, step.duty);
                break;

            case RouteAction::Pause:
            case RouteAction::Finish:
                commandStop ();
                break;
        }
    }

    void RoverController::stopWith (RoverMode       mode,
                                    RoverStopReason reason,
                                    Status          status,
                                    TimePoint       now,
                                    bool            dominated) noexcept
    {
        commandStop ();
        stopReason_    = reason;
        status_        = status;
        stopDominated_ = dominated;
        enterMode (mode, now);
    }

    void RoverController::enterMode (RoverMode mode, TimePoint now) noexcept
    {
        if (mode_ != mode)
        {
            ++transitionCount_;
        }

        mode_      = mode;
        modeSince_ = now;
    }

    void RoverController::commandStop () noexcept
    {
        leftMotor_  = MotorCommand ();
        rightMotor_ = MotorCommand ();
    }

    void RoverController::observe (const RoverInput& input) noexcept
    {
        range_           = input.range;
        rangeObservedAt_ = input.rangeObservedAt;

        if (hasLastUpdate_)
        {
            leftStepEdges_  = input.leftWheel.totalEdges - leftStepStart_;
            rightStepEdges_ = input.rightWheel.totalEdges - rightStepStart_;
        }

        leftWheel_  = input.leftWheel;
        rightWheel_ = input.rightWheel;
    }
}
