#pragma once

#include "bounded_homing_policy.h"
#include "bounded_stepper_sequence.h"
#include "local_identity_registry.h"

#include <stdint.h>

namespace adk {
    // clang-format off
    enum struct CarouselPhase : uint8_t
    {
        Uninitialized,
        Idle,
        AwaitingConfirmation,
        Homing,
        Positioning,
        ReadyAtBin,
        GateIntent,
        Complete,
        Cancelled,
        Stopped,
        Fault
    };

    enum struct CarouselFault : uint8_t
    {
        None,
        InvalidFrame,
        EvidenceFault,
        TimingFault,
        IdentityUnknown,
        IdentityLocked,
        IdentityStorageFault,
        ConfirmationConflict,
        AuthorizationExpired,
        HomingFault,
        PositionFault,
        GateFault,
        AuditFull,
        AuditCorrupt,
        AuditUnsupported,
        AuditIndeterminate,
        AuditStorageFault,
        PresentationFault
    };

    enum struct CarouselAuditStatus : uint8_t
    {
        Success,
        Cancelled,
        Stopped,
        AuthorizationExpired,
        EvidenceFault,
        TimingFault,
        IdentityFault,
        ConfirmationConflict,
        HomingFault,
        PositionFault,
        GateFault,
        AuditCorrupt,
        AuditUnsupported,
        AuditIndeterminate,
        AuditStorageFault,
        PresentationFault,
        RecoveredInterrupted
    };

    struct CarouselConfig
    {
        uint8_t  binCount;
        int32_t  binPositions[maximumCarouselBins];
        uint32_t projectConfigurationId;
        uint8_t  confirmationDigits;
        uint16_t binConfirmationCodes[maximumCarouselBins];
        uint8_t  confirmKey;
        uint8_t  cancelKey;
        uint8_t  digitKeys[10];
        Duration confirmationWindow;
        Duration gateIntentDuration;
        Duration logicalStepInterval;
        Duration maximumStepCommandAge;
        Duration maximumFrameAge;
        Duration maximumInputSkew;
    };

    struct CarouselInputFrame
    {
        TimePoint                observedAt;
        uint32_t                 sequence;
        bool                     hasIdentity;
        IdentityEvidence         identity;
        bool                     hasKey;
        CopiedKeyBatch           key;
        CopiedBinaryEvidence     home;
        CopiedBinaryEvidence     stop;
        CopiedPresentationStatus presentation;
    };

    enum struct CarouselGateIntent : uint8_t
    {
        Closed,
        Open
    };

    struct CarouselIntent
    {
        uint8_t             coilBits;
        CarouselGateIntent  gate;
        TimePoint           gateExpiresAt;
        uint8_t             selectedBin;
        CarouselAuditStatus statusCode;
    };

    struct CarouselAuditRecord
    {
        uint16_t            magic;
        uint8_t             schemaVersion;
        uint8_t             encodedLength;
        uint32_t            projectConfigurationId;
        uint32_t            operationId;
        uint16_t            authorizationEpoch;
        uint16_t            recordSequence;
        TimePoint           occurredAt;
        uint8_t             recordKind;
        CarouselPhase       phase;
        uint8_t             binId;
        uint16_t            identityDigest;
        uint16_t            bindingRevision;
        uint32_t            identityImageGeneration;
        uint32_t            homeEpoch;
        CarouselAuditStatus auditStatus;
        uint16_t            checksum;
    };

    static constexpr uint16_t carouselAuditMagic       = 0x4341;
    static constexpr uint8_t  carouselAuditVersion     = 1;
    static constexpr uint8_t  carouselAuditMaximum     = 8;
    static constexpr uint8_t  carouselAuditRecordBytes = 40;

    struct DurableAuditCandidate
    {
        uint32_t owner;
        uint32_t generation;
        uint32_t operationId;
        uint8_t  slot;
        uint8_t  recordKind;
        uint16_t checksum;
    };

    struct AuditRecordView
    {
        const uint8_t* bytes;
        uint8_t        length;
        uint8_t        slot;
        uint32_t       operationId;
    };

    struct AuditDurableCommitEvidence
    {
        uint32_t        owner;
        uint32_t        generation;
        uint32_t        operationId;
        uint8_t         slot;
        AuditRecordView reconciledRecord;
        bool            synchronized;
        bool            rereadValidated;
        Status          durableStatus;
    };

    struct CarouselSnapshot
    {
        CarouselPhase       phase;
        CarouselFault       fault;
        IdentityDisposition identityDisposition;
        HomingFault         homingFault;
        uint8_t             requestedBin;
        bool                authorizationCurrent;
        bool                positionKnown;
        int32_t             logicalPosition;
        CarouselIntent      intent;
        bool                hasAuditRecord;
        CarouselAuditRecord auditRecord;
        bool                durableAdmissionPending;
        bool                terminalReconciliationPending;
        uint32_t            operationId;
        Status              status;
    };

    struct InertPartsCarousel
    {
        InertPartsCarousel (const CarouselConfig&  config,
                            LocalIdentityRegistry& identityRegistry,
                            BoundedHomingPolicy& homingPolicy, uint8_t* auditSlotBytes,
                            uint16_t auditSlotByteExtent, uint8_t auditSlotStride,
                            uint8_t auditCapacity, uint8_t* auditCandidateBytes,
                            uint8_t auditCandidateCapacity) noexcept;

        InertPartsCarousel (const InertPartsCarousel&)            = delete;
        InertPartsCarousel& operator= (const InertPartsCarousel&) = delete;
        InertPartsCarousel (InertPartsCarousel&&)                 = delete;
        InertPartsCarousel& operator= (InertPartsCarousel&&)      = delete;

        Status initialize                               () noexcept;
        void   shutdown                                 () noexcept;
        Status update                                   (TimePoint now, const CarouselInputFrame& frame) noexcept;
        Result<DurableAuditCandidate> previewAuditWrite () const noexcept;
        Result<AuditRecordView>
        previewAuditExport (const DurableAuditCandidate& candidate) const noexcept;
        Status
        acknowledgeAuditWrite (const DurableAuditCandidate&      candidate,
                               const AuditDurableCommitEvidence& evidence) noexcept;
        CarouselSnapshot snapshot () const noexcept;

      private:
        void prepareTerminal (TimePoint now, CarouselPhase phase,
                              CarouselAuditStatus auditStatus, CarouselFault fault,
                              Status status) noexcept;

        const CarouselConfig*  config_;
        LocalIdentityRegistry* identity_;
        BoundedHomingPolicy*   homing_;
        uint8_t*               auditBytes_;
        uint8_t*               candidateBytes_;
        BoundedStepperSequence stepper_;
        HomingPreview          pendingHoming_;
        StepperCommand         pendingStepCommand_;
        CarouselSnapshot       snapshot_;
        DurableAuditCandidate  candidate_;
        TimePoint              confirmationStartedAt_;
        TimePoint              lastAcceptedAt_;
        TimePoint              cancellationAt_;
        uint16_t               identityDigest_;
        uint16_t               bindingRevision_;
        uint32_t               identityImageGeneration_;
        uint32_t               owner_;
        uint32_t               candidateGeneration_;
        uint32_t               lastFrameSequence_;
        uint16_t               nextRecordSequence_;
        uint16_t               authorizationEpoch_;
        uint16_t               auditExtent_;
        uint8_t                lastInstalledBytes_[carouselAuditRecordBytes];
        uint8_t                confirmationDigits_[4];
        uint8_t                auditStride_;
        uint8_t                auditCapacity_;
        uint8_t                candidateCapacity_;
        uint8_t                auditCount_;
        uint8_t                confirmationDigitCount_;
        bool                   initialized_;
        bool                   hasLastFrame_;
        bool                   candidatePending_;
        bool                   reconciliationRequired_;
        bool                   admissionDurable_;
        bool                   homingStepPending_;
        bool                   startCancellationPending_;
        bool                   shutdownCancellation_;
    };
    // clang-format on
} // namespace adk
