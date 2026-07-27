#include <board.h>
#include <digital_output.h>
#include <pwm_output.h>

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

    void requireOperation (std::size_t index, fake::OperationKind kind, uint8_t pin,
                           int value)
    {
        const auto& trace = fake::trace ();

        require (index < trace.size (), "missing operation");
        require (trace[index].kind == kind, "wrong operation kind");
        require (trace[index].pin == pin, "wrong operation pin");
        require (trace[index].value == value, "wrong operation value");
    }

    bool expectedPwmPin (adk::PinId pin)
    {
        return (pin >= 2 && pin <= 13) || (pin >= 44 && pin <= 46);
    }

    void testMegaCapabilities ()
    {
        for (uint16_t value = 0; value <= 255; ++value)
        {
            const auto pin   = static_cast<adk::PinId> (value);
            const bool valid = value <= 69;

            require (adk::Mega2560Board::validPin (pin) == valid, "Mega pin validity");
            require (
                adk::Mega2560Board::supports (pin, adk::PinCapability::PwmOutput) ==
                    (valid && expectedPwmPin (pin)),
                "Mega PWM capability");
        }
    }

    void testEveryPwmPin ()
    {
        const adk::PinId pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 44, 45, 46};

        for (const auto pin : pins)
        {
            fake::reset ();
            adk::ResourceRegistry resources;
            adk::PwmOutput        output (resources, pin, 127);

            require          (output.initialize ().ok (), "PWM pin initialization");
            require          (output.initialized (), "PWM pin initialized state");
            require          (output.pin () == pin, "PWM pin accessor");
            require          (output.duty () == 127, "PWM initial duty");
            require          (fake::trace ().size () == 1, "PWM initialization trace");
            requireOperation (0, fake::OperationKind::AnalogWrite, pin, 127);

            fake::clearTrace ();
            output.shutdown  ();
            require          (fake::trace ().size () == 2, "PWM shutdown trace");
            requireOperation (0, fake::OperationKind::AnalogWrite, pin, 0);
            requireOperation (1, fake::OperationKind::PinMode, pin, INPUT);
        }
    }

    void testDutyEndpointsAndLifecycle ()
    {
        fake::reset ();
        adk::ResourceRegistry resources;
        adk::PwmOutput        output (resources, 11);

        require (output.pin () == 11, "PWM pin");
        require (output.duty () == 0, "PWM default duty");
        require (!output.initialized (), "PWM starts uninitialized");
        require (output.write (255).error () == adk::StatusCode::NotInitialized,
                 "PWM rejects pre-initialization write");
        require (fake::trace ().empty (), "inactive PWM touched hardware");

        require          (output.initialize ().ok (), "PWM initialization");
        requireOperation (0, fake::OperationKind::AnalogWrite, 11, 0);

        fake::clearTrace ();
        require          (output.initialize ().ok (),
                 "PWM repeated initialization");
        require (fake::trace ().empty (),
                 "repeated PWM initialization touched hardware");

        require          (output.write (255).ok (), "PWM maximum duty");
        require          (output.duty () == 255, "PWM maximum duty state");
        requireOperation (0, fake::OperationKind::AnalogWrite, 11, 255);

        require          (output.write (0).ok (), "PWM minimum duty");
        require          (output.duty () == 0, "PWM minimum duty state");
        requireOperation (1, fake::OperationKind::AnalogWrite, 11, 0);

        fake::clearTrace ();
        output.shutdown  ();
        require          (!output.initialized (), "PWM shutdown state");
        require          (output.duty () == 0, "PWM shutdown duty");
        requireOperation (0, fake::OperationKind::AnalogWrite, 11, 0);
        requireOperation (1, fake::OperationKind::PinMode, 11, INPUT);

        fake::clearTrace ();
        output.shutdown  ();
        require          (fake::trace ().empty (), "repeated PWM shutdown touched hardware");

        require (output.initialize ().ok (), "PWM restart");
        require (output.duty () == 0, "PWM restart initial duty");
    }

    void testUnsupportedAndInvalidPins ()
    {
        const adk::PinId unsupportedPins[] = {0, 1, 14, 43, 47, 54, 69};

        for (const auto pin : unsupportedPins)
        {
            fake::reset ();
            adk::ResourceRegistry resources;
            adk::PwmOutput        output (resources, pin);

            require (output.initialize ().error () == adk::StatusCode::Unsupported,
                     "non-PWM pin status");
            require (!output.initialized (), "non-PWM output remains inactive");
            require (fake::trace ().empty (), "non-PWM pin touched hardware");
        }

        fake::reset ();
        adk::ResourceRegistry resources;
        adk::PwmOutput        invalid (resources, 70);

        require (invalid.initialize ().error () == adk::StatusCode::InvalidPin, "invalid PWM pin");
        require (!invalid.initialized (), "invalid PWM remains inactive");
        require (fake::trace ().empty (), "invalid PWM pin touched hardware");
    }

    void testClaimConflictAndReuse ()
    {
        fake::reset ();
        adk::ResourceRegistry resources;
        adk::DigitalOutput    owner   (resources, 10);
        adk::PwmOutput        blocked (resources, 10, 64);

        require          (owner.initialize ().ok (), "pin owner initialization");
        fake::clearTrace ();
        require          (blocked.initialize ().error () == adk::StatusCode::ResourceBusy,
                 "PWM conflict status");
        require (!blocked.initialized (), "blocked PWM remains inactive");
        require (blocked.duty () == 64, "blocked PWM preserves initial duty");
        require (fake::trace ().empty (), "PWM conflict touched hardware");

        owner.shutdown   ();
        fake::clearTrace ();
        require          (blocked.initialize ().ok (), "PWM claim reuse");
        requireOperation (0, fake::OperationKind::AnalogWrite, 10, 64);
    }

    void testTimerSharingAndRollback ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;
        adk::PwmOutput        first  (resources, 9,  32);
        adk::PwmOutput        second (resources, 10, 64);

        require (first.initialize ().ok (),
                 "first timer peer initializes");
        require (second.initialize ().ok (),
                 "second timer peer shares timer");
        require (resources.claimed ({adk::ResourceKind::Timer, 2}),
                 "shared PWM timer claimed");

        first.shutdown ();

        require (resources.claimed ({adk::ResourceKind::Timer, 2}),
                 "timer remains claimed by peer");

        second.shutdown ();

        require (!resources.claimed ({adk::ResourceKind::Timer, 2}),
                 "last peer releases timer");

        adk::ResourceClaim timerOwner;

        require (resources.claim ({adk::ResourceKind::Timer, 2}, timerOwner)
                     .ok (),
                 "exclusive timer owner");

        adk::PwmOutput blocked (resources, 9, 91);

        fake::clearTrace ();

        require (blocked.initialize ().error () == adk::StatusCode::ResourceBusy,
                 "exclusive timer blocks PWM");
        require (!resources.claimed ({adk::ResourceKind::Pin, 9}),
                 "timer failure rolls back pin");
        require (fake::trace ().empty (), "timer failure performs no I/O");

        timerOwner.release ();

        require (blocked.initialize ().ok (),
                 "PWM initializes after timer release");
    }

    void testDestruction ()
    {
        fake::reset ();
        adk::ResourceRegistry resources;

        {
            adk::PwmOutput output (resources, 6, 200);

            require (output.initialize ().ok (),
                     "scoped PWM initialization");
            fake::clearTrace ();
        }

        require          (fake::trace ().size () == 2, "PWM destruction trace");
        requireOperation (0, fake::OperationKind::AnalogWrite, 6, 0);
        requireOperation (1, fake::OperationKind::PinMode, 6, INPUT);

        fake::clearTrace           ();
        adk::PwmOutput replacement (resources, 6);
        require                    (replacement.initialize ().ok (),
                 "PWM destruction released claim");
    }
} // namespace

int main ()
{
    static_assert (!std::is_copy_constructible<adk::PwmOutput>::value, "");
    static_assert (!std::is_move_constructible<adk::PwmOutput>::value, "");

    testMegaCapabilities          ();
    testEveryPwmPin               ();
    testDutyEndpointsAndLifecycle ();
    testUnsupportedAndInvalidPins ();
    testClaimConflictAndReuse     ();
    testTimerSharingAndRollback   ();
    testDestruction               ();

    std::cout << "All ADK PWM output tests passed.\n";
}
