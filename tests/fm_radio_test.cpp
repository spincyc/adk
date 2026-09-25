#include "check.h"
#include "fake_si4703.h"

#include <Arduino.h>
#include <string>

namespace {

    const adk::Pin Sdio  = 40;
    const adk::Pin Sclk  = 41;
    const adk::Pin Reset = 42;

    const uint16_t Tune    = 1u << 15;
    const uint16_t Unmute  = 1u << 14;
    const uint16_t Enable  = 1u << 0;

    // Let time pass in steps of 10 ms, updating after each, and count the
    // updates in which tuning finished.
    int run (unsigned long ms, const adk::FmRadio& radio)
    {
        int tuned = 0;

        for (unsigned long elapsed = 0; elapsed < ms; elapsed += 10)
        {
            arduino::advance (10);
            adk::update ();
            tuned += radio.wasTuned () ? 1 : 0;
        }

        return tuned;
    }

    // The four groups that carry a station's eight-letter name.
    void sendName (fake::Si4703& chip, const char* name)
    {
        for (uint16_t segment = 0; segment < 4; ++segment)
        {
            uint16_t first   = static_cast<uint16_t> (name[2 * segment] << 8);
            uint16_t letters = static_cast<uint16_t> (first | name[2 * segment + 1]);
            chip.groups.push_back ({0x54A8, segment, 0, letters});
        }
    }

    // Groups of kind 2A, four letters each, with the A/B flag given.
    void sendText (fake::Si4703& chip, std::string text, uint16_t flag = 0)
    {
        text += '\r';

        while (text.size () % 4 != 0)
        {
            text += ' ';
        }

        for (uint16_t segment = 0; segment < text.size () / 4; ++segment)
        {
            auto letters = [&] (size_t at)
            {
                return static_cast<uint16_t> (text[at] << 8 | text[at + 1]);
            };

            uint16_t b = static_cast<uint16_t> (0x2000 | flag << 4 | segment);
            chip.groups.push_back ({0x54A8, b, letters (4 * segment), letters (4 * segment + 2)});
        }
    }
}

TEST (fmRadioWakesOnTheTwoWireBusWithoutEverDriving5V)
{
    fake::Si4703  chip  {Sdio, Sclk, Reset};
    adk::FmRadio  radio {Sdio, Sclk, Reset};

    adk::setup ();
    CHECK (adk::fault () == adk::Fault::None);
    CHECK (chip.twoWire);
    CHECK (radio.ok ());
    CHECK (chip.registers[0x07] == 0x8100);
    CHECK ((chip.registers[0x02] & (Unmute | Enable)) == (Unmute | Enable));
    CHECK (chip.registers[0x04] & (1u << 12));
    CHECK (radio.volume () == 8);
    CHECK ((chip.registers[0x05] & 0x0F) == 8);

    run (200, radio);
    CHECK (!chip.drivenHigh);

    for (adk::Pin pin : {Sdio, Sclk, Reset})
    {
        CHECK (arduino::pin (pin).mode == INPUT);
        CHECK (arduino::pin (pin).output == LOW);
    }
}

TEST (fmRadioWithoutAChipIsNotOk)
{
    adk::FmRadio radio {Sdio, Sclk, Reset};

    adk::setup ();
    CHECK (adk::fault () == adk::Fault::None);
    CHECK (!radio.ok ());

    radio.tune (1011);
    arduino::advance (100);
    adk::update ();
    CHECK (!radio.isTuning ());
}

TEST (fmRadioTunesWhileTheSketchCarriesOn)
{
    fake::Si4703 chip  {Sdio, Sclk, Reset};
    adk::FmRadio radio {Sdio, Sclk, Reset};

    chip.stations = {{136, 42, true}};
    adk::setup ();
    run (200, radio);

    radio.tune (1011);
    CHECK (radio.frequency () == 1011);
    CHECK (radio.isTuning ());
    CHECK (chip.registers[0x03] == (Tune | 136));

    CHECK (run (40, radio) == 0);
    CHECK (radio.isTuning ());
    CHECK (run (60, radio) == 1);
    CHECK (!radio.isTuning ());
    CHECK (radio.frequency () == 1011);
    CHECK (chip.registers[0x03] == 136);
    CHECK (radio.signal () == 42);
    CHECK (radio.isStereo ());
}

TEST (fmRadioStepsTwoTenthsInTheAmericas)
{
    fake::Si4703 chip  {Sdio, Sclk, Reset};
    adk::FmRadio radio {Sdio, Sclk, Reset, adk::FmBand::Americas};

    adk::setup ();
    CHECK ((chip.registers[0x05] & 0xF0) == 0x00);
    CHECK (!(chip.registers[0x04] & (1u << 11)));
    run (200, radio);

    radio.tune (1011);
    CHECK ((chip.registers[0x03] & 0x3FF) == 68);
    run (100, radio);

    radio.step (1);
    CHECK (radio.frequency () == 1013);
    run (100, radio);
    radio.tune (1012);
    CHECK (radio.frequency () == 1011);
    run (100, radio);
    radio.tune (875);
    run (100, radio);
    radio.step (-1);
    CHECK (radio.frequency () == 1079);
    radio.tune (2000);
    CHECK (radio.frequency () == 1079);
}

TEST (fmRadioUsesTheWorldBandUnlessToldOtherwise)
{
    fake::Si4703 chip  {Sdio, Sclk, Reset};
    adk::FmRadio world {Sdio, Sclk, Reset};

    adk::setup ();
    CHECK ((chip.registers[0x05] & 0xF0) == 0x10);
    CHECK (chip.registers[0x04] & (1u << 11));
    world.tune (100);
    CHECK (world.frequency () == 875);
    world.step (-1);
    CHECK (world.frequency () == 1080);
}

TEST (fmRadioCatchesUpWithAKnobTurnedWhileTuning)
{
    fake::Si4703 chip  {Sdio, Sclk, Reset};
    adk::FmRadio radio {Sdio, Sclk, Reset};

    adk::setup ();
    run (200, radio);

    radio.tune (1000);
    run (20, radio);
    radio.step (1);
    radio.step (1);
    run (20, radio);
    radio.step (1);
    CHECK (radio.frequency () == 1003);

    CHECK (run (300, radio) == 1);
    CHECK (radio.frequency () == 1003);
    CHECK ((chip.registers[0x0B] & 0x3FF) == 128);
}

TEST (fmRadioSeeksTheNextStationEitherWay)
{
    fake::Si4703 chip  {Sdio, Sclk, Reset};
    adk::FmRadio radio {Sdio, Sclk, Reset};

    chip.stations = {{25, 30, false}, {136, 42, true}};
    adk::setup ();
    radio.tune (950);
    run (200, radio);

    radio.seekUp ();
    CHECK (radio.isTuning ());
    CHECK (run (200, radio) == 1);
    CHECK (radio.frequency () == 1011);
    CHECK (radio.signal () == 42);

    radio.seekDown ();
    run (200, radio);
    CHECK (radio.frequency () == 900);

    // With only one station, seeking comes back round to it.
    chip.stations = {{25, 30, false}};
    radio.seekUp ();
    run (200, radio);
    CHECK (radio.frequency () == 900);
    CHECK (!radio.isTuning ());
}

TEST (fmRadioLearnsTheStationsName)
{
    fake::Si4703 chip  {Sdio, Sclk, Reset};
    adk::FmRadio radio {Sdio, Sclk, Reset};

    adk::setup ();
    radio.tune (1011);
    run (200, radio);
    CHECK (std::string (radio.stationName ()).empty ());

    // A group with too many errors in its letters is ignored.
    chip.groups.push_back ({0x54A8, 1, 0, 'X' << 8 | 'X', 0, 0, 3});
    sendName (chip, "ADK FM  ");

    int named = 0;

    for (int pass = 0; pass < 10; ++pass)
    {
        arduino::advance (40);
        adk::update ();
        named += radio.nameChanged () ? 1 : 0;
    }

    CHECK (std::string (radio.stationName ()) == "ADK FM  ");
    CHECK (named == 1);

    // The same name again is no change.
    sendName (chip, "ADK FM  ");
    run (400, radio);
    CHECK (!radio.nameChanged ());

    radio.tune (950);
    CHECK (std::string (radio.stationName ()).empty ());
}

TEST (fmRadioReadsTheTextAndStartsAfreshWhenItsFlagFlips)
{
    fake::Si4703 chip  {Sdio, Sclk, Reset};
    adk::FmRadio radio {Sdio, Sclk, Reset};

    adk::setup ();
    radio.tune (1011);
    run (200, radio);

    sendText (chip, "Now playing: Blinky");
    run (400, radio);
    CHECK (std::string (radio.radioText ()) == "Now playing: Blinky");

    sendText (chip, "News", 1);
    run (400, radio);
    CHECK (std::string (radio.radioText ()) == "News");
}

TEST (fmRadioSetsTheVolumeAndStopFallsSilent)
{
    fake::Si4703 chip  {Sdio, Sclk, Reset};
    adk::FmRadio radio {Sdio, Sclk, Reset};

    adk::setup ();
    radio.setVolume (15);
    CHECK ((chip.registers[0x05] & 0x0F) == 15);
    radio.setVolume (40);
    CHECK (radio.volume () == 15);

    adk::stop ();
    CHECK (!(chip.registers[0x02] & Unmute));

    radio.setVolume (5);
    CHECK (chip.registers[0x02] & Unmute);
    CHECK ((chip.registers[0x05] & 0x0F) == 5);
    CHECK (!chip.drivenHigh);
}
