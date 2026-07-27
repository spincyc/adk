#include "greenhouse_controller.h"

#include <Arduino.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {

    using namespace adk;
    namespace fake = adk::test::arduino;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    struct RecordingPump final : PumpOutput
    {
        Status    initializeStatus = StatusCode::Ok;
        Status    commandStatus    = StatusCode::Ok;
        PumpState current          = PumpState::Off;
        uint32_t  initializeCount  = 0;
        uint32_t  shutdownCount    = 0;
        uint32_t  commandCount     = 0;
        bool      active           = false;

        ~RecordingPump () noexcept override
        {
        }

        Status initialize () noexcept override
        {
            ++initializeCount;
            active = initializeStatus.ok ();
            return initializeStatus;
        }

        void shutdown () noexcept override
        {
            ++shutdownCount;
            current = PumpState::Off;
            active  = false;
        }

        Status setState (PumpState state) noexcept override
        {
            ++commandCount;

            if (commandStatus.ok ())
            {
                current = state;
            }

            return commandStatus;
        }

        PumpState state () const noexcept override
        {
            return current;
        }

        bool initialized () const noexcept override
        {
            return active;
        }
    };

    struct RecordingDisplay final : CharacterDisplay
    {
        Status   initializeStatus = StatusCode::Ok;
        Status   updateStatus     = StatusCode::Ok;
        Status   showStatus       = StatusCode::Ok;
        uint32_t initializeCount  = 0;
        uint32_t shutdownCount    = 0;
        uint32_t updateCount      = 0;
        uint32_t showCount        = 0;
        char     lines[2][17]     = {{0}, {0}};
        bool     active           = false;

        ~RecordingDisplay () noexcept override
        {
        }

        Status initialize () noexcept override
        {
            ++initializeCount;
            active = initializeStatus.ok ();
            return initializeStatus;
        }

        void shutdown () noexcept override
        {
            ++shutdownCount;
            active = false;
        }

        bool initialized () const noexcept override
        {
            return active;
        }

        Status update (TimePoint) noexcept override
        {
            ++updateCount;
            return updateStatus;
        }

        bool ready () const noexcept override
        {
            return active;
        }

        Status show (uint8_t row, const char* text) noexcept override
        {
            ++showCount;

            if (row >= 2U)
            {
                return StatusCode::InvalidArgument;
            }

            if (showStatus.ok ())
            {
                std::memcpy (lines[row], text, 17);
            }

            return showStatus;
        }

        const char* line (uint8_t row) const noexcept override
        {
            return row < 2U ? lines[row] : "";
        }
    };

    struct RecordingSink final : RecordSink
    {
        Status                    initializeStatus = StatusCode::Ok;
        Status                    appendStatus     = StatusCode::Ok;
        uint32_t                  initializeCount  = 0;
        uint32_t                  shutdownCount    = 0;
        uint32_t                  appendCount      = 0;
        bool                      active           = false;
        std::vector<std::string> records;

        ~RecordingSink () noexcept override
        {
        }

        Status initialize () noexcept override
        {
            ++initializeCount;
            active = initializeStatus.ok ();
            return initializeStatus;
        }

        void shutdown () noexcept override
        {
            ++shutdownCount;
            active = false;
        }

        Status append (const StableRecord& record) noexcept override
        {
            ++appendCount;

            if (appendStatus.ok ())
            {
                records.emplace_back (record.text, record.length);
            }

            return appendStatus;
        }

        bool initialized () const noexcept override
        {
            return active;
        }
    };

    struct Fixture
    {
        ResourceRegistry resources;
        AnalogInput      analog;
        MoistureSensor   moisture;
        RecordingPump    pump;
        WateringController watering;
        RecordingDisplay display;
        RecordingSink    records;
        GreenhouseController greenhouse;

        explicit Fixture (
            const GreenhouseConfig& greenhouseConfig = {
                Duration (100), Duration (200), Duration (300), Duration (250)},
            const WateringConfig& wateringConfig = {
                350, 600, Duration (5000), Duration (1000)})
            : resources  ()
            , analog     (resources, 54)
            , moisture   (analog, {900, 100, 10})
            , pump       ()
            , watering   (wateringConfig, pump)
            , display    ()
            , records    ()
            , greenhouse (greenhouseConfig,
                          moisture,
                          watering,
                          display,
                          records)
        {
        }
    };

    void requireInvalidConfig (const GreenhouseConfig& config,
                               const char*             message)
    {
        fake::reset ();

        ResourceRegistry     resources;
        AnalogInput          analog   (resources, 54);
        MoistureSensor       moisture (analog, {900, 100, 10});
        RecordingPump        pump;
        WateringController   watering (
            {350, 600, Duration (5000), Duration (1000)}, pump);
        RecordingDisplay     display;
        RecordingSink        records;
        GreenhouseController greenhouse (
            config, moisture, watering, display, records);

        fake::setAnalogInput (54, 500);
        require              (greenhouse.initialize ().error () ==
                                  StatusCode::InvalidArgument,
                              message);
        require              (!analog.initialized (),
                              "invalid config does not acquire input");
        require              (pump.initializeCount == 0,
                              "invalid config does not acquire output");
        require              (display.initializeCount == 0,
                              "invalid config does not acquire display");
        require              (records.initializeCount == 0,
                              "invalid config does not acquire sink");
    }

    void testConfigurationBoundaries ()
    {
        GreenhouseConfig config = {Duration (100), Duration (200),
                                   Duration (300), Duration (250)};

        config.sampleInterval = Duration (0);

        requireInvalidConfig (config, "zero sample interval rejected");

        config = {Duration (100), Duration (0), Duration (300), Duration (250)};

        requireInvalidConfig (config, "zero display interval rejected");

        config = {Duration (100), Duration (200), Duration (0), Duration (250)};

        requireInvalidConfig (config, "zero record interval rejected");

        config = {Duration (100), Duration (200), Duration (300), Duration (99)};

        requireInvalidConfig (config, "staleness shorter than sample rejected");

        config = {Duration (0x80000000UL), Duration (200), Duration (300),
                  Duration (0x80000000UL)};
        requireInvalidConfig (config, "ambiguous interval rejected");
    }

    void testLifecycleAndRollback ()
    {
        fake::reset ();
        Fixture fixture;

        fake::setAnalogInput (54, 500);
        require              (!fixture.greenhouse.initialized (),
                              "construction is inert");
        require              (fixture.greenhouse.observe (TimePoint (0)).error () ==
                     StatusCode::NotInitialized,
                 "inactive observation rejected");
        require              (fixture.greenhouse.initialize ().ok (), "initialize");
        require              (fixture.greenhouse.initialize ().ok (),
                              "repeat initialize");
        require              (fixture.pump.initializeCount == 1,
                              "pump initialized once");
        require              (fixture.display.initializeCount == 1,
                              "display initialized once");
        require              (fixture.records.initializeCount == 1,
                              "sink initialized once");

        fixture.greenhouse.shutdown ();
        require                     (!fixture.greenhouse.initialized (),
                                     "shutdown lifecycle");
        require                     (fixture.pump.current == PumpState::Off,
                                     "shutdown selects off");
        require                     (!fixture.analog.initialized (),
                                     "shutdown releases sensor input");
        fixture.greenhouse.shutdown ();

        fake::reset ();
        Fixture failed;

        failed.records.initializeStatus = StatusCode::ResourceBusy;
        fake::setAnalogInput           (54, 500);
        require                        (failed.greenhouse.initialize ().error () ==
                    StatusCode::ResourceBusy,
                "record acquisition failure returned");
        require (!failed.analog.initialized (), "failure rolls back moisture");
        require (!failed.pump.active, "failure rolls back watering");
        require (!failed.display.active, "failure rolls back display");
    }

    void testInitializationFailureAtEveryDependency ()
    {
        fake::reset ();
        ResourceRegistry resources;
        AnalogInput      blocker  (resources, 54);
        AnalogInput      analog   (resources, 54);
        MoistureSensor   moisture (analog, {900, 100, 10});
        RecordingPump    pump;
        WateringController watering (
            {350, 600, Duration (5000), Duration (1000)}, pump);
        RecordingDisplay display;
        RecordingSink    records;
        GreenhouseController greenhouse (
            {Duration (100), Duration (200), Duration (300), Duration (250)},
            moisture,
            watering,
            display,
            records);

        fake::setAnalogInput (54, 500);
        require              (blocker.initialize ().ok (), "claim moisture pin");
        require              (greenhouse.initialize ().error () ==
                                  StatusCode::ResourceBusy,
                              "moisture initialization failure");
        require              (pump.initializeCount == 0,
                              "moisture failure stops acquisition");

        fake::reset ();
        Fixture wateringFailure;

        wateringFailure.pump.initializeStatus = StatusCode::HardwareFailure;
        fake::setAnalogInput                  (54, 500);
        require                               (
            wateringFailure.greenhouse.initialize ().error () ==
                StatusCode::HardwareFailure,
            "watering initialization failure");
        require (!wateringFailure.analog.initialized (),
                 "watering failure releases moisture");
        require (wateringFailure.display.initializeCount == 0,
                 "watering failure stops before display");

        fake::reset ();
        Fixture displayFailure;

        displayFailure.display.initializeStatus = StatusCode::Unsupported;
        fake::setAnalogInput                    (54, 500);
        require                                 (
            displayFailure.greenhouse.initialize ().error () ==
                StatusCode::Unsupported,
            "display initialization failure");
        require (!displayFailure.analog.initialized (),
                 "display failure releases moisture");
        require (!displayFailure.pump.active,
                 "display failure releases watering");
        require (displayFailure.records.initializeCount == 0,
                 "display failure stops before records");
    }

    void testDisplayRecoveryAndSimultaneousPrecedence ()
    {
        fake::reset ();
        Fixture fixture;

        fake::setAnalogInput (54, 700);
        require              (fixture.greenhouse.initialize ().ok (), "initialize");
        require              (fixture.greenhouse.observe (TimePoint (0)).ok (),
                              "observe");
        require              (fixture.greenhouse.decide (TimePoint (0), {true}).ok (),
                              "decide");
        require              (fixture.greenhouse.actuate (TimePoint (0)).ok (),
                              "actuate");

        fixture.display.showStatus = StatusCode::HardwareFailure;
        require (fixture.greenhouse.present (TimePoint (200)).error () ==
                     StatusCode::HardwareFailure,
                 "display failure returned");
        require (fixture.greenhouse.snapshot ().mode == GreenhouseMode::DisplayFault,
                 "display failure visible");

        fixture.display.showStatus = StatusCode::Ok;
        require (fixture.greenhouse.present (TimePoint (200)).ok (),
                 "display retry");
        require (fixture.greenhouse.snapshot ().mode == GreenhouseMode::Monitoring,
                 "display recovers");

        fake::setAnalogInput (54, 700);
        require              (fixture.greenhouse.observe (TimePoint (1000)).ok (),
                              "refresh dry sample");
        require              (fixture.greenhouse.decide (TimePoint (1000), {true}).ok (),
                              "request watering");
        fixture.pump.commandStatus = StatusCode::HardwareFailure;
        require (fixture.greenhouse.actuate (TimePoint (1000)).error () ==
                     StatusCode::HardwareFailure,
                 "output failure");
        fixture.display.showStatus = StatusCode::HardwareFailure;
        require (fixture.greenhouse.present (TimePoint (1000)).error () ==
                     StatusCode::HardwareFailure,
                 "simultaneous display failure");
        require (fixture.greenhouse.snapshot ().mode == GreenhouseMode::MultipleFaults,
                 "output plus display is multiple fault");
    }

    void testRolloverPrimeSchedulesAndCoalescing ()
    {
        const GreenhouseConfig config = {
            Duration (101), Duration (103), Duration (107), Duration (211)};
        Fixture         fixture (config);
        const TimePoint start   (0xfffffff0UL);

        fake::reset ();

        fake::setAnalogInput (54, 500);
        require              (fixture.greenhouse.initialize ().ok (), "initialize");
        require              (fixture.greenhouse.observe (start).ok (), "anchor");
        require              (fixture.greenhouse.decide (start, {true}).ok (),
                              "decide");
        require              (fixture.greenhouse.actuate (start).ok (), "actuate");
        require              (fixture.greenhouse.present (TimePoint (86)).ok (),
                              "one tick before display rollover boundary");
        require              (fixture.display.showCount == 0, "no early display");
        require              (fixture.greenhouse.present (TimePoint (87)).ok (),
                              "display rollover boundary");
        require              (fixture.display.showCount == 2, "display is due once");
        require              (fixture.greenhouse.record (TimePoint (90)).ok (),
                              "record not yet due");
        require              (fixture.records.appendCount == 0, "no early record");
        require              (fixture.greenhouse.record (TimePoint (91)).ok (),
                              "record rollover boundary");
        require              (fixture.records.appendCount == 1, "record due once");

        fake::setAnalogInput (54, 600);
        require              (fixture.greenhouse.observe (TimePoint (1000)).ok (),
                              "delayed observation coalesces");
        require              (fixture.greenhouse.observe (TimePoint (1000)).ok (),
                              "repeat delayed observation");
        require              (fixture.greenhouse.snapshot ().moisture.rawReading == 600,
                              "latest coalesced sample");
    }

    void testScheduleDecisionAndInhibition ()
    {
        fake::reset ();
        Fixture fixture;

        fake::setAnalogInput (54, 700);
        require              (fixture.greenhouse.initialize ().ok (), "initialize");
        require              (fixture.greenhouse.observe (TimePoint (0)).ok (),
                 "first sample immediate");
        require (fixture.greenhouse.decide (TimePoint (0), {true}).ok (),
                 "first decision");
        require (fixture.greenhouse.actuate (TimePoint (0)).ok (),
                 "first actuation");
        require (fixture.greenhouse.snapshot ().mode == GreenhouseMode::Monitoring,
                 "starts monitoring");

        fake::setAnalogInput (54, 700);
        require              (fixture.greenhouse.observe (TimePoint (1000)).ok (),
                 "delayed sample coalesces");
        require (fixture.greenhouse.decide (TimePoint (1000), {true}).ok (),
                 "dry decision");
        require (fixture.greenhouse.snapshot ().watering.requestedPump ==
                     PumpState::On,
                 "dry sample requests watering");
        require (fixture.greenhouse.actuate (TimePoint (1000)).ok (),
                 "watering applied");
        require (fixture.greenhouse.snapshot ().mode == GreenhouseMode::Watering,
                 "watering mode");

        require (fixture.greenhouse.decide (TimePoint (1100), {false}).ok (),
                 "operator inhibit");
        require (fixture.greenhouse.actuate (TimePoint (1100)).ok (),
                 "inhibition applied");
        require (fixture.pump.current == PumpState::Off, "inhibit forces off");
        require (fixture.greenhouse.snapshot ().mode == GreenhouseMode::Inhibited,
                 "inhibition visible");
    }

    void testWetHysteresisMaximumAndSensorRecovery ()
    {
        const WateringConfig wateringConfig = {
            350, 600, Duration (500), Duration (100)};
        Fixture fixture (
            {Duration (100), Duration (200), Duration (300), Duration (250)},
            wateringConfig);

        fake::reset ();

        fake::setAnalogInput (54, 700);
        require              (fixture.greenhouse.initialize ().ok (), "initialize");
        require              (fixture.greenhouse.observe (TimePoint (0)).ok (),
                              "anchor sample");
        require              (fixture.greenhouse.decide (TimePoint (0), {true}).ok (),
                              "anchor decision");
        require              (fixture.greenhouse.actuate (TimePoint (0)).ok (),
                              "anchor actuation");

        require (fixture.greenhouse.observe (TimePoint (100)).ok (), "dry sample");
        require (fixture.greenhouse.decide (TimePoint (100), {true}).ok (),
                 "dry decision");
        require (fixture.greenhouse.actuate (TimePoint (100)).ok (),
                 "dry actuation");
        require (fixture.pump.current == PumpState::On, "dry starts watering");

        fake::setAnalogInput (54, 500);
        require              (fixture.greenhouse.observe (TimePoint (200)).ok (),
                              "hysteresis sample");
        require              (fixture.greenhouse.decide (TimePoint (200), {true}).ok (),
                              "hysteresis decision");
        require              (fixture.greenhouse.actuate (TimePoint (200)).ok (),
                              "hysteresis actuation");
        require              (fixture.pump.current == PumpState::On,
                              "hysteresis preserves watering");

        fake::setAnalogInput (54, 400);
        require              (fixture.greenhouse.observe (TimePoint (300)).ok (),
                              "wet sample");
        require              (fixture.greenhouse.decide (TimePoint (300), {true}).ok (),
                              "wet decision");
        require              (fixture.greenhouse.actuate (TimePoint (300)).ok (),
                              "wet actuation");
        require              (fixture.pump.current == PumpState::Off,
                              "wet threshold stops watering");

        fake::setAnalogInput (54, 1023);
        require              (fixture.greenhouse.observe (TimePoint (400)).ok (),
                              "fault sample");
        require              (fixture.greenhouse.decide (TimePoint (400), {true}).ok (),
                              "fault decision");
        require              (fixture.greenhouse.actuate (TimePoint (400)).ok (),
                              "fault actuation");
        require              (fixture.greenhouse.snapshot ().mode ==
                                  GreenhouseMode::SensorFault,
                              "range fault is visible");

        fake::setAnalogInput (54, 500);
        require              (fixture.greenhouse.observe (TimePoint (500)).ok (),
                              "recovery sample");
        require              (fixture.greenhouse.decide (TimePoint (500), {true}).ok (),
                              "recovery decision");
        require              (fixture.greenhouse.actuate (TimePoint (500)).ok (),
                              "recovery actuation");
        require              (fixture.greenhouse.snapshot ().mode ==
                                  GreenhouseMode::Monitoring,
                              "valid sample recovers monitoring");

        fake::setAnalogInput (54, 700);
        require              (fixture.greenhouse.observe (TimePoint (600)).ok (),
                              "second dry sample");
        require              (fixture.greenhouse.decide (TimePoint (600), {true}).ok (),
                              "second dry decision");
        require              (fixture.greenhouse.actuate (TimePoint (600)).ok (),
                              "second dry actuation");
        require              (fixture.pump.current == PumpState::On,
                              "watering restarts after minimum off");

        require (fixture.greenhouse.observe (TimePoint (1100)).ok (),
                 "maximum-on sample");
        require (fixture.greenhouse.decide (TimePoint (1100), {true}).ok (),
                 "maximum-on decision");
        require (fixture.greenhouse.actuate (TimePoint (1100)).ok (),
                 "maximum-on actuation");
        require (fixture.greenhouse.snapshot ().watering.state ==
                     WateringState::LockedOut,
                 "maximum-on enters lockout");
        require (fixture.pump.current == PumpState::Off, "lockout forces off");
    }

    void testPresentationAndByteIdenticalRecordRetry ()
    {
        fake::reset ();
        Fixture fixture;

        fake::setAnalogInput (54, 500);
        require              (fixture.greenhouse.initialize ().ok (), "initialize");
        require              (fixture.greenhouse.observe (TimePoint (0)).ok (),
                 "observe");
        require (fixture.greenhouse.decide (TimePoint (0), {true}).ok (), "decide");
        require (fixture.greenhouse.actuate (TimePoint (0)).ok (), "actuate");
        require (fixture.greenhouse.present (TimePoint (199)).ok (),
                 "display not yet due");
        require (fixture.display.showCount == 0, "no early display");
        require (fixture.greenhouse.present (TimePoint (200)).ok (),
                 "display boundary");
        require (fixture.display.showCount == 2, "two stable display rows");

        fixture.records.appendStatus = StatusCode::HardwareFailure;
        require (fixture.greenhouse.record (TimePoint (300)).error () ==
                     StatusCode::HardwareFailure,
                 "record failure retained");
        require (fixture.greenhouse.snapshot ().mode == GreenhouseMode::RecordFault,
                 "record failure visible");

        fixture.records.appendStatus = StatusCode::Ok;
        require (fixture.greenhouse.record (TimePoint (300)).ok (),
                 "same-timestamp record retry");
        require (fixture.records.records.size () == 1, "one accepted record");
        require (fixture.records.records[0].find ("adk-gh,1,0,0,valid,500,") == 0,
                 "retry preserves original decision record");
        require (fixture.greenhouse.snapshot ().recordSequence == 1,
                 "sequence advances after append");
    }

    void testStageOrderAndImmutableDecision ()
    {
        fake::reset ();
        Fixture fixture;

        fake::setAnalogInput (54, 500);
        require              (fixture.greenhouse.initialize ().ok (), "initialize");
        require              (fixture.greenhouse.observe (TimePoint (0)).ok (),
                              "observe");
        require (fixture.greenhouse.present (TimePoint (200)).error () ==
                     StatusCode::InvalidArgument,
                 "present requires a decision");
        require (fixture.greenhouse.record (TimePoint (300)).error () ==
                     StatusCode::InvalidArgument,
                 "record requires a decision");
        require (fixture.greenhouse.decide (TimePoint (0), {true}).ok (),
                 "decision");
        require (fixture.greenhouse.decide (TimePoint (1), {true}).error () ==
                     StatusCode::InvalidArgument,
                 "decision cannot replace unapplied intent");
        require (fixture.greenhouse.observe (TimePoint (100)).error () ==
                     StatusCode::InvalidArgument,
                 "observation cannot split decision and actuation");
        require (fixture.greenhouse.actuate (TimePoint (0)).ok (), "actuation");

        fake::setAnalogInput (54, 700);
        require              (fixture.greenhouse.observe (TimePoint (1000)).ok (),
                              "later observation");
        require (fixture.greenhouse.record (TimePoint (1000)).ok (),
                 "record decided payload");
        require (fixture.records.records.size () == 1, "one record");
        require (fixture.records.records[0].find ("valid,500,500,") !=
                     std::string::npos,
                 "record remains bound to applied decision");
    }

    void testOutputFaultLatchesAndSuppressesWatering ()
    {
        fake::reset ();
        Fixture fixture;

        fake::setAnalogInput (54, 700);
        require              (fixture.greenhouse.initialize ().ok (), "initialize");
        require              (fixture.greenhouse.observe (TimePoint (0)).ok (),
                 "observe");
        require (fixture.greenhouse.decide (TimePoint (0), {true}).ok (), "anchor");
        require (fixture.greenhouse.actuate (TimePoint (0)).ok (), "anchor off");
        require (fixture.greenhouse.observe (TimePoint (1000)).ok (), "refresh");
        require (fixture.greenhouse.decide (TimePoint (1000), {true}).ok (),
                 "watering decision");

        fixture.pump.commandStatus = StatusCode::HardwareFailure;
        require (fixture.greenhouse.actuate (TimePoint (1000)).error () ==
                     StatusCode::HardwareFailure,
                 "output failure returned");
        require (fixture.greenhouse.snapshot ().mode == GreenhouseMode::OutputFault,
                 "output failure dominates");

        fixture.pump.commandStatus = StatusCode::Ok;
        require (fixture.greenhouse.decide (TimePoint (2000), {true}).error () ==
                     StatusCode::HardwareFailure,
                 "latched watering fault retained");
        require (fixture.greenhouse.snapshot ().watering.requestedPump ==
                     PumpState::Off,
                 "latched fault preserves off");
    }

    void testStaleSampleSuppressesWatering ()
    {
        fake::reset ();
        Fixture fixture;

        fake::setAnalogInput (54, 700);
        require              (fixture.greenhouse.initialize ().ok (), "initialize");
        require              (fixture.greenhouse.observe (TimePoint (0)).ok (),
                              "observe");
        require              (fixture.greenhouse.decide (TimePoint (0), {true}).ok (),
                              "anchor");
        require              (fixture.greenhouse.actuate (TimePoint (0)).ok (),
                              "anchor actuation");
        require              (fixture.greenhouse.observe (TimePoint (1000)).ok (),
                              "refresh");
        require              (fixture.greenhouse.decide (TimePoint (1000), {true}).ok (),
                              "watering decision");
        require              (fixture.greenhouse.actuate (TimePoint (1000)).ok (),
                              "watering starts");
        require              (fixture.pump.current == PumpState::On,
                              "pump intent is on");

        require (fixture.greenhouse.decide (TimePoint (1251), {true}).ok (),
                 "stale decision");
        require (fixture.greenhouse.actuate (TimePoint (1251)).ok (),
                 "stale state actuated");
        require (fixture.pump.current == PumpState::Off,
                 "stale sample forces off");
        require (fixture.greenhouse.snapshot ().mode == GreenhouseMode::SensorFault,
                 "staleness is visible");
    }

    struct ReplayResult
    {
        std::string    record;
        GreenhouseMode mode;
        PumpState      pump;
        uint32_t       commands;
    };

    ReplayResult runReplay ()
    {
        fake::reset ();
        Fixture fixture;

        fake::setAnalogInput (54, 500);
        require              (fixture.greenhouse.initialize ().ok (),
                              "replay initialize");
        require              (fixture.greenhouse.observe (TimePoint (0)).ok (),
                              "replay observe zero");
        require              (fixture.greenhouse.decide (TimePoint (0), {true}).ok (),
                              "replay decide zero");
        require              (fixture.greenhouse.actuate (TimePoint (0)).ok (),
                              "replay actuate zero");

        fake::setAnalogInput (54, 700);
        require              (fixture.greenhouse.observe (TimePoint (1000)).ok (),
                              "replay observe dry");
        require              (fixture.greenhouse.decide (TimePoint (1000), {true}).ok (),
                              "replay decide dry");
        require              (fixture.greenhouse.actuate (TimePoint (1000)).ok (),
                              "replay actuate dry");
        require              (fixture.greenhouse.record (TimePoint (1000)).ok (),
                              "replay record");

        const GreenhouseSnapshot snapshot = fixture.greenhouse.snapshot ();

        return {fixture.records.records[0],
                snapshot.mode,
                fixture.pump.current,
                fixture.pump.commandCount};
    }

    void testReplayIsByteIdentical ()
    {
        const ReplayResult first  = runReplay ();
        const ReplayResult second = runReplay ();

        require (first.record == second.record, "replay record bytes");
        require (first.mode == second.mode, "replay mode");
        require (first.pump == second.pump, "replay pump intent");
        require (first.commands == second.commands, "replay command trace");
    }

    void testBoundedSafetyProperties ()
    {
        const uint16_t readings[] = {0, 100, 500, 700, 900, 1023};

        for (uint16_t reading : readings)
        {
            for (uint8_t allowed = 0; allowed < 2; ++allowed)
            {
                fake::reset ();
                Fixture fixture;

                fake::setAnalogInput (54, reading);
                require              (fixture.greenhouse.initialize ().ok (),
                                      "property initialize");
                require              (fixture.greenhouse.observe (TimePoint (0)).ok (),
                                      "property anchor");
                require              (
                    fixture.greenhouse.decide (TimePoint (0), {allowed != 0}).ok (),
                    "property first decision");
                require (fixture.greenhouse.actuate (TimePoint (0)).ok (),
                         "property first actuation");
                require (fixture.pump.current == PumpState::Off,
                         "first decision is always safe");

                fake::setAnalogInput (54, reading);
                require              (fixture.greenhouse.observe (TimePoint (1000)).ok (),
                                      "property sample");
                require              (
                    fixture.greenhouse.decide (TimePoint (1000), {allowed != 0})
                        .ok (),
                    "property decision");
                require (fixture.greenhouse.actuate (TimePoint (1000)).ok (),
                         "property actuation");

                const MoistureSample sample =
                    fixture.greenhouse.snapshot ().moisture;

                if (allowed == 0 || sample.state != MoistureSampleState::Valid)
                {
                    require (fixture.pump.current == PumpState::Off,
                             "invalid or inhibited input remains off");
                }
            }
        }
    }
}

int main ()
{
    testConfigurationBoundaries                  ();
    testLifecycleAndRollback                     ();
    testInitializationFailureAtEveryDependency   ();
    testDisplayRecoveryAndSimultaneousPrecedence ();
    testRolloverPrimeSchedulesAndCoalescing      ();
    testScheduleDecisionAndInhibition            ();
    testWetHysteresisMaximumAndSensorRecovery    ();
    testPresentationAndByteIdenticalRecordRetry  ();
    testStageOrderAndImmutableDecision           ();
    testOutputFaultLatchesAndSuppressesWatering  ();
    testStaleSampleSuppressesWatering            ();
    testReplayIsByteIdentical                    ();
    testBoundedSafetyProperties                  ();

    std::cout << "greenhouse controller tests passed\n";
    return EXIT_SUCCESS;
}
