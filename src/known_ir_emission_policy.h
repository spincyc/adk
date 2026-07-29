#pragma once

#include "pulse_input.h"
#include "status.h"

#include <stdint.h>

namespace adk {

    enum struct LocalIrCodeId : uint8_t
    {
        StationPing        = 0,
        StationReady       = 1,
        StationCancel      = 2,
        StationAcknowledge = 3
    };

    enum struct IrEnvelopeIntent : uint8_t
    {
        Inactive   = 0,
        CarrierOn  = 1,
        CarrierOff = 2
    };

    enum struct IrEmissionDisposition : uint8_t
    {
        Idle      = 0,
        Prepared  = 1,
        Active    = 2,
        Complete  = 3,
        Cancelled = 4,
        Fault     = 5,
        Shutdown  = 6
    };

    struct KnownIrCatalogIdentity
    {
        uint16_t revision;
        uint32_t digest;
    };

    struct KnownIrEmissionConfig
    {
        uint16_t            configurationRevision;
        uint32_t            instanceEpoch;
        MicrosecondDuration maximumEnvelopeDuration;
    };

    struct KnownIrEmissionPreview
    {
        const void*            owner;
        uint16_t               configurationRevision;
        uint32_t               instanceEpoch;
        uint32_t               policyGeneration;
        uint32_t               candidateGeneration;
        uint32_t               transactionId;
        LocalIrCodeId          codeId;
        KnownIrCatalogIdentity catalog;
        uint32_t               candidateDigest;
        MicrosecondTimePoint   startAt;
        MicrosecondTimePoint   completeAt;
        IrEnvelopeIntent       firstIntent;
    };

    enum struct IrEmissionTerminalCause : uint8_t
    {
        None                  = 0,
        Completed             = 1,
        CancelledBeforeCommit = 2,
        CancelledActive       = 3,
        ShutdownBeforeCommit  = 4,
        ShutdownActive        = 5,
        Faulted               = 6
    };

    struct KnownIrEmissionSnapshot
    {
        uint16_t                configurationRevision;
        uint32_t                instanceEpoch;
        uint32_t                policyGeneration;
        uint32_t                candidateGeneration;
        LocalIrCodeId           codeId;
        KnownIrCatalogIdentity  catalog;
        uint32_t                transactionId;
        MicrosecondTimePoint    startAt;
        MicrosecondTimePoint    completeAt;
        uint8_t                 repeatIndex;
        IrEnvelopeIntent        intent;
        IrEmissionDisposition   disposition;
        IrEmissionTerminalCause terminalCause;
        uint32_t                terminalTransactionId;
        MicrosecondTimePoint    terminalAt;
        Status                  status;
    };

    struct KnownIrEmissionPolicy
    {
        explicit KnownIrEmissionPolicy (const KnownIrEmissionConfig& config) noexcept;

        KnownIrEmissionPolicy (const KnownIrEmissionPolicy&)            = delete;
        KnownIrEmissionPolicy& operator= (const KnownIrEmissionPolicy&) = delete;
        KnownIrEmissionPolicy (KnownIrEmissionPolicy&&)                 = delete;
        KnownIrEmissionPolicy& operator= (KnownIrEmissionPolicy&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        void   reset      () noexcept;

        Result<KnownIrEmissionPreview> prepare (LocalIrCodeId        codeId,
                                                uint32_t             transactionId,
                                                MicrosecondTimePoint now) noexcept;
        bool   canCommit (const KnownIrEmissionPreview& preview,
                          MicrosecondTimePoint          now) const noexcept;
        Status commit (const KnownIrEmissionPreview& preview,
                       MicrosecondTimePoint          now) noexcept;
        Status cancel (const KnownIrEmissionPreview& preview,
                       MicrosecondTimePoint          now) noexcept;
        Status cancel (uint32_t transactionId, MicrosecondTimePoint now) noexcept;
        Status update (MicrosecondTimePoint now) noexcept;

        KnownIrEmissionSnapshot snapshot () const noexcept;

      private:
        bool validConfig  () const noexcept;
        bool validPreview (const KnownIrEmissionPreview& preview) const noexcept;
        bool validTime    (MicrosecondTimePoint now) const noexcept;
        void rememberTime (MicrosecondTimePoint now) noexcept;
        void clearToIdle  () noexcept;
        void finish       (IrEmissionDisposition disposition, IrEmissionTerminalCause cause,
                     MicrosecondTimePoint now) noexcept;
        IrEnvelopeIntent intentAt (LocalIrCodeId       codeId,
                                   MicrosecondDuration elapsed) const noexcept;

        KnownIrEmissionConfig   config_;
        KnownIrEmissionSnapshot snapshot_;
        uint32_t                candidateDigest_;
        MicrosecondTimePoint    lastTime_;
        uint32_t                nextPolicyGeneration_;
        uint32_t                nextCandidateGeneration_;
        bool                    initialized_;
        bool                    hasTime_;
    };
} // namespace adk
