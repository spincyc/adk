#include <adk.h>

#include <Arduino.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <vector>

namespace {
    struct Call
    {
        enum struct Kind
        {
            PinMode,
            AnalogRead,
            AnalogWrite,
            DigitalRead,
            DigitalWrite
        };

        Kind kind;
        int  pin;
        int  value;
    };

    std::vector<Call> calls;
    int               analogValue  = 0;
    int               digitalValue = LOW;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    void requireCall (
        std::size_t index,
        Call::Kind  kind,
        int         pin,
        int         value)
    {
        require (index < calls.size (), "missing fake Arduino call");
        require (calls[index].kind  == kind,  "unexpected call kind");
        require (calls[index].pin   == pin,   "unexpected call pin");
        require (calls[index].value == value, "unexpected call value");
    }

    void testColor ()
    {
        const adk::color::Rgb custom (1, 2, 3);

        require (custom.red ()   == 1, "red channel");
        require (custom.green () == 2, "green channel");
        require (custom.blue ()  == 3, "blue channel");
        require (custom == adk::color::Rgb (1, 2, 3), "color equality");
        require (custom != adk::color::off (), "color inequality");
    }

    void testPinsAndLeds ()
    {
        calls.clear ();
        {
            adk::led::Mono mono (9);

            adk::initialize ();
            mono.on         ();
            mono.off        ();
        }

        require     (calls.size () == 3, "mono call count");
        requireCall (0, Call::Kind::PinMode,      9, OUTPUT);
        requireCall (1, Call::Kind::DigitalWrite, 9, HIGH);
        requireCall (2, Call::Kind::DigitalWrite, 9, LOW);
    }

    void testRgbLifetimeRegression ()
    {
        calls.clear ();
        {
            adk::led::Rgb rgb (6, 5, 3);

            adk::initialize ();
            rgb.on          (adk::color::Rgb (10, 20, 30));
            rgb.off         ();
        }

        require     (calls.size () == 9, "RGB call count");
        requireCall (0, Call::Kind::PinMode,     6, OUTPUT);
        requireCall (1, Call::Kind::PinMode,     5, OUTPUT);
        requireCall (2, Call::Kind::PinMode,     3, OUTPUT);
        requireCall (3, Call::Kind::AnalogWrite, 6, 10);
        requireCall (4, Call::Kind::AnalogWrite, 5, 20);
        requireCall (5, Call::Kind::AnalogWrite, 3, 30);
        requireCall (6, Call::Kind::AnalogWrite, 6, 0);
        requireCall (7, Call::Kind::AnalogWrite, 5, 0);
        requireCall (8, Call::Kind::AnalogWrite, 3, 0);

        calls.clear     ();
        adk::initialize ();
        require         (calls.empty (), "destroyed RGB remains registered");
    }
}

void pinMode (uint8_t pin, uint8_t mode)
{
    calls.push_back ({Call::Kind::PinMode, pin, mode});
}

int analogRead (uint8_t pin)
{
    calls.push_back ({Call::Kind::AnalogRead, pin, analogValue});
    return analogValue;
}

void analogWrite (uint8_t pin, int value)
{
    calls.push_back ({Call::Kind::AnalogWrite, pin, value});
}

int digitalRead (uint8_t pin)
{
    calls.push_back ({Call::Kind::DigitalRead, pin, digitalValue});
    return digitalValue;
}

void digitalWrite (uint8_t pin, uint8_t value)
{
    calls.push_back ({Call::Kind::DigitalWrite, pin, value});
}

void delay (unsigned long)
{
}

int main ()
{
    static_assert (!std::is_copy_constructible<adk::Object>::value, "");
    static_assert (!std::is_move_constructible<adk::Object>::value, "");

    testColor                 ();
    testPinsAndLeds           ();
    testRgbLifetimeRegression ();

    std::cout << "All ADK host tests passed.\n";
}
