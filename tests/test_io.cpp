#include <digital_input.h>
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

void requireOperation (std::size_t index, fake::OperationKind kind, uint8_t pin,
                       int value)
{
    const auto& trace = fake::trace ();

    require (index < trace.size (), "missing operation");
    require (trace[index].kind == kind, "wrong operation kind");
    require (trace[index].pin == pin, "wrong operation pin");
    require (trace[index].value == value, "wrong operation value");
}

void requireSameTrace (const std::vector<fake::Operation>& left,
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

void testOutputLifecycle ()
{
    fake::reset ();
    adk::ResourceRegistry resources;
    adk::DigitalOutput    output (resources, 13);

    require (output.pin () == 13, "output pin");
    require (output.level () == adk::Level::Low, "output initial level");
    require (!output.initialized (), "output starts inactive");
    require (output.write (adk::Level::High) == adk::Status::NotInitialized,
             "output rejects write before initialization");
    require (fake::trace ().empty (), "inactive output touched hardware");

    require          (output.initialize () == adk::Status::Ok, "output initialization");
    require          (output.initialized (), "output active");
    require          (fake::trace ().size () == 2, "output initialization trace");
    requireOperation (0, fake::OperationKind::DigitalWrite, 13, LOW);
    requireOperation (1, fake::OperationKind::PinMode, 13, OUTPUT);

    fake::clearTrace ();
    require          (output.initialize () == adk::Status::Ok, "output repeated initialization");
    require          (fake::trace ().empty (),
             "repeated output initialization touched hardware");

    require          (output.write (adk::Level::High) == adk::Status::Ok, "output high write");
    require          (output.level () == adk::Level::High, "output high state");
    requireOperation (0, fake::OperationKind::DigitalWrite, 13, HIGH);

    fake::clearTrace ();
    output.shutdown  ();
    require          (!output.initialized (), "output shutdown state");
    require          (fake::trace ().size () == 1, "output shutdown trace");
    requireOperation (0, fake::OperationKind::PinMode, 13, INPUT);

    fake::clearTrace ();
    output.shutdown  ();
    require          (fake::trace ().empty (), "repeated output shutdown touched hardware");

    require          (output.initialize () == adk::Status::Ok, "output restart");
    require          (output.level () == adk::Level::Low, "output restart level");
    requireOperation (0, fake::OperationKind::DigitalWrite, 13, LOW);
    requireOperation (1, fake::OperationKind::PinMode, 13, OUTPUT);
}

void testOutputInitialHighAndDestruction ()
{
    fake::reset ();
    adk::ResourceRegistry resources;

    {
        adk::DigitalOutput output (resources, 8, adk::Level::High);

        require          (output.initialize () == adk::Status::Ok, "high output initialization");
        requireOperation (0, fake::OperationKind::DigitalWrite, 8, HIGH);
        requireOperation (1, fake::OperationKind::PinMode, 8, OUTPUT);
        fake::clearTrace ();
    }

    require          (fake::trace ().size () == 1, "output destruction trace");
    requireOperation (0, fake::OperationKind::PinMode, 8, INPUT);

    fake::clearTrace               ();
    adk::DigitalOutput replacement (resources, 8);
    require                        (replacement.initialize () == adk::Status::Ok,
             "output destruction released claim");
}

void testOutputErrors ()
{
    fake::reset ();
    adk::ResourceRegistry resources;
    adk::DigitalOutput    owner   (resources, 7);
    adk::DigitalOutput    blocked (resources, 7);

    require          (owner.initialize () == adk::Status::Ok, "output owner initialization");
    fake::clearTrace ();
    require          (blocked.initialize () == adk::Status::ResourceBusy,
             "output conflict status");
    require (fake::trace ().empty (), "output conflict touched hardware");

    owner.shutdown   ();
    fake::clearTrace ();
    require          (blocked.initialize () == adk::Status::Ok, "output claim reuse");

    fake::reset ();
    adk::ResourceRegistry invalidResources;
    adk::DigitalOutput    invalid (invalidResources, NUM_DIGITAL_PINS);
    require                       (invalid.initialize () == adk::Status::InvalidPin, "invalid output pin");
    require                       (fake::trace ().empty (), "invalid output touched hardware");
}

void testInputModesAndSampling ()
{
    fake::reset ();
    adk::ResourceRegistry resources;
    fake::setDigitalInput   (4, HIGH);
    adk::DigitalInput input (resources, 4, adk::Pull::None);

    require      (input.pin () == 4, "input pin");
    require      (input.pull () == adk::Pull::None, "input pull none");
    require      (input.read () == adk::Level::Low, "input cached initial level");
    require      (input.sample () == adk::Level::Low, "inactive input sample");
    input.update ();
    require      (fake::trace ().empty (), "inactive input touched hardware");

    require          (input.initialize () == adk::Status::Ok, "input initialization");
    require          (input.initialized (), "input active");
    require          (input.read () == adk::Level::High, "input initial sample");
    require          (fake::trace ().size () == 2, "input initialization trace");
    requireOperation (0, fake::OperationKind::PinMode, 4, INPUT);
    requireOperation (1, fake::OperationKind::DigitalRead, 4, HIGH);

    fake::clearTrace ();
    require          (input.initialize () == adk::Status::Ok, "input repeated initialization");
    require          (fake::trace ().empty (), "repeated input initialization touched hardware");

    fake::setDigitalInput (4, LOW);
    require               (input.sample () == adk::Level::Low, "input raw sample");
    require               (input.read () == adk::Level::High, "raw sample preserves cache");
    input.update          ();
    require               (input.read () == adk::Level::Low, "input cached update");
    require               (fake::trace ().size () == 2, "input sampling trace");
    requireOperation      (0, fake::OperationKind::DigitalRead, 4, LOW);
    requireOperation      (1, fake::OperationKind::DigitalRead, 4, LOW);

    fake::clearTrace ();
    input.shutdown   ();
    require          (!input.initialized (), "input shutdown state");
    require          (fake::trace ().size () == 1, "input shutdown trace");
    requireOperation (0, fake::OperationKind::PinMode, 4, INPUT);

    fake::clearTrace ();
    input.shutdown   ();
    require          (fake::trace ().empty (), "repeated input shutdown touched hardware");

    fake::reset ();
    adk::ResourceRegistry pullResources;
    fake::setDigitalInput    (5, HIGH);
    adk::DigitalInput pullUp (pullResources, 5, adk::Pull::Up);
    require                  (pullUp.initialize () == adk::Status::Ok, "pull-up initialization");
    requireOperation         (0, fake::OperationKind::PinMode, 5, INPUT_PULLUP);
    requireOperation         (1, fake::OperationKind::DigitalRead, 5, HIGH);
}

void testInputErrorsAndDestruction ()
{
    fake::reset ();
    adk::ResourceRegistry resources;
    adk::DigitalInput     owner   (resources, 6);
    adk::DigitalInput     blocked (resources, 6);

    require          (owner.initialize () == adk::Status::Ok, "input owner initialization");
    fake::clearTrace ();
    require          (blocked.initialize () == adk::Status::ResourceBusy,
             "input conflict status");
    require (fake::trace ().empty (), "input conflict touched hardware");

    owner.shutdown   ();
    fake::clearTrace ();
    require          (blocked.initialize () == adk::Status::Ok, "input claim reuse");

    fake::reset ();
    adk::ResourceRegistry invalidResources;
    adk::DigitalInput     invalid (invalidResources, NUM_DIGITAL_PINS);
    require                       (invalid.initialize () == adk::Status::InvalidPin, "invalid input pin");
    require                       (fake::trace ().empty (), "invalid input touched hardware");

    fake::reset ();
    adk::ResourceRegistry lifetimeResources;
    {
        adk::DigitalInput input (lifetimeResources, 3);
        require                 (input.initialize () == adk::Status::Ok,
                 "lifetime input initialization");
        fake::clearTrace ();
    }
    requireOperation               (0, fake::OperationKind::PinMode, 3, INPUT);
    fake::clearTrace               ();
    adk::DigitalOutput replacement (lifetimeResources, 3);
    require                        (replacement.initialize () == adk::Status::Ok,
             "input destruction released claim");
}

std::vector<fake::Operation> outputTrace ()
{
    fake::reset     ();
    fake::setTimeUs (1250);
    adk::ResourceRegistry resources;
    adk::DigitalOutput    output (resources, 12, adk::Level::High);

    require            (output.initialize () == adk::Status::Ok, "trace output initialization");
    require            (output.write (adk::Level::Low) == adk::Status::Ok, "trace output write");
    output.shutdown    ();
    return fake::trace ();
}

void testTraceDeterminism ()
{
    const auto first  = outputTrace ();
    const auto second = outputTrace ();

    requireSameTrace (first, second);
}
} // namespace

int main ()
{
    static_assert (!std::is_copy_constructible<adk::DigitalOutput>::value, "");
    static_assert (!std::is_move_constructible<adk::DigitalOutput>::value, "");
    static_assert (!std::is_copy_constructible<adk::DigitalInput>::value, "");
    static_assert (!std::is_move_constructible<adk::DigitalInput>::value, "");

    testOutputLifecycle                 ();
    testOutputInitialHighAndDestruction ();
    testOutputErrors                    ();
    testInputModesAndSampling           ();
    testInputErrorsAndDestruction       ();
    testTraceDeterminism                ();

    std::cout << "All ADK I/O tests passed.\n";
}
