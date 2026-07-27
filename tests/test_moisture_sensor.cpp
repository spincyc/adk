#include <moisture_sensor.h>

#include <Arduino.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {
    namespace fake = adk::test::arduino;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    void setReading (uint8_t pin, uint16_t reading)
    {
        fake::setAnalogInput (pin, reading);
    }

    void provesAscendingAndDescendingCalibration ()
    {
        fake::reset ();
        adk::ResourceRegistry resources;
        adk::AnalogInput      ascendingInput (resources, 54);
        adk::MoistureSensor   ascending      (
            ascendingInput, {200, 1000, 16});

        setReading (54, 200);
        require    (ascending.initialize ().ok (), "ascending sensor initializes");

        const uint16_t readings[] = {200, 600, 1000};
        const uint16_t expected[] = {0, 500, 1000};

        for (uint8_t index = 0; index < 3; ++index)
        {
            setReading (54, readings[index]);
            require    (ascending.update (adk::TimePoint (index)).ok (),
                        "ascending sample updates");
            require    (ascending.sample (adk::TimePoint (index),
                                          adk::Duration (10)).moisturePermille
                            == expected[index],
                        "ascending sample maps");
        }

        ascending.shutdown                  ();
        adk::AnalogInput    descendingInput (resources, 54);
        adk::MoistureSensor descending      (
            descendingInput, {900, 100, 16});
        require (descending.initialize ().ok (), "descending sensor initializes");

        const uint16_t descendingReadings[] = {900, 500, 100};

        for (uint8_t index = 0; index < 3; ++index)
        {
            setReading (54, descendingReadings[index]);
            require    (descending.update (adk::TimePoint (index)).ok (),
                        "descending sample updates");
            require    (descending.sample (adk::TimePoint (index),
                                           adk::Duration (10)).moisturePermille
                            == expected[index],
                        "descending sample maps");
        }
    }

    void provesFaultMarginsAndRounding ()
    {
        fake::reset ();
        adk::ResourceRegistry resources;
        adk::AnalogInput      input  (resources, 55);
        adk::MoistureSensor   sensor (input, {200, 1000, 16});

        setReading (55, 200);
        require    (sensor.initialize ().ok (), "fault sensor initializes");

        setReading    (55, 183);
        sensor.update (adk::TimePoint (1));
        require       (sensor.sample (adk::TimePoint (1), adk::Duration (10)).state
                     == adk::MoistureSampleState::InputBelowRange,
                 "below range is explicit");

        setReading    (55, 1017);
        sensor.update (adk::TimePoint (2));
        require       (sensor.sample (adk::TimePoint (2), adk::Duration (10)).state
                     == adk::MoistureSampleState::InputAboveRange,
                 "above range is explicit");

        setReading    (55, 184);
        sensor.update (adk::TimePoint (3));
        require       (sensor.sample (adk::TimePoint (3), adk::Duration (10)).state
                     == adk::MoistureSampleState::Valid,
                 "margin boundary remains valid");

        adk::MoistureSensor rounded (input, {0, 1023, 0});
        sensor.shutdown             ();
        require                     (rounded.initialize ().ok (), "extreme calibration initializes");
        setReading                  (55, 512);
        rounded.update              (adk::TimePoint (4));
        require                     (rounded.sample (adk::TimePoint (4), adk::Duration (10))
                         .moisturePermille == 500,
                 "mapping rounds to nearest permille");

        rounded.shutdown ();

        adk::MoistureSensor descending (input, {900, 100, 16});
        require                        (descending.initialize ().ok (),
                                        "descending margin sensor initializes");

        const uint16_t readings[] = {83, 84, 100, 900, 916, 917};
        const adk::MoistureSampleState states[] = {
            adk::MoistureSampleState::InputBelowRange,
            adk::MoistureSampleState::Valid,
            adk::MoistureSampleState::Valid,
            adk::MoistureSampleState::Valid,
            adk::MoistureSampleState::Valid,
            adk::MoistureSampleState::InputAboveRange
        };
        const uint16_t moisture[] = {0, 1000, 1000, 0, 0, 0};

        for (uint8_t index = 0; index < 6; ++index)
        {
            setReading (55, readings[index]);
            require    (descending.update (adk::TimePoint (10 + index)).ok (),
                        "descending margin sample updates");
            const adk::MoistureSample sample =
                descending.sample (adk::TimePoint (10 + index), adk::Duration (10));
            require (sample.state == states[index], "descending margin state is exact");
            require (sample.moisturePermille == moisture[index],
                     "descending margin clamps before mapping");
        }
    }

    void provesStaleRolloverAndFaultPrecedence ()
    {
        fake::reset ();
        adk::ResourceRegistry resources;
        adk::AnalogInput      input  (resources, 56);
        adk::MoistureSensor   sensor (input, {100, 900, 8});

        setReading (56, 500);
        require    (sensor.initialize ().ok (), "stale sensor initializes");
        require    (sensor.update (adk::TimePoint (0xfffffff0UL)).ok (),
                    "rollover sample updates");

        require (sensor.sample (adk::TimePoint (4), adk::Duration (20)).state
                     == adk::MoistureSampleState::Valid,
                 "equal stale boundary remains valid");
        require (sensor.sample (adk::TimePoint (5), adk::Duration (20)).state
                     == adk::MoistureSampleState::Stale,
                 "one tick beyond boundary is stale");

        setReading    (56, 0);
        sensor.update (adk::TimePoint (6));
        require       (sensor.sample (adk::TimePoint (100), adk::Duration (1)).state
                     == adk::MoistureSampleState::InputBelowRange,
                 "new fault takes precedence over stale history");
    }

    void provesValidationLifecycleAndReplay ()
    {
        const adk::MoistureCalibration invalid[] = {
            {100, 100, 0}, {100, 200, 64}, {0, 1024, 0}, {1024, 0, 0}
        };

        for (const adk::MoistureCalibration& calibration : invalid)
        {
            fake::reset ();
            adk::ResourceRegistry resources;
            adk::AnalogInput      input  (resources, 57);
            adk::MoistureSensor   sensor (input, calibration);

            require (sensor.initialize ().error () ==
                         adk::StatusCode::InvalidArgument,
                     "invalid calibration is rejected");
            require (!input.initialized (), "invalid calibration stays inert");
        }

        fake::reset ();
        adk::ResourceRegistry resources;
        adk::AnalogInput      input  (resources, 57);
        adk::MoistureSensor   sensor (input, {200, 1000, 16});

        require (sensor.update (adk::TimePoint ()).error () ==
                     adk::StatusCode::NotInitialized,
                 "inactive update is rejected");
        setReading    (57, 680);
        require       (sensor.initialize ().ok (), "replay sensor initializes");
        require       (sensor.initialize ().ok (), "initialization repeats");
        sensor.update (adk::TimePoint (600));
        const adk::MoistureSample first =
            sensor.sample (adk::TimePoint (600), adk::Duration (100));
        const adk::MoistureSample second =
            sensor.sample (adk::TimePoint (600), adk::Duration (100));

        require (first.moisturePermille == 600, "golden trace maps");
        require (first.moisturePermille == second.moisturePermille
                     && first.rawReading == second.rawReading
                     && first.observedAt == second.observedAt
                     && first.state == second.state,
                 "repeated observation is deterministic");

        sensor.shutdown ();
        sensor.shutdown ();
        require         (!input.initialized (), "shutdown releases input");
        require         (sensor.sample (adk::TimePoint (601), adk::Duration (100))
                                 .state == adk::MoistureSampleState::Unavailable,
                         "shutdown makes observation unavailable");

        require         (sensor.initialize ().ok (),
                         "sensor reinitializes after shutdown");
        sensor.shutdown ();
    }

    void provesBusyRetryAndDestructorReuse ()
    {
        fake::reset ();
        adk::ResourceRegistry resources;
        adk::AnalogInput      blocker (resources, 58);
        adk::AnalogInput      input   (resources, 58);
        adk::MoistureSensor   sensor  (input, {200, 1000, 16});

        require (blocker.initialize ().ok (), "competing input acquires pin");
        require (sensor.initialize ().error () == adk::StatusCode::ResourceBusy,
                 "busy initialization reports resource ownership");
        require (!sensor.initialized (), "busy initialization remains inert");

        blocker.shutdown ();
        require          (sensor.initialize ().ok (),
                          "busy initialization can be retried");
        sensor.shutdown  ();

        {
            adk::AnalogInput    scopedInput  (resources, 58);
            adk::MoistureSensor scopedSensor (scopedInput, {200, 1000, 16});
            require                          (scopedSensor.initialize ().ok (),
                                              "scoped sensor initializes");
        }

        adk::AnalogInput reused (resources, 58);
        require                 (reused.initialize ().ok (),
                                 "destruction releases the analog resource");
    }
}

int main ()
{
    static_assert (!std::is_copy_constructible<adk::MoistureSensor>::value,
                   "moisture sensor is not copyable");
    static_assert (!std::is_move_constructible<adk::MoistureSensor>::value,
                   "moisture sensor is not movable");

    provesAscendingAndDescendingCalibration ();
    provesFaultMarginsAndRounding           ();
    provesStaleRolloverAndFaultPrecedence   ();
    provesValidationLifecycleAndReplay      ();
    provesBusyRetryAndDestructorReuse       ();
    return EXIT_SUCCESS;
}
