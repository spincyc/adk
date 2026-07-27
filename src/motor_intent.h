#pragma once

#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    struct PowerDomain;

    enum struct MotorDirection : uint8_t
    {
        Stopped,
        Forward,
        Reverse
    };

    enum struct MotorIntentPhase : uint8_t
    {
        Inactive,
        Running,
        WaitingForDeadTime,
        Fault
    };

    struct MotorIntentConfig
    {
        MotorIntentConfig (Duration reversalDeadTime = Duration (20),
                           uint8_t  maximumDuty      = 255) noexcept;

        Duration reversalDeadTime;
        uint8_t  maximumDuty;
    };

    struct MotorCommand
    {
        MotorCommand (MotorDirection direction = MotorDirection::Stopped,
                      uint8_t        duty      = 0) noexcept;

        MotorDirection direction;
        uint8_t        duty;
    };

    struct MotorIntentSnapshot
    {
        MotorCommand requested;
        MotorCommand applied;
        MotorIntentPhase phase;
        Status       status;
        TimePoint    phaseSince;
        TimePoint    nextDeadline;
        bool         hasDeadline;
        uint32_t     transitionCount;
    };

    struct MotorIntent
    {
        MotorIntent (const MotorIntentConfig& config,
                     const PowerDomain&       power) noexcept;

        MotorIntent            (const MotorIntent&) = delete;
        MotorIntent& operator= (const MotorIntent&) = delete;
        MotorIntent            (MotorIntent&&)      = delete;
        MotorIntent& operator= (MotorIntent&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;

        Status command (const MotorCommand& command, TimePoint now) noexcept;
        Status update  (TimePoint now) noexcept;
        Status stop    () noexcept;

        MotorIntentSnapshot snapshot    () const noexcept;
        bool                initialized () const noexcept;

      private:
        bool   configValid  () const noexcept;
        bool   commandValid (const MotorCommand& command) const noexcept;
        bool   timeValid    (TimePoint now) const noexcept;
        bool   deadlineDue  (TimePoint now) const noexcept;
        Status enterFault   (Status status, TimePoint now) noexcept;
        void   apply        (const MotorCommand& command,
                             MotorIntentPhase   phase,
                             TimePoint          now) noexcept;
        void   enterStopped (TimePoint now) noexcept;

        MotorIntentConfig  config_;
        const PowerDomain* power_;
        MotorCommand       requested_;
        MotorCommand       applied_;
        MotorDirection     directionBeforeWait_;
        MotorIntentPhase   phase_;
        Status             status_;
        TimePoint          phaseSince_;
        TimePoint          lastUpdate_;
        bool               initialized_;
        bool               hasLastUpdate_;
        bool               hasDeadline_;
        uint32_t           transitionCount_;
    };
} // namespace adk
