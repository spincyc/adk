#include <mono_led.h>

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

void testActiveHighLifecycle ()
{
    fake::reset ();
    adk::ResourceRegistry resources;
    adk::MonoLed          led (resources, 13);

    require (led.pin () == 13, "LED pin");
    require (led.activeHigh (), "LED active polarity");
    require (!led.active (), "LED starts inactive");
    require (!led.initialized (), "LED starts uninitialized");
    require (led.on ().error () == adk::StatusCode::NotInitialized, "inactive LED rejects on");
    require (led.off ().error () == adk::StatusCode::NotInitialized, "inactive LED rejects off");
    require (fake::trace ().empty (), "inactive LED touched hardware");

    require          (led.initialize ().ok (), "LED initialization");
    require          (led.initialized (), "LED initialized state");
    require          (!led.active (), "LED initializes inactive");
    require          (fake::trace ().size () == 2, "LED initialization trace");
    requireOperation (0, fake::OperationKind::DigitalWrite, 13, LOW);
    requireOperation (1, fake::OperationKind::PinMode, 13, OUTPUT);

    fake::clearTrace ();
    require          (led.initialize ().ok (), "LED repeated initialization");
    require          (fake::trace ().empty (), "repeated LED initialization touched hardware");

    require          (led.on ().ok (), "LED on");
    require          (led.active (), "LED active state");
    requireOperation (0, fake::OperationKind::DigitalWrite, 13, HIGH);

    require          (led.set (false).ok (), "LED set inactive");
    require          (!led.active (), "LED inactive state");
    requireOperation (1, fake::OperationKind::DigitalWrite, 13, LOW);

    fake::clearTrace ();
    require          (led.on ().ok (), "LED on before shutdown");
    fake::clearTrace ();
    led.shutdown     ();
    require          (!led.initialized (), "LED shutdown state");
    require          (!led.active (), "LED shutdown inactive");
    require          (fake::trace ().size () == 2, "LED shutdown trace");
    requireOperation (0, fake::OperationKind::DigitalWrite, 13, LOW);
    requireOperation (1, fake::OperationKind::PinMode, 13, INPUT);

    fake::clearTrace ();
    led.shutdown     ();
    require          (fake::trace ().empty (), "repeated LED shutdown touched hardware");
}

void testActiveLowPolarity ()
{
    fake::reset ();
    adk::ResourceRegistry resources;
    adk::MonoLed          led (resources, 9, false);

    require          (!led.activeHigh (), "active-low polarity");
    require          (led.initialize ().ok (), "active-low initialization");
    requireOperation (0, fake::OperationKind::DigitalWrite, 9, HIGH);
    requireOperation (1, fake::OperationKind::PinMode, 9, OUTPUT);

    fake::clearTrace ();
    require          (led.on ().ok (), "active-low on");
    requireOperation (0, fake::OperationKind::DigitalWrite, 9, LOW);
    require          (led.off ().ok (), "active-low off");
    requireOperation (1, fake::OperationKind::DigitalWrite, 9, HIGH);
}

void testClaimsAndErrors ()
{
    fake::reset ();
    adk::ResourceRegistry resources;
    adk::MonoLed          owner   (resources, 8);
    adk::MonoLed          blocked (resources, 8);

    require          (owner.initialize ().ok (), "LED owner initialization");
    fake::clearTrace ();
    require          (blocked.initialize ().error () == adk::StatusCode::ResourceBusy, "LED conflict status");
    require          (!blocked.initialized (), "blocked LED remains uninitialized");
    require          (!blocked.active (), "blocked LED remains inactive");
    require          (fake::trace ().empty (), "LED conflict touched hardware");

    owner.shutdown   ();
    fake::clearTrace ();
    require          (blocked.initialize ().ok (), "LED claim reuse");

    fake::reset ();
    adk::ResourceRegistry invalidResources;
    adk::MonoLed          invalid (invalidResources, NUM_DIGITAL_PINS);
    require                       (invalid.initialize ().error () == adk::StatusCode::InvalidPin, "invalid LED pin");
    require                       (!invalid.initialized (), "invalid LED remains uninitialized");
    require                       (fake::trace ().empty (), "invalid LED touched hardware");
}

void testDestruction ()
{
    fake::reset ();
    adk::ResourceRegistry resources;

    {
        adk::MonoLed led (resources, 7);

        require          (led.initialize ().ok (), "scoped LED initialization");
        require          (led.on ().ok (), "scoped LED on");
        fake::clearTrace ();
    }

    require          (fake::trace ().size () == 2, "LED destruction trace");
    requireOperation (0, fake::OperationKind::DigitalWrite, 7, LOW);
    requireOperation (1, fake::OperationKind::PinMode, 7, INPUT);

    fake::clearTrace         ();
    adk::MonoLed replacement (resources, 7);
    require                  (replacement.initialize ().ok (),
             "LED destruction released claim");
}
} // namespace

int main ()
{
    static_assert (!std::is_copy_constructible<adk::MonoLed>::value, "");
    static_assert (!std::is_move_constructible<adk::MonoLed>::value, "");

    testActiveHighLifecycle ();
    testActiveLowPolarity   ();
    testClaimsAndErrors     ();
    testDestruction         ();

    std::cout << "All ADK MonoLed tests passed.\n";
}
