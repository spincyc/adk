#include <thermal_gradient_mapper.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <iostream>

namespace {
    constexpr uint8_t  setSourceId     = 19;
    constexpr uint16_t setRevision     = 41;
    constexpr uint32_t controlOwner    = 0xC011066UL;
    constexpr uint8_t  controlSourceId = 23;
    constexpr uint16_t controlRevision = 43;

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
        adk::OneWireRomCode value = {{0x28, serial, 0x16, 0x26, 0x36, 0x46, 0x56, 0}};
        value.bytes[7]            = crc8 (value.bytes, 7);
        return value;
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

    adk::ThermalMapperConfig config (uint8_t count)
    {
        const adk::OneWireRomCode empty = {{0, 0, 0, 0, 0, 0, 0, 0}};
        adk::ThermalMapperConfig  value = {0x660066UL,
                                           47,
                                           setSourceId,
                                           setRevision,
                                           {rom (1), rom (2), rom (3), rom (4)},
                                           {empty, empty, empty, empty},
                                           count,
                                           controlOwner,
                                           controlSourceId,
                                           controlRevision,
                                           adk::Duration (100),
                                           16};

        const uint8_t order[4] = {2, 0, 3, 1};
        for (uint8_t index = 0; index < count; ++index)
        {
            value.spatialOrder[index] = rom (static_cast<uint8_t> (order[index] + 1));
        }
        return value;
    }

    adk::QualifiedDs18b20Probe probe (uint8_t identity, int16_t lower, int16_t upper)
    {
        return {rom (identity),
                7,
                static_cast<uint32_t> (100 + identity),
                static_cast<uint32_t> (200 + identity),
                adk::TimePoint (980),
                adk::TimePoint (1100),
                lower,
                lower,
                upper,
                adk::Ds18b20Resolution::Bits12,
                adk::Ds18b20ProbeQuality::Current,
                adk::Duration (20),
                adk::StatusCode::Ok};
    }

    adk::QualifiedDs18b20Snapshot snapshot ()
    {
        return {setSourceId,
                setRevision,
                7,
                adk::TimePoint (980),
                {probe         (1, 0, 0), probe (2, 0, 0), probe (3, 0, 0),
                 probe (4, 0, 0)},
                4,
                0x0F,
                0,
                adk::Ds18b20SetQuality::Complete,
                adk::StatusCode::Ok};
    }

    adk::ThermalMapperControl control ()
    {
        return {controlOwner, controlSourceId,      controlRevision,
                11,           adk::TimePoint (990), false,
                false,        adk::StatusCode::Ok};
    }

    adk::ThermalMapperResult update (adk::ThermalGradientMapper&          mapper,
                                     const adk::QualifiedDs18b20Snapshot& probes)
    {
        adk::ThermalMapperEnvelope envelope = {adk::TimePoint (1000), probes,
                                               control ()};
        adk::ThermalMapperResult   result;
        const adk::Status          status   = mapper.update (envelope, result);

        require (status.ok (), "gradient update should succeed");
        return result;
    }

    void setByRom (adk::QualifiedDs18b20Snapshot& value, uint8_t identity,
                   int16_t lower, int16_t upper)
    {
        for (uint8_t index = 0; index < 4; ++index)
        {
            if (sameRom (value.probes[index].rom, rom (identity)))
            {
                value.probes[index].rawSixteenths      = lower;
                value.probes[index].lowerRawSixteenths = lower;
                value.probes[index].upperRawSixteenths = upper;
                return;
            }
        }
        require (false, "fixture ROM should exist");
    }

    void testProjectionAndPermutations ()
    {
        for (uint8_t count = 2; count <= 4; ++count)
        {
            adk::ThermalGradientMapper mapper (config (count));

            require (mapper.initialize (adk::TimePoint (900)).ok (),
                     "mapper should initialize");
            adk::QualifiedDs18b20Snapshot probes = snapshot ();

            setByRom (probes, 3, 30, 31);
            setByRom (probes, 1, 10, 11);
            setByRom (probes, 4, 40, 41);
            setByRom (probes, 2, 20, 21);

            const adk::ThermalMapperResult result = update (mapper, probes);

            require (result.intent.probeCount == count,
                     "mapped count should be retained");
            require (result.intent.gradientCount == count - 1,
                     "mapped count should define adjacency");
            const uint8_t order[4] = {3, 1, 4, 2};
            for (uint8_t index = 0; index < count; ++index)
            {
                require (sameRom (result.intent.probes[index].rom, rom (order[index])),
                         "projection should use configured ROM order");
                require (result.intent.probes[index].lowerRawSixteenths ==
                             static_cast<int16_t> (order[index] * 10),
                         "projection should bind values by full ROM");
            }
        }

        adk::ThermalGradientIntent baseline;
        uint8_t                    permutation[4] = {0, 1, 2, 3};
        uint8_t                    permutationCount = 0;
        do
        {
            adk::ThermalGradientMapper mapper (config (3));

            require (mapper.initialize (adk::TimePoint (900)).ok (),
                     "permutation mapper should initialize");
            adk::QualifiedDs18b20Snapshot canonical = snapshot ();

            setByRom (canonical, 1, 10, 11);
            setByRom (canonical, 2, 20, 21);
            setByRom (canonical, 3, 30, 31);
            setByRom (canonical, 4, 40, 41);

            adk::QualifiedDs18b20Snapshot probes = canonical;
            for (uint8_t index = 0; index < 4; ++index)
            {
                probes.probes[index] = canonical.probes[permutation[index]];
            }
            const adk::ThermalMapperResult result = update (mapper, probes);
            if (permutationCount == 0)
            {
                std::memcpy (&baseline, &result.intent, sizeof (baseline));
            }
            else
            {
                require (
                    std::memcmp (&baseline, &result.intent, sizeof (baseline)) == 0,
                    "all input permutations should produce one canonical image");
            }
            ++permutationCount;
        } while (std::next_permutation (permutation, permutation + 4));

        require (permutationCount == 24,
                 "every exact-four input permutation should be tested");
    }

    void testWidenedIntervalsAndThresholds ()
    {
        struct Case
        {
            int16_t                     leftLower;
            int16_t                     leftUpper;
            int16_t                     rightLower;
            int16_t                     rightUpper;
            int32_t                     expectedLower;
            int32_t                     expectedUpper;
            adk::ThermalGradientQuality quality;
        };
        const Case cases[] = {
            {-32768, -32768, 32767, 32767, 65535, 65535,
             adk::ThermalGradientQuality::Rising},
            {32767, 32767, -32768, -32768, -65535, -65535,
             adk::ThermalGradientQuality::Falling},
            {0, 0, 15, 15, 15, 15, adk::ThermalGradientQuality::Flat},
            {0, 0, 16, 16, 16, 16, adk::ThermalGradientQuality::Rising},
            {0, 0, 17, 17, 17, 17, adk::ThermalGradientQuality::Rising},
            {0, 0, -15, -15, -15, -15, adk::ThermalGradientQuality::Flat},
            {0, 0, -16, -16, -16, -16, adk::ThermalGradientQuality::Falling},
            {0, 0, -17, -17, -17, -17, adk::ThermalGradientQuality::Falling},
            {0, 8, 15, 23, 7, 23, adk::ThermalGradientQuality::Indeterminate},
            {0, 20, 0, 20, -20, 20, adk::ThermalGradientQuality::Indeterminate}};

        for (const Case& value : cases)
        {
            adk::ThermalGradientMapper mapper (config (2));

            require (mapper.initialize (adk::TimePoint (900)).ok (),
                     "threshold mapper should initialize");
            adk::QualifiedDs18b20Snapshot probes = snapshot ();

            setByRom (probes, 3, value.leftLower, value.leftUpper);
            setByRom (probes, 1, value.rightLower, value.rightUpper);

            const adk::ThermalMapperResult result = update (mapper, probes);

            require (result.intent.gradients[0].lowerRawSixteenths ==
                             value.expectedLower &&
                         result.intent.gradients[0].upperRawSixteenths ==
                             value.expectedUpper,
                     "gradient should use widened interval subtraction");
            require (result.intent.gradients[0].quality == value.quality,
                     "threshold classification should honor equality");
        }
    }

    void testTiesAndInteriorFaults ()
    {
        adk::ThermalGradientMapper mapper (config (4));

        require (mapper.initialize (adk::TimePoint (900)).ok (),
                 "tie mapper should initialize");
        adk::QualifiedDs18b20Snapshot probes = snapshot ();

        setByRom (probes, 3, -30, -20);
        setByRom (probes, 1, -30, 10);
        setByRom (probes, 4, 5, 10);
        setByRom (probes, 2, 0, 2);

        adk::ThermalMapperResult result = update (mapper, probes);

        require (result.intent.minimumTieMask == 0x03,
                 "minimum ties should retain every mapped slot");
        require (result.intent.maximumTieMask == 0x06,
                 "maximum ties should retain every mapped slot");
        require (sameRom (result.intent.minimumRom, rom (3)) &&
                     sameRom (result.intent.maximumRom, rom (1)),
                 "tie primary should be lowest mapped slot");

        adk::ThermalGradientMapper faultMapper (config (4));

        require (faultMapper.initialize (adk::TimePoint (900)).ok (),
                 "fault mapper should initialize");
        probes.probes[0].quality = adk::Ds18b20ProbeQuality::ScratchpadCrcFault;
        probes.probes[0].status  = adk::StatusCode::Ok;
        probes.validCount        = 3;
        probes.faultMask         = 0x01;

        result                   = update (faultMapper, probes);

        require (result.intent.health == adk::ThermalGradientHealth::Fault,
                 "child fault should dominate health");
        require (result.intent.overallFaultMask == 0x02,
                 "source fault should project to mapped slot");
        require (result.intent.gradients[0].quality ==
                         adk::ThermalGradientQuality::Fault &&
                     result.intent.gradients[1].quality ==
                         adk::ThermalGradientQuality::Fault,
                 "interior fault should fault both incident pairs");
        require (result.intent.gradients[2].quality !=
                     adk::ThermalGradientQuality::Fault,
                 "nonincident pair should preserve healthy evidence");
        require (result.intent.gradients[2].lowerRawSixteenths == -10 &&
                     result.intent.gradients[2].upperRawSixteenths == -3 &&
                     result.intent.gradients[2].quality ==
                         adk::ThermalGradientQuality::Flat,
                 "nonincident pair should retain its exact interval");
    }

    void testEveryChildQualityAndProjectedMasks ()
    {
        const adk::Ds18b20ProbeQuality qualities[] = {
            adk::Ds18b20ProbeQuality::Unqualified,
            adk::Ds18b20ProbeQuality::ConversionPending,
            adk::Ds18b20ProbeQuality::ScratchpadCrcFault,
            adk::Ds18b20ProbeQuality::ResolutionMismatch,
            adk::Ds18b20ProbeQuality::ResetDefaultWithoutConversion,
            adk::Ds18b20ProbeQuality::ImplausibleStep,
            adk::Ds18b20ProbeQuality::Stale,
            adk::Ds18b20ProbeQuality::Missing,
            adk::Ds18b20ProbeQuality::DuplicateIdentity,
            adk::Ds18b20ProbeQuality::TransportFault};

        for (const adk::Ds18b20ProbeQuality quality : qualities)
        {
            adk::ThermalGradientMapper mapper (config (4));

            require (mapper.initialize (adk::TimePoint (900)).ok (),
                     "quality mapper should initialize");
            adk::QualifiedDs18b20Snapshot probes = snapshot ();
            probes.probes[0].quality             = quality;
            probes.validCount                    = 3;
            probes.faultMask                     = 0x01;
            if (quality == adk::Ds18b20ProbeQuality::Missing)
            {
                probes.presentMask =
                    static_cast<uint8_t> (probes.presentMask & ~0x01U);
                probes.quality = adk::Ds18b20SetQuality::Missing;
            }
            else if (quality == adk::Ds18b20ProbeQuality::DuplicateIdentity)
            {
                probes.quality = adk::Ds18b20SetQuality::DuplicateIdentity;
            }
            else if (quality == adk::Ds18b20ProbeQuality::TransportFault)
            {
                probes.quality          = adk::Ds18b20SetQuality::TransportFault;
                probes.probes[0].status = adk::StatusCode::HardwareFailure;
                probes.status           = adk::StatusCode::HardwareFailure;
            }
            else if (quality == adk::Ds18b20ProbeQuality::Unqualified)
            {
                probes.quality = adk::Ds18b20SetQuality::Unqualified;
            }

            const adk::ThermalMapperResult result = update (mapper, probes);

            require (result.intent.overallFaultMask == 0x02,
                     "each child fault should project by configured ROM");
            require (result.intent.probes[1].quality == quality,
                     "fault page should preserve child quality");
            require (result.intent.probes[1].lowerRawSixteenths == 0 &&
                         result.intent.probes[1].upperRawSixteenths == 0,
                     "fault page should not retain numeric temperature");
            require (result.intent.gradients[0].quality ==
                             adk::ThermalGradientQuality::Fault &&
                         result.intent.gradients[1].quality ==
                             adk::ThermalGradientQuality::Fault,
                     "each interior child fault should dominate incident pairs");
            require (result.intent.gradients[2].faultMask == 0,
                     "nonincident pair should not inherit a local fault");
        }
    }
} // namespace

int main ()
{
    testProjectionAndPermutations           ();
    testWidenedIntervalsAndThresholds       ();
    testTiesAndInteriorFaults               ();
    testEveryChildQualityAndProjectedMasks  ();
    std::cout << "thermal gradient mapper gradient tests passed\n";
    return EXIT_SUCCESS;
}
