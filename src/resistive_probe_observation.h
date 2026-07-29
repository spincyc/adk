#pragma once

#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct ProbeQuality : uint8_t
    {
        Unqualified,
        Dry,
        Damp,
        Wet,
        Saturated,
        Disconnected,
        ExcitationFault,
        Stale,
        ProducerFault
    };

    struct ResistiveProbeSample
    {
        uint8_t   sourceId;
        uint16_t  configurationRevision;
        uint16_t  calibrationRevision;
        uint32_t  sequence;
        TimePoint observedAt;
        uint16_t  energizedRaw;
        uint16_t  dischargedRaw;
        Duration  excitationOnTime;
        Duration  cycleTime;
        bool      excitationObservedOffAfterSample;
        Status    status;
    };

    struct ResistiveProbeConfig
    {
        uint16_t adcMaximum;
        uint16_t dryReference;
        uint16_t wetReference;
        uint16_t disconnectedMaximum;
        uint16_t dischargedMaximum;
        uint16_t dampThresholdPermille;
        uint16_t wetThresholdPermille;
        Duration maximumAge;
        Duration maximumExcitationOnTime;
        uint16_t maximumDutyPermille;
    };

    struct ResistiveProbeObservation
    {
        ResistiveProbeSample sample;
        uint16_t             normalizedPermille;
        uint16_t             observedCycleDutyPermille;
        ProbeQuality         quality;
        Duration             age;
        Status               status;
    };

    // Pure copied-sample policy; it owns no ADC, excitation output, or clock.
    struct ResistiveProbeObservationPolicy
    {
        explicit ResistiveProbeObservationPolicy (
            const ResistiveProbeConfig& config) noexcept;

        ResistiveProbeObservationPolicy (const ResistiveProbeObservationPolicy&) =
            delete;
        ResistiveProbeObservationPolicy&
        operator= (const ResistiveProbeObservationPolicy&)                  = delete;
        ResistiveProbeObservationPolicy (ResistiveProbeObservationPolicy&&) = delete;
        ResistiveProbeObservationPolicy&
        operator= (ResistiveProbeObservationPolicy&&) = delete;

        Status initialize () noexcept;
        void   reset      () noexcept;
        Status update     (TimePoint now, const ResistiveProbeSample& sample) noexcept;

        ResistiveProbeObservation snapshot    () const noexcept;
        bool                      initialized () const noexcept;

      private:
        ResistiveProbeConfig      config_;
        ResistiveProbeObservation observation_;
        TimePoint                 lastUpdateAt_;
        bool                      initialized_;
        bool                      hasSample_;
        bool                      hasUpdate_;
        bool                      sequenceExhausted_;
    };
} // namespace adk
