#include <balance_table_instrument.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <type_traits>

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

    void requireStatus (adk::Status status, adk::StatusCode expected,
                        const char* message)
    {
        require (status.error () == expected, message);
    }

    adk::OrientationConfig orientationConfig ()
    {
        return {{adk::SignedAxis::PositiveX, adk::SignedAxis::PositiveY,
                 adk::SignedAxis::PositiveZ},
                1,
                2000000,
                1000,
                5000,
                120000};
    }

    adk::BalancePresentationConfig presentationConfig ()
    {
        return {{0, 1000, 0, false},
                {1000, 0, 0, false},
                {0, 1000, 0, false},
                {0, 0, 1000, false},
                {1000, 1000, 0, false},
                {0, 0, 300, false},
                {0, 0, 700, false},
                {1000, 500, 0, true},
                {1000, 0, 1000, true},
                120000,
                100,
                1000,
                1000,
                20};
    }

    adk::BalanceInstrumentConfig instrumentConfig ()
    {
        return {200,
                1000,
                200,
                adk::Duration (100),
                7,
                adk::Duration (200),

                adk::Duration (20),
                {adk::BalanceDirection::None, {0, 200, 0, false}, {false, 0, 0},
                 adk::Status ()},
                {adk::BalanceDirection::None, {300, 200, 0, false},
                 {false, 0, 0}, adk::Status ()},
                {adk::BalanceDirection::None, {1000, 0, 0, true},
                 {false, 0, 0}, adk::StatusCode::HardwareFailure},
                {adk::BalanceDirection::None, {0, 0, 0, false}, {false, 0, 0},
                 adk::StatusCode::NotInitialized}};
    }

    adk::InertialSource source (uint8_t sourceId = 1)
    {
        return {adk::InertialSourceKind::SyntheticFixture,
                adk::InertialModel::Synthetic,
                sourceId,
                2,
                3,
                2000000,
                250000};
    }

    adk::BalanceInstrumentInput input (uint32_t frameAt, uint32_t frameSequence,
                                       uint32_t inertialSequence,
                                       uint32_t joystickSequence,
                                       uint32_t buttonSequence, int32_t x = 0,
                                       int32_t y = 0, int32_t z = 1000000)
    {
        const adk::InertialSample      sample   = {source (),
                                                   {x, y, z},
                                                   {0, 0, 0},
                                                   adk::TimePoint (frameAt),
                                                   inertialSequence,
                                                   true,
                                                   adk::InertialSaturation::None,
                                                   adk::Status ()};
        const adk::InertialObservation inertial = {sample,
                                                   adk::InertialSampleQuality::Current,
                                                   true,
                                                   adk::Duration (),

                                                   adk::Duration (100),
                                                   7,
                                                   0,
                                                   adk::Status ()};
        return {inertial,
                {0, 0, adk::SensitivityEvent::None, adk::TimePoint (frameAt),
                 joystickSequence, adk::Status ()},
                {false, false, false, adk::TimePoint (frameAt), buttonSequence,
                 adk::Status ()},
                adk::TimePoint (frameAt),
                frameSequence};
    }

    void press (adk::BalanceInstrumentInput& value)
    {
        value.freezeButton.pressed      = true;
        value.freezeButton.pressEvent   = true;
        value.freezeButton.releaseEvent = false;
    }

    void increase (adk::BalanceInstrumentInput& value)
    {
        value.joystick.xPermille = 1000;
        value.joystick.event     = adk::SensitivityEvent::Increase;
    }

    void decrease (adk::BalanceInstrumentInput& value)
    {
        value.joystick.xPermille = -1000;
        value.joystick.event     = adk::SensitivityEvent::Decrease;
    }

    bool estimateEqual (const adk::OrientationEstimate& left,
                        const adk::OrientationEstimate& right)
    {
        return left.pitchMilliDegrees == right.pitchMilliDegrees &&
               left.rollMilliDegrees == right.rollMilliDegrees &&
               left.quality == right.quality && left.status == right.status;
    }

    bool lightEqual (const adk::BalanceLightIntent& left,
                     const adk::BalanceLightIntent& right)
    {
        return left.redPermille == right.redPermille &&
               left.greenPermille == right.greenPermille &&
               left.bluePermille == right.bluePermille && left.fault == right.fault;
    }

    bool presentationEqual (const adk::BalancePresentation& left,
                            const adk::BalancePresentation& right)
    {
        return left.direction == right.direction &&
               lightEqual (left.light, right.light) &&
               left.tone.enabled == right.tone.enabled &&
               left.tone.frequencyHertz == right.tone.frequencyHertz &&
               left.tone.durationMilliseconds == right.tone.durationMilliseconds &&
               left.status == right.status;
    }

    bool evidenceEqual (const adk::BalanceMeasurementEvidence& left,
                        const adk::BalanceMeasurementEvidence& right)
    {
        const adk::CompactInertialEvidence& a = left.provenance;
        const adk::CompactInertialEvidence& b = right.provenance;
        return left.available == right.available &&
               estimateEqual (left.estimate, right.estimate) &&
               a.source.kind == b.source.kind && a.source.model == b.source.model &&
               a.source.sourceId == b.source.sourceId &&
               a.source.configurationRevision == b.source.configurationRevision &&
               a.source.calibrationRevision == b.source.calibrationRevision &&
               a.source.accelerationRangeMicroG == b.source.accelerationRangeMicroG &&
               a.source.angularRateRangeMilliDegreesPerSecond ==
                   b.source.angularRateRangeMilliDegreesPerSecond &&
               a.observedAt == b.observedAt && a.sequence == b.sequence &&
               a.quality == b.quality && a.maximumAge == b.maximumAge &&
               a.freshnessContractRevision == b.freshnessContractRevision &&
               a.saturation == b.saturation &&
               a.acceptedDataReady == b.acceptedDataReady &&
               a.latestDataReady == b.latestDataReady && a.status == b.status;
    }

    bool outputEqual (const adk::BalanceInstrumentOutput& left,
                      const adk::BalanceInstrumentOutput& right)
    {
        return left.mode == right.mode &&
               evidenceEqual (left.liveEvidence, right.liveEvidence) &&

               evidenceEqual (left.frozenEvidence, right.frozenEvidence) &&

               presentationEqual (left.presentation, right.presentation) &&
               left.sensitivityPermille == right.sensitivityPermille &&
               left.acceptedFrameSequence == right.acceptedFrameSequence &&
               left.acceptedFrameAt == right.acceptedFrameAt &&
               left.inertialStatus == right.inertialStatus &&
               left.joystickStatus == right.joystickStatus &&
               left.buttonStatus == right.buttonStatus && left.status == right.status;
    }

    struct Fixture
    {
        adk::BalanceFrameStorage storage;
        adk::BalanceInstrument   instrument;

        explicit Fixture (
            adk::BalanceInstrumentConfig   config            = instrumentConfig (),

            adk::OrientationConfig         orientationValue  = orientationConfig (),

            adk::BalancePresentationConfig presentationValue = presentationConfig ())

            : storage (),
              instrument (config, orientationValue, presentationValue, storage)
        {
        }
    };

    void testLifecycleAndConfiguration ()
    {
        static_assert (!std::is_copy_constructible<adk::BalanceInstrument>::value,
                       "instrument does not copy");
        static_assert (!std::is_move_constructible<adk::BalanceInstrument>::value,
                       "instrument does not move");

        Fixture fixture;
        require (!fixture.instrument.initialized (), "instrument starts inert");

        requireStatus (fixture.instrument.update (input (0, 1, 1, 1, 1)),
                       adk::StatusCode::NotInitialized,
                       "update before initialize is rejected");
        requireStatus (fixture.instrument.acknowledgeFault (),
                       adk::StatusCode::NotInitialized,
                       "acknowledge before initialize is rejected");
        require (fixture.instrument.initialize ().ok () &&
                     fixture.instrument.initialize ().ok (),
                 "initialize is idempotent");
        require (fixture.instrument.snapshot ().mode ==
                     adk::BalanceInstrumentMode::AwaitingFrame,
                 "initialize awaits first frame");
        require (!fixture.storage.available, "initialize leaves replay empty");

        fixture.instrument.shutdown ();

        fixture.instrument.shutdown ();

        require (!fixture.instrument.initialized (), "shutdown is idempotent");

        require (!fixture.storage.available, "shutdown clears replay storage");

        require (!fixture.instrument.snapshot ().presentation.tone.enabled,
                 "shutdown publishes no-tone intent");

        adk::BalanceInstrumentConfig invalid[7];
        for (auto& config : invalid)
        {
            config = instrumentConfig ();
        }
        invalid[0].minimumSensitivityPermille            = 0;
        invalid[1].minimumSensitivityPermille            = 1000;
        invalid[1].maximumSensitivityPermille            = 200;
        invalid[2].sensitivityStepPermille               = 0;
        invalid[3].inertialMaximumAge                    = adk::Duration ();
        invalid[4].inertialFreshnessContractRevision     = 0;
        invalid[5].maximumInputSkew                       = adk::Duration (100);
        invalid[6].diagnosticPhase                        = adk::Duration ();
        for (const auto& config : invalid)
        {
            Fixture rejected (config);

            requireStatus (rejected.instrument.initialize (),
                           adk::StatusCode::InvalidConfiguration,
                           "invalid project configuration is rejected");
            require (!rejected.instrument.initialized () && !rejected.storage.available,
                     "failed initialization leaves project and replay inert");
        }

        adk::BalancePresentationConfig invalidPresentation = presentationConfig ();
        invalidPresentation.fullScaleAngleMilliDegrees     = 0;
        Fixture policyRejected (instrumentConfig (), orientationConfig (),
                                invalidPresentation);
        requireStatus (policyRejected.instrument.initialize (),
                       adk::StatusCode::InvalidConfiguration,
                       "presentation preflight failure rejects initialization");
        require (!policyRejected.instrument.initialized () &&
                     !policyRejected.storage.available,
                 "policy preflight failure leaves owned policies and replay inert");
    }

    void testHappyPathFreezeAndSensitivity ()
    {
        Fixture fixture;
        require (fixture.instrument.initialize ().ok (), "happy fixture initializes");

        adk::BalanceInstrumentInput level = input (0, 1, 1, 1, 1);

        require (fixture.instrument.update (level).ok (), "level frame accepted");

        adk::BalanceInstrumentOutput output = fixture.instrument.snapshot ();

        require (output.mode == adk::BalanceInstrumentMode::Live &&
                     output.liveEvidence.available &&
                     output.liveEvidence.estimate.quality ==
                         adk::OrientationQuality::Level,
                 "first frame enters live level");
        require (output.sensitivityPermille == 600,
                 "sensitivity starts at midpoint toward minimum");

        adk::BalanceInstrumentInput tilted = input (1, 2, 2, 2, 2, 500000, 0, 866025);

        press (tilted);

        increase (tilted);

        require (fixture.instrument.update (tilted).ok (),
                 "same-frame freeze and sensitivity accepted");
        output = fixture.instrument.snapshot ();

        require (output.mode == adk::BalanceInstrumentMode::Frozen &&
                     output.frozenEvidence.available &&
                     output.frozenEvidence.provenance.sequence == 2 &&
                     output.sensitivityPermille == 800,
                 "freeze captures current frame before sensitivity rerender");

        adk::BalanceInstrumentInput changed = input (2, 3, 3, 3, 3, -500000, 0, 866025);

        require (fixture.instrument.update (changed).ok (),
                 "new live evidence advances while frozen");
        output = fixture.instrument.snapshot ();

        require (output.mode == adk::BalanceInstrumentMode::Frozen &&
                     output.liveEvidence.provenance.sequence == 3 &&
                     output.frozenEvidence.provenance.sequence == 2,
                 "frozen evidence remains stable");

        adk::BalanceInstrumentInput unfreeze =
            input (3, 4, 4, 4, 4, -500000, 0, 866025);
        press (unfreeze);

        require (fixture.instrument.update (unfreeze).ok (), "unfreeze accepted");

        output = fixture.instrument.snapshot ();

        require (output.mode == adk::BalanceInstrumentMode::Live &&
                     output.liveEvidence.provenance.sequence == 4,
                 "unfreeze selects newest live estimate");
    }

    void testControlValidationAndReplay ()
    {
        Fixture fixture;
        require (fixture.instrument.initialize ().ok (), "control fixture initializes");

        adk::BalanceInstrumentInput first = input (10, 1, 1, 1, 1);

        require (fixture.instrument.update (first).ok (), "baseline accepted");

        const adk::BalanceInstrumentOutput baseline = fixture.instrument.snapshot ();

        adk::BalanceInstrumentInput malformed = input (11, 2, 2, 2, 2);
        malformed.joystick.xPermille          = 1001;
        requireStatus (fixture.instrument.update (malformed),
                       adk::StatusCode::InvalidArgument,
                       "joystick upper bound enforced");
        require (outputEqual (fixture.instrument.snapshot (), baseline),
                 "invalid joystick frame is atomic");

        malformed                    = input (11, 2, 2, 2, 2);
        malformed.joystick.xPermille = 1;
        requireStatus (fixture.instrument.update (malformed),
                       adk::StatusCode::InvalidArgument, "event must match axis signs");
        require (outputEqual (fixture.instrument.snapshot (), baseline),
                 "event mismatch is atomic");

        malformed                           = input (11, 2, 2, 2, 2);
        malformed.freezeButton.pressed      = true;
        malformed.freezeButton.releaseEvent = true;
        requireStatus (fixture.instrument.update (malformed),
                       adk::StatusCode::InvalidArgument,
                       "button tuple must be consistent");

        malformed = input (11, 2, 2, 0, 2);

        requireStatus (fixture.instrument.update (malformed),
                       adk::StatusCode::InvalidArgument,
                       "zero producer sequence rejected");

        require (fixture.instrument.update (first).ok (),
                 "exact frame replay accepted");
        require (outputEqual (fixture.instrument.snapshot (), baseline),
                 "exact frame replay is stable");
        first.joystick.xPermille = 1;
        requireStatus (fixture.instrument.update (first),
                       adk::StatusCode::InvalidArgument,
                       "changed equal-identity replay rejected");

        adk::BalanceInstrumentInput producerReplay = input (12, 2, 2, 1, 2);
        producerReplay.joystick                    = fixture.storage.previous.joystick;
        producerReplay.joystick.observedAt         = adk::TimePoint (10);

        require (fixture.instrument.update (producerReplay).ok (),
                 "identical producer record may appear in later frame");
        require (fixture.instrument.snapshot ().sensitivityPermille == 600,
                 "producer replay does not reapply an event");
    }

    void testCompleteControlTupleMatrix ()
    {
        for (uint8_t tuple = 0; tuple < 8; ++tuple)
        {
            Fixture fixture;
            require (fixture.instrument.initialize ().ok (),
                     "button tuple fixture initializes");
            require (fixture.instrument.update (input (0, 1, 1, 1, 1)).ok (),
                     "button tuple baseline accepted");

            adk::BalanceInstrumentInput candidate = input (1, 2, 2, 2, 2);
            candidate.freezeButton.pressed        = (tuple & 4U) != 0;
            candidate.freezeButton.pressEvent     = (tuple & 2U) != 0;
            candidate.freezeButton.releaseEvent   = (tuple & 1U) != 0;
            const bool valid = tuple == 0 || tuple == 1 || tuple == 4 || tuple == 6;
            require (fixture.instrument.update (candidate).ok () == valid,
                     "each button tuple has its canonical disposition");
        }

        const int16_t axes[] = {-1000, 0, 1000};
        for (int16_t x : axes)
        {
            for (int16_t y : axes)
            {
                Fixture fixture;
                require (fixture.instrument.initialize ().ok (),
                         "joystick quadrant fixture initializes");
                require (fixture.instrument.update (input (0, 1, 1, 1, 1)).ok (),
                         "joystick quadrant baseline accepted");
                adk::BalanceInstrumentInput candidate = input (1, 2, 2, 2, 2);
                candidate.joystick.xPermille          = x;
                candidate.joystick.yPermille          = y;
                const bool positive = x > 0 || y > 0;
                const bool negative = x < 0 || y < 0;
                const adk::SensitivityEvent canonical =
                    positive && negative
                        ? adk::SensitivityEvent::Contradictory
                        : positive ? adk::SensitivityEvent::Increase
                                   : negative ? adk::SensitivityEvent::Decrease
                                              : adk::SensitivityEvent::None;
                candidate.joystick.event = canonical;
                require (fixture.instrument.update (candidate).ok (),
                         "every joystick center, edge, and corner canonicalizes");

                candidate = input (2, 3, 3, 3, 3);
                candidate.joystick.xPermille = x;
                candidate.joystick.yPermille = y;
                candidate.joystick.event =
                    canonical == adk::SensitivityEvent::None
                        ? adk::SensitivityEvent::Increase
                        : adk::SensitivityEvent::None;
                requireStatus (fixture.instrument.update (candidate),
                               adk::StatusCode::InvalidArgument,
                               "each joystick geometry rejects a mismatched event");
            }
        }
    }

    void testIndependentIdentityOrdering ()
    {
        enum struct Producer : uint8_t
        {
            Frame,
            Inertial,
            Joystick,
            Button
        };
        const Producer producers[] = {Producer::Frame, Producer::Inertial,
                                      Producer::Joystick, Producer::Button};
        for (Producer producer : producers)
        {
            Fixture fixture;
            require (fixture.instrument.initialize ().ok (),
                     "identity fixture initializes");
            adk::BalanceInstrumentInput baseline = input (10, 10, 10, 10, 10);

            require (fixture.instrument.update (baseline).ok (),
                     "identity baseline accepted");

            adk::BalanceInstrumentInput changed = input (11, 11, 11, 11, 11);
            if (producer == Producer::Frame)
            {
                changed.frameSequence = 10;
                changed.frameAt       = baseline.frameAt;
                changed.joystick.xPermille = 1;
            }
            else if (producer == Producer::Inertial)
            {
                changed.inertial.sample.sequence   = 10;
                changed.inertial.sample.observedAt = baseline.inertial.sample.observedAt;
            }
            else if (producer == Producer::Joystick)
            {
                changed.joystick.sequence   = 10;
                changed.joystick.observedAt = baseline.joystick.observedAt;
                changed.joystick.xPermille  = 1;
            }
            else
            {
                changed.freezeButton.sequence   = 10;
                changed.freezeButton.observedAt = baseline.freezeButton.observedAt;
                changed.freezeButton.pressed    = true;
            }
            requireStatus (fixture.instrument.update (changed),
                           adk::StatusCode::InvalidArgument,
                           "changed delta-zero identity is rejected independently");

            adk::BalanceInstrumentInput half = input (11, 11, 11, 11, 11);
            if (producer == Producer::Frame)
                half.frameSequence = 10U + 0x80000000U;
            else if (producer == Producer::Inertial)
                half.inertial.sample.sequence = 10U + 0x80000000U;
            else if (producer == Producer::Joystick)
                half.joystick.sequence = 10U + 0x80000000U;
            else
                half.freezeButton.sequence = 10U + 0x80000000U;
            requireStatus (fixture.instrument.update (half),
                           adk::StatusCode::InvalidArgument,
                           "ambiguous half-range identity is rejected independently");

            adk::BalanceInstrumentInput regression = input (11, 11, 11, 11, 11);
            if (producer == Producer::Frame)
                regression.frameSequence = 9;
            else if (producer == Producer::Inertial)
                regression.inertial.sample.sequence = 9;
            else if (producer == Producer::Joystick)
                regression.joystick.sequence = 9;
            else
                regression.freezeButton.sequence = 9;
            requireStatus (fixture.instrument.update (regression),
                           adk::StatusCode::InvalidArgument,
                           "identity regression is rejected independently");
        }

        Fixture equalTime;
        require (equalTime.instrument.initialize ().ok (),
                 "equal-time fixture initializes");
        require (equalTime.instrument.update (input (10, 1, 1, 1, 1)).ok (),
                 "equal-time baseline accepted");
        requireStatus (equalTime.instrument.update (input (10, 2, 2, 2, 2)),
                       adk::StatusCode::InvalidArgument,
                       "forward sequence with equal frame time is rejected");
    }

    void testFreshnessSkewAndAtomicity ()
    {
        Fixture fixture;
        require (fixture.instrument.initialize ().ok (),
                 "freshness fixture initializes");
        require (fixture.instrument.update (input (100, 1, 1, 1, 1)).ok (),
                 "fresh baseline accepted");
        const adk::BalanceInstrumentOutput baseline = fixture.instrument.snapshot ();
        const adk::BalanceFrameStorage     stored   = fixture.storage;

        adk::BalanceInstrumentInput changed = input (101, 2, 2, 2, 2);

        changed.inertial.maximumAge         = adk::Duration (99);

        requireStatus (fixture.instrument.update (changed),
                       adk::StatusCode::InvalidConfiguration,
                       "freshness maximum mismatch rejected");
        require (outputEqual (fixture.instrument.snapshot (), baseline) &&
                     fixture.storage.available == stored.available &&
                     fixture.storage.previous.frameSequence ==
                         stored.previous.frameSequence,
                 "configuration rejection precedes owned policy mutation");

        changed                                    = input (101, 2, 2, 2, 2);
        changed.inertial.freshnessContractRevision = 8;
        requireStatus (fixture.instrument.update (changed),
                       adk::StatusCode::InvalidConfiguration,
                       "freshness revision mismatch rejected");

        changed                            = input (301, 2, 2, 2, 2);

        changed.inertial.sample.observedAt = adk::TimePoint (100);

        changed.inertial.age               = adk::Duration (201);
        changed.inertial.quality           = adk::InertialSampleQuality::Stale;
        changed.inertial.latestDataReady   = false;
        changed.joystick.observedAt        = adk::TimePoint (100);

        changed.freezeButton.observedAt    = adk::TimePoint (100);

        requireStatus (fixture.instrument.update (changed),
                       adk::StatusCode::InvalidArgument,
                       "one tick beyond input skew faults");
        require (!fixture.instrument.snapshot ().presentation.tone.enabled,
                 "skew fault disables tone");

        Fixture aging;
        require (aging.instrument.initialize ().ok (), "aging fixture initializes");

        adk::BalanceInstrumentInput ready = input (0, 1, 1, 1, 1);

        require (aging.instrument.update (ready).ok (), "ready sample accepted");

        adk::BalanceInstrumentInput stale = input (101, 2, 1, 2, 2);
        stale.inertial.sample             = ready.inertial.sample;
        stale.inertial.latestDataReady    = true;
        stale.inertial.age                = adk::Duration (101);
        stale.inertial.quality            = adk::InertialSampleQuality::Stale;
        stale.inertial.sequenceGap        = 0;
        require (aging.instrument.update (stale).ok (),
                 "same sample ages stale in later frame");
        require (aging.instrument.snapshot ().liveEvidence.provenance.quality ==
                         adk::InertialSampleQuality::Stale &&
                     !aging.instrument.snapshot ().presentation.tone.enabled,
                 "stale evidence remains visible and silent");

        stale.frameAt                  = adk::TimePoint (102);
        stale.frameSequence            = 3;
        stale.joystick.observedAt      = adk::TimePoint (102);
        stale.joystick.sequence        = 3;
        stale.freezeButton.observedAt  = adk::TimePoint (102);
        stale.freezeButton.sequence    = 3;
        stale.inertial.latestDataReady = false;
        stale.inertial.age             = adk::Duration (102);
        stale.inertial.quality         = adk::InertialSampleQuality::Stale;
        require (aging.instrument.update (stale).ok (),
                 "latest not-ready retains accepted payload");
        require (
            aging.instrument.snapshot ().liveEvidence.provenance.sequence == 1 &&
                !aging.instrument.snapshot ().liveEvidence.provenance.latestDataReady,
            "not-ready evidence fabricates no sample or gap");
    }

    void testFaultAcknowledgementAndRecovery ()
    {
        Fixture fixture;
        require (fixture.instrument.initialize ().ok (),
                 "recovery fixture initializes");
        require (fixture.instrument.update (input (0, 1, 1, 1, 1)).ok (),
                 "recovery baseline accepted");

        adk::BalanceInstrumentInput fault = input (1, 2, 2, 2, 2);
        fault.freezeButton.status         = adk::StatusCode::HardwareFailure;
        requireStatus (fixture.instrument.update (fault),
                       adk::StatusCode::HardwareFailure,
                       "structured producer fault is admitted and reported");
        require (fixture.instrument.snapshot ().mode ==
                         adk::BalanceInstrumentMode::Fault &&
                     fixture.instrument.snapshot ().buttonStatus.error () ==
                         adk::StatusCode::HardwareFailure &&
                     !fixture.instrument.snapshot ().presentation.tone.enabled,
                 "button fault latches canonical safe output");
        requireStatus (fixture.instrument.acknowledgeFault (),
                       adk::StatusCode::InvalidArgument,
                       "fault cannot acknowledge on unhealthy evidence");

        adk::BalanceInstrumentInput healthy = input (2, 3, 3, 3, 3);

        requireStatus (fixture.instrument.update (healthy),
                       adk::StatusCode::HardwareFailure,
                       "healthy frame updates evidence while reporting latch");
        const adk::BalanceInstrumentOutput beforeAck = fixture.instrument.snapshot ();
        const uint32_t replaySequence = fixture.storage.previous.frameSequence;
        require (fixture.instrument.acknowledgeFault ().ok (),
                 "healthy latest frame permits acknowledgement");
        adk::BalanceInstrumentOutput recovering = fixture.instrument.snapshot ();

        require (recovering.mode == adk::BalanceInstrumentMode::Recovering &&
                     evidenceEqual (recovering.liveEvidence, beforeAck.liveEvidence) &&
                     recovering.sensitivityPermille == beforeAck.sensitivityPermille &&
                     fixture.storage.previous.frameSequence == replaySequence &&
                     !recovering.presentation.tone.enabled,
                 "acknowledgement preserves evidence and replay identity");

        require (fixture.instrument.update (healthy).ok (),
                 "exact pre-ack replay remains idempotent");
        require (fixture.instrument.snapshot ().mode ==
                     adk::BalanceInstrumentMode::Recovering,
                 "exact replay cannot finish recovery");

        adk::BalanceInstrumentInput recovered = input (3, 4, 4, 4, 4);

        press (recovered);

        increase (recovered);

        require (fixture.instrument.update (recovered).ok (),
                 "forward healthy recovery accepted");
        require (fixture.instrument.snapshot ().mode ==
                         adk::BalanceInstrumentMode::Live &&
                     fixture.instrument.snapshot ().sensitivityPermille ==
                         recovering.sensitivityPermille &&
                     !fixture.instrument.snapshot ().frozenEvidence.available,
                 "recovery frame suppresses freeze and sensitivity controls");
    }

    void testFaultPrecedenceAndRelatch ()
    {
        struct Collision
        {
            adk::StatusCode inertial;
            adk::StatusCode button;
            adk::StatusCode joystick;
            adk::StatusCode expected;
        };
        const Collision collisions[] = {
            {adk::StatusCode::HardwareFailure, adk::StatusCode::InvalidArgument,
             adk::StatusCode::NotInitialized,
             adk::StatusCode::HardwareFailure},
            {adk::StatusCode::Ok, adk::StatusCode::HardwareFailure,
             adk::StatusCode::InvalidArgument,
             adk::StatusCode::HardwareFailure},
            {adk::StatusCode::Ok, adk::StatusCode::Ok,
             adk::StatusCode::HardwareFailure,
             adk::StatusCode::HardwareFailure}};
        for (const Collision& collision : collisions)
        {
            Fixture fixture;
            require (fixture.instrument.initialize ().ok (),
                     "fault collision fixture initializes");
            require (fixture.instrument.update (input (0, 1, 1, 1, 1)).ok (),
                     "fault collision baseline accepted");
            adk::BalanceInstrumentInput fault = input (1, 2, 2, 2, 2);
            fault.inertial.sample.status      = collision.inertial;
            fault.inertial.status             = collision.inertial;
            fault.inertial.quality =
                collision.inertial == adk::StatusCode::Ok
                    ? adk::InertialSampleQuality::Current
                    : adk::InertialSampleQuality::Invalid;
            fault.freezeButton.status = collision.button;
            fault.joystick.status     = collision.joystick;
            requireStatus (fixture.instrument.update (fault), collision.expected,
                           "producer fault precedence is deterministic");
            require (fixture.instrument.snapshot ().inertialStatus.error () ==
                             collision.inertial &&
                         fixture.instrument.snapshot ().buttonStatus.error () ==
                             collision.button &&
                         fixture.instrument.snapshot ().joystickStatus.error () ==
                             collision.joystick,
                     "fault collision preserves each producer status");
        }

        Fixture skew;
        require (skew.instrument.initialize ().ok (),
                 "skew replay fixture initializes");
        require (skew.instrument.update (input (0, 1, 1, 1, 1)).ok (),
                 "skew replay baseline accepted");
        adk::BalanceInstrumentInput delayed = input (201, 2, 2, 2, 2);

        delayed.inertial.sample.observedAt = adk::TimePoint (1);

        delayed.inertial.age = adk::Duration (200);

        delayed.inertial.quality = adk::InertialSampleQuality::Stale;

        delayed.joystick.observedAt = adk::TimePoint (0);

        delayed.freezeButton.observedAt = adk::TimePoint (0);

        requireStatus (skew.instrument.update (delayed),
                       adk::StatusCode::InvalidArgument,
                       "skew fault commits canonical safe frame");
        const adk::BalanceInstrumentOutput faulted = skew.instrument.snapshot ();

        requireStatus (skew.instrument.update (delayed),
                       adk::StatusCode::InvalidArgument,
                       "exact skew replay reports latched result");
        require (outputEqual (skew.instrument.snapshot (), faulted),
                 "exact skew replay is idempotent");

        adk::BalanceInstrumentInput healthy = input (202, 3, 3, 3, 3);

        requireStatus (skew.instrument.update (healthy),
                       adk::StatusCode::InvalidArgument,
                       "healthy frame preserves skew fault latch");
        require (skew.instrument.acknowledgeFault ().ok (),
                 "healthy frame permits skew acknowledgement");
        adk::BalanceInstrumentInput relatched = input (403, 4, 4, 4, 4);

        relatched.joystick.observedAt = adk::TimePoint (202);

        requireStatus (skew.instrument.update (relatched),
                       adk::StatusCode::InvalidArgument,
                       "fault during recovery relatches");
        require (skew.instrument.snapshot ().mode ==
                     adk::BalanceInstrumentMode::Fault,
                 "recovery fault returns to fault mode");
    }

    void testSensitivityClampsAndSuppression ()
    {
        Fixture fixture;
        require (fixture.instrument.initialize ().ok (),
                 "sensitivity fixture initializes");
        require (fixture.instrument.update (input (0, 1, 1, 1, 1)).ok (),
                 "sensitivity baseline accepted");
        uint32_t sequence = 2;
        for (uint8_t step = 0; step < 4; ++step)
        {
            adk::BalanceInstrumentInput candidate =
                input (sequence, sequence, sequence, sequence, sequence);
            increase (candidate);

            require (fixture.instrument.update (candidate).ok (),
                     "sensitivity increase accepted");
            ++sequence;
        }
        require (fixture.instrument.snapshot ().sensitivityPermille == 1000,
                 "sensitivity increase clamps at maximum");
        for (uint8_t step = 0; step < 6; ++step)
        {
            adk::BalanceInstrumentInput candidate =
                input (sequence, sequence, sequence, sequence, sequence);
            decrease (candidate);

            require (fixture.instrument.update (candidate).ok (),
                     "sensitivity decrease accepted");
            ++sequence;
        }
        require (fixture.instrument.snapshot ().sensitivityPermille == 200,
                 "sensitivity decrease clamps at minimum");

        adk::BalanceInstrumentInput unsteady =
            input (sequence, sequence, sequence, sequence, sequence, 0, 0, 0);
        increase (unsteady);

        require (fixture.instrument.update (unsteady).ok (),
                 "unsteady orientation is admitted");
        require (fixture.instrument.snapshot ().sensitivityPermille == 200 &&
                     !fixture.instrument.snapshot ().presentation.tone.enabled,
                 "unsteady orientation suppresses controls and tone");
        ++sequence;

        adk::BalanceInstrumentInput beyond =
            input (sequence, sequence, sequence, sequence, sequence, 0, 0, -1000000);
        increase (beyond);

        require (fixture.instrument.update (beyond).ok (),
                 "beyond-presentation-range orientation is admitted");
        const adk::BalanceInstrumentOutput beyondOutput =
            fixture.instrument.snapshot ();

        require (beyondOutput.liveEvidence.estimate.quality ==
                         adk::OrientationQuality::BeyondPresentationRange &&
                     beyondOutput.sensitivityPermille == 200 &&
                     !beyondOutput.presentation.tone.enabled,
                 "beyond-presentation-range orientation suppresses controls and tone");
    }

    void testIneligibleEvidenceAndOwnedPolicyAtomicity ()
    {
        Fixture fixture;
        require (fixture.instrument.initialize ().ok (),
                 "ineligible fixture initializes");
        require (fixture.instrument.update (input (0, 1, 1, 1, 1)).ok (),
                 "ineligible baseline accepted");

        adk::BalanceInstrumentInput stale = input (101, 2, 1, 2, 2);
        stale.inertial.sample             = fixture.storage.previous.inertial.sample;
        stale.inertial.age                = adk::Duration (101);
        stale.inertial.quality            = adk::InertialSampleQuality::Stale;
        stale.inertial.sequenceGap        = 0;
        stale.joystick.xPermille          = 1000;
        stale.joystick.event              = adk::SensitivityEvent::Increase;
        require (fixture.instrument.update (stale).ok (),
                 "valid stale evidence is admitted");
        require (fixture.instrument.snapshot ().liveEvidence.provenance.quality ==
                         adk::InertialSampleQuality::Stale &&
                     fixture.instrument.snapshot ().sensitivityPermille == 600 &&

                     !fixture.instrument.snapshot ().presentation.tone.enabled,
                 "stale evidence suppresses controls and tone");

        adk::BalanceInstrumentInput saturated = input (102, 3, 2, 3, 3, 2000000, 0, 0);
        saturated.inertial.sample.saturation  = adk::InertialSaturation::Acceleration;
        saturated.inertial.quality            = adk::InertialSampleQuality::Saturated;
        increase (saturated);

        require (fixture.instrument.update (saturated).ok (),
                 "valid saturated evidence is admitted");
        require (fixture.instrument.snapshot ().liveEvidence.provenance.quality ==
                         adk::InertialSampleQuality::Saturated &&
                     fixture.instrument.snapshot ().sensitivityPermille == 600 &&

                     !fixture.instrument.snapshot ().presentation.tone.enabled,
                 "saturation suppresses controls and tone");

        const adk::BalanceInstrumentOutput before = fixture.instrument.snapshot ();
        const uint32_t storedSequence      = fixture.storage.previous.frameSequence;
        adk::BalanceInstrumentInput forged = input (103, 4, 3, 4, 4);
        forged.inertial.sample.dataReady   = false;
        requireStatus (fixture.instrument.update (forged),
                       adk::StatusCode::InvalidArgument,
                       "forged readiness relation is rejected");
        require (outputEqual (fixture.instrument.snapshot (), before) &&
                     fixture.storage.previous.frameSequence == storedSequence,
                 "forged readiness cannot mutate owned policies or project state");

        adk::BalanceInstrumentInput fault = input (104, 4, 3, 4, 4);
        fault.inertial.sample.status      = adk::StatusCode::HardwareFailure;
        fault.inertial.quality            = adk::InertialSampleQuality::Invalid;
        fault.inertial.status             = adk::StatusCode::HardwareFailure;
        requireStatus (fixture.instrument.update (fault),
                       adk::StatusCode::HardwareFailure,
                       "inertial producer fault commits and reports");
        adk::BalanceInstrumentInput healthy = input (105, 5, 4, 5, 5);

        requireStatus (fixture.instrument.update (healthy),
                       adk::StatusCode::HardwareFailure,
                       "healthy evidence does not auto-clear fault");
        require (fixture.instrument.acknowledgeFault ().ok (),
                 "healthy evidence acknowledges fault");
        adk::BalanceInstrumentInput recoveringStale = input (206, 6, 4, 6, 6);
        recoveringStale.inertial.sample             = healthy.inertial.sample;
        recoveringStale.inertial.age                = adk::Duration (101);
        recoveringStale.inertial.quality            = adk::InertialSampleQuality::Stale;
        recoveringStale.inertial.sequenceGap        = 0;
        increase (recoveringStale);

        require (fixture.instrument.update (recoveringStale).ok (),
                 "nonfault ineligible recovery frame is admitted");
        require (fixture.instrument.snapshot ().mode ==
                         adk::BalanceInstrumentMode::Recovering &&
                     fixture.instrument.snapshot ().sensitivityPermille == 600 &&

                     !fixture.instrument.snapshot ().presentation.tone.enabled,
                 "ineligible frame remains recovering and suppresses controls");
    }

    void testFrozenEvidenceFaultLifetime ()
    {
        Fixture fixture;
        require (fixture.instrument.initialize ().ok (),
                 "frozen lifetime fixture initializes");
        adk::BalanceInstrumentInput frozen =
            input (0, 1, 1, 1, 1, 500000, 0, 866025);
        press (frozen);

        require (fixture.instrument.update (frozen).ok (),
                 "frozen lifetime baseline accepted");
        const adk::BalanceMeasurementEvidence evidence =
            fixture.instrument.snapshot ().frozenEvidence;

        adk::BalanceInstrumentInput fault = input (1, 2, 2, 2, 2);
        fault.joystick.status             = adk::StatusCode::HardwareFailure;
        requireStatus (fixture.instrument.update (fault),
                       adk::StatusCode::HardwareFailure,
                       "fault after freeze is admitted");
        require (evidenceEqual (fixture.instrument.snapshot ().frozenEvidence,
                                evidence),
                 "fault preserves frozen measurement evidence");
        adk::BalanceInstrumentInput healthy = input (2, 3, 3, 3, 3);

        requireStatus (fixture.instrument.update (healthy),
                       adk::StatusCode::HardwareFailure,
                       "healthy evidence retains fault and frozen evidence");
        require (evidenceEqual (fixture.instrument.snapshot ().frozenEvidence,
                                evidence),
                 "latched fault preserves frozen evidence");
        require (fixture.instrument.acknowledgeFault ().ok (),
                 "frozen lifetime fault acknowledges");
        require (evidenceEqual (fixture.instrument.snapshot ().frozenEvidence,
                                evidence),
                 "recovery preserves frozen evidence");
        fixture.instrument.shutdown ();

        require (!fixture.instrument.snapshot ().frozenEvidence.available &&
                     !fixture.storage.available,
                 "shutdown clears frozen and replay evidence");
    }

    void testOwnedPolicyPreflightAtomicity ()
    {
        Fixture tested;
        Fixture reference;
        require (tested.instrument.initialize ().ok () &&
                     reference.instrument.initialize ().ok (),
                 "policy atomicity fixtures initialize");
        const adk::BalanceInstrumentInput first =
            input (0, 1, 1, 1, 1, 500000, 0, 866025);
        require (tested.instrument.update (first).ok () &&
                     reference.instrument.update (first).ok (),
                 "policy atomicity baselines accepted");

        adk::BalanceInstrumentInput rejected =
            input (1, 2, 1, 2, 2, -500000, 0, 866025);
        rejected.inertial.sample.observedAt =
            tested.storage.previous.inertial.sample.observedAt;
        requireStatus (tested.instrument.update (rejected),
                       adk::StatusCode::InvalidArgument,
                       "changed inertial delta-zero candidate is rejected");

        const adk::BalanceInstrumentInput valid =
            input (1, 2, 2, 2, 2, 500000, 0, 866025);
        require (tested.instrument.update (valid).ok () &&
                     reference.instrument.update (valid).ok (),
                 "same-direction successor accepted by both fixtures");
        require (outputEqual (tested.instrument.snapshot (),
                              reference.instrument.snapshot ()) &&
                     tested.storage.previous.frameSequence ==
                         reference.storage.previous.frameSequence,
                 "preflight failure preserves orientation and presentation policy state");
    }

    void testAdmittedSafeStateResetsPolicyHistory ()
    {
        Fixture fixture;
        require (fixture.instrument.initialize ().ok (),
                 "safe-state history fixture initializes");
        const adk::BalanceInstrumentInput right =
            input (0, 1, 1, 1, 1, 500000, 0, 866025);

        require (fixture.instrument.update (right).ok (),
                 "right-direction baseline accepted");
        require (fixture.instrument.snapshot ().presentation.direction ==
                     adk::BalanceDirection::Right,
                 "baseline establishes right presentation history");

        adk::BalanceInstrumentInput unsteady = input (1, 2, 2, 2, 2, 0, 0, 0);

        require (fixture.instrument.update (unsteady).ok (),
                 "unsteady safe-state frame admitted");
        require (fixture.instrument.snapshot ().presentation.direction ==
                         adk::BalanceDirection::None &&
                     !fixture.instrument.snapshot ().presentation.tone.enabled,
                 "unsteady frame commits canonical no-direction history");

        const adk::BalanceInstrumentInput left =
            input (2, 3, 3, 3, 3, -500000, 0, 866025);

        require (fixture.instrument.update (left).ok (),
                 "left successor accepted after safe-state reset");
        require (fixture.instrument.snapshot ().presentation.direction ==
                         adk::BalanceDirection::Left &&
                     !fixture.instrument.snapshot ().presentation.tone.enabled,
                 "safe-state reset prevents a spurious right-to-left tone");
    }

    void testTimingWrapAndStorage ()
    {
        Fixture fixture;
        require (fixture.instrument.initialize ().ok (), "timing fixture initializes");

        const uint32_t beforeWrap = std::numeric_limits<uint32_t>::max () - 5U;

        require (fixture.instrument
                     .update (input (beforeWrap, 0xFFFFFFFEU, 0xFFFFFFFEU, 0xFFFFFFFEU,
                                     0xFFFFFFFEU))
                     .ok (),
                 "pre-wrap frame accepted");
        require (
            fixture.instrument
                .update (input (4, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU))

                .ok (),
            "ordinary timestamp rollover accepted");
        adk::BalanceInstrumentInput wrapped = input (5, 1, 1, 1, 1);
        wrapped.inertial.sequenceGap        = 1;
        require (fixture.instrument.update (wrapped).ok (),
                 "sequence rollover skips forbidden zero");
        const adk::BalanceInstrumentOutput stable = fixture.instrument.snapshot ();

        adk::BalanceInstrumentInput halfRange =
            input (5U + 0x80000000U, 1U + 0x80000000U, 2, 2, 2);
        requireStatus (fixture.instrument.update (halfRange),
                       adk::StatusCode::InvalidArgument,
                       "ambiguous half-range rejected");
        require (outputEqual (fixture.instrument.snapshot (), stable),
                 "time rejection preserves output");

        require (fixture.storage.available, "accepted frame is durably copied");

        fixture.instrument.shutdown ();

        require (!fixture.storage.available, "shutdown releases replay lifetime");
    }

    void testSourceDomainTransitions ()
    {
        for (uint8_t member = 0; member < 5; ++member)
        {
            Fixture fixture;
            require (fixture.instrument.initialize ().ok (),
                     "source-domain fixture initializes");
            require (fixture.instrument.update (input (10, 1, 50, 1, 1)).ok (),
                     "source-domain baseline accepted");

            adk::BalanceInstrumentInput changed = input (11, 2, 1, 2, 2);
            if (member == 0)
                changed.inertial.sample.source.sourceId = 2;
            else if (member == 1)
                changed.inertial.sample.source.configurationRevision = 4;
            else if (member == 2)
                changed.inertial.sample.source.calibrationRevision = 4;
            else if (member == 3)
                changed.inertial.sample.source.accelerationRangeMicroG = 3000000;
            else
                changed.inertial.sample.source
                    .angularRateRangeMilliDegreesPerSecond = 500000;

            require (fixture.instrument.update (changed).ok (),
                     "each valid source-domain change starts a fresh sequence domain");
            require (fixture.instrument.snapshot ().liveEvidence.provenance.sequence ==
                             1 &&
                         fixture.storage.previous.inertial.sequenceGap == 0,
                     "fresh source domain admits lower sequence with gap zero");
        }

        for (uint8_t member = 0; member < 2; ++member)
        {
            Fixture fixture;
            require (fixture.instrument.initialize ().ok (),
                     "unsupported source fixture initializes");
            require (fixture.instrument.update (input (10, 1, 1, 1, 1)).ok (),
                     "unsupported source baseline accepted");
            adk::BalanceInstrumentInput changed = input (11, 2, 1, 2, 2);
            if (member == 0)
                changed.inertial.sample.source.kind =
                    adk::InertialSourceKind::Mpu6050Adapter;
            else
                changed.inertial.sample.source.model = adk::InertialModel::Mpu6050;
            requireStatus (fixture.instrument.update (changed),
                           adk::StatusCode::InvalidArgument,
                           "non-E0 source kind and model remain rejected");
        }

        Fixture sameDomain;
        require (sameDomain.instrument.initialize ().ok (),
                 "same-domain fixture initializes");
        require (sameDomain.instrument.update (input (10, 1, 1, 1, 1)).ok (),
                 "same-domain baseline accepted");

        adk::BalanceInstrumentInput mutation = input (11, 2, 1, 2, 2, 1, 0, 1000000);

        requireStatus (sameDomain.instrument.update (mutation),
                       adk::StatusCode::InvalidArgument,
                       "same sequence with changed sample is rejected in one domain");
    }

    void testCanonicalProjectPresentations ()
    {
        for (uint8_t channel = 0; channel < 4; ++channel)
        {
            for (uint8_t field = 0; field < 8; ++field)
            {
                adk::BalanceInstrumentConfig config = instrumentConfig ();
                adk::BalancePresentation* presentation =
                    channel == 0   ? &config.awaitingFramePresentation
                    : channel == 1 ? &config.recoveringPresentation
                    : channel == 2 ? &config.faultPresentation
                                   : &config.shutdownPresentation;
                if (field == 0)
                    presentation->direction = adk::BalanceDirection::Left;
                else if (field == 1)
                    presentation->light.redPermille = 1001;
                else if (field == 2)
                    presentation->light.greenPermille = 1001;
                else if (field == 3)
                    presentation->light.bluePermille = 1001;
                else if (field == 4)
                    presentation->light.fault = channel != 2;
                else if (field == 5)
                    presentation->tone.enabled = true;
                else if (field == 6)
                    presentation->tone.frequencyHertz = 1;
                else
                    presentation->tone.durationMilliseconds = 1;
                Fixture rejected (config);

                requireStatus (rejected.instrument.initialize (),
                               adk::StatusCode::InvalidConfiguration,
                               "every canonical project channel field is validated");
            }
        }
    }

    void testShutdownFromEveryMode ()
    {
        for (uint8_t target = 0; target < 5; ++target)
        {
            Fixture fixture;
            require (fixture.instrument.initialize ().ok (),
                     "shutdown-mode fixture initializes");
            if (target != 0)
            {
                adk::BalanceInstrumentInput first = input (0, 1, 1, 1, 1);
                if (target == 2)
                    press (first);
                if (target == 4)
                    first.joystick.status = adk::StatusCode::HardwareFailure;
                fixture.instrument.update (first);
                if (target == 3)
                {
                    adk::BalanceInstrumentInput fault = input (1, 2, 2, 2, 2);
                    fault.joystick.status = adk::StatusCode::HardwareFailure;
                    fixture.instrument.update (fault);

                    adk::BalanceInstrumentInput healthy = input (2, 3, 3, 3, 3);

                    fixture.instrument.update (healthy);

                    require (fixture.instrument.acknowledgeFault ().ok (),
                             "recovering shutdown fixture reaches recovering");
                }
            }
            require (fixture.instrument.snapshot ().mode ==
                         static_cast<adk::BalanceInstrumentMode> (target),
                     "fixture reaches requested mode before shutdown");
            fixture.instrument.shutdown ();

            require (!fixture.instrument.initialized () && !fixture.storage.available &&
                         fixture.instrument.snapshot ().status.error () ==
                             adk::StatusCode::NotInitialized,
                     "shutdown clears every mode to canonical inert state");
            require (fixture.instrument.initialize ().ok () &&
                         fixture.instrument.update (input (10, 1, 1, 1, 1)).ok (),
                     "every shutdown mode restarts from a clean domain");
        }
    }

    void testTemporalAndDerivedFieldBoundaries ()
    {
        for (uint8_t producer = 0; producer < 3; ++producer)
        {
            Fixture fixture;
            require (fixture.instrument.initialize ().ok (),
                     "future-time fixture initializes");
            adk::BalanceInstrumentInput candidate = input (100, 1, 1, 1, 1);
            if (producer == 0)
                candidate.inertial.sample.observedAt = adk::TimePoint (101);
            else if (producer == 1)
                candidate.joystick.observedAt = adk::TimePoint (101);
            else
                candidate.freezeButton.observedAt = adk::TimePoint (101);
            requireStatus (fixture.instrument.update (candidate),
                           adk::StatusCode::InvalidArgument,
                           "each future producer timestamp is rejected");
        }

        Fixture exactSkew;
        require (exactSkew.instrument.initialize ().ok (),
                 "exact-skew fixture initializes");

        adk::BalanceInstrumentInput edge = input (200, 1, 1, 1, 1);

        edge.inertial.sample.observedAt   = adk::TimePoint (0);

        edge.inertial.age                 = adk::Duration (200);
        edge.inertial.quality             = adk::InertialSampleQuality::Stale;
        edge.joystick.observedAt          = adk::TimePoint (0);
        edge.freezeButton.observedAt      = adk::TimePoint (0);

        require (exactSkew.instrument.update (edge).ok (),
                 "exact maximum skew remains admitted");

        for (uint8_t field = 0; field < 5; ++field)
        {
            Fixture fixture;
            require (fixture.instrument.initialize ().ok (),
                     "derived-forgery fixture initializes");
            adk::BalanceInstrumentInput forged = input (10, 1, 1, 1, 1);
            if (field == 0)
                forged.inertial.age = adk::Duration (1);
            else if (field == 1)
                forged.inertial.sequenceGap = 1;
            else if (field == 2)
                forged.inertial.quality = adk::InertialSampleQuality::Stale;
            else if (field == 3)
                forged.inertial.latestDataReady = false;
            else
                forged.inertial.status = adk::StatusCode::HardwareFailure;
            requireStatus (fixture.instrument.update (forged),
                           adk::StatusCode::InvalidArgument,
                           "each forged derived observation field is rejected");
        }
    }

    void testDiagnosticPhaseEdgesAndFreezeAuthority ()
    {
        Fixture fixture;
        require (fixture.instrument.initialize ().ok (),
                 "diagnostic-phase fixture initializes");

        adk::BalanceInstrumentInput phaseA = input (0, 1, 1, 1, 1, 0, 0, 0);

        press (phaseA);

        require (fixture.instrument.update (phaseA).ok (),
                 "ineligible press frame is admitted");
        require (fixture.instrument.snapshot ().mode ==
                         adk::BalanceInstrumentMode::Live &&
                     fixture.instrument.snapshot ().presentation.light.bluePermille ==
                         300,
                 "ineligible evidence cannot freeze and begins phase A");

        adk::BalanceInstrumentInput beforeEdge = input (19, 2, 2, 2, 2, 0, 0, 0);

        require (fixture.instrument.update (beforeEdge).ok (),
                 "frame before diagnostic edge accepted");
        require (fixture.instrument.snapshot ().presentation.light.bluePermille == 300,
                 "phase A holds through the tick before its edge");

        adk::BalanceInstrumentInput phaseB = input (20, 3, 3, 3, 3, 0, 0, 0);

        require (fixture.instrument.update (phaseB).ok (),
                 "phase B edge accepted");
        require (fixture.instrument.snapshot ().presentation.light.bluePermille == 700,
                 "exact diagnostic edge selects phase B");

        adk::BalanceInstrumentInput secondA = input (40, 4, 4, 4, 4, 0, 0, 0);

        require (fixture.instrument.update (secondA).ok (),
                 "second phase A edge accepted");

        const adk::BalanceInstrumentOutput stable = fixture.instrument.snapshot ();

        require (stable.presentation.light.bluePermille == 300,
                 "two phases return exactly to phase A");
        require (fixture.instrument.update (secondA).ok () &&
                     outputEqual (fixture.instrument.snapshot (), stable),
                 "exact replay neither advances nor rephases diagnostics");
    }

    void testConfiguredProjectIntentTransitions ()
    {
        adk::BalanceInstrumentConfig config = instrumentConfig ();
        config.awaitingFramePresentation.light = {11, 12, 13, false};
        config.recoveringPresentation.light    = {21, 22, 23, false};
        config.faultPresentation.light         = {31, 32, 33, true};
        config.shutdownPresentation.light      = {41, 42, 43, false};
        Fixture fixture (config);

        require (fixture.instrument.initialize ().ok () &&
                     lightEqual (fixture.instrument.snapshot ().presentation.light,
                                 config.awaitingFramePresentation.light),
                 "awaiting-frame transition publishes its configured intent exactly");

        adk::BalanceInstrumentInput fault = input (0, 1, 1, 1, 1);
        fault.joystick.status = adk::StatusCode::HardwareFailure;
        requireStatus (fixture.instrument.update (fault),
                       adk::StatusCode::HardwareFailure,
                       "configured-intent fixture enters fault");
        require (lightEqual (fixture.instrument.snapshot ().presentation.light,
                             config.faultPresentation.light),
                 "fault transition publishes its configured intent exactly");

        adk::BalanceInstrumentInput healthy = input (1, 2, 2, 2, 2);

        requireStatus (fixture.instrument.update (healthy),
                       adk::StatusCode::HardwareFailure,
                       "healthy evidence preserves configured fault");
        require (fixture.instrument.acknowledgeFault ().ok () &&
                     lightEqual (fixture.instrument.snapshot ().presentation.light,
                                 config.recoveringPresentation.light),
                 "acknowledgement publishes configured recovery intent exactly");

        fixture.instrument.shutdown ();

        require (lightEqual (fixture.instrument.snapshot ().presentation.light,
                             config.shutdownPresentation.light),
                 "shutdown publishes its configured intent exactly");
    }

    void testRecoveryHealthExclusions ()
    {
        for (uint8_t exclusion = 0; exclusion < 6; ++exclusion)
        {
            Fixture fixture;
            require (fixture.instrument.initialize ().ok (),
                     "recovery-exclusion fixture initializes");
            adk::BalanceInstrumentInput fault = input (0, 1, 1, 1, 1);
            fault.freezeButton.status = adk::StatusCode::HardwareFailure;
            requireStatus (fixture.instrument.update (fault),
                           adk::StatusCode::HardwareFailure,
                           "recovery-exclusion fixture faults");

            adk::BalanceInstrumentInput healthy = input (1, 2, 2, 2, 2);

            requireStatus (fixture.instrument.update (healthy),
                           adk::StatusCode::HardwareFailure,
                           "recovery-exclusion fixture records healthy evidence");
            require (fixture.instrument.acknowledgeFault ().ok (),
                     "recovery-exclusion fixture acknowledges");

            adk::BalanceInstrumentInput excluded = input (2, 3, 3, 3, 3);
            if (exclusion == 0)
            {
                excluded.inertial.sample = healthy.inertial.sample;
                excluded.inertial.age = adk::Duration (1);
                excluded.inertial.quality = adk::InertialSampleQuality::Stale;
                excluded.inertial.latestDataReady = false;
            }
            else if (exclusion == 1)
            {
                excluded.inertial.sample.accelerationMicroG.x = 2000000;
                excluded.inertial.sample.accelerationMicroG.z = 0;
                excluded.inertial.sample.saturation =
                    adk::InertialSaturation::Acceleration;
                excluded.inertial.quality =
                    adk::InertialSampleQuality::Saturated;
            }
            else if (exclusion == 2)
                excluded.inertial.sample.accelerationMicroG = {0, 0, 0};
            else if (exclusion == 3)
            {
                excluded.joystick.xPermille = 1;
                excluded.joystick.yPermille = -1;
                excluded.joystick.event = adk::SensitivityEvent::Contradictory;
            }
            else if (exclusion == 4)
                excluded.inertial.sample.accelerationMicroG = {0, 0, -1000000};
            else
                excluded.inertial.latestDataReady = false,
                excluded.inertial.quality = adk::InertialSampleQuality::Stale;

            require (fixture.instrument.update (excluded).ok (),
                     "each non-healthy recovery frame remains admissible");
            require (fixture.instrument.snapshot ().mode ==
                         adk::BalanceInstrumentMode::Recovering,
                     "each health exclusion independently prevents recovery");

            adk::BalanceInstrumentInput recovered = input (3, 4, 4, 4, 4);
            if (exclusion == 0)
                recovered.inertial.sequenceGap = 1;
            require (fixture.instrument.update (recovered).ok () &&
                         fixture.instrument.snapshot ().mode ==
                             adk::BalanceInstrumentMode::Live,
                     "forward fully healthy frame completes recovery");
        }
    }

    void testDirectionsDiagonalAndFrozenRerender ()
    {
        struct Pose
        {
            int32_t               x;
            int32_t               y;
            int32_t               z;
            adk::BalanceDirection expected;
        };
        const Pose poses[] = {
            {0, 500000, 866025, adk::BalanceDirection::Forward},
            {0, -500000, 866025, adk::BalanceDirection::Backward},
            {-500000, 0, 866025, adk::BalanceDirection::Left},
            {500000, 0, 866025, adk::BalanceDirection::Right},
            {500000, 500000, 707107, adk::BalanceDirection::Right}};
        for (uint8_t index = 0; index < 5; ++index)
        {
            Fixture fixture;
            require (fixture.instrument.initialize ().ok (),
                     "direction fixture initializes");

            const Pose& pose = poses[index];
            const adk::BalanceInstrumentInput frame =
                input (0, 1, 1, 1, 1, pose.x, pose.y, pose.z);

            require (fixture.instrument.update (frame).ok (),
                     "cardinal or diagonal pose is admitted");
            require (fixture.instrument.snapshot ().presentation.direction ==
                         pose.expected,
                     "instrument preserves canonical direction ordering");
        }

        Fixture frozen;
        require (frozen.instrument.initialize ().ok (),
                 "frozen-rerender fixture initializes");
        adk::BalanceInstrumentInput freeze =
            input (0, 1, 1, 1, 1, 500000, 0, 866025);

        press (freeze);

        require (frozen.instrument.update (freeze).ok (),
                 "right pose freezes");

        const adk::BalanceLightIntent before =
            frozen.instrument.snapshot ().presentation.light;
        adk::BalanceInstrumentInput rerender =
            input (1, 2, 2, 2, 2, -500000, 0, 866025);

        increase (rerender);

        require (frozen.instrument.update (rerender).ok (),
                 "sensitivity changes while frozen");

        const adk::BalanceInstrumentOutput after = frozen.instrument.snapshot ();

        require (after.mode == adk::BalanceInstrumentMode::Frozen &&
                     after.presentation.direction == adk::BalanceDirection::Right &&
                     after.liveEvidence.estimate.rollMilliDegrees < 0 &&
                     after.presentation.light.redPermille > before.redPermille,
                 "frozen evidence rerenders at new sensitivity while live evidence moves");
    }

    void testInvalidEnumsStatusesAndForwardEvents ()
    {
        for (uint8_t field = 0; field < 5; ++field)
        {
            Fixture fixture;
            require (fixture.instrument.initialize ().ok (),
                     "invalid-enum fixture initializes");
            adk::BalanceInstrumentInput malformed = input (0, 1, 1, 1, 1);
            if (field == 0)
                malformed.joystick.event =
                    static_cast<adk::SensitivityEvent> (255);
            else if (field == 1)
                malformed.joystick.status =
                    static_cast<adk::StatusCode> (255);
            else if (field == 2)
                malformed.freezeButton.status =
                    static_cast<adk::StatusCode> (255);
            else if (field == 3)
                malformed.inertial.quality =
                    static_cast<adk::InertialSampleQuality> (255);
            else
                malformed.inertial.sample.saturation =
                    static_cast<adk::InertialSaturation> (255);
            requireStatus (fixture.instrument.update (malformed),
                           adk::StatusCode::InvalidArgument,
                           "invalid enum and status representations are rejected");
        }

        Fixture events;
        require (events.instrument.initialize ().ok (),
                 "forward-event fixture initializes");

        adk::BalanceInstrumentInput first = input (0, 1, 1, 1, 1);

        increase (first);

        require (events.instrument.update (first).ok (),
                 "first increase event accepted");

        adk::BalanceInstrumentInput second = input (1, 2, 2, 2, 2);

        increase (second);

        require (events.instrument.update (second).ok () &&
                     events.instrument.snapshot ().sensitivityPermille == 1000,
                 "identical event values with forward identity reapply");
    }

    void testGoldenDeterminismAndPortableCapacity ()
    {
        static_assert (sizeof (adk::CompactInertialEvidence) <= 64,
                       "compact evidence remains AVR-bounded");
        static_assert (sizeof (adk::BalanceMeasurementEvidence) <= 80,
                       "measurement evidence remains AVR-bounded");
        static_assert (sizeof (adk::BalanceFrameStorage) <= 128,
                       "single replay frame remains AVR-bounded");
        static_assert (sizeof (adk::BalanceInstrumentOutput) <= 256,
                       "project output remains AVR-bounded");
        static_assert (sizeof (adk::BalanceInstrument) <= 512,
                       "project object remains AVR-bounded");

        Fixture left;
        Fixture right;
        require (left.instrument.initialize ().ok () &&
                     right.instrument.initialize ().ok (),
                 "golden fixtures initialize");
        for (uint32_t sequence = 1; sequence <= 4; ++sequence)
        {
            adk::BalanceInstrumentInput frame =
                input (sequence * 10U, sequence, sequence, sequence, sequence,
                       sequence & 1U ? 500000 : -500000, 0, 866025);
            if (sequence == 2)
                press (frame);
            if (sequence == 3)
                increase (frame);

            const adk::Status leftStatus  = left.instrument.update (frame);

            const adk::Status rightStatus = right.instrument.update (frame);

            require (leftStatus.error () == rightStatus.error () &&
                         outputEqual (left.instrument.snapshot (),
                                      right.instrument.snapshot ()),
                     "identical golden traces produce byte-independent semantic output");
        }

        adk::BalanceInstrumentInput malformed = input (50, 5, 5, 5, 5);

        malformed.inertial.age = adk::Duration (1);

        const adk::BalanceInstrumentOutput leftBefore = left.instrument.snapshot ();

        const adk::BalanceInstrumentOutput rightBefore = right.instrument.snapshot ();

        const adk::Status leftStatus  = left.instrument.update (malformed);

        const adk::Status rightStatus = right.instrument.update (malformed);

        require (leftStatus.error () == rightStatus.error () &&
                     outputEqual (left.instrument.snapshot (), leftBefore) &&
                     outputEqual (right.instrument.snapshot (), rightBefore),
                 "malformed golden replay rejects deterministically and atomically");
    }

    void testAcknowledgementPreservesDiagnosticEpoch ()
    {
        Fixture fixture;
        require (fixture.instrument.initialize ().ok (),
                 "ack-phase fixture initializes");
        adk::BalanceInstrumentInput fault = input (0, 1, 1, 1, 1);
        fault.joystick.status = adk::StatusCode::HardwareFailure;
        requireStatus (fixture.instrument.update (fault),
                       adk::StatusCode::HardwareFailure,
                       "ack-phase fixture establishes epoch and faults");

        adk::BalanceInstrumentInput healthy = input (19, 2, 2, 2, 2);

        requireStatus (fixture.instrument.update (healthy),
                       adk::StatusCode::HardwareFailure,
                       "ack-phase fixture records healthy evidence");
        require (fixture.instrument.acknowledgeFault ().ok (),
                 "ack-phase fixture acknowledges without a frame");
        require (fixture.instrument.update (input (20, 3, 3, 3, 3)).ok (),
                 "forward healthy frame exits recovery");
        require (fixture.instrument.update (input (40, 4, 4, 4, 4, 0, 0, 0)).ok (),
                 "post-recovery diagnostic frame accepted");
        require (fixture.instrument.snapshot ().presentation.light.bluePermille == 300,
                 "acknowledgement does not rephase the original diagnostic epoch");
    }

    void testCompleteProducerFaultOrdering ()
    {
        struct FaultSet
        {
            adk::StatusCode inertial;
            adk::StatusCode button;
            adk::StatusCode joystick;
            adk::StatusCode expected;
        };
        const FaultSet sets[] = {
            {adk::StatusCode::InvalidArgument, adk::StatusCode::HardwareFailure,
             adk::StatusCode::Timeout, adk::StatusCode::InvalidArgument},
            {adk::StatusCode::Ok, adk::StatusCode::InvalidConfiguration,
             adk::StatusCode::HardwareFailure,
             adk::StatusCode::InvalidConfiguration},
            {adk::StatusCode::Ok, adk::StatusCode::Ok,
             adk::StatusCode::CapacityExceeded,
             adk::StatusCode::CapacityExceeded}};
        for (const FaultSet& set : sets)
        {
            Fixture fixture;
            require (fixture.instrument.initialize ().ok (),
                     "producer-order fixture initializes");
            adk::BalanceInstrumentInput fault = input (0, 1, 1, 1, 1);
            fault.inertial.sample.status = set.inertial;
            fault.inertial.status        = set.inertial;
            fault.inertial.quality =
                set.inertial == adk::StatusCode::Ok
                    ? adk::InertialSampleQuality::Current
                    : adk::InertialSampleQuality::Invalid;
            fault.freezeButton.status = set.button;
            fault.joystick.status     = set.joystick;
            requireStatus (fixture.instrument.update (fault), set.expected,
                           "inertial then button then joystick ordering is fixed");
        }
    }

    void testFreezeAuthorityExclusions ()
    {
        for (uint8_t exclusion = 0; exclusion < 5; ++exclusion)
        {
            Fixture fixture;
            require (fixture.instrument.initialize ().ok (),
                     "freeze-authority fixture initializes");
            adk::BalanceInstrumentInput candidate =
                input (exclusion == 0 ? 101 : 0, 1, 1, 1, 1);
            if (exclusion == 0)
            {
                candidate.inertial.sample.observedAt = adk::TimePoint (0);

                candidate.inertial.age = adk::Duration (101);
                candidate.inertial.quality = adk::InertialSampleQuality::Stale;
            }
            else if (exclusion == 1)
            {
                candidate.inertial.sample.accelerationMicroG = {2000000, 0, 0};
                candidate.inertial.sample.saturation =
                    adk::InertialSaturation::Acceleration;
                candidate.inertial.quality =
                    adk::InertialSampleQuality::Saturated;
            }
            else if (exclusion == 2)
                candidate.inertial.sample.accelerationMicroG = {0, 0, 0};
            else if (exclusion == 3)
                candidate.inertial.sample.accelerationMicroG = {0, 0, -1000000};
            else
            {
                candidate.joystick.xPermille = 1;
                candidate.joystick.yPermille = -1;
                candidate.joystick.event = adk::SensitivityEvent::Contradictory;
            }

            press (candidate);

            require (fixture.instrument.update (candidate).ok (),
                     "each ineligible press frame is admitted");
            require (fixture.instrument.snapshot ().mode ==
                             adk::BalanceInstrumentMode::Live &&
                         !fixture.instrument.snapshot ().frozenEvidence.available,
                     "stale saturated unsteady beyond and contradictory frames cannot freeze");
        }
    }
} // namespace

#ifndef ADK_BALANCE_TABLE_TEST_PART
#define ADK_BALANCE_TABLE_TEST_PART 0
#endif

#if ADK_BALANCE_TABLE_TEST_PART < 0 || ADK_BALANCE_TABLE_TEST_PART > 3
#error "ADK_BALANCE_TABLE_TEST_PART must be 1, 2, or 3 when defined"
#endif

int main ()
{
    void (*const allTests[])() = {
        testLifecycleAndConfiguration,
        testHappyPathFreezeAndSensitivity,
        testControlValidationAndReplay,
        testCompleteControlTupleMatrix,
        testIndependentIdentityOrdering,
        testFreshnessSkewAndAtomicity,
        testFaultAcknowledgementAndRecovery,
        testFaultPrecedenceAndRelatch,
        testSensitivityClampsAndSuppression,
        testIneligibleEvidenceAndOwnedPolicyAtomicity,
        testFrozenEvidenceFaultLifetime,
        testOwnedPolicyPreflightAtomicity,
        testAdmittedSafeStateResetsPolicyHistory,
        testTimingWrapAndStorage,
        testSourceDomainTransitions,
        testCanonicalProjectPresentations,
        testShutdownFromEveryMode,
        testTemporalAndDerivedFieldBoundaries,
        testDiagnosticPhaseEdgesAndFreezeAuthority,
        testConfiguredProjectIntentTransitions,
        testRecoveryHealthExclusions,
        testDirectionsDiagonalAndFrozenRerender,
        testInvalidEnumsStatusesAndForwardEvents,
        testGoldenDeterminismAndPortableCapacity,
        testAcknowledgementPreservesDiagnosticEpoch,
        testCompleteProducerFaultOrdering,
        testFreezeAuthorityExclusions};
    (void) allTests;

#if ADK_BALANCE_TABLE_TEST_PART == 0 || ADK_BALANCE_TABLE_TEST_PART == 1
    testLifecycleAndConfiguration ();

    testHappyPathFreezeAndSensitivity ();

    testControlValidationAndReplay ();

    testCompleteControlTupleMatrix ();

    testIndependentIdentityOrdering ();

    testTimingWrapAndStorage ();

    testSourceDomainTransitions ();

    testTemporalAndDerivedFieldBoundaries ();

    testInvalidEnumsStatusesAndForwardEvents ();
#endif

#if ADK_BALANCE_TABLE_TEST_PART == 0 || ADK_BALANCE_TABLE_TEST_PART == 2
    testFreshnessSkewAndAtomicity ();

    testFaultAcknowledgementAndRecovery ();

    testFaultPrecedenceAndRelatch ();

    testFrozenEvidenceFaultLifetime ();

    testOwnedPolicyPreflightAtomicity ();

    testAdmittedSafeStateResetsPolicyHistory ();

    testRecoveryHealthExclusions ();

    testAcknowledgementPreservesDiagnosticEpoch ();

    testCompleteProducerFaultOrdering ();
#endif

#if ADK_BALANCE_TABLE_TEST_PART == 0 || ADK_BALANCE_TABLE_TEST_PART == 3
    testSensitivityClampsAndSuppression ();

    testIneligibleEvidenceAndOwnedPolicyAtomicity ();

    testCanonicalProjectPresentations ();

    testShutdownFromEveryMode ();

    testDiagnosticPhaseEdgesAndFreezeAuthority ();

    testConfiguredProjectIntentTransitions ();

    testDirectionsDiagonalAndFrozenRerender ();

    testGoldenDeterminismAndPortableCapacity ();

    testFreezeAuthorityExclusions ();
#endif
    return EXIT_SUCCESS;
}
// clang-format on
