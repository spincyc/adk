#pragma once

#include "pulse_input.h"
#include "status.h"

#include <stdint.h>

namespace adk {

    enum struct OneWireSupplyMode : uint8_t
    {
        ExternallyPowered,
        ParasitePower
    };

    enum struct OneWireOperation : uint8_t
    {
        ResetPresence,
        SearchRomPass,
        ReadRomSingleDrop,
        MatchRomReadPowerSupply,
        MatchRomStartConversion,
        MatchRomReadConversionStatus,
        MatchRomReadScratchpad
    };

    enum struct OneWirePhase : uint8_t
    {
        Inert,
        ResetLow,
        PresenceWindow,
        WriteSlot,
        ReadSlot,
        Complete,
        RollingBack,
        Fault
    };

    enum struct OneWireLineIntent : uint8_t
    {
        Release,
        DriveLow,
        Sample
    };

    enum struct OneWireTransactionQuality : uint8_t
    {
        Unqualified,
        Pending,
        Complete,
        NoPresence,
        Collision,
        TimedOut,
        ProducerFault,
        ReleaseUnconfirmed
    };

    struct OneWireRomCode
    {
        uint8_t bytes[8];
    };

    struct OneWireTransactionConfig
    {
        uint32_t            ownerToken;
        uint16_t            configurationRevision;
        uint8_t             expectedReceiptSourceId;
        uint16_t            expectedReceiptConfigurationRevision;
        bool                singleDrop;
        MicrosecondDuration resetLowMinimum;
        MicrosecondDuration resetLowMaximum;
        MicrosecondDuration resetReleaseMinimum;
        MicrosecondDuration resetReleaseMaximum;
        MicrosecondDuration presenceStartMinimum;
        MicrosecondDuration presenceStartMaximum;
        MicrosecondDuration presenceLowMinimum;
        MicrosecondDuration presenceLowMaximum;
        MicrosecondDuration writeZeroLowMinimum;
        MicrosecondDuration writeZeroLowMaximum;
        MicrosecondDuration writeOneLowMinimum;
        MicrosecondDuration writeOneLowMaximum;
        MicrosecondDuration readInitiateMinimum;
        MicrosecondDuration readInitiateMaximum;
        MicrosecondDuration readSampleMinimum;
        MicrosecondDuration readSampleMaximum;
        MicrosecondDuration completeSlotMinimum;
        MicrosecondDuration completeSlotMaximum;
        MicrosecondDuration slotRecoveryMinimum;
        MicrosecondDuration slotRecoveryMaximum;
        MicrosecondDuration transactionDeadline;
        uint16_t            maximumSlots;
    };

    struct OneWireSearchState
    {
        OneWireRomCode rom;
        uint8_t        lastDiscrepancy;
        bool           lastDevice;
    };

    struct OneWireOperationRequest
    {
        uint32_t             requestSequence;
        OneWireOperation     operation;
        OneWireRomCode       addressedRom;
        OneWireSearchState   search;
        MicrosecondTimePoint startedAt;
        OneWireSupplyMode    supplyMode;
        Status               status;
    };

    struct OneWireStepIntent
    {
        uint32_t             ownerToken;
        uint32_t             lifecycleGeneration;
        uint16_t             configurationRevision;
        uint32_t             requestSequence;
        uint32_t             transactionGeneration;
        OneWireOperation     operation;
        OneWirePhase         phase;
        uint32_t             phaseSequence;
        uint16_t             slotIndex;
        bool                 writeBit;
        OneWireLineIntent    lineIntent;
        bool                 sampleRequired;
        MicrosecondTimePoint earliestAt;
        MicrosecondTimePoint latestAt;
        OneWireRomCode       addressedRom;
    };

    struct OneWireStepReceipt
    {
        uint8_t              sourceId;
        uint16_t             configurationRevision;
        uint32_t             sequence;
        MicrosecondTimePoint observedAt;
        uint32_t             ownerToken;
        uint32_t             lifecycleGeneration;
        uint32_t             requestSequence;
        uint32_t             transactionGeneration;
        OneWireOperation     operation;
        OneWirePhase         phase;
        uint32_t             phaseSequence;
        uint16_t             slotIndex;
        OneWireLineIntent    appliedIntent;
        bool                 sampledHigh;
        bool                 accepted;
        Status               status;
    };

    struct OneWireTransactionSnapshot
    {
        OneWireOperation          operation;
        OneWirePhase              phase;
        OneWireTransactionQuality quality;
        OneWireOperationRequest   request;
        OneWireSearchState        searchResult;
        OneWireRomCode            returnedRom;
        uint8_t                   readBytes[9];
        uint8_t                   readByteCount;
        uint16_t                  acceptedSlotCount;
        bool                      presenceSeen;
        bool                      releaseRequested;
        bool                      releaseConfirmed;
        MicrosecondTimePoint      completedAt;
        Status                    status;
        uint32_t                  ownerToken;
        uint32_t                  lifecycleGeneration;
        uint16_t                  configurationRevision;
        uint32_t                  transactionGeneration;
    };

    struct OneWireTransactionPolicy
    {
        explicit OneWireTransactionPolicy (
            const OneWireTransactionConfig& config) noexcept;

        OneWireTransactionPolicy (const OneWireTransactionPolicy&)            = delete;
        OneWireTransactionPolicy& operator= (const OneWireTransactionPolicy&) = delete;
        OneWireTransactionPolicy (OneWireTransactionPolicy&&)                 = delete;
        OneWireTransactionPolicy& operator= (OneWireTransactionPolicy&&)      = delete;

        Status initialize (MicrosecondTimePoint now,
                           OneWireStepIntent&   releaseIntent) noexcept;
        Status begin (MicrosecondTimePoint now, const OneWireOperationRequest& request,
                      OneWireStepIntent& intent) noexcept;
        Status update (MicrosecondTimePoint now, const OneWireStepReceipt& receipt,
                       OneWireStepIntent& intent) noexcept;
        Status advance (MicrosecondTimePoint now, OneWireStepIntent& intent) noexcept;

        Status cancel (MicrosecondTimePoint now, OneWireStepIntent& intent) noexcept;

        Status reset (MicrosecondTimePoint now,
                      OneWireStepIntent&   releaseIntent) noexcept;
        Status shutdown (MicrosecondTimePoint now,
                         OneWireStepIntent&   releaseIntent) noexcept;
        Status confirmCleanup (MicrosecondTimePoint      now,
                               const OneWireStepReceipt& receipt) noexcept;

        Status snapshot          (
            OneWireTransactionSnapshot& snapshot) const noexcept;
        Status completedEvidence (
            OneWireTransactionSnapshot& evidence) const noexcept;

        bool   initialized () const noexcept;

#if defined(ADK_TESTING)
        void seedSequencesForTest (uint32_t lifecycleGeneration,
                                   uint32_t transactionGeneration,
                                   uint32_t phaseSequence) noexcept;
#endif

      private:
        enum struct Step : uint8_t
        {
            Cleanup,
            ResetDrive,
            ResetRelease,
            PresenceSample,
            PresenceRelease,
            SlotDrive,
            SlotSample,
            SlotRelease,
            SlotComplete,
            SlotRecovery
        };

        Status validateConfig () const noexcept;

        Status startCleanup (MicrosecondTimePoint now, OneWireStepIntent& intent,
                             bool closing) noexcept;
        Status completeReleased (MicrosecondTimePoint now,
                                 OneWireStepIntent&   intent) noexcept;
        Status emitCurrent (MicrosecondTimePoint now,
                            OneWireStepIntent&   intent) const noexcept;
        Status fail (MicrosecondTimePoint now, OneWireTransactionQuality quality,
                     Status status, OneWireStepIntent& intent,
                     bool receiptTriggered = false) noexcept;
        bool validProgression   (MicrosecondTimePoint now) const noexcept;
        bool addressedOperation () const noexcept;

        bool currentWriteBit () const noexcept;

        bool receiptMatches (const OneWireStepReceipt& receipt) const noexcept;

        void   clearSnapshot () noexcept;

        OneWireTransactionConfig   config_;
        OneWireTransactionSnapshot snapshot_;
        MicrosecondTimePoint       lifecycleStartedAt_;
        MicrosecondTimePoint       phaseStartedAt_;
        MicrosecondTimePoint       slotStartedAt_;
        MicrosecondTimePoint       lastAcceptedAt_;
        uint32_t                   lifecycleGeneration_;
        uint32_t                   transactionGeneration_;
        uint32_t                   phaseSequence_;
        OneWireStepReceipt         lastReceipt_;
        uint16_t                   slotIndex_;
        OneWireTransactionQuality  terminalQuality_;
        Status                     terminalStatus_;
        uint8_t                    searchLastDiscrepancy_;
        Step                       step_;
        uint8_t                    flags_;
    };
} // namespace adk
