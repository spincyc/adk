#pragma once

#include <stdint.h>

#include "cue_audit.h"
#include "inert_channel_assessor.h"
#include "inert_cue_scheduler.h"
#include "status.h"
#include "time.h"

namespace adk {

    struct InertCueChannelMap
    {
        InertChannelId channels[InertCuePlan::capacity];
        uint8_t        count;
    };

    enum struct InertShowState : uint8_t
    {
        Startup,
        Review,
        Ready,
        Running,
        Held,
        Complete,
        Cancelled,
        Fault
    };

    enum struct InertShowFault : uint8_t
    {
        None,
        ObservationContradictory,
        AuditFull,
        InvalidInput,
        InternalInvariant
    };

    struct InertShowInput
    {
        const InertChannelObservation* observations;
        uint8_t                        observationCount;
        CueOperatorInput               operatorInput;
    };

    struct InertShowSnapshot
    {
        InertShowState             state;
        InertShowFault             fault;
        InertChannelAssessment     selectedChannel;
        CueSchedulerSnapshot       schedule;
        uint32_t                   auditSequence;
        uint32_t                   traceDigest;
        Status                     status;
        bool                       hasSelectedChannel;
    };

    struct InertShowSimulator
    {
        InertShowSimulator (const InertCueChannelMap& map,
                            InertChannelAssessor&      assessor,
                            InertCueScheduler&         scheduler,
                            CueAuditBuffer&            audit) noexcept;
        ~InertShowSimulator () noexcept;

        InertShowSimulator (const InertShowSimulator&)            = delete;
        InertShowSimulator& operator= (const InertShowSimulator&) = delete;
        InertShowSimulator (InertShowSimulator&&)                 = delete;
        InertShowSimulator& operator= (InertShowSimulator&&)      = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        bool   initialized () const noexcept;

        Status update (TimePoint now, const InertShowInput& input) noexcept;

        InertShowSnapshot snapshot () const noexcept;

      private:
        bool validMap () const noexcept;

        Status canonicalizeFrame (
            TimePoint now, const InertShowInput& input,
            InertChannelObservation (&canonical)[InertChannelAssessor::capacity])
            const noexcept;

        bool sameFrame (
            const InertChannelObservation (
                &canonical)[InertChannelAssessor::capacity],
            const CueOperatorInput& input) const noexcept;

        CueEvidenceGate evidenceGate (
            const InertChannelAssessment& assessment) const noexcept;

        void refreshSnapshot (TimePoint now) noexcept;

        void enterInputFault (TimePoint now, const CueOperatorInput& input,
                              Status status) noexcept;

        void updateDigest (
            TimePoint now,
            const InertChannelObservation (
                &canonical)[InertChannelAssessor::capacity],
            const CueOperatorInput& input) noexcept;

        void digestByte (uint8_t value) noexcept;
        void digestWord (uint32_t value) noexcept;

        InertCueChannelMap       map_;
        InertChannelAssessor&    assessor_;
        InertCueScheduler&       scheduler_;
        CueAuditBuffer&          audit_;
        InertShowSnapshot        snapshot_;
        InertChannelObservation  lastObservations_[InertChannelAssessor::capacity];
        CueOperatorInput         lastInput_;
        TimePoint                lastUpdatedAt_;
        uint32_t                 traceDigest_;
        bool                     hasLastFrame_;
        bool                     ownsAuditSession_;
        bool                     initialized_;
    };
} // namespace adk
