#include <assert.h>
#include <stdint.h>
#include <type_traits>

#include "pulse_input.h"
#include "ultrasonic_ranger.h"

namespace {

    adk::PulseInputConfig pulseConfig ()
    {
        const adk::PulseInputConfig config = {adk::MicrosecondDuration (100),
                                              adk::MicrosecondDuration (200)};
        return config;
    }

    adk::UltrasonicRangerConfig rangerConfig ()
    {
        const adk::UltrasonicRangerConfig config = {adk::MicrosecondDuration (30000),
                                                    adk::MicrosecondDuration (30000),
                                                    20, 4000, 343};
        return config;
    }

    void assertPulse (const adk::PulseInput& input, adk::PulseInputState state,
                      uint32_t duration, bool complete, bool timedOut)
    {
        const adk::PulseInputSnapshot snapshot = input.snapshot ();

        assert (snapshot.state == state);
        assert (snapshot.highDuration.microseconds () == duration);
        assert (snapshot.complete == complete);
        assert (snapshot.timedOut == timedOut);
    }

    void assertRange (const adk::UltrasonicRanger& ranger, adk::RangeState state,
                      uint16_t distance, uint32_t duration, bool valid)
    {
        const adk::RangeReading reading = ranger.reading ();

        assert (reading.state == state);
        assert (reading.distanceMm == distance);
        assert (reading.echoDuration.microseconds () == duration);
        assert (reading.valid == valid);
    }

    void testMicrosecondTimeWrapsDeterministically ()
    {
        const adk::MicrosecondTimePoint before (0xfffffff0u);
        const adk::MicrosecondTimePoint after  (0x00000020u);

        assert (after.elapsedSince (before).microseconds () == 48);
    }

    void testPulseRequiresInitialization ()
    {
        adk::PulseInput input (pulseConfig ());

        assert (input.arm (adk::MicrosecondTimePoint (0), false).error () ==
                adk::StatusCode::NotInitialized);
        assert (input.update (adk::MicrosecondTimePoint (0), false).error () ==
                adk::StatusCode::NotInitialized);
    }

    void testPulseRejectsInvalidConfiguration ()
    {
        const adk::PulseInputConfig configurations[] = {
            {adk::MicrosecondDuration (0), adk::MicrosecondDuration (10)},
            {adk::MicrosecondDuration (10), adk::MicrosecondDuration (0)},
            {adk::MicrosecondDuration (0x80000000u), adk::MicrosecondDuration (10)},
            {adk::MicrosecondDuration (10), adk::MicrosecondDuration (0x80000000u)}};

        for (uint8_t index = 0;
             index < sizeof (configurations) / sizeof (configurations[0]); ++index)
        {
            adk::PulseInput input (configurations[index]);

            assert (input.initialize ().error () == adk::StatusCode::InvalidArgument);
            assert (!input.initialized ());
        }
    }

    void testPulseMeasuresEdgesAndExactBoundary ()
    {
        adk::PulseInput input (pulseConfig ());

        assert      (input.initialize ().ok ());
        assert      (input.arm (adk::MicrosecondTimePoint (1000), false).ok ());
        assertPulse (input, adk::PulseInputState::AwaitingRise, 0, false, false);

        input.update (adk::MicrosecondTimePoint (1100), true);
        assertPulse  (input, adk::PulseInputState::MeasuringHigh, 0, false, false);

        input.update (adk::MicrosecondTimePoint (1300), false);
        assertPulse  (input, adk::PulseInputState::Complete, 200, true, false);

        input.update (adk::MicrosecondTimePoint (1400), true);
        assertPulse  (input, adk::PulseInputState::Complete, 200, true, false);
    }

    void testRepeatedInitializationPreservesMeasurement ()
    {
        adk::PulseInput input (pulseConfig ());

        assert       (input.initialize ().ok ());
        input.arm    (adk::MicrosecondTimePoint (10), false);
        input.update (adk::MicrosecondTimePoint (20), true);

        assert      (input.initialize ().ok ());
        assertPulse (input, adk::PulseInputState::MeasuringHigh, 0, false, false);
    }

    void testPulseTimeoutBoundaries ()
    {
        adk::PulseInput input (pulseConfig ());

        assert       (input.initialize ().ok ());
        input.arm    (adk::MicrosecondTimePoint (0), false);
        input.update (adk::MicrosecondTimePoint (100), false);
        assertPulse  (input, adk::PulseInputState::AwaitingRise, 0, false, false);

        input.update (adk::MicrosecondTimePoint (101), false);
        assertPulse  (input, adk::PulseInputState::Timeout, 0, false, true);

        input.arm    (adk::MicrosecondTimePoint (1000), false);
        input.update (adk::MicrosecondTimePoint (1001), true);
        input.update (adk::MicrosecondTimePoint (1202), true);
        assertPulse  (input, adk::PulseInputState::Timeout, 0, false, true);
    }

    void testPulseRejectsStuckHighBeforeRise ()
    {
        adk::PulseInput input (pulseConfig ());

        assert       (input.initialize ().ok ());
        input.arm    (adk::MicrosecondTimePoint (50), true);
        input.update (adk::MicrosecondTimePoint (150), true);
        assertPulse  (input, adk::PulseInputState::AwaitingLow, 0, false, false);

        input.update (adk::MicrosecondTimePoint (151), true);
        assertPulse  (input, adk::PulseInputState::Timeout, 0, false, true);

        input.arm    (adk::MicrosecondTimePoint (200), true);
        input.update (adk::MicrosecondTimePoint (210), false);
        input.update (adk::MicrosecondTimePoint (220), true);
        input.update (adk::MicrosecondTimePoint (240), false);
        assertPulse  (input, adk::PulseInputState::Complete, 20, true, false);
    }

    void testPulseWraparound ()
    {
        adk::PulseInput input (pulseConfig ());

        assert       (input.initialize ().ok ());
        input.arm    (adk::MicrosecondTimePoint (0xfffffff0u), false);
        input.update (adk::MicrosecondTimePoint (0xfffffff8u), true);
        input.update (adk::MicrosecondTimePoint (0x00000018u), false);
        assertPulse  (input, adk::PulseInputState::Complete, 32, true, false);
    }

    void testRangerConvertsRoundTripDuration ()
    {
        adk::UltrasonicRanger ranger (rangerConfig ());

        assert (ranger.initialize ().ok ());
        assert (ranger.startMeasurement (adk::MicrosecondTimePoint (0), false).ok ());

        ranger.update (adk::MicrosecondTimePoint (100), true);
        assertRange   (ranger, adk::RangeState::Measuring, 0, 0, false);

        ranger.update (adk::MicrosecondTimePoint (5931), false);
        assertRange   (ranger, adk::RangeState::Valid, 1000, 5831, true);
    }

    void testRangerDistinguishesTimeoutAndRange ()
    {
        adk::UltrasonicRanger ranger (rangerConfig ());

        assert                  (ranger.initialize ().ok ());
        ranger.startMeasurement (adk::MicrosecondTimePoint (0), false);
        ranger.update           (adk::MicrosecondTimePoint (30001), false);
        assertRange             (ranger, adk::RangeState::Timeout, 0, 0, false);

        ranger.startMeasurement (adk::MicrosecondTimePoint (40000), false);
        ranger.update           (adk::MicrosecondTimePoint (40010), true);
        ranger.update           (adk::MicrosecondTimePoint (40020), false);
        assertRange             (ranger, adk::RangeState::OutOfRange, 2, 10, false);

        ranger.startMeasurement (adk::MicrosecondTimePoint (50000), false);
        ranger.update           (adk::MicrosecondTimePoint (50010), true);
        ranger.update           (adk::MicrosecondTimePoint (80011), true);
        assertRange             (ranger, adk::RangeState::Timeout, 0, 0, false);
    }

    void testRangerRejectsInvalidConfiguration ()
    {
        adk::UltrasonicRangerConfig config = rangerConfig ();
        config.minimumDistanceMm           = config.maximumDistanceMm;

        adk::UltrasonicRanger ranger (config);

        assert (ranger.initialize ().error () == adk::StatusCode::InvalidArgument);
        assert (!ranger.initialized ());
        assert (
            ranger.startMeasurement (adk::MicrosecondTimePoint (0), false).error () ==
            adk::StatusCode::NotInitialized);
    }

    void testRepeatedRangeTraceIsDeterministic ()
    {
        adk::RangeReading passes[2] =
        {
            {adk::RangeState::Idle, 0, adk::MicrosecondDuration (), false},
            {adk::RangeState::Idle, 0, adk::MicrosecondDuration (), false}
        };

        for (uint8_t pass = 0; pass < 2; ++pass)
        {
            adk::UltrasonicRanger ranger (rangerConfig ());

            assert                        (ranger.initialize ().ok ());
            ranger.startMeasurement       (adk::MicrosecondTimePoint (1000), false);
            ranger.update                 (adk::MicrosecondTimePoint (1100), true);
            ranger.update                 (adk::MicrosecondTimePoint (6931), false);
            passes[pass] = ranger.reading ();
        }

        assert (passes[0].state == passes[1].state);
        assert (passes[0].distanceMm == passes[1].distanceMm);
        assert (passes[0].echoDuration.microseconds () ==
                passes[1].echoDuration.microseconds ());
        assert (passes[0].valid == passes[1].valid);
    }

    static_assert (std::is_trivially_copyable<adk::MicrosecondTimePoint>::value,
                   "microsecond time must remain a small value");
    static_assert (std::is_trivially_copyable<adk::RangeReading>::value,
                   "range readings must remain allocation-free values");
} // namespace

int main ()
{
    testMicrosecondTimeWrapsDeterministically      ();
    testPulseRequiresInitialization                ();
    testPulseRejectsInvalidConfiguration           ();
    testPulseMeasuresEdgesAndExactBoundary         ();
    testRepeatedInitializationPreservesMeasurement ();
    testPulseTimeoutBoundaries                     ();
    testPulseRejectsStuckHighBeforeRise            ();
    testPulseWraparound                            ();
    testRangerConvertsRoundTripDuration            ();
    testRangerDistinguishesTimeoutAndRange         ();
    testRangerRejectsInvalidConfiguration          ();
    testRepeatedRangeTraceIsDeterministic          ();
}
