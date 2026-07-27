#include <motor_intent.h>
#include <power_domain.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    struct TestPower final : adk::PowerDomain
    {
        bool commandAdmitted () const noexcept override
        {
            return commandAdmitted_;
        }

        bool commandAdmitted_ = true;
    };

    bool commandEquals (const adk::MotorCommand& left, const adk::MotorCommand& right)
    {
        return left.direction == right.direction && left.duty == right.duty;
    }

    bool snapshotEquals (const adk::MotorIntentSnapshot& left,
                         const adk::MotorIntentSnapshot& right)
    {
        return commandEquals (left.requested, right.requested) &&
               commandEquals (left.applied, right.applied) &&
               left.phase == right.phase && left.status == right.status &&
               left.phaseSince == right.phaseSince &&
               left.nextDeadline == right.nextDeadline &&
               left.hasDeadline == right.hasDeadline &&
               left.transitionCount == right.transitionCount;
    }

    void requireStopped (const adk::MotorIntentSnapshot& snapshot, const char* message)
    {

        require (snapshot.applied.direction == adk::MotorDirection::Stopped &&
                     snapshot.applied.duty == 0,
                 message);
    }

    void testLifecycleAndValidation ()
    {
        static_assert (!std::is_copy_constructible<adk::MotorIntent>::value,
                       "motor driver must not copy");
        static_assert (!std::is_move_constructible<adk::MotorIntent>::value,
                       "motor driver must not move");

        TestPower        power;
        adk::MotorIntent motor (adk::MotorIntentConfig (adk::Duration (20), 200), power);

        requireStopped (motor.snapshot (), "construction is stopped");

        require (!motor.initialized (), "construction is not initialized");

        require (motor.command ({adk::MotorDirection::Forward, 1}, adk::TimePoint (0))
                         .error () == adk::StatusCode::NotInitialized,
                 "command before initialize rejected");

        require (motor.update (adk::TimePoint (0)).error () ==
                     adk::StatusCode::NotInitialized,
                 "update before initialize rejected");

        require (motor.stop ().error () == adk::StatusCode::NotInitialized,
                 "stop before initialize rejected");

        require (motor.initialize ().ok (), "initialize succeeds");

        require (motor.initialize ().ok (), "initialize is idempotent");


        require (motor.command ({adk::MotorDirection::Stopped, 1}, adk::TimePoint (0))
                         .error () == adk::StatusCode::InvalidArgument,
                 "stopped requires zero duty");

        require (motor.command ({adk::MotorDirection::Forward, 0}, adk::TimePoint (0))
                         .error () == adk::StatusCode::InvalidArgument,
                 "motion requires nonzero duty");

        require (motor.command ({adk::MotorDirection::Forward, 201}, adk::TimePoint (0))
                         .error () == adk::StatusCode::InvalidArgument,
                 "configured duty bound enforced");
        requireStopped (motor.snapshot (), "invalid commands stay stopped");

        motor.shutdown ();
        motor.shutdown ();
        requireStopped (motor.snapshot (), "shutdown remains stopped");

        require (!motor.initialized (), "shutdown clears initialization");

        adk::MotorIntent zeroDeadTime (adk::MotorIntentConfig (adk::Duration (0), 200),
                                       power);
        adk::MotorIntent zeroMaximum (adk::MotorIntentConfig (adk::Duration (20), 0), power);


        require (zeroDeadTime.initialize ().error () ==
                     adk::StatusCode::InvalidArgument,
                 "zero dead time rejected");

        require (zeroMaximum.initialize ().error () == adk::StatusCode::InvalidArgument,
                 "zero maximum duty rejected");
    }

    void testRunningAndSameDirection ()
    {
        TestPower        power;
        adk::MotorIntent motor (adk::MotorIntentConfig (adk::Duration (20), 200), power);


        require (motor.initialize ().ok (), "motor initializes");

        require (
            motor.command ({adk::MotorDirection::Forward, 80}, adk::TimePoint (100))
                .ok (),
            "forward command applies");

        adk::MotorIntentSnapshot snapshot = motor.snapshot ();

        require (snapshot.phase == adk::MotorIntentPhase::Running, "forward enters running");

        require (snapshot.applied.direction == adk::MotorDirection::Forward &&
                     snapshot.applied.duty == 80,
                 "forward intent is applied");

        require (!snapshot.hasDeadline, "running has no deadline");


        require (
            motor.command ({adk::MotorDirection::Forward, 140}, adk::TimePoint (101))
                .ok (),
            "same-direction duty changes immediately");
        snapshot = motor.snapshot ();

        require (snapshot.applied.duty == 140, "new same-direction duty is applied");


        require (motor.stop ().ok (), "stop succeeds");

        snapshot = motor.snapshot ();

        requireStopped (snapshot, "stop dominates motion");

        require (snapshot.phase == adk::MotorIntentPhase::Inactive, "stop enters inactive");

        const uint32_t stoppedTransitions = snapshot.transitionCount;

        require (motor.stop ().ok (), "repeated stop succeeds");

        require (motor.snapshot ().transitionCount == stoppedTransitions,
                 "repeated stop does not invent a transition");
    }

    void testReversalDeadTime ()
    {
        TestPower        power;
        adk::MotorIntent motor (adk::MotorIntentConfig (adk::Duration (20), 200), power);


        require (motor.initialize ().ok (), "motor initializes");

        require (
            motor.command ({adk::MotorDirection::Forward, 100}, adk::TimePoint (10))
                .ok (),
            "forward applies");

        require (
            motor.command ({adk::MotorDirection::Reverse, 120}, adk::TimePoint (30))
                .ok (),
            "reverse request accepted");

        adk::MotorIntentSnapshot snapshot = motor.snapshot ();

        require (snapshot.phase == adk::MotorIntentPhase::WaitingForDeadTime,
                 "reversal waits");
        requireStopped (snapshot, "reversal immediately stops output intent");

        require (snapshot.requested.direction == adk::MotorDirection::Reverse &&
                     snapshot.requested.duty == 120,
                 "reverse remains requested");

        require (snapshot.hasDeadline && snapshot.nextDeadline == adk::TimePoint (50),
                 "exact reversal deadline exposed");


        require (motor.update (adk::TimePoint (49)).ok (),
                 "one tick before deadline remains valid");
        requireStopped (motor.snapshot (), "one tick early remains stopped");

        require (motor.update (adk::TimePoint (50)).ok (), "exact deadline applies");

        snapshot = motor.snapshot ();

        require (snapshot.phase == adk::MotorIntentPhase::Running &&
                     snapshot.applied.direction == adk::MotorDirection::Reverse,
                 "reverse applies at exact deadline");


        require (motor.command ({adk::MotorDirection::Forward, 90}, adk::TimePoint (60))
                     .ok (),
                 "opposite reversal also waits");

        require (motor.update (adk::TimePoint (1000)).ok (),
                 "time jump crosses deadline");

        require (motor.snapshot ().applied.direction == adk::MotorDirection::Forward,
                 "time jump applies pending direction");
    }

    void testCommandsDuringDeadTime ()
    {
        TestPower        power;
        adk::MotorIntent motor (adk::MotorIntentConfig (adk::Duration (20), 200), power);


        require (motor.initialize ().ok (), "motor initializes");

        require (motor.command ({adk::MotorDirection::Forward, 80}, adk::TimePoint (0))
                     .ok (),
                 "forward applies");

        require (motor.command ({adk::MotorDirection::Reverse, 90}, adk::TimePoint (10))
                     .ok (),
                 "reversal begins");

        require (
            motor.command ({adk::MotorDirection::Reverse, 150}, adk::TimePoint (15))
                .ok (),
            "pending duty may change");

        require (motor.snapshot ().nextDeadline == adk::TimePoint (30),
                 "replacement preserves original dead-time deadline");

        require (motor.update (adk::TimePoint (30)).ok (),
                 "updated pending command applies");

        require (motor.snapshot ().applied.duty == 150, "latest pending duty applies");


        require (motor.command ({adk::MotorDirection::Forward, 80}, adk::TimePoint (40))
                     .ok (),
                 "second reversal begins");

        require (motor.command ({adk::MotorDirection::Reverse, 70}, adk::TimePoint (45))
                     .ok (),
                 "return to prior direction cancels reversal");

        require (motor.snapshot ().phase == adk::MotorIntentPhase::Running &&
                     motor.snapshot ().applied.direction ==
                         adk::MotorDirection::Reverse,
                 "prior direction resumes immediately");


        require (motor.command ({adk::MotorDirection::Forward, 60}, adk::TimePoint (50))
                     .ok (),
                 "third reversal begins");

        require (motor.stop ().ok (), "stop cancels pending command");

        requireStopped (motor.snapshot (), "stop during dead time stays stopped");

        require (!motor.snapshot ().hasDeadline, "stop removes reversal deadline");
    }

    void testRolloverAndInvalidTime ()
    {
        TestPower        power;
        adk::MotorIntent motor (adk::MotorIntentConfig (adk::Duration (10), 200), power);


        require (motor.initialize ().ok (), "motor initializes");

        require (motor
                     .command ({adk::MotorDirection::Forward, 80},
                               adk::TimePoint (0xFFFFFFF0UL))
                     .ok (),
                 "forward near rollover");

        require (motor
                     .command ({adk::MotorDirection::Reverse, 80},
                               adk::TimePoint (0xFFFFFFFAUL))
                     .ok (),
                 "reversal near rollover");

        require (motor.update (adk::TimePoint (3)).ok (), "rollover before deadline");

        requireStopped (motor.snapshot (), "rollover one tick early stopped");

        require (motor.update (adk::TimePoint (4)).ok (), "rollover exact deadline");

        require (motor.snapshot ().applied.direction == adk::MotorDirection::Reverse,
                 "rollover deadline applies");


        require (motor.update (adk::TimePoint (3)).error () ==
                     adk::StatusCode::InvalidArgument,
                 "backward time faults");

        require (motor.snapshot ().phase == adk::MotorIntentPhase::Fault,
                 "invalid time fault is sticky");
        requireStopped (motor.snapshot (), "invalid time forces stopped intent");
    }

    void testPowerFaultAndRecovery ()
    {
        TestPower        power;
        adk::MotorIntent motor (adk::MotorIntentConfig (adk::Duration (20), 200), power);


        require (motor.initialize ().ok (), "motor initializes");
        power.commandAdmitted_ = false;


        require (motor.command ({adk::MotorDirection::Forward, 80}, adk::TimePoint (0))
                         .error () == adk::StatusCode::HardwareFailure,
                 "unavailable power rejects motion");

        require (motor.snapshot ().phase == adk::MotorIntentPhase::Fault,
                 "power failure latches fault");
        requireStopped (motor.snapshot (), "power failure forces stopped intent");

        require (motor.snapshot ().requested.direction ==
                     adk::MotorDirection::Forward,
                 "fault preserves rejected motion for diagnosis");

        power.commandAdmitted_ = true;

        require (motor.command ({adk::MotorDirection::Forward, 80}, adk::TimePoint (1))
                         .error () == adk::StatusCode::HardwareFailure,
                 "power return does not clear fault");

        require (motor.stop ().error () == adk::StatusCode::HardwareFailure,
                 "stop preserves sticky fault");

        motor.shutdown ();

        require (motor.initialize ().ok (), "shutdown and initialize clear fault");

        require (motor.command ({adk::MotorDirection::Forward, 80}, adk::TimePoint (2))
                     .ok (),
                 "motion resumes after explicit restart");

        power.commandAdmitted_ = false;

        require (motor.update (adk::TimePoint (3)).error () ==
                     adk::StatusCode::HardwareFailure,
                 "lost power during run faults");
        requireStopped (motor.snapshot (), "lost power stops applied intent");
    }

    void testDeterministicReplay ()
    {
        TestPower        firstPower;
        TestPower        secondPower;
        adk::MotorIntent first  (adk::MotorIntentConfig (adk::Duration (20), 200),
                                 firstPower);
        adk::MotorIntent second (adk::MotorIntentConfig (adk::Duration (20), 200),
                                 secondPower);


        require (first.initialize ().ok () && second.initialize ().ok (),
                 "replay motors initialize");

        const adk::MotorCommand commands[] = {{adk::MotorDirection::Forward, 60},
                                              {adk::MotorDirection::Forward, 100},
                                              {adk::MotorDirection::Reverse, 90}};
        const adk::TimePoint    times[]    = {adk::TimePoint (10), adk::TimePoint (20),
                                              adk::TimePoint (30)};

        for (uint8_t index = 0; index < 3; ++index)
        {
            require (first.command (commands[index], times[index]) ==
                         second.command (commands[index], times[index]),
                     "replay statuses match");
            require (snapshotEquals (first.snapshot (), second.snapshot ()),
                     "replay command snapshots match");
        }


        require (first.update (adk::TimePoint (50)) ==
                     second.update (adk::TimePoint (50)),
                 "replay update statuses match");

        require (snapshotEquals (first.snapshot (), second.snapshot ()),
                 "replay final snapshots match");
    }
} // namespace

int main ()
{
    testLifecycleAndValidation  ();
    testRunningAndSameDirection ();
    testReversalDeadTime        ();
    testCommandsDuringDeadTime  ();
    testRolloverAndInvalidTime  ();
    testPowerFaultAndRecovery   ();
    testDeterministicReplay     ();

    std::cout << "All ADK motor-intent tests passed.\n";
    return EXIT_SUCCESS;
}
