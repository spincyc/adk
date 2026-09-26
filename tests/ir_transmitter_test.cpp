#include "check.h"

#include <Arduino.h>
#include <vector>

namespace {

    // How long the carrier was on, then off, in turn, from TCCR3A's writes.
    std::vector<unsigned long> widths (uint8_t output)
    {
        std::vector<unsigned long> result;
        bool                       on   = false;
        unsigned long              from = 0;

        for (auto [time, value] : TCCR3A.history)
        {
            bool now = value & output;

            if (now != on)
            {
                if (on || !result.empty () || from != 0)
                {
                    result.push_back (time - from);
                }

                on   = now;
                from = time;
            }
        }

        return result;
    }

    bool near (unsigned long width, unsigned long expected)
    {
        return width + 20 >= expected && width <= expected + 20;
    }
}

TEST (irTransmitterMakesA38KilohertzCarrierOnTimerThree)
{
    adk::IrTransmitter ir {3};

    adk::setup ();
    CHECK (adk::fault () == adk::Fault::None);
    CHECK (arduino::pin (3).mode == OUTPUT);
    CHECK (arduino::pin (3).output == LOW);
    CHECK (TCCR3B == (_BV (WGM33) | _BV (WGM32) | _BV (CS30)));
    CHECK (ICR3 == 420);
    CHECK (OCR3C == 141);
    CHECK (!(TCCR3A & _BV (COM3C1)));
}

TEST (irTransmitterOnlyWorksOnTimerThreesPins)
{
    adk::IrTransmitter ir {4};

    adk::setup ();
    CHECK (check::halted.happened);
    CHECK (check::halted.fault == adk::Fault::NotInfrared);
    CHECK (check::halted.pin == 4);
}

TEST (irTransmitterSendsAnNecFrame)
{
    adk::IrTransmitter ir {3};

    adk::setup ();
    arduino::setCallCost (1);
    TCCR3A.history.clear ();
    ir.send (adk::remote::power);

    std::vector<unsigned long> sent = widths (_BV (COM3C1));
    CHECK (sent.size () == 67);
    CHECK (near (sent[0], 9000));
    CHECK (near (sent[1], 4500));

    // Address 0 and 255, then 0x45 and its inverse, least significant first.
    uint32_t bits = 0;

    for (int bit = 0; bit < 32; ++bit)
    {
        CHECK (near (sent[2 + 2 * bit], 562));
        bits |= static_cast<uint32_t> (sent[3 + 2 * bit] > 1000) << bit;
    }

    CHECK (bits == 0xBA45FF00);
    CHECK (near (sent[66], 562));
    CHECK (!(TCCR3A & _BV (COM3C1)));
}

TEST (irTransmitterIsUnderstoodByAnIrReceiver)
{
    adk::IrTransmitter transmitter {5};
    adk::IrReceiver    receiver    {2};

    adk::setup ();
    arduino::setCallCost (1);
    TCCR3A.history.clear ();
    transmitter.send (adk::remote::volumeUp, 0x1234);
    std::vector<unsigned long> sent = widths (_BV (COM3A1));

    // Play the light into the receiver: its output is low while lit.
    arduino::setCallCost (0);

    for (size_t index = 0; index < sent.size (); ++index)
    {
        arduino::drive (2, index % 2 == 0 ? LOW : HIGH);
        arduino::advanceMicros (sent[index]);
    }

    arduino::drive (2, HIGH);
    arduino::advanceMicros (40000);
    adk::update (100);
    CHECK (receiver.wasReceived ());
    CHECK (receiver.command () == adk::remote::volumeUp);
    CHECK (receiver.address () == 0x1234);
}

TEST (irTransmitterRepeatsAndFallsDark)
{
    adk::IrTransmitter ir {2};

    adk::setup ();
    arduino::setCallCost (1);
    TCCR3A.history.clear ();
    ir.repeat ();

    std::vector<unsigned long> sent = widths (_BV (COM3B1));
    CHECK (sent.size () == 3);
    CHECK (near (sent[1], 2250));

    TCCR3A |= _BV (COM3B1);
    adk::stop ();
    CHECK (!(TCCR3A & _BV (COM3B1)));
}

TEST (soundSensorMeasuresHowFarTheSignalSwings)
{
    adk::SoundSensor sound {A5};

    adk::setup ();
    CHECK (adk::isClaimed (A5));

    // Quiet: the signal sits at the middle.
    arduino::pin (A5).analog = 512;
    int levels = 0;

    for (adk::Millis now = 0; now <= 50; now += 5)
    {
        adk::update (now);
        levels += sound.measured () ? 1 : 0;
    }

    CHECK (levels == 1);
    CHECK (sound.level () == 0);

    // A clap: it swings from 300 to 700 and back.
    for (adk::Millis now = 55; now <= 100; now += 5)
    {
        arduino::pin (A5).analog = (now / 5) % 2 ? 300 : 700;
        adk::update (now);
    }

    CHECK (sound.measured ());
    CHECK (sound.level () == 400);
    adk::update (105);
    CHECK (!sound.measured ());
}
