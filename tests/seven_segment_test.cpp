#include "check.h"

#include <Arduino.h>
#include <adk/segments.h>
#include <adk/seven_segment.h>

using namespace adk::segment;

namespace {

    uint8_t lastShifted ()
    {
        return static_cast<uint8_t> (arduino::shifted.back ());
    }
}

TEST (segmentsDrawEveryDigit)
{
    const uint8_t digits [10] = {
        a | b | c | d | e | f,     b | c,             a | b | d | e | g,
        a | b | c | d | g,         b | c | f | g,     a | c | d | f | g,
        a | c | d | e | f | g,     a | b | c,         a | b | c | d | e | f | g,
        a | b | c | d | f | g};

    for (uint8_t digit = 0; digit < 10; ++digit)
    {
        CHECK (adk::segments (static_cast<char> ('0' + digit)) == digits[digit]);
    }
}

TEST (segmentsDrawHexInEitherCaseWithLowercaseBAndD)
{
    CHECK (adk::segments ('A') == (a | b | c | e | f | g));
    CHECK (adk::segments ('b') == (c | d | e | f | g));
    CHECK (adk::segments ('C') == (a | d | e | f));
    CHECK (adk::segments ('d') == (b | c | d | e | g));
    CHECK (adk::segments ('E') == (a | d | e | f | g));
    CHECK (adk::segments ('F') == (a | e | f | g));

    for (char letter = 'A'; letter <= 'F'; ++letter)
    {
        CHECK (adk::segments (letter) == adk::segments (static_cast<char> (letter + 'a' - 'A')));
    }
}

TEST (segmentsDrawLettersThatReadWell)
{
    CHECK (adk::segments ('H') == (b | c | e | f | g));
    CHECK (adk::segments ('h') == (c | e | f | g));
    CHECK (adk::segments ('L') == (d | e | f));
    CHECK (adk::segments ('P') == (a | b | e | f | g));
    CHECK (adk::segments ('S') == adk::segments ('5'));
    CHECK (adk::segments ('U') == (b | c | d | e | f));
    CHECK (adk::segments ('u') == (c | d | e));
    CHECK (adk::segments ('o') == (c | d | e | g));
    CHECK (adk::segments ('n') == (c | e | g));
    CHECK (adk::segments ('r') == (e | g));
    CHECK (adk::segments ('t') == (d | e | f | g));
    CHECK (adk::segments ('y') == (b | c | d | f | g));
    CHECK (adk::segments ('-') == g);
    CHECK (adk::segments ('_') == d);
    CHECK (adk::segments (' ') == 0);
}

TEST (segmentsLeaveOtherCharactersBlankAndNeverLightTheDot)
{
    CHECK (adk::segments ('M') == 0);
    CHECK (adk::segments ('.') == 0);
    CHECK (adk::segments ('\n') == 0);
    CHECK (adk::segments ('\x7F') == 0);
    CHECK (adk::segments (static_cast<char> (200)) == 0);

    for (int code = 0; code < 256; ++code)
    {
        CHECK (!(adk::segments (static_cast<char> (code)) & dot));
    }
}

TEST (sevenSegmentClaimsThreePinsAndStartsBlank)
{
    adk::SevenSegment digit {30, 31, 32};

    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (arduino::pin (30).mode == OUTPUT);
    CHECK (arduino::pin (31).mode == OUTPUT);
    CHECK (arduino::pin (32).mode == OUTPUT);
    CHECK (arduino::shifted == std::string (1, '\0'));
}

TEST (sevenSegmentHaltsOnAPinInUse)
{
    adk::SevenSegment digit {30, 31, 32};
    adk::Led          led   {32};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == 32);
}

TEST (sevenSegmentShowsDigitsHexAndADashForTheRest)
{
    adk::SevenSegment digit {30, 31, 32};
    uint8_t           count = 3;

    adk::setup ();

    digit.show (7);
    CHECK (lastShifted () == (a | b | c));

    digit.show (count);
    CHECK (lastShifted () == adk::segments ('3'));

    digit.show (12);
    CHECK (lastShifted () == adk::segments ('C'));

    digit.show (16);
    CHECK (lastShifted () == g);

    digit.show (-1);
    CHECK (lastShifted () == g);
}

TEST (sevenSegmentShowsCharacters)
{
    adk::SevenSegment digit {30, 31, 32};

    adk::setup ();

    digit.show ('H');
    CHECK (lastShifted () == (b | c | e | f | g));

    digit.show ('?');
    CHECK (lastShifted () == 0);
}

TEST (sevenSegmentDotIsIndependentOfShow)
{
    adk::SevenSegment digit {30, 31, 32};

    adk::setup ();
    digit.show (8);
    digit.dot (true);
    CHECK (lastShifted () == 0xFF);

    digit.show (1);
    CHECK (lastShifted () == (b | c | dot));

    digit.dot (false);
    CHECK (lastShifted () == (b | c));

    digit.dot (true);
    digit.clear ();
    CHECK (lastShifted () == 0);
}

TEST (commonAnodeSevenSegmentInvertsEverySegment)
{
    adk::SevenSegment digit {30, 31, 32, adk::ActiveLow};

    adk::setup ();
    CHECK (lastShifted () == 0xFF);

    digit.show (1);
    CHECK (lastShifted () == static_cast<uint8_t> (~(b | c)));

    adk::stop ();
    CHECK (lastShifted () == 0xFF);
}

TEST (stoppedSevenSegmentIsBlank)
{
    adk::SevenSegment digit {30, 31, 32};

    adk::setup ();
    digit.show (8);
    digit.dot (true);
    adk::stop ();

    CHECK (lastShifted () == 0);
}
