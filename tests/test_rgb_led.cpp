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

    void testEveryColorBoundary ()
    {
        const uint8_t values[] = {0, 1, 127, 254, 255};

        for (const auto red : values)
        {
            for (const auto green : values)
            {
                for (const auto blue : values)
                {
                    const adk::Rgb color (red, green, blue);

                    require (color.red () == red, "boundary red value");
                    require (color.green () == green, "boundary green value");
                    require (color.blue () == blue, "boundary blue value");
                    require (color == adk::Rgb (red, green, blue),
                             "boundary color equality");
                }
            }
        }
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
        require (led.set (adk::Rgb (1, 2, 3)).error () == adk::StatusCode::NotInitialized,
                 "stopped RGB rejects color");
        require (led.off ().error () == adk::StatusCode::NotInitialized,
                 "stopped RGB rejects off");
        require (fake::trace ().empty (), "stopped RGB touches no hardware");

        require (led.initialize ().ok (), "RGB initializes");
        require (led.initialized (), "RGB initialized state");
        require (fake::trace ().size () == 3, "RGB initialization trace");

        requireOperation (0, fake::OperationKind::AnalogWrite, 6,  0);
        requireOperation (1, fake::OperationKind::AnalogWrite, 5,  0);
        requireOperation (2, fake::OperationKind::AnalogWrite, 44, 0);

        fake::clearTrace ();

        require (led.set (adk::Rgb (11, 129, 250)).ok (),
                 "RGB accepts color");
        require (led.color () == adk::Rgb (11, 129, 250), "RGB caches color");
        require (fake::trace ().size () == 3, "RGB color trace");

        requireOperation (0, fake::OperationKind::AnalogWrite, 6,  11);
        requireOperation (1, fake::OperationKind::AnalogWrite, 5,  129);
        requireOperation (2, fake::OperationKind::AnalogWrite, 44, 250);

        fake::clearTrace ();

        require (led.off ().ok (), "RGB turns off");
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

        require (led.initialize ().ok (), "first RGB initialization");

        fake::clearTrace ();

        require (led.initialize ().ok (), "repeated RGB initialization");
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

            require (led.initialize ().error () == adk::StatusCode::InvalidArgument,
                     "zero-ohm channel rejected");
            require (!led.initialized (), "invalid RGB stays stopped");
            require (fake::trace ().empty (), "invalid descriptor touches no hardware");
        }
    }

    void testPinValidationPrecedesHardware ()
    {
        const adk::RgbLedChannel invalidPins[][3] = {
            {{70, 220}, greenChannel, blueChannel},
            {redChannel, {70, 330}, blueChannel},
            {redChannel, greenChannel, {70, 470}}
        };
        const adk::RgbLedChannel unsupportedPins[][3] = {
            {{14, 220}, greenChannel, blueChannel},
            {redChannel, {14, 330}, blueChannel},
            {redChannel, greenChannel, {14, 470}}
        };

        for (const auto& channels : invalidPins)
        {
            fake::reset ();

            adk::ResourceRegistry resources;
            adk::RgbLed           led   (
                resources, channels[0], channels[1], channels[2]);

            require (led.initialize ().error () == adk::StatusCode::InvalidPin,
                     "invalid channel pin rejected");
            require (!led.initialized (), "invalid pin leaves RGB stopped");
            require (fake::trace ().empty (),
                     "all invalid pins checked before hardware");
        }

        for (const auto& channels : unsupportedPins)
        {
            fake::reset ();

            adk::ResourceRegistry resources;
            adk::RgbLed           led   (
                resources, channels[0], channels[1], channels[2]);

            require (led.initialize ().error () == adk::StatusCode::Unsupported,
                     "unsupported channel pin rejected");
            require (!led.initialized (), "unsupported pin leaves RGB stopped");
            require (fake::trace ().empty (),
                     "all capabilities checked before hardware");
        }
    }

    void testDuplicatePinsAreInvalid ()
    {
        const adk::RgbLedChannel duplicatePins[][3] = {
            {redChannel, {6, 330}, blueChannel},
            {redChannel, greenChannel, {6, 470}},
            {redChannel, greenChannel, {5, 470}}
        };

        for (const auto& channels : duplicatePins)
        {
            fake::reset ();

            adk::ResourceRegistry resources;
            adk::RgbLed           led   (
                resources, channels[0], channels[1], channels[2]);

            require (led.initialize ().error () == adk::StatusCode::InvalidArgument,
                     "duplicate RGB pins rejected");
            require (!led.initialized (), "duplicate pins leave RGB stopped");
            require (fake::trace ().empty (),
                     "duplicate pins touch no hardware");
        }
    }

    void testFirstChannelConflict ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;
        adk::DigitalOutput    owner (resources, 6);
        adk::RgbLed           led   (
            resources, redChannel, greenChannel, blueChannel);

        require (owner.initialize ().ok (), "red conflict owner");

        fake::clearTrace ();

        require (led.initialize ().error () == adk::StatusCode::ResourceBusy,
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

        require (owner.initialize ().ok (), "green conflict owner");

        fake::clearTrace ();

        require (led.initialize ().error () == adk::StatusCode::ResourceBusy,
                 "green conflict status");
        require (!led.initialized (), "green conflict leaves RGB stopped");
        require (led.color () == adk::Rgb (), "green rollback preserves off");
        require (fake::trace ().size () == 3, "green rollback trace");

        requireOperation (0, fake::OperationKind::AnalogWrite, 6, 0);
        requireOperation (1, fake::OperationKind::AnalogWrite, 6, 0);
        requireOperation (2, fake::OperationKind::PinMode,     6, INPUT);

        adk::PwmOutput reuse (resources, 6);

        fake::clearTrace ();

        require (reuse.initialize ().ok (),
                 "green rollback releases red claim");
    }

    void testThirdChannelRollback ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;
        adk::DigitalOutput    owner (resources, 44);
        adk::RgbLed           led   (
            resources, redChannel, greenChannel, blueChannel);

        require (owner.initialize ().ok (), "blue conflict owner");

        fake::clearTrace ();

        require (led.initialize ().error () == adk::StatusCode::ResourceBusy,
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

        require (redReuse.initialize ().ok (),
                 "blue rollback releases red claim");
        require (greenReuse.initialize ().ok (),
                 "blue rollback releases green claim");
    }

    void testShutdownAndDestruction ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;

        {
            adk::RgbLed led (resources, redChannel, greenChannel, blueChannel);

            require (led.initialize ().ok (), "scoped RGB initialization");
            require (led.set (adk::Rgb (40, 80, 120)).ok (),
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

        require (replacement.initialize ().ok (),
                 "destruction releases all claims");

        fake::clearTrace ();

        replacement.shutdown ();

        require (fake::trace ().size () == 6, "explicit shutdown trace");

        requireOffTrace (0);

        fake::clearTrace ();

        replacement.shutdown ();

        require (fake::trace ().empty (), "repeated shutdown is inert");

        require (replacement.initialize ().ok (),
                 "RGB restarts after shutdown");
        require (replacement.color () == adk::Rgb (),
                 "RGB restart begins off");
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

    testColorValue                    ();
    testEveryColorBoundary            ();
    testChannelsAndCommonCathode      ();
    testInitializationIsIdempotent    ();
    testDescriptorValidation          ();
    testPinValidationPrecedesHardware ();
    testDuplicatePinsAreInvalid       ();
    testFirstChannelConflict          ();
    testSecondChannelRollback         ();
    testThirdChannelRollback          ();
    testShutdownAndDestruction        ();

    std::cout << "All ADK RGB LED tests passed.\n";
}
