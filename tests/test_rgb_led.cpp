#include <digital_output.h>
#include <rgb_led.h>

#include <Arduino.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {
    namespace fake = adk::test::arduino;

    constexpr adk::RgbLedChannel redChannel   = {6,  220};
    constexpr adk::RgbLedChannel greenChannel = {5,  330};
    constexpr adk::RgbLedChannel blueChannel  = {44, 470};

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    void requireOperation (
        std::size_t        index,
        fake::OperationKind kind,
        uint8_t            pin,
        int                value)
    {
        const auto& trace = fake::trace ();

        require (index < trace.size (), "missing operation");
        require (trace[index].kind == kind, "wrong operation kind");
        require (trace[index].pin == pin, "wrong operation pin");
        require (trace[index].value == value, "wrong operation value");
    }

    void requireOffTrace (std::size_t start)
    {
        requireOperation (start + 0, fake::OperationKind::AnalogWrite, 44, 0);
        requireOperation (start + 1, fake::OperationKind::PinMode,     44, INPUT);
        requireOperation (start + 2, fake::OperationKind::AnalogWrite, 5,  0);
        requireOperation (start + 3, fake::OperationKind::PinMode,     5,  INPUT);
        requireOperation (start + 4, fake::OperationKind::AnalogWrite, 6,  0);
        requireOperation (start + 5, fake::OperationKind::PinMode,     6,  INPUT);
    }

    void testColorValue ()
    {
        const adk::Rgb black;
        const adk::Rgb color (1, 127, 255);

        require (black == adk::Rgb (0, 0, 0), "default color is black");
        require (color.red () == 1, "red channel value");
        require (color.green () == 127, "green channel value");
        require (color.blue () == 255, "blue channel value");
        require (color == adk::Rgb (1, 127, 255), "color equality");
        require (color != black, "color inequality");
    }

    void testChannelsAndCommonCathode ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;
        adk::RgbLed           led   (
            resources, redChannel, greenChannel, blueChannel);

        require (!led.initialized (), "RGB starts stopped");
        require (led.color () == adk::Rgb (), "RGB starts off");
        require (led.redChannel ().pin == 6, "red pin");
        require (led.redChannel ().resistorOhms == 220, "red resistor");
        require (led.greenChannel ().pin == 5, "green pin");
        require (led.greenChannel ().resistorOhms == 330, "green resistor");
        require (led.blueChannel ().pin == 44, "blue pin");
        require (led.blueChannel ().resistorOhms == 470, "blue resistor");
        require (led.set (adk::Rgb (1, 2, 3)) == adk::Status::NotInitialized,
                 "stopped RGB rejects color");
        require (led.off () == adk::Status::NotInitialized,
                 "stopped RGB rejects off");
        require (fake::trace ().empty (), "stopped RGB touches no hardware");

        require (led.initialize () == adk::Status::Ok, "RGB initializes");
        require (led.initialized (), "RGB initialized state");
        require (fake::trace ().size () == 3, "RGB initialization trace");

        requireOperation (0, fake::OperationKind::AnalogWrite, 6,  0);
        requireOperation (1, fake::OperationKind::AnalogWrite, 5,  0);
        requireOperation (2, fake::OperationKind::AnalogWrite, 44, 0);

        fake::clearTrace ();

        require (led.set (adk::Rgb (11, 129, 250)) == adk::Status::Ok,
                 "RGB accepts color");
        require (led.color () == adk::Rgb (11, 129, 250), "RGB caches color");
        require (fake::trace ().size () == 3, "RGB color trace");

        requireOperation (0, fake::OperationKind::AnalogWrite, 6,  11);
        requireOperation (1, fake::OperationKind::AnalogWrite, 5,  129);
        requireOperation (2, fake::OperationKind::AnalogWrite, 44, 250);

        fake::clearTrace ();

        require (led.off () == adk::Status::Ok, "RGB turns off");
        require (led.color () == adk::Rgb (), "off clears cached color");

        requireOperation (0, fake::OperationKind::AnalogWrite, 6,  0);
        requireOperation (1, fake::OperationKind::AnalogWrite, 5,  0);
        requireOperation (2, fake::OperationKind::AnalogWrite, 44, 0);
    }

    void testInitializationIsIdempotent ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;
        adk::RgbLed           led   (
            resources, redChannel, greenChannel, blueChannel);

        require (led.initialize () == adk::Status::Ok, "first RGB initialization");

        fake::clearTrace ();

        require (led.initialize () == adk::Status::Ok, "repeated RGB initialization");
        require (fake::trace ().empty (), "repeated initialize is inert");
    }

    void testDescriptorValidation ()
    {
        const adk::RgbLedChannel invalidChannels[][3] = {
            {{6, 0},   greenChannel, blueChannel},
            {redChannel, {5, 0},     blueChannel},
            {redChannel, greenChannel, {44, 0}}
        };

        for (const auto& channels : invalidChannels)
        {
            fake::reset ();

            adk::ResourceRegistry resources;
            adk::RgbLed           led   (
                resources, channels[0], channels[1], channels[2]);

            require (led.initialize () == adk::Status::InvalidArgument,
                     "zero-ohm channel rejected");
            require (!led.initialized (), "invalid RGB stays stopped");
            require (fake::trace ().empty (), "invalid descriptor touches no hardware");
        }
    }

    void testFirstChannelConflict ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;
        adk::DigitalOutput    owner (resources, 6);
        adk::RgbLed           led   (
            resources, redChannel, greenChannel, blueChannel);

        require (owner.initialize () == adk::Status::Ok, "red conflict owner");

        fake::clearTrace ();

        require (led.initialize () == adk::Status::ResourceBusy,
                 "red conflict status");
        require (!led.initialized (), "red conflict leaves RGB stopped");
        require (fake::trace ().empty (), "red conflict has no partial writes");
    }

    void testSecondChannelRollback ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;
        adk::DigitalOutput    owner (resources, 5);
        adk::RgbLed           led   (
            resources, redChannel, greenChannel, blueChannel);

        require (owner.initialize () == adk::Status::Ok, "green conflict owner");

        fake::clearTrace ();

        require (led.initialize () == adk::Status::ResourceBusy,
                 "green conflict status");
        require (!led.initialized (), "green conflict leaves RGB stopped");
        require (led.color () == adk::Rgb (), "green rollback preserves off");
        require (fake::trace ().size () == 3, "green rollback trace");

        requireOperation (0, fake::OperationKind::AnalogWrite, 6, 0);
        requireOperation (1, fake::OperationKind::AnalogWrite, 6, 0);
        requireOperation (2, fake::OperationKind::PinMode,     6, INPUT);

        adk::PwmOutput reuse (resources, 6);

        fake::clearTrace ();

        require (reuse.initialize () == adk::Status::Ok,
                 "green rollback releases red claim");
    }

    void testThirdChannelRollback ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;
        adk::DigitalOutput    owner (resources, 44);
        adk::RgbLed           led   (
            resources, redChannel, greenChannel, blueChannel);

        require (owner.initialize () == adk::Status::Ok, "blue conflict owner");

        fake::clearTrace ();

        require (led.initialize () == adk::Status::ResourceBusy,
                 "blue conflict status");
        require (!led.initialized (), "blue conflict leaves RGB stopped");
        require (fake::trace ().size () == 6, "blue rollback trace");

        requireOperation (0, fake::OperationKind::AnalogWrite, 6, 0);
        requireOperation (1, fake::OperationKind::AnalogWrite, 5, 0);
        requireOperation (2, fake::OperationKind::AnalogWrite, 5, 0);
        requireOperation (3, fake::OperationKind::PinMode,     5, INPUT);
        requireOperation (4, fake::OperationKind::AnalogWrite, 6, 0);
        requireOperation (5, fake::OperationKind::PinMode,     6, INPUT);

        adk::PwmOutput redReuse   (resources, 6);
        adk::PwmOutput greenReuse (resources, 5);

        fake::clearTrace ();

        require (redReuse.initialize () == adk::Status::Ok,
                 "blue rollback releases red claim");
        require (greenReuse.initialize () == adk::Status::Ok,
                 "blue rollback releases green claim");
    }

    void testShutdownAndDestruction ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;

        {
            adk::RgbLed led (resources, redChannel, greenChannel, blueChannel);

            require (led.initialize () == adk::Status::Ok, "scoped RGB initialization");
            require (led.set (adk::Rgb (40, 80, 120)) == adk::Status::Ok,
                     "scoped RGB color");
            fake::clearTrace ();
        }

        require (fake::trace ().size () == 6, "RGB destruction trace");

        requireOffTrace (0);

        require (fake::analogOutput (6) == 0, "destructor turns red off");
        require (fake::analogOutput (5) == 0, "destructor turns green off");
        require (fake::analogOutput (44) == 0, "destructor turns blue off");
        require (fake::mode (6) == INPUT, "destructor floats red");
        require (fake::mode (5) == INPUT, "destructor floats green");
        require (fake::mode (44) == INPUT, "destructor floats blue");

        fake::clearTrace ();

        adk::RgbLed replacement (
            resources, redChannel, greenChannel, blueChannel);

        require (replacement.initialize () == adk::Status::Ok,
                 "destruction releases all claims");

        fake::clearTrace ();

        replacement.shutdown ();

        require (fake::trace ().size () == 6, "explicit shutdown trace");

        requireOffTrace (0);

        fake::clearTrace ();

        replacement.shutdown ();

        require (fake::trace ().empty (), "repeated shutdown is inert");
    }
}

int main ()
{
    static_assert (std::is_nothrow_constructible<
                       adk::Rgb,
                       uint8_t,
                       uint8_t,
                       uint8_t>::value,
                   "");
    static_assert (!std::is_copy_constructible<adk::RgbLed>::value, "");
    static_assert (!std::is_copy_assignable<adk::RgbLed>::value, "");
    static_assert (!std::is_move_constructible<adk::RgbLed>::value, "");
    static_assert (!std::is_move_assignable<adk::RgbLed>::value, "");
    static_assert (!std::is_copy_constructible<adk::PwmOutput>::value, "");
    static_assert (!std::is_move_constructible<adk::PwmOutput>::value, "");

    testColorValue                 ();
    testChannelsAndCommonCathode   ();
    testInitializationIsIdempotent ();
    testDescriptorValidation       ();
    testFirstChannelConflict       ();
    testSecondChannelRollback      ();
    testThirdChannelRollback       ();
    testShutdownAndDestruction     ();

    std::cout << "All ADK RGB LED tests passed.\n";
}
