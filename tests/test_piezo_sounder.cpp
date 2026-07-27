#include <Arduino.h>
#include <piezo_sounder.h>

#include <cstdlib>
#include <iostream>

namespace {
    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    void requireStatus (adk::Status actual, adk::Status expected, const char* message)
    {
        require (actual == expected, message);
    }

    void resetFakes ()
    {
        adk::test::arduino::reset ();
    }

    std::size_t toneCallCount ()
    {
        std::size_t count = 0;

        for (const adk::test::arduino::Operation& operation :
             adk::test::arduino::trace ())
        {
            if (operation.kind == adk::test::arduino::OperationKind::Tone ||
                operation.kind == adk::test::arduino::OperationKind::NoTone)
            {
                ++count;
            }
        }

        return count;
    }

    void requireTone (std::size_t index, adk::test::arduino::OperationKind kind,
                      uint8_t pin, unsigned int frequency, const char* message)
    {
        std::size_t seen = 0;

        for (const adk::test::arduino::Operation& operation :
             adk::test::arduino::trace ())
        {
            if (operation.kind != adk::test::arduino::OperationKind::Tone &&
                operation.kind != adk::test::arduino::OperationKind::NoTone)
            {
                continue;
            }

            if (seen == index)
            {
                require (operation.kind == kind, message);
                require (operation.pin == pin, message);
                require (operation.value == static_cast<int> (frequency), message);
                return;
            }

            ++seen;
        }

        require (false, message);
    }

    void testLifecycle ()
    {
        resetFakes ();

        adk::ResourceRegistry resources;
        adk::PiezoSounder     sounder (resources, 8);

        require       (!sounder.initialized (), "constructed inert");
        require       (!sounder.sounding (), "constructed silent");
        require       (sounder.pin () == 8, "configured pin");
        require       (sounder.frequency () == 0, "constructed frequency");
        requireStatus (sounder.play (440, adk::Duration (100), adk::TimePoint (0)),
                       adk::Status::NotInitialized, "play before initialize");

        requireStatus (sounder.initialize (), adk::Status::Ok, "initialize");
        requireStatus (sounder.initialize (), adk::Status::Ok, "initialize idempotent");
        require       (sounder.initialized (), "initialized state");
        require       (resources.claimed ({adk::ResourceKind::Pin, 8}), "pin claimed");
        require       (resources.claimed ({adk::ResourceKind::Timer, 2}),
                 "tone timer claimed");
        require (adk::test::arduino::mode (8) == INPUT,
                 "initialize leaves pin high impedance");

        sounder.shutdown ();
        sounder.shutdown ();

        require (!sounder.initialized (), "shutdown state");
        require (!sounder.sounding (), "shutdown silent");
        require (!resources.claimed ({adk::ResourceKind::Pin, 8}),
                 "shutdown releases pin");
        require (!resources.claimed ({adk::ResourceKind::Timer, 2}),
                 "shutdown releases timer");
        require (toneCallCount () == 0, "silent shutdown avoids noTone");
    }

    void testInvalidPin ()
    {
        resetFakes ();

        adk::ResourceRegistry resources;
        adk::PiezoSounder     sounder (resources, NUM_DIGITAL_PINS);

        requireStatus (sounder.initialize (), adk::Status::InvalidPin,
                       "invalid pin rejected");
        require (!sounder.initialized (), "invalid pin remains inert");
        require (adk::test::arduino::trace ().empty (), "invalid pin no I/O");
    }

    void testPinConflict ()
    {
        resetFakes ();

        adk::ResourceRegistry resources;
        adk::ResourceClaim    blocker;

        requireStatus (resources.claim ({adk::ResourceKind::Pin, 8}, blocker),
                       adk::Status::Ok, "pin blocker");

        adk::PiezoSounder sounder (resources, 8);

        requireStatus (sounder.initialize (), adk::Status::ResourceBusy,
                       "pin conflict");
        require (!sounder.initialized (), "pin conflict remains inert");
        require (!resources.claimed ({adk::ResourceKind::Timer, 2}),
                 "pin conflict does not claim timer");
        require (adk::test::arduino::trace ().empty (), "pin conflict no I/O");
    }

    void testTimerConflictRollback ()
    {
        resetFakes ();

        adk::ResourceRegistry resources;
        adk::ResourceClaim    blocker;

        requireStatus (resources.claim ({adk::ResourceKind::Timer, 2}, blocker),
                       adk::Status::Ok, "timer blocker");

        adk::PiezoSounder sounder (resources, 8);

        requireStatus (sounder.initialize (), adk::Status::ResourceBusy,
                       "timer conflict");
        require (!sounder.initialized (), "timer conflict remains inert");
        require (!resources.claimed ({adk::ResourceKind::Pin, 8}),
                 "timer conflict rolls back pin");
        require (adk::test::arduino::trace ().empty (), "timer conflict no I/O");

        blocker.release  ();
        requireStatus    (sounder.initialize (), adk::Status::Ok,
                       "initialize after timer release");
    }

    void testFrequencyAndDurationBounds ()
    {
        resetFakes ();

        adk::ResourceRegistry resources;
        adk::PiezoSounder     sounder (resources, 8);

        requireStatus (sounder.initialize (), adk::Status::Ok, "bounds initialize");

        requireStatus (sounder.play (adk::PiezoSounder::minimumFrequencyHz - 1u,
                                     adk::Duration (1), adk::TimePoint (0)),
                       adk::Status::InvalidArgument, "frequency below minimum");
        requireStatus (sounder.play (adk::PiezoSounder::maximumFrequencyHz + 1u,
                                     adk::Duration (1), adk::TimePoint (0)),
                       adk::Status::InvalidArgument, "frequency above maximum");
        requireStatus (sounder.play (adk::PiezoSounder::minimumFrequencyHz,
                                     adk::Duration (0), adk::TimePoint (0)),
                       adk::Status::InvalidArgument, "zero duration");
        requireStatus (
            sounder.play (adk::PiezoSounder::minimumFrequencyHz,
                          adk::Duration  (adk::PiezoSounder::maximumDurationMs + 1u),
                          adk::TimePoint (0)),
            adk::Status::InvalidArgument, "duration above maximum");
        require (toneCallCount () == 0, "invalid requests make no tone");
        require (!sounder.sounding (), "invalid requests remain silent");

        requireStatus (sounder.play (adk::PiezoSounder::minimumFrequencyHz,
                                     adk::Duration (1), adk::TimePoint (10)),
                       adk::Status::Ok, "minimum frequency accepted");
        sounder.stop  ();
        requireStatus (
            sounder.play (adk::PiezoSounder::maximumFrequencyHz,
                          adk::Duration  (adk::PiezoSounder::maximumDurationMs),
                          adk::TimePoint (20)),
            adk::Status::Ok, "maximum bounds accepted");
    }

    void testExplicitCompletion ()
    {
        resetFakes ();

        adk::ResourceRegistry resources;
        adk::PiezoSounder     sounder (resources, 8);

        requireStatus                  (sounder.initialize (), adk::Status::Ok, "completion initialize");
        adk::test::arduino::clearTrace ();

        requireStatus (sounder.play (440, adk::Duration (100), adk::TimePoint (1000)),
                       adk::Status::Ok, "play tone");
        require     (sounder.sounding (), "tone sounding");
        require     (sounder.frequency () == 440, "tone frequency");
        require     (adk::test::arduino::toneFrequency (8) == 440,
                     "fake records active frequency");
        requireTone (0, adk::test::arduino::OperationKind::Tone, 8, 440, "tone trace");

        sounder.update (adk::TimePoint (1099));
        require        (sounder.sounding (), "tone before deadline");
        require        (toneCallCount () == 1, "no early noTone");

        sounder.update (adk::TimePoint (1100));
        require        (!sounder.sounding (), "tone stops at deadline");
        require        (sounder.frequency () == 0, "stopped frequency");
        require        (adk::test::arduino::toneFrequency (8) == 0,
                        "fake records silence");
        requireTone    (1, adk::test::arduino::OperationKind::NoTone, 8, 0,
                     "noTone trace");
        require (adk::test::arduino::mode (8) == INPUT,
                 "completion restores high impedance");

        sounder.update (adk::TimePoint (1200));
        sounder.stop   ();
        require        (toneCallCount () == 2, "completion and stop idempotent");
    }

    void testReplacementResetsDeadline ()
    {
        resetFakes ();

        adk::ResourceRegistry resources;
        adk::PiezoSounder     sounder (resources, 8);

        requireStatus (sounder.initialize (), adk::Status::Ok,
                       "replacement initialize");
        requireStatus (sounder.play (440, adk::Duration (100), adk::TimePoint (10)),
                       adk::Status::Ok, "first tone");
        requireStatus (sounder.play (880, adk::Duration (50), adk::TimePoint (80)),
                       adk::Status::Ok, "replacement tone");

        require     (sounder.frequency () == 880, "replacement frequency");
        requireTone (0, adk::test::arduino::OperationKind::Tone, 8, 440,
                     "first tone trace");
        requireTone (1, adk::test::arduino::OperationKind::Tone, 8, 880,
                     "replacement tone trace");

        sounder.update (adk::TimePoint (110));
        require        (sounder.sounding (), "old deadline ignored");
        sounder.update (adk::TimePoint (130));
        require        (!sounder.sounding (), "replacement deadline used");
        requireTone    (2, adk::test::arduino::OperationKind::NoTone, 8, 0,
                     "replacement stop trace");
    }

    void testInvalidReplacementPreservesTone ()
    {
        resetFakes ();

        adk::ResourceRegistry resources;
        adk::PiezoSounder     sounder (resources, 8);

        requireStatus (sounder.initialize (), adk::Status::Ok,
                       "preservation initialize");
        requireStatus (sounder.play (440, adk::Duration (100), adk::TimePoint (10)),
                       adk::Status::Ok, "preservation tone");
        requireStatus (sounder.play (0, adk::Duration (1), adk::TimePoint (20)),
                       adk::Status::InvalidArgument,
                       "invalid replacement rejected");
        require (sounder.sounding (), "invalid replacement preserves sounding state");
        require (sounder.frequency () == 440,
                 "invalid replacement preserves frequency");
        require (adk::test::arduino::toneFrequency (8) == 440,
                 "invalid replacement preserves hardware");
        require (toneCallCount () == 1, "invalid replacement emits no operation");

        sounder.update (adk::TimePoint (109));
        require        (sounder.sounding (), "original deadline remains active");
        sounder.update (adk::TimePoint (110));
        require        (!sounder.sounding (), "original deadline completes");
    }

    void testClockWrap ()
    {
        resetFakes ();

        adk::ResourceRegistry resources;
        adk::PiezoSounder     sounder (resources, 8);
        const uint32_t        start = 0xfffffff0u;

        requireStatus (sounder.initialize (), adk::Status::Ok, "wrap initialize");
        requireStatus (sounder.play (1000, adk::Duration (32), adk::TimePoint (start)),
                       adk::Status::Ok, "wrap play");

        sounder.update (adk::TimePoint (15));
        require        (sounder.sounding (), "wrap before deadline");
        sounder.update (adk::TimePoint (16));
        require        (!sounder.sounding (), "wrap exact deadline");
        requireTone    (1, adk::test::arduino::OperationKind::NoTone, 8, 0, "wrap noTone");
    }

    void testShutdownAndDestruction ()
    {
        resetFakes ();

        adk::ResourceRegistry resources;

        {
            adk::PiezoSounder sounder (resources, 8);

            requireStatus (sounder.initialize (), adk::Status::Ok,
                           "shutdown initialize");
            requireStatus (sounder.play (1000, adk::Duration (100), adk::TimePoint (0)),
                           adk::Status::Ok, "shutdown play");

            sounder.shutdown ();

            require     (!sounder.sounding (), "shutdown stops tone");
            requireTone (1, adk::test::arduino::OperationKind::NoTone, 8, 0,
                         "shutdown noTone");
            require (adk::test::arduino::mode (8) == INPUT, "shutdown high impedance");
        }

        require (!resources.claimed ({adk::ResourceKind::Pin, 8}),
                 "destruction leaves pin released");
        require (!resources.claimed ({adk::ResourceKind::Timer, 2}),
                 "destruction leaves timer released");

        adk::test::arduino::clearTrace ();

        {
            adk::PiezoSounder sounder (resources, 8);

            requireStatus (sounder.initialize (), adk::Status::Ok,
                           "destructor initialize");
            requireStatus (sounder.play (500, adk::Duration (100), adk::TimePoint (0)),
                           adk::Status::Ok, "destructor play");
        }

        requireTone (1, adk::test::arduino::OperationKind::NoTone, 8, 0,
                     "destructor noTone");
        require (!resources.claimed ({adk::ResourceKind::Pin, 8}),
                 "destructor releases pin");
        require (!resources.claimed ({adk::ResourceKind::Timer, 2}),
                 "destructor releases timer");
    }
} // namespace

int main ()
{
    testLifecycle                        ();
    testInvalidPin                       ();
    testPinConflict                      ();
    testTimerConflictRollback            ();
    testFrequencyAndDurationBounds       ();
    testExplicitCompletion               ();
    testReplacementResetsDeadline        ();
    testInvalidReplacementPreservesTone  ();
    testClockWrap                        ();
    testShutdownAndDestruction           ();

    std::cout << "All ADK piezo sounder tests passed.\n";
}
