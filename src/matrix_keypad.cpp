#include "matrix_keypad.h"

#include <Arduino.h>

namespace adk {

    namespace {

        struct ArduinoMatrixKeypadIo final : MatrixKeypadIo
        {
            Status setMode (PinId pin, MatrixKeypadPinMode mode) noexcept override
            {
                uint8_t arduinoMode = INPUT;

                switch (mode)
                {
                    case MatrixKeypadPinMode::HighImpedance: arduinoMode = INPUT;        break;
                    case MatrixKeypadPinMode::InputPullUp:   arduinoMode = INPUT_PULLUP; break;
                    case MatrixKeypadPinMode::Output:        arduinoMode = OUTPUT;       break;
                }

                pinMode (pin, arduinoMode);
                return Status::Ok;
            }

            Status write (PinId pin, bool high) noexcept override
            {
                digitalWrite (pin, high ? HIGH : LOW);
                return Status::Ok;
            }

            Status read (PinId pin, bool& high) noexcept override
            {
                high = digitalRead (pin) != LOW;
                return Status::Ok;
            }
        };

        ArduinoMatrixKeypadIo arduinoIo;
    }

    MatrixKeypadIo::~MatrixKeypadIo () noexcept = default;

    MatrixKeypad::MatrixKeypad (ResourceRegistry&       resources,
                                const MatrixKeypadPins& pins,
                                const KeypadConfig&     config,
                                MatrixKeypadIo*         io) noexcept
        : resources_   (&resources)
        , pins_        (pins)
        , claims_      ()
        , keypad_      (config)
        , io_          (io == nullptr ? &arduinoIo : io)
        , initialized_ (false)
    {
    }

    MatrixKeypad::~MatrixKeypad () noexcept
    {
        shutdown ();
    }

    Status MatrixKeypad::initialize () noexcept
    {
        if (initialized_)
        {
            return Status::Ok;
        }

        if (!pinsValid ())
        {
            return Status::InvalidPin;
        }

        Status status = keypad_.initialize ();

        if (status != Status::Ok)
        {
            return status;
        }

        const PinId pins[7] =
        {
            pins_.row0,
            pins_.row1,
            pins_.row2,
            pins_.row3,
            pins_.column0,
            pins_.column1,
            pins_.column2
        };

        for (uint8_t index = 0; index < 7; ++index)
        {
            status = resources_->claim (
                {ResourceKind::Pin, pins[index]}, claims_[index]);

            if (status != Status::Ok)
            {
                releaseClaims    ();
                keypad_.shutdown ();
                return status;
            }
        }

        const PinId rows[4] = {pins_.row0, pins_.row1, pins_.row2, pins_.row3};

        for (uint8_t row = 0; row < 4; ++row)
        {
            status = io_->setMode (rows[row], MatrixKeypadPinMode::HighImpedance);

            if (status != Status::Ok)
            {
                makePinsSafe     ();
                releaseClaims    ();
                keypad_.shutdown ();
                return status;
            }
        }

        const PinId columns[3] = {pins_.column0, pins_.column1, pins_.column2};

        for (uint8_t column = 0; column < 3; ++column)
        {
            status = io_->setMode (columns[column], MatrixKeypadPinMode::InputPullUp);

            if (status != Status::Ok)
            {
                makePinsSafe     ();
                releaseClaims    ();
                keypad_.shutdown ();
                return status;
            }
        }

        initialized_ = true;
        return Status::Ok;
    }

    void MatrixKeypad::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        makePinsSafe     ();
        keypad_.shutdown ();
        releaseClaims    ();
        initialized_ = false;
    }

    Status MatrixKeypad::update (TimePoint now) noexcept
    {
        if (!initialized_)
        {
            return Status::NotInitialized;
        }

        uint16_t pressedMask = 0;
        const Status scanStatus = scan (pressedMask);
        const Status keypadStatus =
            keypad_.update (now, {pressedMask, scanStatus == Status::Ok});

        return scanStatus == Status::Ok ? keypadStatus : scanStatus;
    }

    MatrixKeypadPins MatrixKeypad::pins () const noexcept
    {
        return pins_;
    }

    KeypadSnapshot MatrixKeypad::snapshot () const noexcept
    {
        return keypad_.snapshot ();
    }

    bool MatrixKeypad::initialized () const noexcept
    {
        return initialized_;
    }

    bool MatrixKeypad::pinsValid () const noexcept
    {
        const PinId pins[7] =
        {
            pins_.row0,
            pins_.row1,
            pins_.row2,
            pins_.row3,
            pins_.column0,
            pins_.column1,
            pins_.column2
        };

        for (uint8_t index = 0; index < 7; ++index)
        {
            if (pins[index] >= NUM_DIGITAL_PINS)
            {
                return false;
            }

            for (uint8_t prior = 0; prior < index; ++prior)
            {
                if (pins[index] == pins[prior])
                {
                    return false;
                }
            }
        }

        return true;
    }

    void MatrixKeypad::makeRowsSafe () noexcept
    {
        const PinId rows[4] = {pins_.row3, pins_.row2, pins_.row1, pins_.row0};

        for (uint8_t row = 0; row < 4; ++row)
        {
            static_cast<void> (
                io_->setMode (rows[row], MatrixKeypadPinMode::HighImpedance));
        }
    }

    void MatrixKeypad::makePinsSafe () noexcept
    {
        const PinId columns[3] = {pins_.column2, pins_.column1, pins_.column0};

        for (uint8_t column = 0; column < 3; ++column)
        {
            static_cast<void> (
                io_->setMode (columns[column], MatrixKeypadPinMode::HighImpedance));
        }

        makeRowsSafe ();
    }

    void MatrixKeypad::releaseClaims () noexcept
    {
        for (uint8_t index = 7; index != 0; --index)
        {
            claims_[index - 1U].release ();
        }
    }

    Status MatrixKeypad::scan (uint16_t& pressedMask) noexcept
    {
        const PinId rows[4] =
        {
            pins_.row0,
            pins_.row1,
            pins_.row2,
            pins_.row3
        };
        const PinId columns[3] =
        {
            pins_.column0,
            pins_.column1,
            pins_.column2
        };
        pressedMask = 0;

        for (uint8_t row = 0; row < 4; ++row)
        {
            Status status = io_->write (rows[row], false);

            if (status == Status::Ok)
            {
                status = io_->setMode (rows[row], MatrixKeypadPinMode::Output);
            }

            if (status != Status::Ok)
            {
                makeRowsSafe ();
                pressedMask = 0;
                return status;
            }

            for (uint8_t column = 0; column < 3; ++column)
            {
                bool high = true;

                status = io_->read (columns[column], high);

                if (status != Status::Ok)
                {
                    makeRowsSafe ();
                    pressedMask = 0;
                    return status;
                }

                if (!high)
                {
                    const uint8_t key = static_cast<uint8_t> (row * 3U + column);

                    pressedMask |= static_cast<uint16_t> (1U << key);
                }
            }

            status = io_->setMode (rows[row], MatrixKeypadPinMode::HighImpedance);

            if (status != Status::Ok)
            {
                makeRowsSafe ();
                pressedMask = 0;
                return status;
            }
        }

        return Status::Ok;
    }
}
