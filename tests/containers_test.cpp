#include "check.h"

#include <Arduino.h>

TEST (arrayKnowsItsSizeAndWalksInOrder)
{
    adk::Array pins {22, 23, 24};
    int        total = 0;

    for (auto pin : pins)
    {
        total += pin;
    }

    CHECK (pins.size () == 3);
    CHECK (total == 69);
    CHECK (pins.front () == 22 && pins.back () == 24);

    pins.fill (7);
    CHECK (pins[1] == 7);
}

TEST (arrayCanHoldParts)
{
    adk::Array lamps {adk::Led {26}, adk::Led {27}};

    CHECK (adk::start ());
    lamps[1].on ();
    CHECK (arduino::pin (27).output == HIGH);
}

TEST (vectorGrowsToItsCapacityAndNoFurther)
{
    adk::Vector<int, 3> keys;

    CHECK (keys.empty ());
    CHECK (keys.push_back (1) && keys.push_back (2) && keys.push_back (3));
    CHECK (!keys.push_back (4));
    CHECK (keys.full () && keys.size () == 3);

    keys.erase (0);
    CHECK (keys.size () == 2 && keys.front () == 2 && keys.back () == 3);

    keys.pop_back ();
    keys.clear ();
    CHECK (keys.empty ());
}

TEST (dequeWrapsRoundItsRing)
{
    adk::Deque<int, 3> snake;

    snake.push_back (1);
    snake.push_back (2);
    snake.push_front (0);
    CHECK (snake.full () && !snake.push_back (9));

    snake.pop_back ();
    snake.push_front (-1);
    CHECK (snake.front () == -1 && snake.back () == 1);

    int walked [3] {};
    int count = 0;
    for (auto part : snake)
    {
        walked[count++] = part;
    }
    CHECK (count == 3 && walked[0] == -1 && walked[1] == 0 && walked[2] == 1);

    snake.pop_front ();
    CHECK (snake.size () == 2 && snake.front () == 0);
}

TEST (spanViewsAnArrayWithoutCopying)
{
    int  values [] {4, 5, 6};
    adk::Span view {values};

    view[0] = 9;
    CHECK (values[0] == 9);
    CHECK (view.size () == 3 && view.back () == 6);
}

TEST (printPutsPartsInARow)
{
    arduino::Log log;

    adk::println (log, "Red wins in ", 243, " ms", '!');
    CHECK (log.text == "Red wins in 243 ms!\r\n");
}

TEST (spanViewsAVectorsItemsSoFar)
{
    adk::Vector<int, 4> taps;

    taps.push_back (120);
    taps.push_back (480);

    adk::Span view {taps};
    CHECK (view.size () == 2 && view[1] == 480);
}

TEST (equalComparesAnyTwoLists)
{
    adk::Array          code {'2', '4', '6', '8'};
    adk::Vector<char, 4> typed;

    for (char key : {'2', '4', '6'})
    {
        typed.push_back (key);
    }
    CHECK (!adk::equal (typed, code));

    typed.push_back ('8');
    CHECK (adk::equal (typed, code));

    typed[0] = '1';
    CHECK (!adk::equal (code, typed));
}

TEST (textCollectsWhatIsPrinted)
{
    adk::Text<10> message;

    adk::print (message, "SCORE ", 17);
    CHECK (strcmp (message.c_str (), "SCORE 17") == 0);

    adk::print (message, " and more");
    CHECK (message.size () == 10);
    CHECK (strcmp (message.c_str (), "SCORE 17 a") == 0);

    message.clear ();
    CHECK (message.size () == 0 && message.c_str ()[0] == '\0');

    adk::print (message, 'S', 'L', 'S');
    CHECK (message == "SLS");
    CHECK (!(message == "SL"));
}

TEST (fixedPrintsASetCountOfDecimals)
{
    arduino::Log log;

    adk::print (log, adk::fixed (21.46, 1), "C ", adk::fixed (54.0, 0), '%');
    CHECK (log.text == "21.5C 54%");
}

TEST (hexPrintsCapitalDigitsPaddedWithZeros)
{
    arduino::Log log;

    adk::print (log, adk::hex (0x0C, 2), ' ', adk::hex (0x1A2B3C4D, 8), ' ', adk::hex (0xBEEF, 2));
    CHECK (log.text == "0C 1A2B3C4D BEEF");

    arduino::Log bare;

    adk::print (bare, adk::hex (0), ' ', adk::hex (255), ' ', adk::hex (0x5, 4));
    CHECK (bare.text == "0 FF 0005");
}

TEST (hexPrintsIntoText)
{
    adk::Text<16> card;

    adk::print (card, "Card 0x", adk::hex (0xAB, 4));
    CHECK (card == "Card 0x00AB");
}
