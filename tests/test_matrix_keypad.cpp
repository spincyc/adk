#include <matrix_keypad.h>

#include <Arduino.h>

#include <cassert>
#include <type_traits>

namespace {

    namespace fake = adk::test::arduino;

    constexpr adk::MatrixKeypadPins pins = {22, 23, 24, 25, 26, 27, 28};

    struct RecordingMatrixIo : adk::MatrixKeypadIo
    {
        RecordingMatrixIo () noexcept
            : pressedMask (0)
            , operation   (0)
            , failAt      (0)
            , activeRow   (4)
            , modes       ()
        {
        }

        adk::Status setMode (adk::PinId pin, adk::MatrixKeypadPinMode mode) noexcept override
        {
            if (fail ())
            {
                return adk::Status::HardwareFailure;
            }

            modes[pin] = mode;

            if (pin >= pins.row0 && pin <= pins.row3)
            {
                const uint8_t row = static_cast<uint8_t> (pin - pins.row0);

                if (mode == adk::MatrixKeypadPinMode::Output)
                {
                    activeRow = row;
                }
                else if (activeRow == row)
                {
                    activeRow = 4;
                }
            }

            return adk::Status::Ok;
        }

        adk::Status write (adk::PinId, bool high) noexcept override
        {
            if (fail ())
            {
                return adk::Status::HardwareFailure;
            }

            return high ? adk::Status::HardwareFailure : adk::Status::Ok;
        }

        adk::Status read (adk::PinId pin, bool& high) noexcept override
        {
            if (fail ())
            {
                return adk::Status::HardwareFailure;
            }

            if (activeRow >= 4 || pin < pins.column0 || pin > pins.column2)
            {
                return adk::Status::HardwareFailure;
            }

            const uint8_t column = static_cast<uint8_t> (pin - pins.column0);
            const uint8_t key    = static_cast<uint8_t> (activeRow * 3U + column);

            high = (pressedMask & static_cast<uint16_t> (1U << key)) == 0;
            return adk::Status::Ok;
        }

        bool fail () noexcept
        {
            ++operation;
            return failAt != 0 && operation == failAt;
        }

        uint16_t                  pressedMask;
        uint16_t                  operation;
        uint16_t                  failAt;
        uint8_t                   activeRow;
        adk::MatrixKeypadPinMode  modes[NUM_DIGITAL_PINS];
    };

    void lifecycleAndSafeScan ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;
        adk::MatrixKeypad     keypad (resources, pins);

        assert (!keypad.initialized ());
        assert (keypad.update (adk::TimePoint (0)) == adk::Status::NotInitialized);
        assert (fake::trace ().empty ());
        assert (keypad.initialize () == adk::Status::Ok);
        assert (keypad.initialize () == adk::Status::Ok);
        assert (keypad.initialized ());

        for (uint8_t row = 22; row <= 25; ++row)
        {
            assert (fake::mode (row) == INPUT);
        }

        for (uint8_t column = 26; column <= 28; ++column)
        {
            assert (fake::mode (column) == INPUT_PULLUP);
        }

        fake::setDigitalInput (26, HIGH);
        fake::setDigitalInput (27, HIGH);
        fake::setDigitalInput (28, HIGH);
        fake::clearTrace      ();

        assert (keypad.update (adk::TimePoint (0)) == adk::Status::Ok);
        assert (keypad.snapshot ().rawMask == 0);
        assert (fake::trace ().size () == 24);

        for (uint8_t row = 0; row < 4; ++row)
        {
            const std::size_t offset = static_cast<std::size_t> (row) * 6U;

            assert (fake::trace ()[offset].kind ==
                    fake::OperationKind::DigitalWrite);
            assert (fake::trace ()[offset].pin == static_cast<uint8_t> (22 + row));
            assert (fake::trace ()[offset].value == LOW);
            assert (fake::trace ()[offset + 1U].kind ==
                    fake::OperationKind::PinMode);
            assert (fake::trace ()[offset + 1U].value == OUTPUT);
            assert (fake::trace ()[offset + 5U].kind ==
                    fake::OperationKind::PinMode);
            assert (fake::trace ()[offset + 5U].value == INPUT);
            assert (fake::mode (static_cast<uint8_t> (22 + row)) == INPUT);
        }

        fake::clearTrace ();
        keypad.shutdown  ();

        assert (!keypad.initialized ());
        assert (fake::trace ().size () == 7);

        for (uint8_t pin = 22; pin <= 28; ++pin)
        {
            assert (fake::mode (pin) == INPUT);
            assert (!resources.claimed ({adk::ResourceKind::Pin, pin}));
        }

        fake::clearTrace ();
        keypad.shutdown  ();

        assert (fake::trace ().empty ());
    }

    void scanFeedsInterpreter ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;
        adk::MatrixKeypad     keypad (resources, pins);

        assert (keypad.initialize () == adk::Status::Ok);

        fake::setDigitalInput (26, LOW);
        fake::setDigitalInput (27, HIGH);
        fake::setDigitalInput (28, HIGH);

        assert (keypad.update (adk::TimePoint (0)) == adk::Status::Ok);
        assert (keypad.update (adk::TimePoint (20)) == adk::Status::Ok);
        assert (keypad.snapshot ().rawMask == 0x249U);
        assert (keypad.snapshot ().state == adk::KeypadState::InvalidChord);
    }

    void everyPositionAndChordFeedsInterpreter ()
    {
        static const adk::KeypadKey keys[12] =
        {
            adk::KeypadKey::Digit1,
            adk::KeypadKey::Digit2,
            adk::KeypadKey::Digit3,
            adk::KeypadKey::Digit4,
            adk::KeypadKey::Digit5,
            adk::KeypadKey::Digit6,
            adk::KeypadKey::Digit7,
            adk::KeypadKey::Digit8,
            adk::KeypadKey::Digit9,
            adk::KeypadKey::Star,
            adk::KeypadKey::Digit0,
            adk::KeypadKey::Hash
        };

        for (uint8_t key = 0; key < 12; ++key)
        {
            adk::ResourceRegistry resources;
            RecordingMatrixIo     io;
            adk::MatrixKeypad     keypad (resources, pins, adk::KeypadConfig (), &io);

            assert (keypad.initialize () == adk::Status::Ok);
            io.pressedMask = static_cast<uint16_t> (1U << key);

            assert (keypad.update (adk::TimePoint (0)) == adk::Status::Ok);
            assert (keypad.update (adk::TimePoint (20)) == adk::Status::Ok);
            assert (keypad.snapshot ().rawMask == io.pressedMask);
            assert (keypad.snapshot ().state == adk::KeypadState::Pressed);
            assert (keypad.snapshot ().key == keys[key]);
            assert (keypad.snapshot ().pressEvent);
            assert (io.activeRow == 4);
        }

        for (uint8_t first = 0; first < 12; ++first)
        {
            for (uint8_t second = static_cast<uint8_t> (first + 1U);
                 second < 12;
                 ++second)
            {
                adk::ResourceRegistry resources;
                RecordingMatrixIo     io;
                adk::MatrixKeypad     keypad (
                    resources, pins, adk::KeypadConfig (), &io);

                assert (keypad.initialize () == adk::Status::Ok);
                io.pressedMask = static_cast<uint16_t> (
                    (1U << first) | (1U << second));

                assert (keypad.update (adk::TimePoint (0)) == adk::Status::Ok);
                assert (keypad.update (adk::TimePoint (20)) == adk::Status::Ok);
                assert (keypad.snapshot ().rawMask == io.pressedMask);
                assert (keypad.snapshot ().state == adk::KeypadState::InvalidChord);
                assert (!keypad.snapshot ().pressEvent);
                assert (io.activeRow == 4);
            }
        }
    }

    void scanFailuresAreExplicitAndRowsBecomeSafe ()
    {
        for (uint16_t failAt = 1; failAt <= 24; ++failAt)
        {
            adk::ResourceRegistry resources;
            RecordingMatrixIo     io;
            adk::MatrixKeypad     keypad (resources, pins, adk::KeypadConfig (), &io);

            assert (keypad.initialize () == adk::Status::Ok);
            io.operation   = 0;
            io.failAt      = failAt;
            io.pressedMask = 0x0800U;

            const adk::Status status = keypad.update (adk::TimePoint (0));

            if (status == adk::Status::HardwareFailure)
            {
                assert (keypad.snapshot ().rawMask == 0);
                assert (io.activeRow == 4);

                for (uint8_t row = pins.row0; row <= pins.row3; ++row)
                {
                    assert (io.modes[row] ==
                            adk::MatrixKeypadPinMode::HighImpedance);
                }

                io.operation = 0;
                assert (keypad.update (adk::TimePoint (20)) ==
                        adk::Status::HardwareFailure);
                assert (keypad.snapshot ().state == adk::KeypadState::Fault);

                io.failAt    = 0;
                io.operation = 0;
                assert (keypad.update (adk::TimePoint (40)) ==
                        adk::Status::HardwareFailure);
            }
        }
    }

    void initializationIoFailureRollsBack ()
    {
        for (uint16_t failAt = 1; failAt <= 7; ++failAt)
        {
            adk::ResourceRegistry resources;
            RecordingMatrixIo     io;
            adk::MatrixKeypad     keypad (resources, pins, adk::KeypadConfig (), &io);

            io.failAt = failAt;
            assert (keypad.initialize () == adk::Status::HardwareFailure);
            assert (!keypad.initialized ());
            assert (io.activeRow == 4);

            for (uint8_t pin = pins.row0; pin <= pins.column2; ++pin)
            {
                assert (!resources.claimed ({adk::ResourceKind::Pin, pin}));
            }
        }
    }

    void rejectsInvalidAndBusyPinsWithoutSideEffects ()
    {
        const adk::MatrixKeypadPins invalid[] =
        {
            {22, 22, 24, 25, 26, 27, 28},
            {22, 23, 24, 25, 26, 27, NUM_DIGITAL_PINS}
        };

        for (const auto& invalidPins : invalid)
        {
            fake::reset ();

            adk::ResourceRegistry resources;
            adk::MatrixKeypad     keypad (resources, invalidPins);

            assert (keypad.initialize () == adk::Status::InvalidPin);
            assert (fake::trace ().empty ());
        }

        for (uint8_t occupiedPin = pins.row0; occupiedPin <= pins.column2;
             ++occupiedPin)
        {
            fake::reset ();

            adk::ResourceRegistry resources;
            adk::ResourceClaim    occupied;
            adk::MatrixKeypad     keypad (resources, pins);

            assert (resources.claim (
                        {adk::ResourceKind::Pin, occupiedPin}, occupied) ==
                    adk::Status::Ok);
            assert (keypad.initialize () == adk::Status::ResourceBusy);
            assert (fake::trace ().empty ());

            for (uint8_t pin = pins.row0; pin <= pins.column2; ++pin)
            {
                assert (resources.claimed ({adk::ResourceKind::Pin, pin}) ==
                        (pin == occupiedPin));
            }
        }

        fake::reset ();

        adk::ResourceRegistry resources;
        adk::MatrixKeypad     keypad (
            resources, pins, adk::KeypadConfig (adk::Duration (0)));

        assert (keypad.initialize () == adk::Status::InvalidArgument);
        assert (fake::trace ().empty ());

        for (uint8_t pin = pins.row0; pin <= pins.column2; ++pin)
        {
            assert (!resources.claimed ({adk::ResourceKind::Pin, pin}));
        }
    }

    void destructionReleasesEveryPin ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;

        {
            adk::MatrixKeypad keypad (resources, pins);

            assert (keypad.initialize () == adk::Status::Ok);
        }

        for (uint8_t pin = 22; pin <= 28; ++pin)
        {
            assert (fake::mode (pin) == INPUT);
            assert (!resources.claimed ({adk::ResourceKind::Pin, pin}));
        }
    }

    static_assert (!std::is_copy_constructible<adk::MatrixKeypad>::value,
                   "matrix keypad owns claims");
    static_assert (!std::is_move_constructible<adk::MatrixKeypad>::value,
                   "matrix keypad has stable ownership");
}

int main ()
{
    lifecycleAndSafeScan                        ();
    scanFeedsInterpreter                        ();
    everyPositionAndChordFeedsInterpreter       ();
    scanFailuresAreExplicitAndRowsBecomeSafe    ();
    initializationIoFailureRollsBack            ();
    rejectsInvalidAndBusyPinsWithoutSideEffects ();
    destructionReleasesEveryPin                 ();
}
