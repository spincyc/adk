#include "check.h"

#include <Arduino.h>
#include <adk/keypad.h>

namespace {

    const char* const Printed = "123A456B789C*0#D";

    const adk::Pin Rows    [4] = {22, 23, 24, 25};
    const adk::Pin Columns [4] = {26, 27, 28, 29};

    // Plays the keypad: a column reads low while a held key joins it to a
    // row pulled low. Anything that could short two pins is a mistake: a
    // row driven high, or two rows driven at once.
    struct Keys
    {
        Keys ()
        {
            arduino::onDigitalRead  = [this] (uint8_t pin) { return read (pin); };
            arduino::onDigitalWrite = [this] (uint8_t pin, uint8_t) { checkRows (pin); };
        }

        int read (uint8_t pin)
        {
            int  driven = 0;
            bool joined = false;

            for (uint8_t row = 0; row < 4; ++row)
            {
                if (arduino::pin (Rows[row]).mode != OUTPUT)
                {
                    continue;
                }

                ++driven;

                for (uint8_t column = 0; column < 4; ++column)
                {
                    char key = Printed[row * 4 + column];
                    joined   = joined || (pin == Columns[column] && held.find (key) != held.npos);
                }
            }

            mistakes += driven > 1 ? 1 : 0;
            return joined ? LOW : arduino::pin (pin).input;
        }

        // A row's level is set while it still floats, so it is never driven high.
        void checkRows (uint8_t pin)
        {
            for (adk::Pin row : Rows)
            {
                mistakes += (pin == row && arduino::pin (row).mode == OUTPUT) ? 1 : 0;
            }
        }

        std::string held;
        int         mistakes = 0;
    };
}

TEST (keypadRowsFloatAndColumnsArePulledUp)
{
    adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};

    adk::setup ();

    CHECK (!check::halted.happened);

    for (adk::Pin row : Rows)
    {
        CHECK (arduino::pin (row).mode == INPUT);
    }

    for (adk::Pin column : Columns)
    {
        CHECK (arduino::pin (column).mode == INPUT_PULLUP);
    }
}

TEST (keypadPinUsedTwiceHalts)
{
    adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 22}};

    adk::setup ();

    CHECK (check::halted.happened);
    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == 22);
}

TEST (keypadPinTakenByAnotherPartHalts)
{
    adk::Led    led    {27};
    adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == 27);
}

TEST (keypadOnAMissingPinHalts)
{
    adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 70}};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::NoSuchPin);
    CHECK (check::halted.pin == 70);
}

TEST (everyKeyReadsAsPrinted)
{
    adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};
    Keys        keys;
    std::string typed;

    adk::setup ();

    for (adk::Millis now = 0; typed.size () < 16; now += 100)
    {
        keys.held = Printed[typed.size ()];
        adk::update (now);
        adk::update (now + 20);
        typed += keypad.key ();

        keys.held.clear ();
        adk::update (now + 50);
        adk::update (now + 70);
    }

    CHECK (typed == Printed);
    CHECK (keys.mistakes == 0);
}

TEST (keyIsDebouncedAndReportedOnce)
{
    adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};
    Keys        keys;

    adk::setup ();
    adk::update (0);

    keys.held = "5";
    adk::update (100);
    adk::update (119);
    CHECK (keypad.key () == '\0');
    CHECK (keypad.heldKey () == '\0');

    adk::update (120);
    CHECK (keypad.key () == '5');
    CHECK (keypad.heldKey () == '5');
    CHECK (keypad.isPressed ('5'));
    CHECK (!keypad.isPressed ('6'));

    adk::update (121);
    CHECK (keypad.key () == '\0');
    CHECK (keypad.heldKey () == '5');

    keys.held.clear ();
    adk::update (200);
    adk::update (219);
    CHECK (keypad.isPressed ('5'));

    adk::update (220);
    CHECK (keypad.heldKey () == '\0');
    CHECK (keypad.key () == '\0');
    CHECK (!keypad.isPressed ('\0'));
}

TEST (aBouncingKeyRestartsTheWait)
{
    adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};
    Keys        keys;

    adk::setup ();

    keys.held = "8";
    adk::update (0);
    keys.held.clear ();
    adk::update (5);
    keys.held = "8";
    adk::update (10);
    adk::update (29);
    CHECK (keypad.key () == '\0');

    adk::update (30);
    CHECK (keypad.key () == '8');
}

TEST (onlyOneRowIsPulledLowAtATime)
{
    adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};
    Keys        keys;

    adk::setup ();

    // A whole column held joins all four rows.
    keys.held = "147*";
    adk::update (0);
    adk::update (20);

    CHECK (keypad.key () == '1');
    CHECK (keys.mistakes == 0);

    for (adk::Pin row : Rows)
    {
        CHECK (arduino::pin (row).mode == INPUT);
    }
}

TEST (aSecondKeyWaitsForTheFirstToBeLetGo)
{
    adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};
    Keys        keys;

    adk::setup ();

    keys.held = "5";
    adk::update (0);
    adk::update (20);
    CHECK (keypad.key () == '5');

    keys.held = "51";
    adk::update (30);
    adk::update (100);
    CHECK (keypad.key () == '\0');
    CHECK (keypad.heldKey () == '5');

    keys.held = "1";
    adk::update (110);
    adk::update (129);
    CHECK (keypad.key () == '\0');

    adk::update (130);
    CHECK (keypad.key () == '1');
}

TEST (aKeyHeldAtStartupIsHeldButNotPressed)
{
    adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};
    Keys        keys;

    keys.held = "#";
    adk::setup ();
    adk::update (0);
    adk::update (50);

    CHECK (keypad.heldKey () == '#');
    CHECK (keypad.key () == '\0');
}
