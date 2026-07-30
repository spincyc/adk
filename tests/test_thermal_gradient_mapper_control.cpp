#include <thermal_gradient_mapper.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>

namespace {
    constexpr uint32_t mapperOwner     = 0x660066UL;
    constexpr uint32_t controlOwner    = 0xC0116600UL;
    constexpr uint8_t  setSource       = 7;
    constexpr uint16_t setRevision     = 31;
    constexpr uint8_t  controlSource   = 9;
    constexpr uint16_t controlRevision = 41;

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
                    crc ^= 0x8CU;
                }
                value >>= 1;
            }
        }
        return crc;
    }

    adk::OneWireRomCode rom (uint8_t serial)
    {
        adk::OneWireRomCode value = {{0x28, serial, 0x11, 0x22, 0x33, 0x44, 0x55, 0}};
        value.bytes[7]            = crc8 (value.bytes, 7);
        return value;
    }

    adk::ThermalMapperConfig config (uint8_t count = 4)
    {
        const adk::OneWireRomCode one   = rom (1);
        const adk::OneWireRomCode two   = rom (2);
        const adk::OneWireRomCode three = rom (3);
        const adk::OneWireRomCode four  = rom (4);
        adk::ThermalMapperConfig value = {
            mapperOwner,
            17,
            setSource,
            setRevision,
            {one, two, three, four},
            {one, two, three, four},
            count,
            controlOwner,
            controlSource,
            controlRevision,
            adk::Duration (20),
            16};
        const adk::OneWireRomCode zero = {{0, 0, 0, 0, 0, 0, 0, 0}};
        for (uint8_t index = count; index < 4; ++index)
        {
            value.spatialOrder[index] = zero;
        }
        return value;
    }

    adk::QualifiedDs18b20Snapshot frame (uint32_t sequence, uint32_t observedAt,
                                         uint32_t freshThrough = 200)
    {
        adk::QualifiedDs18b20Snapshot value;
        value.sourceId                      = setSource;
        value.configurationRevision         = setRevision;
        value.cycleSequence                 = sequence;
        value.observedAt                    = adk::TimePoint (observedAt);
        value.validCount                    = 4;
        value.presentMask                   = 0x0F;
        value.faultMask                     = 0;
        value.quality                       = adk::Ds18b20SetQuality::Complete;
        value.status                        = adk::StatusCode::Ok;
        for (uint8_t index = 0; index < 4; ++index)
        {
            adk::QualifiedDs18b20Probe& probe = value.probes[index];
            probe.rom                         = rom (static_cast<uint8_t> (index + 1));
            probe.cycleSequence               = sequence;
            probe.conversionGeneration        = 100U + index;
            probe.readTransactionGeneration   = 200U + index;
            probe.observedAt                  = adk::TimePoint (observedAt);
            probe.freshThrough                = adk::TimePoint (freshThrough);
            probe.rawSixteenths               = static_cast<int16_t> (160 + index * 32);
            probe.lowerRawSixteenths          = probe.rawSixteenths;
            probe.upperRawSixteenths          = probe.rawSixteenths;
            probe.resolution                  = adk::Ds18b20Resolution::Bits12;
            probe.quality                     = adk::Ds18b20ProbeQuality::Current;
            probe.age                         = adk::Duration (0);
            probe.status                      = adk::StatusCode::Ok;
        }
        return value;
    }

    adk::ThermalMapperControl noControl ()
    {
        return {0, 0, 0, 0, adk::TimePoint (0), false, false, adk::StatusCode::Ok};
    }

    adk::ThermalMapperControl control (uint32_t sequence, uint32_t observedAt,
                                       bool next, bool record)
    {
        return {controlOwner,
                controlSource,
                controlRevision,
                sequence,
                adk::TimePoint (observedAt),
                next,
                record,
                adk::StatusCode::Ok};
    }

    adk::ThermalMapperEnvelope envelope (uint32_t                             now,
                                         const adk::QualifiedDs18b20Snapshot& probes,
                                         const adk::ThermalMapperControl&     input)
    {
        return {adk::TimePoint (now), probes, input};
    }

    void pageOrderAndWrap ()
    {
        adk::ThermalGradientMapper mapper (config ());

        require (mapper.initialize (adk::TimePoint (100)).ok (),
                 "page mapper initializes");
        const adk::QualifiedDs18b20Snapshot probes = frame (1, 100);
        adk::ThermalMapperResult            result;
        require (mapper
                     .update (envelope (100, probes,
                                       control (1, 100, false, false)),
                              result)
                     .ok (),
                 "initial frame is accepted");
        require (result.intent.pageIndex == 0 &&
                     result.intent.pageKind == adk::ThermalMapperPageKind::Overall &&
                     result.intent.selectedSlot == 0 &&
                     result.intent.selectedGradient == 0,
                 "page zero is overall");

        for (uint8_t page = 1; page < 8; ++page)
        {
            const adk::Status advanced = mapper.update (
                envelope (100 + page, probes,
                          control (page + 1U, 100 + page, true, false)),
                result);
            if (!advanced.ok ())
            {
                std::cerr << "page advance failed at page "
                          << static_cast<unsigned> (page) << " status "
                          << static_cast<unsigned> (advanced.error ()) << '\n';
            }
            require (advanced.ok (), "fresh next edge advances");
            require (result.intent.pageIndex == page, "page index advances once");
            if (page <= 4)
            {
                require (result.intent.pageKind == adk::ThermalMapperPageKind::Probe &&
                             result.intent.selectedSlot == page - 1 &&
                             result.intent.selectedGradient == 0,
                         "probe pages retain configured order");
            }
            else
            {
                require (result.intent.pageKind ==
                                 adk::ThermalMapperPageKind::AdjacentGradient &&
                             result.intent.selectedSlot == 0 &&
                             result.intent.selectedGradient == page - 5,
                         "gradient pages retain configured adjacency order");
            }
        }
        require (mapper.update (envelope (108, probes, control (9, 108, true, false)),
                                result)
                         .ok () &&
                     result.intent.pageIndex == 0 &&
                     result.intent.pageKind == adk::ThermalMapperPageKind::Overall &&
                     result.intent.selectedSlot == 0 &&
                     result.intent.selectedGradient == 0,
                 "last page wraps to overall");
    }

    void edgeAndRecordPrecedence ()
    {
        adk::ThermalGradientMapper mapper (config (2));

        require (mapper.initialize (adk::TimePoint (10)).ok (),
                 "edge mapper initializes");
        const adk::QualifiedDs18b20Snapshot probes = frame (1, 10, 100);
        adk::ThermalMapperResult            result;
        require (
            mapper.update (envelope (10, probes, control (1, 10, true, true)), result)
                .ok (),
            "simultaneous edges are accepted");
        require (result.intent.pageIndex == 1 && result.hasRecord,
                 "next edge precedes record edge");
        require (result.record.recordEdgeOwnerToken == controlOwner &&
                     result.record.recordEdgeSourceId == controlSource &&
                     result.record.recordEdgeConfigurationRevision == controlRevision &&
                     result.record.recordEdgeSequence == 1 &&
                     result.record.recordEdgeObservedAt == adk::TimePoint (10),
                 "record retains complete control attribution");

        require (
            mapper.update (envelope (11, probes, control (1, 10, true, true)), result)
                    .ok () &&
                result.intent.pageIndex == 1 && !result.hasRecord,
            "identical held edge neither pages nor records");

        const adk::ThermalMapperControl held = control (2, 12, false, false);

        require (mapper.update (envelope (12, probes, held), result).ok () &&
                     result.intent.pageIndex == 1 && !result.hasRecord,
                 "fresh held levels without edges do nothing");

        require (
            mapper.update (envelope (13, probes, control (3, 13, false, true)), result)
                    .ok () &&
                result.intent.pageIndex == 1 && result.hasRecord,
            "fresh record edge records an unchanged frame");

        adk::ThermalMapperResult canary = result;
        const adk::ThermalMapperResult before = canary;
        require (
            mapper.update (envelope (14, probes, control (2, 12, true, false)), canary)
                        .error () == adk::StatusCode::InvalidArgument &&
                std::memcmp (&canary, &before, sizeof canary) == 0,
            "an edge older than a consumed no-edge control cannot act");

        adk::QualifiedDs18b20Snapshot changed = probes;
        changed.cycleSequence                 = 2;
        changed.observedAt                    = adk::TimePoint (15);
        for (uint8_t index = 0; index < 4; ++index)
        {
            changed.probes[index].cycleSequence = 2;
            changed.probes[index].observedAt    = adk::TimePoint (15);
            changed.probes[index].freshThrough  = adk::TimePoint (100);
        }
        require (mapper
                     .update (envelope (15, changed,
                                       control (4, 15, false, false)),
                              result)
                     .ok () &&
                     !result.hasRecord,
                 "a fresh frame without a record edge emits no record");
    }

    void controlValidationAndAtomicity ()
    {
        adk::ThermalGradientMapper mapper (config ());

        require (mapper.initialize (adk::TimePoint (100)).ok (),
                 "validation mapper initializes");
        const adk::QualifiedDs18b20Snapshot probes   = frame (1, 100, 1000);
        adk::ThermalMapperResult            accepted;
        require (mapper
                     .update (envelope (100, probes, control (10, 100, true, false)),
                              accepted)
                     .ok (),
                 "validation baseline commits");
        adk::ThermalGradientIntent retained;
        require (mapper.snapshot (retained).ok (), "baseline snapshot is readable");

        adk::ThermalMapperControl invalid[] = {
            control (11, 101, true, false),         control (11, 101, true, false),
            control (11, 101, true, false),         control (10, 101, true, false),
            control (9, 101, true, false),          control (11, 121, true, false),
            control (11, 0x80000065UL, true, false),
            control (0, 101, true, false)};
        invalid[0].ownerToken ^= 1U;
        invalid[1].sourceId ^= 1U;
        invalid[2].configurationRevision ^= 1U;

        for (uint8_t index = 0; index < sizeof invalid / sizeof invalid[0]; ++index)
        {
            adk::ThermalMapperResult canary = accepted;
            const adk::ThermalMapperResult before = canary;
            const uint32_t now = index == 5 ? 142U : (index == 6 ? 102U : 102U);
            require (mapper.update (envelope (now, probes, invalid[index]), canary)
                             .error () == adk::StatusCode::InvalidArgument,
                     "invalid control is structurally rejected");
            require (std::memcmp (&canary, &before, sizeof canary) == 0,
                     "structural rejection preserves caller result");
            adk::ThermalGradientIntent after;
            require (mapper.snapshot (after).ok () &&
                         after.pageIndex == retained.pageIndex &&
                         after.pageKind == retained.pageKind &&
                         after.selectedSlot == retained.selectedSlot &&
                         after.selectedGradient == retained.selectedGradient &&
                         after.health == retained.health &&
                         after.overallFaultMask == retained.overallFaultMask,
                     "structural rejection preserves mapper state");
        }

        adk::ThermalMapperControl failed = control (11, 102, true, false);
        failed.status                    = adk::StatusCode::HardwareFailure;
        adk::ThermalMapperResult canary = accepted;
        const adk::ThermalMapperResult before = canary;
        require (mapper.update (envelope (102, probes, failed), canary).error () ==
                         adk::StatusCode::InvalidArgument &&
                     std::memcmp (&canary, &before, sizeof canary) == 0,
                 "failed control status rejects atomically before semantics");

        require (mapper
                     .update (envelope (120, probes, control (11, 100, true, false)),
                              accepted)
                     .ok (),
                 "control age is accepted at the inclusive maximum");
        require (mapper.update (envelope (121, probes, control (12, 100, true, false)),
                                accepted)
                         .error () == adk::StatusCode::InvalidArgument,
                 "control one tick beyond maximum age rejects");
    }

    void sequenceAndTimeWrap ()
    {
        adk::ThermalGradientMapper mapper (config ());

        require (mapper.initialize (adk::TimePoint (0xFFFFFFF0UL)).ok (),
                 "wrap mapper initializes");
        adk::QualifiedDs18b20Snapshot probes =
            frame (0xFFFFFFFEUL, 0xFFFFFFF0UL, 40);
        for (uint8_t index = 0; index < 4; ++index)
        {
            probes.probes[index].age = adk::Duration (5);
        }
        adk::ThermalMapperResult result;
        require (
            mapper
                .update (envelope (0xFFFFFFF5UL, probes,
                                   control (0xFFFFFFFEUL, 0xFFFFFFF5UL, true, false)),
                         result)
                .ok (),
            "pre-wrap control commits");
        require (
            mapper
                .update (envelope (5, probes, control (0xFFFFFFFFUL, 5, true, false)),
                         result)
                .ok (),
            "time wraps with forward sequence");
        require (
            mapper.update (envelope (6, probes, control (1, 6, true, false)), result)
                .ok (),
            "control sequence wraps without using zero");

        adk::ThermalMapperResult canary = result;
        const adk::ThermalMapperResult before = canary;
        require (
            mapper.update (envelope (7, probes, control (0x80000001UL, 7, true, false)),
                           canary)
                        .error () == adk::StatusCode::InvalidArgument &&
                std::memcmp (&canary, &before, sizeof canary) == 0,
            "exact sequence half-range rejects atomically");
    }

    void frameAbsenceAndFreshness ()
    {
        adk::ThermalGradientMapper mapper (config ());

        require (mapper.initialize (adk::TimePoint (50)).ok (),
                 "freshness mapper initializes");
        adk::QualifiedDs18b20Snapshot absent = frame (0, 50, 100);
        adk::ThermalMapperResult result;
        require (
            mapper.update (envelope (50, absent, control (1, 50, true, true)), result)
                    .error () == adk::StatusCode::InvalidArgument,
            "controls cannot act before a frame is accepted");
        adk::ThermalGradientIntent inert;
        require (mapper.snapshot (inert).ok () && inert.pageIndex == 0 &&
                     inert.health == adk::ThermalGradientHealth::Qualifying,
                 "no-frame control preserves qualifying page zero");

        adk::QualifiedDs18b20Snapshot probes = frame (1, 60, 100);

        probes.probes[0].freshThrough        = adk::TimePoint (80);
        probes.probes[1].freshThrough        = adk::TimePoint (90);
        probes.probes[2].freshThrough        = adk::TimePoint (100);
        probes.probes[3].freshThrough        = adk::TimePoint (110);

        require (mapper
                     .update (envelope (60, probes,
                                       control (1, 60, false, false)),
                              result)
                     .ok (),
                 "heterogeneous freshness frame commits");
        require (
            mapper.update (envelope (79, probes, control (2, 79, true, false)), result)
                    .ok () &&
                result.intent.overallFaultMask == 0,
            "unchanged frame remains fresh below first bound");
        require (
            mapper.update (envelope (80, probes, control (3, 80, false, true)), result)
                    .ok () &&
                result.hasRecord && result.intent.overallFaultMask == 0,
            "unchanged frame remains fresh at inclusive bound");
        require (
            mapper.update (envelope (81, probes, control (4, 81, false, true)), result)
                    .ok () &&
                result.hasRecord && (result.intent.overallFaultMask & 0x01U) != 0,
            "one tick past bound commits stale fault and can record it");
        require ((result.intent.overallFaultMask & 0x0EU) == 0,
                 "heterogeneous later bounds remain independently fresh");

        adk::ThermalGradientMapper wrapped (config ());

        require (wrapped.initialize (adk::TimePoint (0xFFFFFFF0UL)).ok (),
                 "freshness-wrap mapper initializes");
        adk::QualifiedDs18b20Snapshot wrapFrame = frame (0xFFFFFFFEUL, 0xFFFFFFF0UL, 5);
        for (uint8_t index = 0; index < 4; ++index)
        {
            wrapFrame.probes[index].age = adk::Duration (21);
        }

        require (wrapped
                     .update (envelope (5, wrapFrame,
                                       control (1, 5, false, false)),
                              result)
                     .ok () &&
                     result.intent.overallFaultMask == 0,
                 "freshThrough equality survives wrap");
        require (wrapped.update (envelope (6, wrapFrame, control (2, 6, true, false)),
                                 result)
                         .ok () &&
                     result.intent.overallFaultMask == 0x0F,
                 "one tick after wrapped freshness faults every slot");
    }

    void resetAndShutdown ()
    {
        adk::ThermalGradientMapper mapper (config ());

        require (mapper.initialize (adk::TimePoint (10)).ok (),
                 "lifecycle mapper initializes");
        adk::ThermalMapperResult            result;
        const adk::QualifiedDs18b20Snapshot probes = frame (1, 10, 100);

        require (
            mapper.update (envelope (10, probes, control (1, 10, true, true)), result)
                    .ok () &&
                result.hasRecord,
            "lifecycle baseline records");
        const uint32_t priorLifecycle = result.record.lifecycleGeneration;
        require (mapper.reset (adk::TimePoint (20)).ok (), "reset succeeds");
        adk::ThermalGradientIntent resetIntent;
        require (mapper.snapshot (resetIntent).ok () && resetIntent.pageIndex == 0 &&
                     resetIntent.health == adk::ThermalGradientHealth::Qualifying &&
                     !resetIntent.outputsInactive,
                 "reset clears frame, page, and presentation");
        require (
            mapper.update (envelope (20, probes, control (1, 20, true, true)), result)
                    .ok () &&
                result.hasRecord && result.record.recordSequence == 1 &&
                result.record.lifecycleGeneration == priorLifecycle + 1U,
            "reset admits copied evidence under a new lifecycle and record sequence");

        require (mapper.shutdown ().ok () && mapper.shutdown ().ok () &&
                     !mapper.initialized (),
                 "shutdown is idempotent");
        require (mapper.snapshot (resetIntent).error () ==
                     adk::StatusCode::NotInitialized,
                 "shutdown makes snapshot unavailable");
        require (mapper.update (envelope (21, probes, noControl ()), result).error () ==
                     adk::StatusCode::NotInitialized,
                 "shutdown rejects updates");
    }
} // namespace

int main ()
{
    pageOrderAndWrap ();

    edgeAndRecordPrecedence ();

    controlValidationAndAtomicity ();

    sequenceAndTimeWrap ();

    frameAbsenceAndFreshness ();

    resetAndShutdown ();
    std::cout << "thermal gradient mapper control tests passed\n";
    return EXIT_SUCCESS;
}
