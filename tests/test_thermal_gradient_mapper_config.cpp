#include <thermal_gradient_mapper.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

#if defined (ADK_TESTING)
namespace adk {
    struct ThermalGradientMapperTestAccess
    {
        static void seedLifecycleGeneration (ThermalGradientMapper& mapper,
                                             uint32_t               generation)
        {
            mapper.lifecycleGeneration_ = generation;
        }
    };
} // namespace adk
#endif

namespace {
    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::OneWireRomCode emptyRom ()
    {
        return {{0, 0, 0, 0, 0, 0, 0, 0}};
    }

    const adk::OneWireRomCode& sourceRom (uint8_t index)
    {
        static const adk::OneWireRomCode values[4] = {
            {{0x28, 0x04, 0, 0, 0, 0, 0, 0xc2}},
            {{0x28, 0x02, 0, 0, 0, 0, 0, 0x70}},
            {{0x28, 0x01, 0, 0, 0, 0, 0, 0x29}},
            {{0x28, 0x03, 0, 0, 0, 0, 0, 0x47}}};
        return values[index];
    }

    adk::ThermalMapperConfig config (uint8_t spatialCount = 4)
    {
        adk::ThermalMapperConfig value = {
            0x13572468UL,
            31,
            7,
            19,
            {emptyRom (), emptyRom (), emptyRom (), emptyRom ()},

            {emptyRom (), emptyRom (), emptyRom (), emptyRom ()},
            spatialCount,
            0x24681357UL,
            11,
            23,
            adk::Duration (250),
            16};
        for (uint8_t index = 0; index < 4; ++index)
        {
            value.sourceRoms[index]   = sourceRom (index);
            value.spatialOrder[index] = spatialCount >= 2 && index < spatialCount
                                            ? sourceRom (uint8_t (3U - index))

                                            : emptyRom ();
        }
        return value;
    }

#if defined (ADK_THERMAL_MAPPER_CONFIG_CONFIGURATION)
    void requireInitialize (const adk::ThermalMapperConfig& value,
                            adk::StatusCode expected, const char* message)
    {
        adk::ThermalGradientMapper mapper (value);

        require (mapper.initialize (adk::TimePoint (100)).error () == expected,
                 message);
    }
#endif

#if defined (ADK_THERMAL_MAPPER_CONFIG_SNAPSHOT_IDENTITY) || \
    defined (ADK_THERMAL_MAPPER_CONFIG_SNAPSHOT_PROBE) || \
    defined (ADK_THERMAL_MAPPER_CONFIG_SNAPSHOT_LIFECYCLE)
#if defined (ADK_THERMAL_MAPPER_CONFIG_SNAPSHOT_LIFECYCLE)
    adk::ThermalGradientIntent filledIntent ()
    {
        adk::ThermalGradientIntent intent;
        unsigned char*             bytes = reinterpret_cast<unsigned char*> (&intent);
        for (size_t index = 0; index < sizeof intent; ++index)
        {
            bytes[index] = 0xa5;
        }
        return intent;
    }
#endif

    adk::QualifiedDs18b20Probe currentProbe (uint8_t index)
    {
        const int16_t raw = static_cast<int16_t> (160 + index * 16);
        return {sourceRom (index),
                7,
                uint32_t (100U + index),

                uint32_t (200U + index),

                adk::TimePoint (980),

                adk::TimePoint (1100),
                raw,
                raw,
                raw,
                adk::Ds18b20Resolution::Bits12,
                adk::Ds18b20ProbeQuality::Current,
                adk::Duration (20),
                adk::StatusCode::Ok};
    }

    adk::QualifiedDs18b20Snapshot completeSnapshot ()
    {
        return {
            7,
            19,
            7,
            adk::TimePoint (980),

            {currentProbe (0), currentProbe (1), currentProbe (2), currentProbe (3)},
            4,
            0x0F,
            0,
            adk::Ds18b20SetQuality::Complete,
            adk::StatusCode::Ok};
    }

    adk::ThermalMapperControl noControl ()
    {
        return {0, 0, 0, 0, adk::TimePoint (), false, false, adk::StatusCode::Ok};
    }

    template <typename Value> void fillBytes (Value& value, uint8_t byte)
    {
        unsigned char* bytes = reinterpret_cast<unsigned char*> (&value);
        for (size_t index = 0; index < sizeof value; ++index)
        {
            bytes[index] = byte;
        }
    }

#if defined (ADK_THERMAL_MAPPER_CONFIG_SNAPSHOT_IDENTITY) || \
    defined (ADK_THERMAL_MAPPER_CONFIG_SNAPSHOT_PROBE)
    template <typename Mutator>
    void requireSnapshotRejection (Mutator mutate, const char* message)
    {
        adk::ThermalGradientMapper mapper (config ());

        require (mapper.initialize (adk::TimePoint (900)).ok (),
                 "structural fixture initializes");

        adk::ThermalGradientIntent beforeIntent;
        require (mapper.snapshot (beforeIntent).ok (), "structural fixture snapshots");

        adk::QualifiedDs18b20Snapshot probes = completeSnapshot ();

        mutate (probes);

        const adk::ThermalMapperEnvelope envelope = {adk::TimePoint (1000), probes,

                                                     noControl ()};
        adk::ThermalMapperResult         result;
        fillBytes (result, 0xa5);
        adk::ThermalMapperResult beforeResult;
        std::memcpy (&beforeResult, &result, sizeof result);

        const adk::Status status = mapper.update (envelope, result);

        if (status.error () != adk::StatusCode::InvalidArgument)
        {
            std::cerr << "Unexpected status " << unsigned (status.error ())
                      << " for: " << message << '\n';
        }
        require (status.error () == adk::StatusCode::InvalidArgument, message);

        require (std::memcmp (&result, &beforeResult, sizeof result) == 0,
                 "structural rejection preserves complete caller result");

        adk::ThermalGradientIntent afterIntent;
        require (mapper.snapshot (afterIntent).ok (),
                 "structural rejection leaves mapper snapshot available");
        if (std::memcmp (&afterIntent, &beforeIntent, sizeof afterIntent) != 0)
        {
            std::cerr << "Context: " << message << '\n';
        }
        require (std::memcmp (&afterIntent, &beforeIntent, sizeof afterIntent) == 0,
                 "structural rejection preserves complete mapper state");
    }
#endif

#if defined (ADK_THERMAL_MAPPER_CONFIG_SNAPSHOT_LIFECYCLE)
    void requireSnapshotAccepted (const adk::QualifiedDs18b20Snapshot& probes,
                                  const char*                          message)
    {
        adk::ThermalGradientMapper mapper (config ());

        require (mapper.initialize (adk::TimePoint (900)).ok (),
                 "accepted structural fixture initializes");
        const adk::ThermalMapperEnvelope envelope = {adk::TimePoint (1000), probes,

                                                     noControl ()};
        adk::ThermalMapperResult         result;
        require (mapper.update (envelope, result).ok (), message);
    }
#endif
#endif

#if defined (ADK_THERMAL_MAPPER_CONFIG_CONFIGURATION)
    void testSpatialCountBoundaries ()
    {
        requireInitialize (config (1), adk::StatusCode::InvalidConfiguration,
                           "one mapped source rejects");
        requireInitialize (config (2), adk::StatusCode::Ok,
                           "two mapped sources initialize");
        requireInitialize (config (3), adk::StatusCode::Ok,
                           "three mapped sources initialize");
        requireInitialize (config (4), adk::StatusCode::Ok,
                           "four mapped sources initialize");
        requireInitialize (config (5), adk::StatusCode::InvalidConfiguration,
                           "five mapped sources reject");
    }

    void testSourceIdentityValidation ()
    {
        adk::ThermalMapperConfig value = config ();

        for (uint8_t index = 0; index < 4; ++index)
        {
            value = config ();

            value.sourceRoms[index] = emptyRom ();

            requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                               "zero ROM in every source slot rejects");

            value                            = config ();
            value.sourceRoms[index].bytes[0] = 0x10;
            requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                               "wrong family in every source slot rejects");

            value = config ();
            value.sourceRoms[index].bytes[7] ^= 0x01U;
            requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                               "bad CRC in every source slot rejects");
        }
        for (uint8_t left = 0; left < 4; ++left)
        {
            for (uint8_t right = uint8_t (left + 1U); right < 4; ++right)
            {
                value                   = config ();
                value.sourceRoms[right] = value.sourceRoms[left];
                requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                                   "every duplicate source pair rejects");

                value                          = config ();
                const adk::OneWireRomCode held = value.sourceRoms[left];
                value.sourceRoms[left]         = value.sourceRoms[right];
                value.sourceRoms[right]        = held;
                requireInitialize (value, adk::StatusCode::Ok,
                                   "source configuration order has no spatial meaning");
            }
        }
    }

    void testSpatialIdentityValidation ()
    {
        adk::ThermalMapperConfig value = config (2);
        value.spatialOrder[1]          = value.spatialOrder[0];
        requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                           "duplicate mapped identity rejects");

        value                 = config (2);
        value.spatialOrder[1] = {{0x28, 0x05, 0, 0, 0, 0, 0, 0x9b}};
        requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                           "foreign mapped identity rejects");

        value = config (2);

        requireInitialize (value, adk::StatusCode::Ok,
                           "ordered subset with two unmapped sources initializes");

        for (uint8_t count = 2; count <= 3; ++count)
        {
            for (uint8_t index = count; index < 4; ++index)
            {
                value = config (count);

                value.spatialOrder[index] = sourceRom (0);

                requireInitialize (
                    value, adk::StatusCode::InvalidConfiguration,
                    "each nonzero identity outside mapped count rejects");
            }
        }
    }

    void testScalarConfigurationValidation ()
    {
        adk::ThermalMapperConfig value = config ();
        value.ownerToken               = 0;
        requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                           "zero mapper owner rejects");

        value                       = config ();
        value.configurationRevision = 0;
        requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                           "zero mapper revision rejects");

        value                     = config ();
        value.expectedSetSourceId = 0;
        requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                           "zero set source rejects");

        value                                  = config ();
        value.expectedSetConfigurationRevision = 0;
        requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                           "zero set revision rejects");

        value                           = config ();
        value.expectedControlOwnerToken = 0;
        requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                           "zero control owner rejects");

        value                         = config ();
        value.expectedControlSourceId = 0;
        requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                           "zero control source rejects");

        value                                      = config ();
        value.expectedControlConfigurationRevision = 0;
        requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                           "zero control revision rejects");

        value = config ();

        value.maximumControlAge = adk::Duration ();

        requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                           "zero maximum control age rejects");

        value = config ();

        value.maximumControlAge = adk::Duration (1);

        requireInitialize (value, adk::StatusCode::Ok,
                           "minimum positive control age initializes");

        value = config ();

        value.maximumControlAge = adk::Duration (0x7fffffffUL);

        requireInitialize (value, adk::StatusCode::Ok,
                           "largest unambiguous control age initializes");

        value = config ();

        value.maximumControlAge = adk::Duration (0x80000000UL);

        requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                           "half-range control age rejects");

        value = config ();

        value.maximumControlAge = adk::Duration (0x80000001UL);

        requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                           "above-half-range control age rejects");

        value                                 = config ();
        value.meaningfulGradientRawSixteenths = 0;
        requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                           "zero gradient threshold rejects");

        value                                 = config ();
        value.meaningfulGradientRawSixteenths = 1;
        requireInitialize (value, adk::StatusCode::Ok,
                           "minimum positive gradient threshold initializes");

        value                                 = config ();
        value.meaningfulGradientRawSixteenths = 2880;
        requireInitialize (value, adk::StatusCode::Ok,
                           "maximum gradient threshold initializes");

        value                                 = config ();
        value.meaningfulGradientRawSixteenths = 2881;
        requireInitialize (value, adk::StatusCode::InvalidConfiguration,
                           "above-maximum gradient threshold rejects");
    }

    void testConfigurationIsCopied ()
    {
        adk::ThermalMapperConfig value = config (2);

        adk::ThermalGradientMapper mapper (value);

        value.ownerToken                       = 0;
        value.configurationRevision            = 0;
        value.expectedSetSourceId              = 0;
        value.expectedSetConfigurationRevision = 0;
        value.sourceRoms[0]                    = emptyRom ();

        value.spatialOrder[0]                      = emptyRom ();
        value.spatialCount                         = 0;
        value.expectedControlOwnerToken            = 0;
        value.expectedControlSourceId              = 0;
        value.expectedControlConfigurationRevision = 0;
        value.maximumControlAge                    = adk::Duration ();
        value.meaningfulGradientRawSixteenths      = 0;

        require (mapper.initialize (adk::TimePoint (100)).ok (),
                 "constructor copies complete caller configuration");
    }
#endif

#if defined (ADK_THERMAL_MAPPER_CONFIG_SNAPSHOT_IDENTITY)
    void testSnapshotTopLevelStructure ()
    {
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.sourceId = 0;
            },
            "zero set source rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.sourceId = 8;
            },
            "foreign set source rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.configurationRevision = 0;
            },
            "zero set revision rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.configurationRevision = 20;
            },
            "foreign set revision rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.cycleSequence = 0;
            },
            "zero set cycle sequence rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.observedAt = adk::TimePoint (1001);
            },
            "future set observation rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.observedAt = adk::TimePoint (0x800003e8UL);
            },
            "half-range set observation rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.quality = static_cast<adk::Ds18b20SetQuality> (0xff);
            },
            "invalid set quality enum rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.status = static_cast<adk::StatusCode> (0xff);
            },
            "invalid set status enum rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.quality = adk::Ds18b20SetQuality::TransportFault;
            },
            "complete fields with transport-fault set quality reject");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.status = adk::StatusCode::HardwareFailure;
            },
            "complete set quality with failure status rejects");
    }

    void testSnapshotIdentityAndAggregateCoherence ()
    {
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[0].rom = emptyRom ();
            },
            "zero probe identity rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[1].rom = value.probes[0].rom;
            },
            "duplicate probe identity rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[2].rom.bytes[0] = 0x10;
            },
            "foreign probe family rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[2].rom.bytes[7] ^= 1U;
            },
            "bad probe ROM CRC rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[3].rom = {{0x28, 0x05, 0, 0, 0, 0, 0, 0x9b}};
            },
            "foreign valid probe identity rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.validCount = 3;
            },
            "wrong complete valid count rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.validCount = 5;
            },
            "out-of-range valid count rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.presentMask = 0x07;
            },
            "complete present mask missing bit rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.presentMask = 0x1F;
            },
            "present mask high bit rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.faultMask = 0x01;
            },
            "complete fault mask rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.faultMask = 0x10;
            },
            "fault mask high bit rejects");
    }
#endif

#if defined (ADK_THERMAL_MAPPER_CONFIG_SNAPSHOT_PROBE)
    void testSnapshotCurrentProbeCoherence ()
    {
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[0].cycleSequence = 8;
            },
            "probe cycle mismatch rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[0].conversionGeneration = 0;
            },
            "zero current conversion generation rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[0].readTransactionGeneration = 0;
            },
            "zero current read generation rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[0].observedAt = adk::TimePoint (1001);
            },
            "future probe observation rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[0].freshThrough = adk::TimePoint (979);
            },
            "fresh-through before observation rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[0].freshThrough = adk::TimePoint (0x800003d4UL);
            },
            "half-range freshness interval rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[0].rawSixteenths++;
            },
            "raw value outside singleton interval rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[0].lowerRawSixteenths++;
            },
            "inverted current interval rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[0].upperRawSixteenths--;
            },
            "raw value above current interval rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[0].resolution = static_cast<adk::Ds18b20Resolution> (0xff);
            },
            "invalid resolution enum rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[0].quality = static_cast<adk::Ds18b20ProbeQuality> (0xff);
            },
            "invalid probe quality enum rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[0].status = adk::StatusCode::HardwareFailure;
            },
            "current probe with failure status rejects");
        requireSnapshotRejection (
            [] (adk::QualifiedDs18b20Snapshot& value)
            {
                value.probes[0].status = static_cast<adk::StatusCode> (0xff);
            },
            "invalid probe status enum rejects");
    }

    void testSnapshotResolutionDomains ()
    {
        for (uint8_t resolution = 0; resolution < 4; ++resolution)
        {
            adk::ThermalGradientMapper mapper (config ());

            require (mapper.initialize (adk::TimePoint (900)).ok (),
                     "resolution fixture initializes");
            adk::QualifiedDs18b20Snapshot probes = completeSnapshot ();
            const int16_t                 uncertainty =
                static_cast<int16_t> ((1U << (3U - resolution)) - 1U);
            for (uint8_t index = 0; index < 4; ++index)
            {
                probes.probes[index].resolution =
                    static_cast<adk::Ds18b20Resolution> (resolution);
                probes.probes[index].upperRawSixteenths = static_cast<int16_t> (
                    probes.probes[index].rawSixteenths + uncertainty);
            }
            const adk::ThermalMapperEnvelope envelope = {adk::TimePoint (1000), probes,

                                                         noControl ()};
            adk::ThermalMapperResult         result;
            require (mapper.update (envelope, result).ok (),
                     "each coherent DS18B20 resolution interval is accepted");
        }

        for (uint8_t quality = 0;
             quality <= static_cast<uint8_t> (adk::Ds18b20ProbeQuality::TransportFault);
             ++quality)
        {
            if (quality == static_cast<uint8_t> (adk::Ds18b20ProbeQuality::Current))
            {
                continue;
            }
            requireSnapshotRejection (
                [quality] (adk::QualifiedDs18b20Snapshot& value)
                {
                    value.probes[0].quality =
                        static_cast<adk::Ds18b20ProbeQuality> (quality);
                },
                "noncurrent probe with complete aggregates rejects");
        }
    }

    void testCopiedAgeIsRecomputed ()
    {
        adk::ThermalGradientMapper mapper (config ());

        require (mapper.initialize (adk::TimePoint (900)).ok (),
                 "age fixture initializes");
        adk::QualifiedDs18b20Snapshot probes = completeSnapshot ();

        for (uint8_t index = 0; index < 4; ++index)
        {
            probes.probes[index].age = adk::Duration (UINT32_MAX);
        }
        const adk::ThermalMapperEnvelope envelope = {adk::TimePoint (1000), probes,

                                                     noControl ()};
        adk::ThermalMapperResult         result;
        require (mapper.update (envelope, result).ok (),
                 "copied producer age does not control acceptance");
        for (uint8_t index = 0; index < 4; ++index)
        {
            require (result.intent.probes[index].age == adk::Duration (20),
                     "mapper recomputes age from supplied now and observation");
        }
    }
#endif

#if defined (ADK_THERMAL_MAPPER_CONFIG_SNAPSHOT_LIFECYCLE)
    void testCanonicalUnhealthySnapshots ()
    {
        struct FaultCase
        {
            adk::Ds18b20ProbeQuality quality;
            const char*              message;
        };
        const FaultCase localFaults[] = {
            {adk::Ds18b20ProbeQuality::ConversionPending,
             "canonical conversion-pending snapshot is accepted"},
            {adk::Ds18b20ProbeQuality::ScratchpadCrcFault,
             "canonical scratchpad-CRC snapshot is accepted"},
            {adk::Ds18b20ProbeQuality::ResolutionMismatch,
             "canonical resolution-mismatch snapshot is accepted"},
            {adk::Ds18b20ProbeQuality::ResetDefaultWithoutConversion,
             "canonical reset-default snapshot is accepted"},
            {adk::Ds18b20ProbeQuality::ImplausibleStep,
             "canonical implausible-step snapshot is accepted"},
            {adk::Ds18b20ProbeQuality::Stale, "canonical stale snapshot is accepted"}};
        for (const FaultCase& fault : localFaults)
        {
            adk::QualifiedDs18b20Snapshot probes = completeSnapshot ();
            probes.probes[0].quality             = fault.quality;
            if (fault.quality ==
                adk::Ds18b20ProbeQuality::ResetDefaultWithoutConversion)
            {
                probes.probes[0].rawSixteenths      = 1360;
                probes.probes[0].lowerRawSixteenths = 1360;
                probes.probes[0].upperRawSixteenths = 1360;
                probes.probes[0].freshThrough       = probes.probes[0].observedAt;
            }
            else if (fault.quality == adk::Ds18b20ProbeQuality::Stale)
            {
                probes.probes[0].freshThrough = adk::TimePoint (999);
            }
            probes.validCount = 3;
            probes.faultMask  = 0x01;
            requireSnapshotAccepted (probes, fault.message);
        }

        adk::QualifiedDs18b20Snapshot missing = completeSnapshot ();
        missing.probes[0].quality             = adk::Ds18b20ProbeQuality::Missing;
        missing.validCount                    = 3;
        missing.presentMask                   = 0x0E;
        missing.faultMask                     = 0x01;
        missing.quality                       = adk::Ds18b20SetQuality::Missing;
        requireSnapshotAccepted (missing, "canonical missing snapshot is accepted");

        adk::QualifiedDs18b20Snapshot transport = completeSnapshot ();
        transport.validCount                    = 0;
        transport.presentMask                   = 0;
        transport.faultMask                     = 0x0F;
        transport.quality = adk::Ds18b20SetQuality::TransportFault;
        transport.status  = adk::StatusCode::HardwareFailure;
        for (uint8_t index = 0; index < 4; ++index)
        {
            transport.probes[index].quality = adk::Ds18b20ProbeQuality::TransportFault;
            transport.probes[index].status  = adk::StatusCode::HardwareFailure;
        }
        requireSnapshotAccepted (
            transport, "canonical producer transport-fault snapshot is accepted");

        adk::QualifiedDs18b20Snapshot duplicate = completeSnapshot ();
        duplicate.validCount                    = 0;
        duplicate.presentMask                   = 0x03;
        duplicate.faultMask                     = 0x0F;
        duplicate.quality           = adk::Ds18b20SetQuality::DuplicateIdentity;
        duplicate.probes[0].quality = adk::Ds18b20ProbeQuality::DuplicateIdentity;
        duplicate.probes[1].quality = adk::Ds18b20ProbeQuality::ScratchpadCrcFault;
        duplicate.probes[2].quality = adk::Ds18b20ProbeQuality::Missing;
        duplicate.probes[3].quality = adk::Ds18b20ProbeQuality::Missing;
        requireSnapshotAccepted (duplicate,
                                 "canonical duplicate-identity collision is accepted");

        adk::QualifiedDs18b20Snapshot unknown = completeSnapshot ();
        unknown.validCount                    = 1;
        unknown.presentMask                   = 0x03;
        unknown.faultMask                     = 0x0E;
        unknown.quality                       = adk::Ds18b20SetQuality::UnknownIdentity;
        unknown.probes[1].quality = adk::Ds18b20ProbeQuality::ScratchpadCrcFault;
        unknown.probes[2].quality = adk::Ds18b20ProbeQuality::Missing;
        unknown.probes[3].quality = adk::Ds18b20ProbeQuality::Missing;
        requireSnapshotAccepted (unknown,
                                 "canonical unknown-identity collision is accepted");
    }

    void testSnapshotReplayAndStructuralPrecedence ()
    {
        adk::ThermalGradientMapper mapper (config ());

        require (mapper.initialize (adk::TimePoint (900)).ok (),
                 "replay fixture initializes");
        const adk::QualifiedDs18b20Snapshot probes = completeSnapshot ();

        adk::ThermalMapperEnvelope first = {adk::TimePoint (1000), probes,

                                            noControl ()};
        adk::ThermalMapperResult   result;
        require (mapper.update (first, result).ok (),
                 "canonical first snapshot is accepted");
        const adk::ThermalGradientIntent accepted = result.intent;

        first.now = adk::TimePoint (1001);

        require (mapper.update (first, result).ok (),
                 "byte-identical snapshot replay is idempotent");
        require (result.intent.pageIndex == accepted.pageIndex &&
                     result.intent.pageKind == accepted.pageKind &&
                     result.intent.probes[0].lowerRawSixteenths ==
                         accepted.probes[0].lowerRawSixteenths &&
                     result.intent.probes[0].age == adk::Duration (21) &&
                     !result.hasRecord,
                 "identical replay only refreshes mapper-derived age");

        adk::QualifiedDs18b20Snapshot changed = probes;
        changed.probes[0].rawSixteenths++;
        changed.probes[0].lowerRawSixteenths++;
        changed.probes[0].upperRawSixteenths++;
        adk::ThermalMapperControl validEdge = {
            0x24681357UL,          11,   23,   1,
            adk::TimePoint (1002), true, true, adk::StatusCode::Ok};

        const adk::ThermalMapperEnvelope collision = {adk::TimePoint (1002), changed,
                                                      validEdge};
        adk::ThermalMapperResult         sentinel;
        fillBytes (sentinel, 0x5a);
        adk::ThermalMapperResult before;
        std::memcpy (&before, &sentinel, sizeof sentinel);

        require (mapper.update (collision, sentinel).error () ==
                     adk::StatusCode::InvalidArgument,
                 "changed duplicate frame dominates valid page and record edges");
        require (std::memcmp (&sentinel, &before, sizeof sentinel) == 0,
                 "changed duplicate collision preserves caller output");

        adk::ThermalGradientIntent after;
        require (mapper.snapshot (after).ok (),
                 "changed duplicate collision leaves snapshot available");
        require (after.pageIndex == accepted.pageIndex &&
                     after.pageKind == accepted.pageKind &&
                     after.probes[0].lowerRawSixteenths ==
                         accepted.probes[0].lowerRawSixteenths &&
                     after.probes[0].age == adk::Duration (21),
                 "changed duplicate collision preserves retained behavior");

        adk::QualifiedDs18b20Snapshot reverse = probes;
        reverse.cycleSequence                 = 6;
        for (uint8_t index = 0; index < 4; ++index)
        {
            reverse.probes[index].cycleSequence = 6;
        }
        const adk::ThermalMapperEnvelope reversed = {adk::TimePoint (1003), reverse,

                                                     noControl ()};

        fillBytes (sentinel, 0x3c);
        adk::ThermalMapperResult beforeReverse;
        std::memcpy (&beforeReverse, &sentinel, sizeof sentinel);

        require (mapper.update (reversed, sentinel).error () ==
                     adk::StatusCode::InvalidArgument,
                 "reverse frame sequence rejects");
        require (std::memcmp (&sentinel, &beforeReverse, sizeof sentinel) == 0,
                 "reverse frame rejection preserves caller output");
    }

    void testLifecycleAndAtomicSnapshots ()
    {
        adk::ThermalGradientMapper mapper (config ());

        adk::ThermalGradientIntent sentinel = filledIntent ();
        adk::ThermalGradientIntent before;
        std::memcpy (&before, &sentinel, sizeof sentinel);
        adk::ThermalMapperResult updateResult;
        fillBytes (updateResult, 0x6c);
        adk::ThermalMapperResult beforeUpdate;
        std::memcpy (&beforeUpdate, &updateResult, sizeof updateResult);

        const adk::ThermalMapperEnvelope envelope = {adk::TimePoint (100),

                                                     completeSnapshot (), noControl ()};

        require (!mapper.initialized (), "construction is inert");

        require (mapper.update (envelope, updateResult).error () ==
                     adk::StatusCode::NotInitialized,
                 "update before initialize rejects");
        require (std::memcmp (&updateResult, &beforeUpdate, sizeof updateResult) == 0,
                 "pre-initialize update preserves complete result");
        require (mapper.reset (adk::TimePoint (99)).error () ==
                     adk::StatusCode::NotInitialized,
                 "reset before initialize rejects");
        require (mapper.shutdown ().ok (), "shutdown before initialize is idempotent");

        require (mapper.snapshot (sentinel).error () == adk::StatusCode::NotInitialized,
                 "snapshot before initialize rejects");
        require (std::memcmp (&sentinel, &before, sizeof sentinel) == 0,
                 "rejected snapshot preserves caller bytes");

        require (mapper.initialize (adk::TimePoint (100)).ok (),
                 "first initialize succeeds");
        require (mapper.initialized (), "initialized state is observable");

        require (mapper.initialize (adk::TimePoint (101)).ok (),
                 "repeated initialize is idempotent");
        require (mapper.snapshot (sentinel).ok (), "initialized snapshot succeeds");

        require (sentinel.health == adk::ThermalGradientHealth::Qualifying &&
                     sentinel.probeCount == 0 && sentinel.gradientCount == 0,
                 "initial intent is qualifying with no projected evidence");

        require (mapper.reset (adk::TimePoint (102)).ok (), "reset succeeds");

        require (mapper.initialized (), "reset preserves initialized lifecycle");

        require (mapper.snapshot (sentinel).ok () &&
                     sentinel.health == adk::ThermalGradientHealth::Qualifying,
                 "reset restores qualifying intent");

        require (mapper.shutdown ().ok (), "shutdown succeeds");

        require (!mapper.initialized (), "shutdown makes mapper inert");

        require (mapper.shutdown ().ok (), "repeated shutdown is idempotent");

        before = filledIntent ();

        std::memcpy (&sentinel, &before, sizeof sentinel);

        fillBytes (updateResult, 0x9d);
        adk::ThermalMapperResult afterShutdown;
        std::memcpy (&afterShutdown, &updateResult, sizeof updateResult);

        require (mapper.update (envelope, updateResult).error () ==
                     adk::StatusCode::NotInitialized,
                 "update after shutdown rejects");
        require (std::memcmp (&updateResult, &afterShutdown, sizeof updateResult) == 0,
                 "post-shutdown update preserves complete result");
        require (mapper.snapshot (sentinel).error () == adk::StatusCode::NotInitialized,
                 "snapshot after shutdown rejects");
        require (std::memcmp (&sentinel, &before, sizeof sentinel) == 0,
                 "post-shutdown snapshot rejection is atomic");

        require (mapper.initialize (adk::TimePoint (103)).ok (),
                 "reinitialize after shutdown succeeds");
        require (mapper.initialized (), "reinitialized state is observable");
    }

#if defined (ADK_TESTING)
    void testLifecycleGenerationExhaustion ()
    {
        adk::ThermalGradientMapper beforeInitialize (config ());

        adk::ThermalGradientMapperTestAccess::seedLifecycleGeneration (beforeInitialize,
                                                                       UINT32_MAX);
        require (beforeInitialize.initialize (adk::TimePoint (100)).error () ==
                     adk::StatusCode::CapacityExceeded,
                 "lifecycle exhaustion rejects initialize");
        require (!beforeInitialize.initialized (),
                 "failed exhausted initialize remains inert");

        adk::ThermalGradientMapper duringLifecycle (config ());

        require (duringLifecycle.initialize (adk::TimePoint (100)).ok (),
                 "reset exhaustion fixture initializes");
        adk::ThermalGradientMapperTestAccess::seedLifecycleGeneration (duringLifecycle,
                                                                       UINT32_MAX);
        require (duringLifecycle.reset (adk::TimePoint (101)).error () ==
                     adk::StatusCode::CapacityExceeded,
                 "lifecycle exhaustion rejects reset");
        require (!duringLifecycle.initialized (),
                 "exhausted reset fails closed to inert");

        adk::ThermalGradientIntent sentinel = filledIntent ();
        adk::ThermalGradientIntent before;
        std::memcpy (&before, &sentinel, sizeof sentinel);

        require (duringLifecycle.snapshot (sentinel).error () ==
                     adk::StatusCode::NotInitialized,
                 "snapshot after reset exhaustion rejects");
        require (std::memcmp (&sentinel, &before, sizeof sentinel) == 0,
                 "exhausted reset preserves rejected snapshot output");
        require (duringLifecycle.initialize (adk::TimePoint (102)).error () ==
                     adk::StatusCode::CapacityExceeded,
                 "lifecycle exhaustion is terminal");
    }
#endif
#endif
} // namespace

int main ()
{
#if defined (ADK_THERMAL_MAPPER_CONFIG_CONFIGURATION)
    testSpatialCountBoundaries ();

    testSourceIdentityValidation ();

    testSpatialIdentityValidation ();

    testScalarConfigurationValidation ();

    testConfigurationIsCopied ();

#elif defined (ADK_THERMAL_MAPPER_CONFIG_SNAPSHOT_IDENTITY)
    testSnapshotTopLevelStructure ();

    testSnapshotIdentityAndAggregateCoherence ();

#elif defined (ADK_THERMAL_MAPPER_CONFIG_SNAPSHOT_PROBE)
    testSnapshotCurrentProbeCoherence ();

    testSnapshotResolutionDomains ();

    testCopiedAgeIsRecomputed ();

#elif defined (ADK_THERMAL_MAPPER_CONFIG_SNAPSHOT_LIFECYCLE)
    testCanonicalUnhealthySnapshots ();

    testSnapshotReplayAndStructuralPrecedence ();

    testLifecycleAndAtomicSnapshots ();
#if defined (ADK_TESTING)
    testLifecycleGenerationExhaustion ();
#endif
#else
#error "Select one thermal mapper configuration test partition"
#endif

    std::cout << "thermal gradient mapper configuration tests passed\n";
    return EXIT_SUCCESS;
}
