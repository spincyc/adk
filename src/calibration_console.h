#pragma once

#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct CalibrationField : uint8_t
    {
        Minimum,
        Maximum
    };

    enum struct CalibrationConsoleState : uint8_t
    {
        Selecting,
        Editing,
        Committed,
        Cancelled,
        Fault
    };

    struct CalibrationConsoleConfig
    {
        CalibrationConsoleConfig (uint16_t lowerLimit, uint16_t upperLimit,
                                  uint16_t minimumSeparation,
                                  Duration acknowledgement) noexcept;

        uint16_t lowerLimit;
        uint16_t upperLimit;
        uint16_t minimumSeparation;
        Duration acknowledgement;
    };

    struct CalibrationConsoleInput
    {
        CalibrationConsoleInput () noexcept;

        int16_t joystickX;
        int16_t joystickY;
        bool    selectEvent;
        int8_t  encoderDelta;
        bool    cancelEvent;
        bool    inputValid;
    };

    struct CalibrationConsoleSnapshot
    {
        CalibrationConsoleState state;
        CalibrationField        field;
        uint16_t                committedMinimum;
        uint16_t                committedMaximum;
        uint16_t                previewMinimum;
        uint16_t                previewMaximum;
        bool                    changed;
        Status                  status;
    };

    struct CalibrationConsole
    {
        explicit CalibrationConsole (const CalibrationConsoleConfig& config) noexcept;

        Status initialize (uint16_t initialMinimum, uint16_t initialMaximum) noexcept;
        void   shutdown   () noexcept;
        Status update     (TimePoint now,
                           const CalibrationConsoleInput& input) noexcept;

        bool                       initialized () const noexcept;
        CalibrationConsoleSnapshot snapshot    () const noexcept;

      private:
        static constexpr int16_t selectionThreshold = 500;

        bool     configValid        () const noexcept;
        bool     initialRangeValid  (uint16_t initialMinimum,
                                     uint16_t initialMaximum) const noexcept;
        bool     timeValid          (TimePoint now) const noexcept;
        bool     acknowledgementDue (TimePoint now) const noexcept;
        uint16_t coarseCandidate    (int16_t joystickY) const noexcept;
        uint16_t clampCandidate     (int32_t candidate) const noexcept;
        void     beginEditing       (TimePoint now) noexcept;
        void     commit             (TimePoint now) noexcept;
        void     cancel             (TimePoint now) noexcept;
        void     enterFault         (Status status, TimePoint now) noexcept;
        void     refreshChanged     () noexcept;

        CalibrationConsoleConfig   config_;
        CalibrationConsoleSnapshot snapshot_;
        TimePoint                  stateSince_;
        TimePoint                  lastUpdate_;
        bool                       initialized_;
        bool                       hasLastUpdate_;
    };
} // namespace adk
