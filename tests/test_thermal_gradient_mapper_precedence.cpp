#include <thermal_gradient_mapper.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>

#if defined(ADK_TESTING)
namespace adk {
    struct ThermalGradientMapperTestAccess
    {
        static void exhaustRecordSequence (ThermalGradientMapper& mapper)
        {
            mapper.recordSequenceExhausted_ = true;
        }
    };
} // namespace adk
#endif

namespace {
    constexpr uint32_t mapperOwner     = UINT32_C (0x6600aa55);
    constexpr uint16_t mapperRevision  = 66;
    constexpr uint8_t  setSource       = 7;
    constexpr uint16_t setRevision     = 19;
    constexpr uint32_t controlOwner    = UINT32_C (0xc066c066);
    constexpr uint8_t  controlSource   = 11;
    constexpr uint16_t controlRevision = 23;

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
        adk::OneWireRomCode value = {
            {0x28, serial, 0x10, 0x20, 0x30, 0x40, 0x50, 0}};
        value.bytes[7] = crc8 (value.bytes, 7);
        return value;
    }

    adk::OneWireRomCode emptyRom ()
    {
        return {{0, 0, 0, 0, 0, 0, 0, 0}};
    }

    adk::ThermalMapperConfig config ()
    {
        adk::ThermalMapperConfig value = {
            mapperOwner,
            mapperRevision,
            setSource,
            setRevision,
            {emptyRom (), emptyRom (), emptyRom (), emptyRom ()},
            {emptyRom (), emptyRom (), emptyRom (), emptyRom ()},
            3,
            controlOwner,
            controlSource,
            controlRevision,
            adk::Duration (50),
            16};
        for (uint8_t index = 0; index < 4; ++index)
        {
            value.sourceRoms[index] = rom (static_cast<uint8_t> (index + 1U));
        }
        value.spatialOrder[0] = rom (3);
        value.spatialOrder[1] = rom (1);
        value.spatialOrder[2] = rom (4);
        return value;
    }

    adk::QualifiedDs18b20Snapshot frame (uint32_t sequence, uint32_t observedAt,
                                         uint32_t freshThrough)
    {
        adk::QualifiedDs18b20Snapshot value;
        value.sourceId              = setSource;
        value.configurationRevision = setRevision;
        value.cycleSequence         = sequence;
        value.observedAt            = adk::TimePoint (observedAt);
        value.validCount            = 4;
        value.presentMask           = 0x0f;
        value.faultMask             = 0;
        value.quality               = adk::Ds18b20SetQuality::Complete;
        value.status                = adk::StatusCode::Ok;
        for (uint8_t index = 0; index < 4; ++index)
        {
            const int16_t raw = static_cast<int16_t> (160 + 32 * index);
            value.probes[index] = {
                rom (static_cast<uint8_t> (index + 1U)),
                sequence,
                uint32_t       (100U + index),
                uint32_t       (200U + index),
                adk::TimePoint (observedAt),
                adk::TimePoint (freshThrough),
                raw,
                raw,
                raw,
                adk::Ds18b20Resolution::Bits12,
                adk::Ds18b20ProbeQuality::Current,
                adk::Duration (0),
                adk::StatusCode::Ok};
        }
        return value;
    }

    adk::ThermalMapperControl control (uint32_t sequence, uint32_t observedAt,
                                       bool nextEdge, bool recordEdge)
    {
        return {controlOwner,
                controlSource,
                controlRevision,
                sequence,
                adk::TimePoint (observedAt),
                nextEdge,
                recordEdge,
                adk::StatusCode::Ok};
    }

    adk::ThermalMapperEnvelope envelope (
        uint32_t now, const adk::QualifiedDs18b20Snapshot& probes,
        const adk::ThermalMapperControl& input)
    {
        return {adk::TimePoint (now), probes, input};
    }

    template <typename Value> void fillBytes (Value& value, uint8_t byte)
    {
        unsigned char* bytes = reinterpret_cast<unsigned char*> (&value);
        for (size_t index = 0; index < sizeof value; ++index)
        {
            bytes[index] = byte;
        }
    }

    void requireRejectedAtomically (
        adk::ThermalGradientMapper& mapper, const adk::ThermalMapperEnvelope& input,
        adk::StatusCode expected, const adk::ThermalMapperResult& retainedResult,
        const adk::ThermalGradientIntent& retainedIntent, const char* message)
    {
        adk::ThermalMapperResult guarded = retainedResult;
        require (mapper.update (input, guarded).error () == expected, message);
        require (std::memcmp (&guarded, &retainedResult, sizeof guarded) == 0,
                 "rejection preserves every caller-result byte");

        adk::ThermalGradientIntent after;
        fillBytes (after, 0xa5);

        require (mapper.snapshot (after).ok (),
                 "retained mapper state remains observable");
        require (std::memcmp (&after, &retainedIntent, sizeof after) == 0,
                 "rejection preserves complete retained mapper behavior");
    }

    void testLifecyclePrecedesMalformedEnvelope ()
    {
        adk::ThermalGradientMapper mapper (config ());

        adk::QualifiedDs18b20Snapshot malformed = frame (0, 200, 300);
        malformed.sourceId                      = 0;
        adk::ThermalMapperControl       badControl =
            control (0, 300, true, true);
        badControl.ownerToken                    = 0;
        const adk::ThermalMapperEnvelope input =
            envelope (100, malformed, badControl);

        adk::ThermalMapperResult guarded;
        fillBytes (guarded, 0x5a);
        const adk::ThermalMapperResult before = guarded;
        require (mapper.update (input, guarded).error () ==
                     adk::StatusCode::NotInitialized,
                 "lifecycle rejection precedes malformed envelope, frame, and control");
        require (std::memcmp (&guarded, &before, sizeof guarded) == 0,
                 "lifecycle rejection preserves caller result");
    }

    void testEnvelopeFrameControlPrecedenceAndRecovery ()
    {
        adk::ThermalGradientMapper mapper (config ());

        require (mapper.initialize (adk::TimePoint (100)).ok (),
                 "precedence mapper initializes");

        const adk::QualifiedDs18b20Snapshot baselineFrame = frame (1, 110, 500);
        adk::ThermalMapperResult            baselineResult;
        require (mapper
                     .update (envelope (110, baselineFrame,
                                        control (1, 110, false, false)),
                              baselineResult)
                     .ok (),
                 "precedence baseline commits");
        adk::ThermalGradientIntent baselineIntent;
        require (mapper.snapshot (baselineIntent).ok (),
                 "precedence baseline snapshots");

        adk::QualifiedDs18b20Snapshot badFrame = frame (0, 90, 500);
        badFrame.sourceId                      = 0;
        adk::ThermalMapperControl badControl   = control (0, 90, true, true);
        badControl.ownerToken                  = 0;
        requireRejectedAtomically (
            mapper, envelope (100, badFrame, badControl),
            adk::StatusCode::InvalidArgument, baselineResult, baselineIntent,
            "envelope chronology rejects before malformed frame and control");

        requireRejectedAtomically (
            mapper, envelope (120, badFrame, badControl),
            adk::StatusCode::InvalidArgument, baselineResult, baselineIntent,
            "malformed frame rejects before malformed control");

        const adk::QualifiedDs18b20Snapshot goodFrame = frame (2, 120, 500);

        requireRejectedAtomically (
            mapper, envelope (120, goodFrame, badControl),
            adk::StatusCode::InvalidArgument, baselineResult, baselineIntent,
            "malformed control rejects after envelope and frame validation");

        adk::ThermalMapperResult recovered;
        require (mapper
                     .update (envelope (120, goodFrame, control (2, 120, false, false)),
                              recovered)
                     .ok (),
                 "valid call after three collision rejections commits");
        require (recovered.intent.pageIndex == 0 && !recovered.hasRecord,
                 "recovery proves rejected calls consumed no page or record state");
    }

    void testControlPrecedesFreshnessAndPagePrecedesRecord ()
    {
        adk::ThermalGradientMapper mapper (config ());

        require (mapper.initialize (adk::TimePoint (100)).ok (),
                 "freshness mapper initializes");

        adk::ThermalMapperResult baselineResult;
        require (mapper.update (envelope (120, frame (1, 120, 130),
                                         control (1, 120, false, false)),
                                baselineResult)
                     .ok (),
                 "freshness baseline commits");
        adk::ThermalGradientIntent baselineIntent;
        require (mapper.snapshot (baselineIntent).ok (),
                 "freshness baseline snapshots");

        adk::ThermalMapperControl invalid = control (2, 131, true, true);
        invalid.configurationRevision++;
        requireRejectedAtomically (
            mapper, envelope (131, frame (1, 120, 130), invalid),
            adk::StatusCode::InvalidArgument, baselineResult, baselineIntent,
            "control structure rejects before unchanged-frame child freshness");

        adk::ThermalMapperResult collision;
        const adk::Status collisionStatus =
            mapper.update (envelope (131, frame (1, 120, 130),
                                     control (2, 131, true, true)),
                           collision);
        require (collisionStatus.ok (),
                 "stale child with simultaneous page and record edges commits");
        require (collision.intent.health == adk::ThermalGradientHealth::Fault &&
                     collision.intent.overallFaultMask != 0,
                 "child freshness faults thermal classification before presentation");
        require (collision.intent.pageIndex == 1 &&
                     collision.intent.pageKind == adk::ThermalMapperPageKind::Probe,
                 "page edge applies after fault classification");
        require (collision.hasRecord && collision.record.recordSequence == 1 &&
                     collision.record.health == adk::ThermalGradientHealth::Fault &&
                     collision.record.faultMask == collision.intent.overallFaultMask,
                 "record follows page selection and preserves fault decision");

        adk::ThermalMapperResult held = collision;
        require (mapper
                     .update (envelope (132, frame (1, 120, 130),
                                        control (2, 131, true, true)),
                              held)
                     .ok (),
                 "identical collision replay is accepted");
        require (held.intent.pageIndex == 1 && !held.hasRecord,
                 "held page and record edges do not repeat");
    }

#if defined(ADK_TESTING)
    void testRecordFailureDoesNotPartiallyApplyPage ()
    {
        adk::ThermalGradientMapper mapper (config ());

        require (mapper.initialize (adk::TimePoint (100)).ok (),
                 "record-failure mapper initializes");

        adk::ThermalMapperResult baselineResult;
        require (mapper.update (envelope (120, frame (1, 120, 500),
                                         control (1, 120, false, false)),
                                baselineResult)
                     .ok (),
                 "record-failure baseline commits");
        adk::ThermalGradientIntent baselineIntent;
        require (mapper.snapshot (baselineIntent).ok (),
                 "record-failure baseline snapshots");

        adk::ThermalGradientMapperTestAccess::exhaustRecordSequence (mapper);

        requireRejectedAtomically (
            mapper,
            envelope (121, frame (2, 121, 500), control (2, 121, true, true)),
            adk::StatusCode::CapacityExceeded, baselineResult, baselineIntent,
            "record exhaustion atomically rejects a simultaneous page edge");
    }
#endif
} // namespace

int main ()
{
    testLifecyclePrecedesMalformedEnvelope ();

    testEnvelopeFrameControlPrecedenceAndRecovery ();

    testControlPrecedesFreshnessAndPagePrecedesRecord ();
#if defined(ADK_TESTING)
    testRecordFailureDoesNotPartiallyApplyPage ();
#endif

    std::cout << "thermal gradient mapper precedence tests passed\n";
    return EXIT_SUCCESS;
}
