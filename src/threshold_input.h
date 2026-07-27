#pragma once

#include <stdint.h>

namespace adk {

    enum struct ThresholdDirection : uint8_t
    {
        Rising,
        Falling
    };

    enum struct ThresholdFault : uint8_t
    {
        None,
        InvalidConfiguration,
        BelowRange,
        AboveRange,
        Disconnected,
        Saturated,
        SourceFailure
    };

    struct ThresholdInputConfig
    {
        uint16_t           validMinimum;
        uint16_t           validMaximum;
        uint16_t           activateAt;
        uint16_t           deactivateAt;
        ThresholdDirection direction;
    };

    struct ThresholdObservation
    {
        uint16_t       raw;
        ThresholdFault fault;
        bool           active;
        bool           changed;
        bool           valid;
    };

    struct ThresholdInput
    {
        explicit ThresholdInput (const ThresholdInputConfig& config) noexcept;

        void                 reset       () noexcept;
        bool                 validConfig () const noexcept;
        ThresholdObservation update      (uint16_t       raw,
                                          ThresholdFault sourceFault =
                                              ThresholdFault::None) noexcept;
        ThresholdObservation observation () const noexcept;

      private:
        bool           configValid    () const noexcept;
        ThresholdFault classify       (uint16_t raw,
                                       ThresholdFault sourceFault) const noexcept;
        bool           nextActive     (uint16_t raw) const noexcept;

        ThresholdInputConfig config_;
        ThresholdObservation observation_;
    };
}
