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

    void requireError (Status status, StatusCode expected, const char* message)
    {
        require (status.error () == expected, message);
    }

    struct FakeClimateSensor final : ClimateSensor
    {
        Status        initializeStatus = StatusCode::Ok;
        Status        updateStatus     = StatusCode::Ok;
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

        require      (!station.initialized (), "construction is inert");
        requireError (station.update (TimePoint (0)),
                      StatusCode::NotInitialized,
                      "update before initialize");
        requireError (station.initialize (), StatusCode::Ok, "initialize");
        require      (station.initialized (), "initialized state");
        require      (sensor.initializeCount == 1, "sensor acquired once");
        requireError (station.initialize (), StatusCode::Ok, "repeat initialize");
        require      (sensor.initializeCount == 1, "repeat does not reacquire");

        station.shutdown             ();
        require                      (!station.initialized (),
                                      "shutdown clears lifecycle");
        require                      (!sensor.active, "shutdown releases sensor");
        require                      (station.snapshot ().status.error () ==
                                          StatusCode::NotInitialized,
                                      "shutdown snapshot");
        station.shutdown             ();
        requireError                 (station.initialize (),
                                      StatusCode::Ok,
                                      "reinitialize");

        EnvironmentalStationConfig invalid = testConfig ();

        invalid.samplePeriod = Duration (0);

        FakeClimateSensor    unusedSensor;
        EnvironmentalStation invalidStation (unusedSensor, invalid);

        requireError (invalidStation.initialize (),
                      StatusCode::InvalidArgument,
                      "zero period rejected");
        require (unusedSensor.initializeCount == 0,
                 "invalid config does not touch sensor");

        invalid = testConfig ();

        invalid.staleAfter = Duration (0x80000000U);

        FakeClimateSensor    ambiguousSensor;
        EnvironmentalStation ambiguousStation (ambiguousSensor, invalid);

        requireError (ambiguousStation.initialize (),
                      StatusCode::InvalidArgument,
                      "ambiguous freshness rejected");
    }

    void testAcquisitionRollbackAndDestruction ()
    {
        FakeClimateSensor sensor;

        sensor.initializeStatus = StatusCode::ResourceBusy;

        {
            EnvironmentalStation station (sensor, testConfig ());

            requireError (station.initialize (),
                          StatusCode::ResourceBusy,
                          "sensor acquisition failure reported");
            require (!station.initialized (), "failed station remains inert");
            require (sensor.shutdownCount == 1, "failed acquisition rolls back");
        }

        require (sensor.shutdownCount == 2, "destructor cleanup is safe");

        sensor.initializeStatus = StatusCode::Ok;

        {
            EnvironmentalStation station (sensor, testConfig ());

            requireError (station.initialize (),
                          StatusCode::Ok,
                          "sensor reusable after rollback");
        }

        require (!sensor.active, "destructor releases active sensor");
    }

    void testCadenceAndStableRecords ()
    {
        FakeClimateSensor    sensor;
        EnvironmentalStation station (sensor, testConfig ());

        requireError (station.initialize (), StatusCode::Ok, "cadence initialize");
        requireError (station.update (TimePoint (100)),
                      StatusCode::Ok,
                      "first observation");

        EnvironmentalSnapshot snapshot = station.snapshot ();

        require (snapshot.recordReady, "first record ready");
        require (snapshot.record.sequence == 1, "first sequence");
        require (snapshot.record.health == EnvironmentalHealth::Healthy,
                 "healthy record");
        require (snapshot.record.recordedAt == TimePoint (100), "record timestamp");
        require (snapshot.nextSampleAt == TimePoint (2100), "next deadline");
        require (sensor.updateCount == 1, "one sensor update");

        requireError (station.update (TimePoint (2099)),
                      StatusCode::Ok,
                      "before deadline");
        snapshot = station.snapshot ();

        require  (!snapshot.recordReady, "no early record event");
        require  (snapshot.record.sequence == 1, "record remains stable");
        require  (sensor.updateCount == 1, "no early sensor update");

        sensor.current.temperatureCentiCelsius = 2275;
        requireError (station.update (TimePoint (2100)),
                      StatusCode::Ok,
                      "exact deadline");
        snapshot = station.snapshot ();

        require  (snapshot.recordReady, "deadline record event");
        require  (snapshot.record.sequence == 2, "second sequence");
        require  (snapshot.record.sample.temperatureCentiCelsius == 2275,
                  "new sample captured");

        requireError (station.update (TimePoint (10000)),
                      StatusCode::Ok,
                      "late update");
        require (sensor.updateCount == 3, "late update does not catch up");
        require (station.snapshot ().nextSampleAt == TimePoint (12000),
                 "late update schedules from observation");
    }

    void testHealthRecordsAndRecovery ()
    {
        FakeClimateSensor    sensor;
        EnvironmentalStation station (sensor, testConfig ());

        requireError (station.initialize (), StatusCode::Ok, "health initialize");

        sensor.current.state = ClimateSampleState::Unavailable;
        requireError (station.update (TimePoint (0)),
                      StatusCode::Ok,
                      "starting record");
        require (station.snapshot ().record.health ==
                     EnvironmentalHealth::Starting,
                 "unavailable sample is starting");

        sensor.current.state = ClimateSampleState::ChecksumFailure;
        requireError (station.update (TimePoint (2000)),
                      StatusCode::Ok,
                      "invalid sample can complete transport");
        require (station.snapshot ().record.health == EnvironmentalHealth::SensorFault,
                 "invalid sample is explicit");

        sensor.current.state = ClimateSampleState::Stale;
        requireError (station.update (TimePoint (4000)),
                      StatusCode::Ok,
                      "stale record");
        require (station.snapshot ().record.health == EnvironmentalHealth::Stale,
                 "stale distinguished");

        sensor.current.state = ClimateSampleState::Valid;
        sensor.updateStatus = StatusCode::HardwareFailure;
        requireError (station.update (TimePoint (6000)),
                      StatusCode::HardwareFailure,
                      "transport failure returned");
        require (station.snapshot ().record.health == EnvironmentalHealth::SensorFault,
                 "transport failure recorded");
        require (station.snapshot ().record.sensorStatus.error () ==
                     StatusCode::HardwareFailure,
                 "transport status preserved");

        sensor.updateStatus = StatusCode::Ok;
        requireError (station.update (TimePoint (8000)),
                      StatusCode::Ok,
                      "healthy recovery");
        require (station.snapshot ().record.health == EnvironmentalHealth::Healthy,
                 "recovery visible");

        // A sensor reports an out-of-range reading with the same
        // InvalidArgument status it uses for a timing failure, so only the
        // sample state separates them. The station must not call a rejected
        // reading a timing fault.
        sensor.current.state = ClimateSampleState::TemperatureOutOfRange;
        sensor.updateStatus  = StatusCode::InvalidArgument;
        requireError (station.update (TimePoint (10000)),
                      StatusCode::InvalidArgument,
                      "out-of-range temperature returned");
        require (station.snapshot ().record.health ==
                     EnvironmentalHealth::SensorFault,
                 "out-of-range temperature is a sensor fault, not a timing fault");

        sensor.current.state = ClimateSampleState::HumidityOutOfRange;
        requireError (station.update (TimePoint (12000)),
                      StatusCode::InvalidArgument,
                      "out-of-range humidity returned");
        require (station.snapshot ().record.health ==
                     EnvironmentalHealth::SensorFault,
                 "out-of-range humidity is a sensor fault, not a timing fault");
    }

    void testTimingResetAndRollover ()
    {
        FakeClimateSensor    sensor;
        EnvironmentalStation station (sensor, testConfig ());

        requireError (station.initialize (), StatusCode::Ok, "timing initialize");
        requireError (station.update (TimePoint (0xfffffff0U)),
                      StatusCode::Ok,
                      "rollover anchor");
        require (station.snapshot ().nextSampleAt == TimePoint (1984),
                 "deadline wraps");
        requireError (station.update (TimePoint (1984)),
                      StatusCode::Ok,
                      "wrapped deadline");
        require (station.snapshot ().record.sequence == 2, "wrapped record");

        requireError (station.update (TimePoint (0x800007d0U)),
                      StatusCode::InvalidArgument,
                      "ambiguous reverse time rejected");
        require (station.snapshot ().record.health == EnvironmentalHealth::TimingFault,
                 "timing fault visible");
        require (!station.snapshot ().hasDeadline, "timing fault cancels schedule");

        requireError (station.reset (), StatusCode::Ok, "explicit reset");
        require      (sensor.initializeCount == 2, "reset restarts sensor schedule");
        require      (station.snapshot ().record.sequence == 0, "reset clears records");
        requireError (station.update (TimePoint (10)),
                      StatusCode::Ok,
                      "update after reset");
    }

    void testDeterministicReplay ()
    {
        const RecordedClimateFrame frames[] = {
            {TimePoint (0),
             {2100, 450, TimePoint (0), ClimateSampleState::Valid},
             StatusCode::Ok},
            {TimePoint (2000),
             {0, 0, TimePoint (2000), ClimateSampleState::ChecksumFailure},
             StatusCode::Ok},
            {TimePoint (4000),
             {2200, 500, TimePoint (4000), ClimateSampleState::Valid},
             StatusCode::Ok}};
        RecordedClimateSensor firstSensor  (frames, 3);
        RecordedClimateSensor secondSensor (frames, 3);
        EnvironmentalStation  first        (firstSensor, testConfig ());
        EnvironmentalStation  second       (secondSensor, testConfig ());
        const TimePoint       times[] = {
            TimePoint (0),
            TimePoint (1000),
            TimePoint (2000),
            TimePoint (4000)};

        requireError (first.initialize (), StatusCode::Ok, "first replay initialize");
        requireError (second.initialize (),
                      StatusCode::Ok,
                      "second replay initialize");

        for (size_t index = 0; index < 4; ++index)
        {
            require (first.update (times[index]).error () ==
                         second.update (times[index]).error (),
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
