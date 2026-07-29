#pragma once

#include "pulse_input.h"
#include "status.h"

#include <stdint.h>

namespace adk {

    struct InertEscapeConsole;
    struct InertEscapeConsoleTestAccess;

    enum struct OperatorControl : uint8_t
    {
        None,
        Previous,
        Next,
        Select,
        Acknowledge
    };

    enum struct OperatorChordDisposition : uint8_t
    {
        None,
        SingleControl,
        InvalidChord,
        InvalidEvidence
    };

    enum struct PanelDiagnostic : uint8_t
    {
        None,
        InputRecovered,
        PresentationRecovered,
        ClueIncomplete,
        ClueInvalid,
        ClueStale,
        ClueContradictory,
        AuditPending,
        AuditIndeterminate,
        OperatorChordInvalid,
        SourceFault,
        ConfigurationFault,
        TimingFault,
        InternalFault,
        Stopped
    };

    enum struct PanelPresentationMode : uint8_t
    {
        Blank,
        Ready,
        Reviewing,
        ConfirmationRequired,
        Solved,
        Fault,
        Stopped
    };

    enum struct PanelAuditKind : uint8_t
    {
        None,
        AcknowledgedDiagnostic,
        PuzzleSolved,
        StopAsserted,
        StopReleased
    };

    enum struct PanelAuditSlotState : uint8_t
    {
        Empty,
        Prepared,
        Committed
    };

    enum struct PanelAuditDisposition : uint8_t
    {
        Empty,
        Ready,
        PrepareRequired,
        AcknowledgeRequired,
        Indeterminate,
        Corrupt
    };

    struct OperatorSourceIdentity
    {
        uint16_t sourceId;
        uint16_t configurationRevision;
        uint32_t sessionEpoch;
    };

    struct OperatorControlEvidence
    {
        uint8_t                pressedMask;
        OperatorSourceIdentity source;
        uint32_t               sourceSequence;
        MicrosecondTimePoint   observedAt;
        Status                 status;
    };

    struct OperatorStopEvidence
    {
        bool                   asserted;
        OperatorSourceIdentity source;
        uint32_t               sourceSequence;
        MicrosecondTimePoint   observedAt;
        Status                 status;
    };

    struct PanelPresentationIntent
    {
        PanelPresentationMode mode;
        PanelDiagnostic       diagnostic;
        uint8_t               selectedCell;
        uint32_t              diagnosticGeneration;
        bool                  acknowledgeAvailable;
    };

    struct PanelPresentationEvidence
    {
        uint32_t             intentGeneration;
        MicrosecondTimePoint observedAt;
        Status               status;
    };

    struct PanelAuditRecord
    {
        uint32_t               formatMagic;
        uint16_t               formatVersion;
        uint16_t               configurationRevision;
        uint32_t               instanceEpoch;
        uint32_t               recordSequence;
        uint32_t               operationId;
        PanelAuditKind         kind;
        PanelDiagnostic        diagnostic;
        uint32_t               diagnosticGeneration;
        uint16_t               parentConfigurationRevision;
        uint32_t               parentInstanceEpoch;
        uint32_t               parentGeneration;
        uint32_t               clueGeneration;
        uint16_t               satisfiedRuleMask;
        uint32_t               policyDigest;
        bool                   stopPresent;
        bool                   stopAsserted;
        OperatorSourceIdentity stopSource;
        uint32_t               stopSourceSequence;
        MicrosecondTimePoint   stopObservedAt;
        MicrosecondTimePoint   occurredAt;
        uint32_t               payloadDigest;
        uint32_t               checksum;
        PanelAuditSlotState    state;
    };

    struct PanelAuditImage
    {
        PanelAuditRecord slots[2];
    };

    struct PanelAuditPreview
    {
        uintptr_t        ownerToken;
        uint32_t         lifecycleGeneration;
        uint16_t         configurationRevision;
        uint32_t         instanceEpoch;
        uint32_t         panelGeneration;
        uint32_t         operationId;
        uint8_t          slotIndex;
        PanelAuditRecord record;
        uint32_t         imageDigest;
    };

    struct PanelAcknowledgePreview
    {
        uintptr_t         ownerToken;
        uint32_t          lifecycleGeneration;
        uint16_t          configurationRevision;
        uint32_t          instanceEpoch;
        uint32_t          panelGeneration;
        uint32_t          operationId;
        PanelDiagnostic   diagnostic;
        uint32_t          diagnosticGeneration;
        PanelAuditPreview audit;
    };

    struct FaultAwareOperatorPanelConfig
    {
        uint16_t               configurationRevision;
        uint32_t               instanceEpoch;
        MicrosecondDuration    maximumInputAge;
        uint8_t                selectableCellCount;
        OperatorSourceIdentity controlSource;
        OperatorSourceIdentity stopSource;
    };

    struct FaultAwareOperatorPanelInput
    {
        MicrosecondTimePoint      now;
        bool                      auditImagePresent;
        PanelAuditImage           auditImage;
        bool                      stopPresent;
        OperatorStopEvidence      stop;
        bool                      controlPresent;
        OperatorControlEvidence   control;
        bool                      diagnosticPresent;
        PanelDiagnostic           diagnostic;
        uint32_t                  diagnosticGeneration;
        bool                      auditAcknowledgePresent;
        PanelAuditPreview         auditAcknowledge;
        bool                      acknowledgePresent;
        PanelAcknowledgePreview   acknowledge;
        bool                      presentationPresent;
        PanelPresentationEvidence presentation;
    };

    struct FaultAwareOperatorPanelSnapshot
    {
        uint16_t                 configurationRevision;
        uint32_t                 instanceEpoch;
        uint32_t                 generation;
        bool                     stopped;
        uint8_t                  selectedCell;
        OperatorChordDisposition chordDisposition;
        PanelDiagnostic          diagnostic;
        uint32_t                 diagnosticGeneration;
        PanelAuditDisposition    auditDisposition;
        PanelPresentationIntent  presentation;
        Status                   status;
    };

    struct FaultAwareOperatorPanel
    {
        explicit FaultAwareOperatorPanel (
            const FaultAwareOperatorPanelConfig& config) noexcept;
        ~FaultAwareOperatorPanel () noexcept;

        FaultAwareOperatorPanel& operator= (const FaultAwareOperatorPanel&) = delete;
        FaultAwareOperatorPanel (const FaultAwareOperatorPanel&)            = delete;
        FaultAwareOperatorPanel& operator= (FaultAwareOperatorPanel&&)      = delete;
        FaultAwareOperatorPanel (FaultAwareOperatorPanel&&)                 = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        void   reset       () noexcept;
        bool   initialized () const noexcept;

        Result<PanelAuditPreview> prepareAudit (uint32_t             operationId,
                                                PanelAuditKind       kind,
                                                MicrosecondTimePoint now) noexcept;
        bool canAcknowledgeAudit (const PanelAuditPreview& preview) const noexcept;
        Result<PanelAcknowledgePreview>
        prepareAcknowledge (uint32_t operationId, MicrosecondTimePoint now) noexcept;
        Status update      (const FaultAwareOperatorPanelInput& input) noexcept;

        FaultAwareOperatorPanelSnapshot snapshot            () const noexcept;
        PanelAuditImage                 canonicalAuditImage () const noexcept;

      private:
        friend struct InertEscapeConsole;
        friend struct InertEscapeConsoleTestAccess;

        struct PreparedUpdate
        {
            FaultAwareOperatorPanelSnapshot snapshot;
            PanelAuditImage                 image;
            bool                            consumeAudit;
            bool                            consumeAcknowledge;
            bool                            invalidateCandidates;
            bool                            retainStop;
            OperatorStopEvidence            stop;
            bool                            retainControl;
            OperatorControlEvidence         control;
            bool                            stopTransitionPending;
            bool                            presentationFailureRetained;
            uint32_t                        failedIntentGeneration;
            MicrosecondTimePoint            now;
            bool                            retainNow;
            bool                            projectDiagnosticRetained;
            bool                            presentationOnlyDiagnostic;
            bool                            presentationPrimaryRetained;
            bool                            auditDiagnosticRetained;
        };

        struct ProjectUpdateView
        {
            MicrosecondTimePoint                 now;
            bool                                 auditImagePresent;
            const PanelAuditImage&               auditImage;
            bool                                 stopPresent;
            const OperatorStopEvidence&          stop;
            bool                                 controlPresent;
            const OperatorControlEvidence&       control;
            bool                                 diagnosticPresent;
            PanelDiagnostic                      diagnostic;
            uint32_t                             diagnosticGeneration;
            bool                                 auditAcknowledgePresent;
            const PanelAuditPreview&             auditAcknowledge;
            bool                                 acknowledgePresent;
            const PanelAcknowledgePreview&        acknowledge;
            bool                                 presentationPresent;
            const PanelPresentationEvidence&      presentation;
        };

        Result<PanelAuditPreview>
        preparePuzzleSolved (uint32_t operationId, uint16_t parentConfigurationRevision,
                             uint32_t parentInstanceEpoch, uint32_t parentGeneration,
                             uint32_t clueGeneration, uint16_t satisfiedRuleMask,
                             uint32_t policyDigest, MicrosecondTimePoint now) noexcept;
        Status preflightUpdate (const FaultAwareOperatorPanelInput& input,
                                PreparedUpdate& prepared) const noexcept;
        Status preflightProjectUpdate (
            MicrosecondTimePoint now, bool auditImagePresent,
            const PanelAuditImage& auditImage, bool stopPresent,
            const OperatorStopEvidence& stop, bool controlPresent,
            const OperatorControlEvidence& control,
            bool auditAcknowledgePresent,
            const PanelAuditPreview& auditAcknowledge,
            bool acknowledgePresent,
            const PanelAcknowledgePreview& acknowledge,
            bool presentationPresent,
            const PanelPresentationEvidence& presentation,
            PanelDiagnostic derivedDiagnostic, uint32_t derivedGeneration,
            bool puzzleSolveEligible, PreparedUpdate& prepared) const noexcept;
        template <typename Input> Status preflightUpdateInternal (
            const Input& input, bool projectOverride,
            PanelDiagnostic derivedDiagnostic, uint32_t derivedGeneration,
            bool puzzleSolveEligible, PreparedUpdate& prepared) const noexcept;
        void applyPreparedUpdate            (const PreparedUpdate& prepared) noexcept;
        void invalidatePreparedCandidates   () noexcept;
        bool solvePreparationEligible       () const noexcept;

        Status prepareRecord (
            uint32_t operationId, PanelAuditKind kind, PanelDiagnostic diagnostic,
            uint32_t diagnosticGeneration, uint16_t parentConfigurationRevision,
            uint32_t parentInstanceEpoch, uint32_t parentGeneration,
            uint32_t clueGeneration, uint16_t satisfiedRuleMask, uint32_t policyDigest,
            bool stopPresent, bool stopAsserted,
            const OperatorSourceIdentity& stopSource, uint32_t stopSourceSequence,
            MicrosecondTimePoint stopObservedAt, MicrosecondTimePoint now) noexcept;

        FaultAwareOperatorPanelConfig   config_;
        FaultAwareOperatorPanelSnapshot snapshot_;
        PanelAuditImage                 image_;
        PanelAuditPreview               auditCandidate_;
        OperatorStopEvidence            retainedStop_;
        OperatorControlEvidence         retainedControl_;
        MicrosecondTimePoint             retainedNow_;
        uint32_t                        lifecycleGeneration_;
        uint32_t                        failedIntentGeneration_;
        uint32_t                        acknowledgeOperationId_;
        bool initialized_                   : 1;
        bool auditCandidateLive_            : 1;
        bool acknowledgeCandidateLive_      : 1;
        bool stopRetained_                  : 1;
        bool controlRetained_               : 1;
        bool stopTransitionPending_         : 1;
        bool presentationFailureRetained_   : 1;
        bool nowRetained_                   : 1;
        bool lifecycleExhausted_            : 1;
        bool projectDiagnosticRetained_     : 1;
        bool presentationPrimaryRetained_  : 1;
        bool auditDiagnosticRetained_       : 1;
    };
} // namespace adk
