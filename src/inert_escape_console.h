#pragma once

#include "clue_constraint_model.h"
#include "fault_aware_operator_panel.h"

#include <stdint.h>

namespace adk {

    struct InertEscapeConsoleTestAccess;

    enum struct EscapeClueFamily : uint8_t
    {
        Sequence,
        Pattern,
        Orientation,
        Presence,
        Rhythm,
        Alignment
    };

    enum struct EscapeConsoleDisposition : uint8_t
    {
        Uninitialized,
        AwaitingClues,
        AwaitingOperator,
        AuditPending,
        Solved,
        Stopped,
        InvalidOperatorChord,
        AuditIndeterminate,
        InvalidEvidence,
        StaleEvidence,
        ContradictoryEvidence,
        SourceFault,
        ConfigurationFault,
        TimingFault,
        InternalFault
    };

    enum struct EscapeLatchIntent : uint8_t
    {
        Inactive,
        RequestDemonstrationRelease
    };

    enum struct EscapeLampIntent : uint8_t
    {
        Off,
        Ready,
        Progress,
        Confirmation,
        Solved,
        Fault,
        Stopped
    };

    struct EscapeFamilySnapshot
    {
        EscapeClueFamily family;
        uint8_t          firstClueId;
        uint8_t          secondClueId;
        bool             complete;
        ClueQuality      weakestQuality;
    };

    struct EscapeConsoleConfig
    {
        uint16_t                      configurationRevision;
        uint32_t                      instanceEpoch;
        ClueConstraintConfig          clueModel;
        FaultAwareOperatorPanelConfig panel;
        EscapeClueFamily              clueFamilies[12];
        uint32_t                      policyDigest;
    };

    struct EscapeConsolePreview
    {
        uintptr_t         ownerToken;
        uint32_t          lifecycleGeneration;
        uint16_t          configurationRevision;
        uint32_t          instanceEpoch;
        uint32_t          consoleGeneration;
        uint32_t          operationId;
        uint32_t          clueGeneration;
        uint16_t          satisfiedRuleMask;
        uint32_t          policyDigest;
        PanelAuditPreview audit;
    };

    struct EscapeConsoleUpdate
    {
        MicrosecondTimePoint      now;
        bool                      auditImagePresent;
        PanelAuditImage           auditImage;
        bool                      clueUpdatePresent;
        ClueConstraintUpdate      clueUpdate;
        bool                      stopPresent;
        OperatorStopEvidence      stop;
        bool                      controlPresent;
        OperatorControlEvidence   control;
        bool                      auditAcknowledgePresent;
        PanelAuditPreview         auditAcknowledge;
        bool                      acknowledgePresent;
        PanelAcknowledgePreview   acknowledge;
        bool                      presentationPresent;
        PanelPresentationEvidence presentation;
        bool                      solvePreviewPresent;
        EscapeConsolePreview      solvePreview;
    };

    struct EscapeConsoleSnapshot
    {
        uint16_t                 configurationRevision;
        uint32_t                 instanceEpoch;
        uint32_t                 generation;
        uint32_t                 operationId;
        EscapeFamilySnapshot     families[6];
        EscapeConsoleDisposition disposition;
        EscapeLatchIntent        latchIntent;
        EscapeLampIntent         lampIntent;
        PanelPresentationIntent  presentation;
        PanelAuditDisposition    auditDisposition;
        Status                   status;
    };

    struct InertEscapeConsole
    {
        explicit InertEscapeConsole (const EscapeConsoleConfig& config) noexcept;
        ~InertEscapeConsole         () noexcept;

        InertEscapeConsole& operator= (const InertEscapeConsole&) = delete;
        InertEscapeConsole (const InertEscapeConsole&)            = delete;
        InertEscapeConsole& operator= (InertEscapeConsole&&)      = delete;
        InertEscapeConsole (InertEscapeConsole&&)                 = delete;

        Status initialize  () noexcept;
        void   shutdown    () noexcept;
        void   reset       () noexcept;
        bool   initialized () const noexcept;

        Result<EscapeConsolePreview> prepareSolve (uint32_t             operationId,
                                                   MicrosecondTimePoint now) noexcept;
        Result<PanelAuditPreview> preparePanelAudit (uint32_t             operationId,
                                                     PanelAuditKind       kind,
                                                     MicrosecondTimePoint now) noexcept;
        Result<PanelAcknowledgePreview>
               preparePanelAcknowledge (uint32_t             operationId,
                                        MicrosecondTimePoint now) noexcept;
        bool   canCommit (const EscapeConsolePreview& preview) const noexcept;
        Status update    (const EscapeConsoleUpdate& input) noexcept;

        EscapeConsoleSnapshot           snapshot            () const noexcept;
        ClueConstraintSnapshot          clueSnapshot        () const noexcept;
        FaultAwareOperatorPanelSnapshot panelSnapshot       () const noexcept;
        PanelAuditImage                 canonicalAuditImage () const noexcept;

      private:
        friend struct InertEscapeConsoleTestAccess;

        bool advanceLifecycle () noexcept;
        void clearSnapshot    (EscapeConsoleDisposition disposition,
                               Status                   status) noexcept;
        EscapeConsoleDisposition retainedDisposition () const noexcept;
        void retainDisposition                       (EscapeConsoleDisposition disposition) noexcept;
        bool lifecycleExhausted                      () const noexcept;
        EscapeClueFamily familyFor                   (uint8_t clueId) const noexcept;
        void summarizeFamilies                       (EscapeConsoleSnapshot& snapshot) const noexcept;

        ClueConstraintModel     clueModel_;
        FaultAwareOperatorPanel panel_;
        uint16_t                configurationRevision_;
        uint32_t                instanceEpoch_;
        uint32_t                policyDigest_;
        uint32_t                lifecycleGeneration_;
        uint32_t                operationId_;
        uint8_t                 packedFamilies_[5];
    };
} // namespace adk
