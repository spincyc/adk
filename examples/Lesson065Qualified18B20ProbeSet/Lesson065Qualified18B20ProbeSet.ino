// E0 copied-evidence fixture. This sketch stages complete synthetic Lesson 064
// transaction snapshots into named memory result cells. It owns no probe, pin,
// pull-up, bus, power path, adapter, or physical temperature claim.
#include <Adk.h>
#include <qualified_18b20_probe_set_policy.h>
#include <string.h>

namespace {

    struct ProbeResultCell
    {
        int16_t  rawSixteenths;
        int16_t  lowerRawSixteenths;
        int16_t  upperRawSixteenths;
        uint32_t cycleSequence;
        uint32_t conversionGeneration;
        uint32_t readTransactionGeneration;
        uint32_t age;
        uint8_t  quality;
        uint8_t  status;
        uint8_t  configuredOrderPass;
    };

    struct SetResultCell
    {
        uint32_t cycleSequence;
        uint8_t  validCount;
        uint8_t  presentMask;
        uint8_t  faultMask;
        uint8_t  quality;
        uint8_t  status;
        uint8_t  initialized;
        uint8_t  completedStages;
        uint8_t  predictionPass;
        uint8_t  complete;
    };

    constexpr uint8_t  fixtureSourceId                     = 65;
    constexpr uint16_t fixtureConfigurationRevision        = 1;
    constexpr uint32_t fixtureOwnerToken                   = UINT32_C (650065);
    constexpr uint16_t fixtureOneWireConfigurationRevision = 64;

    adk::Qualified18B20ProbeSetPolicy
        fixturePolicy ({fixtureSourceId,
                        fixtureConfigurationRevision,
                        fixtureOwnerToken,
                        fixtureOneWireConfigurationRevision,
                        {{{{0x28, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29}},
                          adk::Ds18b20Resolution::Bits9,
                          adk::Duration (94),
                          adk::Duration (1000),
                          -880,
                          2000,
                          160},
                         {{{0x28, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70}},
                          adk::Ds18b20Resolution::Bits10,
                          adk::Duration (188),
                          adk::Duration (1000),
                          -880,
                          2000,
                          160},
                         {{{0x28, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47}},
                          adk::Ds18b20Resolution::Bits11,
                          adk::Duration (375),
                          adk::Duration (1000),
                          -880,
                          2000,
                          160},
                         {{{0x28, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc2}},
                          adk::Ds18b20Resolution::Bits12,
                          adk::Duration (750),
                          adk::Duration (1000),
                          -880,
                          2000,
                          160}}});
    adk::Ds18b20CycleBuilder fixtureBuilder;

    volatile ProbeResultCell probeResultCells[4];
    volatile SetResultCell   setResultCell;

    uint32_t requestSequence;
    uint32_t transactionGeneration;
    bool     replayActive;

    // clang-format off
    adk::Status acquireCopiedProbeFixture   ();
    void        configureProbeReplay        ();
    adk::Status startProbeSetPolicy         ();
    void        clearTransaction            (
        adk::OneWireTransactionSnapshot& transaction,
        adk::OneWireOperation operation, const adk::OneWireRomCode& rom,
        uint16_t acceptedSlotCount, uint32_t startedAt, uint32_t completedAt);
    adk::OneWireRomCode configuredRom        (uint8_t index);
    adk::OneWireSearchState searchState      (uint8_t pass);
    void        prepareSearchPass            (
        uint8_t index, adk::OneWireTransactionSnapshot& transaction,
        adk::OneWireSearchState& requestSearch);
    void        prepareConversionStart       (
        uint8_t index, adk::OneWireTransactionSnapshot& transaction);
    void        prepareConversionStatus      (
        uint8_t index, adk::OneWireTransactionSnapshot& transaction);
    void        prepareScratchpad            (
        uint8_t index, adk::OneWireTransactionSnapshot& transaction);
    adk::Status replayCopiedCycle            ();
    bool        equalRom                     (
        const adk::OneWireRomCode& left, const adk::OneWireRomCode& right);
    bool        decideProbeSet              (const adk::QualifiedDs18b20Snapshot& snapshot);
    void        presentProbeSet             (
        const adk::QualifiedDs18b20Snapshot& snapshot, bool prediction);
    void        finishReplay                (adk::Status status);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = acquireCopiedProbeFixture ();

    if (!fixtureStatus.ok ())
    {
        finishReplay (fixtureStatus);
        return;
    }

    configureProbeReplay ();

    const adk::Status initializeStatus = startProbeSetPolicy ();

    const bool initializationFailed = !initializeStatus.ok ();
    if (initializationFailed)
    {
        finishReplay (initializeStatus);
    }
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    replayActive = false;
    finishReplay (replayCopiedCycle ());
}

namespace {

    adk::Status acquireCopiedProbeFixture ()
    {
        return fixtureSourceId != 0 && fixtureConfigurationRevision != 0 &&
                       fixtureOwnerToken != 0
                   ? adk::StatusCode::Ok
                   : adk::StatusCode::InternalInvariant;
    }

    void configureProbeReplay ()
    {
        requestSequence       = 0;
        transactionGeneration = 0;
        replayActive          = true;

        setResultCell.cycleSequence = 0;
        setResultCell.validCount    = 0;
        setResultCell.presentMask   = 0;
        setResultCell.faultMask     = 0;
        setResultCell.quality =
            static_cast<uint8_t> (adk::Ds18b20SetQuality::Unqualified);
        setResultCell.status = static_cast<uint8_t> (adk::StatusCode::NotInitialized);
        setResultCell.initialized     = 0;
        setResultCell.completedStages = 0;
        setResultCell.predictionPass  = 0;
        setResultCell.complete        = 0;
    }

    adk::Status startProbeSetPolicy ()
    {
        const adk::Status status = fixturePolicy.initialize ();

        setResultCell.initialized = fixturePolicy.initialized () ? 1 : 0;
        return status;
    }

    void clearTransaction (adk::OneWireTransactionSnapshot& transaction,
                           adk::OneWireOperation            operation,
                           const adk::OneWireRomCode& rom, uint16_t acceptedSlotCount,
                           uint32_t startedAt, uint32_t completedAt)
    {
        ++requestSequence;
        ++transactionGeneration;
        memset (static_cast<void*> (&transaction), 0, sizeof transaction);
        transaction.operation               = operation;
        transaction.phase                   = adk::OneWirePhase::Complete;
        transaction.quality                 = adk::OneWireTransactionQuality::Complete;
        transaction.request.requestSequence = requestSequence;
        transaction.request.operation       = operation;
        transaction.request.addressedRom    = rom;
        transaction.request.startedAt       = adk::MicrosecondTimePoint (startedAt);
        transaction.request.supplyMode      = adk::OneWireSupplyMode::ExternallyPowered;
        transaction.request.status          = adk::StatusCode::Ok;
        transaction.acceptedSlotCount       = acceptedSlotCount;
        transaction.presenceSeen            = true;
        transaction.releaseRequested        = true;
        transaction.releaseConfirmed        = true;
        transaction.completedAt             = adk::MicrosecondTimePoint (completedAt);
        transaction.status                  = adk::StatusCode::Ok;
        transaction.ownerToken              = fixtureOwnerToken;
        transaction.lifecycleGeneration     = 1;
        transaction.configurationRevision   = fixtureOneWireConfigurationRevision;
        transaction.transactionGeneration   = transactionGeneration;
    }

    adk::OneWireRomCode configuredRom (uint8_t index)
    {
        const uint8_t serials[4] = {0x01, 0x02, 0x03, 0x04};
        const uint8_t crcs[4]    = {0x29, 0x70, 0x47, 0xc2};
        return {{0x28, serials[index], 0, 0, 0, 0, 0, crcs[index]}};
    }

    adk::OneWireSearchState searchState (uint8_t pass)
    {
        if (pass == 0)
        {
            return {{{0, 0, 0, 0, 0, 0, 0, 0}}, 0, false};
        }
        const uint8_t configuredIndexes[4] = {3, 1, 0, 2};
        const uint8_t discrepancies[4]     = {10, 9, 10, 0};
        return {configuredRom (configuredIndexes[pass - 1]), discrepancies[pass - 1],
                pass == 4};
    }

    void prepareSearchPass (uint8_t index, adk::OneWireTransactionSnapshot& transaction,
                            adk::OneWireSearchState& requestSearch)
    {
        const adk::OneWireRomCode zeroRom = {{0, 0, 0, 0, 0, 0, 0, 0}};
        requestSearch                     = searchState (index);

        clearTransaction (transaction, adk::OneWireOperation::SearchRomPass, zeroRom,
                          200, 1000 + index * 1000, 1500 + index * 1000);
        transaction.request.search = requestSearch;
        transaction.searchResult   = searchState (index + 1);
    }

    void prepareConversionStart (uint8_t                          index,
                                 adk::OneWireTransactionSnapshot& transaction)
    {
        const uint32_t startedAt = 6000 + index * 3000;
        clearTransaction (transaction, adk::OneWireOperation::MatchRomStartConversion,
                          configuredRom (index), 80, startedAt, startedAt + 500);
    }

    void prepareConversionStatus (uint8_t                          index,
                                  adk::OneWireTransactionSnapshot& transaction)
    {
        const uint32_t startedAt = 7000 + index * 3000;
        clearTransaction (transaction,
                          adk::OneWireOperation::MatchRomReadConversionStatus,
                          configuredRom (index), 1, startedAt, startedAt + 500);
        transaction.readBytes[0]  = 1;
        transaction.readByteCount = 1;
    }

    void prepareScratchpad (uint8_t index, adk::OneWireTransactionSnapshot& transaction)
    {
        const uint8_t  lowBytes[4]       = {0x40, 0x50, 0x60, 0x70};
        const uint8_t  configurations[4] = {0x1f, 0x3f, 0x5f, 0x7f};
        const uint8_t  crcs[4]           = {0x2c, 0x08, 0x64, 0x40};
        const uint32_t startedAt         = 8000 + index * 3000;
        clearTransaction (transaction, adk::OneWireOperation::MatchRomReadScratchpad,
                          configuredRom (index), 152, startedAt, startedAt + 500);
        const uint8_t scratchpad[9] = {
            lowBytes[index], 0x01, 0x4b, 0x46, configurations[index], 0xff, 0x0c, 0x10,
            crcs[index]};
        for (uint8_t byteIndex = 0; byteIndex < 9; ++byteIndex)
        {
            transaction.readBytes[byteIndex] = scratchpad[byteIndex];
        }
        transaction.readByteCount = 9;
    }

    adk::Status replayCopiedCycle ()
    {
        adk::Status status = fixturePolicy.beginCycle (
            adk::TimePoint (20), fixtureSourceId, fixtureConfigurationRevision, 1,
            adk::TimePoint (20), fixtureBuilder);
        if (!status.ok ())
        {
            return status;
        }

        for (uint8_t pass = 0; pass < 4; ++pass)
        {
            adk::OneWireTransactionSnapshot transaction;
            adk::OneWireSearchState         requestSearch;
            prepareSearchPass (pass, transaction, requestSearch);

            status = fixturePolicy.ingestSearchPass (fixtureBuilder, transaction,
                                                     requestSearch);
            if (!status.ok ())
            {
                return status;
            }
        }

        status = fixturePolicy.finishSearch (fixtureBuilder, true, false,
                                             adk::StatusCode::Ok);
        if (!status.ok ())
        {
            return status;
        }

        for (uint8_t index = 0; index < 4; ++index)
        {
            adk::OneWireTransactionSnapshot transaction;
            prepareConversionStart (index, transaction);

            status = fixturePolicy.ingestConversionStart (
                fixtureBuilder, static_cast<uint32_t> (index) + 1U, transaction);
            if (!status.ok ())
            {
                return status;
            }

            prepareConversionStatus (index, transaction);

            status = fixturePolicy.ingestConversionStatus (
                fixtureBuilder, static_cast<uint32_t> (index) + 1U, transaction);
            if (!status.ok ())
            {
                return status;
            }

            prepareScratchpad (index, transaction);

            status = fixturePolicy.ingestScratchpad (fixtureBuilder,
                                                     static_cast<uint32_t> (index) + 1U,
                                                     adk::TimePoint (20), transaction);
            if (!status.ok ())
            {
                return status;
            }
        }

        adk::QualifiedDs18b20Snapshot snapshot;
        status =
            fixturePolicy.finalizeCycle (adk::TimePoint (20), fixtureBuilder, snapshot);
        if (status.ok ())
        {
            presentProbeSet (snapshot, decideProbeSet (snapshot));
            setResultCell.completedStages = 7;
        }
        return status;
    }

    bool equalRom (const adk::OneWireRomCode& left, const adk::OneWireRomCode& right)
    {
        for (uint8_t index = 0; index < 8; ++index)
        {
            if (left.bytes[index] != right.bytes[index])
            {
                return false;
            }
        }
        return true;
    }

    bool decideProbeSet (const adk::QualifiedDs18b20Snapshot& snapshot)
    {
        const int16_t expectedLower[4] = {320, 336, 352, 368};
        const int16_t expectedUpper[4] = {327, 339, 353, 368};
        if (snapshot.quality != adk::Ds18b20SetQuality::Complete ||
            snapshot.validCount != 4 || snapshot.presentMask != 0x0f ||
            snapshot.faultMask != 0 || snapshot.cycleSequence != 1 ||
            snapshot.status.error () != adk::StatusCode::Ok)
        {
            return false;
        }

        for (uint8_t index = 0; index < 4; ++index)
        {
            const adk::QualifiedDs18b20Probe& probe = snapshot.probes[index];
            if (probe.quality != adk::Ds18b20ProbeQuality::Current ||
                !equalRom (probe.rom, configuredRom (index)) ||
                probe.rawSixteenths != expectedLower[index] ||
                probe.lowerRawSixteenths != expectedLower[index] ||
                probe.upperRawSixteenths != expectedUpper[index] ||
                probe.age.milliseconds () != 0 || probe.cycleSequence != 1 ||
                probe.conversionGeneration != static_cast<uint32_t> (index) + 1U ||
                probe.readTransactionGeneration !=
                    7U + static_cast<uint32_t> (index) * 3U ||
                probe.status.error () != adk::StatusCode::Ok)
            {
                return false;
            }
        }
        return true;
    }

    void presentProbeSet (const adk::QualifiedDs18b20Snapshot& snapshot,
                          bool                                 prediction)
    {
        for (uint8_t index = 0; index < 4; ++index)
        {
            const adk::QualifiedDs18b20Probe& probe = snapshot.probes[index];
            volatile ProbeResultCell&         cell  = probeResultCells[index];

            cell.rawSixteenths             = probe.rawSixteenths;
            cell.lowerRawSixteenths        = probe.lowerRawSixteenths;
            cell.upperRawSixteenths        = probe.upperRawSixteenths;
            cell.cycleSequence             = probe.cycleSequence;
            cell.conversionGeneration      = probe.conversionGeneration;
            cell.readTransactionGeneration = probe.readTransactionGeneration;
            cell.age                       = probe.age.milliseconds ();
            cell.quality                   = static_cast<uint8_t> (probe.quality);
            cell.status = static_cast<uint8_t> (probe.status.error ());
            cell.configuredOrderPass =
                equalRom (probe.rom, configuredRom (index)) ? 1 : 0;
        }

        setResultCell.cycleSequence  = snapshot.cycleSequence;
        setResultCell.validCount     = snapshot.validCount;
        setResultCell.presentMask    = snapshot.presentMask;
        setResultCell.faultMask      = snapshot.faultMask;
        setResultCell.quality        = static_cast<uint8_t> (snapshot.quality);
        setResultCell.status         = static_cast<uint8_t> (snapshot.status.error ());
        setResultCell.predictionPass = prediction ? 1 : 0;
    }

    void finishReplay (adk::Status status)
    {
        setResultCell.status   = static_cast<uint8_t> (status.error ());
        setResultCell.complete = 1;
    }

} // namespace
