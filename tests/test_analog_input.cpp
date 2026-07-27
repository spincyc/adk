#include <analog_input.h>
#include <digital_output.h>

#include <Arduino.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <vector>

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

void requireOperation (
    std::size_t         index,
    fake::OperationKind kind,
    uint8_t             pin,
    int                 value)
{
    const auto& trace = fake::trace ();

    require (index < trace.size (), "missing operation");
    require (trace[index].kind == kind, "wrong operation kind");
    require (trace[index].pin == pin, "wrong operation pin");
    require (trace[index].value == value, "wrong operation value");
}

void requireSameTrace (
    const std::vector<fake::Operation>& left,
    const std::vector<fake::Operation>& right)
{
    require (left.size () == right.size (), "trace size differs");

    for (std::size_t index = 0; index < left.size (); ++index)
    {
        require (left[index].kind == right[index].kind, "trace kind differs");
        require (left[index].pin == right[index].pin, "trace pin differs");
        require (left[index].value == right[index].value, "trace value differs");
        require (left[index].timeUs == right[index].timeUs, "trace time differs");
    }
}

void testLifecycleAndSampling ()
{
    fake::reset ();
    adk::ResourceRegistry resources;
    adk::AnalogInput      input (resources, 54);

    require      (input.pin () == 54, "input pin");
    require      (input.read () == 0, "input initial reading");
    require      (input.sample () == 0, "inactive sample");
    require      (!input.initialized (), "input starts inactive");
    input.update ();
    require      (fake::trace ().empty (), "inactive update touched hardware");

    fake::setAnalogInput (54, 317);
    require              (input.initialize ().ok (), "input initialization");
    require              (input.initialized (), "input active");
    require              (input.read () == 317, "initial sample cached");
    require              (fake::trace ().size () == 2, "initialization trace");
    requireOperation     (0, fake::OperationKind::PinMode, 54, INPUT);
    requireOperation     (1, fake::OperationKind::AnalogRead, 54, 317);

    fake::clearTrace ();
    require          (input.initialize ().ok (), "repeated initialization");
    require          (fake::trace ().empty (), "repeated initialization touched hardware");

    fake::setAnalogInput (54, 811);
    require              (input.sample () == 811, "direct sample");
    require              (input.read () == 317, "direct sample preserves cache");
    input.update         ();
    require              (input.read () == 811, "update caches sample");
    require              (fake::trace ().size () == 2, "sampling trace");
    requireOperation     (0, fake::OperationKind::AnalogRead, 54, 811);
    requireOperation     (1, fake::OperationKind::AnalogRead, 54, 811);

    fake::clearTrace ();
    input.shutdown   ();
    require          (!input.initialized (), "shutdown state");
    require          (input.read () == 811, "shutdown retains last reading");
    require          (input.sample () == 811, "inactive sample retains reading");
    require          (fake::trace ().size () == 1, "shutdown trace");
    requireOperation (0, fake::OperationKind::PinMode, 54, INPUT);

    fake::clearTrace ();
    input.shutdown   ();
    require          (fake::trace ().empty (), "repeated shutdown touched hardware");
}

void testReadingBounds ()
{
    fake::reset ();
    adk::ResourceRegistry resources;
    adk::AnalogInput      input (resources, 69);

    fake::setAnalogInput (69, 0);
    require              (input.initialize ().ok (), "minimum initialization");
    require              (input.read () == 0, "minimum reading");

    fake::setAnalogInput (69, 1023);
    input.update         ();
    require              (input.read () == adk::AnalogInput::maximumReading,
             "maximum reading");

    fake::setAnalogInput (69, -1);
    input.update         ();
    require              (input.read () == 0, "negative fake reading clamps low");

    fake::setAnalogInput (69, 1024);
    input.update         ();
    require              (input.read () == adk::AnalogInput::maximumReading,
             "oversized fake reading clamps high");
}

void testMegaAnalogPinMap ()
{
    for (uint8_t pin = 54; pin <= 69; ++pin)
    {
        fake::reset ();
        adk::ResourceRegistry resources;
        adk::AnalogInput      input (resources, pin);

        fake::setAnalogInput (pin, pin);
        require              (input.initialize ().ok (),
                 "Mega analog pin accepted");
        require              (input.read () == pin, "Mega analog pin sampled");
    }
}

void testErrorsAndClaimReuse ()
{
    fake::reset ();
    adk::ResourceRegistry resources;
    adk::AnalogInput      owner   (resources, 55);
    adk::AnalogInput      blocked (resources, 55);

    require          (owner.initialize ().ok (), "owner initialization");
    fake::clearTrace ();
    require          (blocked.initialize ().error () == adk::StatusCode::ResourceBusy,
             "claim conflict");
    require          (fake::trace ().empty (), "claim conflict touched hardware");

    owner.shutdown   ();
    fake::clearTrace ();
    require          (blocked.initialize ().ok (), "claim reuse");

    fake::reset ();
    adk::ResourceRegistry unsupportedResources;
    adk::AnalogInput      unsupported (unsupportedResources, 53);
    require                           (unsupported.initialize ().error () == adk::StatusCode::Unsupported,
             "digital-only pin rejected");
    require                           (fake::trace ().empty (), "unsupported pin touched hardware");

    adk::ResourceRegistry invalidResources;
    adk::AnalogInput      invalid (invalidResources, 70);
    require                       (invalid.initialize ().error () == adk::StatusCode::InvalidPin,
             "invalid pin rejected");
    require                       (fake::trace ().empty (), "invalid pin touched hardware");
}

void testDestructionReleasesClaim ()
{
    fake::reset ();
    adk::ResourceRegistry resources;

    {
        fake::setAnalogInput     (56, 512);
        adk::AnalogInput   input (resources, 56);

        require                  (input.initialize ().ok (),
                 "lifetime initialization");
        fake::clearTrace         ();
    }

    require          (fake::trace ().size () == 1, "destruction trace");
    requireOperation (0, fake::OperationKind::PinMode, 56, INPUT);

    fake::clearTrace               ();
    adk::DigitalOutput replacement (resources, 56);
    require                        (replacement.initialize ().ok (),
             "destruction released claim");
}

std::vector<fake::Operation> analogTrace ()
{
    fake::reset          ();
    fake::setTimeUs      (2500);
    fake::setAnalogInput (57, 123);
    adk::ResourceRegistry resources;
    adk::AnalogInput      input (resources, 57);

    require              (input.initialize ().ok (), "trace initialization");
    fake::setAnalogInput (57, 987);
    input.update         ();
    input.shutdown       ();
    return fake::trace   ();
}

void testTraceDeterminism ()
{
    const auto first  = analogTrace ();
    const auto second = analogTrace ();

    requireSameTrace (first, second);
}
}

int main ()
{
    static_assert (!std::is_copy_constructible<adk::AnalogInput>::value, "");
    static_assert (!std::is_copy_assignable<adk::AnalogInput>::value, "");
    static_assert (!std::is_move_constructible<adk::AnalogInput>::value, "");
    static_assert (!std::is_move_assignable<adk::AnalogInput>::value, "");

    testLifecycleAndSampling     ();
    testReadingBounds            ();
    testMegaAnalogPinMap         ();
    testErrorsAndClaimReuse      ();
    testDestructionReleasesClaim ();
    testTraceDeterminism         ();

    std::cout << "All ADK analog-input tests passed.\n";
}
