#include "check.h"

#include <Arduino.h>
#include <adk/lcd.h>

#include <stdio.h>
#include <string>
#include <vector>

namespace {

    // The wiring of the Elegoo LCD1602 lesson: RS 7, E 8, D4-D7 on 9-12.
    const adk::Pin Rs     = 7;
    const adk::Pin Enable = 8;
    const adk::Pin D4     = 9;

    // One nibble the controller latched as E fell, when E rose and fell, and
    // whether RS and D4-D7 held still while E was high.
    struct Latch
    {
        uint8_t       nibble;
        uint8_t       rs;
        unsigned long rose;
        unsigned long fell;
        bool          steady;
    };

    std::vector<Latch> latches;
    bool               enableHigh = false;

    uint8_t lines ()
    {
        uint8_t nibble = 0;

        for (uint8_t line = 0; line < 4; ++line)
        {
            uint8_t level = arduino::pin (static_cast<uint8_t> (D4 + line)).output;
            nibble        = static_cast<uint8_t> (nibble | (level << line));
        }

        return nibble;
    }

    // Play the LCD controller: note every nibble E latches.
    void listen ()
    {
        latches.clear ();
        enableHigh = false;

        arduino::onDigitalWrite = [] (uint8_t pin, uint8_t value)
        {
            uint8_t rs = arduino::pin (Rs).output;

            if (pin == Enable && value == HIGH && !enableHigh)
            {
                latches.push_back ({lines (), rs, arduino::now (), 0, true});
                enableHigh = true;
            }
            else if (pin == Enable && value == LOW && enableHigh)
            {
                Latch& latch = latches.back ();
                bool   held  = lines () == latch.nibble && rs == latch.rs;

                latch.fell   = arduino::now ();
                latch.steady = latch.steady && held;
                enableHigh   = false;
            }
            else if (enableHigh)
            {
                latches.back ().steady = false;
            }
        };
    }

    // The bytes latched from the given nibble on, high nibble first:
    // instructions as <C3>, printable characters as themselves, and other
    // data as {1F}.
    std::string sent (size_t first)
    {
        std::string text;

        for (size_t index = first; index + 1 < latches.size (); index += 2)
        {
            const Latch& high  = latches[index];
            const Latch& low   = latches[index + 1];
            unsigned     value = static_cast<unsigned> ((high.nibble << 4) | low.nibble);
            char         token [16];

            if (high.rs == HIGH && value >= ' ' && value <= '~')
            {
                snprintf (token, sizeof token, "%c", value);
            }
            else
            {
                snprintf (token, sizeof token, high.rs == HIGH ? "{%02X}" : "<%02X>", value);
            }

            text += token;
        }

        return text;
    }

    // The time from the end of one nibble to the start of the next.
    unsigned long gapAfter (size_t index)
    {
        return latches[index + 1].rose - latches[index].fell;
    }

    // Every E pulse was at least 1 us long and the lines held still during it.
    bool clean ()
    {
        for (const Latch& latch : latches)
        {
            if (latch.fell - latch.rose < 1 || !latch.steady)
            {
                return false;
            }
        }

        return !enableHigh;
    }
}

TEST (lcdStartsTheWayTheDatasheetSays)
{
    adk::Lcd lcd {Rs, Enable, 9, 10, 11, 12};

    listen ();
    adk::setup ();

    for (adk::Pin pin : {Rs, Enable, adk::Pin (9), adk::Pin (10), adk::Pin (11), adk::Pin (12)})
    {
        CHECK (arduino::pin (pin).mode == OUTPUT);
    }

    CHECK (latches.size () == 4 + 2 * 4);
    CHECK (latches[0].nibble == 0x3 && latches[1].nibble == 0x3);
    CHECK (latches[2].nibble == 0x3 && latches[3].nibble == 0x2);
    CHECK (latches[0].rs == LOW && latches[3].rs == LOW);
    CHECK (sent (4) == "<28><0C><01><06>");

    CHECK (latches[0].rose >= 40000);
    CHECK (gapAfter (0) >= 4100);
    CHECK (gapAfter (1) >= 100);
    CHECK (gapAfter (2) >= 100);
    CHECK (gapAfter (3) >= 37);
    CHECK (gapAfter (5) >= 37);
    CHECK (gapAfter (7) >= 37);
    CHECK (gapAfter (9) >= 1520);
    CHECK (arduino::now () - latches[11].fell >= 37);
    CHECK (clean ());
    CHECK (arduino::pin (Enable).output == LOW);
}

TEST (lcdPrintsCharactersAsData)
{
    adk::Lcd lcd {Rs, Enable, 9, 10, 11, 12};

    adk::setup ();
    listen ();

    CHECK (lcd.print ("Hi") == 2);
    lcd.print (23.5, 1);

    CHECK (sent (0) == "Hi23.5");
    CHECK (latches[0].rs == HIGH);
    CHECK (gapAfter (1) >= 37);
    CHECK (clean ());
}

TEST (lcdSetCursorAddressesEachRowAndClamps)
{
    adk::Lcd lcd {Rs, Enable, 9, 10, 11, 12};

    adk::setup ();
    listen ();

    lcd.setCursor (3, 1);
    lcd.setCursor (0, 0);
    lcd.setCursor (20, 5);

    CHECK (sent (0) == "<C3><80><CF>");
}

TEST (lcdNewlineMovesToTheOtherRow)
{
    adk::Lcd lcd {Rs, Enable, 9, 10, 11, 12};

    adk::setup ();
    listen ();

    lcd.print ("A\nB\nC");
    CHECK (lcd.println ("Hi") == 4);

    CHECK (sent (0) == "A<C0>B<80>CHi<C0>");
}

TEST (lcdClearAndHomeWaitForTheController)
{
    adk::Lcd lcd {Rs, Enable, 9, 10, 11, 12};

    adk::setup ();
    listen ();

    lcd.clear ();
    lcd.print ("A");
    lcd.home ();
    lcd.print ("B");

    CHECK (sent (0) == "<01>A<02>B");
    CHECK (gapAfter (1) >= 1520);
    CHECK (gapAfter (5) >= 1520);
}

TEST (lcdCreateCharFillsASlotAndPutsTheCursorBack)
{
    const uint8_t heart [8] = {0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00, 0x00};
    adk::Lcd      lcd {Rs, Enable, 9, 10, 11, 12};

    adk::setup ();
    listen ();

    lcd.print ("Hi");
    lcd.createChar (2, heart);
    lcd.write (uint8_t (2));

    CHECK (sent (0) == "Hi<50>{00}{0A}{1F}{1F}{0E}{04}{00}{00}<82>{02}");

    listen ();
    lcd.createChar (9, heart);
    CHECK (sent (0).substr (0, 4) == "<48>");
}

TEST (lcdCursorRunsOnThroughTheHiddenColumns)
{
    const uint8_t blank [8] = {};
    adk::Lcd      lcd {Rs, Enable, 9, 10, 11, 12};

    adk::setup ();

    for (int column = 0; column < 40; ++column)
    {
        lcd.write ('x');
    }

    listen ();
    lcd.createChar (0, blank);
    CHECK (sent (0).substr (sent (0).size () - 4) == "<C0>");

    for (int column = 0; column < 40; ++column)
    {
        lcd.write ('x');
    }

    listen ();
    lcd.createChar (0, blank);
    CHECK (sent (0).substr (sent (0).size () - 4) == "<80>");
}

TEST (lcdKeepsItsTextWhenStopped)
{
    adk::Lcd lcd {Rs, Enable, 9, 10, 11, 12};

    adk::setup ();
    lcd.print ("Bye");
    listen ();

    adk::stop ();
    adk::update (1000);

    CHECK (latches.empty ());
}

TEST (lcdSharingAPinHaltsBeforeStarting)
{
    adk::Led led {10};
    adk::Lcd lcd {Rs, Enable, 9, 10, 11, 12};

    listen ();
    adk::setup ();

    CHECK (check::halted.happened);
    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == 10);
    CHECK (latches.empty ());
}

TEST (lcdAtMovesTheCursorAndHandsBackTheScreen)
{
    adk::Lcd lcd {7, 8, 9, 10, 11, 12};

    adk::setup ();
    adk::Lcd& same = lcd.at (3, 1);
    CHECK (&same == &lcd);
}
