#pragma once

#include "analog_input.h"
#include "button.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    struct JoystickAxisConfig
    {
        JoystickAxisConfig (PinId    pin,
                            uint16_t center,
                            uint16_t observedMinimum,
                            uint16_t observedMaximum,
                            uint16_t deadZone,
                            bool     inverted = false) noexcept;

        PinId    pin;
        uint16_t center;
        uint16_t observedMinimum;
        uint16_t observedMaximum;
        uint16_t deadZone;
        bool     inverted;
    };

    struct AnalogJoystickConfig
    {
        AnalogJoystickConfig (const JoystickAxisConfig& xAxis,
                              const JoystickAxisConfig& yAxis,
                              const ButtonConfig&       selectButton) noexcept;

        JoystickAxisConfig xAxis;
        JoystickAxisConfig yAxis;
        ButtonConfig       selectButton;
    };

    struct JoystickAxisSnapshot
    {
        uint16_t raw;
        int16_t  position;
        bool     centered;
        bool     saturated;
    };

    struct AnalogJoystickSnapshot
    {
        JoystickAxisSnapshot x;
        JoystickAxisSnapshot y;
        bool                 rawSelected;
        bool                 selected;
        bool                 selectEvent;
        bool                 releaseEvent;
        Status               status;
    };

    struct AnalogJoystick
    {
        static constexpr int16_t minimumPosition = -1000;
        static constexpr int16_t maximumPosition =  1000;

        AnalogJoystick  (ResourceRegistry&           resources,
                         const AnalogJoystickConfig& config) noexcept;
        ~AnalogJoystick () noexcept;

        AnalogJoystick            (const AnalogJoystick&) = delete;
        AnalogJoystick& operator= (const AnalogJoystick&) = delete;
        AnalogJoystick            (AnalogJoystick&&)      = delete;
        AnalogJoystick& operator= (AnalogJoystick&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status update     (TimePoint now) noexcept;

        bool                   initialized  () const noexcept;
        AnalogJoystickSnapshot snapshot     () const noexcept;

        const AnalogInput& xInput       () const noexcept;
        const AnalogInput& yInput       () const noexcept;
        const Button&      selectButton () const noexcept;

      private:
        static bool validAxis (const JoystickAxisConfig& axis) noexcept;

        JoystickAxisSnapshot deriveAxis (
            uint16_t                  raw,
            const JoystickAxisConfig& config) const noexcept;
        void setStatus (Status status) noexcept;

        AnalogJoystickConfig   config_;
        AnalogInput            xInput_;
        AnalogInput            yInput_;
        Button                 selectButton_;
        AnalogJoystickSnapshot snapshot_;
        bool                   initialized_;
    };
}
