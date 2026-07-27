#include "environmental_station.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>

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

    struct FakeClimateSensor final : ClimateSensor
    {
        Status        initializeStatus = Status::Ok;
        Status        updateStatus     = Status::Ok;
        ClimateSample current = {2150, 480, TimePoint (0), ClimateSampleState::Valid};
        uint32_t      initializeCount = 0;
        uint32_t      shutdownCount   = 0;
        uint32_t      updateCount     = 0;
        bool          active          = false;

        ~FakeClimateSensor () noexcept override
        {
        }

        Status initialize () noexcept override
        {
            ++initializeCount;
            active = initializeStatus == Status::Ok;
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

        Status update (TimePoint now) noexcept override
        {
            ++updateCount;

            if (current.state == ClimateSampleState::Valid)
            {
                current.observedAt = now;
            }

            return updateStatus;
        }

        ClimateSample sample (TimePoint now,
                              Duration  staleAfter) const noexcept override
        {
            ClimateSample result = current;

            if (result.state == ClimateSampleState::Valid &&
                now.elapsedSince (result.observedAt) > staleAfter)
            {
                result.state = ClimateSampleState::Stale;
            }

            return result;
        }
    };

    EnvironmentalStationConfig testConfig ()
    {
        EnvironmentalStationConfig config;

        config.samplePeriod = Duration (2000);
        config.staleAfter   = Duration (5000);

        return config;
    }

    void testLifecycleAndConfiguration ()
    {
        FakeClimateSensor    sensor;
        EnvironmentalStation station (sensor, testConfig ());

        require (!station.initialized (), "construction is inert");
        require (station.update (TimePoint (0)) == Status::NotInitialized,
                 "update before initialize");
        require (station.initialize () == Status::Ok, "initialize");
        require (station.initialized (), "initialized state");
        require (sensor.initializeCount == 1, "sensor acquired once");
        require (station.initialize () == Status::Ok, "repeat initialize");
        require (sensor.initializeCount == 1, "repeat does not reacquire");

        station.shutdown             ();
        require                      (!station.initialized (),
                                      "shutdown clears lifecycle");
        require                      (!sensor.active, "shutdown releases sensor");
        require                      (station.snapshot ().status ==
                                          Status::NotInitialized,
                                      "shutdown snapshot");
        station.shutdown             ();
        require                      (station.initialize () == Status::Ok,
                                      "reinitialize");

        EnvironmentalStationConfig invalid = testConfig ();

        invalid.samplePeriod = Duration (0);

        FakeClimateSensor    unusedSensor;
        EnvironmentalStation invalidStation (unusedSensor, invalid);

        require (invalidStation.initialize () == Status::InvalidArgument,
                 "zero period rejected");
        require (unusedSensor.initializeCount == 0,
                 "invalid config does not touch sensor");

        invalid = testConfig ();

        invalid.staleAfter = Duration (0x80000000U);

        FakeClimateSensor    ambiguousSensor;
        EnvironmentalStation ambiguousStation (ambiguousSensor, invalid);

        require (ambiguousStation.initialize () == Status::InvalidArgument,
                 "ambiguous freshness rejected");
    }

    void testAcquisitionRollbackAndDestruction ()
    {
        FakeClimateSensor sensor;

        sensor.initializeStatus = Status::ResourceBusy;

        {
            EnvironmentalStation station (sensor, testConfig ());

            require (station.initialize () == Status::ResourceBusy,
                     "sensor acquisition failure reported");
            require (!station.initialized (), "failed station remains inert");
            require (sensor.shutdownCount == 1, "failed acquisition rolls back");
        }

        require (sensor.shutdownCount == 2, "destructor cleanup is safe");

        sensor.initializeStatus = Status::Ok;

        {
            EnvironmentalStation station (sensor, testConfig ());

            require (station.initialize () == Status::Ok,
                     "sensor reusable after rollback");
        }

        require (!sensor.active, "destructor releases active sensor");
    }

    void testCadenceAndStableRecords ()
    {
        FakeClimateSensor    sensor;
        EnvironmentalStation station (sensor, testConfig ());

        require (station.initialize () == Status::Ok, "cadence initialize");
        require (station.update (TimePoint (100)) == Status::Ok, "first observation");

        EnvironmentalSnapshot snapshot = station.snapshot ();

        require (snapshot.recordReady, "first record ready");
        require (snapshot.record.sequence == 1, "first sequence");
        require (snapshot.record.health == EnvironmentalHealth::Healthy,
                 "healthy record");
        require (snapshot.record.recordedAt == TimePoint (100), "record timestamp");
        require (snapshot.nextSampleAt == TimePoint (2100), "next deadline");
        require (sensor.updateCount == 1, "one sensor update");

        require  (station.update (TimePoint (2099)) == Status::Ok,
                  "before deadline");
        snapshot = station.snapshot ();

        require  (!snapshot.recordReady, "no early record event");
        require  (snapshot.record.sequence == 1, "record remains stable");
        require  (sensor.updateCount == 1, "no early sensor update");

        sensor.current.temperatureCentiCelsius = 2275;
        require  (station.update (TimePoint (2100)) == Status::Ok,
                  "exact deadline");
        snapshot = station.snapshot ();

        require  (snapshot.recordReady, "deadline record event");
        require  (snapshot.record.sequence == 2, "second sequence");
        require  (snapshot.record.sample.temperatureCentiCelsius == 2275,
                  "new sample captured");

        require (station.update (TimePoint (10000)) == Status::Ok, "late update");
        require (sensor.updateCount == 3, "late update does not catch up");
        require (station.snapshot ().nextSampleAt == TimePoint (12000),
                 "late update schedules from observation");
    }

    void testHealthRecordsAndRecovery ()
    {
        FakeClimateSensor    sensor;
        EnvironmentalStation station (sensor, testConfig ());

        require (station.initialize () == Status::Ok, "health initialize");

        sensor.current.state = ClimateSampleState::Unavailable;
        require (station.update (TimePoint (0)) == Status::Ok, "starting record");
        require (station.snapshot ().record.health ==
                     EnvironmentalHealth::Starting,
                 "unavailable sample is starting");

        sensor.current.state = ClimateSampleState::ChecksumFailure;
        require (station.update (TimePoint (2000)) == Status::Ok,
                 "invalid sample can complete transport");
        require (station.snapshot ().record.health == EnvironmentalHealth::SensorFault,
                 "invalid sample is explicit");

        sensor.current.state = ClimateSampleState::Stale;
        require (station.update (TimePoint (4000)) == Status::Ok, "stale record");
        require (station.snapshot ().record.health == EnvironmentalHealth::Stale,
                 "stale distinguished");

        sensor.current.state = ClimateSampleState::Valid;
        sensor.updateStatus  = Status::HardwareFailure;
        require (station.update (TimePoint (6000)) == Status::HardwareFailure,
                 "transport failure returned");
        require (station.snapshot ().record.health == EnvironmentalHealth::SensorFault,
                 "transport failure recorded");
        require (station.snapshot ().record.sensorStatus == Status::HardwareFailure,
                 "transport status preserved");

        sensor.updateStatus = Status::Ok;
        require (station.update (TimePoint (8000)) == Status::Ok, "healthy recovery");
        require (station.snapshot ().record.health == EnvironmentalHealth::Healthy,
                 "recovery visible");
    }

    void testTimingResetAndRollover ()
    {
        FakeClimateSensor    sensor;
        EnvironmentalStation station (sensor, testConfig ());

        require (station.initialize () == Status::Ok, "timing initialize");
        require (station.update (TimePoint (0xfffffff0U)) == Status::Ok,
                 "rollover anchor");
        require (station.snapshot ().nextSampleAt == TimePoint (1984),
                 "deadline wraps");
        require (station.update (TimePoint (1984)) == Status::Ok, "wrapped deadline");
        require (station.snapshot ().record.sequence == 2, "wrapped record");

        require (station.update (TimePoint (0x800007d0U)) == Status::InvalidArgument,
                 "ambiguous reverse time rejected");
        require (station.snapshot ().record.health == EnvironmentalHealth::TimingFault,
                 "timing fault visible");
        require (!station.snapshot ().hasDeadline, "timing fault cancels schedule");

        require (station.reset () == Status::Ok, "explicit reset");
        require (sensor.initializeCount == 2, "reset restarts sensor schedule");
        require (station.snapshot ().record.sequence == 0, "reset clears records");
        require (station.update (TimePoint (10)) == Status::Ok, "update after reset");
    }

    void testDeterministicReplay ()
    {
        const RecordedClimateFrame frames[] = {
            {TimePoint (0),
             {2100, 450, TimePoint (0), ClimateSampleState::Valid},
             Status::Ok},
            {TimePoint (2000),
             {0, 0, TimePoint (2000), ClimateSampleState::ChecksumFailure},
             Status::Ok},
            {TimePoint (4000),
             {2200, 500, TimePoint (4000), ClimateSampleState::Valid},
             Status::Ok}};
        RecordedClimateSensor firstSensor  (frames, 3);
        RecordedClimateSensor secondSensor (frames, 3);
        EnvironmentalStation  first        (firstSensor, testConfig ());
        EnvironmentalStation  second       (secondSensor, testConfig ());
        const TimePoint       times[] = {
            TimePoint (0),
            TimePoint (1000),
            TimePoint (2000),
            TimePoint (4000)};

        require (first.initialize () == Status::Ok, "first replay initialize");
        require (second.initialize () == Status::Ok, "second replay initialize");

        for (size_t index = 0; index < 4; ++index)
        {
            require (first.update (times[index]) == second.update (times[index]),
                     "replay status");

            const EnvironmentalSnapshot left  = first.snapshot  ();
            const EnvironmentalSnapshot right = second.snapshot ();

            require (left.record.sequence == right.record.sequence, "replay sequence");
            require (left.record.sample.state == right.record.sample.state,
                     "replay sample state");
            require (left.record.health == right.record.health, "replay health");
            require (left.record.recordedAt == right.record.recordedAt,
                     "replay timestamp");
            require (left.recordReady == right.recordReady, "replay event");
        }
    }
} // namespace

int main ()
{
    testLifecycleAndConfiguration          ();
    testAcquisitionRollbackAndDestruction  ();
    testCadenceAndStableRecords            ();
    testHealthRecordsAndRecovery           ();
    testTimingResetAndRollover             ();
    testDeterministicReplay                ();

    static_assert (!std::is_copy_constructible<EnvironmentalStation>::value,
                   "station owns the sensor lifecycle");
    static_assert (!std::is_move_constructible<EnvironmentalStation>::value,
                   "station relationship has a stable address");

    std::cout << "environmental station tests passed\n";
    return 0;
}
