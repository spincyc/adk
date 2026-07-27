#include <digital_output.h>
#include <shift_register.h>

#include <Arduino.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {
    namespace fake = adk::test::arduino;

    constexpr adk::ShiftRegisterPins registerPins = {22, 23, 24};

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

    void requireTransfer (uint8_t value)
    {
        require          (fake::trace ().size () == 27, "transfer operation count");
        requireOperation (0, fake::OperationKind::DigitalWrite, 24, LOW);

        std::size_t index = 1;

        for (uint8_t mask = 0x80U; mask != 0; mask >>= 1U)
        {
            const uint8_t bit = (value & mask) == 0 ? LOW : HIGH;

            requireOperation (index++, fake::OperationKind::DigitalWrite, 22, bit);
            requireOperation (index++, fake::OperationKind::DigitalWrite, 23, HIGH);
            requireOperation (index++, fake::OperationKind::DigitalWrite, 23, LOW);
        }

        requireOperation (index++, fake::OperationKind::DigitalWrite, 24, HIGH);
        requireOperation (index,   fake::OperationKind::DigitalWrite, 24, LOW);
    }

    void testLifecycleAndTransfer ()
    {
        fake::reset ();

        adk::ResourceRegistry     resources;
        adk::ShiftRegisterOutput output (resources, registerPins);

        require (!output.initialized (), "register starts stopped");
        require (output.value () == 0, "register starts at inactive value");
        require (output.inactiveValue () == 0, "default inactive value");
        require (output.pins ().data == 22, "data pin descriptor");
        require (output.pins ().clock == 23, "clock pin descriptor");
        require (output.pins ().latch == 24, "latch pin descriptor");
        require (output.show (0xA5U) == adk::Status::NotInitialized,
                 "stopped register rejects transfer");
        require (output.clear () == adk::Status::NotInitialized,
                 "stopped register rejects clear");
        require (fake::trace ().empty (), "stopped register touches no hardware");

        require (output.initialize () == adk::Status::Ok,
                 "register initializes");
        require (output.initialized (), "register initialized state");
        require (output.value () == 0, "initialization publishes inactive value");

        fake::clearTrace ();

        require (output.show (0xA5U) == adk::Status::Ok,
                 "register accepts byte");
        require         (output.value () == 0xA5U, "register caches published byte");
        requireTransfer (0xA5U);

        fake::clearTrace ();

        require         (output.clear () == adk::Status::Ok, "register clears");
        require         (output.value () == 0, "clear caches zero");
        requireTransfer (0);

        fake::clearTrace ();

        require (output.initialize () == adk::Status::Ok,
                 "repeated initialization succeeds");
        require (fake::trace ().empty (), "repeated initialization is inert");
    }

    void testInactivePatternAndShutdown ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;

        {
            adk::ShiftRegisterOutput output (resources, registerPins, 0xFFU);

            require (output.value () == 0xFFU, "custom inactive initial cache");
            require (output.inactiveValue () == 0xFFU,
                     "custom inactive descriptor");
            require (output.initialize () == adk::Status::Ok,
                     "custom inactive register initializes");

            fake::clearTrace ();
            require          (output.show (0x42U) == adk::Status::Ok,
                     "custom inactive register shows byte");
            fake::clearTrace ();
        }

        require (fake::trace ().size () == 30,
                 "destruction publishes inactive then floats pins");

        for (std::size_t index = 0; index < 27; ++index)
        {
            require (fake::trace ()[index].kind ==
                         fake::OperationKind::DigitalWrite,
                     "inactive transfer uses digital writes");
        }

        for (std::size_t index = 1; index < 25; index += 3)
        {
            requireOperation (
                index, fake::OperationKind::DigitalWrite, 22, HIGH);
        }

        requireOperation (27, fake::OperationKind::PinMode, 24, INPUT);
        requireOperation (28, fake::OperationKind::PinMode, 23, INPUT);
        requireOperation (29, fake::OperationKind::PinMode, 22, INPUT);
        require          (fake::mode (22) == INPUT, "data pin floats");
        require          (fake::mode (23) == INPUT, "clock pin floats");
        require          (fake::mode (24) == INPUT, "latch pin floats");

        fake::clearTrace ();

        adk::ShiftRegisterOutput replacement (resources, registerPins);

        require (replacement.initialize () == adk::Status::Ok,
                 "destruction releases all pins");

        fake::clearTrace      ();

        replacement.shutdown ();

        require (fake::trace ().size () == 30,
                 "explicit shutdown publishes inactive then floats");

        fake::clearTrace      ();

        replacement.shutdown ();

        require              (fake::trace ().empty (), "repeated shutdown is inert");
    }

    void testInvalidPins ()
    {
        const adk::ShiftRegisterPins invalidPins[] = {
            {NUM_DIGITAL_PINS, 23,                24},
            {22,               NUM_DIGITAL_PINS, 24},
            {22,               23,                NUM_DIGITAL_PINS},
            {22,               22,                24},
            {22,               23,                22},
            {22,               23,                23}
        };

        for (const auto& pins : invalidPins)
        {
            fake::reset ();

            adk::ResourceRegistry     resources;
            adk::ShiftRegisterOutput output (resources, pins);
            const bool               duplicate =
                pins.data == pins.clock || pins.data == pins.latch ||
                pins.clock == pins.latch;
            const adk::Status expected = duplicate
                                             ? adk::Status::InvalidArgument
                                             : adk::Status::InvalidPin;

            require (output.initialize () == expected,
                     "invalid pin descriptor rejected");
            require (!output.initialized (), "invalid register stays stopped");
            require (fake::trace ().empty (),
                     "invalid descriptor touches no hardware");
        }
    }

    void testClaimRollback ()
    {
        const uint8_t blockedPins[] = {22, 23, 24};

        for (uint8_t blockedPin : blockedPins)
        {
            fake::reset ();

            adk::ResourceRegistry     resources;
            adk::DigitalOutput        owner (resources, blockedPin);
            adk::ShiftRegisterOutput output (resources, registerPins);

            require (owner.initialize () == adk::Status::Ok,
                     "conflict owner initializes");
            fake::clearTrace ();

            require (output.initialize () == adk::Status::ResourceBusy,
                     "pin conflict reported");
            require (!output.initialized (), "conflict leaves register stopped");

            owner.shutdown   ();
            fake::clearTrace ();

            adk::DigitalOutput dataReuse  (resources, registerPins.data);
            adk::DigitalOutput clockReuse (resources, registerPins.clock);
            adk::DigitalOutput latchReuse (resources, registerPins.latch);

            require (dataReuse.initialize () == adk::Status::Ok,
                     "rollback releases data pin");
            require (clockReuse.initialize () == adk::Status::Ok,
                     "rollback releases clock pin");
            require (latchReuse.initialize () == adk::Status::Ok,
                     "rollback releases latch pin");
        }
    }
}

int main ()
{
    static_assert (!std::is_copy_constructible<
                       adk::ShiftRegisterOutput>::value,
                   "");
    static_assert (!std::is_copy_assignable<
                       adk::ShiftRegisterOutput>::value,
                   "");
    static_assert (!std::is_move_constructible<
                       adk::ShiftRegisterOutput>::value,
                   "");
    static_assert (!std::is_move_assignable<
                       adk::ShiftRegisterOutput>::value,
                   "");

    testLifecycleAndTransfer       ();
    testInactivePatternAndShutdown ();
    testInvalidPins                ();
    testClaimRollback              ();

    std::cout << "All ADK shift register tests passed.\n";
}
