#include "rotary_encoder.h"

#include <Arduino.h>

namespace adk {

    namespace {

        // The step between two readings of the contacts, CLK as the high bit
        // and DT as the low one, indexed by previous << 2 | current. Turning
        // clockwise CLK changes first: 11, 01, 00, 10, 11. When both change
        // at once a step was missed, and which way it went is unknown.
        constexpr int8_t Steps [16] PROGMEM = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

        // Both contacts open, as a KY-040's are in every detent.
        constexpr uint8_t Resting = 0b11;
    }

    RotaryEncoder::RotaryEncoder (Pin clk, Pin dt, uint8_t stepsPerDetent)
        : position_       (0)
        , clk_            (clk)
        , dt_             (dt)
        , stepsPerDetent_ (stepsPerDetent > 0 ? stepsPerDetent : 1)
        , contacts_       (0)
        , steps_          (0)
        , turned_         (0)
    {
    }

    void RotaryEncoder::setup ()
    {
        // The module has its own pull-ups; the internal ones cover a bare
        // encoder wired straight to GND.
        if (claimInput (clk_, true) && claimInput (dt_, true))
        {
            contacts_ = read ();
            steps_    = 0;
        }
    }

    void RotaryEncoder::update (Millis)
    {
        uint8_t contacts = read ();
        uint8_t index    = static_cast<uint8_t> ((contacts_ << 2) | contacts);
        int8_t  step     = static_cast<int8_t> (pgm_read_byte (&Steps[index]));

        contacts_ = contacts;
        steps_    = static_cast<int8_t> (steps_ + step);
        turned_   = 0;

        // Detents count only where the knob comes to rest, as the steps
        // taken since the last rest rounded to whole detents. A missed step
        // then costs nothing, a bounce cancels out, and the count can never
        // drift out of step with the clicks.
        if (isResting (contacts))
        {
            int8_t half = static_cast<int8_t> (stepsPerDetent_ / 2);
            int8_t away = static_cast<int8_t> (steps_ < 0 ? steps_ - half : steps_ + half);

            turned_    = static_cast<int8_t> (away / stepsPerDetent_);
            position_ += turned_;
            steps_     = 0;
        }
    }

    long RotaryEncoder::position () const
    {
        return position_;
    }

    int8_t RotaryEncoder::turned () const
    {
        return turned_;
    }

    void RotaryEncoder::reset (long position)
    {
        position_ = position;
    }

    bool RotaryEncoder::isResting (uint8_t contacts) const
    {
        // Four steps apart, detents rest with both contacts open; two steps
        // apart, with both open or both closed; one step apart, anywhere.
        return contacts == Resting || stepsPerDetent_ == 1
            || (stepsPerDetent_ == 2 && contacts == 0);
    }

    uint8_t RotaryEncoder::read () const
    {
        return static_cast<uint8_t> ((digitalRead (clk_) == HIGH ? 2 : 0) |
                                     (digitalRead (dt_) == HIGH ? 1 : 0));
    }
}
