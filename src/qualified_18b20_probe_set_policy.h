#pragma once

#include "one_wire_transaction_policy.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct Ds18b20Resolution : uint8_t
    {
        Bits9,
        Bits10,
        Bits11,
        Bits12
    };

    enum struct Ds18b20ProbeQuality : uint8_t
    {
        Unqualified,
        ConversionPending,
        Current,
        ScratchpadCrcFault,
        ResolutionMismatch,
        ResetDefaultWithoutConversion,
        ImplausibleStep,
        Stale,
        Missing,
        DuplicateIdentity,
        TransportFault
    };

    enum struct Ds18b20SetQuality : uint8_t
    {
        Unqualified,
        Complete,
        TransportFault,
        DuplicateIdentity,
        UnknownIdentity,
        Missing
    };

    struct Ds18b20NormalizedTransactionRef
    {
        uint32_t             requestSequence;
        uint32_t             transactionGeneration;
        MicrosecondTimePoint startedAt;
        MicrosecondTimePoint completedAt;
    };

    struct Ds18b20NormalizedSearchPass
    {
        Ds18b20NormalizedTransactionRef transaction;
        OneWireSearchState              requestSearch;
        OneWireSearchState              completedSearch;
    };

    struct Ds18b20NormalizedProbeWitness
    {
        OneWireRomCode                    rom;
        uint32_t                          conversionGeneration;
        Ds18b20NormalizedTransactionRef   conversionStart;
        Ds18b20NormalizedTransactionRef   conversionStatus;
        Ds18b20NormalizedTransactionRef   scratchpadRead;
        TimePoint                         scratchpadObservedAt;
        uint8_t                           scratchpad[9];
        bool                              conversionCompletedHigh;
        bool                              conversionStatusPresent;
    };

    struct Qualified18B20ProbeSetPolicy;

    struct Ds18b20CycleBuilder
    {
        Ds18b20CycleBuilder () noexcept;

        Ds18b20CycleBuilder (const Ds18b20CycleBuilder&)            = delete;
        Ds18b20CycleBuilder& operator= (const Ds18b20CycleBuilder&) = delete;
        Ds18b20CycleBuilder (Ds18b20CycleBuilder&&)                 = delete;
        Ds18b20CycleBuilder& operator= (Ds18b20CycleBuilder&&)      = delete;

      private:
        friend struct Qualified18B20ProbeSetPolicy;

        uint8_t                       sourceId;
        uint16_t                      configurationRevision;
        uint32_t                      cycleSequence;
        TimePoint                     observedAt;
        uint32_t                      policyGeneration;
        uint32_t                      oneWireOwnerToken;
        uint32_t                      oneWireLifecycleGeneration;
        uint16_t                      oneWireConfigurationRevision;
        Ds18b20NormalizedSearchPass   searchPasses[4];
        Ds18b20NormalizedProbeWitness probes[4];
        uint8_t                       searchPassCount;
        uint8_t                       probeCount;
        uint8_t                       lastTransactionTag;
        bool                          cycleBegun;
        bool                          searchFinished;
        bool                          searchComplete;
        bool                          searchOverCapacity;
        Status                        status;
    };

    struct Ds18b20ProbeConfig
    {
        OneWireRomCode    rom;
        Ds18b20Resolution resolution;
        Duration          conversionDeadline;
        Duration          maximumAge;
        int16_t           minimumRawSixteenths;
        int16_t           maximumRawSixteenths;
        uint16_t          maximumStepRawSixteenths;
    };

    struct QualifiedDs18b20Probe
    {
        OneWireRomCode      rom;
        uint32_t            cycleSequence;
        uint32_t            conversionGeneration;
        uint32_t            readTransactionGeneration;
        TimePoint           observedAt;
        TimePoint           freshThrough;
        int16_t             rawSixteenths;
        int16_t             lowerRawSixteenths;
        int16_t             upperRawSixteenths;
        Ds18b20Resolution   resolution;
        Ds18b20ProbeQuality quality;
        Duration            age;
        Status              status;
    };

    struct QualifiedDs18b20Snapshot
    {
        uint8_t               sourceId;
        uint16_t              configurationRevision;
        uint32_t              cycleSequence;
        TimePoint             observedAt;
        QualifiedDs18b20Probe probes[4];
        uint8_t               validCount;
        uint8_t               presentMask;
        uint8_t               faultMask;
        Ds18b20SetQuality     quality;
        Status                status;
    };

    struct QualifiedDs18b20SetConfig
    {
        uint8_t            expectedSourceId;
        uint16_t           expectedConfigurationRevision;
        uint32_t           expectedOneWireOwnerToken;
        uint16_t           expectedOneWireConfigurationRevision;
        Ds18b20ProbeConfig probes[4];
    };

    struct Qualified18B20ProbeSetPolicy
    {
        explicit Qualified18B20ProbeSetPolicy (
            const QualifiedDs18b20SetConfig& config) noexcept;

        Qualified18B20ProbeSetPolicy (
            const Qualified18B20ProbeSetPolicy&) = delete;
        Qualified18B20ProbeSetPolicy&
        operator= (const Qualified18B20ProbeSetPolicy&)                       = delete;
        Qualified18B20ProbeSetPolicy (Qualified18B20ProbeSetPolicy&&)         = delete;
        Qualified18B20ProbeSetPolicy&
        operator= (Qualified18B20ProbeSetPolicy&&)                            = delete;

        Status initialize () noexcept;
        void   reset      () noexcept;
        Status beginCycle (TimePoint now, uint8_t sourceId,
                           uint16_t configurationRevision,
                           uint32_t cycleSequence, TimePoint observedAt,
                           Ds18b20CycleBuilder& builder) const noexcept;
        Status ingestSearchPass (
            Ds18b20CycleBuilder&              builder,
            const OneWireTransactionSnapshot& transaction,
            const OneWireSearchState&          requestSearch) const noexcept;
        Status finishSearch (Ds18b20CycleBuilder& builder, bool searchComplete,
                             bool searchOverCapacity,
                             Status producerStatus) const noexcept;
        Status ingestConversionStart (
            Ds18b20CycleBuilder&              builder,
            uint32_t                          conversionGeneration,
            const OneWireTransactionSnapshot& transaction) const noexcept;
        Status ingestConversionStatus (
            Ds18b20CycleBuilder&              builder,
            uint32_t                          conversionGeneration,
            const OneWireTransactionSnapshot& transaction) const noexcept;
        Status ingestScratchpad (
            Ds18b20CycleBuilder&              builder,
            uint32_t                          conversionGeneration,
            TimePoint                         scratchpadObservedAt,
            const OneWireTransactionSnapshot& transaction) const noexcept;
        Status finalizeCycle (
            TimePoint now, const Ds18b20CycleBuilder& builder,
            QualifiedDs18b20Snapshot& snapshot) noexcept;

        Status snapshot    (QualifiedDs18b20Snapshot& snapshot) const noexcept;
        bool   initialized () const noexcept;

      private:
        QualifiedDs18b20SetConfig config_;
        QualifiedDs18b20Snapshot  snapshot_;
        Ds18b20CycleBuilder       lastCycle_;
        uint32_t                  policyGeneration_;
        bool                      initialized_;
        bool                      hasLastCycle_;
    };
} // namespace adk
