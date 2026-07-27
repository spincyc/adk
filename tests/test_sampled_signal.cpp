#include <assert.h>
#include <stdint.h>
#include <type_traits>

#include "sampled_signal.h"

namespace {

    void testCalibrationRejectsInvalidRange ()
    {
        const adk::LinearCalibrationConfig config = {400, 400, 0, 1000, true};
        const adk::LinearCalibration       calibration (config);

        assert (!calibration.valid ());
        assert (!calibration.map (400).ok ());
        assert (calibration.map (400).error () == adk::StatusCode::InvalidArgument);
    }

    void testCalibrationMapsAndRounds ()
    {
        const adk::LinearCalibrationConfig config = {100, 900, 0, 1000, true};
        const adk::LinearCalibration       calibration (config);

        assert (calibration.valid ());
        assert (calibration.map (100).value () == 0);
        assert (calibration.map (500).value () == 500);
        assert (calibration.map (900).value () == 1000);
        assert (calibration.map (101).value () == 1);
    }

    void testCalibrationClampsOrRejectsOutsideObservations ()
    {
        const adk::LinearCalibrationConfig clampedConfig = {100, 900, 0, 1000, true};
        const adk::LinearCalibration       clamped (clampedConfig);
        const adk::LinearCalibrationConfig strictConfig = {100, 900, 0, 1000, false};
        const adk::LinearCalibration       strict (strictConfig);

        assert (clamped.map (0).value () == 0);
        assert (clamped.map (1023).value () == 1000);
        assert (!strict.map (99).ok ());
        assert (strict.map (99).error () == adk::StatusCode::InvalidArgument);
        assert (!strict.map (901).ok ());
        assert (strict.map (901).error () == adk::StatusCode::InvalidArgument);
    }

    void testCalibrationSupportsDescendingOutput ()
    {
        const adk::LinearCalibrationConfig config = {100, 900, 1000, 0, true};
        const adk::LinearCalibration       calibration (config);

        assert (calibration.map (100).value () == 1000);
        assert (calibration.map (500).value () == 500);
        assert (calibration.map (900).value () == 0);
    }

    void testCalibrationPreservesExtremeEndpointsAndMonotonicity ()
    {
        const adk::LinearCalibrationConfig ascendingConfig = {
            0, UINT16_MAX, 0, UINT16_MAX, true
        };
        const adk::LinearCalibrationConfig descendingConfig = {
            0, UINT16_MAX, UINT16_MAX, 0, true
        };
        const adk::LinearCalibration ascending  (ascendingConfig);
        const adk::LinearCalibration descending (descendingConfig);
        uint16_t                     previousAscending  = 0;
        uint16_t                     previousDescending = UINT16_MAX;

        for (uint32_t sample = 0; sample <= UINT16_MAX; sample += 257U)
        {
            const uint16_t ascendingValue =
                ascending.map (static_cast<uint16_t> (sample)).value ();
            const uint16_t descendingValue =
                descending.map (static_cast<uint16_t> (sample)).value ();

            assert (ascendingValue >= previousAscending);
            assert (descendingValue <= previousDescending);

            previousAscending  = ascendingValue;
            previousDescending = descendingValue;
        }

        assert (ascending.map (UINT16_MAX).value () == UINT16_MAX);
        assert (descending.map (UINT16_MAX).value () == 0);
    }

    void testMovingAverageRejectsInvalidWindows ()
    {
        adk::MovingAverage empty     (0);
        adk::MovingAverage excessive (
            static_cast<uint8_t> (adk::MovingAverage::maximumWindowSize + 1U));

        assert (!empty.valid ());
        assert (!excessive.valid ());
        assert (!empty.addSample (10).ok ());
        assert (empty.addSample (10).error () == adk::StatusCode::InvalidArgument);
        assert (!excessive.value ().ok ());
        assert (excessive.value ().error () == adk::StatusCode::InvalidArgument);
    }

    void testMovingAverageWarmsAndRolls ()
    {
        adk::MovingAverage average (3);

        assert (average.valid ());
        assert (!average.hasValue ());
        assert (!average.value ().ok ());
        assert (average.value ().error () == adk::StatusCode::NotInitialized);
        assert (average.addSample (10).value () == 10);
        assert (average.addSample (20).value () == 15);
        assert (average.addSample (30).value () == 20);
        assert (average.addSample (50).value () == 33);
        assert (average.sampleCount () == 3);
    }

    void testMovingAverageUsesFullWidthSum ()
    {
        adk::MovingAverage average (adk::MovingAverage::maximumWindowSize);

        for (uint8_t index = 0; index < adk::MovingAverage::maximumWindowSize; ++index)
        {
            assert (average.addSample (UINT16_MAX).value () == UINT16_MAX);
        }
    }

    void testMovingAverageResetReplaysDeterministically ()
    {
        const uint16_t     samples[]    = {100, 500, 900, 300};
        uint16_t           firstPass[4] = {};
        adk::MovingAverage average (3);

        for (uint8_t index = 0; index < 4; ++index)
        {
            firstPass[index] = average.addSample (samples[index]).value ();
        }

        average.reset ();

        for (uint8_t index = 0; index < 4; ++index)
        {
            assert (average.addSample (samples[index]).value () == firstPass[index]);
        }
    }

    void testMovingAverageMatchesReferenceAcrossRingWraps ()
    {
        const uint8_t windows[] = {
            1, adk::MovingAverage::maximumWindowSize
        };

        for (uint8_t windowIndex = 0; windowIndex < 2; ++windowIndex)
        {
            const uint8_t window = windows[windowIndex];
            adk::MovingAverage average (window);
            uint16_t           history[adk::MovingAverage::maximumWindowSize] = {};
            uint8_t            count = 0;
            uint8_t            next  = 0;

            for (uint16_t index = 0; index < window * 3U + 1U; ++index)
            {
                const uint16_t sample = index % 2U == 0 ? 0 : UINT16_MAX;
                history[next] = sample;
                next = static_cast<uint8_t> ((next + 1U) % window);
                if (count < window)
                {
                    ++count;
                }

                uint32_t referenceSum = 0;
                for (uint8_t historyIndex = 0; historyIndex < count; ++historyIndex)
                {
                    referenceSum += history[historyIndex];
                }
                const uint16_t reference =
                    static_cast<uint16_t> ((referenceSum + count / 2U) / count);

                assert (average.addSample (sample).value () == reference);
            }
        }
    }

    void testDeadbandHoldsSmallChanges ()
    {
        adk::Deadband deadband (10);

        assert (!deadband.hasValue ());
        assert (!deadband.value ().ok ());
        assert (deadband.value ().error () == adk::StatusCode::NotInitialized);
        assert (deadband.addSample (500) == 500);
        assert (deadband.addSample (509) == 500);
        assert (deadband.addSample (510) == 510);
        assert (deadband.addSample (501) == 510);
        assert (deadband.addSample (500) == 500);
    }

    void testZeroWidthDeadbandTracksEverySample ()
    {
        adk::Deadband deadband (0);

        assert (deadband.addSample (100) == 100);
        assert (deadband.addSample (101) == 101);
        assert (deadband.width () == 0);

        deadband.reset             ();

        assert (!deadband.hasValue ());
    }

    void testDeadbandHandlesExtremeWidthAndResetReplay ()
    {
        adk::Deadband deadband (UINT16_MAX);

        assert (deadband.addSample (0) == 0);
        assert (deadband.addSample (UINT16_MAX - 1U) == 0);
        assert (deadband.addSample (UINT16_MAX) == UINT16_MAX);

        deadband.reset ();

        assert (deadband.addSample (0) == 0);
        assert (deadband.addSample (UINT16_MAX - 1U) == 0);
        assert (deadband.addSample (UINT16_MAX) == UINT16_MAX);
    }
}

int main ()
{
    static_assert (!std::is_polymorphic<adk::LinearCalibration>::value, "");
    static_assert (!std::is_polymorphic<adk::MovingAverage>::value, "");
    static_assert (!std::is_polymorphic<adk::Deadband>::value, "");

    testCalibrationRejectsInvalidRange                      ();
    testCalibrationMapsAndRounds                            ();
    testCalibrationClampsOrRejectsOutsideObservations       ();
    testCalibrationSupportsDescendingOutput                 ();
    testCalibrationPreservesExtremeEndpointsAndMonotonicity ();
    testMovingAverageRejectsInvalidWindows                  ();
    testMovingAverageWarmsAndRolls                          ();
    testMovingAverageUsesFullWidthSum                       ();
    testMovingAverageResetReplaysDeterministically          ();
    testMovingAverageMatchesReferenceAcrossRingWraps        ();
    testDeadbandHoldsSmallChanges                           ();
    testZeroWidthDeadbandTracksEverySample                  ();
    testDeadbandHandlesExtremeWidthAndResetReplay           ();
}
