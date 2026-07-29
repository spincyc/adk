#include <qualified_18b20_probe_set_policy.h>

#include <cstdint>
#include <cstring>
#include <cstdlib>
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
                    crc ^= 0x8CU;
                }
                value >>= 1U;
            }
        }
        return crc;
    }

    adk::OneWireRomCode rom (uint8_t serial)
    {
        adk::OneWireRomCode value = {{0x28, serial, uint8_t (serial + 1U),
                                      uint8_t (serial + 2U), uint8_t (serial + 3U),
                                      uint8_t (serial + 4U), uint8_t (serial + 5U), 0}};
        value.bytes[7]            = crc8 (value.bytes, 7);
        return value;
    }

    adk::OneWireRomCode emptyRom ()
    {
        return {{0, 0, 0, 0, 0, 0, 0, 0}};
    }

    bool sameRom (const adk::OneWireRomCode& left, const adk::OneWireRomCode& right)
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

    adk::QualifiedDs18b20SetConfig config ()
    {
        static const adk::OneWireRomCode knownGood[4] = {
            {{0x28, 0x04, 0, 0, 0, 0, 0, 0xc2}},
            {{0x28, 0x02, 0, 0, 0, 0, 0, 0x70}},
            {{0x28, 0x01, 0, 0, 0, 0, 0, 0x29}},
            {{0x28, 0x03, 0, 0, 0, 0, 0, 0x47}}};
        const adk::Ds18b20ProbeConfig  empty = {emptyRom (),
                                                adk::Ds18b20Resolution::Bits12,
                                                adk::Duration (),
                                                adk::Duration (),
                                                0,
                                                0,
                                                0};
        adk::QualifiedDs18b20SetConfig value = {
            7, 19, 0x12345678UL, 23, {empty, empty, empty, empty}};
        value.expectedSourceId                     = 7;
        value.expectedConfigurationRevision        = 19;
        value.expectedOneWireOwnerToken            = 0x12345678UL;
        value.expectedOneWireConfigurationRevision = 23;
        for (uint8_t index = 0; index < 4; ++index)
        {
            value.probes[index].rom                = knownGood[index];
            value.probes[index].resolution         = adk::Ds18b20Resolution::Bits12;
            value.probes[index].conversionDeadline = adk::Duration (750);
            value.probes[index].maximumAge         = adk::Duration (5000);
            value.probes[index].minimumRawSixteenths     = -880;
            value.probes[index].maximumRawSixteenths     = 2000;
            value.probes[index].maximumStepRawSixteenths = 160;
        }
        return value;
    }

    adk::QualifiedDs18b20Snapshot blankSnapshot ()
    {
        const adk::QualifiedDs18b20Probe empty = {emptyRom (),
                                                  0,
                                                  0,
                                                  0,
                                                  adk::TimePoint (),
                                                  adk::TimePoint (),
                                                  0,
                                                  0,
                                                  0,
                                                  adk::Ds18b20Resolution::Bits12,
                                                  adk::Ds18b20ProbeQuality::Unqualified,
                                                  adk::Duration (),
                                                  adk::StatusCode::NotInitialized};
        return {0,
                0,
                0,
                adk::TimePoint (),
                {empty, empty, empty, empty},
                0,
                0,
                0,
                adk::Ds18b20SetQuality::Unqualified,
                adk::StatusCode::NotInitialized};
    }

    adk::OneWireSearchState searchState (const adk::OneWireRomCode& value,
                                         uint8_t discrepancy, bool last)
    {
        return {value, discrepancy, last};
    }

    adk::OneWireTransactionSnapshot
    searchSnapshot (uint32_t requestSequence, uint32_t generation,
                    const adk::OneWireSearchState& requestSearch,
                    const adk::OneWireSearchState& result)
    {
        adk::OneWireOperationRequest request = {
            requestSequence,
            adk::OneWireOperation::SearchRomPass,
            emptyRom (),
            requestSearch,
            adk::MicrosecondTimePoint (1000U + generation * 1000U),
            adk::OneWireSupplyMode::ExternallyPowered,
            adk::StatusCode::Ok};
        return {adk::OneWireOperation::SearchRomPass,
                adk::OneWirePhase::Complete,
                adk::OneWireTransactionQuality::Complete,
                request,
                result,
                emptyRom (),
                {0, 0, 0, 0, 0, 0, 0, 0, 0},
                0,
                200,
                true,
                true,
                true,
                adk::MicrosecondTimePoint (1500U + generation * 1000U),
                adk::StatusCode::Ok,
                0x12345678UL,
                3,
                23,
                generation};
    }

    void initialize (adk::Qualified18B20ProbeSetPolicy& policy)
    {
        require (policy.initialize ().ok (), "policy initializes");
        require (policy.initialized (), "initialized state is observable");
    }

    void begin (const adk::Qualified18B20ProbeSetPolicy& policy,
                adk::Ds18b20CycleBuilder& builder, uint32_t sequence = 1,
                uint32_t observedAt = 9000)
    {
        require (policy
                     .beginCycle (adk::TimePoint (10000), 7, 19, sequence,
                                  adk::TimePoint (observedAt), builder)
                     .ok (),
                 "cycle begins");
    }

    void ingestChain (const adk::Qualified18B20ProbeSetPolicy& policy,
                      adk::Ds18b20CycleBuilder&                builder,
                      const adk::OneWireRomCode* identities, uint8_t count,
                      bool terminal)
    {
        adk::OneWireSearchState request = {emptyRom (), 0, false};
        for (uint8_t index = 0; index < count; ++index)
        {
            const bool                    last = terminal && index + 1U == count;
            const adk::OneWireSearchState result =
                searchState (identities[index], last ? 0 : uint8_t (index + 1U), last);
            const adk::OneWireTransactionSnapshot transaction = searchSnapshot (
                uint32_t (index + 1U), uint32_t (index + 1U), request, result);
            require (policy.ingestSearchPass (builder, transaction, request).ok (),
                     "search pass ingests");
            request = result;
        }
    }

    bool sameBuilderImage (const adk::Ds18b20CycleBuilder& left,
                           const adk::Ds18b20CycleBuilder& right)
    {
        return std::memcmp (&left, &right, sizeof left) == 0;
    }

    bool sameBuilderBytes (
        const adk::Ds18b20CycleBuilder& builder,
        const unsigned char (&bytes)[sizeof (adk::Ds18b20CycleBuilder)])
    {
        return std::memcmp (&builder, bytes, sizeof builder) == 0;
    }

    adk::QualifiedDs18b20Snapshot finalize (
        adk::Qualified18B20ProbeSetPolicy& policy,
        const adk::Ds18b20CycleBuilder& builder, adk::StatusCode status)
    {
        adk::QualifiedDs18b20Snapshot output = blankSnapshot ();

        require (policy.finalizeCycle (adk::TimePoint (10000), builder, output)
                     .error () == status,
                 "finalized search returns expected status");
        return output;
    }

    void testLifecycleAndConfiguration ()
    {
        const adk::QualifiedDs18b20SetConfig good = config ();

        adk::Qualified18B20ProbeSetPolicy    policy (good);

        adk::QualifiedDs18b20Snapshot        output = blankSnapshot ();

        require (policy.snapshot (output).error () == adk::StatusCode::NotInitialized,
                 "snapshot rejects inert construction");
        adk::Ds18b20CycleBuilder builder;
        require (policy.beginCycle (adk::TimePoint (2), 7, 19, 1, adk::TimePoint (1),
                                    builder)
                         .error () == adk::StatusCode::NotInitialized,
                 "begin rejects inert construction");
        initialize (policy);

        require (policy.snapshot (output).ok (), "initialized snapshot succeeds");

        require (output.validCount == 0 && output.presentMask == 0 &&
                     output.quality == adk::Ds18b20SetQuality::Unqualified,
                 "initialize publishes exact empty state");
        for (uint8_t index = 0; index < 4; ++index)
        {
            require (sameRom (output.probes[index].rom, good.probes[index].rom),
                     "configured order survives initialization");
            require (output.probes[index].quality ==
                         adk::Ds18b20ProbeQuality::Unqualified,
                     "configured slot starts unqualified");
        }
        policy.reset ();

        require (policy.initialized (), "reset preserves initialized lifecycle");

        require (policy.snapshot (output).ok (), "snapshot succeeds after reset");
        for (uint8_t index = 0; index < 4; ++index)
        {
            require (sameRom (output.probes[index].rom, good.probes[index].rom) &&
                         output.probes[index].quality ==
                             adk::Ds18b20ProbeQuality::Unqualified,
                     "reset preserves identity and clears volatile evidence");
        }

        for (uint8_t byte = 0; byte < 8; ++byte)
        {
            adk::QualifiedDs18b20SetConfig damaged = good;
            damaged.probes[2].rom.bytes[byte] ^= 1U;
            adk::Qualified18B20ProbeSetPolicy bad (damaged);

            require (bad.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "every configured ROM-byte corruption rejects");
            const bool remainsInert = !bad.initialized ();

            require (remainsInert, "failed initialize stays inert");
        }

        adk::QualifiedDs18b20SetConfig family = good;
        family.probes[1].rom.bytes[0]         = 0x10;
        family.probes[1].rom.bytes[7]         = crc8 (family.probes[1].rom.bytes, 7);

        adk::Qualified18B20ProbeSetPolicy wrongFamily (family);

        require (wrongFamily.initialize ().error () ==
                     adk::StatusCode::InvalidConfiguration,
                 "non-18B20 family rejects even with valid CRC");

        adk::QualifiedDs18b20SetConfig duplicate = good;
        duplicate.probes[3].rom                  = duplicate.probes[0].rom;
        adk::Qualified18B20ProbeSetPolicy repeated (duplicate);

        require (repeated.initialize ().error () ==
                     adk::StatusCode::InvalidConfiguration,
                 "duplicate configured identity rejects");
    }

    void requireInvalidConfig (const adk::QualifiedDs18b20SetConfig& value,
                               const char* message)
    {
        adk::Qualified18B20ProbeSetPolicy policy (value);

        require (policy.initialize ().error () ==
                     adk::StatusCode::InvalidConfiguration,
                 message);
        require (!policy.initialized (), "invalid configuration stays inert");
    }

    void testConfigurationMatrixAndCopyIsolation ()
    {
        const adk::QualifiedDs18b20SetConfig good = config ();
        adk::QualifiedDs18b20SetConfig damaged = good;
        damaged.expectedSourceId = 0;
        requireInvalidConfig (damaged, "zero source rejects");
        damaged = good;
        damaged.expectedConfigurationRevision = 0;
        requireInvalidConfig (damaged, "zero configuration revision rejects");
        damaged = good;
        damaged.expectedOneWireOwnerToken = 0;
        requireInvalidConfig (damaged, "zero one-wire owner rejects");
        damaged = good;
        damaged.expectedOneWireConfigurationRevision = 0;
        requireInvalidConfig (damaged, "zero one-wire configuration rejects");

        const adk::Ds18b20Resolution resolutions[4] = {
            adk::Ds18b20Resolution::Bits9,
            adk::Ds18b20Resolution::Bits10,
            adk::Ds18b20Resolution::Bits11,
            adk::Ds18b20Resolution::Bits12};
        const uint16_t deadlines[4] = {94, 188, 375, 750};
        for (uint8_t index = 0; index < 4; ++index)
        {
            damaged = good;
            damaged.probes[index].resolution = resolutions[index];
            damaged.probes[index].conversionDeadline =
                adk::Duration (deadlines[index]);
            adk::Qualified18B20ProbeSetPolicy exact (damaged);

            require (exact.initialize ().ok (),
                     "resolution-specific deadline boundary accepts");

            damaged.probes[index].conversionDeadline = adk::Duration ();

            requireInvalidConfig (damaged, "zero conversion deadline rejects");
            damaged.probes[index].conversionDeadline =
                adk::Duration (uint32_t (deadlines[index]) + 1U);
            requireInvalidConfig (damaged, "late conversion deadline rejects");
        }

        damaged = good;
        damaged.probes[0].resolution =
            static_cast<adk::Ds18b20Resolution> (0xffU);
        requireInvalidConfig (damaged, "invalid resolution enum rejects");
        damaged = good;
        damaged.probes[0].maximumAge = adk::Duration ();

        requireInvalidConfig (damaged, "zero maximum age rejects");

        damaged.probes[0].maximumAge = adk::Duration (0x80000000UL);

        requireInvalidConfig (damaged, "half-range maximum age rejects");

        damaged.probes[0].maximumAge = adk::Duration (0x80000001UL);

        requireInvalidConfig (damaged, "over-half-range maximum age rejects");
        damaged = good;
        damaged.probes[0].minimumRawSixteenths = 2;
        damaged.probes[0].maximumRawSixteenths = 1;
        requireInvalidConfig (damaged, "inverted temperature interval rejects");

        damaged = good;
        adk::Qualified18B20ProbeSetPolicy copied (damaged);
        damaged.expectedSourceId = 0;
        damaged.probes[0].rom.bytes[0] = 0;
        require (copied.initialize ().ok (),
                 "construction copies configuration before caller mutation");
        adk::QualifiedDs18b20Snapshot output = blankSnapshot ();

        require (copied.snapshot (output).ok (), "copied policy snapshots");
        require (sameRom (output.probes[0].rom, good.probes[0].rom),
                 "caller mutation cannot alter retained configured identity");
    }

    void testSearchChainAndCapacity ()
    {
        const adk::QualifiedDs18b20SetConfig good = config ();

        adk::Qualified18B20ProbeSetPolicy    policy (good);

        initialize (policy);

        adk::Ds18b20CycleBuilder builder;
        begin (policy, builder);

        const adk::OneWireRomCode order[4] = {good.probes[2].rom, good.probes[0].rom,
                                              good.probes[3].rom, good.probes[1].rom};
        ingestChain (policy, builder, order, 4, true);

        require (policy.finishSearch (builder, true, false, adk::StatusCode::Ok).ok (),
                 "complete four-ROM permutation finishes");

        adk::Ds18b20CycleBuilder discontinuous;
        begin (policy, discontinuous, 2);

        const adk::OneWireSearchState empty = {emptyRom (), 0, false};

        const adk::OneWireSearchState first =
            searchState (good.probes[0].rom, 1, false);
        require (policy
                     .ingestSearchPass (discontinuous,
                                        searchSnapshot (1, 1, empty, first), empty)
                     .ok (),
                 "first chained pass ingests");
        const adk::OneWireSearchState wrongRequest =
            searchState (good.probes[3].rom, 2, false);
        unsigned char beforeDiscontinuous[sizeof discontinuous];
        std::memcpy (beforeDiscontinuous, &discontinuous,
                     sizeof discontinuous);
        require (policy.ingestSearchPass (
                           discontinuous,
                           searchSnapshot (2, 2, wrongRequest,
                                           searchState (good.probes[1].rom, 0, true)),
                           wrongRequest)
                         .error () == adk::StatusCode::InvalidArgument,
                 "discontinuous search request rejects");
        require (sameBuilderBytes (discontinuous, beforeDiscontinuous),
                 "discontinuous rejection preserves builder byte image");

        adk::Ds18b20CycleBuilder afterTerminal;
        begin (policy, afterTerminal, 3);

        const adk::OneWireRomCode one[1] = {good.probes[0].rom};

        ingestChain (policy, afterTerminal, one, 1, true);

        const adk::OneWireSearchState terminal =
            searchState (good.probes[0].rom, 0, true);
        require (policy.ingestSearchPass (
                           afterTerminal,
                           searchSnapshot (2, 2, terminal,
                                           searchState (good.probes[1].rom, 0, true)),
                           terminal)
                         .error () == adk::StatusCode::InvalidArgument,
                 "pass after terminal result rejects");

        adk::Ds18b20CycleBuilder over;
        begin (policy, over, 4);

        ingestChain (policy, over, order, 4, false);

        require (policy.finishSearch (over, false, true, adk::StatusCode::Ok).ok (),
                 "four nonterminal results admit explicit over-capacity");

        adk::Ds18b20CycleBuilder contradictory;
        begin (policy, contradictory, 5);

        ingestChain (policy, contradictory, order, 4, true);

        require (policy.finishSearch (contradictory, true, true, adk::StatusCode::Ok)
                         .error () == adk::StatusCode::InvalidArgument,
                 "complete and over-capacity cannot both be true");

        adk::Ds18b20CycleBuilder emptyComplete;
        begin (policy, emptyComplete, 6);

        require (policy.finishSearch (emptyComplete, true, false, adk::StatusCode::Ok)
                         .error () == adk::StatusCode::InvalidArgument,
                 "zero-pass search cannot claim completeness");
    }

    void testMembershipAndPermutation ()
    {
        const adk::QualifiedDs18b20SetConfig good = config ();
        uint32_t cycleSequence = 1;
        for (uint8_t first = 0; first < 4; ++first)
        for (uint8_t second = 0; second < 4; ++second)
        for (uint8_t third = 0; third < 4; ++third)
        for (uint8_t fourth = 0; fourth < 4; ++fourth)
        {
            if (first == second || first == third || first == fourth ||
                second == third || second == fourth || third == fourth)
                continue;
            adk::Qualified18B20ProbeSetPolicy policy (good);

            initialize (policy);

            adk::Ds18b20CycleBuilder builder;

            begin (policy, builder, cycleSequence++);

            adk::OneWireRomCode order[4] = {emptyRom (), emptyRom (), emptyRom (),
                                            emptyRom ()};
            order[0] = good.probes[first].rom;
            order[1] = good.probes[second].rom;
            order[2] = good.probes[third].rom;
            order[3] = good.probes[fourth].rom;
            ingestChain (policy, builder, order, 4, true);

            require (
                policy.finishSearch (builder, true, false, adk::StatusCode::Ok).ok (),
                "search permutation finishes");
        }

        adk::Qualified18B20ProbeSetPolicy duplicatePolicy (good);

        initialize (duplicatePolicy);

        adk::Ds18b20CycleBuilder duplicate;

        begin (duplicatePolicy, duplicate);

        const adk::OneWireRomCode repeated[2] = {good.probes[0].rom,
                                                 good.probes[0].rom};
        ingestChain (duplicatePolicy, duplicate, repeated, 2, true);

        require (
            duplicatePolicy.finishSearch (duplicate, true, false, adk::StatusCode::Ok)
                .ok (),
            "duplicate derived identity is retained as domain evidence");

        adk::Qualified18B20ProbeSetPolicy unknownPolicy (good);

        initialize (unknownPolicy);

        adk::Ds18b20CycleBuilder unknown;

        begin (unknownPolicy, unknown);

        const adk::OneWireRomCode stranger[1] = {rom (0x70)};

        ingestChain (unknownPolicy, unknown, stranger, 1, true);

        require (unknownPolicy.finishSearch (unknown, true, false, adk::StatusCode::Ok)
                     .ok (),
                 "unknown CRC-valid identity is retained as domain evidence");

        adk::Qualified18B20ProbeSetPolicy corruptPolicy (good);

        initialize (corruptPolicy);

        adk::Ds18b20CycleBuilder corrupt;

        begin (corruptPolicy, corrupt);

        adk::OneWireRomCode broken[1] = {good.probes[0].rom};
        broken[0].bytes[4] ^= 1U;
        ingestChain (corruptPolicy, corrupt, broken, 1, true);

        require (corruptPolicy.finishSearch (corrupt, true, false, adk::StatusCode::Ok)
                     .ok (),
                 "invalid discovered CRC is retained as set transport evidence");
    }

    adk::QualifiedDs18b20Snapshot finalizedSearch (
        const adk::QualifiedDs18b20SetConfig& good,
        const adk::OneWireRomCode* found, uint8_t count, bool complete,
        bool overCapacity, adk::Status producerStatus,
        adk::StatusCode finalStatus)
    {
        adk::Qualified18B20ProbeSetPolicy policy (good);

        initialize (policy);
        adk::Ds18b20CycleBuilder builder;

        begin (policy, builder);

        ingestChain (policy, builder, found, count, complete);

        require (policy.finishSearch (builder, complete, overCapacity,
                                      producerStatus).ok (),
                 "search outcome stages");
        return finalize (policy, builder, finalStatus);
    }

    void testFinalizedSearchOutcomes ()
    {
        const adk::QualifiedDs18b20SetConfig good = config ();
        const adk::OneWireRomCode subset[1] = {good.probes[2].rom};
        const adk::QualifiedDs18b20Snapshot missing =
            finalizedSearch (good, subset, 1, true, false,
                             adk::StatusCode::Ok,
                             adk::StatusCode::HardwareFailure);
        require (missing.quality == adk::Ds18b20SetQuality::Missing &&
                     missing.presentMask == 0x04U &&
                     missing.probes[0].quality ==
                         adk::Ds18b20ProbeQuality::Missing &&
                     missing.probes[2].quality ==
                         adk::Ds18b20ProbeQuality::TransportFault,
                 "complete subset proves missing while retaining present fault");

        const adk::OneWireRomCode repeated[2] = {
            good.probes[1].rom, good.probes[1].rom};
        const adk::QualifiedDs18b20Snapshot duplicate =
            finalizedSearch (good, repeated, 2, true, false,
                             adk::StatusCode::Ok, adk::StatusCode::Ok);
        require (duplicate.quality ==
                     adk::Ds18b20SetQuality::DuplicateIdentity &&
                     duplicate.probes[1].quality ==
                         adk::Ds18b20ProbeQuality::DuplicateIdentity,
                 "duplicate identity wins over simultaneous missing roles");

        const adk::OneWireRomCode withUnknown[2] = {
            good.probes[0].rom, rom (0x70)};
        const adk::QualifiedDs18b20Snapshot unknown =
            finalizedSearch (good, withUnknown, 2, true, false,
                             adk::StatusCode::Ok,
                             adk::StatusCode::HardwareFailure);
        require (unknown.quality == adk::Ds18b20SetQuality::UnknownIdentity &&
                     unknown.presentMask == 0x01U,
                 "unknown identity wins over simultaneous missing roles");

        adk::OneWireRomCode damaged = good.probes[0].rom;
        damaged.bytes[3] ^= 1U;
        const adk::OneWireRomCode corrupt[1] = {damaged};
        const adk::QualifiedDs18b20Snapshot invalid =
            finalizedSearch (good, corrupt, 1, true, false,
                             adk::StatusCode::Ok,
                             adk::StatusCode::HardwareFailure);
        require (invalid.quality == adk::Ds18b20SetQuality::TransportFault &&
                     invalid.presentMask == 0,
                 "invalid discovered ROM becomes set transport fault");

        const adk::OneWireRomCode full[4] = {
            good.probes[0].rom, good.probes[1].rom,
            good.probes[2].rom, good.probes[3].rom};
        const adk::QualifiedDs18b20Snapshot capacity =
            finalizedSearch (good, full, 4, false, true,
                             adk::StatusCode::CapacityExceeded,
                             adk::StatusCode::CapacityExceeded);
        require (capacity.quality == adk::Ds18b20SetQuality::TransportFault &&
                     capacity.presentMask == 0,
                 "over-capacity cannot prove membership or absence");

        const adk::OneWireRomCode truncatedSet[2] = {
            good.probes[0].rom, good.probes[1].rom};
        const adk::QualifiedDs18b20Snapshot truncated =
            finalizedSearch (good, truncatedSet, 2, false, false,
                             adk::StatusCode::HardwareFailure,
                             adk::StatusCode::HardwareFailure);
        require (truncated.quality ==
                     adk::Ds18b20SetQuality::TransportFault &&
                     truncated.presentMask == 0,
                 "truncated producer failure cannot prove membership");

        for (uint8_t byte = 0; byte < 8; ++byte)
        {
            adk::OneWireRomCode damagedRom = good.probes[0].rom;
            damagedRom.bytes[byte] ^= 1U;
            const adk::OneWireRomCode damagedSet[1] = {damagedRom};
            const adk::QualifiedDs18b20Snapshot damaged =
                finalizedSearch (good, damagedSet, 1, true, false,
                                 adk::StatusCode::Ok,
                                 adk::StatusCode::HardwareFailure);
            require (damaged.quality ==
                         adk::Ds18b20SetQuality::TransportFault,
                     "every discovered ROM-byte corruption is transport fault");
        }
    }

    void testInitialSearchAtomicity ()
    {
        const adk::QualifiedDs18b20SetConfig good = config ();

        adk::Qualified18B20ProbeSetPolicy policy (good);

        initialize (policy);
        const adk::OneWireSearchState result =
            searchState (good.probes[0].rom, 0, true);
        adk::OneWireSearchState malformed[3] = {
            {good.probes[0].rom, 0, false},
            {emptyRom (), 1, false},
            {emptyRom (), 0, true}};
        for (uint8_t index = 0; index < 3; ++index)
        {
            adk::Ds18b20CycleBuilder builder;
            begin (policy, builder, uint32_t (index + 1U));
            unsigned char before[sizeof builder];
            std::memcpy (before, &builder, sizeof builder);
            const adk::OneWireTransactionSnapshot transaction =
                searchSnapshot (1, 1, malformed[index], result);
            require (policy.ingestSearchPass (builder, transaction,
                                              malformed[index])
                         .error () == adk::StatusCode::InvalidArgument,
                     "malformed initial search request rejects");
            require (sameBuilderBytes (builder, before),
                     "malformed initial rejection preserves builder bytes");
        }

        adk::Ds18b20CycleBuilder unterminated;
        begin (policy, unterminated, 4);
        const adk::OneWireRomCode one[1] = {good.probes[0].rom};
        ingestChain (policy, unterminated, one, 1, false);
        unsigned char beforeFinish[sizeof unterminated];
        std::memcpy (beforeFinish, &unterminated, sizeof unterminated);

        require (policy.finishSearch (unterminated, true, false,
                                      adk::StatusCode::Ok)
                     .error () == adk::StatusCode::InvalidArgument,
                 "complete search requires terminal result");
        require (sameBuilderBytes (unterminated, beforeFinish),
                 "failed terminal finish preserves builder bytes");
    }

    void testFinishSealingAndBuilderEpoch ()
    {
        const adk::QualifiedDs18b20SetConfig good = config ();

        adk::Qualified18B20ProbeSetPolicy policy (good);

        initialize (policy);

        adk::Ds18b20CycleBuilder statusBuilder;
        begin (policy, statusBuilder);
        unsigned char beforeStatus[sizeof statusBuilder];
        std::memcpy (beforeStatus, &statusBuilder, sizeof statusBuilder);

        const adk::Status invalidStatus (
            static_cast<adk::StatusCode> (0xffU));
        require (policy.finishSearch (statusBuilder, false, false,
                                      invalidStatus)
                     .error () == adk::StatusCode::InvalidArgument,
                 "out-of-domain producer status rejects");
        require (sameBuilderBytes (statusBuilder, beforeStatus),
                 "invalid producer status preserves builder bytes");

        require (policy.finishSearch (statusBuilder, false, false,
                                      adk::StatusCode::HardwareFailure)
                     .ok (),
                 "typed producer fault seals search");
        unsigned char sealed[sizeof statusBuilder];
        std::memcpy (sealed, &statusBuilder, sizeof statusBuilder);

        require (policy.finishSearch (statusBuilder, false, false,
                                      adk::StatusCode::Timeout)
                     .error () == adk::StatusCode::InvalidArgument,
                 "sealed search rejects repeated finish");
        require (sameBuilderBytes (statusBuilder, sealed),
                 "repeated finish preserves sealed image");

        adk::Ds18b20CycleBuilder stale;

        begin (policy, stale, 2);

        const adk::OneWireSearchState empty = {emptyRom (), 0, false};
        const adk::OneWireSearchState result =
            searchState (good.probes[0].rom, 0, true);
        const adk::OneWireTransactionSnapshot transaction =
            searchSnapshot (1, 1, empty, result);
        policy.reset ();
        unsigned char beforeStale[sizeof stale];
        std::memcpy (beforeStale, &stale, sizeof stale);

        require (policy.ingestSearchPass (stale, transaction, empty).error () ==
                     adk::StatusCode::InvalidArgument,
                 "pre-reset builder rejects in new policy epoch");
        require (sameBuilderBytes (stale, beforeStale),
                 "stale builder rejection preserves bytes");
        require (policy.finishSearch (stale, false, false,
                                      adk::StatusCode::HardwareFailure)
                     .error () == adk::StatusCode::InvalidArgument,
                 "stale builder cannot finish after reset");

        adk::Ds18b20CycleBuilder restart;

        begin (policy, restart, 3);

        ingestChain (policy, restart, &good.probes[0].rom, 1, true);
        unsigned char beforeFailedBegin[sizeof restart];

        std::memcpy (beforeFailedBegin, &restart, sizeof restart);

        require (policy.beginCycle (adk::TimePoint (10000), 0, 19, 4,
                                    adk::TimePoint (9500), restart)
                     .error () == adk::StatusCode::InvalidArgument,
                 "failed restart rejects invalid source");
        require (sameBuilderBytes (restart, beforeFailedBegin),
                 "failed restart preserves prior staged builder");
        require (policy.beginCycle (adk::TimePoint (10000), 7, 19, 4,
                                    adk::TimePoint (9500), restart)
                     .ok (),
                 "valid restart replaces staged builder");
        adk::Ds18b20CycleBuilder expectedRestart;
        require (policy.beginCycle (adk::TimePoint (10000), 7, 19, 4,
                                    adk::TimePoint (9500), expectedRestart)
                     .ok (),
                 "comparison restart begins");
        require (sameBuilderImage (restart, expectedRestart),
                 "successful restart clears all prior staged evidence");
    }

    void testAtomicBuilderAndGlobalTransactionIdentity ()
    {
        const adk::QualifiedDs18b20SetConfig good = config ();

        adk::Qualified18B20ProbeSetPolicy policy (good);

        initialize (policy);

        adk::Ds18b20CycleBuilder neverBegun;
        adk::Ds18b20CycleBuilder beforeNeverBegun;
        require (sameBuilderImage (neverBegun, beforeNeverBegun),
                 "fresh builders have canonical byte image");
        const adk::OneWireSearchState empty = {emptyRom (), 0, false};
        const adk::OneWireSearchState result =
            searchState (good.probes[0].rom, 0, true);
        require (policy.ingestSearchPass (
                           neverBegun, searchSnapshot (1, 1, empty, result),
                           empty)
                     .error () == adk::StatusCode::InvalidArgument,
                 "never-begun builder rejects search ingestion");
        require (sameBuilderImage (neverBegun, beforeNeverBegun),
                 "failed never-begun ingestion preserves every byte");
        require (policy.finishSearch (neverBegun, true, false,
                                      adk::StatusCode::Ok)
                     .error () == adk::StatusCode::InvalidArgument,
                 "never-begun builder rejects search completion");
        require (sameBuilderImage (neverBegun, beforeNeverBegun),
                 "failed never-begun completion preserves every byte");

        const adk::OneWireRomCode found[1] = {good.probes[0].rom};
        adk::Ds18b20CycleBuilder first;

        begin (policy, first, 1);

        ingestChain (policy, first, found, 1, true);

        require (policy.finishSearch (first, true, false,
                                      adk::StatusCode::Ok).ok (),
                 "first transaction-identity cycle finishes");
        finalize (policy, first, adk::StatusCode::HardwareFailure);

        adk::Ds18b20CycleBuilder reused;

        begin (policy, reused, 2, 10000);

        ingestChain (policy, reused, found, 1, true);

        require (policy.finishSearch (reused, true, false,
                                      adk::StatusCode::Ok).ok (),
                 "reused transaction-identity cycle stages");
        adk::QualifiedDs18b20Snapshot canary = blankSnapshot ();
        const adk::QualifiedDs18b20Snapshot beforeCanary = canary;
        require (policy.finalizeCycle (adk::TimePoint (11000), reused, canary)
                     .error () == adk::StatusCode::InvalidArgument,
                 "global transaction and request-sequence reuse rejects");
        require (std::memcmp (&canary, &beforeCanary, sizeof canary) == 0,
                 "reused transaction rejection preserves caller output");
    }
} // namespace

int main ()
{
    testLifecycleAndConfiguration ();

    testConfigurationMatrixAndCopyIsolation ();

    testSearchChainAndCapacity ();

    testMembershipAndPermutation ();

    testFinalizedSearchOutcomes ();

    testInitialSearchAtomicity ();

    testFinishSealingAndBuilderEpoch ();

    testAtomicBuilderAndGlobalTransactionIdentity ();
    std::cout << "qualified 18B20 identity tests passed\n";
    return EXIT_SUCCESS;
}
