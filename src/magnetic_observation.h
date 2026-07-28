#pragma once

#include "analog_input.h"
#include "digital_input.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct MagneticPolarity : uint8_t
    {
        Negative,
        Neutral,
        Positive,
        Unspecified
    };

    enum struct MagneticQuality : uint8_t
    {
        Unqualified,
        Valid,
        BelowQualifiedRange,
        AboveQualifiedRange
    };

    enum struct MagneticSource : uint8_t
    {
        LinearAnalog,
        ContactDigital
    };

    struct MagneticObservation
    {
        MagneticSource   source;
        uint16_t         raw;
        Level            rawLevel;
        TimePoint        observedAt;
        MagneticPolarity polarity;
        bool             activationEvent;
        bool             deactivationEvent;
        bool             active;
        Duration         stableFor;
        MagneticQuality  quality;
        Status           status;
    };

    struct LinearHallConfig
    {
        PinId    pin;
        uint16_t qualifiedMinimum;
        uint16_t qualifiedMaximum;
        uint16_t negativeActivate;
        uint16_t negativeRelease;
        uint16_t positiveRelease;
        uint16_t positiveActivate;
        Duration dwell;
        bool     reversePolarity;
    };

    struct MagneticContactConfig
    {
        PinId    pin;
        Pull     pull;
        Level    closedLevel;
        Duration dwell;
    };

    struct LinearHall
    {
        LinearHall (ResourceRegistry&       resources,
                    const LinearHallConfig& config) noexcept;
        ~LinearHall () noexcept;

        LinearHall (const LinearHall&)            = delete;
        LinearHall& operator= (const LinearHall&) = delete;
        LinearHall (LinearHall&&)                 = delete;
        LinearHall& operator= (LinearHall&&)      = delete;

        Status initialize () noexcept;
        void   update     (TimePoint now) noexcept;
        void   shutdown   () noexcept;

        MagneticObservation snapshot    () const noexcept;
        bool                initialized () const noexcept;

      private:
        MagneticPolarity classify       (uint16_t raw) const noexcept;
        MagneticPolarity reported       (MagneticPolarity value) const noexcept;
        void             clearCandidate () noexcept;
        void             publish        (TimePoint       now,
                                         uint16_t        raw,
                                         MagneticQuality quality) noexcept;

        LinearHallConfig    config_;
        AnalogInput         input_;
        MagneticObservation snapshot_;
        MagneticPolarity    candidate_;
        TimePoint           candidateSince_;
        TimePoint           stableSince_;
        TimePoint           lastUpdate_;
        bool                hasCandidate_;
        bool                hasUpdate_;
        bool                initialized_;
    };

    struct MagneticContact
    {
        MagneticContact (ResourceRegistry&            resources,
                         const MagneticContactConfig& config) noexcept;
        ~MagneticContact () noexcept;

        MagneticContact (const MagneticContact&)            = delete;
        MagneticContact& operator= (const MagneticContact&) = delete;
        MagneticContact (MagneticContact&&)                 = delete;
        MagneticContact& operator= (MagneticContact&&)      = delete;

        Status initialize () noexcept;
        void   update     (TimePoint now) noexcept;
        void   shutdown   () noexcept;

        MagneticObservation snapshot    () const noexcept;
        bool                initialized () const noexcept;

      private:
        void clearCandidate () noexcept;

        MagneticContactConfig config_;
        DigitalInput          input_;
        MagneticObservation   snapshot_;
        bool                  candidateActive_;
        TimePoint             candidateSince_;
        TimePoint             stableSince_;
        TimePoint             lastUpdate_;
        bool                  hasCandidate_;
        bool                  hasUpdate_;
        bool                  initialized_;
    };
} // namespace adk
