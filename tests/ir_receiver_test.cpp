#include "check.h"

#include <Arduino.h>
#include <adk/ir_receiver.h>

namespace {

    const adk::Pin Signal = 2;

    uint32_t frame (uint8_t address, uint8_t command)
    {
        return static_cast<uint32_t> (address) | static_cast<uint32_t> (address ^ 0xFF) << 8
             | static_cast<uint32_t> (command) << 16 | static_cast<uint32_t> (command ^ 0xFF) << 24;
    }

    // Plays a remote aimed at the receiver on one pin. The receiver's output
    // is low during a mark, while infrared arrives.
    struct Remote
    {
        explicit Remote (adk::Pin pin = Signal)
            : pin_ (pin)
        {
        }

        void pulse (unsigned long mark, unsigned long space) const
        {
            arduino::drive         (pin_, LOW);
            arduino::advanceMicros (mark);
            arduino::drive         (pin_, HIGH);
            arduino::advanceMicros (space);
        }

        // The 32 bits of a frame, least significant first, then its final
        // mark and a pause, every width scaled by percent.
        void data (uint32_t bits, unsigned long percent = 100) const
        {
            for (int bit = 0; bit < 32; ++bit)
            {
                unsigned long space = ((bits >> bit) & 1) ? 1687 : 562;
                pulse (562 * percent / 100, space * percent / 100);
            }

            pulse (562 * percent / 100, 40000);
        }

        void send (uint32_t bits, unsigned long percent = 100) const
        {
            pulse (9000 * percent / 100, 4500 * percent / 100);
            data  (bits, percent);
        }

        // A button pressed on the kit's remote, and one repeat of it held.
        void press (uint8_t command) const
        {
            send (frame (0x00, command));
        }

        void hold () const
        {
            pulse (9000, 2250);
            pulse (562, 96000);
        }

        adk::Pin pin_;
    };
}

TEST (irReceiverNeedsAnInterruptPin)
{
    adk::IrReceiver ir {22};

    adk::setup ();

    CHECK (check::halted.happened);
    CHECK (check::halted.fault == adk::Fault::NotInterrupt);
    CHECK (check::halted.pin == 22);
}

TEST (irReceiverSharingItsPinHalts)
{
    adk::IrReceiver ir  {Signal};
    adk::Led        led {Signal};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::PinInUse);
    CHECK (check::halted.pin == Signal);
}

TEST (irReceiverListensWithThePullUp)
{
    adk::IrReceiver ir {Signal};

    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (arduino::pin (Signal).mode == INPUT_PULLUP);
    CHECK (!ir.wasReceived ());
}

TEST (irReceiverDecodesAButtonForOneUpdate)
{
    adk::IrReceiver ir {Signal};
    Remote          remote;

    adk::setup ();
    adk::update (0);

    remote.press (adk::remote::power);
    CHECK (!ir.wasReceived ());

    adk::update (100);
    CHECK (ir.wasReceived ());
    CHECK (!ir.isRepeat ());
    CHECK (ir.command () == adk::remote::power);
    CHECK (ir.address () == 0x00);

    adk::update (101);
    CHECK (!ir.wasReceived ());
    CHECK (ir.command () == adk::remote::power);
}

TEST (irReceiverReportsRepeatsWhileAButtonIsHeld)
{
    adk::IrReceiver ir {Signal};
    Remote          remote;

    adk::setup ();
    remote.press (adk::remote::volumeUp);
    adk::update (0);

    remote.hold ();
    adk::update (1);
    CHECK (ir.wasReceived ());
    CHECK (ir.isRepeat ());
    CHECK (ir.command () == adk::remote::volumeUp);

    adk::update (2);
    CHECK (!ir.wasReceived ());
    CHECK (!ir.isRepeat ());

    remote.hold ();
    adk::update (3);
    CHECK (ir.wasReceived ());
    CHECK (ir.isRepeat ());
}

TEST (irReceiverKeepsAButtonThatARepeatFollowsBeforeTheUpdate)
{
    adk::IrReceiver ir {Signal};
    Remote          remote;

    adk::setup ();
    remote.press (adk::remote::digit7);
    remote.hold ();
    adk::update (0);

    CHECK (ir.wasReceived ());
    CHECK (!ir.isRepeat ());
    CHECK (ir.command () == adk::remote::digit7);
}

TEST (irReceiverIgnoresARepeatWithoutAButton)
{
    adk::IrReceiver ir {Signal};
    Remote          remote;

    adk::setup ();
    remote.hold ();
    adk::update (0);

    CHECK (!ir.wasReceived ());
}

TEST (irReceiverIgnoresARepeatLongAfterTheButton)
{
    adk::IrReceiver ir {Signal};
    Remote          remote;

    adk::setup ();
    remote.press (adk::remote::up);
    adk::update (0);

    arduino::advanceMicros (300000);
    remote.hold ();
    adk::update (1);

    CHECK (!ir.wasReceived ());
}

TEST (irReceiverRejectsACorruptedFrameAndItsRepeats)
{
    adk::IrReceiver ir {Signal};
    Remote          remote;

    adk::setup ();
    remote.press (adk::remote::digit1);
    adk::update (0);
    CHECK (ir.wasReceived ());

    remote.send (frame (0x00, adk::remote::digit2) ^ (1UL << 30));
    adk::update (1);
    CHECK (!ir.wasReceived ());
    CHECK (ir.command () == adk::remote::digit1);

    // The held button is no longer the one before, so its repeats mean nothing.
    remote.hold ();
    adk::update (2);
    CHECK (!ir.wasReceived ());
}

TEST (irReceiverDropsAFrameCutShort)
{
    adk::IrReceiver ir {Signal};
    Remote          remote;
    uint32_t        bits = frame (0x00, adk::remote::play);

    adk::setup ();
    remote.pulse (9000, 4500);

    for (int bit = 0; bit < 16; ++bit)
    {
        remote.pulse (562, ((bits >> bit) & 1) ? 1687 : 562);
    }

    arduino::advanceMicros (50000);
    remote.data (bits >> 16);
    adk::update (0);
    CHECK (!ir.wasReceived ());

    remote.press (adk::remote::play);
    adk::update (1);
    CHECK (ir.wasReceived ());
    CHECK (ir.command () == adk::remote::play);
}

TEST (irReceiverIgnoresNoiseBetweenButtons)
{
    adk::IrReceiver ir {Signal};
    Remote          remote;

    adk::setup ();

    for (unsigned long width = 50; width < 3000; width += 97)
    {
        remote.pulse (width, width * 3 / 2);
    }

    adk::update (0);
    CHECK (!ir.wasReceived ());

    remote.press (adk::remote::eq);
    adk::update (1);
    CHECK (ir.wasReceived ());
    CHECK (ir.command () == adk::remote::eq);
}

TEST (irReceiverReadsASixteenBitAddress)
{
    adk::IrReceiver ir {Signal};
    Remote          remote;

    adk::setup ();
    // Address 0x1234, then command 0x4A and its inverse.
    remote.send (0xB54A1234);
    adk::update (0);

    CHECK (ir.wasReceived ());
    CHECK (ir.address () == 0x1234);
    CHECK (ir.command () == 0x4A);
}

TEST (irReceiverAllowsAQuarterEitherWay)
{
    adk::IrReceiver ir {Signal};
    Remote          remote;

    adk::setup ();

    remote.send (frame (0x00, adk::remote::digit3), 76);
    adk::update (0);
    CHECK (ir.wasReceived ());

    remote.send (frame (0x00, adk::remote::digit4), 124);
    adk::update (1);
    CHECK (ir.wasReceived ());
    CHECK (ir.command () == adk::remote::digit4);

    remote.send (frame (0x00, adk::remote::digit5), 70);
    adk::update (2);
    CHECK (!ir.wasReceived ());

    remote.send (frame (0x00, adk::remote::digit5), 130);
    adk::update (3);
    CHECK (!ir.wasReceived ());
}

TEST (irReceiverLeaderMarkLimitsAreExact)
{
    adk::IrReceiver ir {Signal};
    Remote          remote;
    uint32_t        bits = frame (0x00, adk::remote::forward);

    adk::setup ();

    remote.pulse (6750, 4500);
    remote.data  (bits);
    adk::update (0);
    CHECK (ir.wasReceived ());

    remote.pulse (11250, 4500);
    remote.data  (bits);
    adk::update (1);
    CHECK (ir.wasReceived ());

    remote.pulse (6749, 4500);
    remote.data  (bits);
    adk::update (2);
    CHECK (!ir.wasReceived ());

    remote.pulse (11251, 4500);
    remote.data  (bits);
    adk::update (3);
    CHECK (!ir.wasReceived ());
}

TEST (irReceiversOnTwoInterruptsDecodeApart)
{
    adk::IrReceiver left    {2};
    adk::IrReceiver right   {21};
    Remote          toLeft  {2};
    Remote          toRight {21};

    adk::setup ();

    toLeft.press (adk::remote::back);
    adk::update (0);
    CHECK (left.wasReceived ());
    CHECK (!right.wasReceived ());

    toRight.press (adk::remote::forward);
    adk::update (1);
    CHECK (!left.wasReceived ());
    CHECK (right.wasReceived ());
    CHECK (right.command () == adk::remote::forward);
    CHECK (left.command () == adk::remote::back);
}

TEST (remoteDigitButtonsKnowTheirNumbers)
{
    static_assert (adk::remote::digitOf (adk::remote::digit0) == 0);
    static_assert (adk::remote::digitOf (adk::remote::digit7) == 7);

    CHECK (adk::remote::digitOf (adk::remote::digit9) == 9);
    CHECK (adk::remote::digitOf (adk::remote::power) == -1);
}
