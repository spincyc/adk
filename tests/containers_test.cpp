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
