#include "ir_receiver.h"

#include <Arduino.h>

namespace adk {

    namespace {

        // NEC timing in microseconds. Every code starts with a 9 ms leader
        // mark. In a frame a 4.5 ms space follows, then 32 bits, each a short
        // mark and a short space for a 0 or a long one for a 1, then a final
        // short mark. In a repeat a 2.25 ms space and the final mark follow.
        const uint32_t LeaderMark  = 9000;
        const uint32_t FrameSpace  = 4500;
        const uint32_t RepeatSpace = 2250;
        const uint32_t BitMark     = 562;
        const uint32_t ZeroSpace   = 562;
        const uint32_t OneSpace    = 1687;

        // A held button repeats every 108 ms. A repeat much later than the
        // code before it may follow a press whose frame was missed, so it is
        // ignored rather than taken for the button before.
        const uint32_t RepeatWindow = 250000;

        enum Phase : uint8_t
        {
            Waiting,    // for a leader mark
            Leading,    // after a leader mark; its space says frame or repeat
            Framing,    // counting bits
            Repeating   // for a repeat's final mark
        };

        enum Heard : uint8_t
        {
            Nothing,
            Frame,
            Repeat
        };

        // The receiver on each of the six interrupts.
        IrReceiver* receivers [6];

        // Receivers stretch and shrink pulses, so allow a quarter either way.
        bool near (uint32_t width, uint32_t expected)
        {
            return width >= expected - expected / 4 && width <= expected + expected / 4;
        }
    }

    template <uint8_t Interrupt>
    void IrReceiver::onEdge ()
    {
        receivers[Interrupt]->edge ();
    }

    IrReceiver::IrReceiver (Pin pin)
        : lastEdge_     (0)
        , heldAt_       (0)
        , bits_         (0)
        , phase_        (Waiting)
        , count_        (0)
        , held_         (false)
        , heardAddress_ (0)
        , heardCommand_ (0)
        , heard_        (Nothing)
        , address_      (0)
        , command_      (0)
        , pin_          (pin)
        , received_     (false)
        , repeat_       (false)
    {
    }

    void IrReceiver::setup ()
    {
        if (!claimInterrupt (pin_, true))
        {
            return;
        }

        // A handler takes no arguments, so each interrupt has its own, which
        // finds its receiver in the table.
        uint8_t interrupt   = static_cast<uint8_t> (digitalPinToInterrupt (pin_));
        void  (*handler) () = nullptr;

        switch (interrupt)
        {
            case 0:  handler = onEdge<0>; break;
            case 1:  handler = onEdge<1>; break;
            case 2:  handler = onEdge<2>; break;
            case 3:  handler = onEdge<3>; break;
            case 4:  handler = onEdge<4>; break;
            case 5:  handler = onEdge<5>; break;
            default: return;
        }

        receivers[interrupt] = this;
        attachInterrupt (interrupt, handler, CHANGE);
    }

    bool IrReceiver::wasReceived () const
    {
        return received_;
    }

    bool IrReceiver::isRepeat () const
    {
        return repeat_;
    }

    uint8_t IrReceiver::command () const
    {
        return command_;
    }

    uint16_t IrReceiver::address () const
    {
        return address_;
    }

    void IrReceiver::update (Millis)
    {
        // Copy the code out with interrupts off, so it cannot change halfway.
        noInterrupts ();
        uint8_t  heard  = heard_;
        uint16_t sender = heardAddress_;
        uint8_t  button = heardCommand_;
        heard_          = Nothing;
        interrupts ();

        received_ = heard != Nothing;
        repeat_   = heard == Repeat;

        if (heard == Frame)
        {
            address_ = sender;
            command_ = button;
        }
    }

    void IrReceiver::edge ()
    {
        uint32_t now   = static_cast<uint32_t> (micros ());
        uint32_t width = now - lastEdge_;
        lastEdge_      = now;

        // The output is low during a mark, so a rising edge ends a mark and a
        // falling edge ends a space.
        bool mark = digitalRead (pin_) == HIGH;

        // A leader mark starts a new code wherever the decoder was, so a code
        // cut short never spoils the next.
        if (mark && near (width, LeaderMark))
        {
            phase_ = Leading;
            return;
        }

        switch (phase_)
        {
            case Leading:
                if (!mark && near (width, FrameSpace))
                {
                    phase_ = Framing;
                    bits_  = 0;
                    count_ = 0;
                    held_  = false;
                }
                else
                {
                    phase_ = (!mark && near (width, RepeatSpace)) ? Repeating : Waiting;
                }
                break;

            case Framing:
                if (!mark)
                {
                    space (width);
                }
                else if (!near (width, BitMark))
                {
                    phase_ = Waiting;
                }
                else if (count_ == 32)
                {
                    finish (now);
                    phase_ = Waiting;
                }
                break;

            case Repeating:
                if (mark && near (width, BitMark) && held_ && now - heldAt_ < RepeatWindow)
                {
                    heldAt_ = now;

                    // A frame not yet collected stays a frame, so it is not lost.
                    if (heard_ == Nothing)
                    {
                        heard_ = Repeat;
                    }
                }
                phase_ = Waiting;
                break;

            default:
                break;
        }
    }

    void IrReceiver::space (uint32_t width)
    {
        bool one = near (width, OneSpace);

        if (count_ == 32 || !(one || near (width, ZeroSpace)))
        {
            phase_ = Waiting;
            return;
        }

        // Bits arrive least significant first.
        bits_ |= static_cast<uint32_t> (one) << count_;
        ++count_;
    }

    void IrReceiver::finish (uint32_t now)
    {
        // The address, the address inverted, the command and the command
        // inverted. A remote with a 16-bit address sends that instead of the
        // first two, so only the command is checked.
        uint8_t low      = static_cast<uint8_t> (bits_);
        uint8_t high     = static_cast<uint8_t> (bits_ >> 8);
        uint8_t button   = static_cast<uint8_t> (bits_ >> 16);
        uint8_t inverted = static_cast<uint8_t> (bits_ >> 24);

        if (static_cast<uint8_t> (~button) != inverted)
        {
            return;
        }

        heardAddress_ = (static_cast<uint8_t> (~low) == high) ? low : static_cast<uint16_t> (bits_);
        heardCommand_ = button;
        heard_        = Frame;
        held_         = true;
        heldAt_       = now;
    }
}
