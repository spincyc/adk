#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "calibration_console.h"

namespace {

    adk::CalibrationConsoleConfig config (
        uint16_t lower = 100, uint16_t upper = 900, uint16_t separation = 100,
        uint32_t acknowledgement = 10) noexcept
    {
        return {lower, upper, separation, adk::Duration (acknowledgement)};
    }

    adk::CalibrationConsoleInput input (
        int16_t x = 0, int16_t y = 0, bool select = false, int8_t delta = 0,
        bool cancel = false, bool valid = true) noexcept
    {
        adk::CalibrationConsoleInput result;

        result.joystickX   = x;
        result.joystickY   = y;
        result.selectEvent = select;
        result.encoderDelta = delta;
        result.cancelEvent = cancel;
        result.inputValid  = valid;
        return result;
    }

    void expectPair (const adk::CalibrationConsoleSnapshot& snapshot,
                     uint16_t committedMinimum, uint16_t committedMaximum,
                     uint16_t previewMinimum, uint16_t previewMaximum) noexcept
    {
        assert               (snapshot.committedMinimum == committedMinimum);
        assert               (snapshot.committedMaximum == committedMaximum);
        assert               (snapshot.previewMinimum == previewMinimum);
        assert               (snapshot.previewMaximum == previewMaximum);
    }

    void startEditing (adk::CalibrationConsole& console, adk::TimePoint now,
                       int16_t fieldX = -501) noexcept
    {
        assert               (console.update (now, input (fieldX)).ok ());
        assert               (console.update (adk::TimePoint (now.milliseconds () + 1),
                                input (0, 0, true))
                    .ok ());
        assert (console.snapshot ().state ==
                adk::CalibrationConsoleState::Editing);
    }

    void testConfigurationAndLifecycle ()
    {
        adk::CalibrationConsole console (config ());

        assert               (!console.initialized ());
        assert               (console.initialize (200, 700).ok ());
        assert               (console.initialized ());
        assert               (console.initialize (200, 700).ok ());

        adk::CalibrationConsoleSnapshot snapshot = console.snapshot ();

        assert                       (snapshot.state == adk::CalibrationConsoleState::Selecting);
        assert                       (snapshot.field == adk::CalibrationField::Minimum);
        assert                       (!snapshot.changed);
        assert                       (snapshot.status.ok ());
        expectPair                   (snapshot, 200, 700, 200, 700);

        console.shutdown                                               ();
        console.shutdown                                               ();
        assert                                                         (!console.initialized ());
        snapshot = console.snapshot                                    ();
        expectPair                                                     (snapshot, 200, 700, 200, 700);
        assert                                                         (console.update (adk::TimePoint (0), input ()).error () ==
                adk::StatusCode::NotInitialized);

        assert                       (console.initialize (250, 800).ok ());
        expectPair                   (console.snapshot (), 250, 800, 250, 800);

        adk::CalibrationConsole reversed                                         (config (900, 100, 10));
        assert                                                                   (reversed.initialize (200, 700).error () ==
                adk::StatusCode::InvalidConfiguration);
        assert (!reversed.initialized ());

        adk::CalibrationConsole impossible                                           (config (100, 150, 51));
        assert                                                                       (impossible.initialize (100, 150).error () ==
                adk::StatusCode::InvalidConfiguration);

        adk::CalibrationConsole overflow                                         (config (0, UINT16_MAX, UINT16_MAX));
        assert                                                                   (overflow.initialize (0, UINT16_MAX).ok ());
        assert                                                                   (overflow.snapshot ().status.ok ());

        adk::CalibrationConsole invalidLow                                           (config ());
        assert                                                                       (invalidLow.initialize (99, 700).error () ==
                adk::StatusCode::InvalidConfiguration);

        adk::CalibrationConsole invalidHigh                                            (config ());
        assert                                                                         (invalidHigh.initialize (200, 901).error () ==
                adk::StatusCode::InvalidConfiguration);

        adk::CalibrationConsole invalidOrder                                             (config ());
        assert                                                                           (invalidOrder.initialize (700, 700).error () ==
                adk::StatusCode::InvalidConfiguration);

        adk::CalibrationConsole invalidSeparation                                                  (config ());
        assert                                                                                     (invalidSeparation.initialize (650, 700).error () ==
                adk::StatusCode::InvalidConfiguration);

        adk::CalibrationConsole noAcknowledgement                                                  (config (100, 900, 100, 0));
        assert                                                                                     (noAcknowledgement.initialize (200, 700).error () ==
                adk::StatusCode::InvalidConfiguration);

        adk::CalibrationConsole ambiguousTime (
            config (100, 900, 100, UINT32_C (0x80000000)));
        assert (ambiguousTime.initialize (200, 700).error () ==
                adk::StatusCode::InvalidConfiguration);
    }

    void testSelectionAndCenterBands ()
    {
        adk::CalibrationConsole console (config ());

        assert               (console.initialize (200, 700).ok ());
        assert               (console.update (adk::TimePoint (0), input (501)).ok ());
        assert               (console.snapshot ().field == adk::CalibrationField::Maximum);

        assert               (console.update (adk::TimePoint (1), input (500)).ok ());
        assert               (console.snapshot ().field == adk::CalibrationField::Maximum);
        assert               (console.update (adk::TimePoint (2), input (-500)).ok ());
        assert               (console.snapshot ().field == adk::CalibrationField::Maximum);

        assert               (console.update (adk::TimePoint (3), input (-501)).ok ());
        assert               (console.snapshot ().field == adk::CalibrationField::Minimum);
        assert               (console.update (adk::TimePoint (4), input (0, 0, true)).ok ());
        assert               (console.snapshot ().state ==
                adk::CalibrationConsoleState::Editing);

        assert                                                                                   (console.update (adk::TimePoint (5), input (0, 501)).ok ());
        const uint16_t coarse = console.snapshot                                                 ().previewMinimum;
        assert                                                                                   (coarse >= 100);
        assert                                                                                   (coarse <= 600);

        assert               (console.update (adk::TimePoint (6), input (0, 500)).ok ());
        assert               (console.snapshot ().previewMinimum == coarse);
        assert               (console.update (adk::TimePoint (7), input (0, -500)).ok ());
        assert               (console.snapshot ().previewMinimum == coarse);
    }

    void testCoarseEndpointsClampsAndTrim ()
    {
        adk::CalibrationConsole minimum (config ());

        assert                           (minimum.initialize (200, 700).ok ());
        startEditing                     (minimum, adk::TimePoint (0));

        assert (minimum
                    .update                            (adk::TimePoint (2), input (0, INT16_MIN, false, -1))
                    .ok                                ());
        assert (minimum.snapshot ().previewMinimum == 100);

        assert (minimum
                    .update                            (adk::TimePoint (3), input (0, INT16_MAX, false, 1))
                    .ok                                ());
        assert (minimum.snapshot ().previewMinimum == 600);

        assert               (minimum.update (adk::TimePoint (4), input (0, 0, false, -1)).ok ());
        assert               (minimum.snapshot ().previewMinimum == 599);
        assert               (minimum.update (adk::TimePoint (5), input (0, 0, false, 1)).ok ());
        assert               (minimum.snapshot ().previewMinimum == 600);

        adk::CalibrationConsole maximum (config ());

        assert                           (maximum.initialize (200, 700).ok ());
        startEditing                     (maximum, adk::TimePoint (0), 501);
        assert                           (maximum
                    .update                            (adk::TimePoint (2), input (0, INT16_MIN, false, -1))
                    .ok                                ());
        assert (maximum.snapshot ().previewMaximum == 300);

        assert (maximum
                    .update                            (adk::TimePoint (3), input (0, INT16_MAX, false, 1))
                    .ok                                ());
        assert (maximum.snapshot ().previewMaximum == 900);

        assert               (maximum.update (adk::TimePoint (4), input (0, 0, false, -1)).ok ());
        assert               (maximum.snapshot ().previewMaximum == 899);
        assert               (maximum.update (adk::TimePoint (5), input (0, 0, false, 1)).ok ());
        assert               (maximum.snapshot ().previewMaximum == 900);
    }

    void testCommitAcknowledgementAndRollover ()
    {
        adk::CalibrationConsole console (config ());

        assert                           (console.initialize (200, 700).ok ());
        startEditing                     (console, adk::TimePoint (UINT32_MAX - 3));
        assert                           (console
                    .update (adk::TimePoint (UINT32_MAX - 1),
                             input (0, 0, false, 7))
                    .ok ());
        assert               (console.snapshot ().previewMinimum == 207);
        assert               (console
                    .update                            (adk::TimePoint (UINT32_MAX), input (0, 0, true))
                    .ok                                ());

        adk::CalibrationConsoleSnapshot snapshot = console.snapshot ();

        assert                       (snapshot.state == adk::CalibrationConsoleState::Committed);
        assert                       (!snapshot.changed);
        expectPair                   (snapshot, 207, 700, 207, 700);

        assert               (console.update (adk::TimePoint (8), input ()).ok ());
        assert               (console.snapshot ().state ==
                adk::CalibrationConsoleState::Committed);
        assert               (console.update (adk::TimePoint (9), input ()).ok ());
        assert               (console.snapshot ().state ==
                adk::CalibrationConsoleState::Selecting);

        startEditing                                                   (console, adk::TimePoint (10));
        assert                                                         (console.update (adk::TimePoint (12), input (0, 0, true)).ok ());
        snapshot = console.snapshot                                    ();
        assert                                                         (snapshot.state == adk::CalibrationConsoleState::Committed);
        assert                                                         (!snapshot.changed);

        assert               (console.update (adk::TimePoint (13), input (0, 0, true)).ok ());
        assert               (console.snapshot ().state ==
                adk::CalibrationConsoleState::Committed);
    }

    void testCancelRollbackAndPrecedence ()
    {
        adk::CalibrationConsole console (config ());

        assert                           (console.initialize (200, 700).ok ());
        startEditing                     (console, adk::TimePoint (0), 501);
        assert                           (console.update (adk::TimePoint (2), input (0, 0, false, 9)).ok ());
        assert                           (console.snapshot ().previewMaximum == 709);

        assert (console
                    .update                            (adk::TimePoint (3), input (0, 0, true, 0, true))
                    .ok                                ());
        adk::CalibrationConsoleSnapshot snapshot = console.snapshot ();

        assert                       (snapshot.state == adk::CalibrationConsoleState::Cancelled);
        assert                       (!snapshot.changed);
        expectPair                   (snapshot, 200, 700, 200, 700);

        assert (console.update (adk::TimePoint (12), input (0, 0, false, 0, true))
                    .ok ());
        assert (console.snapshot ().state ==
                adk::CalibrationConsoleState::Cancelled);
        assert               (console.update (adk::TimePoint (13), input ()).ok ());
        assert               (console.snapshot ().state ==
                adk::CalibrationConsoleState::Selecting);

        assert (console.update (adk::TimePoint (14), input (0, 0, false, 0, true))
                    .ok ());
        assert (console.snapshot ().state ==
                adk::CalibrationConsoleState::Selecting);
    }

    void testFaultAndExplicitRecovery ()
    {
        adk::CalibrationConsole console (config ());

        assert               (console.initialize (200, 700).ok ());
        assert               (console.update (adk::TimePoint (100), input ()).ok ());
        assert               (console.update (adk::TimePoint (99), input ()).error () ==
                adk::StatusCode::InvalidArgument);
        assert (console.snapshot ().state ==
                adk::CalibrationConsoleState::Fault);

        assert (console.update (adk::TimePoint (101), input ()).error () ==
                adk::StatusCode::InvalidArgument);
        assert (console.initialize (200, 700).ok ());

        startEditing                     (console, adk::TimePoint (200));
        assert                           (console.update (adk::TimePoint (202), input (0, 0, false, 5)).ok ());
        assert                           (console.update (adk::TimePoint (203), input (0, 0, false, 0,
                                                            false, false))
                    .error () == adk::StatusCode::HardwareFailure);
        assert (console.snapshot ().state ==
                adk::CalibrationConsoleState::Fault);
        expectPair (console.snapshot (), 200, 700, 200, 700);

        assert (console.update (adk::TimePoint (204), input (0, 0, true)).error () ==
                adk::StatusCode::HardwareFailure);
        expectPair (console.snapshot (), 200, 700, 200, 700);

        assert               (console.initialize (250, 750).ok ());
        assert               (console.snapshot ().state ==
                adk::CalibrationConsoleState::Selecting);
        expectPair (console.snapshot (), 250, 750, 250, 750);
    }

    void testShutdownFromEveryState ()
    {
        for (uint8_t target = 0; target < 5; ++target)
        {
            adk::CalibrationConsole console (config ());

            assert (console.initialize (200, 700).ok ());

            if (target >= 1)
            {
                startEditing (console, adk::TimePoint (0));
            }
            if (target == 2)
            {
                assert (console.update (adk::TimePoint (2), input (0, 0, true)).ok ());
            }
            else if (target == 3)
            {
                assert (console
                            .update (adk::TimePoint (2),
                                     input (0, 0, false, 0, true))
                            .ok ());
            }
            else if (target == 4)
            {
                assert (console.update (adk::TimePoint (2),
                                        input (0, 0, false, 0, false, false))
                            .error () == adk::StatusCode::HardwareFailure);
            }

            console.shutdown                             ();
            assert                                       (!console.initialized ());
            expectPair                                   (console.snapshot (), 200, 700, 200, 700);
        }
    }

    void testReplayIsByteIdentical ()
    {
        adk::CalibrationConsole left                                       (config ());
        adk::CalibrationConsole right                                      (config ());

        assert               (left.initialize (200, 700).ok ());
        assert               (right.initialize (200, 700).ok ());

        const adk::TimePoint times[] = {
            adk::TimePoint                           (0), adk::TimePoint (1), adk::TimePoint (2),
            adk::TimePoint                           (3), adk::TimePoint (4), adk::TimePoint (14)};
        const adk::CalibrationConsoleInput inputs[] = {
            input                  (501), input (0, 0, true), input (0, 700),
            input                  (0, 0, false, -3), input (0, 0, true), input ()};

        for (uint8_t index = 0; index < sizeof (times) / sizeof (times[0]);
             ++index)
        {
            assert (left.update (times[index], inputs[index]) ==
                    right.update (times[index], inputs[index]));

            const adk::CalibrationConsoleSnapshot leftSnapshot = left.snapshot                                                                                   ();
            const adk::CalibrationConsoleSnapshot rightSnapshot = right.snapshot                                                                                 ();

            assert (memcmp (&leftSnapshot, &rightSnapshot,
                            sizeof (adk::CalibrationConsoleSnapshot)) == 0);
        }
    }
}

int main ()
{
    testConfigurationAndLifecycle                                                ();
    testSelectionAndCenterBands                                                  ();
    testCoarseEndpointsClampsAndTrim                                             ();
    testCommitAcknowledgementAndRollover                                         ();
    testCancelRollbackAndPrecedence                                              ();
    testFaultAndExplicitRecovery                                                 ();
    testShutdownFromEveryState                                                   ();
    testReplayIsByteIdentical                                                    ();
    return 0;
}
