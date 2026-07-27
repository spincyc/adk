#include <assert.h>
#include <stdint.h>
#include <type_traits>

#include "climate_sensor.h"

namespace {

    constexpr adk::ClimateSampleLimits dht11Limits = {0, 5000, 1000};

    struct DestructorProbe final : adk::ClimateSensor
    {
        explicit DestructorProbe (bool& destroyed) noexcept
            : destroyed_ (destroyed), initialized_ (false)
        {
        }

        ~DestructorProbe () noexcept override
        {
            shutdown ();
            destroyed_ = true;
        }

        adk::Status initialize () noexcept override
        {
            initialized_ = true;
            return adk::StatusCode::Ok;
        }

        void shutdown () noexcept override
        {
            initialized_ = false;
        }

        bool initialized () const noexcept override
        {
            return initialized_;
        }

        adk::Status update (adk::TimePoint) noexcept override
        {
            return initialized_ ? adk::StatusCode::Ok : adk::StatusCode::NotInitialized;
        }

        adk::ClimateSample sample (adk::TimePoint,
                                   adk::Duration) const noexcept override
        {
            return {0, 0, adk::TimePoint (0), adk::ClimateSampleState::Unavailable};
        }

      private:
        bool& destroyed_;
        bool  initialized_;
    };

    adk::ClimateSample validSample (int16_t temperature, uint16_t humidity,
                                    uint32_t observedAt)
    {
        return adk::validateClimateSample (temperature, humidity,
                                           adk::TimePoint (observedAt), dht11Limits);
    }

    adk::ClimateSample faultSample (int16_t temperature, uint16_t humidity,
                                    uint32_t observedAt, adk::ClimateSampleState state)
    {
        return {temperature, humidity, adk::TimePoint (observedAt), state};
    }

    void testValidationSeparatesTemperatureAndHumidity ()
    {
        assert (validSample (0, 0, 10).state == adk::ClimateSampleState::Valid);
        assert (validSample (5000, 1000, 10).state == adk::ClimateSampleState::Valid);
        assert (validSample (-1, 500, 10).state ==
                adk::ClimateSampleState::TemperatureOutOfRange);
        assert (validSample (5001, 500, 10).state ==
                adk::ClimateSampleState::TemperatureOutOfRange);
        assert (validSample (2000, 1001, 10).state ==
                adk::ClimateSampleState::HumidityOutOfRange);
    }

    void testValidationRejectsInvalidLimits ()
    {
        const adk::ClimateSampleLimits reversed          = {100, -100, 1000};
        const adk::ClimateSampleLimits excessiveHumidity = {0, 5000, 1001};

        assert (
            adk::validateClimateSample (0, 500, adk::TimePoint (0), reversed).state ==
            adk::ClimateSampleState::InvalidLimits);
        assert (adk::validateClimateSample (2000, 1001, adk::TimePoint (0),
                                            excessiveHumidity)
                    .state == adk::ClimateSampleState::InvalidLimits);
    }

    void testLifecycleAndEmptyTrace ()
    {
        adk::RecordedClimateSensor sensor (nullptr, 0);

        assert ((sensor.update (adk::TimePoint (0))).error () == adk::StatusCode::NotInitialized);
        assert ((sensor.initialize ()).ok ());
        assert ((sensor.initialize ()).ok ());
        assert (sensor.initialized ());
        assert ((sensor.update (adk::TimePoint (0))).ok ());
        assert (sensor.sample (adk::TimePoint (0), adk::Duration (1)).state ==
                adk::ClimateSampleState::Unavailable);

        sensor.shutdown ();
        sensor.shutdown ();

        assert (!sensor.initialized ());
        assert (sensor.frameIndex () == 0);
    }

    void testInvalidTraceIsRejected ()
    {
        const adk::RecordedClimateFrame backwards[] = {
            {adk::TimePoint (100), validSample (1000, 500, 100), adk::StatusCode::Ok},
            {adk::TimePoint (99), validSample (1100, 510, 99), adk::StatusCode::Ok}};
        adk::RecordedClimateSensor nullTrace (nullptr,   1);
        adk::RecordedClimateSensor badOrder  (backwards, 2);

        assert ((nullTrace.initialize ()).error () == adk::StatusCode::InvalidArgument);
        assert ((badOrder.initialize ()).error () == adk::StatusCode::InvalidArgument);
    }

    void testFramesAdvanceOnlyWhenDue ()
    {
        const adk::RecordedClimateFrame frames[] = {
            {adk::TimePoint (10), validSample (2100, 450, 10), adk::StatusCode::Ok},
            {adk::TimePoint (20),
             faultSample (2100, 450, 20, adk::ClimateSampleState::ChecksumFailure),
             adk::StatusCode::HardwareFailure},
            {adk::TimePoint (20), validSample (2200, 460, 20), adk::StatusCode::Ok}};
        adk::RecordedClimateSensor sensor (frames, 3);

        assert ((sensor.initialize ()).ok ());
        assert ((sensor.update (adk::TimePoint (9))).ok ());
        assert (sensor.frameIndex () == 0);
        assert ((sensor.update (adk::TimePoint (10))).ok ());
        assert (sensor.frameIndex () == 1);
        assert (sensor.sample (adk::TimePoint (10), adk::Duration (10))
                    .temperatureCentiCelsius == 2100);
        assert ((sensor.update (adk::TimePoint (20))).ok ());
        assert (sensor.frameIndex () == 3);
        assert (sensor.sample (adk::TimePoint (20), adk::Duration (10))
                    .temperatureCentiCelsius == 2200);
    }

    void testEqualTimeUsesTraceOrderAndFinalStatus ()
    {
        const adk::RecordedClimateFrame frames[] = {
            {adk::TimePoint (7), validSample (2000, 400, 7), adk::StatusCode::Ok},
            {adk::TimePoint (7),
             faultSample (2000, 400, 7, adk::ClimateSampleState::ChecksumFailure),
             adk::StatusCode::HardwareFailure}};
        adk::RecordedClimateSensor sensor (frames, 2);

        assert ((sensor.initialize ()).ok ());
        assert ((sensor.update (adk::TimePoint (7))).error () == adk::StatusCode::HardwareFailure);
        assert (sensor.frameIndex () == 2);
        assert (sensor.sample (adk::TimePoint (7), adk::Duration (1)).state ==
                adk::ClimateSampleState::ChecksumFailure);
        assert ((sensor.update (adk::TimePoint (7))).ok ());
    }

    void testFaultRetainsDiagnosticValuesAndCanRecover ()
    {
        const adk::RecordedClimateFrame frames[] = {
            {adk::TimePoint (0),
             faultSample (1700, 320, 0, adk::ClimateSampleState::TransportTimeout),
             adk::StatusCode::HardwareFailure},
            {adk::TimePoint (5), validSample (2300, 480, 5), adk::StatusCode::Ok}};
        adk::RecordedClimateSensor sensor (frames, 2);

        assert ((sensor.initialize ()).ok ());
        assert ((sensor.update (adk::TimePoint (0))).error () == adk::StatusCode::HardwareFailure);
        const adk::ClimateSample fault =
            sensor.sample (adk::TimePoint (100), adk::Duration (1));
        assert (fault.state == adk::ClimateSampleState::TransportTimeout);
        assert (fault.temperatureCentiCelsius == 1700);

        assert ((sensor.update (adk::TimePoint (5))).ok ());
        assert (sensor.sample (adk::TimePoint (5), adk::Duration (1)).state ==
                adk::ClimateSampleState::Valid);
    }

    void testStaleBoundaryIsInclusiveAndWrapSafe ()
    {
        const adk::RecordedClimateFrame frames[] = {
            {adk::TimePoint (UINT32_MAX - 2U), validSample (2400, 500, UINT32_MAX - 2U),
             adk::StatusCode::Ok}};
        adk::RecordedClimateSensor sensor (frames, 1);

        assert ((sensor.initialize ()).ok ());
        assert ((sensor.update (adk::TimePoint (UINT32_MAX - 2U))).ok ());
        assert (sensor.sample (adk::TimePoint (1), adk::Duration (4)).state ==
                adk::ClimateSampleState::Valid);
        assert (sensor.sample (adk::TimePoint (2), adk::Duration (4)).state ==
                adk::ClimateSampleState::Stale);
    }

    void testTimingOutsideHalfRangeIsRejected ()
    {
        const adk::RecordedClimateFrame frames[] = {
            {adk::TimePoint (10), validSample (2400, 500, 10), adk::StatusCode::Ok}};
        adk::RecordedClimateSensor sensor (frames, 1);

        assert ((sensor.initialize ()).ok ());
        assert ((sensor.update     (adk::TimePoint (10))).ok ());

        const adk::ClimateSample maximumAge = sensor.sample (
            adk::TimePoint (10), adk::Duration (static_cast<uint32_t> (INT32_MAX)));
        const adk::ClimateSample excessivePolicy =
            sensor.sample (adk::TimePoint (10),
                           adk::Duration (static_cast<uint32_t> (INT32_MAX) + 1U));
        const adk::ClimateSample backwardsAge =
            sensor.sample (adk::TimePoint (9), adk::Duration (100));

        assert (maximumAge.state == adk::ClimateSampleState::Valid);
        assert (excessivePolicy.state == adk::ClimateSampleState::InvalidTiming);
        assert (backwardsAge.state == adk::ClimateSampleState::InvalidTiming);

        assert ((sensor.update (adk::TimePoint (9))).error () == adk::StatusCode::InvalidArgument);
        assert (sensor.frameIndex () == 1);
    }

    void testWrapBoundaryAndLargeValidAdvance ()
    {
        const adk::RecordedClimateFrame frames[] = {
            {adk::TimePoint (UINT32_MAX), validSample (2400, 500, UINT32_MAX),
             adk::StatusCode::Ok},
            {adk::TimePoint (0), validSample (2500, 510, 0), adk::StatusCode::Ok}};
        adk::RecordedClimateSensor sensor (frames, 2);

        assert ((sensor.initialize ()).ok ());
        assert ((sensor.update (adk::TimePoint (UINT32_MAX - 1U))).ok ());
        assert (sensor.frameIndex () == 0);
        assert ((sensor.update (adk::TimePoint (UINT32_MAX))).ok ());
        assert (sensor.frameIndex () == 1);
        assert ((sensor.update (adk::TimePoint (0))).ok ());
        assert (sensor.frameIndex () == 2);
        assert (
            (sensor.update (
                 adk::TimePoint (static_cast<uint32_t> (INT32_MAX)))).ok ());
    }

    void testShutdownResetsReplay ()
    {
        const adk::RecordedClimateFrame frames[] = {
            {adk::TimePoint (1), validSample (2000, 400, 1), adk::StatusCode::Ok},
            {adk::TimePoint (2), validSample (2100, 410, 2), adk::StatusCode::Ok}};
        adk::RecordedClimateSensor sensor (frames, 2);

        assert ((sensor.initialize ()).ok ());
        assert ((sensor.update (adk::TimePoint (2))).ok ());
        const adk::ClimateSample first =
            sensor.sample (adk::TimePoint (2), adk::Duration (10));

        sensor.shutdown ();

        assert ((sensor.initialize ()).ok ());
        assert ((sensor.update (adk::TimePoint (2))).ok ());
        const adk::ClimateSample second =
            sensor.sample (adk::TimePoint (2), adk::Duration (10));

        assert (first.temperatureCentiCelsius == second.temperatureCentiCelsius);
        assert (first.humidityPermille == second.humidityPermille);
        assert (first.observedAt == second.observedAt);
        assert (first.state == second.state);
    }

    void testReplayProducesIdenticalStatusAndSamples ()
    {
        const adk::RecordedClimateFrame frames[] = {
            {adk::TimePoint (1), validSample (2000, 400, 1), adk::StatusCode::Ok},
            {adk::TimePoint (3),
             faultSample (2000, 400, 3, adk::ClimateSampleState::TransportTimeout),
             adk::StatusCode::HardwareFailure},
            {adk::TimePoint (5), validSample (2100, 410, 5), adk::StatusCode::Ok}};
        const adk::TimePoint       times[] = {adk::TimePoint (0), adk::TimePoint (1),
                                              adk::TimePoint (3), adk::TimePoint (5)};
        adk::Status statuses[4] = {};
        adk::ClimateSample samples[4] = {
            {0, 0, adk::TimePoint (0), adk::ClimateSampleState::Unavailable},
            {0, 0, adk::TimePoint (0), adk::ClimateSampleState::Unavailable},
            {0, 0, adk::TimePoint (0), adk::ClimateSampleState::Unavailable},
            {0, 0, adk::TimePoint (0), adk::ClimateSampleState::Unavailable}
        };
        adk::RecordedClimateSensor sensor (frames, 3);

        assert ((sensor.initialize ()).ok ());

        for (size_t index = 0; index < 4; ++index)
        {
            statuses[index] = sensor.update (times[index]);
            samples[index]  = sensor.sample (times[index], adk::Duration (10));
        }

        sensor.shutdown ();

        assert ((sensor.initialize ()).ok ());

        for (size_t index = 0; index < 4; ++index)
        {
            const adk::Status        status = sensor.update (times[index]);
            const adk::ClimateSample sample =
                sensor.sample (times[index], adk::Duration (10));

            assert (status == statuses[index]);
            assert (sample.temperatureCentiCelsius ==
                    samples[index].temperatureCentiCelsius);
            assert (sample.humidityPermille == samples[index].humidityPermille);
            assert (sample.observedAt == samples[index].observedAt);
            assert (sample.state == samples[index].state);
        }
    }

    void testVirtualDestruction ()
    {
        bool                destroyed = false;
        adk::ClimateSensor* sensor    = new DestructorProbe (destroyed);

        assert ((sensor->initialize ()).ok ());
        assert (sensor->initialized ());

        delete sensor;

        assert (destroyed);
    }
} // namespace

int main ()
{
    static_assert (std::has_virtual_destructor<adk::ClimateSensor>::value,
                   "ClimateSensor needs polymorphic destruction");
    static_assert (!std::is_copy_constructible<adk::RecordedClimateSensor>::value,
                   "RecordedClimateSensor owns replay state");
    static_assert (!std::is_move_constructible<adk::RecordedClimateSensor>::value,
                   "RecordedClimateSensor has a stable address");

    testValidationSeparatesTemperatureAndHumidity  ();
    testValidationRejectsInvalidLimits             ();
    testLifecycleAndEmptyTrace                     ();
    testInvalidTraceIsRejected                     ();
    testFramesAdvanceOnlyWhenDue                   ();
    testEqualTimeUsesTraceOrderAndFinalStatus      ();
    testFaultRetainsDiagnosticValuesAndCanRecover  ();
    testStaleBoundaryIsInclusiveAndWrapSafe        ();
    testTimingOutsideHalfRangeIsRejected           ();
    testWrapBoundaryAndLargeValidAdvance           ();
    testShutdownResetsReplay                       ();
    testReplayProducesIdenticalStatusAndSamples    ();
    testVirtualDestruction                         ();

    return 0;
}
