#pragma once

#include "digital_input.h"
#include "status.h"
#include "time.h"

namespace adk {

    struct ButtonConfig
    {
        ButtonConfig (PinId    pin,
                      Pull     pull         = Pull::Up,
                      Level    pressedLevel = Level::Low,
                      Duration debounce     = Duration (20)) noexcept;

        PinId    pin;
        Pull     pull;
        Level    pressedLevel;
        Duration debounce;
    };

    struct Button
    {
        Button            (ResourceRegistry& resources,
                           const ButtonConfig& config) noexcept;
        ~Button           () noexcept;

        Button            (const Button&) = delete;
        Button& operator= (const Button&) = delete;
        Button            (Button&&)      = delete;
        Button& operator= (Button&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        void   update     (TimePoint now) noexcept;

        bool initialized  () const noexcept;
        bool rawPressed   () const noexcept;
        bool pressed      () const noexcept;
        bool pressEvent   () const noexcept;
        bool releaseEvent () const noexcept;

        const DigitalInput& input () const noexcept;

      private:
        bool levelIsPressed (Level level) const noexcept;

        DigitalInput input_;
        Level        pressedLevel_;
        Duration     debounce_;
        TimePoint    candidateSince_;
        bool         candidatePressed_;
        bool         stablePressed_;
        bool         pressEvent_;
        bool         releaseEvent_;
        bool         pressArmed_;
        bool         timingCandidate_;
    };
}
