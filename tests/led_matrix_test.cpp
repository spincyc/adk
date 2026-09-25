#include "check.h"

#include <Arduino.h>
#include <adk/font.h>
#include <adk/led_matrix.h>

#include <stdio.h>
#include <string>

namespace {

    // The wiring of the Elegoo MAX7219 lesson: DIN 12, CLK 11, CS 10.
    const adk::Pin Data  = 12;
    const adk::Pin Clock = 11;
    const adk::Pin Load  = 10;

    // Play the MAX7219: while load is low the bytes shifted in are one
    // register write, taken as load rises. Every write is logged as "AAVV"
    // and kept in the chip's registers.
    struct Chip
    {
        uint8_t     registers [16];
        std::string log;
        size_t      frameStart;
        bool        loadLow;
        bool        framed;
    };

    Chip chip;

    void listen ()
    {
        chip = Chip {};
        chip.frameStart = arduino::shifted.size ();
        chip.framed     = true;

        arduino::onDigitalWrite = [] (uint8_t pin, uint8_t value)
        {
            if (pin != Load)
            {
                return;
            }

            if (value == LOW)
            {
                chip.framed     = chip.framed && !chip.loadLow;
                chip.framed     = chip.framed && arduino::shifted.size () == chip.frameStart;
                chip.frameStart = arduino::shifted.size ();
                chip.loadLow    = true;
                return;
            }

            if (!chip.loadLow)
            {
                return;
            }

            std::string frame = arduino::shifted.substr (chip.frameStart);
            char        token [16];

            chip.frameStart = arduino::shifted.size ();
            chip.loadLow    = false;

            if (frame.size () != 2)
            {
                chip.framed = false;
                return;
            }

            uint8_t address = static_cast<uint8_t> (frame[0]);
            uint8_t data    = static_cast<uint8_t> (frame[1]);

            snprintf (token, sizeof token, " %02X%02X", address, data);
            chip.log += token;
            chip.registers[address & 0x0F] = data;
        };
    }

    // The writes since the last call, such as "0180 0301".
    std::string sent ()
    {
        std::string log = chip.log.empty () ? "" : chip.log.substr (1);
        chip.log.clear ();
        return log;
    }

    // Row y as the chip shows it.
    uint8_t shown (uint8_t y)
    {
        return chip.registers[y + 1];
    }

    bool dark ()
    {
        for (uint8_t y = 0; y < 8; ++y)
        {
            if (shown (y) != 0)
            {
                return false;
            }
        }

        return true;
    }
}

TEST (matrixSetupWakesTheChipWithABlankPicture)
{
    adk::LedMatrix matrix {Data, Clock, Load};

    listen ();
    adk::setup ();

    CHECK (arduino::pin (Data).mode == OUTPUT);
    CHECK (arduino::pin (Clock).mode == OUTPUT);
    CHECK (arduino::pin (Load).mode == OUTPUT);
    CHECK (arduino::pin (Load).output == HIGH);
    CHECK (sent () == "0F00 0B07 0900 0A07 0100 0200 0300 0400 0500 0600 0700 0800 0C01");
    CHECK (chip.framed);
}

TEST (matrixDrawsOnTheNextUpdate)
{
    adk::LedMatrix matrix {Data, Clock, Load};

    adk::setup ();
    listen ();

    matrix.set (0, 0);
    CHECK (matrix.get (0, 0));
    CHECK (sent () == "");

    adk::update (0);
    CHECK (sent () == "0180");

    matrix.set (7, 2);
    matrix.set (7, 2);
    adk::update (1);
    CHECK (sent () == "0301");

    matrix.set (0, 0, false);
    adk::update (2);
    CHECK (sent () == "0100");
    CHECK (!matrix.get (0, 0));

    matrix.set (8, 0);
    matrix.set (0, 8);
    adk::update (3);
    CHECK (sent () == "");
    CHECK (!matrix.get (8, 0));
    CHECK (chip.framed);
}

TEST (matrixRowBitSevenIsTheLeftColumn)
{
    adk::LedMatrix matrix {Data, Clock, Load};

    adk::setup ();
    listen ();

    matrix.row (4, 0xA0);
    adk::update (0);

    CHECK (sent () == "05A0");
    CHECK (matrix.get (0, 4));
    CHECK (!matrix.get (1, 4));
    CHECK (matrix.get (2, 4));
}

TEST (matrixSendsOnlyTheRowsThatChanged)
{
    const uint8_t smile [8] = {0x3C, 0x42, 0xA5, 0x81, 0xA5, 0x99, 0x42, 0x3C};
    const uint8_t frown [8] = {0x3C, 0x42, 0xA5, 0x81, 0x99, 0xA5, 0x42, 0x3C};
    adk::LedMatrix matrix {Data, Clock, Load};

    adk::setup ();
    listen ();

    matrix.show (smile);
    adk::update (0);
    CHECK (sent () == "013C 0242 03A5 0481 05A5 0699 0742 083C");

    matrix.show (frown);
    adk::update (1);
    CHECK (sent () == "0599 06A5");

    matrix.clear ();
    adk::update (2);
    CHECK (sent () == "0100 0200 0300 0400 0500 0600 0700 0800");
    CHECK (dark ());
}

TEST (matrixBrightnessIsSentOnceAndLimited)
{
    adk::LedMatrix matrix {Data, Clock, Load};

    adk::setup ();
    listen ();

    matrix.brightness (15);
    CHECK (sent () == "");

    adk::update (0);
    CHECK (sent () == "0A0F");

    matrix.brightness (99);
    adk::update (1);
    CHECK (sent () == "");

    matrix.brightness (0);
    adk::update (2);
    CHECK (sent () == "0A00");
}

TEST (matrixScrollsOneColumnEachStep)
{
    adk::LedMatrix matrix {Data, Clock, Load};

    adk::setup ();
    listen ();

    matrix.scroll ("A", 100);
    CHECK (matrix.isScrolling ());
    CHECK (sent () == "");

    // The first update brings the first column of A in at the right edge.
    adk::update (1000);
    CHECK (shown (0) == 0x00);
    CHECK (shown (1) == 0x01);
    CHECK (shown (6) == 0x01);
    CHECK (shown (7) == 0x00);

    adk::update (1099);
    CHECK (sent () == "0201 0301 0401 0501 0601 0701");

    adk::update (1100);
    CHECK (shown (0) == 0x01);
    CHECK (shown (1) == 0x02);
    CHECK (shown (4) == 0x03);

    // Step 8: A is whole at the left edge.
    for (adk::Millis now = 1200; now <= 1700; now += 100)
    {
        adk::update (now);
    }

    CHECK (shown (0) == 0x70);
    CHECK (shown (1) == 0x88);
    CHECK (shown (4) == 0xF8);
    CHECK (shown (6) == 0x88);
    CHECK (shown (7) == 0x00);

    // Step 12: only A's last column is left, at x 0.
    for (adk::Millis now = 1800; now <= 2100; now += 100)
    {
        adk::update (now);
    }

    CHECK (shown (0) == 0x00);
    CHECK (shown (1) == 0x80);

    adk::update (2199);
    CHECK (matrix.isScrolling ());

    // Step 5 + 8 = 13: A has gone.
    adk::update (2200);
    CHECK (!matrix.isScrolling ());
    CHECK (dark ());

    sent ();
    adk::update (2300);
    CHECK (sent () == "");
    CHECK (chip.framed);
}

TEST (matrixLeavesAGapBetweenCharacters)
{
    adk::LedMatrix matrix {Data, Clock, Load};

    adk::setup ();
    listen ();

    matrix.scroll ("AB", 50);

    for (adk::Millis now = 0; now <= 350; now += 50)
    {
        adk::update (now);
    }

    // Step 8: A fills x 0-4, x 5 is the gap, and B's first columns are at 6-7.
    CHECK (shown (0) == 0x73);
    CHECK (shown (4) == 0xFA);

    for (uint8_t y = 0; y < 8; ++y)
    {
        CHECK ((shown (y) & 0x04) == 0);
    }

    // Eleven columns, gone after 11 + 8 = 19 steps.
    for (adk::Millis now = 400; now <= 850; now += 50)
    {
        adk::update (now);
    }

    adk::update (899);
    CHECK (matrix.isScrolling ());

    adk::update (900);
    CHECK (!matrix.isScrolling ());
}

TEST (matrixScrollAskedForEveryLoopKeepsGoing)
{
    adk::LedMatrix matrix {Data, Clock, Load};
    adk::Millis    now = 0;

    adk::setup ();

    // "Hi" is 11 columns, so it scrolls for 19 steps.
    for (; now < 1800; now += 10)
    {
        matrix.scroll ("Hi", 100);
        adk::update (now);
        CHECK (matrix.isScrolling ());
    }

    matrix.scroll ("Hi", 100);
    adk::update (now);
    CHECK (!matrix.isScrolling ());

    matrix.scroll ("Hi", 100);
    CHECK (matrix.isScrolling ());
}

TEST (matrixEmptyTextDoesNotScroll)
{
    adk::LedMatrix matrix {Data, Clock, Load};

    adk::setup ();
    matrix.scroll ("");
    CHECK (!matrix.isScrolling ());

    matrix.scroll (nullptr);
    CHECK (!matrix.isScrolling ());
}

TEST (matrixDrawingEndsAScroll)
{
    adk::LedMatrix matrix {Data, Clock, Load};

    adk::setup ();
    listen ();

    matrix.scroll ("Hi", 100);
    adk::update (0);
    sent ();

    matrix.set (0, 7);
    CHECK (!matrix.isScrolling ());

    adk::update (1000);
    CHECK (sent () == "0880");
}

TEST (matrixStopEndsTheScrollAndGoesDark)
{
    adk::LedMatrix matrix {Data, Clock, Load};

    adk::setup ();
    listen ();

    matrix.scroll ("A", 100);
    adk::update (0);
    adk::update (400);
    CHECK (!dark ());

    adk::stop ();
    CHECK (!matrix.isScrolling ());
    CHECK (dark ());

    sent ();
    adk::update (500);
    CHECK (sent () == "");
    CHECK (chip.framed);
}

TEST (matrixSharingAPinHaltsBeforeStarting)
{
    adk::Led       led    {Clock};
    adk::LedMatrix matrix {Data, Clock, Load};

    listen ();
    adk::setup ();

    CHECK (check::halted.happened);
    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == Clock);
    CHECK (sent () == "");
}

TEST (fontDrawsPrintableAsciiAndQuestionMarksTheRest)
{
    CHECK (adk::fontColumn ('A', 0) == 0x7E);
    CHECK (adk::fontColumn ('A', 1) == 0x11);
    CHECK (adk::fontColumn (' ', 2) == 0x00);
    CHECK (adk::fontColumn ('~', 1) == 0x04);
    CHECK (adk::fontColumn ('A', 5) == 0x00);

    for (uint8_t column = 0; column < 5; ++column)
    {
        CHECK (adk::fontColumn ('\t', column) == adk::fontColumn ('?', column));
        CHECK (adk::fontColumn (static_cast<char> (0xB0), column) == adk::fontColumn ('?', column));
    }

    for (char character = ' '; character <= '~'; ++character)
    {
        for (uint8_t column = 0; column < 5; ++column)
        {
            CHECK ((adk::fontColumn (character, column) & 0x80) == 0);
        }
    }
}
