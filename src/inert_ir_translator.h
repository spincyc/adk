#pragma once

#include "captured_ir_evidence.h"
#include "known_ir_emission_policy.h"

#include <stdint.h>

namespace adk {

    struct SyntheticIrReceiveFixture
    {
        InfraredProtocol protocol;
        uint32_t         address;
        uint32_t         command;
        LocalIrCodeId    receivedCode;
        LocalIrCodeId    translatedCode;
        bool             transmissionAllowed;
    };

    static constexpr uint16_t syntheticIrMappingRevision = 1;
    static constexpr uint8_t  syntheticIrFixtureCount    = 4;
    static constexpr uint32_t syntheticIrMappingDigest   = UINT32_C (0xa8f94d6b);

    extern const IrSourceIdentity syntheticIrReceiveSource;
    extern const SyntheticIrReceiveFixture
        syntheticIrReceiveFixtures[syntheticIrFixtureCount];

    enum struct IrTranslationDisposition : uint8_t
    {
        Idle                  = 0,
        Translated            = 1,
        Cancelled             = 2,
        RepeatRejected        = 3,
        UnknownObserved       = 4,
        MalformedObserved     = 5,
        SelfEchoSuppressed    = 6,
        UnlistedValidObserved = 7,
        AttributionMismatch   = 8,
        ReceiveTimeout        = 9,
        Fault                 = 10
    };

    struct IrEmitterEvidence
    {
        IrSourceIdentity     source;
        uint32_t             transactionId;
        MicrosecondTimePoint startedAt;
        MicrosecondTimePoint completedAt;
        Status               status;
    };

    struct IrReceiveEnvelope
    {
        IrSourceIdentity     source;
        Status               sourceStatus;
        MicrosecondTimePoint observedAt;
        PulseFrame           frame;
    };

    struct IrTranslatorConfig
    {
        uint16_t            configurationRevision;
        uint32_t            instanceEpoch;
        uint32_t            mappingDigest;
        MicrosecondDuration maximumEnvelopeDuration;
        IrSourceIdentity    qualifiedReceiveSource;
        IrSourceIdentity    localEmitterSource;
        MicrosecondDuration echoGuard;
        MicrosecondDuration responseWindow;
    };

    struct IrTranslatorPreview
    {
        const void*            owner;
        uint32_t               instanceEpoch;
        uint16_t               configurationRevision;
        uint32_t               mappingDigest;
        KnownIrCatalogIdentity emissionCatalog;
        uint32_t               parentGeneration;
        uint32_t               operationId;
        uint32_t               inputDigest;
        uint32_t               evidenceGeneration;
        CapturedIrProvenance   receiveProvenance;
        LocalIrCodeId          receivedCode;
        LocalIrCodeId          transmitCode;
        KnownIrEmissionPreview emission;
    };

    struct IrRoundTripResult
    {
        bool                     complete;
        uint32_t                 operationId;
        LocalIrCodeId            transmittedCode;
        KnownIrCatalogIdentity   emissionCatalog;
        IrSourceIdentity         transmitSource;
        MicrosecondTimePoint     actualStartedAt;
        MicrosecondTimePoint     actualCompletedAt;
        MicrosecondDuration      elapsed;
        IrCaptureDisposition     receiveDisposition;
        EvidenceStrength         receiveStrength;
        CapturedIrProvenance     receiveProvenance;
        uint32_t                 receiveEvidenceGeneration;
        IrTranslationDisposition correlationDisposition;
        Status                   status;
    };

    struct IrTranslatorSnapshot
    {
        uint32_t                 operationId;
        LocalIrCodeId            receivedCode;
        LocalIrCodeId            transmitCode;
        CapturedIrProvenance     receiveProvenance;
        IrSourceIdentity         transmitSource;
        uint32_t                 transmitTransactionId;
        IrTranslationDisposition disposition;
        uint32_t                 suppressedEchoCount;
        IrEnvelopeIntent         transmitIntent;
        IrRoundTripResult        roundTrip;
        Status                   status;
    };

    struct IrTranslatorUpdateInput
    {
        MicrosecondTimePoint now;
        bool                 cancelPresent;
        uint32_t             cancelOperationId;
        bool                 commitPresent;
        IrTranslatorPreview  commitPreview;
        bool                 receivePresent;
        IrReceiveEnvelope    receive;
        bool                 actualEmissionPresent;
        IrEmitterEvidence    actualEmission;
    };

    struct InertIrTranslator
    {
        InertIrTranslator (const IrTranslatorConfig& config,
                           IrPulseStorage            pulseStorage) noexcept;
        ~InertIrTranslator () noexcept;

        InertIrTranslator (const InertIrTranslator&)            = delete;
        InertIrTranslator& operator= (const InertIrTranslator&) = delete;
        InertIrTranslator (InertIrTranslator&&)                 = delete;
        InertIrTranslator& operator= (InertIrTranslator&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        void   reset      () noexcept;

        Result<IrTranslatorPreview>
        prepareTranslation (uint32_t operationId, MicrosecondTimePoint now) noexcept;
        bool   canCommit   (const IrTranslatorPreview& preview,
                          MicrosecondTimePoint       now) const noexcept;
        Status update (const IrTranslatorUpdateInput& input) noexcept;

        IrTranslatorSnapshot    snapshot         () const noexcept;
        CapturedIrSnapshot      receiveSnapshot  () const noexcept;
        KnownIrEmissionSnapshot emissionSnapshot () const noexcept;

      private:
        bool validConfiguration () const noexcept;
        bool validUpdate        (const IrTranslatorUpdateInput& input) const noexcept;
        bool validPreview       (const IrTranslatorPreview& preview,
                           MicrosecondTimePoint       now) const noexcept;
        void classifyReceive  () noexcept;
        void correlateReceive () noexcept;
        void clearCandidate   () noexcept;

        IrTranslatorConfig    config_;
        KnownIrEmissionConfig emissionConfig_;
        InfraredDecoder       decoder_;
        CapturedIrEvidence    evidence_;
        KnownIrEmissionPolicy emission_;
        IrTranslatorSnapshot  snapshot_;
        IrTranslatorPreview   candidate_;
        IrEmitterEvidence     actualEmission_;
        uint32_t              generation_;
        MicrosecondTimePoint  lastUpdateAt_;
        bool                  initialized_;
        bool                  pulseStorageValid_;
        bool                  hasUpdateTime_;
        bool                  hasCandidate_;
        bool                  hasActualEmission_;
        bool                  terminalCancelled_;
    };
} // namespace adk
