#include "keypad.h"

namespace adk {

    KeypadConfig::KeypadConfig (Duration debounce) noexcept
        : debounce (debounce)
    {
    }

    KeypadSample::KeypadSample (uint16_t pressedMask, bool valid) noexcept
        : pressedMask (pressedMask)
        , valid       (valid)
    {
    }

    Keypad::Keypad (const KeypadConfig& config) noexcept
        : config_          (config)
        , candidateSince_  ()
        , lastUpdate_      ()
        , candidateMask_   (0)
        , stableMask_      (0)
        , rawMask_         (0)
        , status_          (Status::NotInitialized)
        , candidateValid_  (true)
        , stableValid_     (true)
        , initialized_     (false)
        , hasLastUpdate_   (false)
        , pressArmed_      (true)
        , pressEvent_      (false)
        , releaseEvent_    (false)
    {
    }

    Status Keypad::initialize () noexcept
    {
        if (initialized_)
        {
            return Status::Ok;
        }

        if (config_.debounce.milliseconds () == 0 ||
            config_.debounce.milliseconds () >= 0x80000000UL)
        {
            return Status::InvalidArgument;
        }

        candidateSince_ = TimePoint ();
        lastUpdate_     = TimePoint ();
        candidateMask_  = 0;
        stableMask_     = 0;
        rawMask_        = 0;
        status_         = Status::Ok;
        candidateValid_ = true;
        stableValid_    = true;
        initialized_    = true;
        hasLastUpdate_  = false;
        pressArmed_     = true;
        pressEvent_     = false;
        releaseEvent_   = false;

        return Status::Ok;
    }

    void Keypad::shutdown () noexcept
    {
        initialized_  = false;
        status_       = Status::NotInitialized;
        stableMask_   = 0;
        rawMask_      = 0;
        pressEvent_   = false;
        releaseEvent_ = false;
    }

    Status Keypad::update (TimePoint now, const KeypadSample& sample) noexcept
    {
        pressEvent_   = false;
        releaseEvent_ = false;

        if (!initialized_)
        {
            return Status::NotInitialized;
        }

        if (!timeValid (now) || (sample.pressedMask & 0xf000U) != 0)
        {
            status_ = Status::InvalidArgument;
            return status_;
        }

        rawMask_ = sample.pressedMask;
        status_  = stableValid_ ? Status::Ok : Status::HardwareFailure;

        if (candidateMask_ != rawMask_ || candidateValid_ != sample.valid)
        {
            candidateMask_  = rawMask_;
            candidateValid_ = sample.valid;
            candidateSince_ = now;
        }
        else if (now.elapsedSince (candidateSince_) >= config_.debounce)
        {
            acceptState (now, candidateMask_, candidateValid_);
        }

        lastUpdate_    = now;
        hasLastUpdate_ = true;

        return status_;
    }

    bool Keypad::initialized () const noexcept
    {
        return initialized_;
    }

    KeypadSnapshot Keypad::snapshot () const noexcept
    {
        KeypadState state = KeypadState::Released;
        KeypadKey   key   = KeypadKey::None;

        if (!stableValid_)
        {
            state = KeypadState::Fault;
        }
        else if (oneKey (stableMask_))
        {
            state = KeypadState::Pressed;
            key   = keyFromMask (stableMask_);
        }
        else if (stableMask_ != 0)
        {
            state = KeypadState::InvalidChord;
        }

        return {key, state, status_, rawMask_, pressEvent_, releaseEvent_};
    }

    bool Keypad::oneKey (uint16_t mask) noexcept
    {
        return mask != 0 && (mask & static_cast<uint16_t> (mask - 1U)) == 0;
    }

    KeypadKey Keypad::keyFromMask (uint16_t mask) noexcept
    {
        static const KeypadKey keys[12] =
        {
            KeypadKey::Digit1,
            KeypadKey::Digit2,
            KeypadKey::Digit3,
            KeypadKey::Digit4,
            KeypadKey::Digit5,
            KeypadKey::Digit6,
            KeypadKey::Digit7,
            KeypadKey::Digit8,
            KeypadKey::Digit9,
            KeypadKey::Star,
            KeypadKey::Digit0,
            KeypadKey::Hash
        };

        for (uint8_t index = 0; index < 12; ++index)
        {
            if (mask == static_cast<uint16_t> (1U << index))
            {
                return keys[index];
            }
        }

        return KeypadKey::None;
    }

    bool Keypad::timeValid (TimePoint now) const noexcept
    {
        if (!hasLastUpdate_)
        {
            return true;
        }

        return now.elapsedSince (lastUpdate_).milliseconds () < 0x80000000UL;
    }

    void Keypad::acceptState (TimePoint, uint16_t mask, bool valid) noexcept
    {
        if (stableMask_ == mask && stableValid_ == valid)
        {
            return;
        }

        const bool wasPressed = oneKey (stableMask_) && stableValid_;

        stableMask_  = mask;
        stableValid_ = valid;
        status_      = valid ? Status::Ok : Status::HardwareFailure;

        if (mask == 0 && valid)
        {
            releaseEvent_ = wasPressed || !pressArmed_;
            pressArmed_   = true;
            return;
        }

        if (oneKey (mask) && valid && pressArmed_)
        {
            pressEvent_ = true;
            pressArmed_ = false;
            return;
        }

        pressArmed_ = false;
    }
}
