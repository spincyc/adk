#include <resistive_probe_observation.h>

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

    adk::ResistiveProbeConfig config ()
    {
        return {
            1023, 200, 800, 10, 20, 250, 750, adk::Duration (100), adk::Duration (20),
            200};
    }

    adk::ResistiveProbeSample sample (uint16_t energized = 500, uint16_t discharged = 5,
                                      uint32_t observedAt = 100, uint32_t sequence = 1,
                                      adk::Status status = adk::StatusCode::Ok)
    {
        return {7,
                11,
                13,
                sequence,
                adk::TimePoint (observedAt),
                energized,
                discharged,
                adk::Duration (10),
                adk::Duration (100),
                true,
                status};
    }

    bool sampleEqual (const adk::ResistiveProbeSample& left,
                      const adk::ResistiveProbeSample& right)
    {
        return left.sourceId == right.sourceId &&
               left.configurationRevision == right.configurationRevision &&
               left.calibrationRevision == right.calibrationRevision &&
               left.sequence == right.sequence && left.observedAt == right.observedAt &&
               left.energizedRaw == right.energizedRaw &&
               left.dischargedRaw == right.dischargedRaw &&
               left.excitationOnTime == right.excitationOnTime &&
               left.cycleTime == right.cycleTime &&
               left.excitationObservedOffAfterSample ==
                   right.excitationObservedOffAfterSample &&
               left.status == right.status;
    }

    bool observationEqual (const adk::ResistiveProbeObservation& left,
                           const adk::ResistiveProbeObservation& right)
    {
        return sampleEqual (left.sample, right.sample) &&
               left.normalizedPermille == right.normalizedPermille &&
               left.observedCycleDutyPermille == right.observedCycleDutyPermille &&
               left.quality == right.quality && left.age == right.age &&
               left.status == right.status;
    }

    using ObservationWitness = std::array<uint8_t, 37>;

    void append16 (ObservationWitness& witness, std::size_t& offset, uint16_t value)
    {
        witness[offset++] = static_cast<uint8_t> (value);
        witness[offset++] = static_cast<uint8_t> (value >> 8U);
    }

    void append32 (ObservationWitness& witness, std::size_t& offset, uint32_t value)
    {
        witness[offset++] = static_cast<uint8_t> (value);
        witness[offset++] = static_cast<uint8_t> (value >> 8U);
        witness[offset++] = static_cast<uint8_t> (value >> 16U);
        witness[offset++] = static_cast<uint8_t> (value >> 24U);
    }

    ObservationWitness
    canonicalWitness (const adk::ResistiveProbeObservation& observation)
    {
        ObservationWitness witness{};
        std::size_t        offset = 0;
        witness[offset++]         = observation.sample.sourceId;
        append16 (witness, offset, observation.sample.configurationRevision);
        append16 (witness, offset, observation.sample.calibrationRevision);
        append32 (witness, offset, observation.sample.sequence);
        append32 (witness, offset, observation.sample.observedAt.milliseconds ());
        append16 (witness, offset, observation.sample.energizedRaw);
        append16 (witness, offset, observation.sample.dischargedRaw);
        append32 (witness, offset, observation.sample.excitationOnTime.milliseconds ());
        append32 (witness, offset, observation.sample.cycleTime.milliseconds ());
        witness[offset++] =
            observation.sample.excitationObservedOffAfterSample ? 1U : 0U;
        witness[offset++] = static_cast<uint8_t> (observation.sample.status.error ());
        append16                                                                  (witness, offset, observation.normalizedPermille);
        append16                                                                  (witness, offset, observation.observedCycleDutyPermille);
        witness[offset++] = static_cast<uint8_t> (observation.quality);
        append32                                                           (witness, offset, observation.age.milliseconds ());
        witness[offset++] = static_cast<uint8_t> (observation.status.error ());
        require                                                            (offset == witness.size (), "canonical witness size is exact");
        return witness;
    }

    void requireRejectedAtomically (adk::ResistiveProbeObservationPolicy& policy,
                                    adk::TimePoint                        now,
                                    const adk::ResistiveProbeSample&      input,
                                    adk::StatusCode expected, const char* message)
    {
        const adk::ResistiveProbeObservation before = policy.snapshot ();
        require                                                       (policy.update (now, input).error () == expected, message);
        if (!observationEqual                                         (before, policy.snapshot ()))
        {
            std::cerr << "FAIL: rejected update is atomic after " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    void testLifecycleAndConfiguration ()
    {
        static_assert (
            !std::is_copy_constructible<adk::ResistiveProbeObservationPolicy>::value,
            "probe policy must not copy");
        static_assert (
            !std::is_move_constructible<adk::ResistiveProbeObservationPolicy>::value,
            "probe policy must not move");
        static_assert (
            std::is_trivially_destructible<adk::ResistiveProbeObservationPolicy>::value,
            "probe policy destruction must be inert");

        adk::ResistiveProbeObservationPolicy policy (config ());
        require                                     (!policy.initialized (), "construction is inert");
        require                                     (policy.snapshot ().quality == adk::ProbeQuality::Unqualified &&
                     policy.snapshot ().status.error () ==
                         adk::StatusCode::NotInitialized,
                 "construction has canonical unqualified snapshot");
        require (policy.update (adk::TimePoint (100), sample ()).error () ==
                     adk::StatusCode::NotInitialized,
                 "update before initialize rejects");
        require (policy.initialize ().ok (), "valid configuration initializes");
        require (policy.initialize ().ok (), "initialize is idempotent");
        require (policy.initialized (), "initialized state visible");
        require (policy.update (adk::TimePoint (100), sample ()).ok (),
                 "initialized policy accepts evidence");
        policy.reset ();
        require      (policy.initialized (), "reset preserves configuration");
        require      (policy.snapshot ().quality == adk::ProbeQuality::Unqualified &&
                     policy.snapshot ().status.error () ==
                         adk::StatusCode::NotInitialized,
                 "reset clears observation");
        require (policy.update (adk::TimePoint (100), sample ()).ok (),
                 "reset clears ordering history");

        adk::ResistiveProbeConfig largest = config         ();
        largest.maximumAge                = adk::Duration  (0x7fffffffUL);
        largest.maximumExcitationOnTime   = adk::Duration  (0x7fffffffUL);
        adk::ResistiveProbeObservationPolicy largestPolicy (largest);
        require                                            (largestPolicy.initialize ().ok (),
                 "largest wrap-safe durations initialize");

        using Mutator           = void (*) (adk::ResistiveProbeConfig&);
        const Mutator invalid[] = {
            [] (adk::ResistiveProbeConfig& value)
            {
                value.adcMaximum = 0;
            },
            [] (adk::ResistiveProbeConfig& value)
            {
                value.dryReference = value.wetReference;
            },
            [] (adk::ResistiveProbeConfig& value)
            {
                value.dryReference = value.adcMaximum + 1U;
            },
            [] (adk::ResistiveProbeConfig& value)
            {
                value.wetReference = value.adcMaximum + 1U;
            },
            [] (adk::ResistiveProbeConfig& value)
            {
                value.disconnectedMaximum = value.adcMaximum + 1U;
            },
            [] (adk::ResistiveProbeConfig& value)
            {
                value.dischargedMaximum = value.adcMaximum + 1U;
            },
            [] (adk::ResistiveProbeConfig& value)
            {
                value.dampThresholdPermille = 1001;
            },
            [] (adk::ResistiveProbeConfig& value)
            {
                value.wetThresholdPermille = 1001;
            },
            [] (adk::ResistiveProbeConfig& value)
            {
                value.dampThresholdPermille = value.wetThresholdPermille + 1U;
            },
            [] (adk::ResistiveProbeConfig& value)
            {
                value.maximumAge = adk::Duration (0);
            },
            [] (adk::ResistiveProbeConfig& value)
            {
                value.maximumAge = adk::Duration (0x80000000UL);
            },
            [] (adk::ResistiveProbeConfig& value)
            {
                value.maximumExcitationOnTime = adk::Duration (0);
            },
            [] (adk::ResistiveProbeConfig& value)
            {
                value.maximumExcitationOnTime = adk::Duration (0x80000000UL);
            },
            [] (adk::ResistiveProbeConfig& value)
            {
                value.maximumDutyPermille = 1001;
            }};
        for (Mutator mutate : invalid)
        {
            adk::ResistiveProbeConfig rejected = config    ();
            mutate                                         (rejected);
            adk::ResistiveProbeObservationPolicy candidate (rejected);
            require                                        (candidate.initialize ().error () ==
                             adk::StatusCode::InvalidConfiguration &&
                         !candidate.initialized (),
                     "invalid configuration remains inert");
        }
    }

    void testStructuralDomainAndAtomicity ()
    {
        adk::ResistiveProbeObservationPolicy policy (config ());
        require                                     (policy.initialize ().ok (), "domain fixture initializes");
        require                                     (policy.update (adk::TimePoint (100), sample ()).ok (),
                 "domain fixture seeds history");

        using Mutator           = void (*) (adk::ResistiveProbeSample&);
        const Mutator invalid[] = {[] (adk::ResistiveProbeSample& value)
                                   {
                                       value.sourceId = 0;
                                   },
                                   [] (adk::ResistiveProbeSample& value)
                                   {
                                       value.configurationRevision = 0;
                                   },
                                   [] (adk::ResistiveProbeSample& value)
                                   {
                                       value.calibrationRevision = 0;
                                   },
                                   [] (adk::ResistiveProbeSample& value)
                                   {
                                       value.sequence = 0;
                                   },
                                   [] (adk::ResistiveProbeSample& value)
                                   {
                                       value.energizedRaw = 1024;
                                   },
                                   [] (adk::ResistiveProbeSample& value)
                                   {
                                       value.dischargedRaw = 1024;
                                   },
                                   [] (adk::ResistiveProbeSample& value)
                                   {
                                       value.cycleTime = adk::Duration (0);
                                   },
                                   [] (adk::ResistiveProbeSample& value)
                                   {
                                       value.cycleTime = adk::Duration (0x80000000UL);
                                   },
                                   [] (adk::ResistiveProbeSample& value)
                                   {
                                       value.excitationOnTime = adk::Duration (101);
                                   },
                                   [] (adk::ResistiveProbeSample& value)
                                   {
                                       value.status =
                                           static_cast<adk::StatusCode> (0xff);
                                   }};
        for (Mutator mutate : invalid)
        {
            adk::ResistiveProbeSample input = sample (500, 5, 101, 2);
            mutate                                   (input);
            requireRejectedAtomically                (policy, adk::TimePoint (101), input,
                                       adk::StatusCode::InvalidArgument,
                                       "malformed producer evidence rejects");
        }

        adk::ResistiveProbeSample changed = sample (500, 5, 101, 2);
        changed.sourceId                  = 8;
        requireRejectedAtomically (policy, adk::TimePoint (101), changed,
                                   adk::StatusCode::InvalidArgument,
                                   "source identity cannot drift");
        changed                       = sample (500, 5, 101, 2);
        changed.configurationRevision = 12;
        requireRejectedAtomically (policy, adk::TimePoint (101), changed,
                                   adk::StatusCode::InvalidArgument,
                                   "configuration revision cannot drift");
        changed                     = sample (500, 5, 101, 2);
        changed.calibrationRevision = 14;
        requireRejectedAtomically (policy, adk::TimePoint (101), changed,
                                   adk::StatusCode::InvalidArgument,
                                   "calibration revision cannot drift");
    }

    void testCalibrationSlopesAndThresholds ()
    {
        const struct
        {
            uint16_t          raw;
            uint16_t          normalized;
            adk::ProbeQuality quality;
        } increasing[] = {
            {0, 0, adk::ProbeQuality::Dry},      {199, 0, adk::ProbeQuality::Dry},
            {200, 0, adk::ProbeQuality::Dry},    {349, 248, adk::ProbeQuality::Dry},
            {350, 250, adk::ProbeQuality::Damp}, {649, 748, adk::ProbeQuality::Damp},
            {650, 750, adk::ProbeQuality::Wet},  {800, 1000, adk::ProbeQuality::Wet},
            {801, 1000, adk::ProbeQuality::Wet}};
        uint32_t                             sequence = 1;
        adk::ResistiveProbeObservationPolicy policy (config ());
        require                                     (policy.initialize ().ok (), "increasing slope initializes");
        for (const auto& fixture : increasing)
        {
            adk::ResistiveProbeSample input =
                sample (fixture.raw, 11, 100 + sequence, sequence);
            require (policy.update (adk::TimePoint (100 + sequence), input).ok (),
                     "increasing-slope endpoint accepts");
            require (policy.snapshot ().normalizedPermille == fixture.normalized &&
                         policy.snapshot ().quality == fixture.quality,
                     "increasing slope normalization and threshold exact");
            ++sequence;
        }

        adk::ResistiveProbeConfig descending = config ();
        descending.dryReference              = 800;
        descending.wetReference              = 200;
        adk::ResistiveProbeObservationPolicy reverse (descending);
        require                                      (reverse.initialize ().ok (), "descending slope initializes");
        const struct
        {
            uint16_t raw;
            uint16_t normalized;
        } decreasing[] = {{1022, 0},  {800, 0},    {650, 250},
                          {350, 750}, {200, 1000}, {199, 1000}};
        sequence       = 1;
        for (const auto& fixture : decreasing)
        {
            require (reverse
                         .update (adk::TimePoint (sequence),
                                  sample (fixture.raw, 11, sequence, sequence))
                         .ok (),
                     "descending-slope endpoint accepts");
            require (reverse.snapshot ().normalizedPermille == fixture.normalized,
                     "descending slope is monotonic and clamped");
            ++sequence;
        }
    }

    void testEveryAdcCodeForBothSlopes ()
    {
        for (uint16_t raw = 0; raw <= 1023; ++raw)
        {
            const uint16_t increasing =
                raw <= 200   ? 0
                : raw >= 800 ? 1000
                             : static_cast<uint16_t> (
                                   (static_cast<uint32_t> (raw - 200) * 1000U) / 600U);
            const uint16_t decreasing =
                raw >= 800   ? 0
                : raw <= 200 ? 1000
                             : static_cast<uint16_t> (
                                   (static_cast<uint32_t> (800 - raw) * 1000U) / 600U);

            adk::ResistiveProbeObservationPolicy forward                (config ());
            adk::ResistiveProbeConfig            reverseConfig = config ();
            reverseConfig.dryReference                         = 800;
            reverseConfig.wetReference                         = 200;
            adk::ResistiveProbeObservationPolicy reverse (reverseConfig);
            require                                      (forward.initialize ().ok () && reverse.initialize ().ok (),
                     "exhaustive ADC fixtures initialize");
            require (
                forward.update (adk::TimePoint (1), sample (raw, 11, 1, 1)).ok () &&
                    reverse.update (adk::TimePoint (1), sample (raw, 11, 1, 1)).ok (),
                "every ADC code accepts for both calibration slopes");
            require (forward.snapshot ().normalizedPermille == increasing &&
                         reverse.snapshot ().normalizedPermille == decreasing,
                     "every ADC code normalizes exactly for both slopes");
            require ((raw == 1023) == (forward.snapshot ().quality ==
                                       adk::ProbeQuality::Saturated) &&
                         (raw == 1023) == (reverse.snapshot ().quality ==
                                           adk::ProbeQuality::Saturated),
                     "only declared full scale has saturation precedence");
        }
    }

    void testContaminationDriftAndElectricalFaultTraces ()
    {
        adk::ResistiveProbeObservationPolicy drift (config ());
        require                                    (drift.initialize ().ok (), "contamination-drift fixture initializes");
        const uint16_t contaminationDriftRaw[] = {220, 280, 340, 400, 460,
                                                  520, 580, 640, 700, 760};
        uint16_t       previous                = 0;
        for (uint32_t index = 0;
             index < sizeof contaminationDriftRaw / sizeof contaminationDriftRaw[0];
             ++index)
        {
            const uint32_t time = 100U + index;
            require (
                drift
                    .update (adk::TimePoint (time),
                             sample (contaminationDriftRaw[index], 5, time, index + 1U))
                    .ok (),
                "contamination-drift sample accepts");
            require (drift.snapshot ().normalizedPermille >= previous,
                     "contamination drift remains monotonic");
            previous = drift.snapshot ().normalizedPermille;
        }
        require (drift.snapshot ().quality == adk::ProbeQuality::Wet,
                 "contamination drift crosses into wet without a depth claim");

        adk::ResistiveProbeObservationPolicy stuckHigh (config ());
        require                                        (stuckHigh.initialize ().ok (), "stuck-high fixture initializes");
        adk::ResistiveProbeSample stuck = sample       (700, 700);
        require                                        (stuckHigh.update (adk::TimePoint (100), stuck).ok () &&
                     stuckHigh.snapshot ().quality ==
                         adk::ProbeQuality::ExcitationFault,
                 "named stuck-high discharge trace faults");

        adk::ResistiveProbeObservationPolicy backfeed (config ());
        require                                       (backfeed.initialize ().ok (), "backfeed fixture initializes");
        adk::ResistiveProbeSample fed = sample        (500, 21);
        require                                       (backfeed.update (adk::TimePoint (100), fed).ok () &&
                     backfeed.snapshot ().quality == adk::ProbeQuality::ExcitationFault,
                 "named backfeed-above-discharge-limit trace faults");
    }

    void testQualityPrecedenceAndDuty ()
    {
        const struct
        {
            uint16_t          energized;
            uint16_t          discharged;
            uint32_t          age;
            bool              off;
            uint32_t          onTime;
            uint32_t          cycleTime;
            adk::Status       status;
            adk::ProbeQuality quality;
        } cases[] = {
            {500, 5, 0, true, 10, 100, adk::StatusCode::HardwareFailure,
             adk::ProbeQuality::ProducerFault},
            {1023, 30, 0, false, 30, 100, adk::StatusCode::HardwareFailure,
             adk::ProbeQuality::ProducerFault},
            {1023, 5, 0, false, 10, 100, adk::StatusCode::Ok,
             adk::ProbeQuality::ExcitationFault},
            {1023, 21, 0, true, 10, 100, adk::StatusCode::Ok,
             adk::ProbeQuality::ExcitationFault},
            {1023, 5, 0, true, 21, 100, adk::StatusCode::Ok,
             adk::ProbeQuality::ExcitationFault},
            {1023, 5, 0, true, 21, 101, adk::StatusCode::Ok,
             adk::ProbeQuality::ExcitationFault},
            {1023, 5, 0, true, 20, 100, adk::StatusCode::Ok,
             adk::ProbeQuality::Saturated},
            {10, 10, 0, true, 10, 100, adk::StatusCode::Ok,
             adk::ProbeQuality::Disconnected},
            {500, 5, 100, true, 20, 100, adk::StatusCode::Ok, adk::ProbeQuality::Damp},
            {11, 10, 101, true, 10, 100, adk::StatusCode::Ok, adk::ProbeQuality::Stale},
            {500, 5, 101, true, 10, 100, adk::StatusCode::Ok,
             adk::ProbeQuality::Stale}};
        uint32_t sequence = 1;
        for (const auto& fixture : cases)
        {
            adk::ResistiveProbeObservationPolicy policy (config ());
            require                                     (policy.initialize ().ok (), "quality fixture initializes");
            adk::ResistiveProbeSample input =
                sample (fixture.energized, fixture.discharged, 100, sequence);
            input.excitationObservedOffAfterSample = fixture.off;
            input.excitationOnTime                 = adk::Duration (fixture.onTime);
            input.cycleTime                        = adk::Duration (fixture.cycleTime);
            input.status                           = fixture.status;
            const adk::Status result =
                policy.update (adk::TimePoint (100 + fixture.age), input);
            if (fixture.status.ok () ? !result.ok () : result != fixture.status)
            {
                std::cerr << "FAIL: well-formed unhealthy observation "
                             "is accepted (case "
                          << sequence << ", status "
                          << static_cast<unsigned> (result.error ()) << ")\n";
                std::exit (EXIT_FAILURE);
            }
            require (policy.snapshot ().quality == fixture.quality,
                     "quality precedence exact");
            ++sequence;
        }

        adk::ResistiveProbeObservationPolicy duty       (config ());
        require                                         (duty.initialize ().ok (), "duty fixture initializes");
        adk::ResistiveProbeSample input = sample        ();
        input.excitationOnTime          = adk::Duration (1);
        input.cycleTime                 = adk::Duration (3);
        require                                         (duty.update (adk::TimePoint (100), input).ok (),
                 "fractional duty accepts");
        require (duty.snapshot ().observedCycleDutyPermille == 333 &&
                     duty.snapshot ().quality == adk::ProbeQuality::ExcitationFault,
                 "duty uses widened floor division and gate");
    }

    void testOrderingTimeRolloverAndExhaustion ()
    {
        adk::ResistiveProbeObservationPolicy policy (config ());
        require                                     (policy.initialize ().ok (), "ordering fixture initializes");
        require                                     (policy.update (adk::TimePoint (100), sample ()).ok (),
                 "ordering fixture seeds");
        requireRejectedAtomically (
            policy, adk::TimePoint (101), sample (500, 5, 101, 1),
            adk::StatusCode::InvalidArgument, "duplicate sequence rejects");
        requireRejectedAtomically (
            policy, adk::TimePoint (101), sample (500, 5, 101, UINT32_MAX),
            adk::StatusCode::InvalidArgument, "regressed sequence rejects");
        requireRejectedAtomically (
            policy, adk::TimePoint (101), sample (500, 5, 101, 1U + 0x80000000UL),
            adk::StatusCode::InvalidArgument, "ambiguous half-range sequence rejects");
        requireRejectedAtomically (policy, adk::TimePoint (99), sample (500, 5, 99, 2),
                                   adk::StatusCode::InvalidArgument,
                                   "policy time regression rejects");
        requireRejectedAtomically (
            policy, adk::TimePoint (100 + 0x80000000UL), sample (500, 5, 100, 2),
            adk::StatusCode::InvalidArgument, "exact half-range policy time rejects");
        requireRejectedAtomically (
            policy, adk::TimePoint (100), sample (500, 5, 101, 2),
            adk::StatusCode::InvalidArgument, "future observed time rejects");

        adk::ResistiveProbeObservationPolicy rollover (config ());
        require                                       (rollover.initialize ().ok (), "rollover fixture initializes");
        require                                       (rollover
                     .update (adk::TimePoint (UINT32_MAX - 2U),
                              sample (500, 5, UINT32_MAX - 3U, UINT32_MAX))
                     .ok (),
                 "maximum producer sequence accepts");
        requireRejectedAtomically (
            rollover, adk::TimePoint (2), sample (500, 5, 1, 1),
            adk::StatusCode::CapacityExceeded,
            "producer sequence exhaustion rejects wrap until reset");
        requireRejectedAtomically (
            rollover, adk::TimePoint (UINT32_MAX - 2U),
            sample                   (500, 5, UINT32_MAX - 3U, UINT32_MAX),
            adk::StatusCode::CapacityExceeded,
            "producer sequence exhaustion rejects later duplicate updates");
        rollover.reset ();
        require        (rollover.update (adk::TimePoint (2), sample (500, 5, 1, 1)).ok (),
                 "reset explicitly begins a new producer sequence epoch");
    }

    void testEveryProducerStatusAndReplay ()
    {
        const adk::StatusCode statuses[] = {
            adk::StatusCode::InvalidArgument,   adk::StatusCode::InvalidConfiguration,
            adk::StatusCode::InvalidPin,        adk::StatusCode::Unsupported,
            adk::StatusCode::ResourceBusy,      adk::StatusCode::NotInitialized,
            adk::StatusCode::CapacityExceeded,  adk::StatusCode::Timeout,
            adk::StatusCode::InternalInvariant, adk::StatusCode::HardwareFailure};
        uint32_t sequence = 1;
        for (adk::StatusCode status : statuses)
        {
            adk::ResistiveProbeObservationPolicy policy (config ());
            require                                     (policy.initialize ().ok (), "producer fixture initializes");
            require                                     (policy.update (adk::TimePoint (100),
                                    sample (500, 5, 100, sequence, status))
                             .error () == status,
                     "producer fault returns copied producer status");
            require (policy.snapshot ().quality == adk::ProbeQuality::ProducerFault &&
                         policy.snapshot ().status.error () == status,
                     "producer status preserved exactly");
            ++sequence;
        }

        adk::ResistiveProbeObservationPolicy left  (config ());
        adk::ResistiveProbeObservationPolicy right (config ());
        require                                    (left.initialize ().ok () && right.initialize ().ok (),
                 "replay policies initialize");

        const adk::ResistiveProbeSample canonicalInput = sample ();
        require                                                 (left.update (adk::TimePoint (100), canonicalInput).ok (),
                 "canonical witness input accepts");
        const ObservationWitness canonicalBytes = canonicalWitness (left.snapshot ());
        const ObservationWitness expectedCanonicalBytes = {
            0x07, 0x0b, 0x00, 0x0d, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64,
            0x00, 0x00, 0x00, 0xf4, 0x01, 0x05, 0x00, 0x0a, 0x00, 0x00,
            0x00, 0x64, 0x00, 0x00, 0x00, 0x01, 0x00, 0xf4, 0x01, 0x64,
            0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
        require (canonicalBytes == expectedCanonicalBytes,
                 "canonical witness matches independently specified bytes");
        require (left.update (adk::TimePoint (100), canonicalInput).ok (),
                 "pre-exhaustion byte-identical duplicate is idempotent");
        require (canonicalWitness (left.snapshot ()) == canonicalBytes,
                 "pre-exhaustion duplicate preserves every canonical byte");

        left.reset ();
        require    (left.initialize ().ok (), "left replay policy resets");
        for (uint32_t index = 0; index < 80; ++index)
        {
            const uint32_t            time  = UINT32_MAX - 20U + index;
            adk::ResistiveProbeSample input = sample (
                static_cast<uint16_t> (100U + index * 11U), 5, time, index + 1U);
            require (left.update (adk::TimePoint (time + 2U), input).ok () &&
                         right.update (adk::TimePoint (time + 2U), input).ok (),
                     "replay trace accepts identically");
            require (observationEqual (left.snapshot (), right.snapshot ()),
                     "replay trace is field-identical");
            const ObservationWitness leftBytes  = canonicalWitness (left.snapshot ());
            const ObservationWitness rightBytes = canonicalWitness (right.snapshot ());
            require                                                (std::memcmp (leftBytes.data (), rightBytes.data (),
                                  leftBytes.size ()) == 0,
                     "replay trace has byte-identical canonical witness");
        }
    }

} // namespace

int main ()
{
    testLifecycleAndConfiguration                  ();
    testStructuralDomainAndAtomicity               ();
    testCalibrationSlopesAndThresholds             ();
    testEveryAdcCodeForBothSlopes                  ();
    testContaminationDriftAndElectricalFaultTraces ();
    testQualityPrecedenceAndDuty                   ();
    testOrderingTimeRolloverAndExhaustion          ();
    testEveryProducerStatusAndReplay               ();
    std::cout << "resistive probe observation tests passed\n";
}
