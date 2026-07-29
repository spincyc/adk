#include <thermal_radiant_observation.h>

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <type_traits>

namespace {

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::ThermalRadiantConfig config ()
    {
        return {20000, 30000, adk::Duration (100), adk::Duration (20),
                adk::Duration (50)};
    }

    adk::ThermalRadiantEnvelope envelope (uint32_t observedAt = 100,
                                          uint32_t sequence   = 1)
    {
        return {{1, 11, 21, sequence, adk::TimePoint (observedAt), 15000, 1000, false,
                 adk::StatusCode::Ok},
                {2, 12, 22, sequence, adk::TimePoint (observedAt), 100,
                 adk::ThresholdState::Below, false, adk::StatusCode::Ok},
                {3, 13, 23, sequence, adk::TimePoint (observedAt), 200,
                 adk::ThresholdState::Below, false, adk::StatusCode::Ok}};
    }

    bool thermalSampleEqual (const adk::ConvertedThermalSample& left,
                             const adk::ConvertedThermalSample& right)
    {
        return left.sourceId == right.sourceId &&
               left.configurationRevision == right.configurationRevision &&
               left.calibrationRevision == right.calibrationRevision &&
               left.sequence == right.sequence && left.observedAt == right.observedAt &&
               left.milliCelsius == right.milliCelsius &&
               left.uncertaintyMilliCelsius == right.uncertaintyMilliCelsius &&
               left.saturated == right.saturated && left.status == right.status;
    }

    bool categoricalSampleEqual (const adk::CategoricalThresholdSample& left,
                                 const adk::CategoricalThresholdSample& right)
    {
        return left.sourceId == right.sourceId &&
               left.configurationRevision == right.configurationRevision &&
               left.calibrationRevision == right.calibrationRevision &&
               left.sequence == right.sequence && left.observedAt == right.observedAt &&
               left.raw == right.raw && left.state == right.state &&
               left.saturated == right.saturated && left.status == right.status;
    }

    bool observationEqual (const adk::ThermalRadiantObservation& left,
                           const adk::ThermalRadiantObservation& right)
    {
        return thermalSampleEqual (left.envelope.thermistor,
                                   right.envelope.thermistor) &&
               categoricalSampleEqual (left.envelope.digitalTemperature,
                                       right.envelope.digitalTemperature) &&
               categoricalSampleEqual (left.envelope.radiant, right.envelope.radiant) &&
               left.thermalQuality == right.thermalQuality &&
               left.radiantQuality == right.radiantQuality &&
               left.thermistorAge == right.thermistorAge &&
               left.digitalTemperatureAge == right.digitalTemperatureAge &&
               left.radiantAge == right.radiantAge &&
               left.thermalHazard == right.thermalHazard &&
               left.radiantHazard == right.radiantHazard && left.status == right.status;
    }

    void requireRejectedAtomically (adk::ThermalRadiantObservationPolicy& policy,
                                    adk::TimePoint                        now,
                                    const adk::ThermalRadiantEnvelope&    input,
                                    adk::StatusCode expected, const char* message)
    {
        const adk::ThermalRadiantObservation before = policy.snapshot ();
        require                                                       (policy.update (now, input).error () == expected, message);
        require                                                       (observationEqual (before, policy.snapshot ()),
                 "rejected envelope leaves the complete snapshot unchanged");
    }

    void testLifecycleAndConfiguration ()
    {
        static_assert (
            !std::is_copy_constructible<adk::ThermalRadiantObservationPolicy>::value,
            "thermal-radiant policy must not copy");
        static_assert (
            !std::is_move_constructible<adk::ThermalRadiantObservationPolicy>::value,
            "thermal-radiant policy must not move");
        static_assert (
            std::is_trivially_destructible<adk::ThermalRadiantObservationPolicy>::value,
            "thermal-radiant destruction must remain inert");

        adk::ThermalRadiantObservationPolicy policy (config ());
        require                                     (!policy.initialized (), "construction is inert");
        require                                     (
            policy.snapshot ().thermalQuality == adk::ThermalQuality::Unqualified &&
                policy.snapshot ().radiantQuality == adk::RadiantQuality::Unqualified &&
                policy.snapshot ().status.error () == adk::StatusCode::NotInitialized,
            "construction publishes the canonical empty snapshot");
        require (policy.update (adk::TimePoint (100), envelope ()).error () ==
                     adk::StatusCode::NotInitialized,
                 "update before initialization rejects");
        require (policy.initialize ().ok () && policy.initialize ().ok (),
                 "initialize is successful and idempotent");
        require (policy.update (adk::TimePoint (100), envelope ()).ok (),
                 "initialized policy accepts an envelope");
        policy.reset ();
        require      (policy.initialized (), "reset preserves valid configuration");
        require      (
            policy.snapshot ().thermalQuality == adk::ThermalQuality::Unqualified &&
                policy.snapshot ().radiantQuality == adk::RadiantQuality::Unqualified,
            "reset publishes unqualified source state");
        require (policy.update (adk::TimePoint (1), envelope (1, 1)).ok (),
                 "reset begins new ordering and candidate epochs");

        adk::ThermalRadiantConfig largest = config         ();
        largest.maximumAge                = adk::Duration  (0x7fffffffUL);
        largest.radiantPulseMaximum       = adk::Duration  (0x7ffffffeUL);
        largest.radiantSustainedMinimum   = adk::Duration  (0x7fffffffUL);
        adk::ThermalRadiantObservationPolicy largestPolicy (largest);
        require                                            (largestPolicy.initialize ().ok (),
                 "largest wrap-safe duration values initialize");

        using Mutator           = void (*) (adk::ThermalRadiantConfig&);
        const Mutator invalid[] = {
            [] (adk::ThermalRadiantConfig& value)
            {
                value.warningMilliCelsius = value.alarmMilliCelsius;
            },
            [] (adk::ThermalRadiantConfig& value)
            {
                value.warningMilliCelsius = value.alarmMilliCelsius + 1;
            },
            [] (adk::ThermalRadiantConfig& value)
            {
                value.maximumAge = adk::Duration (0);
            },
            [] (adk::ThermalRadiantConfig& value)
            {
                value.maximumAge = adk::Duration (0x80000000UL);
            },
            [] (adk::ThermalRadiantConfig& value)
            {
                value.radiantPulseMaximum = adk::Duration (0);
            },
            [] (adk::ThermalRadiantConfig& value)
            {
                value.radiantPulseMaximum = value.radiantSustainedMinimum;
            },
            [] (adk::ThermalRadiantConfig& value)
            {
                value.radiantSustainedMinimum = adk::Duration (0x80000000UL);
            }};
        for (Mutator mutate : invalid)
        {
            adk::ThermalRadiantConfig rejected = config    ();
            mutate                                         (rejected);
            adk::ThermalRadiantObservationPolicy candidate (rejected);
            require                                        (candidate.initialize ().error () ==
                             adk::StatusCode::InvalidConfiguration &&
                         !candidate.initialized (),
                     "invalid configuration remains inert");
        }
    }

    void testStructuralValidationAndSourceIdentity ()
    {
        adk::ThermalRadiantObservationPolicy policy (config ());
        require                                     (policy.initialize ().ok (), "structural fixture initializes");
        require                                     (policy.update (adk::TimePoint (100), envelope ()).ok (),
                 "structural fixture seeds accepted state");

        using Mutator           = void (*) (adk::ThermalRadiantEnvelope&);
        const Mutator invalid[] = {
            [] (adk::ThermalRadiantEnvelope& value)
            {
                value.thermistor.sourceId = 0;
            },
            [] (adk::ThermalRadiantEnvelope& value)
            {
                value.digitalTemperature.configurationRevision = 0;
            },
            [] (adk::ThermalRadiantEnvelope& value)
            {
                value.radiant.calibrationRevision = 0;
            },
            [] (adk::ThermalRadiantEnvelope& value)
            {
                value.thermistor.sequence = 0;
            },
            [] (adk::ThermalRadiantEnvelope& value)
            {
                value.digitalTemperature.state =
                    static_cast<adk::ThresholdState> (0xff);
            },
            [] (adk::ThermalRadiantEnvelope& value)
            {
                value.radiant.status = static_cast<adk::StatusCode> (0xff);
            },
            [] (adk::ThermalRadiantEnvelope& value)
            {
                value.digitalTemperature.sourceId = value.thermistor.sourceId;
            },
            [] (adk::ThermalRadiantEnvelope& value)
            {
                value.radiant.sourceId = value.digitalTemperature.sourceId;
            },
            [] (adk::ThermalRadiantEnvelope& value)
            {
                value.radiant.sourceId = value.thermistor.sourceId;
            }};
        for (Mutator mutate : invalid)
        {
            adk::ThermalRadiantEnvelope input = envelope (101, 2);
            mutate                                       (input);
            requireRejectedAtomically                    (policy, adk::TimePoint (101), input,
                                       adk::StatusCode::InvalidArgument,
                                       "malformed copied evidence rejects atomically");
        }

        adk::ThermalRadiantEnvelope changed = envelope (101, 2);
        changed.thermistor.sourceId         = 4;
        requireRejectedAtomically (policy, adk::TimePoint (101), changed,
                                   adk::StatusCode::InvalidArgument,
                                   "thermistor source identity cannot drift");
        changed                                          = envelope (101, 2);
        changed.digitalTemperature.configurationRevision = 99;
        requireRejectedAtomically (policy, adk::TimePoint (101), changed,
                                   adk::StatusCode::InvalidArgument,
                                   "digital configuration identity cannot drift");
        changed                             = envelope (101, 2);
        changed.radiant.calibrationRevision = 99;
        requireRejectedAtomically (policy, adk::TimePoint (101), changed,
                                   adk::StatusCode::InvalidArgument,
                                   "radiant calibration identity cannot drift");
    }

    void testThermalThresholdsExtremesAndCategoricalMapping ()
    {
        const struct
        {
            int32_t             value;
            uint32_t            uncertainty;
            adk::ThresholdState digital;
            adk::ThermalQuality quality;
            bool                hazard;
        } cases[] = {{INT32_MIN, UINT32_MAX, adk::ThresholdState::Below,
                      adk::ThermalQuality::Disagreement, true},
                     {INT32_MAX, UINT32_MAX, adk::ThresholdState::Below,
                      adk::ThermalQuality::Disagreement, true},
                     {18999, 1000, adk::ThresholdState::Below,
                      adk::ThermalQuality::Normal, false},
                     {19000, 1000, adk::ThresholdState::Below,
                      adk::ThermalQuality::Disagreement, false},
                     {20000, 0, adk::ThresholdState::Below,
                      adk::ThermalQuality::Disagreement, false},
                     {28999, 1000, adk::ThresholdState::Below,
                      adk::ThermalQuality::Disagreement, false},
                     {29000, 1000, adk::ThresholdState::Below,
                      adk::ThermalQuality::Disagreement, true},
                     {30000, 0, adk::ThresholdState::Below,
                      adk::ThermalQuality::Disagreement, true},
                     {INT32_MIN, 0, adk::ThresholdState::AtOrAbove,
                      adk::ThermalQuality::Alarm, true},
                     {15000, 0, adk::ThresholdState::AtOrAbove,
                      adk::ThermalQuality::Alarm, true},
                     {25000, 0, adk::ThresholdState::AtOrAbove,
                      adk::ThermalQuality::Alarm, true},
                     {INT32_MAX, 0, adk::ThresholdState::AtOrAbove,
                      adk::ThermalQuality::Alarm, true}};

        uint32_t sequence = 1;
        for (const auto& fixture : cases)
        {
            adk::ThermalRadiantObservationPolicy policy (config ());
            require                                     (policy.initialize ().ok (),
                     "thermal boundary fixture initializes");
            adk::ThermalRadiantEnvelope input        = envelope (100, sequence);
            input.thermistor.milliCelsius            = fixture.value;
            input.thermistor.uncertaintyMilliCelsius = fixture.uncertainty;
            input.digitalTemperature.state           = fixture.digital;
            require (policy.update (adk::TimePoint (100), input).ok (),
                     "signed extreme and threshold fixture accepts");
            require (policy.snapshot ().thermalQuality == fixture.quality &&
                         policy.snapshot ().thermalHazard == fixture.hazard,
                     "widened thermal threshold and categorical mapping are exact");
            ++sequence;
        }
    }

    void testThermalQualityPrecedenceAndIndependentAges ()
    {
        const struct
        {
            bool                thermSaturated;
            bool                digitalSaturated;
            uint32_t            thermAge;
            uint32_t            digitalAge;
            adk::StatusCode     thermStatus;
            adk::StatusCode     digitalStatus;
            adk::ThermalQuality quality;
            adk::StatusCode     status;
        } cases[] = {{false, false, 0, 0, adk::StatusCode::HardwareFailure,
                      adk::StatusCode::Ok, adk::ThermalQuality::ProducerFault,
                      adk::StatusCode::HardwareFailure},
                     {true, true, 101, 101, adk::StatusCode::Ok, adk::StatusCode::Ok,
                      adk::ThermalQuality::Saturated, adk::StatusCode::Ok},
                     {false, false, 101, 0, adk::StatusCode::Ok, adk::StatusCode::Ok,
                      adk::ThermalQuality::Stale, adk::StatusCode::Ok},
                     {false, false, 0, 101, adk::StatusCode::Ok, adk::StatusCode::Ok,
                      adk::ThermalQuality::Stale, adk::StatusCode::Ok},
                     {false, false, 100, 100, adk::StatusCode::Ok, adk::StatusCode::Ok,
                      adk::ThermalQuality::Normal, adk::StatusCode::Ok}};

        uint32_t sequence = 1;
        for (const auto& fixture : cases)
        {
            adk::ThermalRadiantObservationPolicy policy (config ());
            require                                     (policy.initialize ().ok (),
                     "thermal precedence fixture initializes");
            adk::ThermalRadiantEnvelope input = envelope       (200, sequence);
            input.thermistor.observedAt       = adk::TimePoint (200 - fixture.thermAge);
            input.digitalTemperature.observedAt =
                adk::TimePoint (200 - fixture.digitalAge);
            input.radiant.observedAt           = adk::TimePoint (200);
            input.thermistor.saturated         = fixture.thermSaturated;
            input.digitalTemperature.saturated = fixture.digitalSaturated;
            input.thermistor.status            = fixture.thermStatus;
            input.digitalTemperature.status    = fixture.digitalStatus;
            require (policy.update (adk::TimePoint (200), input).error () ==
                         fixture.status,
                     "thermal unhealthy evidence is accepted with exact status");
            const adk::ThermalRadiantObservation result = policy.snapshot ();
            require                                                       (result.thermalQuality == fixture.quality &&
                         result.thermistorAge == adk::Duration (fixture.thermAge) &&
                         result.digitalTemperatureAge ==
                             adk::Duration (fixture.digitalAge),
                     "thermal precedence and role-specific ages are exact");
            ++sequence;
        }
    }

    void testRadiantPulseSustainedResetAndRollover ()
    {
        adk::ThermalRadiantObservationPolicy policy (config ());
        require                                     (policy.initialize ().ok (), "radiant fixture initializes");

        adk::ThermalRadiantEnvelope input = envelope (100, 1);
        input.radiant.state               = adk::ThresholdState::AtOrAbove;
        require (policy.update (adk::TimePoint (100), input).ok () &&
                     policy.snapshot ().radiantQuality ==
                         adk::RadiantQuality::AbruptChange &&
                     policy.snapshot ().radiantHazard,
                 "inactive-to-active edge starts abrupt candidate");

        const adk::ThermalRadiantObservation duplicate = policy.snapshot ();
        require                                                          (policy.update (adk::TimePoint (100), input).ok () &&
                     observationEqual (duplicate, policy.snapshot ()),
                 "byte-identical duplicate cannot age or extend candidate");

        input               = envelope (149, 2);
        input.radiant.state = adk::ThresholdState::AtOrAbove;
        require (policy.update (adk::TimePoint (149), input).ok () &&
                     policy.snapshot ().radiantQuality ==
                         adk::RadiantQuality::AbruptChange,
                 "active duration immediately below sustained remains abrupt");
        input               = envelope (150, 3);
        input.radiant.state = adk::ThresholdState::AtOrAbove;
        require (policy.update (adk::TimePoint (150), input).ok () &&
                     policy.snapshot ().radiantQuality ==
                         adk::RadiantQuality::Sustained,
                 "sustained equality is inclusive");
        input               = envelope (151, 4);
        input.radiant.state = adk::ThresholdState::Below;
        require (policy.update (adk::TimePoint (151), input).ok () &&
                     policy.snapshot ().radiantQuality ==
                         adk::RadiantQuality::Sustained &&
                     policy.snapshot ().radiantHazard,
                 "once sustained, a prompt inactive edge cannot recast the event as a "
                 "pulse");

        policy.reset                   ();
        input               = envelope (200, 1);
        input.radiant.state = adk::ThresholdState::AtOrAbove;
        require (policy.update (adk::TimePoint (200), input).ok (),
                 "short pulse starts");
        input               = envelope (220, 2);
        input.radiant.state = adk::ThresholdState::Below;
        require (policy.update (adk::TimePoint (220), input).ok () &&
                     policy.snapshot ().radiantQuality ==
                         adk::RadiantQuality::AbruptChange &&
                     !policy.snapshot ().radiantHazard,
                 "completed pulse equality is abrupt and nonhazardous");
        adk::ThermalRadiantEnvelope companionAdvance = input;
        companionAdvance.thermistor.sequence         = 3;
        companionAdvance.thermistor.observedAt       = adk::TimePoint (221);
        require                                                       (policy.update (adk::TimePoint (221), companionAdvance).ok () &&
                     policy.snapshot ().radiantQuality ==
                         adk::RadiantQuality::AbruptChange &&
                     !policy.snapshot ().radiantHazard,
                 "unchanged radiant evidence retains completed pulse classification");

        policy.reset                   ();
        input               = envelope (300, 1);
        input.radiant.state = adk::ThresholdState::AtOrAbove;
        require (policy.update (adk::TimePoint (300), input).ok (),
                 "long pulse starts");
        input               = envelope (321, 2);
        input.radiant.state = adk::ThresholdState::Below;
        require (policy.update (adk::TimePoint (321), input).ok () &&
                     policy.snapshot ().radiantQuality ==
                         adk::RadiantQuality::Sustained &&
                     policy.snapshot ().radiantHazard,
                 "completed pulse above maximum is conservatively sustained");

        policy.reset                   ();
        input               = envelope (UINT32_MAX - 20U, 1);
        input.radiant.state = adk::ThresholdState::AtOrAbove;
        require (policy.update (adk::TimePoint (UINT32_MAX - 20U), input).ok (),
                 "rollover candidate starts");
        input               = envelope (29, 2);
        input.radiant.state = adk::ThresholdState::AtOrAbove;
        require (policy.update (adk::TimePoint (29), input).ok () &&
                     policy.snapshot ().radiantQuality ==
                         adk::RadiantQuality::Sustained,
                 "radiant duration crosses rollover exactly");

        input                   = envelope (30, 3);
        input.radiant.saturated = true;
        require (policy.update (adk::TimePoint (30), input).ok () &&
                     policy.snapshot ().radiantQuality ==
                         adk::RadiantQuality::SaturatedAmbient &&
                     !policy.snapshot ().radiantHazard,
                 "ambient saturation clears active candidate");
        input               = envelope (31, 4);
        input.radiant.state = adk::ThresholdState::AtOrAbove;
        require (policy.update (adk::TimePoint (31), input).ok () &&
                     policy.snapshot ().radiantQuality ==
                         adk::RadiantQuality::AbruptChange,
                 "post-saturation active evidence begins a fresh candidate");

        policy.reset                   ();
        input               = envelope (1000, 1);
        input.radiant.state = adk::ThresholdState::Below;
        require (policy.update (adk::TimePoint (1000), input).ok () &&
                     policy.snapshot ().radiantQuality == adk::RadiantQuality::Quiet,
                 "reset clears the candidate and publishes quiet on inactive input");
    }

    void testRadiantPrecedence ()
    {
        const struct
        {
            adk::StatusCode     producer;
            bool                saturated;
            uint32_t            age;
            adk::ThresholdState state;
            adk::RadiantQuality quality;
            adk::StatusCode     status;
        } cases[] = {{adk::StatusCode::HardwareFailure, true, 101,
                      adk::ThresholdState::AtOrAbove,
                      adk::RadiantQuality::ProducerFault,
                      adk::StatusCode::HardwareFailure},
                     {adk::StatusCode::Ok, true, 101, adk::ThresholdState::AtOrAbove,
                      adk::RadiantQuality::SaturatedAmbient, adk::StatusCode::Ok},
                     {adk::StatusCode::Ok, false, 101, adk::ThresholdState::AtOrAbove,
                      adk::RadiantQuality::Stale, adk::StatusCode::Ok},
                     {adk::StatusCode::Ok, false, 100, adk::ThresholdState::Below,
                      adk::RadiantQuality::Quiet, adk::StatusCode::Ok}};

        uint32_t sequence = 1;
        for (const auto& fixture : cases)
        {
            adk::ThermalRadiantObservationPolicy policy (config ());
            require                                     (policy.initialize ().ok (),
                     "radiant precedence fixture initializes");
            adk::ThermalRadiantEnvelope input = envelope       (200, sequence);
            input.radiant.observedAt          = adk::TimePoint (200 - fixture.age);
            input.radiant.status              = fixture.producer;
            input.radiant.saturated           = fixture.saturated;
            input.radiant.state               = fixture.state;
            require (policy.update (adk::TimePoint (200), input).error () ==
                         fixture.status,
                     "radiant precedence envelope returns exact status");
            require (policy.snapshot ().radiantQuality == fixture.quality &&
                         policy.snapshot ().radiantAge == adk::Duration (fixture.age),
                     "radiant precedence and independent age are exact");
            ++sequence;
        }
    }

    void testOrderingDuplicateAtomicityAndExhaustion ()
    {
        adk::ThermalRadiantObservationPolicy policy        (config ());
        require                                            (policy.initialize ().ok (), "ordering fixture initializes");
        const adk::ThermalRadiantEnvelope first = envelope (100, 1);
        require                                            (policy.update (adk::TimePoint (100), first).ok (),
                 "ordering fixture seeds");
        const adk::ThermalRadiantObservation firstSnapshot = policy.snapshot ();
        require                                                              (policy.update (adk::TimePoint (100), first).ok () &&
                     observationEqual (firstSnapshot, policy.snapshot ()),
                 "equal-time byte-identical envelope is idempotent");
        require (policy.update (adk::TimePoint (101), first).ok () &&
                     observationEqual (firstSnapshot, policy.snapshot ()),
                 "forward-time duplicate is idempotent without aging");
        requireRejectedAtomically (
            policy, adk::TimePoint (99), first, adk::StatusCode::InvalidArgument,
            "byte-identical envelope cannot conceal backward policy time");
        requireRejectedAtomically (
            policy, adk::TimePoint (100 + 0x80000000UL), first,
            adk::StatusCode::InvalidArgument,
            "byte-identical envelope cannot conceal half-range policy time");
        adk::ThermalRadiantEnvelope newerAtEqualPolicyTime = envelope (100, 2);
        requireRejectedAtomically                                     (
            policy, adk::TimePoint (100), newerAtEqualPolicyTime,
            adk::StatusCode::InvalidArgument,
            "equal policy time accepts only byte-identical evidence");

        adk::ThermalRadiantEnvelope changed = first;
        changed.radiant.raw                 = 201;
        requireRejectedAtomically (policy, adk::TimePoint (100), changed,
                                   adk::StatusCode::InvalidArgument,
                                   "changed whole-envelope duplicate rejects");
        changed                             = envelope (101, 2);
        changed.digitalTemperature.sequence = 1;
        requireRejectedAtomically (policy, adk::TimePoint (101), changed,
                                   adk::StatusCode::InvalidArgument,
                                   "partial duplicate rejects");
        changed = envelope        (101, UINT32_MAX);
        requireRejectedAtomically (policy, adk::TimePoint (101), changed,
                                   adk::StatusCode::InvalidArgument,
                                   "modular sequence regression rejects");
        changed = envelope        (101, 1U + 0x80000000UL);
        requireRejectedAtomically (policy, adk::TimePoint (101), changed,
                                   adk::StatusCode::InvalidArgument,
                                   "exact half-range sequence ambiguity rejects");
        changed = envelope        (99, 2);
        requireRejectedAtomically (policy, adk::TimePoint (101), changed,
                                   adk::StatusCode::InvalidArgument,
                                   "per-source observed time regression rejects");
        changed = envelope        (101, 2);
        requireRejectedAtomically (policy, adk::TimePoint (99), changed,
                                   adk::StatusCode::InvalidArgument,
                                   "future source time rejects");
        requireRejectedAtomically (policy, adk::TimePoint (100 + 0x80000000UL),
                                   envelope (100, 2), adk::StatusCode::InvalidArgument,
                                   "exact half-range policy time rejects");

        adk::ThermalRadiantObservationPolicy exhausted (config ());
        require                                        (exhausted.initialize ().ok (),
                 "sequence exhaustion fixture initializes");
        require (exhausted
                     .update (adk::TimePoint (UINT32_MAX - 1U),
                              envelope (UINT32_MAX - 1U, UINT32_MAX))
                     .ok (),
                 "maximum sequence is accepted once");
        requireRejectedAtomically (exhausted, adk::TimePoint (1), envelope (1, 1),
                                   adk::StatusCode::CapacityExceeded,
                                   "sequence exhaustion rejects wrapping envelope");
        const adk::ThermalRadiantObservation exhaustedSnapshot = exhausted.snapshot ();
        require                                                                     (exhausted
                         .update (adk::TimePoint (UINT32_MAX - 1U),
                                  envelope (UINT32_MAX - 1U, UINT32_MAX))
                         .ok () &&
                     observationEqual (exhaustedSnapshot, exhausted.snapshot ()),
                 "exact terminal-sequence duplicate remains idempotent");
        adk::ThermalRadiantEnvelope partialAdvance = envelope (UINT32_MAX, UINT32_MAX);
        partialAdvance.digitalTemperature.sequence = 1;
        partialAdvance.radiant.sequence            = 1;
        requireRejectedAtomically (
            exhausted, adk::TimePoint (UINT32_MAX), partialAdvance,
            adk::StatusCode::CapacityExceeded,
            "changed whole-envelope update rejects after all sources exhaust");
        exhausted.reset ();
        require         (exhausted.update (adk::TimePoint (1), envelope (1, 1)).ok (),
                 "reset explicitly begins a new sequence epoch");

        adk::ThermalRadiantObservationPolicy independent (config ());
        require                                          (independent.initialize ().ok (),
                 "independent exhaustion fixture initializes");
        adk::ThermalRadiantEnvelope oneExhausted = envelope (UINT32_MAX - 1U, 1);
        oneExhausted.thermistor.sequence         = UINT32_MAX;
        require (
            independent.update (adk::TimePoint (UINT32_MAX - 1U), oneExhausted).ok (),
            "one source can reach terminal sequence independently");
        adk::ThermalRadiantEnvelope companionsAdvance   = oneExhausted;
        companionsAdvance.digitalTemperature.sequence   = 2;
        companionsAdvance.digitalTemperature.observedAt = adk::TimePoint (UINT32_MAX);
        companionsAdvance.radiant.sequence              = 2;
        companionsAdvance.radiant.observedAt            = adk::TimePoint (UINT32_MAX);
        require                                                          (
            independent.update (adk::TimePoint (UINT32_MAX), companionsAdvance).ok (),
            "terminal source remains byte-identical while independent sources advance");
        adk::ThermalRadiantEnvelope exhaustedChanges = companionsAdvance;
        exhaustedChanges.thermistor.milliCelsius++;
        requireRejectedAtomically (
            independent, adk::TimePoint (0), exhaustedChanges,
            adk::StatusCode::CapacityExceeded,
            "changed evidence from the exhausted source is rejected atomically");
    }

    using Witness = std::array<uint8_t, 76>;

    void append16 (Witness& witness, std::size_t& offset, uint16_t value)
    {
        witness[offset++] = static_cast<uint8_t> (value);
        witness[offset++] = static_cast<uint8_t> (value >> 8U);
    }

    void append32 (Witness& witness, std::size_t& offset, uint32_t value)
    {
        witness[offset++] = static_cast<uint8_t> (value);
        witness[offset++] = static_cast<uint8_t> (value >> 8U);
        witness[offset++] = static_cast<uint8_t> (value >> 16U);
        witness[offset++] = static_cast<uint8_t> (value >> 24U);
    }

    void appendThermal (Witness& witness, std::size_t& offset,
                        const adk::ConvertedThermalSample& sample)
    {
        witness[offset++] = sample.sourceId;
        append16 (witness, offset, sample.configurationRevision);
        append16 (witness, offset, sample.calibrationRevision);
        append32 (witness, offset, sample.sequence);
        append32 (witness, offset, sample.observedAt.milliseconds ());
        append32 (witness, offset, static_cast<uint32_t> (sample.milliCelsius));
        append32 (witness, offset, sample.uncertaintyMilliCelsius);
        witness[offset++] = sample.saturated ? 1U : 0U;
        witness[offset++] = static_cast<uint8_t> (sample.status.error ());
    }

    void appendCategorical (Witness& witness, std::size_t& offset,
                            const adk::CategoricalThresholdSample& sample)
    {
        witness[offset++] = sample.sourceId;
        append16 (witness, offset, sample.configurationRevision);
        append16 (witness, offset, sample.calibrationRevision);
        append32 (witness, offset, sample.sequence);
        append32 (witness, offset, sample.observedAt.milliseconds ());
        append16 (witness, offset, sample.raw);
        witness[offset++] = static_cast<uint8_t> (sample.state);
        witness[offset++] = sample.saturated ? 1U : 0U;
        witness[offset++] = static_cast<uint8_t> (sample.status.error ());
    }

    Witness canonicalWitness (const adk::ThermalRadiantObservation& observation)
    {
        Witness     witness{};
        std::size_t offset = 0;
        appendThermal     (witness, offset, observation.envelope.thermistor);
        appendCategorical (witness, offset, observation.envelope.digitalTemperature);
        appendCategorical (witness, offset, observation.envelope.radiant);
        witness[offset++] = static_cast<uint8_t> (observation.thermalQuality);
        witness[offset++] = static_cast<uint8_t> (observation.radiantQuality);
        append32 (witness, offset, observation.thermistorAge.milliseconds ());
        append32 (witness, offset, observation.digitalTemperatureAge.milliseconds ());
        append32 (witness, offset, observation.radiantAge.milliseconds ());
        witness[offset++] = observation.thermalHazard ? 1U : 0U;
        witness[offset++] = observation.radiantHazard ? 1U : 0U;
        witness[offset++] = static_cast<uint8_t> (observation.status.error ());
        require                                                            (offset == witness.size (), "canonical witness size is exact");
        return witness;
    }

    void testEveryProducerStatusAndCanonicalReplay ()
    {
        const adk::StatusCode statuses[] = {
            adk::StatusCode::InvalidArgument,   adk::StatusCode::InvalidConfiguration,
            adk::StatusCode::InvalidPin,        adk::StatusCode::Unsupported,
            adk::StatusCode::ResourceBusy,      adk::StatusCode::NotInitialized,
            adk::StatusCode::CapacityExceeded,  adk::StatusCode::Timeout,
            adk::StatusCode::InternalInvariant, adk::StatusCode::HardwareFailure};
        for (adk::StatusCode status : statuses)
        {
            adk::ThermalRadiantObservationPolicy thermalPolicy (config ());
            require                                            (thermalPolicy.initialize ().ok (),
                     "thermal producer fixture initializes");
            adk::ThermalRadiantEnvelope thermalInput = envelope ();
            thermalInput.thermistor.status           = status;
            require (
                thermalPolicy.update (adk::TimePoint (100), thermalInput).error () ==
                        status &&
                    thermalPolicy.snapshot ().thermalQuality ==
                        adk::ThermalQuality::ProducerFault,
                "every thermistor producer status is preserved");

            adk::ThermalRadiantObservationPolicy digitalPolicy (config ());
            require                                            (digitalPolicy.initialize ().ok (),
                     "digital producer fixture initializes");
            adk::ThermalRadiantEnvelope digitalInput = envelope ();
            digitalInput.digitalTemperature.status   = status;
            require (
                digitalPolicy.update (adk::TimePoint (100), digitalInput).error () ==
                        status &&
                    digitalPolicy.snapshot ().thermalQuality ==
                        adk::ThermalQuality::ProducerFault,
                "every categorical thermal producer status is preserved");

            adk::ThermalRadiantObservationPolicy radiantPolicy (config ());
            require                                            (radiantPolicy.initialize ().ok (),
                     "radiant producer fixture initializes");
            adk::ThermalRadiantEnvelope radiantInput = envelope ();
            radiantInput.radiant.status              = status;
            require (
                radiantPolicy.update (adk::TimePoint (100), radiantInput).error () ==
                        status &&
                    radiantPolicy.snapshot ().radiantQuality ==
                        adk::RadiantQuality::ProducerFault,
                "every radiant producer status is preserved");
        }

        adk::ThermalRadiantObservationPolicy collisions (config ());
        require                                         (collisions.initialize ().ok (),
                 "producer collision fixture initializes");
        adk::ThermalRadiantEnvelope collisionInput = envelope ();
        collisionInput.thermistor.status = adk::StatusCode::InvalidConfiguration;
        collisionInput.digitalTemperature.status = adk::StatusCode::Timeout;
        collisionInput.radiant.status            = adk::StatusCode::HardwareFailure;
        require (collisions.update (adk::TimePoint (100), collisionInput).error () ==
                         adk::StatusCode::InvalidConfiguration &&
                     collisions.snapshot ().thermalQuality ==
                         adk::ThermalQuality::ProducerFault &&
                     collisions.snapshot ().radiantQuality ==
                         adk::RadiantQuality::ProducerFault,
                 "thermistor producer status wins a three-source collision");

        collisionInput                           = envelope (101, 2);
        collisionInput.digitalTemperature.status = adk::StatusCode::Timeout;
        collisionInput.radiant.status            = adk::StatusCode::HardwareFailure;
        require (collisions.update (adk::TimePoint (101), collisionInput).error () ==
                         adk::StatusCode::Timeout &&
                     collisions.snapshot ().thermalQuality ==
                         adk::ThermalQuality::ProducerFault &&
                     collisions.snapshot ().radiantQuality ==
                         adk::RadiantQuality::ProducerFault,
                 "digital producer status wins when thermistor is healthy");

        for (uint8_t mask = 1; mask < 8; ++mask)
        {
            adk::ThermalRadiantObservationPolicy combination (config ());
            require                                          (combination.initialize ().ok (),
                     "producer combination fixture initializes");
            adk::ThermalRadiantEnvelope combined = envelope ();
            combined.thermistor.status = (mask & 1U) != 0U
                                             ? adk::StatusCode::InvalidConfiguration
                                             : adk::StatusCode::Ok;
            combined.digitalTemperature.status =
                (mask & 2U) != 0U ? adk::StatusCode::Timeout : adk::StatusCode::Ok;
            combined.radiant.status = (mask & 4U) != 0U
                                          ? adk::StatusCode::HardwareFailure
                                          : adk::StatusCode::Ok;
            const adk::StatusCode expectedStatus =
                (mask & 1U) != 0U   ? adk::StatusCode::InvalidConfiguration
                : (mask & 2U) != 0U ? adk::StatusCode::Timeout
                                    : adk::StatusCode::HardwareFailure;
            require (combination.update (adk::TimePoint (100), combined).error () ==
                             expectedStatus &&
                         combination.snapshot ().thermalQuality ==
                             ((mask & 3U) != 0U ? adk::ThermalQuality::ProducerFault
                                                : adk::ThermalQuality::Normal) &&
                         combination.snapshot ().radiantQuality ==
                             ((mask & 4U) != 0U ? adk::RadiantQuality::ProducerFault
                                                : adk::RadiantQuality::Quiet),
                     "every simultaneous producer-fault subset has fixed precedence");
        }

        adk::ThermalRadiantObservationPolicy left  (config ());
        adk::ThermalRadiantObservationPolicy right (config ());
        require                                    (left.initialize ().ok () && right.initialize ().ok (),
                 "replay policies initialize");
        const adk::ThermalRadiantEnvelope canonicalInput = envelope ();
        require                                                     (left.update (adk::TimePoint (100), canonicalInput).ok (),
                 "canonical replay input accepts");
        const Witness canonicalBytes = canonicalWitness (left.snapshot ());
        const Witness expected       = {
            0x01, 0x0b, 0x00, 0x15, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00,
            0x00, 0x00, 0x98, 0x3a, 0x00, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00,
            0x00, 0x02, 0x0c, 0x00, 0x16, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64,
            0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0d, 0x00,
            0x17, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0xc8,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        require (canonicalBytes == expected,
                 "canonical replay matches independently specified bytes");

        left.reset  ();
        right.reset ();
        for (uint32_t index = 0; index < 80; ++index)
        {
            const uint32_t              time  = UINT32_MAX - 20U + index;
            adk::ThermalRadiantEnvelope input = envelope (time, index + 1U);
            input.thermistor.milliCelsius  = static_cast<int32_t> (15000 + index * 250);
            input.digitalTemperature.state = index % 5U == 0U
                                                 ? adk::ThresholdState::AtOrAbove
                                                 : adk::ThresholdState::Below;
            input.radiant.state = index % 4U < 2U ? adk::ThresholdState::AtOrAbove
                                                  : adk::ThresholdState::Below;
            require (left.update (adk::TimePoint (time), input).ok () &&
                         right.update (adk::TimePoint (time), input).ok (),
                     "canonical replay trace accepts identically");
            require (observationEqual (left.snapshot (), right.snapshot ()),
                     "canonical replay trace is field-identical");
            const Witness leftBytes  = canonicalWitness (left.snapshot ());
            const Witness rightBytes = canonicalWitness (right.snapshot ());
            require                                     (std::memcmp (leftBytes.data (), rightBytes.data (),
                                  leftBytes.size ()) == 0,
                     "canonical replay trace is byte-stable");
        }
    }
} // namespace

int main ()
{
    testLifecycleAndConfiguration                      ();
    testStructuralValidationAndSourceIdentity          ();
    testThermalThresholdsExtremesAndCategoricalMapping ();
    testThermalQualityPrecedenceAndIndependentAges     ();
    testRadiantPulseSustainedResetAndRollover          ();
    testRadiantPrecedence                              ();
    testOrderingDuplicateAtomicityAndExhaustion        ();
    testEveryProducerStatusAndCanonicalReplay          ();
    std::cout << "thermal radiant observation tests passed\n";
}
