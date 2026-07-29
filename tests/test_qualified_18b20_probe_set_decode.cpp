#include <qualified_18b20_probe_set_policy.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

// clang-format off
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
                crc >>= 1;
                if (mix)
                {
                    crc ^= 0x8c;
                }
                value >>= 1;
            }
        }
        return crc;
    }

    adk::OneWireRomCode rom (uint8_t serial)
    {
        adk::OneWireRomCode value = {{0x28, serial, 0x22, 0x33, 0x44, 0x55, 0x66, 0}};
        value.bytes[7]            = crc8 (value.bytes, 7);
        return value;
    }

    bool romEqual (const adk::OneWireRomCode& left, const adk::OneWireRomCode& right)
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

    uint8_t resolutionByte (adk::Ds18b20Resolution resolution)
    {
        switch (resolution)
        {
            case adk::Ds18b20Resolution::Bits9: return 0x1f;
            case adk::Ds18b20Resolution::Bits10: return 0x3f;
            case adk::Ds18b20Resolution::Bits11: return 0x5f;
            case adk::Ds18b20Resolution::Bits12: return 0x7f;
        }
        return 0;
    }

    adk::QualifiedDs18b20SetConfig config ()
    {
        adk::QualifiedDs18b20SetConfig value;
        value.expectedSourceId                     = 7;
        value.expectedConfigurationRevision        = 19;
        value.expectedOneWireOwnerToken            = 0x12345678UL;
        value.expectedOneWireConfigurationRevision = 23;
        const uint16_t deadlines[4]                 = {94, 188, 375, 750};
        for (uint8_t index = 0; index < 4; ++index)
        {
            value.probes[index].rom = rom (static_cast<uint8_t> (0x10 + index));
            value.probes[index].resolution =
                static_cast<adk::Ds18b20Resolution> (index);
            value.probes[index].conversionDeadline =
                adk::Duration (deadlines[index]);
            value.probes[index].maximumAge               = adk::Duration (100);
            value.probes[index].minimumRawSixteenths     = -880;
            value.probes[index].maximumRawSixteenths     = 2000;
            value.probes[index].maximumStepRawSixteenths = 16;
        }
        return value;
    }

    adk::OneWireTransactionSnapshot
    transaction (adk::OneWireOperation      operation,
                 const adk::OneWireRomCode& addressedRom, uint32_t generation,
                 uint32_t startedAt, uint32_t completedAt)
    {
        adk::OneWireTransactionSnapshot value;
        value.operation               = operation;
        value.phase                   = adk::OneWirePhase::Complete;
        value.quality                 = adk::OneWireTransactionQuality::Complete;
        value.request.requestSequence = generation;
        value.request.operation       = operation;
        value.request.addressedRom    = addressedRom;
        value.request.search          = {{0, 0, 0, 0, 0, 0, 0, 0}, 0, false};
        value.request.startedAt       = adk::MicrosecondTimePoint (startedAt);
        value.request.supplyMode      = adk::OneWireSupplyMode::ExternallyPowered;
        value.request.status          = adk::StatusCode::Ok;
        value.searchResult            = {{0, 0, 0, 0, 0, 0, 0, 0}, 0, false};
        value.returnedRom             = addressedRom;
        for (uint8_t index = 0; index < 9; ++index)
        {
            value.readBytes[index] = 0;
        }
        value.readByteCount         = 0;
        value.acceptedSlotCount     = 0;
        value.presenceSeen          = true;
        value.releaseRequested      = true;
        value.releaseConfirmed      = true;
        value.completedAt           = adk::MicrosecondTimePoint (completedAt);
        value.status                = adk::StatusCode::Ok;
        value.ownerToken            = 0x12345678UL;
        value.lifecycleGeneration   = 3;
        value.configurationRevision = 23;
        value.transactionGeneration = generation;
        switch (operation)
        {
            case adk::OneWireOperation::SearchRomPass:
                value.acceptedSlotCount = 200;
                break;
            case adk::OneWireOperation::MatchRomStartConversion:
                value.acceptedSlotCount = 80;
                break;
            case adk::OneWireOperation::MatchRomReadConversionStatus:
                value.acceptedSlotCount = 1;
                value.readByteCount     = 1;
                value.readBytes[0]      = 1;
                break;
            case adk::OneWireOperation::MatchRomReadScratchpad:
                value.acceptedSlotCount = 152;
                value.readByteCount     = 9;
                break;
            default: break;
        }
        return value;
    }

    adk::Status addSearch (const adk::Qualified18B20ProbeSetPolicy& policy,
                           adk::Ds18b20CycleBuilder&                builder,
                           const adk::QualifiedDs18b20SetConfig&    cfg,
                           uint32_t transactionOffset = 0)
    {
        adk::OneWireSearchState request = {};
        const adk::OneWireRomCode zeroRom = {{0, 0, 0, 0, 0, 0, 0, 0}};
        for (uint8_t index = 0; index < 4; ++index)
        {
            adk::OneWireTransactionSnapshot value = transaction (
                adk::OneWireOperation::SearchRomPass, zeroRom,
                transactionOffset + 10 + index,
                transactionOffset * 10 + 100 + index * 10,
                transactionOffset * 10 + 105 + index * 10);
            value.request.search   = request;
            value.searchResult.rom = cfg.probes[index].rom;
            value.searchResult.lastDiscrepancy =
                index == 3 ? 0 : static_cast<uint8_t> (3 - index);
            value.searchResult.lastDevice = index == 3;
            value.returnedRom             = zeroRom;
            const adk::Status status =
                policy.ingestSearchPass (builder, value, request);
            if (!status.ok ())
            {
                return status;
            }
            request = value.searchResult;
        }
        return policy.finishSearch (builder, true, false, adk::StatusCode::Ok);
    }

    void setScratchpad (adk::OneWireTransactionSnapshot& value, int16_t raw,
                        adk::Ds18b20Resolution resolution)
    {
        value.readBytes[0] = static_cast<uint8_t> (raw & 0xff);
        value.readBytes[1] = static_cast<uint8_t> (static_cast<uint16_t> (raw) >> 8);
        value.readBytes[2] = 0x4b;
        value.readBytes[3] = 0x46;
        value.readBytes[4] = resolutionByte (resolution);
        value.readBytes[5] = 0xff;
        value.readBytes[6] = 0x0c;
        value.readBytes[7] = 0x10;
        value.readBytes[8] = crc8 (value.readBytes, 8);
    }

    adk::Status addProbe (const adk::Qualified18B20ProbeSetPolicy& policy,
                          adk::Ds18b20CycleBuilder&                builder,
                          const adk::Ds18b20ProbeConfig& probe, uint8_t index,
                          int16_t raw, uint32_t observedAt,
                          bool conversionHigh = true,
                          uint32_t transactionOffset = 0)
    {
        const uint32_t base =
            transactionOffset + 100 + static_cast<uint32_t> (index) * 3;
        const uint32_t timeBase =
            transactionOffset * 10 + 1000 +
            static_cast<uint32_t> (index) * 1000;
        adk::OneWireTransactionSnapshot start =
            transaction (adk::OneWireOperation::MatchRomStartConversion, probe.rom,
                         base, timeBase, timeBase + 10);
        const uint32_t conversionGeneration =
            transactionOffset + static_cast<uint32_t> (index) + 1U;
        adk::Status status =
            policy.ingestConversionStart (builder, conversionGeneration, start);

        if (!status.ok ())
        {
            return status;
        }
        adk::OneWireTransactionSnapshot complete =
            transaction (adk::OneWireOperation::MatchRomReadConversionStatus, probe.rom,
                         base + 1, timeBase + 100, timeBase + 110);
        complete.readBytes[0] = conversionHigh ? 1 : 0;
        status =
            policy.ingestConversionStatus (builder, conversionGeneration, complete);

        if (!status.ok () || !conversionHigh)
        {
            return status;
        }
        adk::OneWireTransactionSnapshot scratch =
            transaction (adk::OneWireOperation::MatchRomReadScratchpad, probe.rom,
                         base + 2, timeBase + 200, timeBase + 210);
        setScratchpad (scratch, raw, probe.resolution);

        return policy.ingestScratchpad (builder, conversionGeneration,
                                        adk::TimePoint (observedAt), scratch);
    }

    adk::QualifiedDs18b20Snapshot cycle (adk::Qualified18B20ProbeSetPolicy&    policy,
                                         const adk::QualifiedDs18b20SetConfig& cfg,
                                         uint32_t sequence, uint32_t now,
                                         const int16_t raw[4])
    {
        adk::Ds18b20CycleBuilder builder;
        require (policy
                     .beginCycle (adk::TimePoint (now), 7, 19, sequence,
                                  adk::TimePoint (now), builder)
                     .ok (),
                 "begin decode cycle");
        const uint32_t transactionOffset = (sequence - 1U) * 1000U;
        require (addSearch (policy, builder, cfg, transactionOffset).ok (),
                 "complete search");
        for (uint8_t index = 0; index < 4; ++index)
        {
            require (
                addProbe (policy, builder, cfg.probes[index], index, raw[index],
                          now, true, transactionOffset)
                    .ok (),
                "add completed probe");
        }
        adk::QualifiedDs18b20Snapshot result;
        const adk::Status finalizeStatus =
            policy.finalizeCycle (adk::TimePoint (now), builder, result);
        if (!finalizeStatus.ok ())
        {
            std::cerr << "decode finalize status: "
                      << static_cast<unsigned> (finalizeStatus.error ()) << '\n';
        }
        require (finalizeStatus.ok (), "finalize decode cycle");
        return result;
    }

    void testSignedValuesAndResolutionIntervals ()
    {
        const adk::QualifiedDs18b20SetConfig cfg = config ();

        adk::Qualified18B20ProbeSetPolicy    policy (cfg);

        require (policy.initialize ().ok (), "initialize decode policy");

        const int16_t                       raw[4] = {-1, -162, 401, 2000};
        const adk::QualifiedDs18b20Snapshot result = cycle (policy, cfg, 1, 1000, raw);
        const int16_t                       expectedRaw[4]   = {-8, -164, 400, 2000};
        const int16_t                       expectedLower[4] = {-8, -164, 400, 2000};
        const int16_t                       expectedUpper[4] = {-1, -161, 401, 2000};
        for (uint8_t index = 0; index < 4; ++index)
        {
            require (romEqual (result.probes[index].rom, cfg.probes[index].rom),
                     "configured slot order retained");
            require (result.probes[index].rawSixteenths == expectedRaw[index],
                     "signed resolution-masked raw value retained");
            require (result.probes[index].lowerRawSixteenths == expectedLower[index],
                     "resolution lower interval");
            require (result.probes[index].upperRawSixteenths == expectedUpper[index],
                     "resolution upper interval");
            require (result.probes[index].quality == adk::Ds18b20ProbeQuality::Current,
                     "decoded probe current");
        }
    }

    int16_t maskedRaw (int16_t raw, adk::Ds18b20Resolution resolution)
    {
        const uint8_t undefinedMask = static_cast<uint8_t> (
            (UINT8_C (1) << (3U - static_cast<uint8_t> (resolution))) - 1U);
        return static_cast<int16_t> (
            static_cast<uint16_t> (raw) &
            static_cast<uint16_t> (~undefinedMask));
    }

    void testLiteralCrcAndSignedEndpointsAtEveryResolution ()
    {
        const uint8_t literalScratchpad[9] = {
            0x50, 0x05, 0x4b, 0x46, 0x7f, 0xff, 0x0c, 0x10, 0x1c};
        require (crc8 (literalScratchpad, 8) == literalScratchpad[8],
                 "literal DS18B20 +85 scratchpad CRC vector");

        const int16_t rawValues[5] = {0, 401, -162, -880, 2000};
        for (uint8_t resolutionIndex = 0; resolutionIndex < 4;
             ++resolutionIndex)
        {
            adk::QualifiedDs18b20SetConfig cfg = config ();
            const adk::Ds18b20Resolution resolution =
                static_cast<adk::Ds18b20Resolution> (resolutionIndex);
            for (uint8_t slot = 0; slot < 4; ++slot)
            {
                cfg.probes[slot].resolution = resolution;
                const uint16_t deadlines[4] = {94, 188, 375, 750};
                cfg.probes[slot].conversionDeadline =
                    adk::Duration (deadlines[resolutionIndex]);
                cfg.probes[slot].maximumRawSixteenths = 2007;
            }
            for (uint8_t vectorIndex = 0; vectorIndex < 5; ++vectorIndex)
            {
                adk::Qualified18B20ProbeSetPolicy policy (cfg);

                require (policy.initialize ().ok (),
                         "initialize signed endpoint policy");
                const int16_t raw[4] = {
                    rawValues[vectorIndex], rawValues[vectorIndex],
                    rawValues[vectorIndex], rawValues[vectorIndex]};
                const adk::QualifiedDs18b20Snapshot result =
                    cycle (policy, cfg, 1, 1000, raw);
                const int16_t expected =
                    maskedRaw (rawValues[vectorIndex], resolution);
                const int16_t upper = static_cast<int16_t> (
                    expected +
                    ((UINT8_C (1)
                      << (3U - static_cast<uint8_t> (resolution))) -
                     1U));
                require (result.probes[0].rawSixteenths == expected,
                         "signed endpoint masked at each resolution");
                require (result.probes[0].lowerRawSixteenths == expected,
                         "endpoint lower interval at each resolution");
                require (result.probes[0].upperRawSixteenths == upper,
                         "endpoint upper interval at each resolution");
            }
        }
    }

    adk::QualifiedDs18b20Snapshot
    cycleWithFirstScratchpadConfig (adk::Qualified18B20ProbeSetPolicy& policy,
                                    const adk::QualifiedDs18b20SetConfig& cfg,
                                    uint8_t configByte)
    {
        adk::Ds18b20CycleBuilder builder;
        require (policy.beginCycle (adk::TimePoint (1000), 7, 19, 1,
                                    adk::TimePoint (1000), builder)
                     .ok (),
                 "begin configuration-byte cycle");
        require (addSearch (policy, builder, cfg).ok (),
                 "configuration-byte search");
        for (uint8_t index = 0; index < 4; ++index)
        {
            const uint32_t base = 100 + static_cast<uint32_t> (index) * 3;
            const uint32_t timeBase =
                1000 + static_cast<uint32_t> (index) * 1000;
            adk::OneWireTransactionSnapshot start =
                transaction (adk::OneWireOperation::MatchRomStartConversion,
                             cfg.probes[index].rom, base, timeBase,
                             timeBase + 10);
            require (policy.ingestConversionStart (builder, index + 1, start)
                         .ok (),
                     "configuration-byte conversion start");
            adk::OneWireTransactionSnapshot complete =
                transaction (
                    adk::OneWireOperation::MatchRomReadConversionStatus,
                    cfg.probes[index].rom, base + 1, timeBase + 100,
                    timeBase + 110);
            require (policy.ingestConversionStatus (builder, index + 1,
                                                    complete)
                         .ok (),
                     "configuration-byte conversion complete");
            adk::OneWireTransactionSnapshot scratch =
                transaction (adk::OneWireOperation::MatchRomReadScratchpad,
                             cfg.probes[index].rom, base + 2, timeBase + 200,
                             timeBase + 210);
            setScratchpad (scratch, 320, cfg.probes[index].resolution);
            if (index == 0)
            {
                scratch.readBytes[4] = configByte;
                scratch.readBytes[8] = crc8 (scratch.readBytes, 8);
            }
            require (policy.ingestScratchpad (builder, index + 1,
                                              adk::TimePoint (1000), scratch)
                         .ok (),
                     "configuration-byte scratchpad");
        }
        adk::QualifiedDs18b20Snapshot result;
        require (policy.finalizeCycle (adk::TimePoint (1000), builder, result)
                     .ok (),
                 "finalize configuration-byte cycle");
        return result;
    }

    void testReservedConfigurationBitsAndResolutionMismatch ()
    {
        const adk::QualifiedDs18b20SetConfig cfg = config ();
        const uint8_t malformed[2] = {0xff, 0x7e};
        for (uint8_t index = 0; index < 2; ++index)
        {
            adk::Qualified18B20ProbeSetPolicy policy (cfg);

            require (policy.initialize ().ok (),
                     "initialize reserved-bit policy");
            const adk::QualifiedDs18b20Snapshot result =
                cycleWithFirstScratchpadConfig (policy, cfg, malformed[index]);
            require (result.probes[0].quality ==
                         adk::Ds18b20ProbeQuality::ResolutionMismatch,
                     "reserved configuration bits rejected");
        }

        adk::Qualified18B20ProbeSetPolicy policy (cfg);

        require (policy.initialize ().ok (), "initialize mismatch policy");
        const adk::QualifiedDs18b20Snapshot mismatch =
            cycleWithFirstScratchpadConfig (
                policy, cfg,
                resolutionByte (adk::Ds18b20Resolution::Bits12));
        require (mismatch.probes[0].quality ==
                     adk::Ds18b20ProbeQuality::ResolutionMismatch,
                 "proved resolution must equal configured resolution");
    }

    void testScratchpadCrcCoversEveryByte ()
    {
        const adk::QualifiedDs18b20SetConfig cfg = config ();
        for (uint8_t corruptIndex = 0; corruptIndex < 9; ++corruptIndex)
        {
            adk::Qualified18B20ProbeSetPolicy policy (cfg);

            require (policy.initialize ().ok (), "initialize CRC policy");
            adk::Ds18b20CycleBuilder builder;
            require (policy
                         .beginCycle (adk::TimePoint (1000), 7, 19, 1,
                                      adk::TimePoint (1000), builder)
                         .ok (),
                     "begin CRC cycle");
            require (addSearch (policy, builder, cfg).ok (), "CRC search");

            for (uint8_t index = 0; index < 4; ++index)
            {
                const uint32_t base = 100 + static_cast<uint32_t> (index) * 3;
                const uint32_t timeBase =
                    1000 + static_cast<uint32_t> (index) * 1000;
                adk::OneWireTransactionSnapshot start =
                    transaction (adk::OneWireOperation::MatchRomStartConversion,
                                 cfg.probes[index].rom, base, timeBase,
                                 timeBase + 10);
                require (policy.ingestConversionStart (builder, index + 1, start).ok (),
                         "CRC conversion start");
                adk::OneWireTransactionSnapshot complete =
                    transaction (adk::OneWireOperation::MatchRomReadConversionStatus,
                                 cfg.probes[index].rom, base + 1,
                                 timeBase + 100, timeBase + 110);
                require (
                    policy.ingestConversionStatus (builder, index + 1, complete).ok (),
                    "CRC conversion complete");
                adk::OneWireTransactionSnapshot scratch =
                    transaction (adk::OneWireOperation::MatchRomReadScratchpad,
                                 cfg.probes[index].rom, base + 2,
                                 timeBase + 200, timeBase + 210);
                setScratchpad (scratch, 320, cfg.probes[index].resolution);
                if (index == 0)
                {
                    scratch.readBytes[corruptIndex] ^= 0x01;
                }
                require (policy
                             .ingestScratchpad (builder, index + 1,
                                                adk::TimePoint (1000), scratch)
                             .ok (),
                         "ingest corrupt scratchpad evidence");
            }
            adk::QualifiedDs18b20Snapshot result;
            require (
                policy.finalizeCycle (adk::TimePoint (1000), builder, result).ok (),
                "commit CRC fault");
            require (result.probes[0].quality ==
                         adk::Ds18b20ProbeQuality::ScratchpadCrcFault,
                     "each scratchpad byte participates in CRC");
            require ((result.faultMask & 1U) != 0, "CRC fault marked");
            require (result.validCount == 3, "CRC fault does not erase peers");
        }
    }

    void testFreshnessAndStepBoundaries ()
    {
        adk::QualifiedDs18b20SetConfig cfg = config ();
        for (uint8_t index = 0; index < 4; ++index)
        {
            cfg.probes[index].maximumStepRawSixteenths = 16;
            cfg.probes[index].maximumAge               = adk::Duration (100);
            cfg.probes[index].resolution               = adk::Ds18b20Resolution::Bits12;
        }
        adk::Qualified18B20ProbeSetPolicy policy (cfg);

        require (policy.initialize ().ok (), "initialize boundary policy");
        const int16_t baseline[4] = {320, 320, 320, 320};
        cycle (policy, cfg, 1, 1000, baseline);

        const int16_t                 atStep[4] = {336, 304, 336, 304};
        adk::QualifiedDs18b20Snapshot result    = cycle (policy, cfg, 2, 1100, atStep);

        require (result.probes[0].quality == adk::Ds18b20ProbeQuality::Current,
                 "positive step at maximum accepted");
        require (result.probes[1].quality == adk::Ds18b20ProbeQuality::Current,
                 "negative step at maximum accepted");
        require (result.probes[0].age.milliseconds () == 0,
                 "fresh sample age starts at scratchpad observation");

        const int16_t pastStep[4] = {353, 287, 337, 303};
        result                    = cycle (policy, cfg, 3, 1200, pastStep);

        require (result.probes[0].quality == adk::Ds18b20ProbeQuality::ImplausibleStep,
                 "positive step one past maximum warned");
        require (result.probes[1].quality == adk::Ds18b20ProbeQuality::ImplausibleStep,
                 "negative step one past maximum warned");

        const int16_t nextFromWarning[4] = {354, 286, 338, 302};
        result = cycle (policy, cfg, 4, 1300, nextFromWarning);

        require (result.probes[0].quality == adk::Ds18b20ProbeQuality::Current,
                 "warned positive sample becomes next baseline");
        require (result.probes[1].quality == adk::Ds18b20ProbeQuality::Current,
                 "warned negative sample becomes next baseline");
    }

    void testStepBelowAtAndOnePastBothDirections ()
    {
        const int16_t deltas[6] = {15, 16, 17, -15, -16, -17};
        for (uint8_t caseIndex = 0; caseIndex < 6; ++caseIndex)
        {
            adk::QualifiedDs18b20SetConfig cfg = config ();
            for (uint8_t slot = 0; slot < 4; ++slot)
            {
                cfg.probes[slot].resolution =
                    adk::Ds18b20Resolution::Bits12;
            }
            adk::Qualified18B20ProbeSetPolicy policy (cfg);

            require (policy.initialize ().ok (), "initialize exact-step policy");
            const int16_t baseline[4] = {320, 320, 320, 320};
            cycle (policy, cfg, 1, 1000, baseline);
            const int16_t candidate[4] = {
                static_cast<int16_t> (320 + deltas[caseIndex]), 320, 320, 320};
            const adk::QualifiedDs18b20Snapshot result =
                cycle (policy, cfg, 2, 1100, candidate);
            const bool onePast = caseIndex == 2 || caseIndex == 5;
            require (
                result.probes[0].quality ==
                    (onePast ? adk::Ds18b20ProbeQuality::ImplausibleStep
                             : adk::Ds18b20ProbeQuality::Current),
                "step below, at, and one past in both directions");
        }
    }

    void testFreshnessBelowAtAndPastMaximum ()
    {
        const adk::QualifiedDs18b20SetConfig cfg = config ();

        const uint32_t nowValues[3] = {1099, 1100, 1101};
        for (uint8_t boundary = 0; boundary < 3; ++boundary)
        {
            adk::Qualified18B20ProbeSetPolicy policy (cfg);

            require (policy.initialize ().ok (), "initialize freshness policy");
            adk::Ds18b20CycleBuilder builder;
            require (policy
                         .beginCycle (adk::TimePoint (nowValues[boundary]), 7, 19,
                                      1, adk::TimePoint (1000), builder)
                         .ok (),
                     "begin freshness cycle");
            require (addSearch (policy, builder, cfg).ok (), "freshness search");
            for (uint8_t index = 0; index < 4; ++index)
            {
                require (
                    addProbe (policy, builder, cfg.probes[index], index, 320, 1000)
                        .ok (),
                    "freshness probe");
            }

            adk::QualifiedDs18b20Snapshot result;
            require (policy
                         .finalizeCycle (adk::TimePoint (nowValues[boundary]),
                                         builder, result)
                         .ok (),
                     "finalize freshness boundary");
            require (result.probes[0].age.milliseconds () ==
                         99U + boundary,
                     "freshness boundary age retained");
            const adk::Ds18b20ProbeQuality expected =
                boundary < 2 ? adk::Ds18b20ProbeQuality::Current
                             : adk::Ds18b20ProbeQuality::Stale;
            require (result.probes[0].quality == expected,
                     "freshness below, at, and past maximum classified");
        }
    }

    adk::Status finalizeAtTimes (
        adk::Qualified18B20ProbeSetPolicy& policy,
        const adk::QualifiedDs18b20SetConfig& cfg, uint32_t now,
        uint32_t scratchpadObservedAt, adk::QualifiedDs18b20Snapshot& output)
    {
        adk::Ds18b20CycleBuilder builder;
        adk::Status status =
            policy.beginCycle (adk::TimePoint (now), 7, 19, 1,
                               adk::TimePoint (scratchpadObservedAt), builder);
        if (!status.ok ())
        {
            return status;
        }
        status = addSearch (policy, builder, cfg);

        if (!status.ok ())
        {
            return status;
        }
        for (uint8_t index = 0; index < 4; ++index)
        {
            status = addProbe (policy, builder, cfg.probes[index], index, 320,
                               scratchpadObservedAt);
            if (!status.ok ())
            {
                return status;
            }
        }
        return policy.finalizeCycle (adk::TimePoint (now), builder, output);
    }

    void testTimeZeroRolloverAndHalfRangeAtomicity ()
    {
        const adk::QualifiedDs18b20SetConfig cfg = config ();
        {
            adk::Qualified18B20ProbeSetPolicy policy (cfg);

            require (policy.initialize ().ok (), "initialize time-zero policy");
            adk::QualifiedDs18b20Snapshot result;
            require (finalizeAtTimes (policy, cfg, 0, 0, result).ok (),
                     "time zero accepted");
            require (result.probes[0].age.milliseconds () == 0,
                     "time-zero age");
        }
        {
            adk::Qualified18B20ProbeSetPolicy policy (cfg);

            require (policy.initialize ().ok (), "initialize rollover policy");
            adk::QualifiedDs18b20Snapshot result;
            require (finalizeAtTimes (policy, cfg, 25, UINT32_MAX - 50U,
                                      result)
                         .ok (),
                     "freshness rollover accepted");
            require (result.probes[0].age.milliseconds () == 76,
                     "freshness rollover age");
        }
        {
            adk::Qualified18B20ProbeSetPolicy policy (cfg);

            require (policy.initialize ().ok (), "initialize half-range policy");
            adk::QualifiedDs18b20Snapshot output;
            require (policy.snapshot (output).ok (),
                     "seed half-range output canary");
            output.sourceId      = 0xa5;
            output.cycleSequence = UINT32_C (0xa5a5a5a5);
            adk::QualifiedDs18b20Snapshot canary = output;
            const adk::Status status =
                finalizeAtTimes (policy, cfg, 0, UINT32_C (0x80000000),
                                 output);
            require (status == adk::StatusCode::InvalidArgument,
                     "freshness half-range rejected");
            require (std::memcmp (&output, &canary, sizeof output) == 0,
                     "failed half-range finalization preserves output canary");
            adk::QualifiedDs18b20Snapshot retained;
            require (policy.snapshot (retained).ok (),
                     "snapshot remains readable after atomic rejection");
            require (retained.quality == adk::Ds18b20SetQuality::Unqualified,
                     "failed half-range finalization preserves policy state");
        }
    }

    void testPlus85AfterCompletedConversion ()
    {
        const adk::QualifiedDs18b20SetConfig cfg = config ();

        adk::Qualified18B20ProbeSetPolicy    policy (cfg);

        require (policy.initialize ().ok (), "initialize +85 policy");
        const int16_t                       raw[4] = {1360, 1360, 1360, 1360};
        const adk::QualifiedDs18b20Snapshot result = cycle (policy, cfg, 1, 1000, raw);
        for (uint8_t index = 0; index < 4; ++index)
        {
            require (result.probes[index].rawSixteenths == 1360,
                     "+85 raw value retained");
            require (result.probes[index].quality == adk::Ds18b20ProbeQuality::Current,
                     "+85 after correlated conversion is current");
        }
    }

    void testPlus85WithoutCompletedConversionIsResetDefault ()
    {
        const adk::QualifiedDs18b20SetConfig cfg = config ();

        adk::Qualified18B20ProbeSetPolicy    policy (cfg);

        require (policy.initialize ().ok (), "initialize reset-default policy");
        adk::Ds18b20CycleBuilder builder;
        require (policy
                     .beginCycle (adk::TimePoint (1000), 7, 19, 1,
                                  adk::TimePoint (1000), builder)
                     .ok (),
                 "begin reset-default cycle");
        require (addSearch (policy, builder, cfg).ok (), "reset-default search");

        adk::OneWireTransactionSnapshot start =
            transaction (adk::OneWireOperation::MatchRomStartConversion,
                         cfg.probes[0].rom, 100, 1000, 1010);
        require (policy.ingestConversionStart (builder, 1, start).ok (),
                 "ingest reset-default conversion start");

        adk::OneWireTransactionSnapshot incomplete =
            transaction (adk::OneWireOperation::MatchRomReadConversionStatus,
                         cfg.probes[0].rom, 101, 1050, 1060);
        incomplete.readBytes[0] = 0;
        require (policy.ingestConversionStatus (builder, 1, incomplete).ok (),
                 "ingest incomplete conversion status");

        adk::OneWireTransactionSnapshot scratch =
            transaction (adk::OneWireOperation::MatchRomReadScratchpad,
                         cfg.probes[0].rom, 102, 1200, 1210);
        setScratchpad (scratch, 1360, cfg.probes[0].resolution);

        require (
            policy.ingestScratchpad (builder, 1, adk::TimePoint (1000), scratch).ok (),
            "ingest +85 without completion");
        for (uint8_t index = 1; index < 4; ++index)
        {
            require (
                addProbe (policy, builder, cfg.probes[index], index, 320, 1000).ok (),
                "add completed peer probe");
        }
        adk::QualifiedDs18b20Snapshot result;
        require (policy.finalizeCycle (adk::TimePoint (1000), builder, result).ok (),
                 "commit reset-default evidence");
        require (result.probes[0].quality ==
                     adk::Ds18b20ProbeQuality::ResetDefaultWithoutConversion,
                 "+85 without completed conversion is reset default");
        require (result.probes[0].rawSixteenths == 1360,
                 "reset-default raw register value remains inspectable");
    }

    adk::Status addSearchWithoutFirst (
        const adk::Qualified18B20ProbeSetPolicy& policy,
        adk::Ds18b20CycleBuilder& builder,
        const adk::QualifiedDs18b20SetConfig& cfg, uint32_t transactionOffset)
    {
        adk::OneWireSearchState request = {};
        const adk::OneWireRomCode zeroRom = {{0, 0, 0, 0, 0, 0, 0, 0}};
        for (uint8_t index = 1; index < 4; ++index)
        {
            adk::OneWireTransactionSnapshot value = transaction (
                adk::OneWireOperation::SearchRomPass, zeroRom,
                transactionOffset + 10 + index,
                transactionOffset * 10 + 100 + index * 10,
                transactionOffset * 10 + 105 + index * 10);
            value.request.search            = request;
            value.searchResult.rom          = cfg.probes[index].rom;
            value.searchResult.lastDiscrepancy =
                index == 3 ? 0 : static_cast<uint8_t> (3 - index);
            value.searchResult.lastDevice = index == 3;
            value.returnedRom             = zeroRom;
            const adk::Status status =
                policy.ingestSearchPass (builder, value, request);
            if (!status.ok ())
            {
                return status;
            }
            request = value.searchResult;
        }
        return policy.finishSearch (builder, true, false,
                                    adk::StatusCode::Ok);
    }

    void testSideQualitiesNeverGrowStepTrust ()
    {
        adk::QualifiedDs18b20SetConfig cfg = config ();
        for (uint8_t index = 0; index < 4; ++index)
        {
            cfg.probes[index].maximumStepRawSixteenths = 16;
        }
        adk::Qualified18B20ProbeSetPolicy policy (cfg);

        require (policy.initialize ().ok (), "initialize trust-marker policy");

        adk::Ds18b20CycleBuilder resetDefault;
        require (policy.beginCycle (adk::TimePoint (1000), 7, 19, 1,
                                    adk::TimePoint (1000), resetDefault)
                     .ok (),
                 "begin reset-default trust cycle");
        require (addSearch (policy, resetDefault, cfg).ok (),
                 "reset-default trust search");
        adk::OneWireTransactionSnapshot start =
            transaction (adk::OneWireOperation::MatchRomStartConversion,
                         cfg.probes[0].rom, 100, 1000, 1010);
        require (policy.ingestConversionStart (resetDefault, 1, start).ok (),
                 "trust reset-default conversion start");
        adk::OneWireTransactionSnapshot low =
            transaction (adk::OneWireOperation::MatchRomReadConversionStatus,
                         cfg.probes[0].rom, 101, 1100, 1110);
        low.readBytes[0] = 0;
        require (policy.ingestConversionStatus (resetDefault, 1, low).ok (),
                 "trust reset-default conversion low");
        adk::OneWireTransactionSnapshot scratch =
            transaction (adk::OneWireOperation::MatchRomReadScratchpad,
                         cfg.probes[0].rom, 102, 1200, 1210);
        setScratchpad (scratch, 1360, cfg.probes[0].resolution);

        require (policy.ingestScratchpad (resetDefault, 1,
                                          adk::TimePoint (1000), scratch)
                     .ok (),
                 "trust reset-default scratchpad");
        for (uint8_t index = 1; index < 4; ++index)
        {
            require (addProbe (policy, resetDefault, cfg.probes[index], index,
                               320, 1000)
                         .ok (),
                     "trust reset-default peer");
        }
        adk::QualifiedDs18b20Snapshot result;
        require (policy.finalizeCycle (adk::TimePoint (1000), resetDefault,
                                       result)
                     .ok (),
                 "commit reset-default trust cycle");
        require (result.probes[0].quality ==
                     adk::Ds18b20ProbeQuality::ResetDefaultWithoutConversion,
                 "reset-default does not establish trust");

        adk::Ds18b20CycleBuilder missing;
        require (policy.beginCycle (adk::TimePoint (2000), 7, 19, 2,
                                    adk::TimePoint (2000), missing)
                     .ok (),
                 "begin missing carry cycle");
        require (addSearchWithoutFirst (policy, missing, cfg, 1000).ok (),
                 "complete search proves first probe missing");
        require (policy.finalizeCycle (adk::TimePoint (2000), missing, result) ==
                     adk::StatusCode::HardwareFailure,
                 "commit missing carry cycle");
        require (result.probes[0].quality ==
                     adk::Ds18b20ProbeQuality::Missing,
                 "missing carries without trust");

        adk::Ds18b20CycleBuilder transport;
        require (policy.beginCycle (adk::TimePoint (3000), 7, 19, 3,
                                    adk::TimePoint (3000), transport)
                     .ok (),
                 "begin no-witness transport cycle");
        require (policy.finishSearch (transport, false, false,
                                      adk::StatusCode::HardwareFailure)
                     .ok (),
                 "finish no-witness transport cycle");
        require (policy.finalizeCycle (adk::TimePoint (3000), transport,
                                       result) ==
                     adk::StatusCode::HardwareFailure,
                 "commit no-witness transport carry");
        require (result.probes[0].quality ==
                     adk::Ds18b20ProbeQuality::TransportFault,
                 "transport carries without trust");

        adk::Ds18b20CycleBuilder pending;
        require (policy.beginCycle (adk::TimePoint (4000), 7, 19, 4,
                                    adk::TimePoint (4000), pending)
                     .ok (),
                 "begin pending carry cycle");
        require (addSearch (policy, pending, cfg, 3000).ok (),
                 "pending carry search");
        start = transaction (adk::OneWireOperation::MatchRomStartConversion,
                             cfg.probes[0].rom, 3100, 31000, 31010);
        require (policy.ingestConversionStart (pending, 3001, start).ok (),
                 "pending carry conversion start");
        low = transaction (adk::OneWireOperation::MatchRomReadConversionStatus,
                           cfg.probes[0].rom, 3101, 31100, 31110);
        low.readBytes[0] = 0;
        require (policy.ingestConversionStatus (pending, 3001, low).ok (),
                 "pending carry status low");
        for (uint8_t index = 1; index < 4; ++index)
        {
            require (addProbe (policy, pending, cfg.probes[index], index, 320,
                               4000, true, 3000)
                         .ok (),
                     "pending carry peer");
        }
        require (policy.finalizeCycle (adk::TimePoint (4000), pending, result)
                     .ok (),
                 "commit pending carry cycle");
        require (result.probes[0].quality ==
                     adk::Ds18b20ProbeQuality::ConversionPending,
                 "pending carries without trust");

        const int16_t farSample[4] = {-800, 320, 320, 320};
        result = cycle (policy, cfg, 5, 5000, farSample);

        require (result.probes[0].quality ==
                     adk::Ds18b20ProbeQuality::Current,
                 "first trusted sample ignores side-quality carried values");
    }
} // namespace

int main ()
{
    testSignedValuesAndResolutionIntervals ();

    testLiteralCrcAndSignedEndpointsAtEveryResolution ();

    testReservedConfigurationBitsAndResolutionMismatch ();

    testScratchpadCrcCoversEveryByte ();

    testFreshnessAndStepBoundaries ();

    testStepBelowAtAndOnePastBothDirections ();

    testFreshnessBelowAtAndPastMaximum ();

    testTimeZeroRolloverAndHalfRangeAtomicity ();

    testPlus85AfterCompletedConversion ();

    testPlus85WithoutCompletedConversionIsResetDefault ();

    testSideQualitiesNeverGrowStepTrust ();

    std::cout << "qualified 18B20 decode tests passed\n";
    return EXIT_SUCCESS;
}
