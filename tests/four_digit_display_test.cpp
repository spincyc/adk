#include "check.h"

#include <Arduino.h>
#include <adk/four_digit_display.h>
#include <adk/segments.h>

namespace {

    const uint8_t Latch      = 32;
    const uint8_t Digits [4] = {22, 23, 24, 25};

    // The digits whose pins are low, such as "2", or "" when all are dark.
    std::string lit ()
    {
        std::string digits;

        for (uint8_t digit = 0; digit < 4; ++digit)
        {
            if (arduino::pin (Digits[digit]).output == LOW)
            {
                digits += static_cast<char> ('1' + digit);
            }
        }

        return digits;
    }

    // Update through one whole scan, 2 ms a digit, and collect the segments
    // each digit was lit with. A digit that was never lit reads as blank.
    std::string scan (adk::Millis& now)
    {
        std::string shown (4, '\0');

        for (uint8_t step = 0; step < 4; ++step)
        {
            adk::update (now);
            now += 2;

            for (uint8_t digit = 0; digit < 4; ++digit)
            {
                if (arduino::pin (Digits[digit]).output == LOW)
                {
                    shown[digit] = arduino::shifted.back ();
                }
            }
        }

        return shown;
    }

    // The segments of each character, as the display should show them.
    std::string glyphs (const char* characters)
    {
        std::string bytes;

        for (; *characters; ++characters)
        {
            bytes += static_cast<char> (adk::segments (*characters));
        }

        return bytes;
    }

    std::string dotted (std::string bytes, size_t position)
    {
        bytes[position] = static_cast<char> (bytes[position] | adk::segment::dot);
        return bytes;
    }
}

TEST (fourDigitDisplayClaimsItsPinsAndStartsDark)
{
    adk::FourDigitDisplay display {30, 31, 32, 22, 23, 24, 25};
    const uint8_t         pins [] = {30, 31, 32, 22, 23, 24, 25};

    adk::setup ();

    CHECK (!check::halted.happened);

    for (uint8_t pin : pins)
    {
        CHECK (arduino::pin (pin).mode == OUTPUT);
    }

    CHECK (lit () == "");
    CHECK (arduino::shifted == std::string (1, '\0'));
}

TEST (fourDigitDisplayHaltsOnADigitPinInUse)
{
    adk::Led              led     {24};
    adk::FourDigitDisplay display {30, 31, 32, 22, 23, 24, 25};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == 24);
}

TEST (fourDigitDisplayLightsOneDigitAtATimeEvery2Ms)
{
    adk::FourDigitDisplay display {30, 31, 32, 22, 23, 24, 25};

    adk::setup ();
    display.show (1234);
    arduino::shifted.clear ();

    adk::update (5000);
    CHECK (lit () == "1");
    CHECK (arduino::shifted == glyphs ("1"));

    adk::update (5001);
    CHECK (lit () == "1");
    CHECK (arduino::shifted == glyphs ("1"));

    adk::update (5002);
    CHECK (lit () == "2");
    CHECK (arduino::shifted == glyphs ("12"));

    adk::update (5004);
    CHECK (lit () == "3");

    adk::update (5006);
    CHECK (lit () == "4");

    adk::update (5008);
    CHECK (lit () == "1");
    CHECK (arduino::shifted == glyphs ("12341"));
}

TEST (fourDigitDisplayTimesEachDigitFromTheLastSwitch)
{
    adk::FourDigitDisplay display {30, 31, 32, 22, 23, 24, 25};

    adk::setup ();
    adk::update (0);
    adk::update (3);
    CHECK (lit () == "2");

    adk::update (4);
    CHECK (lit () == "2");

    adk::update (5);
    CHECK (lit () == "3");
}

TEST (fourDigitDisplayDarkensEveryDigitBeforeTheSegmentsChange)
{
    adk::FourDigitDisplay display {30, 31, 32, 22, 23, 24, 25};
    int                   latches = 0;
    int                   ghosts  = 0;

    adk::setup ();
    display.show (8888);

    arduino::onDigitalWrite = [&] (uint8_t pin, uint8_t value)
    {
        if (pin == Latch && value == HIGH)
        {
            ++latches;
            ghosts += lit () != "";
        }
    };

    for (adk::Millis now = 0; now < 40; now += 2)
    {
        adk::update (now);
    }

    CHECK (latches == 20);
    CHECK (ghosts == 0);
}

TEST (fourDigitDisplayShowsNumbersRightAligned)
{
    adk::FourDigitDisplay display {30, 31, 32, 22, 23, 24, 25};
    adk::Millis           now = 0;

    adk::setup ();

    display.show (0);
    CHECK (scan (now) == glyphs ("   0"));

    display.show (42);
    CHECK (scan (now) == glyphs ("  42"));

    display.show (9999);
    CHECK (scan (now) == glyphs ("9999"));

    display.show (-5);
    CHECK (scan (now) == glyphs ("  -5"));

    display.show (-999);
    CHECK (scan (now) == glyphs ("-999"));
}

TEST (fourDigitDisplayShowsDecimalsAfterTheDot)
{
    adk::FourDigitDisplay display {30, 31, 32, 22, 23, 24, 25};
    adk::Millis           now = 0;

    adk::setup ();

    display.show (123, 1);
    CHECK (scan (now) == dotted (glyphs (" 123"), 2));

    display.show (5, 1);
    CHECK (scan (now) == dotted (glyphs ("  05"), 2));

    display.show (7, 3);
    CHECK (scan (now) == dotted (glyphs ("0007"), 0));

    display.show (-25, 2);
    CHECK (scan (now) == dotted (glyphs ("-025"), 1));

    display.show (-250, 3);
    CHECK (scan (now) == glyphs ("----"));

    display.show (1, 4);
    CHECK (scan (now) == glyphs ("----"));
}

TEST (fourDigitDisplayShowsDashesOutOfRange)
{
    adk::FourDigitDisplay display {30, 31, 32, 22, 23, 24, 25};
    adk::Millis           now = 0;

    adk::setup ();

    display.show (10000);
    CHECK (scan (now) == glyphs ("----"));

    display.show (-1000);
    CHECK (scan (now) == glyphs ("----"));
}

TEST (fourDigitDisplayShowsTextWithDots)
{
    adk::FourDigitDisplay display {30, 31, 32, 22, 23, 24, 25};
    adk::Millis           now = 0;

    adk::setup ();

    display.show ("Hi");
    CHECK (scan (now) == glyphs ("Hi  "));

    display.show ("HELLO");
    CHECK (scan (now) == glyphs ("HELL"));

    display.show ("12.34");
    CHECK (scan (now) == dotted (glyphs ("1234"), 1));

    display.show ("ABCD.E");
    CHECK (scan (now) == dotted (glyphs ("ABCD"), 3));

    display.show (".5");
    CHECK (scan (now) == dotted (glyphs (" 5  "), 0));

    display.show ("1..2");
    CHECK (scan (now) == dotted (dotted (glyphs ("1 2 "), 0), 1));

    display.show ("");
    CHECK (scan (now) == glyphs ("    "));
}

TEST (fourDigitDisplayShowsMinutesAndSeconds)
{
    adk::FourDigitDisplay display {30, 31, 32, 22, 23, 24, 25};
    adk::Millis           now = 0;

    adk::setup ();

    display.showTime (5, 9);
    CHECK (scan (now) == dotted (glyphs ("0509"), 1));

    display.showTime (99, 59);
    CHECK (scan (now) == dotted (glyphs ("9959"), 1));

    display.showTime (100, 0);
    CHECK (scan (now) == glyphs ("----"));

    display.showTime (1, 60);
    CHECK (scan (now) == glyphs ("----"));
}

TEST (fourDigitDisplayClears)
{
    adk::FourDigitDisplay display {30, 31, 32, 22, 23, 24, 25};
    adk::Millis           now = 0;

    adk::setup ();
    display.show (8888);
    scan (now);

    display.clear ();
    CHECK (scan (now) == glyphs ("    "));
}

TEST (stoppedFourDigitDisplayIsDark)
{
    adk::FourDigitDisplay display {30, 31, 32, 22, 23, 24, 25};
    adk::Millis           now = 0;

    adk::setup ();
    display.show ("8.8.8.8.");
    scan (now);
    CHECK (lit () != "");

    adk::stop ();
    CHECK (lit () == "");
    CHECK (arduino::shifted.back () == '\0');

    CHECK (scan (now) == glyphs ("    "));
    CHECK (scan (now) == glyphs ("    "));
}
