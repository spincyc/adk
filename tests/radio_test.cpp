#include "check.h"

#include <Arduino.h>
#include <string>

namespace {

    const adk::Pin Transmit = 46;
    const adk::Pin Receive  = 43;

    // The air between the modules: the receiver hears what the transmitter
    // sends, and, when noisy, random bits while nothing is sending, as the
    // real receiver turns its gain up until it hears a signal.
    struct Air
    {
        explicit Air (const adk::RadioTransmitter* transmitter = nullptr)
        {
            arduino::onDigitalRead = [this, transmitter] (uint8_t pin)
            {
                if (pin != Receive)
                {
                    return static_cast<int> (arduino::pin (pin).input);
                }

                if (noisy && !(transmitter && transmitter->isSending ()))
                {
                    seed = seed * 1103515245 + 12345;
                    return static_cast<int> ((seed >> 16) & 1);
                }

                return static_cast<int> (arduino::pin (Transmit).output ^ flip);
            };
        }

        // Timer 1's interrupt, count times: 16,000 a second.
        void run (unsigned count)
        {
            for (unsigned tick = 0; tick < count; ++tick)
            {
                TIMER1_COMPA_vect ();
            }
        }

        // Until the transmitter is quiet, and a little longer.
        void runUntilSent (const adk::RadioTransmitter& radio)
        {
            while (radio.isSending ())
            {
                run (1);
            }

            run (64);
        }

        bool     noisy = false;
        uint8_t  flip  = 0;
        uint32_t seed  = 1;
    };

    // What the transmitter's pin holds at the middle of each bit.
    std::string bitsSent (adk::RadioTransmitter& radio, Air& air)
    {
        std::string bits;

        while (radio.isSending ())
        {
            air.run (4);
            bits += arduino::pin (Transmit).output ? '1' : '0';
            air.run (4);
        }

        return bits;
    }

    std::string symbolBits (uint8_t symbol)
    {
        std::string bits;

        for (int bit = 0; bit < 6; ++bit)
        {
            bits += ((symbol >> bit) & 1) ? '1' : '0';
        }

        return bits;
    }
}

TEST (radioClaimsItsPinsAndStartsTimerOne)
{
    adk::RadioTransmitter transmitter {Transmit};
    adk::RadioReceiver    receiver    {Receive};

    adk::setup ();
    CHECK (adk::fault () == adk::Fault::None);
    CHECK (arduino::pin (Transmit).mode == OUTPUT);
    CHECK (arduino::pin (Transmit).output == LOW);
    CHECK (arduino::pin (Receive).mode == INPUT);
    CHECK (TCCR1B == (_BV (WGM12) | _BV (CS11)));
    CHECK (OCR1A == 124);
    CHECK (TIMSK1 & _BV (OCIE1A));
}

TEST (radioSendsWhatRadioHeadSends)
{
    adk::RadioTransmitter transmitter {Transmit};
    Air                   air;

    adk::setup ();
    CHECK (transmitter.send ("Hi"));

    // Six training symbols, the start symbol, then the length (9: two
    // letters and seven more bytes) as two symbols, high four bits first.
    std::string expected;

    for (int symbol : {0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x38, 0x2C, 0x0D, 0x25})
    {
        expected += symbolBits (static_cast<uint8_t> (symbol));
    }

    // 26 symbols, then the line falls quiet.
    std::string sent = bitsSent (transmitter, air);
    CHECK (sent.substr (0, expected.size ()) == expected);
    CHECK (sent.size () == 6 * (8 + 2 * 9u) + 1);
    CHECK (sent.back () == '0');
}

TEST (radioMessageArrivesWholeForOneUpdate)
{
    adk::RadioTransmitter transmitter {Transmit};
    adk::RadioReceiver    receiver    {Receive};
    Air                   air;

    adk::setup ();
    CHECK (transmitter.send ("Hello"));
    CHECK (transmitter.isSending ());
    air.runUntilSent (transmitter);

    adk::update (0);
    CHECK (receiver.wasReceived ());
    CHECK (std::string (receiver.text ()) == "Hello");
    CHECK (receiver.length () == 5);

    adk::update (1);
    CHECK (!receiver.wasReceived ());
    CHECK (std::string (receiver.text ()) == "Hello");
}

TEST (radioFindsAMessageInNoise)
{
    adk::RadioTransmitter transmitter {Transmit};
    adk::RadioReceiver    receiver    {Receive};
    Air                   air {&transmitter};

    adk::setup ();
    air.noisy = true;
    air.run (8000);
    adk::update (0);
    CHECK (!receiver.wasReceived ());

    CHECK (transmitter.send ("Dinner's ready!"));
    air.runUntilSent (transmitter);
    adk::update (1);
    CHECK (receiver.wasReceived ());
    CHECK (std::string (receiver.text ()) == "Dinner's ready!");
}

TEST (radioRejectsAMessageNoiseGotInto)
{
    adk::RadioTransmitter transmitter {Transmit};
    adk::RadioReceiver    receiver    {Receive};
    Air                   air;

    adk::setup ();
    CHECK (transmitter.send ("Hello"));

    // A burst that inverts two bits in the middle of the message.
    air.run (6 * 8 * 16);
    air.flip = 1;
    air.run (16);
    air.flip = 0;
    air.runUntilSent (transmitter);

    adk::update (0);
    CHECK (!receiver.wasReceived ());

    // The next message gets through.
    CHECK (transmitter.send ("Again"));
    air.runUntilSent (transmitter);
    adk::update (1);
    CHECK (receiver.wasReceived ());
    CHECK (std::string (receiver.text ()) == "Again");
}

TEST (radioCarriesBytesAndTheLongestMessage)
{
    adk::RadioTransmitter transmitter {Transmit};
    adk::RadioReceiver    receiver    {Receive};
    Air                   air;

    adk::setup ();
    const uint8_t bytes [] = {0, 1, 2, 255};
    CHECK (transmitter.send (bytes, 4));
    air.runUntilSent (transmitter);
    adk::update (0);
    CHECK (receiver.wasReceived ());
    CHECK (receiver.length () == 4);
    CHECK (memcmp (receiver.text (), bytes, 4) == 0);

    std::string longest (adk::RadioTransmitter::MaxLength, 'x');
    CHECK (transmitter.send (longest.c_str ()));
    air.runUntilSent (transmitter);
    adk::update (1);
    CHECK (receiver.wasReceived ());
    CHECK (std::string (receiver.text ()) == longest);
}

TEST (radioRefusesATooLongOrOverlappingMessage)
{
    adk::RadioTransmitter transmitter {Transmit};
    Air                   air;

    adk::setup ();
    std::string tooLong (adk::RadioTransmitter::MaxLength + 1, 'x');
    CHECK (!transmitter.send (tooLong.c_str ()));
    CHECK (!transmitter.isSending ());

    CHECK (transmitter.send ("One"));
    CHECK (!transmitter.send ("Two"));
    air.runUntilSent (transmitter);
    CHECK (transmitter.send ("Two"));
}

TEST (radioStopFallsSilent)
{
    adk::RadioTransmitter transmitter {Transmit};
    Air                   air;

    adk::setup ();
    CHECK (transmitter.send ("Hello"));
    air.run (100);
    adk::stop ();
    CHECK (!transmitter.isSending ());
    CHECK (arduino::pin (Transmit).output == LOW);
    air.run (100);
    CHECK (arduino::pin (Transmit).output == LOW);
}

TEST (radioAllowsOneTransmitterAndLeavesPinsElevenAndTwelveUnable)
{
    {
        adk::RadioTransmitter first  {Transmit};
        adk::RadioTransmitter second {47};

        adk::setup ();
        CHECK (check::halted.happened);
        CHECK (check::halted.fault == adk::Fault::TimerInUse);
        CHECK (check::halted.pin == 47);
    }

    check::halted = {};
    adk::RadioReceiver receiver {Receive};
    adk::PwmOutput     lamp     {11};

    adk::setup ();
    CHECK (check::halted.happened);
    CHECK (check::halted.fault == adk::Fault::TimerInUse);
}
