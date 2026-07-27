#include "shift_register.h"

#include <Arduino.h>

namespace adk {

    ShiftRegisterOutput::ShiftRegisterOutput (
        ResourceRegistry&        resources,
        const ShiftRegisterPins& pins,
        uint8_t                  inactiveValue) noexcept
        : pins_          (pins)
        , data_          (resources, pins.data)
        , clock_         (resources, pins.clock)
        , latch_         (resources, pins.latch)
        , value_         (inactiveValue)
        , inactiveValue_ (inactiveValue)
    {
    }

    ShiftRegisterOutput::~ShiftRegisterOutput () noexcept
    {
        shutdown ();
    }

    Status ShiftRegisterOutput::initialize () noexcept
    {
        if (initialized ())
        {
            return Status::Ok;
        }

        if (pins_.data >= NUM_DIGITAL_PINS ||
            pins_.clock >= NUM_DIGITAL_PINS ||
            pins_.latch >= NUM_DIGITAL_PINS)
        {
            return Status::InvalidPin;
        }

        if (pins_.data == pins_.clock ||
            pins_.data == pins_.latch ||
            pins_.clock == pins_.latch)
        {
            return Status::InvalidArgument;
        }

        Status status = data_.initialize ();

        if (status != Status::Ok)
        {
            return status;
        }

        status = clock_.initialize ();

        if (status != Status::Ok)
        {
            data_.shutdown ();
            return status;
        }

        status = latch_.initialize ();

        if (status != Status::Ok)
        {
            clock_.shutdown ();
            data_ .shutdown ();
            return status;
        }

        status = show (inactiveValue_);

        if (status != Status::Ok)
        {
            shutdown ();
            return status;
        }

        return Status::Ok;
    }

    void ShiftRegisterOutput::shutdown () noexcept
    {
        if (!initialized ())
        {
            return;
        }

        show (inactiveValue_);

        latch_.shutdown ();
        clock_.shutdown ();
        data_ .shutdown ();

        value_ = inactiveValue_;
    }

    Status ShiftRegisterOutput::show (uint8_t value) noexcept
    {
        if (!initialized ())
        {
            return Status::NotInitialized;
        }

        latch_.write (Level::Low);

        for (uint8_t mask = 0x80U; mask != 0; mask >>= 1U)
        {
            data_ .write ((value & mask) == 0 ? Level::Low : Level::High);
            clock_.write (Level::High);
            clock_.write (Level::Low);
        }

        latch_.write (Level::High);
        latch_.write (Level::Low);

        value_ = value;
        return Status::Ok;
    }

    Status ShiftRegisterOutput::clear () noexcept
    {
        return show (0);
    }

    uint8_t ShiftRegisterOutput::value () const noexcept
    {
        return value_;
    }

    uint8_t ShiftRegisterOutput::inactiveValue () const noexcept
    {
        return inactiveValue_;
    }

    const ShiftRegisterPins& ShiftRegisterOutput::pins () const noexcept
    {
        return pins_;
    }

    bool ShiftRegisterOutput::initialized () const noexcept
    {
        return data_.initialized () && clock_.initialized () &&
               latch_.initialized ();
    }
}
