#include <watering_controller.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <vector>

namespace {

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    struct RecordingPumpOutput final : adk::PumpOutput
    {
        adk::Status initialize () noexcept override
        {
            ++initializeCalls;

            if (!initializeStatus.ok ())
            {
                return initializeStatus;
            }

            initialized_ = true;
            state_       = adk::PumpState::Off;
            return adk::StatusCode::Ok;
        }

        void shutdown () noexcept override
        {
            ++shutdownCalls;
            initialized_ = false;
            state_       = adk::PumpState::Off;
        }

        adk::Status setState (adk::PumpState state) noexcept override
        {
            commands.push_back (state);

            if (failNextCommand)
            {
                failNextCommand = false;
                return adk::StatusCode::HardwareFailure;
            }

            state_ = state;
            return adk::StatusCode::Ok;
        }

        adk::PumpState state () const noexcept override
        {
            return state_;
        }

        bool initialized () const noexcept override
        {
            return initialized_;
        }

        adk::Status                 initializeStatus = adk::StatusCode::Ok;
        std::vector<adk::PumpState> commands;
        adk::PumpState              state_          = adk::PumpState::Off;
        uint8_t                     initializeCalls = 0;
        uint8_t                     shutdownCalls   = 0;
        bool                        initialized_    = false;
        bool                        failNextCommand = false;
    };

    adk::WateringConfig config ()
    {
        return {350, 600, adk::Duration (5000), adk::Duration (2000)};
    }

    adk::MoistureSample
    sample (uint16_t                 moisture,
            adk::MoistureSampleState state = adk::MoistureSampleState::Valid)
    {
        return {moisture, 500, adk::TimePoint (), state};
    }

    void requireSnapshot (const adk::WateringController& controller,
                          adk::WateringState state, adk::WateringReason reason,
                          adk::PumpState pump, const char* message)
    {
        const adk::WateringSnapshot snapshot = controller.snapshot ();

        require (snapshot.state == state, message);
        require (snapshot.reason == reason, message);
        require (snapshot.requestedPump == pump, message);
    }

    void decideAndActuate (adk::WateringController& controller, uint32_t now,
                           const adk::MoistureSample& moisture,
                           bool                       wateringAllowed = true)
    {
        require (
            controller.decide (adk::TimePoint (now), moisture, wateringAllowed).ok (),
            "decision succeeds");
        require (controller.actuate ().ok (), "actuation succeeds");
    }

    void testLifecycleAndConfiguration ()
    {
        static_assert (!std::is_copy_constructible<adk::WateringController>::value,
                       "watering controller must not copy");
        static_assert (!std::is_move_constructible<adk::WateringController>::value,
                       "watering controller must not move");

        const adk::WateringConfig invalid[] = {
            {600, 600, adk::Duration  (1), adk::Duration (1)},
            {601, 600, adk::Duration  (1), adk::Duration (1)},
            {350, 1001, adk::Duration (1), adk::Duration (1)},
            {350, 600, adk::Duration  (0), adk::Duration (1)},
            {350, 600, adk::Duration  (1), adk::Duration (0)},
            {350, 600, adk::Duration  (0x80000000UL), adk::Duration (1)},
            {350, 600, adk::Duration  (1), adk::Duration (0x80000000UL)}};

        for (const adk::WateringConfig& value : invalid)
        {
            RecordingPumpOutput     pump;
            adk::WateringController controller (value, pump);

            require (controller.initialize ().error () ==
                         adk::StatusCode::InvalidArgument,
                     "invalid configuration is rejected");
            require (pump.initializeCalls == 0,
                     "invalid configuration touches no output");
        }

        RecordingPumpOutput     pump;
        adk::WateringController controller (config (), pump);

        require (controller.decide (adk::TimePoint (), sample (500), true).error () ==
                     adk::StatusCode::NotInitialized,
                 "decision before initialization is rejected");
        require (controller.actuate ().error () == adk::StatusCode::NotInitialized,
                 "actuation before initialization is rejected");
        require (controller.initialize ().ok (), "initialization succeeds");
        require (controller.initialize ().ok (), "initialization is idempotent");
        require (pump.initializeCalls == 1, "output initializes once");

        controller.shutdown ();
        controller.shutdown ();
        require             (!controller.initialized (), "shutdown clears initialization");
        require             (pump.state () == adk::PumpState::Off, "shutdown leaves output off");
    }

    void testInitializationFailureRollsBack ()
    {
        RecordingPumpOutput pump;
        pump.initializeStatus = adk::StatusCode::ResourceBusy;
        adk::WateringController controller (config (), pump);

        require (controller.initialize ().error () == adk::StatusCode::ResourceBusy,
                 "output initialization failure is propagated");
        require (pump.shutdownCalls == 1, "failed initialization rolls output back");
        require (!controller.initialized (), "failed initialization stays inert");
    }

    void testGoldenTraceAndNoDuplicateCommands ()
    {
        RecordingPumpOutput     pump;
        adk::WateringController controller (config (), pump);

        require (controller.initialize ().ok (), "controller initializes");

        decideAndActuate (controller, 0, sample (500));
        requireSnapshot  (controller, adk::WateringState::Idle,
                         adk::WateringReason::None, adk::PumpState::Off,
                         "first sample anchors off interval");

        decideAndActuate (controller, 1999, sample (300));
        requireSnapshot  (controller, adk::WateringState::Idle,
                         adk::WateringReason::MinimumOffTime, adk::PumpState::Off,
                         "one tick before minimum remains off");

        decideAndActuate (controller, 2000, sample (300));
        requireSnapshot  (controller, adk::WateringState::Watering,
                         adk::WateringReason::DryThreshold, adk::PumpState::On,
                         "minimum-off boundary starts watering");

        const std::size_t onCommands = pump.commands.size ();
        decideAndActuate                                  (controller, 4000, sample (500));
        require                                           (pump.commands.size () == onCommands,
                 "unchanged watering state emits no duplicate command");

        decideAndActuate (controller, 4500, sample (600));
        requireSnapshot  (controller, adk::WateringState::Idle,
                         adk::WateringReason::WetThreshold, adk::PumpState::Off,
                         "wet threshold stops watering");
        decideAndActuate (controller, 6500, sample (300));
        decideAndActuate (controller, 11499, sample (300));
        requireSnapshot  (controller, adk::WateringState::Watering,
                         adk::WateringReason::DryThreshold, adk::PumpState::On,
                         "one tick before maximum remains watering");
        decideAndActuate (controller, 11500, sample (300));
        requireSnapshot  (controller, adk::WateringState::LockedOut,
                         adk::WateringReason::MaximumOnTime, adk::PumpState::Off,
                         "maximum-on boundary locks out");
        decideAndActuate (controller, 13500, sample (300));
        requireSnapshot  (controller, adk::WateringState::Idle,
                         adk::WateringReason::MinimumOffTime, adk::PumpState::Off,
                         "lockout expires without starting");
        decideAndActuate (controller, 13501, sample (300));
        requireSnapshot  (controller, adk::WateringState::Watering,
                         adk::WateringReason::DryThreshold, adk::PumpState::On,
                         "later dry decision restarts");
        decideAndActuate (controller, 14000,
                          sample (0, adk::MoistureSampleState::Stale));
        requireSnapshot (controller, adk::WateringState::SensorFault,
                         adk::WateringReason::InvalidSample, adk::PumpState::Off,
                         "stale sample stops watering");
    }

    void testExactMoistureThresholds ()
    {
        RecordingPumpOutput     pump;
        adk::WateringController controller (config (), pump);

        require          (controller.initialize ().ok (), "controller initializes");
        decideAndActuate (controller, 0, sample (350));
        decideAndActuate (controller, 2000, sample (350));
        requireSnapshot  (controller, adk::WateringState::Idle,
                         adk::WateringReason::None, adk::PumpState::Off,
                         "exact dry threshold remains off");
        decideAndActuate (controller, 2001, sample (349));
        requireSnapshot  (controller, adk::WateringState::Watering,
                         adk::WateringReason::DryThreshold, adk::PumpState::On,
                         "one permille below dry threshold starts");
        decideAndActuate (controller, 2002, sample (599));
        requireSnapshot  (controller, adk::WateringState::Watering,
                         adk::WateringReason::DryThreshold, adk::PumpState::On,
                         "one permille below wet threshold continues");
        decideAndActuate (controller, 2003, sample (600));
        requireSnapshot  (controller, adk::WateringState::Idle,
                         adk::WateringReason::WetThreshold, adk::PumpState::Off,
                         "exact wet threshold stops");
    }

    void testValidSampleRecoversSensorFaultIntoFreshOffInterval ()
    {
        RecordingPumpOutput     pump;
        adk::WateringController controller (config (), pump);

        require          (controller.initialize ().ok (), "controller initializes");
        decideAndActuate (
            controller, 0, sample (0, adk::MoistureSampleState::Unavailable));
        decideAndActuate (controller, 5000, sample (300));
        requireSnapshot  (controller, adk::WateringState::Idle,
                         adk::WateringReason::None, adk::PumpState::Off,
                         "valid sample recovers fault to safe idle");
        decideAndActuate (controller, 6999, sample (300));
        requireSnapshot  (controller, adk::WateringState::Idle,
                         adk::WateringReason::MinimumOffTime, adk::PumpState::Off,
                         "recovery starts a fresh minimum-off interval");
        decideAndActuate (controller, 7000, sample (300));
        requireSnapshot  (controller, adk::WateringState::Watering,
                         adk::WateringReason::DryThreshold, adk::PumpState::On,
                         "watering may resume after recovered off interval");
    }

    void testInvalidSamplesFromEveryReachableState ()
    {
        const adk::MoistureSampleState faults[] = {
            adk::MoistureSampleState::Unavailable,
            adk::MoistureSampleState::InputBelowRange,
            adk::MoistureSampleState::InputAboveRange,
            adk::MoistureSampleState::Stale
        };

        for (const adk::MoistureSampleState fault : faults)
        {
            RecordingPumpOutput     pump;
            adk::WateringController controller (config (), pump);

            require (controller.initialize ().ok (), "controller initializes");

            decideAndActuate (controller, 0, sample (0, fault));
            requireSnapshot  (controller, adk::WateringState::SensorFault,
                             adk::WateringReason::InvalidSample, adk::PumpState::Off,
                             "invalid starting sample enters sensor fault");

            decideAndActuate (controller, 1, sample (500));
            decideAndActuate (controller, 2, sample (0, fault));
            requireSnapshot  (controller, adk::WateringState::SensorFault,
                             adk::WateringReason::InvalidSample, adk::PumpState::Off,
                             "invalid idle sample enters sensor fault");

            decideAndActuate (controller, 3, sample (300));
            decideAndActuate (controller, 2003, sample (300));
            decideAndActuate (controller, 2004, sample (0, fault));
            requireSnapshot  (controller, adk::WateringState::SensorFault,
                             adk::WateringReason::InvalidSample, adk::PumpState::Off,
                             "invalid watering sample stops output");

            decideAndActuate (controller, 2005, sample (300));
            decideAndActuate (controller, 4005, sample (300));
            decideAndActuate (controller, 9005, sample (300));
            requireSnapshot  (controller, adk::WateringState::LockedOut,
                             adk::WateringReason::MaximumOnTime, adk::PumpState::Off,
                             "trace reaches lockout");
            decideAndActuate (controller, 9006, sample (0, fault));
            requireSnapshot  (controller, adk::WateringState::SensorFault,
                             adk::WateringReason::InvalidSample, adk::PumpState::Off,
                             "invalid locked-out sample enters sensor fault");
        }
    }

    void testPrecedenceAndOperatorInhibit ()
    {
        RecordingPumpOutput     pump;
        adk::WateringController controller (config (), pump);

        require          (controller.initialize ().ok (), "controller initializes");
        decideAndActuate (controller, 0, sample (300));
        decideAndActuate (controller, 2000, sample (300));

        decideAndActuate (controller, 7000, sample (700));
        requireSnapshot  (controller, adk::WateringState::LockedOut,
                         adk::WateringReason::MaximumOnTime, adk::PumpState::Off,
                         "timeout precedes wet threshold");

        decideAndActuate (controller, 7100, sample (300), false);
        requireSnapshot  (controller, adk::WateringState::Idle,
                         adk::WateringReason::OperatorInhibit, adk::PumpState::Off,
                         "operator inhibit keeps output off");
    }

    void testSampleFaultsAndRollover ()
    {
        const adk::MoistureSampleState faults[] = {
            adk::MoistureSampleState::Unavailable,
            adk::MoistureSampleState::InputBelowRange,
            adk::MoistureSampleState::InputAboveRange, adk::MoistureSampleState::Stale};

        for (const adk::MoistureSampleState fault : faults)
        {
            RecordingPumpOutput     pump;
            adk::WateringController controller (config (), pump);

            require          (controller.initialize ().ok (), "controller initializes");
            decideAndActuate (controller, 0xFFFFFF00UL, sample (300));
            decideAndActuate (controller, 0x000006D0UL, sample (300));
            requireSnapshot  (controller, adk::WateringState::Watering,
                             adk::WateringReason::DryThreshold, adk::PumpState::On,
                             "minimum-off timer crosses rollover");
            decideAndActuate (controller, 0x000006D1UL, sample (0, fault));
            requireSnapshot  (controller, adk::WateringState::SensorFault,
                             adk::WateringReason::InvalidSample, adk::PumpState::Off,
                             "every invalid sample state stops output");
        }
    }

    void testOutputFailureIsStickyUntilReinitialization ()
    {
        RecordingPumpOutput     pump;
        adk::WateringController controller (config (), pump);

        require          (controller.initialize ().ok (), "controller initializes");
        decideAndActuate (controller, 0, sample (300));
        require          (controller.decide (adk::TimePoint (2000), sample (300), true).ok (),
                 "dry decision requests on");
        pump.failNextCommand = true;

        require (controller.actuate ().error () == adk::StatusCode::HardwareFailure,
                 "on failure is reported");
        requireSnapshot (controller, adk::WateringState::OutputFault,
                         adk::WateringReason::OutputFailure, adk::PumpState::Off,
                         "output failure requests safe off");
        require (
            controller.decide (adk::TimePoint (3000), sample (300), true).error () ==
                adk::StatusCode::HardwareFailure,
            "output fault is sticky");

        controller.shutdown ();
        require             (controller.initialize ().ok (),
                 "reinitialization clears output fault");

        decideAndActuate (controller, 0, sample (300));
        decideAndActuate (controller, 2000, sample (300));
        require          (controller.decide (adk::TimePoint (2001), sample (600), true).ok (),
                 "wet decision requests off");
        pump.failNextCommand = true;

        require (controller.actuate ().error () == adk::StatusCode::HardwareFailure,
                 "off failure is reported");
        require (pump.state () == adk::PumpState::Off,
                 "off failure receives one best-effort safe command");
        requireSnapshot (controller, adk::WateringState::OutputFault,
                         adk::WateringReason::OutputFailure, adk::PumpState::Off,
                         "off failure also latches output fault");
    }

    void driveToState (adk::WateringController& controller,
                       RecordingPumpOutput&     pump,
                       adk::WateringState       state)
    {
        require (controller.initialize ().ok (), "controller initializes");

        if (state == adk::WateringState::Starting)
        {
            return;
        }

        if (state == adk::WateringState::SensorFault)
        {
            decideAndActuate (
                controller, 0, sample (0, adk::MoistureSampleState::Stale));
            return;
        }

        decideAndActuate (controller, 0, sample (300));

        if (state == adk::WateringState::Idle)
        {
            return;
        }

        require (controller.decide (adk::TimePoint (2000), sample (300), true).ok (),
                 "dry decision requests watering");

        if (state == adk::WateringState::OutputFault)
        {
            pump.failNextCommand = true;
            require (!controller.actuate ().ok (), "injected output fault occurs");
            return;
        }

        require (controller.actuate ().ok (), "watering actuation succeeds");

        if (state == adk::WateringState::LockedOut)
        {
            decideAndActuate (controller, 7000, sample (300));
        }
    }

    void testShutdownAndDestructionFromEveryState ()
    {
        const adk::WateringState states[] = {
            adk::WateringState::Starting, adk::WateringState::Idle,
            adk::WateringState::Watering, adk::WateringState::LockedOut,
            adk::WateringState::SensorFault, adk::WateringState::OutputFault
        };

        for (const adk::WateringState state : states)
        {
            RecordingPumpOutput pump;

            {
                adk::WateringController controller (config (), pump);

                driveToState        (controller, pump, state);
                controller.shutdown ();
                controller.shutdown ();

                require (!controller.initialized (), "shutdown clears lifecycle");
                require (!pump.commands.empty () &&
                             pump.commands.back () == adk::PumpState::Off,
                         "shutdown trace ends with off");
                require (pump.shutdownCalls == 1,
                         "repeated shutdown releases output exactly once");
            }

            require (pump.shutdownCalls == 1,
                     "destruction after shutdown adds no output operation");
            require (pump.state () == adk::PumpState::Off,
                     "shutdown leaves output off from every state");
        }

        for (const adk::WateringState state : states)
        {
            RecordingPumpOutput pump;

            {
                adk::WateringController controller (config (), pump);

                driveToState (controller, pump, state);
            }

            require (!pump.commands.empty () &&
                         pump.commands.back () == adk::PumpState::Off,
                     "destruction trace ends with off");
            require (pump.shutdownCalls == 1,
                     "destruction releases output exactly once");
            require (pump.state () == adk::PumpState::Off,
                     "destruction leaves output off from every state");
        }
    }

    struct ReplayResult
    {
        std::vector<adk::WateringSnapshot> snapshots;
        std::vector<adk::PumpState>        commands;
    };

    ReplayResult runReplay ()
    {
        RecordingPumpOutput     pump;
        adk::WateringController controller (config (), pump);
        ReplayResult            result;
        const uint32_t          times[]    = {0, 1999, 2000, 4000, 4500,
                                             6500, 11500, 13500, 13501, 14000};
        const uint16_t          moisture[] = {500, 300, 300, 500, 600,
                                             300, 300, 300, 300, 0};

        require (controller.initialize ().ok (), "replay controller initializes");

        for (uint8_t index = 0; index < 10; ++index)
        {
            const adk::MoistureSampleState state =
                index == 9 ? adk::MoistureSampleState::Stale
                           : adk::MoistureSampleState::Valid;

            decideAndActuate           (controller, times[index], sample (moisture[index], state));
            result.snapshots.push_back (controller.snapshot ());
        }

        result.commands = pump.commands;
        return result;
    }

    void testIdenticalReplay ()
    {
        const ReplayResult first  = runReplay ();
        const ReplayResult second = runReplay ();

        require (first.commands == second.commands,
                 "identical replay emits identical output commands");
        require (first.snapshots.size () == second.snapshots.size (),
                 "identical replay emits equal snapshot counts");

        for (std::size_t index = 0; index < first.snapshots.size (); ++index)
        {
            const adk::WateringSnapshot& left  = first.snapshots[index];
            const adk::WateringSnapshot& right = second.snapshots[index];

            require (left.state == right.state && left.reason == right.reason &&
                         left.requestedPump == right.requestedPump &&
                         left.stateSince == right.stateSince,
                     "identical replay emits identical snapshots");
        }
    }
} // namespace

int main ()
{
    testLifecycleAndConfiguration                          ();
    testInitializationFailureRollsBack                     ();
    testGoldenTraceAndNoDuplicateCommands                  ();
    testExactMoistureThresholds                            ();
    testValidSampleRecoversSensorFaultIntoFreshOffInterval ();
    testInvalidSamplesFromEveryReachableState              ();
    testPrecedenceAndOperatorInhibit                       ();
    testSampleFaultsAndRollover                            ();
    testOutputFailureIsStickyUntilReinitialization         ();
    testShutdownAndDestructionFromEveryState               ();
    testIdenticalReplay                                    ();
}
