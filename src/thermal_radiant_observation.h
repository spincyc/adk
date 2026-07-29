#pragma once

#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct ThresholdState : uint8_t
    {
        Below,
        AtOrAbove
    };

    enum struct ThermalQuality : uint8_t
    {
        Unqualified,
        Normal,
        Warning,
        Alarm,
        Disagreement,
        Saturated,
        Stale,
        ProducerFault
    };

    enum struct RadiantQuality : uint8_t
    {
        Unqualified,
        Quiet,
        AbruptChange,
        Sustained,
        SaturatedAmbient,
        Stale,
        ProducerFault
    };

    struct ConvertedThermalSample
    {
        uint8_t   sourceId;
        uint16_t  configurationRevision;
        uint16_t  calibrationRevision;
        uint32_t  sequence;
        TimePoint observedAt;
        int32_t   milliCelsius;
        uint32_t  uncertaintyMilliCelsius;
        bool      saturated;
        Status    status;
    };

    struct CategoricalThresholdSample
    {
        uint8_t        sourceId;
        uint16_t       configurationRevision;
        uint16_t       calibrationRevision;
        uint32_t       sequence;
        TimePoint      observedAt;
        uint16_t       raw;
        ThresholdState state;
        bool           saturated;
        Status         status;
    };

    struct ThermalRadiantEnvelope
    {
        ConvertedThermalSample     thermistor;
        CategoricalThresholdSample digitalTemperature;
        CategoricalThresholdSample radiant;
    };

    struct ThermalRadiantConfig
    {
        int32_t  warningMilliCelsius;
        int32_t  alarmMilliCelsius;
        Duration maximumAge;
        Duration radiantPulseMaximum;
        Duration radiantSustainedMinimum;
    };

    struct ThermalRadiantObservation
    {
        ThermalRadiantEnvelope envelope;
        ThermalQuality         thermalQuality;
        RadiantQuality         radiantQuality;
        Duration               thermistorAge;
        Duration               digitalTemperatureAge;
        Duration               radiantAge;
        bool                   thermalHazard;
        bool                   radiantHazard;
        Status                 status;
    };

    // Pure copied-evidence policy; it owns no sensor, transport, or clock.
    struct ThermalRadiantObservationPolicy
    {
        explicit ThermalRadiantObservationPolicy (
            const ThermalRadiantConfig& config) noexcept;

        ThermalRadiantObservationPolicy (const ThermalRadiantObservationPolicy&) =
            delete;
        ThermalRadiantObservationPolicy&
        operator= (const ThermalRadiantObservationPolicy&)                  = delete;
        ThermalRadiantObservationPolicy (ThermalRadiantObservationPolicy&&) = delete;
        ThermalRadiantObservationPolicy&
        operator= (ThermalRadiantObservationPolicy&&) = delete;

        Status initialize () noexcept;
        void   reset      () noexcept;
        Status update     (TimePoint now,
                           const ThermalRadiantEnvelope& envelope) noexcept;

        ThermalRadiantObservation snapshot    () const noexcept;
        bool                      initialized () const noexcept;

      private:
        ThermalRadiantConfig      config_;
        ThermalRadiantObservation observation_;
        TimePoint                 lastUpdateAt_;
        TimePoint                 radiantActiveSince_;
        bool                      initialized_;
        bool                      hasEnvelope_;
        bool                      hasUpdate_;
        bool                      radiantCandidateActive_;
        bool                      radiantCandidateSustained_;
        bool                      thermistorSequenceExhausted_;
        bool                      digitalSequenceExhausted_;
        bool                      radiantSequenceExhausted_;
    };
} // namespace adk
