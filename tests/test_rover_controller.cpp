#include "rover_controller.h"

#include <cstdlib>
#include <iostream>

namespace {

    using namespace adk;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (1);
        }
    }

    RangeReading validRange (uint16_t distanceMm)
    {
        return {
            RangeState::Valid,
            distanceMm,
            MicrosecondDuration (static_cast<uint32_t> (distanceMm) * 6u),
            true};
    }

    RoverControllerConfig testConfig ()
    {
        RoverControllerConfig config;

        config.minimumClearanceMm         = 200;
        config.resumeClearanceMm          = 250;
        config.rangeMaximumAge            = Duration (20);
        config.motionStartTimeout         = Duration (5);
        config.wheelMismatchTimeout       = Duration (4);
        config.maximumWheelEdgeDifference = 2;
        config.maximumMotorDuty           = 200;

        return config;
    }

    RoverInput inputAt (uint32_t   now,
                        uint16_t   rangeMm,
                        uint32_t   leftEdges,
                        uint32_t   rightEdges,
                        bool       startEvent    = false,
                        bool       stopRequested = false,
                        Status     leftStatus    = Status (),
                        Status     rightStatus   = Status ())
    {
        return RoverInput (
            validRange             (rangeMm),
            TimePoint              (now),
            RoverWheelObservation  (leftEdges, leftStatus),
            RoverWheelObservation  (rightEdges, rightStatus),
            startEvent,
            stopRequested,
            TimePoint              (now));
    }

    void requireStopped (const RoverControllerSnapshot& snapshot,
                         const char*                    message)
    {
        require (snapshot.leftMotor.direction == MotorDirection::Stopped, message);
        require (snapshot.leftMotor.duty == 0, message);
        require (snapshot.rightMotor.direction == MotorDirection::Stopped, message);
        require (snapshot.rightMotor.duty == 0, message);
    }

    void requireDriving (const RoverControllerSnapshot& snapshot,
                         MotorDirection                 left,
                         MotorDirection                 right,
                         uint8_t                        duty,
                         const char*                    message)
    {
        require (snapshot.leftMotor.direction == left, message);
        require (snapshot.leftMotor.duty == duty, message);
        require (snapshot.rightMotor.direction == right, message);
        require (snapshot.rightMotor.duty == duty, message);
    }

    struct DifferentialDrivePlant
    {
        explicit DifferentialDrivePlant (uint16_t edgesPerSecond,
                                         uint8_t  leftGain  = 100,
                                         uint8_t  rightGain = 100) noexcept
            : edgesPerSecond_ (edgesPerSecond)
            , leftGain_       (leftGain)
            , rightGain_      (rightGain)
            , leftEdges_      (0)
            , rightEdges_     (0)
            , leftRemainder_  (0)
            , rightRemainder_ (0)
        {
        }

        void advance (const RoverControllerSnapshot& snapshot,
                      Duration                       elapsed) noexcept
        {
            leftEdges_ += edgeDelta (snapshot.leftMotor,
                                     elapsed,
                                     leftGain_,
                                     leftRemainder_);
            rightEdges_ += edgeDelta (snapshot.rightMotor,
                                      elapsed,
                                      rightGain_,
                                      rightRemainder_);
        }

        RoverWheelObservation leftWheel () const noexcept
        {
            return RoverWheelObservation (leftEdges_);
        }

        RoverWheelObservation rightWheel () const noexcept
        {
            return RoverWheelObservation (rightEdges_);
        }

      private:
        uint32_t edgeDelta (const MotorCommand& command,
                            Duration            elapsed,
                            uint8_t             gain,
                            uint32_t&            remainder) noexcept
        {
            if (command.direction == MotorDirection::Stopped)
            {
                return 0;
            }

            const uint32_t numerator =
                static_cast<uint32_t> (command.duty) *
                    edgesPerSecond_ *
                    elapsed.milliseconds () *
                    gain +
                remainder;
            const uint32_t denominator = 25500000u;
            const uint32_t edges       = numerator / denominator;

            remainder = numerator % denominator;
            return edges;
        }

        uint16_t edgesPerSecond_;
        uint8_t  leftGain_;
        uint8_t  rightGain_;
        uint32_t leftEdges_;
        uint32_t rightEdges_;
        uint32_t leftRemainder_;
        uint32_t rightRemainder_;
    };

    void testConfigurationAndLifecycle ()
    {
        const RouteStep route[] = {
            RouteStep (RouteAction::Drive, 4, Duration (20), 100),
            RouteStep ()};
        RoverController controller (testConfig (), route, 2);

        require (!controller.initialized (), "constructed controller inert");
        require (controller.update (inputAt (0, 500, 0, 0)).error () ==
                     StatusCode::NotInitialized,
                 "update before initialize");
        require (controller.initialize ().ok (), "initialize");
        require (controller.initialize ().ok (), "repeat initialize");
        require (controller.snapshot ().mode == RoverMode::Ready, "ready after initialize");

        requireStopped (controller.snapshot (), "ready intent stopped");

        controller.shutdown ();

        require (!controller.initialized (), "shutdown inactive");
        require (controller.snapshot ().mode == RoverMode::Inactive, "shutdown mode");

        requireStopped (controller.snapshot (), "shutdown intent stopped");

        RoverController emptyRoute (testConfig (), nullptr, 0);

        require (emptyRoute.initialize ().error () == StatusCode::InvalidArgument,
                 "empty route rejected");

        RoverControllerConfig invalidConfig = testConfig ();
        invalidConfig.resumeClearanceMm      = invalidConfig.minimumClearanceMm;
        RoverController invalidThreshold (invalidConfig, route, 2);

        require (invalidThreshold.initialize ().error () == StatusCode::InvalidArgument,
                 "invalid hysteresis rejected");

        const RouteStep invalidRoute[] = {
            RouteStep (RouteAction::Drive, 0, Duration (20), 100),
            RouteStep ()};
        RoverController invalidStep (testConfig (), invalidRoute, 2);

        require (invalidStep.initialize ().error () == StatusCode::InvalidArgument,
                 "zero edge motion rejected");

        const RouteStep highDutyRoute[] = {
            RouteStep (RouteAction::Drive, 4, Duration (20), 201),
            RouteStep ()};
        RoverController highDuty (testConfig (), highDutyRoute, 2);

        require (highDuty.initialize ().error () == StatusCode::InvalidArgument,
                 "excess duty rejected");

        RouteStep oversizedRoute[RoverController::routeCapacity + 1];
        RoverController oversized (
            testConfig (),
            oversizedRoute,
            static_cast<uint8_t> (RoverController::routeCapacity + 1));

        require (oversized.initialize ().error () == StatusCode::InvalidArgument,
                 "oversized route rejected");
    }

    void testRouteNarrative ()
    {
        const RouteStep route[] = {
            RouteStep (RouteAction::Drive, 2, Duration (20), 100),
            RouteStep (RouteAction::TurnLeft, 1, Duration (20), 80),
            RouteStep (RouteAction::TurnRight, 1, Duration (20), 70),
            RouteStep (RouteAction::Pause, 0, Duration (3), 0),
            RouteStep ()};
        RoverController controller (testConfig (), route, 5);

        require (controller.initialize ().ok (), "route initialize");
        require (controller.update (inputAt (10, 500, 0, 0)).ok (), "observe ready");

        requireStopped (controller.snapshot (), "no motion before start");

        require (controller.update (inputAt (11, 500, 0, 0, true)).ok (), "start route");
        require (controller.snapshot ().mode == RoverMode::Driving, "drive mode");

        requireDriving (controller.snapshot (),
                        MotorDirection::Forward,
                        MotorDirection::Forward,
                        100,
                        "drive command");

        require (controller.update (inputAt (12, 500, 1, 1)).ok (), "drive progress");

        const RoverControllerSnapshot progress = controller.snapshot ();

        require (progress.range.distanceMm == 500, "snapshot keeps range evidence");
        require (progress.rangeObservedAt == TimePoint (12), "snapshot keeps range time");
        require (progress.leftWheel.totalEdges == 1, "snapshot keeps left evidence");
        require (progress.rightWheel.totalEdges == 1, "snapshot keeps right evidence");
        require (progress.leftStepEdges == 1, "snapshot keeps left progress");
        require (progress.rightStepEdges == 1, "snapshot keeps right progress");
        require (progress.status.ok (), "snapshot keeps operation status");
        require (progress.hasDeadline, "moving snapshot has deadline");

        require (!controller.snapshot ().routeAdvanced, "target not reached early");
        require (controller.update (inputAt (13, 500, 2, 2)).ok (), "drive target");
        require (controller.snapshot ().routeIndex == 1, "advance to left turn");
        require (controller.snapshot ().routeAdvanced, "advance event");

        requireDriving (controller.snapshot (),
                        MotorDirection::Reverse,
                        MotorDirection::Forward,
                        80,
                        "left turn command");

        require (controller.update (inputAt (14, 500, 3, 3)).ok (), "left target");
        require (controller.snapshot ().routeIndex == 2, "advance to right turn");

        requireDriving (controller.snapshot (),
                        MotorDirection::Forward,
                        MotorDirection::Reverse,
                        70,
                        "right turn command");

        require (controller.update (inputAt (15, 500, 4, 4)).ok (), "right target");
        require (controller.snapshot ().mode == RoverMode::Paused, "pause mode");

        requireStopped (controller.snapshot (), "pause command");

        require (controller.snapshot ().hasDeadline, "pause deadline visible");
        require (controller.snapshot ().nextDeadline == TimePoint (18), "pause deadline");

        require (controller.update (inputAt (17, 500, 4, 4)).ok (), "before pause deadline");
        require (controller.snapshot ().mode == RoverMode::Paused, "pause remains");
        require (controller.update (inputAt (18, 500, 4, 4)).ok (), "pause deadline");
        require (controller.snapshot ().mode == RoverMode::RouteComplete,
                 "route complete");
        requireStopped (controller.snapshot (), "complete command stopped");
    }

    void testStopDominanceAndRestart ()
    {
        const RouteStep route[] = {
            RouteStep (RouteAction::Drive, 8, Duration (30), 100),
            RouteStep ()};
        RoverController controller (testConfig (), route, 2);

        require (controller.initialize ().ok (), "stop initialize");
        require (controller.update (inputAt (0, 500, 0, 0, true)).ok (), "stop start");
        require (controller.update (inputAt (1, 100, 1, 1, true, true)).ok (),
                 "stop dominates chord and obstacle");

        const RoverControllerSnapshot stopped = controller.snapshot ();

        require (stopped.mode == RoverMode::Stopped, "stop mode");
        require (stopped.stopReason == RoverStopReason::StopRequested, "stop reason");
        require (stopped.stopDominated, "stop dominance evidence");

        requireStopped (stopped, "stop dominates motor intent");

        require (controller.update (inputAt (2, 500, 1, 1)).ok (), "release stop");
        require (controller.snapshot ().mode == RoverMode::Stopped,
                 "release does not restart");
        require (controller.update (inputAt (3, 500, 1, 1, true)).ok (),
                 "explicit restart");
        require (controller.snapshot ().mode == RoverMode::Driving,
                 "fresh stationary start restarts");
        require (controller.snapshot ().routeIndex == 0, "restart resets route");

        controller.shutdown ();

        require        (!controller.initialized (), "active shutdown releases controller");
        requireStopped (controller.snapshot (), "active shutdown stops intent");
        require        (controller.snapshot ().mode == RoverMode::Inactive,
                        "active shutdown mode");
    }

    void testObstacleHysteresis ()
    {
        const RouteStep route[] = {
            RouteStep (RouteAction::Drive, 8, Duration (30), 100),
            RouteStep ()};
        RoverController controller (testConfig (), route, 2);

        require (controller.initialize ().ok (), "obstacle initialize");
        require (controller.update (inputAt (0, 500, 0, 0, true)).ok (),
                 "obstacle start");
        require (controller.update (inputAt (1, 201, 1, 1)).ok (),
                 "outside stop threshold");
        require (controller.snapshot ().mode == RoverMode::Driving,
                 "outside threshold drives");

        require (controller.update (inputAt (2, 200, 2, 2)).ok (),
                 "exact stop threshold");
        require (controller.snapshot ().mode == RoverMode::ObstacleHold,
                 "threshold holds");
        requireStopped (controller.snapshot (), "hold stopped");

        require (controller.update (inputAt (3, 249, 2, 2)).ok (),
                 "below resume threshold");
        require (controller.snapshot ().mode == RoverMode::ObstacleHold,
                 "hysteresis retains hold");
        require (controller.update (inputAt (4, 250, 2, 2)).ok (),
                 "exact resume threshold");
        require (controller.snapshot ().mode == RoverMode::Driving,
                 "resume threshold drives");
    }

    void testFaultsAndTimeouts ()
    {
        const RouteStep route[] = {
            RouteStep (RouteAction::Drive, 20, Duration (30), 100),
            RouteStep ()};

        RoverController noMotion (testConfig (), route, 2);

        require (noMotion.initialize ().ok (), "no-motion initialize");
        require (noMotion.update (inputAt (0, 500, 0, 0, true)).ok (),
                 "no-motion start");
        require (noMotion.update (inputAt (5, 500, 0, 0)).error () ==
                     StatusCode::HardwareFailure,
                 "exact motion timeout");
        require (noMotion.snapshot ().stopReason ==
                     RoverStopReason::MotionDidNotStart,
                 "motion timeout reason");
        requireStopped (noMotion.snapshot (), "motion timeout stopped");

        RoverController mismatch (testConfig (), route, 2);

        require (mismatch.initialize ().ok (), "mismatch initialize");
        require (mismatch.update (inputAt (10, 500, 0, 0, true)).ok (),
                 "mismatch start");
        require (mismatch.update (inputAt (11, 500, 4, 0)).ok (),
                 "mismatch begins");
        require (mismatch.update (inputAt (15, 500, 8, 0)).error () ==
                     StatusCode::HardwareFailure,
                 "exact mismatch timeout");
        require (mismatch.snapshot ().stopReason == RoverStopReason::WheelMismatch,
                 "mismatch reason");

        RoverController encoderFault (testConfig (), route, 2);

        require (encoderFault.initialize ().ok (), "encoder initialize");
        require (encoderFault.update (
                     inputAt (20,
                              500,
                              0,
                              0,
                              true,
                              false,
                              StatusCode::HardwareFailure)).error () ==
                     StatusCode::HardwareFailure,
                 "encoder fault");
        require (encoderFault.snapshot ().stopReason == RoverStopReason::EncoderFault,
                 "encoder fault reason");
        requireStopped (encoderFault.snapshot (), "encoder fault stopped");

        RoverController staleRange (testConfig (), route, 2);

        require (staleRange.initialize ().ok (), "stale initialize");

        const RoverInput staleInput (
            validRange (500),

            TimePoint (0),

            RoverWheelObservation (),
            RoverWheelObservation (),
            true,
            false,
            TimePoint (21));

        require (staleRange.update (staleInput).error () ==
                     StatusCode::HardwareFailure,
                 "stale range faults");
        require (staleRange.snapshot ().stopReason == RoverStopReason::RangeStale,
                 "stale reason");

        RoverController invalidRange (testConfig (), route, 2);

        require (invalidRange.initialize ().ok (), "invalid range initialize");

        RoverInput invalidInput = inputAt (0, 500, 0, 0, true);
        invalidInput.range      = {
            RangeState::Timeout,
            0,
            MicrosecondDuration (0),
            false};

        require (invalidRange.update (invalidInput).error () ==
                     StatusCode::HardwareFailure,
                 "invalid range faults");
        require (invalidRange.snapshot ().stopReason ==
                     RoverStopReason::RangeInvalid,
                 "invalid range reason");

        RoverController unexpectedMotion (testConfig (), route, 2);

        require (unexpectedMotion.initialize ().ok (), "unexpected initialize");
        require (unexpectedMotion.update (inputAt (30, 500, 0, 0)).ok (),
                 "unexpected anchor");
        require (unexpectedMotion.update (inputAt (31, 500, 1, 0)).error () ==
                     StatusCode::HardwareFailure,
                 "movement while ready faults");
        require (unexpectedMotion.snapshot ().stopReason ==
                     RoverStopReason::UnexpectedMotion,
                 "unexpected motion reason");
        requireStopped (unexpectedMotion.snapshot (), "unexpected motion stopped");
    }

    void testTimeRolloverAndRouteTimeout ()
    {
        const RouteStep route[] = {
            RouteStep (RouteAction::Drive, 20, Duration (10), 100),
            RouteStep ()};
        RoverController rollover (testConfig (), route, 2);

        require (rollover.initialize ().ok (), "rollover initialize");
        require (rollover.update (
                     inputAt (0xfffffffau, 500, 0, 0, true)).ok (),
                 "rollover start");
        require (rollover.update (inputAt (0xffffffffu, 500, 1, 1)).ok (),
                 "rollover progress");
        require (rollover.update (inputAt (4, 500, 2, 2)).error () ==
                     StatusCode::HardwareFailure,
                 "exact wrapped route timeout");
        require (rollover.snapshot ().stopReason == RoverStopReason::RouteTimeout,
                 "wrapped timeout reason");

        RoverController backward (testConfig (), route, 2);

        require (backward.initialize ().ok (), "backward initialize");
        require (backward.update (inputAt (100, 500, 0, 0)).ok (),
                 "backward anchor");
        require (backward.update (inputAt (99, 500, 0, 0)).error () ==
                     StatusCode::InvalidArgument,
                 "backward time rejected");
        require (backward.snapshot ().stopReason == RoverStopReason::InvalidTime,
                 "backward reason");
        requireStopped (backward.snapshot (), "invalid time stopped");
    }

    void testDeterministicPlantReplay ()
    {
        const RouteStep route[] = {
            RouteStep (RouteAction::Drive, 6, Duration (200), 200),
            RouteStep ()};
        RoverController first  (testConfig (), route, 2);
        RoverController second (testConfig (), route, 2);

        DifferentialDrivePlant firstPlant  (2550);
        DifferentialDrivePlant secondPlant (2550);

        require (first.initialize ().ok (), "first plant initialize");
        require (second.initialize ().ok (), "second plant initialize");

        for (uint32_t tick = 0; tick < 20; ++tick)
        {
            const TimePoint now (tick);
            const bool      start = tick == 0;
            const RoverInput firstInput (
                validRange (500),
                now,
                firstPlant.leftWheel (),

                firstPlant.rightWheel (),
                start,
                false,
                now);
            const RoverInput secondInput (
                validRange (500),
                now,
                secondPlant.leftWheel (),

                secondPlant.rightWheel (),
                start,
                false,
                now);

            require (first.update (firstInput).error () ==
                         second.update (secondInput).error (),
                     "plant replay status");

            const RoverControllerSnapshot left  = first.snapshot ();

            const RoverControllerSnapshot right = second.snapshot ();

            require (left.mode == right.mode, "plant replay mode");
            require (left.stopReason == right.stopReason, "plant replay reason");
            require (left.routeIndex == right.routeIndex, "plant replay route");
            require (left.leftMotor.direction == right.leftMotor.direction,
                     "plant replay left direction");
            require (left.leftMotor.duty == right.leftMotor.duty,
                     "plant replay left duty");
            require (left.rightMotor.direction == right.rightMotor.direction,
                     "plant replay right direction");
            require (left.rightMotor.duty == right.rightMotor.duty,
                     "plant replay right duty");
            require (left.leftWheel.totalEdges == right.leftWheel.totalEdges,
                     "plant replay left evidence");
            require (left.rightWheel.totalEdges == right.rightWheel.totalEdges,
                     "plant replay right evidence");
            require (left.transitionCount == right.transitionCount,
                     "plant replay transitions");

            firstPlant.advance (left, Duration (1));

            secondPlant.advance (right, Duration (1));
        }

        require (first.snapshot ().mode == RoverMode::RouteComplete,
                 "plant completes route");
    }
}

int main ()
{
    testConfigurationAndLifecycle ();

    testRouteNarrative ();

    testStopDominanceAndRestart ();

    testObstacleHysteresis ();

    testFaultsAndTimeouts ();

    testTimeRolloverAndRouteTimeout ();

    testDeterministicPlantReplay ();

    std::cout << "rover controller tests passed\n";
    return 0;
}
