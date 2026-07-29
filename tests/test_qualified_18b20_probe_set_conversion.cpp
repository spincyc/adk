#include <qualified_18b20_probe_set_policy.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {
    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    uint8_t crc8 (const uint8_t* bytes, uint8_t count)
    {
        uint8_t crc = 0;
        for (uint8_t index = 0; index < count; ++index)
        {
            uint8_t value = bytes[index];
            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                const bool mix = ((crc ^ value) & 1U) != 0;
                crc >>= 1U;
                if (mix)
                {
                    crc ^= 0x8cU;
                }
                value >>= 1U;
            }
        }
        return crc;
    }

    adk::OneWireRomCode rom (uint8_t serial)
    {
        adk::OneWireRomCode value = {{0x28, serial, 0x10, 0x20, 0x30, 0x40, 0x50, 0}};
        value.bytes[7]            = crc8 (value.bytes, 7);
        return value;
    }

    adk::OneWireRomCode emptyRom ()
    {
        return {{0, 0, 0, 0, 0, 0, 0, 0}};
    }

    bool sameRom (const adk::OneWireRomCode& left, const adk::OneWireRomCode& right)
    {
        return std::memcmp (left.bytes, right.bytes, sizeof left.bytes) == 0;
    }

    adk::QualifiedDs18b20SetConfig config ()
    {
        adk::QualifiedDs18b20SetConfig value;
        value.expectedSourceId                     = 9;
        value.expectedConfigurationRevision        = 31;
        value.expectedOneWireOwnerToken            = 0x12345678UL;
        value.expectedOneWireConfigurationRevision = 17;
        for (uint8_t index = 0; index < 4; ++index)
        {
            value.probes[index].rom        = rom (static_cast<uint8_t> (index + 1U));
            value.probes[index].resolution = adk::Ds18b20Resolution::Bits12;
            value.probes[index].conversionDeadline       = adk::Duration (750);
            value.probes[index].maximumAge               = adk::Duration (5000);
            value.probes[index].minimumRawSixteenths     = -880;
            value.probes[index].maximumRawSixteenths     = 2000;
            value.probes[index].maximumStepRawSixteenths = 160;
        }
        return value;
    }

    adk::QualifiedDs18b20SetConfig configFor (
        adk::Ds18b20Resolution resolution, uint32_t deadline)
    {
        adk::QualifiedDs18b20SetConfig value = config ();
        value.probes[0].resolution            = resolution;
        value.probes[0].conversionDeadline    = adk::Duration (deadline);
        return value;
    }

    adk::OneWireTransactionSnapshot
    transaction (adk::OneWireOperation      operation,
                 const adk::OneWireRomCode& addressedRom, uint32_t requestSequence,
                 uint32_t transactionGeneration, uint32_t startedAt,
                 uint32_t completedAt)
    {
        adk::OneWireTransactionSnapshot value;
        value.operation               = operation;
        value.phase                   = adk::OneWirePhase::Complete;
        value.quality                 = adk::OneWireTransactionQuality::Complete;
        value.request.requestSequence = requestSequence;
        value.request.operation       = operation;
        value.request.addressedRom    = addressedRom;
        value.request.search          = {emptyRom (), 0, false};

        value.request.startedAt       = adk::MicrosecondTimePoint (startedAt);
        value.request.supplyMode      = adk::OneWireSupplyMode::ExternallyPowered;
        value.request.status          = adk::StatusCode::Ok;
        value.searchResult            = {emptyRom (), 0, false};

        value.returnedRom             = emptyRom ();
        for (uint8_t index = 0; index < 9; ++index)
        {
            value.readBytes[index] = 0;
        }
        value.readByteCount         = 0;
        value.acceptedSlotCount     = 72;
        value.presenceSeen          = true;
        value.releaseRequested      = true;
        value.releaseConfirmed      = true;
        value.completedAt           = adk::MicrosecondTimePoint (completedAt);
        value.status                = adk::StatusCode::Ok;
        value.ownerToken            = 0x12345678UL;
        value.lifecycleGeneration   = 5;
        value.configurationRevision = 17;
        value.transactionGeneration = transactionGeneration;
        return value;
    }

    adk::OneWireTransactionSnapshot searchTransaction (
        const adk::OneWireRomCode& found, uint32_t requestSequence = 1,
        uint32_t transactionGeneration = 10)
    {
        adk::OneWireTransactionSnapshot value = transaction (
            adk::OneWireOperation::SearchRomPass, emptyRom (), requestSequence,
            transactionGeneration, 1000, 3000);
        value.acceptedSlotCount    = 200;
        value.request.search       = {emptyRom (), 0, false};
        value.searchResult         = {found, 0, true};
        value.request.addressedRom = emptyRom ();
        value.returnedRom          = emptyRom ();
        return value;
    }

    adk::OneWireTransactionSnapshot
    conversionStart (const adk::OneWireRomCode& addressedRom,
                     uint32_t requestSequence = 2,
                     uint32_t transactionGeneration = 11)
    {
        adk::OneWireTransactionSnapshot value =
            transaction (adk::OneWireOperation::MatchRomStartConversion, addressedRom,
                         requestSequence, transactionGeneration, 4000, 5000);
        value.acceptedSlotCount = 80;
        return value;
    }

    adk::OneWireTransactionSnapshot
    conversionStatus (const adk::OneWireRomCode& addressedRom, uint32_t generation,
                      uint32_t observedAt, bool completedHigh)
    {
        adk::OneWireTransactionSnapshot value =
            transaction (adk::OneWireOperation::MatchRomReadConversionStatus,
                         addressedRom, 3, generation, observedAt - 10U, observedAt);
        value.acceptedSlotCount = 1;
        value.readByteCount     = 1;
        value.readBytes[0]      = completedHigh ? 1 : 0;
        return value;
    }

    adk::OneWireTransactionSnapshot scratchpad (const adk::OneWireRomCode& addressedRom,
                                                uint32_t                   generation,
                                                uint32_t                   completedAt,
                                                int16_t rawSixteenths,
                                                adk::Ds18b20Resolution resolution =
                                                    adk::Ds18b20Resolution::Bits12)
    {
        adk::OneWireTransactionSnapshot value =
            transaction (adk::OneWireOperation::MatchRomReadScratchpad, addressedRom, 4,
                         generation, completedAt - 500U, completedAt);
        value.acceptedSlotCount = 152;
        value.readByteCount     = 9;
        value.readBytes[0]      = static_cast<uint8_t> (rawSixteenths);
        value.readBytes[1] =
            static_cast<uint8_t> (static_cast<uint16_t> (rawSixteenths) >> 8U);
        value.readBytes[2] = 75;
        value.readBytes[3] = 70;
        value.readBytes[4] = static_cast<uint8_t> (
            0x1fU | (static_cast<uint8_t> (resolution) << 5U));
        value.readBytes[5] = 0xff;
        value.readBytes[6] = 0x0c;
        value.readBytes[7] = 0x10;
        value.readBytes[8] = crc8 (value.readBytes, 8);
        return value;
    }

    void beginSearchedCycle (const adk::Qualified18B20ProbeSetPolicy& policy,
                             adk::Ds18b20CycleBuilder&                builder,
                             uint32_t policyTime = 100,
                             uint32_t requestSequence = 1,
                             uint32_t transactionGeneration = 10)
    {
        require (policy
                     .beginCycle (adk::TimePoint (policyTime), 9, 31, 1,
                                  adk::TimePoint (policyTime), builder)
                     .ok (),
                 "cycle begins");
        const adk::OneWireRomCode first = rom (1);

        const adk::OneWireTransactionSnapshot search =
            searchTransaction (first, requestSequence,
                               transactionGeneration);

        const adk::OneWireSearchState requestSearch = {emptyRom (), 0, false};

        require (policy.ingestSearchPass (builder, search, requestSearch).ok (),
                 "terminal search pass ingests");

        require (policy.finishSearch (builder, true, false, adk::StatusCode::Ok).ok (),
                 "terminal search completes");
    }

    void verifyAttributionAndAtomicity ()
    {
        adk::Qualified18B20ProbeSetPolicy policy (config ());

        require (policy.initialize ().ok (), "policy initializes");

        adk::Ds18b20CycleBuilder builder;
        beginSearchedCycle (policy, builder);

        const adk::OneWireRomCode first = rom (1);

        const adk::OneWireTransactionSnapshot start = conversionStart (first);

        require (policy.ingestConversionStart (builder, 41, start).ok (),
                 "conversion start ingests");

        unsigned char before[sizeof builder];
        std::memcpy (before, &builder, sizeof builder);

        adk::OneWireTransactionSnapshot crossed =
            conversionStatus (rom (2), 12, 6000, true);
        require (policy.ingestConversionStatus (builder, 41, crossed).error () ==
                     adk::StatusCode::InvalidArgument,
                 "conversion status rejects crossed ROM attribution");
        require (std::memcmp (before, &builder, sizeof builder) == 0,
                 "crossed-ROM rejection leaves builder byte-identical");

        crossed = conversionStatus (first, 12, 6000, true);
        crossed.ownerToken++;
        require (policy.ingestConversionStatus (builder, 41, crossed).error () ==
                     adk::StatusCode::InvalidArgument,
                 "conversion status rejects crossed owner attribution");
        require (std::memcmp (before, &builder, sizeof builder) == 0,
                 "crossed-owner rejection leaves builder byte-identical");

        crossed = conversionStatus (first, 12, 6000, true);
        crossed.lifecycleGeneration++;
        require (policy.ingestConversionStatus (builder, 41, crossed).error () ==
                     adk::StatusCode::InvalidArgument,
                 "conversion status rejects crossed lifecycle attribution");
        require (std::memcmp (before, &builder, sizeof builder) == 0,
                 "crossed-lifecycle rejection leaves builder byte-identical");

        crossed = conversionStatus (first, 12, 6000, true);

        require (policy.ingestConversionStatus (builder, 42, crossed).error () ==
                     adk::StatusCode::InvalidArgument,
                 "conversion status rejects crossed conversion generation");
        require (std::memcmp (before, &builder, sizeof builder) == 0,
                 "crossed-generation rejection leaves builder byte-identical");

        require (policy
                     .ingestConversionStatus (builder, 41,
                                              conversionStatus (first, 12, 6000, true))
                     .ok (),
                 "matching conversion status ingests");

        std::memcpy (before, &builder, sizeof builder);
        adk::OneWireTransactionSnapshot wrongPredecessor =
            scratchpad (first, 14, 6500, 400);
        require (policy.ingestScratchpad (builder, 41, adk::TimePoint (110),
                                          wrongPredecessor)
                         .error () == adk::StatusCode::InvalidArgument,
                 "scratchpad rejects a non-successor transaction generation");
        require (std::memcmp (before, &builder, sizeof builder) == 0,
                 "predecessor rejection leaves builder byte-identical");

        wrongPredecessor = scratchpad (first, 13, 6500, 400);
        wrongPredecessor.request.requestSequence = 3;

        require (
            policy
                    .ingestScratchpad (builder, 41, adk::TimePoint (110),
                                       wrongPredecessor)
                    .error () == adk::StatusCode::InvalidArgument,
            "scratchpad request sequence must follow conversion status");
        require (std::memcmp (before, &builder, sizeof builder) == 0,
                 "scratchpad request-order rejection preserves builder");

        wrongPredecessor = scratchpad (first, 13, 6500, 400);

        wrongPredecessor.request.startedAt = adk::MicrosecondTimePoint (5999);

        require (
            policy
                    .ingestScratchpad (builder, 41, adk::TimePoint (110),
                                       wrongPredecessor)
                    .error () == adk::StatusCode::InvalidArgument,
            "scratchpad time must follow conversion-status completion");
        require (std::memcmp (before, &builder, sizeof builder) == 0,
                 "scratchpad time-order rejection preserves builder");

        require (policy
                     .ingestScratchpad (builder, 41, adk::TimePoint (110),
                                       scratchpad (first, 13, 6500, 400))
                     .ok (),
                 "scratchpad accepts exact staged predecessor chain");
    }

    void verifyPendingDeadlineAndClockWitness ()
    {
        const adk::OneWireRomCode first = rom (1);

        {
            adk::Qualified18B20ProbeSetPolicy policy (config ());

            require (policy.initialize ().ok (), "pending policy initializes");

            adk::Ds18b20CycleBuilder builder;

            beginSearchedCycle (policy, builder);

            require (policy.ingestConversionStart (builder, 50, conversionStart (first))
                         .ok (),
                     "pending conversion starts");
            require (policy
                         .ingestConversionStatus (
                             builder, 50, conversionStatus (first, 12, 753999, false))
                         .ok (),
                     "low conversion status immediately before deadline ingests");

            adk::QualifiedDs18b20Snapshot result;
            const adk::Status             finalized =
                policy.finalizeCycle (adk::TimePoint (120), builder, result);
            require (finalized.error () != adk::StatusCode::InvalidArgument &&
                         finalized.error () != adk::StatusCode::InvalidConfiguration,
                     "pre-deadline pending cycle commits typed evidence");
            require (result.probes[0].quality ==
                         adk::Ds18b20ProbeQuality::ConversionPending,
                     "pre-deadline low status remains pending");
        }

        {
            adk::Qualified18B20ProbeSetPolicy policy (config ());

            require (policy.initialize ().ok (), "deadline policy initializes");

            adk::Ds18b20CycleBuilder builder;

            beginSearchedCycle (policy, builder);

            require (policy.ingestConversionStart (builder, 51, conversionStart (first))
                         .ok (),
                     "deadline conversion starts");
            require (policy
                         .ingestConversionStatus (
                             builder, 51, conversionStatus (first, 12, 754000, false))
                         .ok (),
                     "low conversion status at deadline remains typed evidence");

            adk::QualifiedDs18b20Snapshot result;
            const adk::Status             finalized =
                policy.finalizeCycle (adk::TimePoint (120), builder, result);
            require (!finalized.ok (),
                     "low status at deadline reports transport failure");
            require (result.probes[0].quality ==
                         adk::Ds18b20ProbeQuality::TransportFault,
                     "deadline expiry is not published as pending");
        }

        {
            adk::Qualified18B20ProbeSetPolicy policy (config ());

            require (policy.initialize ().ok (), "clock policy initializes");

            adk::Ds18b20CycleBuilder builder;

            beginSearchedCycle (policy, builder, 0xfffffffeUL);

            require (policy.ingestConversionStart (builder, 52, conversionStart (first))
                         .ok (),
                     "clock conversion starts");
            require (policy
                         .ingestConversionStatus (
                             builder, 52, conversionStatus (first, 12, 6000, true))
                         .ok (),
                     "clock conversion completes");
            require (policy
                         .ingestScratchpad (builder, 52, adk::TimePoint (0xffffffffUL),
                                            scratchpad (first, 13, 6500, 400))
                         .ok (),
                     "scratchpad retains supplied policy-clock rollover witness");

            adk::QualifiedDs18b20Snapshot result;
            const adk::Status             finalized =
                policy.finalizeCycle (adk::TimePoint (1), builder, result);
            require (finalized.error () != adk::StatusCode::InvalidArgument &&
                         finalized.error () != adk::StatusCode::InvalidConfiguration,
                     "forward policy-clock rollover commits typed evidence");
            require (result.probes[0].observedAt == adk::TimePoint (0xffffffffUL) &&
                         result.probes[0].age == adk::Duration (2),
                     "freshness uses the scratchpad policy-clock witness");
            require (sameRom (result.probes[0].rom, first),
                     "clock witness remains attributed to the exact ROM");
        }
    }

    void verifyResolutionDeadlineMatrix ()
    {
        const adk::Ds18b20Resolution resolutions[4] = {
            adk::Ds18b20Resolution::Bits9,
            adk::Ds18b20Resolution::Bits10,
            adk::Ds18b20Resolution::Bits11,
            adk::Ds18b20Resolution::Bits12};
        const uint32_t deadlines[4] = {94, 188, 375, 750};
        const int32_t  offsets[3]   = {-1, 0, 1};
        const adk::OneWireRomCode first = rom (1);

        for (uint8_t resolutionIndex = 0; resolutionIndex < 4;
             ++resolutionIndex)
        {
            for (uint8_t boundary = 0; boundary < 3; ++boundary)
            {
                for (uint8_t highIndex = 0; highIndex < 2; ++highIndex)
                {
                    const bool high = highIndex != 0;
                    adk::Qualified18B20ProbeSetPolicy policy (
                        configFor (resolutions[resolutionIndex],
                                   deadlines[resolutionIndex]));

                    require (policy.initialize ().ok (),
                             "deadline matrix policy initializes");

                    adk::Ds18b20CycleBuilder builder;

                    beginSearchedCycle (policy, builder);

                    require (
                        policy
                            .ingestConversionStart (
                                builder, 80, conversionStart (first))
                            .ok (),
                        "deadline matrix conversion starts");

                    const uint32_t deadlineAt =
                        4000U + deadlines[resolutionIndex] * 1000U;
                    const uint32_t statusAt = static_cast<uint32_t> (
                        static_cast<int32_t> (deadlineAt) +
                        offsets[boundary]);

                    require (
                        policy
                            .ingestConversionStatus (
                                builder, 80,
                                conversionStatus (first, 12, statusAt, high))
                            .ok (),
                        "deadline matrix status ingests");

                    if (high && boundary < 2)
                    {
                        const adk::OneWireTransactionSnapshot read =
                            scratchpad (
                                first, 13, statusAt + 1000U, 400,
                                resolutions[resolutionIndex]);

                        require (
                            policy
                                .ingestScratchpad (
                                    builder, 80, adk::TimePoint (110),
                                    read)
                                .ok (),
                            "timely completed-high evidence accepts scratchpad");
                    }

                    struct GuardedSnapshot
                    {
                        uint32_t                         before;
                        adk::QualifiedDs18b20Snapshot snapshot;
                        uint32_t                         after;
                    };
                    GuardedSnapshot guarded;
                    guarded.before = 0x13579bdfUL;
                    guarded.after  = 0x2468ace0UL;

                    policy.finalizeCycle (adk::TimePoint (120), builder,
                                          guarded.snapshot);

                    require (
                        guarded.before == 0x13579bdfUL &&
                            guarded.after == 0x2468ace0UL,
                        "deadline matrix preserves output canaries");

                    const adk::Ds18b20ProbeQuality expected =
                        !high
                            ? (boundary == 0
                                   ? adk::Ds18b20ProbeQuality::ConversionPending
                                   : adk::Ds18b20ProbeQuality::TransportFault)
                            : (boundary < 2
                                   ? adk::Ds18b20ProbeQuality::Current
                                   : adk::Ds18b20ProbeQuality::TransportFault);

                    require (guarded.snapshot.probes[0].quality == expected,
                             "resolution deadline boundary quality is exact");
                }
            }
        }
    }

    void verifyOrderingRegressions ()
    {
        adk::Qualified18B20ProbeSetPolicy policy (config ());

        require (policy.initialize ().ok (), "ordering policy initializes");

        adk::Ds18b20CycleBuilder builder;

        beginSearchedCycle (policy, builder);

        const adk::OneWireRomCode first = rom (1);

        const adk::OneWireTransactionSnapshot start = conversionStart (first);

        require (policy.ingestConversionStart (builder, 90, start).ok (),
                 "ordering conversion starts");

        unsigned char before[sizeof builder];
        std::memcpy (before, &builder, sizeof builder);

        adk::OneWireTransactionSnapshot malformed =
            conversionStatus (first, 12, 6000, true);
        malformed.request.requestSequence = start.request.requestSequence;

        require (
            policy.ingestConversionStatus (builder, 90, malformed).error () ==
                adk::StatusCode::InvalidArgument,
            "status request sequence must follow conversion start");
        require (std::memcmp (before, &builder, sizeof builder) == 0,
                 "request-sequence rejection preserves builder canary");

        malformed = conversionStatus (first, 11, 6000, true);

        require (
            policy.ingestConversionStatus (builder, 90, malformed).error () ==
                adk::StatusCode::InvalidArgument,
            "status transaction generation must follow conversion start");
        require (std::memcmp (before, &builder, sizeof builder) == 0,
                 "generation-order rejection preserves builder canary");

        malformed = conversionStatus (first, 12, 3999, true);

        require (
            policy.ingestConversionStatus (builder, 90, malformed).error () ==
                adk::StatusCode::InvalidArgument,
            "status transaction time must not precede conversion start");
        require (std::memcmp (before, &builder, sizeof builder) == 0,
                 "time-order rejection preserves builder canary");
    }

    void requireBuilderUnchanged (const unsigned char* before,
                                  const adk::Ds18b20CycleBuilder& builder,
                                  const char* message)
    {
        require (std::memcmp (before, &builder, sizeof builder) == 0, message);
    }

    void verifyRoleCorrelationMatrix ()
    {
        const adk::OneWireRomCode first = rom (1);

        {
            adk::Qualified18B20ProbeSetPolicy policy (config ());

            require (policy.initialize ().ok (),
                     "start-correlation policy initializes");

            adk::Ds18b20CycleBuilder builder;
            beginSearchedCycle (policy, builder);

            unsigned char before[sizeof builder];
            std::memcpy (before, &builder, sizeof builder);

            for (uint8_t field = 0; field < 11; ++field)
            {
                adk::OneWireTransactionSnapshot malformed =
                    conversionStart (first);
                switch (field)
                {
                    case 0: malformed.ownerToken++; break;
                    case 1: malformed.lifecycleGeneration++; break;
                    case 2: malformed.configurationRevision++; break;
                    case 3: malformed.request.addressedRom = rom (5); break;
                    case 4: malformed.request.requestSequence = 1; break;
                    case 5: malformed.transactionGeneration = 10; break;
                    case 6:
                        malformed.request.startedAt =
                            adk::MicrosecondTimePoint (2999);
                        break;
                    case 7: malformed.request.requestSequence = 0; break;
                    case 8: malformed.transactionGeneration = 0; break;
                    case 9:
                        malformed.request.requestSequence = 0x80000001UL;
                        break;
                    case 10:
                        malformed.transactionGeneration = 0x8000000aUL;
                        break;
                }
                const adk::Status rejected =
                    policy.ingestConversionStart (builder, 100, malformed);
                require (
                    rejected.error () == adk::StatusCode::InvalidArgument,
                    "start rejects every crossed predecessor field");
                requireBuilderUnchanged (
                    before, builder,
                    "failed start preserves the complete builder image");
            }
        }

        {
            adk::Qualified18B20ProbeSetPolicy policy (config ());

            require (policy.initialize ().ok (),
                     "status-correlation policy initializes");

            adk::Ds18b20CycleBuilder builder;

            beginSearchedCycle (policy, builder);

            require (
                policy
                    .ingestConversionStart (builder, 101,
                                            conversionStart (first))
                    .ok (),
                "status-correlation conversion starts");

            unsigned char before[sizeof builder];
            std::memcpy (before, &builder, sizeof builder);

            for (uint8_t field = 0; field < 12; ++field)
            {
                adk::OneWireTransactionSnapshot malformed =
                    conversionStatus (first, 12, 6000, true);
                uint32_t conversionGeneration = 101;
                switch (field)
                {
                    case 0: malformed.ownerToken++; break;
                    case 1: malformed.lifecycleGeneration++; break;
                    case 2: malformed.configurationRevision++; break;
                    case 3: malformed.request.addressedRom = rom (2); break;
                    case 4: malformed.request.requestSequence = 2; break;
                    case 5: malformed.transactionGeneration = 11; break;
                    case 6:
                        malformed.request.startedAt =
                            adk::MicrosecondTimePoint (4999);
                        break;
                    case 7: conversionGeneration++; break;
                    case 8: malformed.request.requestSequence = 0; break;
                    case 9: malformed.transactionGeneration = 0; break;
                    case 10:
                        malformed.request.requestSequence = 0x80000002UL;
                        break;
                    case 11:
                        malformed.transactionGeneration = 0x8000000bUL;
                        break;
                }
                require (
                    policy
                            .ingestConversionStatus (
                                builder, conversionGeneration, malformed)
                            .error () == adk::StatusCode::InvalidArgument,
                    "status rejects every crossed predecessor field");
                requireBuilderUnchanged (
                    before, builder,
                    "failed status preserves the complete builder image");
            }

            require (
                policy
                    .ingestConversionStatus (
                        builder, 101,
                        conversionStatus (first, 12, 6000, true))
                    .ok (),
                "scratch-correlation status ingests");

            std::memcpy (before, &builder, sizeof builder);

            for (uint8_t field = 0; field < 12; ++field)
            {
                adk::OneWireTransactionSnapshot malformed =
                    scratchpad (first, 13, 6500, 400);
                uint32_t conversionGeneration = 101;
                switch (field)
                {
                    case 0: malformed.ownerToken++; break;
                    case 1: malformed.lifecycleGeneration++; break;
                    case 2: malformed.configurationRevision++; break;
                    case 3: malformed.request.addressedRom = rom (2); break;
                    case 4: malformed.request.requestSequence = 3; break;
                    case 5: malformed.transactionGeneration = 12; break;
                    case 6:
                        malformed.request.startedAt =
                            adk::MicrosecondTimePoint (5999);
                        break;
                    case 7: conversionGeneration++; break;
                    case 8: malformed.request.requestSequence = 0; break;
                    case 9: malformed.transactionGeneration = 0; break;
                    case 10:
                        malformed.request.requestSequence = 0x80000003UL;
                        break;
                    case 11:
                        malformed.transactionGeneration = 0x8000000cUL;
                        break;
                }
                require (
                    policy
                            .ingestScratchpad (
                                builder, conversionGeneration,
                                adk::TimePoint (110), malformed)
                            .error () == adk::StatusCode::InvalidArgument,
                    "scratchpad rejects every crossed predecessor field");
                requireBuilderUnchanged (
                    before, builder,
                    "failed scratchpad preserves the complete builder image");
            }
        }
    }

    void verifyNestedSequenceExhaustion ()
    {
        adk::Qualified18B20ProbeSetPolicy policy (config ());

        require (policy.initialize ().ok (),
                 "nested-exhaustion policy initializes");

        adk::Ds18b20CycleBuilder builder;
        beginSearchedCycle (policy, builder, 100, 0xfffffffeUL,
                            0xfffffffeUL);

        const adk::OneWireRomCode first = rom (1);
        const adk::OneWireTransactionSnapshot terminalStart =
            conversionStart (first, 0xffffffffUL, 0xffffffffUL);

        require (
            policy.ingestConversionStart (builder, 110, terminalStart).ok (),
            "nested sequences may reach their final nonzero value");

        unsigned char before[sizeof builder];
        std::memcpy (before, &builder, sizeof builder);

        adk::OneWireTransactionSnapshot wrapped =
            conversionStatus (first, 0, 6000, true);
        wrapped.request.requestSequence = 0;

        require (
            policy.ingestConversionStatus (builder, 110, wrapped).error () ==
                adk::StatusCode::InvalidArgument,
            "nested request and transaction sequences never wrap through zero");
        requireBuilderUnchanged (
            before, builder,
            "nested exhaustion rejection preserves complete builder image");
    }
} // namespace

int main ()
{
    verifyAttributionAndAtomicity ();

    verifyPendingDeadlineAndClockWitness ();

    verifyResolutionDeadlineMatrix ();

    verifyOrderingRegressions ();

    verifyRoleCorrelationMatrix ();

    verifyNestedSequenceExhaustion ();
    return EXIT_SUCCESS;
}
