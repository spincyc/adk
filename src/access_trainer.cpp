#include "access_trainer.h"

namespace adk {

    static constexpr uint32_t maximumDuration = 0x7fffffffu;

    AccessInput::AccessInput (const KeypadSnapshot& keypad,
                              bool componentFault) noexcept
        : keypad         (keypad)
        , componentFault (componentFault)
    {
    }

    AccessTrainer::AccessTrainer (const AccessTrainerConfig& config) noexcept
        : config_             (config)
        , auditRecord_        ()
        , entered_            {}
        , state_              (AccessState::Fault)
        , status_             (StatusCode::NotInitialized)
        , stateSince_         (TimePoint (0))
        , lastUpdate_         (TimePoint (0))
        , nextAuditSequence_  (0)
        , enteredCount_       (0)
        , failedAttempts_     (0)
        , entryOverflow_      (false)
        , initialized_        (false)
        , hasLastUpdate_      (false)
        , clearEntry_         (false)
        , hasAuditRecord_     (false)
    {
    }

    Status AccessTrainer::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        initialized_       = false;
        state_             = AccessState::Fault;
        status_            = StatusCode::InvalidArgument;
        enteredCount_      = 0;
        failedAttempts_    = 0;
        nextAuditSequence_ = 0;
        entryOverflow_     = false;
        hasLastUpdate_     = false;
        clearEntry_        = true;
        hasAuditRecord_    = false;
        clear ();

        if (!configValid ())
        {
            return status_;
        }

        state_       = AccessState::Ready;
        status_      = StatusCode::Ok;
        initialized_ = true;
        return status_;
    }

    Status AccessTrainer::reset (TimePoint now) noexcept
    {
        if (!configValid ())
        {
            initialized_ = false;
            state_       = AccessState::Fault;
            status_      = StatusCode::InvalidArgument;
            return status_;
        }

        state_              = AccessState::Ready;
        status_             = StatusCode::Ok;
        stateSince_         = now;
        lastUpdate_         = now;
        failedAttempts_     = 0;
        initialized_        = true;
        hasLastUpdate_      = true;
        clearEntry_         = true;
        hasAuditRecord_     = false;
        clear ();

        emitAudit (AccessAuditKind::Reset, now);

        return status_;
    }

    void AccessTrainer::shutdown () noexcept
    {
        state_             = AccessState::Fault;
        status_            = StatusCode::NotInitialized;
        enteredCount_      = 0;
        failedAttempts_    = 0;
        entryOverflow_     = false;
        initialized_       = false;
        hasLastUpdate_     = false;
        clearEntry_        = true;
        hasAuditRecord_    = false;
        clear ();
    }

    Status AccessTrainer::update (TimePoint now,
                                  const AccessInput& input) noexcept
    {
        clearEntry_     = false;
        hasAuditRecord_ = false;

        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!timeValid (now))
        {
            enterFault (StatusCode::InvalidArgument, now);
            return status_;
        }

        lastUpdate_    = now;
        hasLastUpdate_ = true;

        if (state_ == AccessState::Fault)
        {
            return status_;
        }

        if (input.componentFault ||
            input.keypad.state == KeypadState::Fault ||
            !input.keypad.status.ok ())
        {
            enterFault (StatusCode::HardwareFailure, now);
            return status_;
        }

        status_ = StatusCode::Ok;

        switch (state_)
        {
            case AccessState::Ready:
            case AccessState::Entering:
                if (input.keypad.pressEvent &&
                    input.keypad.state == KeypadState::Pressed)
                {
                    const KeypadKey key = input.keypad.key;

                    if (digit (key))
                    {
                        if (enteredCount_ <
                            AccessTrainerConfig::credentialCapacity)
                        {
                            entered_[enteredCount_++] = key;
                        }
                        else
                        {
                            entryOverflow_ = true;
                        }

                        state_ = AccessState::Entering;
                    }
                    else if (key == KeypadKey::Star)
                    {
                        clear ();
                        clearEntry_ = true;
                        state_      = AccessState::Ready;
                    }
                    else if (key == KeypadKey::Hash)
                    {
                        submit (now);
                    }
                }
                break;

            case AccessState::Granted:
                if (deadlineDue (now, config_.grantDuration))
                {
                    enter (AccessState::Ready, now);
                    clear ();
                    clearEntry_ = true;

                    emitAudit (AccessAuditKind::GrantExpired, now);
                }
                break;

            case AccessState::Denied:
                if (deadlineDue (now, config_.deniedDuration))
                {
                    enter (AccessState::Ready, now);
                    clear ();
                    clearEntry_ = true;

                    emitAudit (AccessAuditKind::DeniedExpired, now);
                }
                break;

            case AccessState::LockedOut:
                if (deadlineDue (now, config_.lockoutDuration))
                {
                    failedAttempts_ = 0;
                    enter (AccessState::Ready, now);
                    clear ();
                    clearEntry_ = true;

                    emitAudit (AccessAuditKind::LockoutExpired, now);
                }
                break;

            case AccessState::Fault:
                break;
        }

        return status_;
    }

    AccessSnapshot AccessTrainer::snapshot () const noexcept
    {
        const bool open = initialized_ && state_ == AccessState::Granted;

        AccessSnapshot result = {
            state_,
            status_,
            ledIntent (),
            open ? SoftLatchIntent::Open : SoftLatchIntent::Closed,
            auditRecord_,
            enteredCount_,
            failedAttempts_,
            open,
            clearEntry_,
            hasAuditRecord_
        };

        return result;
    }

    bool AccessTrainer::configValid () const noexcept
    {
        if (config_.credentialLength == 0 ||
            config_.credentialLength >
                AccessTrainerConfig::credentialCapacity ||
            config_.maximumFailedAttempts == 0)
        {
            return false;
        }

        const uint32_t grant = config_.grantDuration.milliseconds ();

        const uint32_t denied = config_.deniedDuration.milliseconds ();

        const uint32_t lockout = config_.lockoutDuration.milliseconds ();

        if (grant == 0 || grant > maximumDuration ||
            denied == 0 || denied > maximumDuration ||
            lockout == 0 || lockout > maximumDuration)
        {
            return false;
        }

        for (uint8_t index = 0; index < config_.credentialLength; ++index)
        {
            if (!digit (config_.credential[index]))
            {
                return false;
            }
        }

        return true;
    }

    bool AccessTrainer::timeValid (TimePoint now) const noexcept
    {
        return !hasLastUpdate_ || now == lastUpdate_ ||
               now.elapsedSince (lastUpdate_).milliseconds () <= maximumDuration;
    }

    bool AccessTrainer::deadlineDue (TimePoint now,
                                     Duration duration) const noexcept
    {
        return now.elapsedSince (stateSince_) >= duration;
    }

    bool AccessTrainer::digit (KeypadKey key) const noexcept
    {
        return key >= KeypadKey::Digit0 && key <= KeypadKey::Digit9;
    }

    bool AccessTrainer::credentialMatches () const noexcept
    {
        uint8_t difference = static_cast<uint8_t> (
            enteredCount_ ^ config_.credentialLength);

        for (uint8_t index = 0;
             index < AccessTrainerConfig::credentialCapacity;
             ++index)
        {
            const KeypadKey entered = index < enteredCount_
                                          ? entered_[index]
                                          : KeypadKey::None;
            const KeypadKey expected = index < config_.credentialLength
                                           ? config_.credential[index]
                                           : KeypadKey::None;

            difference |= static_cast<uint8_t> (entered) ^
                          static_cast<uint8_t> (expected);
        }

        return difference == 0 && !entryOverflow_;
    }

    AccessLedIntent AccessTrainer::ledIntent () const noexcept
    {
        switch (state_)
        {
            case AccessState::Ready:
                return AccessLedIntent::Ready;
            case AccessState::Entering:
                return AccessLedIntent::Entering;
            case AccessState::Granted:
                return AccessLedIntent::Granted;
            case AccessState::Denied:
                return AccessLedIntent::Denied;
            case AccessState::LockedOut:
                return AccessLedIntent::LockedOut;
            case AccessState::Fault:
                return AccessLedIntent::Fault;
        }

        return AccessLedIntent::Fault;
    }

    void AccessTrainer::emitAudit (AccessAuditKind kind,
                                   TimePoint       now) noexcept
    {
        auditRecord_.sequence       = nextAuditSequence_++;
        auditRecord_.kind           = kind;
        auditRecord_.state          = state_;
        auditRecord_.time           = now;
        auditRecord_.failedAttempts = failedAttempts_;
        hasAuditRecord_             = true;
    }

    void AccessTrainer::clear () noexcept
    {
        for (uint8_t index = 0;
             index < AccessTrainerConfig::credentialCapacity;
             ++index)
        {
            entered_[index] = KeypadKey::None;
        }

        enteredCount_  = 0;
        entryOverflow_ = false;
    }

    void AccessTrainer::enter (AccessState state, TimePoint now) noexcept
    {
        state_      = state;
        stateSince_ = now;
    }

    void AccessTrainer::enterFault (Status status, TimePoint now) noexcept
    {
        state_      = AccessState::Fault;
        stateSince_ = now;
        status_     = status;
        clear ();
        clearEntry_ = true;

        emitAudit (AccessAuditKind::Fault, now);
    }

    void AccessTrainer::submit (TimePoint now) noexcept
    {
        const bool granted = credentialMatches ();

        clear ();
        clearEntry_ = true;

        if (granted)
        {
            enter (AccessState::Granted, now);

            emitAudit (AccessAuditKind::Granted, now);
            return;
        }

        if (failedAttempts_ < UINT8_MAX)
        {
            ++failedAttempts_;
        }

        if (failedAttempts_ >= config_.maximumFailedAttempts)
        {
            enter (AccessState::LockedOut, now);

            emitAudit (AccessAuditKind::LockoutStarted, now);
        }
        else
        {
            enter     (AccessState::Denied, now);
            emitAudit (AccessAuditKind::Denied, now);
        }
    }
}
