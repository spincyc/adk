#pragma once

#include "keypad.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct AccessState : uint8_t
    {
        Ready,
        Entering,
        Granted,
        Denied,
        LockedOut,
        Fault
    };

    enum struct AccessLedIntent : uint8_t
    {
        Ready,
        Entering,
        Granted,
        Denied,
        LockedOut,
        Fault
    };

    enum struct SoftLatchIntent : uint8_t
    {
        Closed,
        Open
    };

    enum struct AccessAuditKind : uint8_t
    {
        Granted,
        Denied,
        LockoutStarted,
        GrantExpired,
        DeniedExpired,
        LockoutExpired,
        Fault,
        Reset
    };

    struct AccessTrainerConfig
    {
        static constexpr uint8_t credentialCapacity = 8;

        KeypadKey credential[credentialCapacity] = {};
        Duration  grantDuration                  = Duration (3000);
        Duration  deniedDuration                 = Duration (1000);
        Duration  lockoutDuration                = Duration (10000);
        uint8_t   credentialLength               = 0;
        uint8_t   maximumFailedAttempts          = 3;
    };

    struct AccessInput
    {
        explicit AccessInput (const KeypadSnapshot& keypad,
                              bool componentFault = false) noexcept;

        KeypadSnapshot keypad;
        bool           componentFault;
    };

    struct AccessAuditRecord
    {
        uint16_t        sequence       = 0;
        AccessAuditKind kind           = AccessAuditKind::Reset;
        AccessState     state          = AccessState::Fault;
        TimePoint       time           = TimePoint (0);
        uint8_t         failedAttempts = 0;
    };

    struct AccessSnapshot
    {
        AccessState     state;
        Status          status;
        AccessLedIntent ledIntent;
        SoftLatchIntent softLatchIntent;
        AccessAuditRecord auditRecord;
        uint8_t         enteredCount;
        uint8_t         failedAttempts;
        bool            softLatchOpen;
        bool            clearEntry;
        bool            hasAuditRecord;
    };

    struct AccessTrainer
    {
        explicit AccessTrainer (const AccessTrainerConfig& config) noexcept;

        Status initialize  () noexcept;
        Status reset       (TimePoint now) noexcept;
        void   shutdown    () noexcept;
        Status update      (TimePoint now, const AccessInput& input) noexcept;

        AccessSnapshot snapshot () const noexcept;

      private:
        bool            configValid       () const noexcept;
        bool            timeValid         (TimePoint now) const noexcept;
        bool            deadlineDue       (TimePoint now,
                                           Duration duration) const noexcept;
        bool            digit             (KeypadKey key) const noexcept;
        bool            credentialMatches () const noexcept;
        AccessLedIntent ledIntent         () const noexcept;
        void            emitAudit         (AccessAuditKind kind,
                                           TimePoint       now) noexcept;
        void            clear             () noexcept;
        void            enter             (AccessState state,
                                           TimePoint   now) noexcept;
        void            enterFault        (Status status, TimePoint now) noexcept;
        void            submit            (TimePoint now) noexcept;

        AccessTrainerConfig config_;
        AccessAuditRecord  auditRecord_;
        KeypadKey          entered_[AccessTrainerConfig::credentialCapacity];
        AccessState        state_;
        Status             status_;
        TimePoint          stateSince_;
        TimePoint          lastUpdate_;
        uint16_t           nextAuditSequence_;
        uint8_t            enteredCount_;
        uint8_t            failedAttempts_;
        bool               entryOverflow_;
        bool               initialized_;
        bool               hasLastUpdate_;
        bool               clearEntry_;
        bool               hasAuditRecord_;
    };
}
