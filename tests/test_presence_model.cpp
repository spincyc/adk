#include <presence_model.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {
    constexpr uint8_t pirBit  = static_cast<uint8_t> (adk::PresenceSourceBit::Pir);
    constexpr uint8_t beamBit = static_cast<uint8_t> (adk::PresenceSourceBit::Beam);
    constexpr uint8_t guardBit =
        static_cast<uint8_t> (adk::PresenceSourceBit::FinishGuard);
    constexpr uint8_t rangeBit = static_cast<uint8_t> (adk::PresenceSourceBit::Range);

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::PirObservationConfig pirConfig ()
    {
        return {3,
                adk::Level::High,
                adk::Duration (100),
                adk::Duration (10),
                adk::Duration (20),
                adk::Duration (50)};
    }

    adk::PirSample pirSample (uint32_t now, adk::Level level,
                              adk::Status status = adk::Status ())
    {
        return {3, adk::TimePoint (now), level, status};
    }

    adk::PresenceModelConfig
    presenceConfig (uint8_t required  = pirBit | beamBit | rangeBit,
                    uint8_t agreement = pirBit | beamBit | rangeBit)
    {
        return {required,
                agreement,
                adk::Duration (20),
                adk::Duration (20),
                adk::Duration (20),
                adk::Duration (20),
                adk::Duration (30),
                adk::Duration (10),
                100,
                500};
    }

    adk::PirObservation pirObservation (uint32_t      now,
                                        adk::PirPhase phase  = adk::PirPhase::Motion,
                                        adk::Status   status = adk::Status ())
    {
        return {3,
                adk::TimePoint (now),
                phase == adk::PirPhase::ReadyClear ? adk::Level::Low : adk::Level::High,
                phase,
                false,
                false,
                adk::Duration (),
                status};
    }

    adk::BeamObservation beamObservation (uint32_t now, bool interrupted,
                                          bool        interruptionEvent = false,
                                          bool        restorationEvent  = false,
                                          adk::Status status = adk::Status ())
    {
        return {{4, 7, adk::TimePoint (now)},
                interrupted ? adk::Level::Low : adk::Level::High,
                interrupted,
                interruptionEvent,
                restorationEvent,
                adk::Duration (),
                status.ok     () ? adk::OpticalQuality::Valid
                             : adk::OpticalQuality::SourceFault,
                status};
    }

    adk::ReflectiveObservation guardObservation (uint32_t now, bool active,
                                                 adk::Status status = adk::Status ())
    {
        return {{5, 9, adk::TimePoint (now)},
                400,
                100,
                900,
                375,
                active,
                false,
                false,
                adk::Duration (),
                status.ok     () ? adk::OpticalQuality::Valid
                             : adk::OpticalQuality::SourceFault,
                status};
    }

    adk::TimedRangeEvidence
    rangeEvidence (uint32_t now, uint16_t distance = 250,
                   adk::RangeState state  = adk::RangeState::Valid,
                   adk::Status     status = adk::Status ())
    {
        const bool     valid   = state == adk::RangeState::Valid;
        const uint32_t latency = state == adk::RangeState::Timeout ? 1000U : 600U;
        const uint32_t echo    = valid ? 500U : 0U;

        return {6,
                adk::TimePoint            (now - 1U),
                adk::TimePoint            (now),
                adk::MicrosecondTimePoint (1000),
                adk::MicrosecondDuration  (latency),
                {state, static_cast<uint16_t> (valid ? distance : 0U),
                 adk::MicrosecondDuration (echo), valid},
                status};
    }

    adk::PresenceInput emptyInput (uint32_t now)
    {
        const adk::PirObservation absentPir = {
            0,     adk::TimePoint (), adk::Level::Low, adk::PirPhase::Warming, false,
            false, adk::Duration  (),  adk::Status ()};
        const adk::BeamObservation       absentBeam = {{0, 0, adk::TimePoint ()},
                                                       adk::Level::Low,
                                                       false,
                                                       false,
                                                       false,
                                                       adk::Duration (),
                                                       adk::OpticalQuality::Unqualified,
                                                       adk::Status ()};
        const adk::ReflectiveObservation absentGuard = {
            {0, 0, adk::TimePoint ()},
            0,
            0,
            0,
            0,
            false,
            false,
            false,
            adk::Duration (),
            adk::OpticalQuality::Unqualified,
            adk::Status ()};
        const adk::TimedRangeEvidence absentRange = {
            0,
            adk::TimePoint                                      (),
            adk::TimePoint                                      (),
            adk::MicrosecondTimePoint                           (),
            adk::MicrosecondDuration                            (),
            {adk::RangeState::Idle, 0, adk::MicrosecondDuration (), false},
            adk::Status                                         ()};
        return {adk::TimePoint (now),
                {false, absentPir},
                {false, absentBeam},
                {false, absentGuard},
                {false, absentRange}};
    }

    adk::PresenceInput validInput (uint32_t now, bool motion = true,
                                   bool interrupted = true, uint16_t distance = 250)
    {
        adk::PresenceInput input = emptyInput (now);
        input.pir  = {true, pirObservation    (now, motion ? adk::PirPhase::Motion
                                                        : adk::PirPhase::ReadyClear)};
        input.beam = {true, beamObservation         (now, interrupted)};
        input.finishGuard = {true, guardObservation (now, interrupted)};
        input.range       = {true, rangeEvidence    (now, distance)};
        return input;
    }

    void testPirLifecycleAndConfiguration ()
    {
        static_assert (!std::is_copy_constructible<adk::PirObservationPolicy>::value,
                       "PIR policy owns deterministic state");

        adk::PirObservationPolicy policy (pirConfig ());
        require                          (!policy.initialized (), "PIR starts inert");
        require                          (policy.update (pirSample (0, adk::Level::Low)).error () ==
                     adk::StatusCode::NotInitialized,
                 "PIR rejects update before initialize");
        require (policy.initialize ().ok (), "PIR initializes");
        require (policy.initialize ().ok (), "PIR initialization is idempotent");
        require (policy.snapshot ().phase == adk::PirPhase::Warming,
                 "PIR begins warming");

        policy.reset ();
        require      (policy.initialized (), "PIR reset preserves initialization");
        require      (policy.snapshot ().phase == adk::PirPhase::Warming,
                 "PIR reset restores canonical phase");

        adk::PirObservationConfig invalid[] = {
            {3, static_cast<adk::Level> (2), adk::Duration (1), adk::Duration (1),
             adk::Duration (1), adk::Duration (2)},
            {3, adk::Level::High, adk::Duration (1), adk::Duration (2),
             adk::Duration (2), adk::Duration (1)}};
        for (const auto& config : invalid)
        {
            adk::PirObservationPolicy rejected (config);
            require                            (rejected.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "PIR invalid configuration rejected");
        }
    }

    void testPirWarmupQualificationAndStuckRecovery ()
    {
        adk::PirObservationPolicy policy (pirConfig ());
        require                          (policy.initialize ().ok (), "PIR trace initializes");

        require (policy.update (pirSample (1000, adk::Level::Low)).ok (),
                 "first PIR sample starts warmup");
        require (policy.snapshot ().phase == adk::PirPhase::Warming,
                 "PIR remains warming initially");
        policy.update (pirSample (1099, adk::Level::Low));
        require       (policy.snapshot ().phase == adk::PirPhase::Warming,
                 "PIR warmup excludes boundary minus one");
        policy.update (pirSample (1100, adk::Level::Low));
        require       (policy.snapshot ().phase == adk::PirPhase::Warming,
                 "clear still requires qualification after warmup");
        policy.update (pirSample (1119, adk::Level::Low));
        require       (policy.snapshot ().phase == adk::PirPhase::Warming,
                 "clear qualification excludes boundary minus one");
        policy.update (pirSample (1120, adk::Level::Low));
        require       (policy.snapshot ().phase == adk::PirPhase::ReadyClear,
                 "clear qualifies at exact boundary");
        require (policy.snapshot ().clearEvent, "clear transition emits event");

        policy.update (pirSample (1121, adk::Level::High));
        policy.update (pirSample (1130, adk::Level::High));
        require       (policy.snapshot ().phase == adk::PirPhase::ReadyClear,
                 "motion qualification excludes boundary minus one");
        policy.update (pirSample (1131, adk::Level::High));
        require       (policy.snapshot ().phase == adk::PirPhase::Motion,
                 "motion qualifies at boundary");
        require       (policy.snapshot ().motionEvent, "motion transition emits event");
        policy.update (pirSample (1180, adk::Level::High));
        require       (policy.snapshot ().phase == adk::PirPhase::Motion,
                 "motion is not stuck before threshold");
        policy.update (pirSample (1181, adk::Level::High));
        require       (policy.snapshot ().phase == adk::PirPhase::StuckMotion,
                 "held motion becomes stuck at threshold");

        policy.update (pirSample (1182, adk::Level::Low));
        policy.update (pirSample (1202, adk::Level::Low));
        require       (policy.snapshot ().phase == adk::PirPhase::ReadyClear,
                 "qualified clear recovers from stuck");
        require (policy.snapshot ().clearEvent, "stuck recovery emits clear");
    }

    void testPirFaultTimeReplayAndWrap ()
    {
        adk::PirObservationPolicy policy                   (pirConfig ());
        require                                            (policy.initialize ().ok (), "PIR replay initializes");
        const adk::PirSample sample = pirSample            (100, adk::Level::Low);
        require                                            (policy.update (sample).ok (), "PIR accepts first frame");
        const adk::PirObservation before = policy.snapshot ();
        require                                            (policy.update (sample).ok (), "identical PIR frame is idempotent");
        require                                            (policy.snapshot ().observedAt == before.observedAt,
                 "identical PIR frame preserves snapshot");
        require (policy.update (pirSample (100, adk::Level::High)).error () ==
                     adk::StatusCode::InvalidArgument,
                 "changed same-time PIR frame faults");
        require (policy.snapshot ().phase == adk::PirPhase::Fault,
                 "changed same-time PIR publishes fault");

        policy.reset      ();
        policy.initialize ();
        require           (
            policy.update (pirSample (5, adk::Level::Low,
                                      adk::Status (adk::StatusCode::HardwareFailure)))
                    .error () == adk::StatusCode::HardwareFailure,
            "PIR propagates source failure");
        require (policy.snapshot ().phase == adk::PirPhase::Fault,
                 "source failure publishes PIR fault");

        policy.reset      ();
        policy.initialize ();
        policy.update     (pirSample (0xfffffff0U, adk::Level::Low));
        policy.update     (pirSample (0x00000054U, adk::Level::Low));
        require           (policy.snapshot ().phase == adk::PirPhase::Warming,
                 "PIR warmup spans rollover deterministically");
        require (policy.update (pirSample (0x80000054U, adk::Level::Low)).error () ==
                     adk::StatusCode::InvalidArgument,
                 "PIR rejects half-range jump");
    }

    void testPresenceLifecycleAndConfiguration ()
    {
        adk::PresenceModel model (presenceConfig ());
        require                  (!model.initialized (), "presence starts inert");
        require                  (model.update (emptyInput (0)).error () ==
                     adk::StatusCode::NotInitialized,
                 "presence rejects update before initialize");
        require (model.initialize ().ok (), "presence initializes");
        require (model.initialize ().ok (), "presence initialization idempotent");
        require (model.snapshot ().quality == adk::PresenceQuality::Unqualified,
                 "presence starts unqualified");
        model.reset ();
        require     (model.initialized (), "presence reset preserves initialization");

        adk::PresenceModelConfig invalid[] = {
            presenceConfig (0x80, 0), presenceConfig (pirBit, pirBit),
            presenceConfig (pirBit, pirBit | beamBit),
            presenceConfig (pirBit | beamBit, pirBit | beamBit)};
        invalid[3].approachMinimumMm = 0;
        for (const auto& config : invalid)
        {
            adk::PresenceModel rejected (config);
            require                     (rejected.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "presence invalid configuration rejected");
        }
    }

    void testPresenceAbsentFreshStaleAndRangeBounds ()
    {
        adk::PresenceModel model                            (presenceConfig ());
        require                                             (model.initialize ().ok (), "presence freshness initializes");
        require                                             (model.update (emptyInput (10)).ok (), "absent frame accepted");
        const adk::PresenceSnapshot absent = model.snapshot ();
        require                                             (absent.quality == adk::PresenceQuality::Unqualified,
                 "missing required source is unqualified");
        require (!absent.pir.available && !absent.beam.available,
                 "absent sources remain unavailable");
        require (!absent.passageEvent, "absence emits no passage");

        require (model.update (validInput (20)).ok (), "fresh frame accepted");
        require (model.snapshot ().quality == adk::PresenceQuality::Valid,
                 "fresh required evidence is valid");
        require (model.snapshot ().pirEligible, "motion makes PIR eligible");
        require (model.snapshot ().range.approachValid,
                 "inclusive approach range is valid");

        adk::PresenceInput minimum = validInput (21, true, true, 100);
        require                                 (model.update (minimum).ok (), "minimum approach accepted");
        require                                 (model.snapshot ().range.approachValid,
                 "minimum approach is inclusive");
        adk::PresenceInput maximum = validInput (22, true, true, 500);
        require                                 (model.update (maximum).ok (), "maximum approach accepted");
        require                                 (model.snapshot ().range.approachValid,
                 "maximum approach is inclusive");
        adk::PresenceInput outside = validInput (23, true, false, 501);
        require                                 (model.update (outside).ok (), "outside range remains valid evidence");
        require                                 (!model.snapshot ().range.approachValid,
                 "outside range is not an approach");

        adk::PresenceInput stale               = validInput     (50);
        stale.pir.value.observedAt             = adk::TimePoint (30);
        stale.beam.value.provenance.observedAt = adk::TimePoint (30);
        stale.range.value.startedAt            = adk::TimePoint (29);
        stale.range.value.completedAt          = adk::TimePoint (30);
        require                                                 (model.update (stale).ok (), "age boundary frame accepted");
        require                                                 (model.snapshot ().quality == adk::PresenceQuality::Stale,
                 "maximum age boundary is stale");
        require (model.snapshot ().pir.stale && model.snapshot ().beam.stale &&
                     model.snapshot ().range.stale,
                 "each old source retains stale state");
    }

    void testPresenceDisagreementWindowAndRecovery ()
    {
        adk::PresenceModel model (presenceConfig ());
        require                  (model.initialize ().ok (), "disagreement initializes");

        adk::PresenceInput mismatch = validInput (100, true, false, 250);
        require                                  (model.update (mismatch).ok (), "mismatch candidate accepted");
        require                                  (model.snapshot ().quality == adk::PresenceQuality::Valid,
                 "mismatch waits through agreement window");
        require (model.snapshot ().disagreement, "candidate mismatch is observable");
        require (model.snapshot ().disagreementFor == adk::Duration (),
                 "candidate starts at zero duration");

        mismatch = validInput (109, true, false, 250);
        model.update          (mismatch);
        require               (model.snapshot ().quality == adk::PresenceQuality::Valid,
                 "disagreement excludes boundary minus one");
        mismatch = validInput (110, true, false, 250);
        model.update          (mismatch);
        require               (model.snapshot ().quality == adk::PresenceQuality::Disagreement,
                 "disagreement publishes at exact boundary");
        require (!model.snapshot ().passageEvent, "disagreement suppresses passage");

        require (model.update (validInput (111, true, true, 250)).ok (),
                 "matching evidence recovers");
        require (!model.snapshot ().disagreement,
                 "matching evidence clears disagreement");
        require (model.snapshot ().quality == adk::PresenceQuality::Valid,
                 "matching evidence restores validity");
    }

    void testPresencePassageCandidateAndDeduplication ()
    {
        adk::PresenceModelConfig config = presenceConfig ();
        config.agreementSources         = 0;
        adk::PresenceModel model (config);
        require                  (model.initialize ().ok (), "passage initializes");

        adk::PresenceInput enter           = validInput (100);
        enter.beam.value.interruptionEvent = true;
        model.update (enter);
        require      (!model.snapshot ().passageEvent,
                 "beam interruption starts but does not emit passage");

        adk::PresenceInput leave          = validInput (130, true, false, 250);
        leave.beam.value.restorationEvent = true;
        model.update (leave);
        require      (model.snapshot ().passageEvent,
                 "restoration at passage boundary emits");
        model.update (leave);
        require      (model.snapshot ().passageEvent,
                 "identical same-time replay is byte-stable");

        adk::PresenceInput later = validInput (131, true, false, 250);
        model.update                          (later);
        require                               (!model.snapshot ().passageEvent, "passage event lasts one new frame");

        model.reset                                     ();
        model.initialize                                ();
        enter                              = validInput (200);
        enter.beam.value.interruptionEvent = true;
        model.update                                   (enter);
        leave                             = validInput (231, true, false, 250);
        leave.beam.value.restorationEvent = true;
        model.update (leave);
        require      (!model.snapshot ().passageEvent,
                 "too-long passage candidate is discarded");
    }

    void testPresenceTimingTupleAndFaultPrecedence ()
    {
        adk::PresenceModel model (presenceConfig ());
        require                  (model.initialize ().ok (), "fault precedence initializes");
        require                  (model.update (validInput (100)).ok (), "baseline accepted");

        adk::PresenceInput future   = validInput     (101);
        future.pir.value.observedAt = adk::TimePoint (102);
        require                                      (model.update (future).error () == adk::StatusCode::InvalidArgument,
                 "future evidence faults");
        require (model.snapshot ().quality == adk::PresenceQuality::TimingFault,
                 "future evidence has timing precedence");
        require (!model.snapshot ().passageEvent, "malformed frame cannot partly emit");

        model.reset                                     ();
        model.initialize                                ();
        adk::PresenceInput badRange        = validInput (200);
        badRange.range.value.reading.valid = false;
        require (model.update (badRange).error () == adk::StatusCode::InvalidArgument,
                 "malformed range tuple rejected");
        require (model.snapshot ().quality == adk::PresenceQuality::TimingFault,
                 "malformed tuple publishes canonical fault");

        model.reset                                  ();
        model.initialize                             ();
        adk::PresenceInput sourceFault = validInput  (300);
        sourceFault.beam.value.status  = adk::Status (adk::StatusCode::HardwareFailure);
        sourceFault.beam.value.quality = adk::OpticalQuality::SourceFault;
        require (model.update (sourceFault).error () ==
                     adk::StatusCode::HardwareFailure,
                 "source failure propagated");
        require (model.snapshot ().quality == adk::PresenceQuality::SourceFault,
                 "source failure classified");
    }

    void testPresenceSameTimeRolloverAndRangeStates ()
    {
        adk::PresenceModel model                    (presenceConfig ());
        require                                     (model.initialize ().ok (), "replay initializes");
        const adk::PresenceInput frame = validInput (100);
        require                                     (model.update (frame).ok (), "first aggregate frame accepted");
        require                                     (model.update (frame).ok (), "identical aggregate replay accepted");

        adk::PresenceInput changed = frame;
        changed.pir.value.rawLevel = adk::Level::Low;
        require (model.update (changed).error () == adk::StatusCode::InvalidArgument,
                 "changed same-time aggregate frame faults");

        model.reset                                ();
        model.initialize                           ();
        adk::PresenceInput timeout = validInput    (200);
        timeout.range.value        = rangeEvidence (200, 0, adk::RangeState::Timeout);
        require                                    (model.update (timeout).ok (), "semantic timeout is accepted evidence");
        require                                    (!model.snapshot ().range.valid, "semantic timeout is not valid range");
        require                                    (model.snapshot ().quality == adk::PresenceQuality::Unqualified &&
                     model.snapshot ().status.ok (),
                 "required semantic timeout leaves the aggregate unqualified");

        model.reset                            ();
        model.initialize                       ();
        adk::PresenceInput before = validInput (0xfffffff8U);
        require                                (model.update (before).ok (), "pre-rollover frame accepted");
        adk::PresenceInput after = validInput  (0x00000008U);
        require                                (model.update (after).ok (), "rollover frame accepted");
        require                                (model.snapshot ().quality == adk::PresenceQuality::Valid,
                 "aggregate time spans rollover");

        adk::PresenceInput halfRange = validInput (0x80000008U);
        require                                   (model.update (halfRange).error () == adk::StatusCode::InvalidArgument,
                 "aggregate rejects half-range jump");
    }

    void testPresenceRangeSemanticsAndCanonicalFault ()
    {
        adk::PresenceModel model (presenceConfig ());
        require                  (model.initialize ().ok (), "range semantics initializes");

        adk::PresenceInput timeout = validInput    (100);
        timeout.range.value        = rangeEvidence (100, 0, adk::RangeState::Timeout);
        require                                    (model.update (timeout).ok (), "semantic timeout tuple is accepted");
        require                                    (model.snapshot ().range.available && !model.snapshot ().range.valid &&
                     !model.snapshot ().range.approachValid,
                 "semantic timeout remains available non-approach evidence");
        require (model.snapshot ().quality == adk::PresenceQuality::Unqualified &&
                     model.snapshot ().status.ok (),
                 "required semantic timeout is unqualified rather than a source fault");

        model.reset                                ();
        adk::PresenceInput outOfRange = validInput (200);
        outOfRange.range.value = rangeEvidence     (200, 0, adk::RangeState::OutOfRange);
        require                                    (model.update (outOfRange).ok (), "out-of-range zero edge is accepted");
        require                                    (model.snapshot ().range.evidence.reading.state ==
                         adk::RangeState::OutOfRange &&
                     !model.snapshot ().range.valid,
                 "out-of-range state remains distinct from timeout");

        model.reset                                 ();
        adk::PresenceInput sourceFault = validInput (300);
        sourceFault.range.value        = {
            6,
            adk::TimePoint                                      (300),
            adk::TimePoint                                      (300),
            adk::MicrosecondTimePoint                           (),
            adk::MicrosecondDuration                            (),
            {adk::RangeState::Idle, 0, adk::MicrosecondDuration (), false},
            adk::Status                                         (adk::StatusCode::HardwareFailure)};
        require (model.update (sourceFault).error () ==
                     adk::StatusCode::HardwareFailure,
                 "canonical non-Ok range evidence propagates source status");
        require (model.snapshot ().range.evidence.startedAt == adk::TimePoint (300) &&
                     model.snapshot ().range.evidence.completedAt ==
                         adk::TimePoint (300),
                 "range source fault retains its two course-clock epochs");
        require (model.snapshot ().quality == adk::PresenceQuality::SourceFault,
                 "canonical range failure is a source fault");
    }

    void testPresenceImmediateAgreementAndEventIdentity ()
    {
        adk::PresenceModelConfig config = presenceConfig ();
        config.agreementWindow          = adk::Duration  ();
        adk::PresenceModel rejected                      (config);
        require                                          (rejected.initialize ().error () ==
                     adk::StatusCode::InvalidConfiguration,
                 "zero agreement window is rejected when comparison is enabled");

        adk::PresenceModel model                        (presenceConfig ());
        require                                         (model.initialize ().ok (), "event identity model initializes");
        adk::PresenceInput enter           = validInput (100);
        enter.beam.value.interruptionEvent = true;
        require (model.update (enter).ok (), "event identity baseline accepted");

        adk::PresenceInput replay = validInput (101);
        replay.beam.value         = enter.beam.value;
        require (model.update (replay).ok (),
                 "later frame may retain copied event evidence");
        require (!model.snapshot ().beam.activationEvent,
                 "later frame does not forward one optical event identity twice");
        require (!model.snapshot ().passageEvent,
                 "later frame does not apply one optical event identity twice");
    }

    void testPresenceOpticalIdentityAcrossAbsence ()
    {
        adk::PresenceModel beamModel                        (presenceConfig ());
        require                                             (beamModel.initialize ().ok (), "beam absence replay initializes");
        adk::PresenceInput beamEvent           = validInput (100);
        beamEvent.beam.value.interruptionEvent = true;
        require (beamModel.update (beamEvent).ok (), "beam event is accepted");

        adk::PresenceInput beamAbsent = validInput                    (101);
        beamAbsent.beam               = emptyInput                    (101).beam;
        beamAbsent.finishGuard        = emptyInput                    (101).finishGuard;
        require                                                       (beamModel.update (beamAbsent).ok (), "beam absence is accepted");
        const adk::PresenceSnapshot absentBefore = beamModel.snapshot ();
        require                                                       (beamModel.update (beamAbsent).ok (),
                 "identical same-time canonical absence is idempotent");
        const adk::PresenceSnapshot absentAfter = beamModel.snapshot ();
        require                                                      (absentAfter.quality == absentBefore.quality &&
                     absentAfter.status == absentBefore.status &&
                     absentAfter.pir.available == absentBefore.pir.available &&
                     absentAfter.pir.evidence.observedAt ==
                         absentBefore.pir.evidence.observedAt &&
                     absentAfter.pir.age == absentBefore.pir.age &&
                     absentAfter.beam.available == absentBefore.beam.available &&
                     absentAfter.finishGuard.available ==
                         absentBefore.finishGuard.available &&
                     absentAfter.range.available == absentBefore.range.available &&
                     absentAfter.range.evidence.completedAt ==
                         absentBefore.range.evidence.completedAt &&
                     absentAfter.range.age == absentBefore.range.age &&
                     absentAfter.pirEligible == absentBefore.pirEligible &&
                     absentAfter.passageEvent == absentBefore.passageEvent &&
                     absentAfter.disagreement == absentBefore.disagreement &&
                     absentAfter.disagreementFor == absentBefore.disagreementFor,
                 "same-time absence replay preserves the public snapshot");

        adk::PresenceInput beamReplay = validInput (102);
        beamReplay.beam.value         = beamEvent.beam.value;
        require (beamModel.update (beamReplay).ok (),
                 "beam identity may reappear after absence");
        require (!beamModel.snapshot ().beam.activationEvent,
                 "beam event identity remains deduplicated across absence");

        adk::PresenceInput backwardBeam = validInput      (103);
        backwardBeam.beam.value         = beamObservation (99, true);
        require                                           (beamModel.update (backwardBeam).error () ==
                     adk::StatusCode::InvalidArgument,
                 "beam source epoch cannot move backward across absence");

        adk::PresenceModel guardModel                                   (presenceConfig ());
        require                                                         (guardModel.initialize ().ok (), "guard absence replay initializes");
        adk::PresenceInput guardEvent                = validInput       (200);
        guardEvent.finishGuard.value                 = guardObservation (200, true);
        guardEvent.finishGuard.value.activationEvent = true;
        require (guardModel.update (guardEvent).ok (), "guard event is accepted");

        adk::PresenceInput guardAbsent = validInput  (201);
        guardAbsent.finishGuard         = emptyInput (201).finishGuard;
        require                                      (guardModel.update (guardAbsent).ok (), "guard absence is accepted");

        adk::PresenceInput guardReplay = validInput (202);
        guardReplay.finishGuard.value  = guardEvent.finishGuard.value;
        require (guardModel.update (guardReplay).ok (),
                 "guard identity may reappear after absence");
        require (!guardModel.snapshot ().finishGuard.activationEvent,
                 "guard event identity remains deduplicated across absence");

        adk::PresenceInput backwardGuard = validInput       (203);
        backwardGuard.finishGuard.value  = guardObservation (199, true);
        require                                             (guardModel.update (backwardGuard).error () ==
                     adk::StatusCode::InvalidArgument,
                 "guard source epoch cannot move backward across absence");
    }

    void testPresenceDisagreementSuppressesPassageCandidate ()
    {
        adk::PresenceModel model (presenceConfig ());
        require                  (model.initialize ().ok (), "disagreement passage initializes");

        adk::PresenceInput interrupted = validInput (100, false, true, 250);
        interrupted.beam.value.interruptionEvent = true;
        require (model.update (interrupted).ok (),
                 "pre-threshold mismatch interruption is accepted");
        require (model.snapshot ().disagreement &&
                     model.snapshot ().quality == adk::PresenceQuality::Valid,
                 "pre-threshold disagreement remains observable");
        require (!model.snapshot ().passageEvent,
                 "disagreement interruption cannot emit passage");

        adk::PresenceInput restored = validInput (101, false, false, 501);
        restored.beam.value.restorationEvent = true;
        require (model.update (restored).ok (),
                 "matching restoration after mismatch is accepted");
        require (!model.snapshot ().disagreement,
                 "matching restoration clears disagreement");
        require (!model.snapshot ().passageEvent,
                 "restoration cannot close a candidate suppressed by disagreement");
    }

    static_assert (std::is_trivially_copyable<adk::PresenceSnapshot>::value,
                   "presence snapshot remains an allocation-free copied value");
    static_assert (!std::is_copy_constructible<adk::PresenceModel>::value,
                   "presence model owns deterministic transition state");
    static_assert (!std::is_copy_assignable<adk::PresenceModel>::value,
                   "presence model cannot duplicate transition state");
    static_assert (!std::is_move_constructible<adk::PresenceModel>::value,
                   "presence model remains at a stable address");
    static_assert (!std::is_move_assignable<adk::PresenceModel>::value,
                   "presence model cannot transfer transition state");
#if defined(__AVR__)
    static_assert (sizeof (adk::PresenceModel) <= 128U,
                   "presence model must fit the AVR largest-object budget");
#endif
} // namespace

int main ()
{
    testPirLifecycleAndConfiguration                   ();
    testPirWarmupQualificationAndStuckRecovery         ();
    testPirFaultTimeReplayAndWrap                      ();
    testPresenceLifecycleAndConfiguration              ();
    testPresenceAbsentFreshStaleAndRangeBounds         ();
    testPresenceDisagreementWindowAndRecovery          ();
    testPresencePassageCandidateAndDeduplication       ();
    testPresenceTimingTupleAndFaultPrecedence          ();
    testPresenceSameTimeRolloverAndRangeStates         ();
    testPresenceRangeSemanticsAndCanonicalFault        ();
    testPresenceImmediateAgreementAndEventIdentity     ();
    testPresenceOpticalIdentityAcrossAbsence           ();
    testPresenceDisagreementSuppressesPassageCandidate ();
}
