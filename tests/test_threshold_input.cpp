#include <assert.h>
#include <stdint.h>
#include <type_traits>

#include "threshold_input.h"

namespace {

    adk::ThresholdInputConfig risingConfig ()
    {
        const adk::ThresholdInputConfig config = {
            100, 900, 700, 600, adk::ThresholdDirection::Rising
        };
        return config;
    }

    adk::ThresholdInputConfig fallingConfig ()
    {
        const adk::ThresholdInputConfig config = {
            100, 900, 300, 400, adk::ThresholdDirection::Falling
        };
        return config;
    }

    void assertObservation (const adk::ThresholdObservation& observation,
                            uint16_t                         raw,
                            adk::ThresholdFault              fault,
                            bool                             active,
                            bool                             changed,
                            bool                             valid)
    {
        assert (observation.raw == raw);
        assert (observation.fault == fault);
        assert (observation.active == active);
        assert (observation.changed == changed);
        assert (observation.valid == valid);
    }

    void testInvalidConfigurations ()
    {
        const adk::ThresholdInputConfig configs[] = {
            { 900, 100, 700, 600, adk::ThresholdDirection::Rising },
            { 100, 900, 901, 600, adk::ThresholdDirection::Rising },
            { 100, 900, 700, 99, adk::ThresholdDirection::Rising },
            { 100, 900, 600, 600, adk::ThresholdDirection::Rising },
            { 100, 900, 400, 400, adk::ThresholdDirection::Falling },
            { 100, 900, 500, 400, adk::ThresholdDirection::Falling }
        };

        for (uint8_t index = 0; index < sizeof (configs) / sizeof (configs[0]); ++index)
        {
            adk::ThresholdInput input (configs[index]);

            assert (!input.validConfig ());

            assertObservation (input.update (500),
                               500,
                               adk::ThresholdFault::InvalidConfiguration,
                               false,
                               false,
                               false);
        }
    }

    void testRisingHysteresisBoundaries ()
    {
        adk::ThresholdInput input (risingConfig ());

        assert (input.validConfig ());

        assertObservation (input.update (699), 699, adk::ThresholdFault::None,
                           false, false, true);
        assertObservation (input.update (700), 700, adk::ThresholdFault::None,
                           true, true, true);
        assertObservation (input.update (600), 600, adk::ThresholdFault::None,
                           true, false, true);
        assertObservation (input.update (599), 599, adk::ThresholdFault::None,
                           false, true, true);
    }

    void testFallingHysteresisBoundaries ()
    {
        adk::ThresholdInput input (fallingConfig ());

        assertObservation (input.update (301), 301, adk::ThresholdFault::None,
                           false, false, true);
        assertObservation (input.update (300), 300, adk::ThresholdFault::None,
                           true, true, true);
        assertObservation (input.update (400), 400, adk::ThresholdFault::None,
                           true, false, true);
        assertObservation (input.update (401), 401, adk::ThresholdFault::None,
                           false, true, true);
    }

    void testRangeFaultsPreserveHysteresisState ()
    {
        adk::ThresholdInput input (risingConfig ());

        input.update (700);

        assertObservation (input.update (99), 99, adk::ThresholdFault::BelowRange,
                           true, false, false);
        assertObservation (input.update (901), 901, adk::ThresholdFault::AboveRange,
                           true, false, false);
        assertObservation (input.update (600), 600, adk::ThresholdFault::None,
                           true, false, true);
        assertObservation (input.update (599), 599, adk::ThresholdFault::None,
                           false, true, true);
    }

    void testExplicitFaultEvidenceHasPrecedence ()
    {
        const adk::ThresholdFault faults[] = {
            adk::ThresholdFault::Disconnected,
            adk::ThresholdFault::Saturated,
            adk::ThresholdFault::SourceFailure,
            adk::ThresholdFault::BelowRange,
            adk::ThresholdFault::AboveRange
        };
        adk::ThresholdInput input (risingConfig ());

        for (uint8_t index = 0; index < sizeof (faults) / sizeof (faults[0]); ++index)
        {
            assertObservation (input.update (500, faults[index]),
                               500,
                               faults[index],
                               false,
                               false,
                               false);
        }
    }

    void testResetClearsDecisionButRetainsConfiguration ()
    {
        adk::ThresholdInput input (risingConfig ());

        input.update (700);
        input.reset  ();

        assert (input.validConfig ());

        assertObservation (input.observation (),
                           0,
                           adk::ThresholdFault::SourceFailure,
                           false,
                           false,
                           false);

        assertObservation (input.update (650), 650, adk::ThresholdFault::None,
                           false, false, true);
    }

    void testRepeatedTraceIsDeterministic ()
    {
        const uint16_t samples[] = { 500, 700, 650, 99, 600, 599, 800 };
        adk::ThresholdObservation firstPass[7] = {};
        adk::ThresholdInput       input (risingConfig ());

        for (uint8_t index = 0; index < 7; ++index)
        {
            firstPass[index] = input.update (samples[index]);
        }

        input.reset ();

        for (uint8_t index = 0; index < 7; ++index)
        {
            const adk::ThresholdObservation replay = input.update (samples[index]);

            assert (replay.raw == firstPass[index].raw);
            assert (replay.fault == firstPass[index].fault);
            assert (replay.active == firstPass[index].active);
            assert (replay.changed == firstPass[index].changed);
            assert (replay.valid == firstPass[index].valid);
        }
    }

    void testExhaustiveRisingSweep ()
    {
        adk::ThresholdInput input (risingConfig ());

        for (uint16_t raw = 100; raw <= 900; ++raw)
        {
            const adk::ThresholdObservation observation = input.update (raw);

            assert (observation.valid);
            assert (observation.active == (raw >= 700));
        }

        for (uint16_t raw = 900; raw >= 100; --raw)
        {
            const adk::ThresholdObservation observation = input.update (raw);

            assert (observation.valid);
            assert (observation.active == (raw >= 600));
        }
    }

    void testExhaustiveFallingSweep ()
    {
        adk::ThresholdInput input (fallingConfig ());

        for (uint16_t raw = 900; raw >= 100; --raw)
        {
            const adk::ThresholdObservation observation = input.update (raw);

            assert (observation.valid);
            assert (observation.active == (raw <= 300));
        }

        for (uint16_t raw = 100; raw <= 900; ++raw)
        {
            const adk::ThresholdObservation observation = input.update (raw);

            assert (observation.valid);
            assert (observation.active == (raw <= 400));
        }
    }
}

int main ()
{
    static_assert (!std::is_polymorphic<adk::ThresholdInput>::value, "");

    testInvalidConfigurations                      ();
    testRisingHysteresisBoundaries                 ();
    testFallingHysteresisBoundaries                ();
    testRangeFaultsPreserveHysteresisState         ();
    testExplicitFaultEvidenceHasPrecedence         ();
    testResetClearsDecisionButRetainsConfiguration ();
    testRepeatedTraceIsDeterministic               ();
    testExhaustiveRisingSweep                      ();
    testExhaustiveFallingSweep                     ();
}
