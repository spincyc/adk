#pragma once

#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct KeypadKey : uint8_t
    {
        None,
        Digit0,
        Digit1,
        Digit2,
        Digit3,
        Digit4,
        Digit5,
        Digit6,
        Digit7,
        Digit8,
        Digit9,
        Star,
        Hash
    };

    enum struct KeypadState : uint8_t
    {
        Released,
        Pressed,
        InvalidChord,
        Fault
    };

    struct KeypadConfig
    {
        explicit KeypadConfig (Duration debounce = Duration (20)) noexcept;

        Duration debounce;
    };

    struct KeypadSample
    {
        KeypadSample (uint16_t pressedMask = 0, bool valid = true) noexcept;

        uint16_t pressedMask;
        bool     valid;
    };

    struct KeypadSnapshot
    {
        KeypadKey   key;
        KeypadState state;
        Status      status;
        uint16_t    rawMask;
        bool        pressEvent;
        bool        releaseEvent;
    };

    struct Keypad
    {
        explicit Keypad (const KeypadConfig& config) noexcept;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status update     (TimePoint now, const KeypadSample& sample) noexcept;

        bool           initialized () const noexcept;
        KeypadSnapshot snapshot    () const noexcept;

      private:
        static bool      oneKey      (uint16_t mask) noexcept;
        static KeypadKey keyFromMask (uint16_t mask) noexcept;

        bool timeValid   (TimePoint now) const noexcept;
        void acceptState (TimePoint now, uint16_t mask, bool valid) noexcept;

        KeypadConfig config_;
        TimePoint    candidateSince_;
        TimePoint    lastUpdate_;
        uint16_t     candidateMask_;
        uint16_t     stableMask_;
        uint16_t     rawMask_;
        Status       status_;
        bool         candidateValid_;
        bool         stableValid_;
        bool         initialized_;
        bool         hasLastUpdate_;
        bool         pressArmed_;
        bool         pressEvent_;
        bool         releaseEvent_;
    };
}
