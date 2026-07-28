#include <optical_observation.h>

#include <cstdlib>
#include <iostream>
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

    adk::ReflectiveObservationConfig
    reflectiveConfig (adk::Duration dwell          = adk::Duration (10),

                      bool          darkerIsActive = true)
    {
        return {7, 12, 50, 950, 100, 900, 200, 300, dwell, darkerIsActive};
    }

    adk::BeamObservationConfig
    beamConfig (adk::Duration interruptDwell = adk::Duration (10),

                adk::Duration restoreDwell   = adk::Duration (20))

    {
        return {9, 31, adk::Level::Low, interruptDwell, restoreDwell};
    }

    adk::ReflectiveSample reflectiveSample (uint32_t now, uint16_t raw,

                                            adk::Status status   = adk::StatusCode::Ok,
                                            uint8_t     sourceId = 7,
                                            uint16_t    revision = 12)
    {
        return {{sourceId, revision, adk::TimePoint (now)}, raw, status};

    }

    adk::BeamSample beamSample (uint32_t now, adk::Level level,

                                adk::Status status = adk::StatusCode::Ok,
                                uint8_t sourceId = 9, uint16_t revision = 31)
    {
        return {{sourceId, revision, adk::TimePoint (now)}, level, status};

    }

    void requireReflectivePayloadEqual (const adk::ReflectiveObservation& left,

                                        const adk::ReflectiveObservation& right,
                                        const char*                       message)
    {
        require (left.provenance.sourceId == right.provenance.sourceId &&

                     left.provenance.calibrationRevision ==
                         right.provenance.calibrationRevision &&
                     left.provenance.observedAt == right.provenance.observedAt &&
                     left.raw == right.raw &&
                     left.normalizedPermille == right.normalizedPermille &&
                     left.markerActive == right.markerActive &&
                     left.activationEvent == right.activationEvent &&
                     left.deactivationEvent == right.deactivationEvent &&
                     left.stableFor == right.stableFor,
                 message);
    }

    void requireBeamPayloadEqual (const adk::BeamObservation& left,

                                  const adk::BeamObservation& right,
                                  const char*                 message)
    {
        require (left.provenance.sourceId == right.provenance.sourceId &&

                     left.provenance.calibrationRevision ==
                         right.provenance.calibrationRevision &&
                     left.provenance.observedAt == right.provenance.observedAt &&
                     left.rawLevel == right.rawLevel &&
                     left.interrupted == right.interrupted &&
                     left.interruptionEvent == right.interruptionEvent &&
                     left.restorationEvent == right.restorationEvent &&
                     left.stableFor == right.stableFor,
                 message);
    }

    void testTraitsAndLifecycle ()

    {
        static_assert (

            !std::is_copy_constructible<adk::ReflectiveObservationPolicy>::value,
            "reflective policy must not copy");
        static_assert (

            !std::is_move_constructible<adk::ReflectiveObservationPolicy>::value,
            "reflective policy must not move");
        static_assert (!std::is_copy_constructible<adk::BeamObservationPolicy>::value,

                       "beam policy must not copy");
        static_assert (!std::is_move_constructible<adk::BeamObservationPolicy>::value,

                       "beam policy must not move");

        adk::ReflectiveObservationPolicy reflective (reflectiveConfig ());

        require (!reflective.initialized (), "reflective starts inert");

        require (reflective.snapshot ().quality == adk::OpticalQuality::Unqualified,

                 "reflective starts unqualified");
        require (reflective.update (reflectiveSample (0, 500)).error () ==

                     adk::StatusCode::NotInitialized,
                 "reflective rejects update before initialize");
        require (reflective.initialize ().ok (), "reflective initializes");

        require (reflective.initialize ().ok (), "reflective initialize is idempotent");

        require (reflective.initialized (), "reflective reports initialized");

        require (reflective.snapshot ().darkReference == 100 &&

                     reflective.snapshot ().lightReference == 900,

                 "reflective exposes configured references");
        reflective.reset ();

        require (reflective.snapshot ().quality == adk::OpticalQuality::Unqualified,

                 "reflective reset clears qualification");

        adk::BeamObservationPolicy beam (beamConfig ());

        require (!beam.initialized (), "beam starts inert");

        require (beam.update (beamSample (0, adk::Level::High)).error () ==

                     adk::StatusCode::NotInitialized,
                 "beam rejects update before initialize");
        require (beam.initialize ().ok (), "beam initializes");

        require (beam.initialize ().ok (), "beam initialize is idempotent");

        beam.reset ();

        require (!beam.snapshot ().interrupted &&

                     beam.snapshot ().quality == adk::OpticalQuality::Unqualified,

                 "beam reset clears state");
    }

    void testInvalidConfigurations ()

    {
        adk::ReflectiveObservationConfig config = reflectiveConfig ();

        config.qualifiedMinimum                 = 951;
        adk::ReflectiveObservationPolicy bounds (config);

        require (bounds.initialize ().error () == adk::StatusCode::InvalidConfiguration,

                 "reversed qualified bounds rejected");

        config                  = reflectiveConfig ();

        config.qualifiedMaximum = 1024;
        adk::ReflectiveObservationPolicy adc (config);

        require (adc.initialize ().error () == adk::StatusCode::InvalidConfiguration,

                 "qualified range above ADC rejected");

        config               = reflectiveConfig ();

        config.darkReference = 49;
        adk::ReflectiveObservationPolicy reference (config);

        require (reference.initialize ().error () ==

                     adk::StatusCode::InvalidConfiguration,
                 "reference outside qualified range rejected");

        config                = reflectiveConfig ();

        config.lightReference = config.darkReference;
        adk::ReflectiveObservationPolicy degenerate (config);

        require (degenerate.initialize ().error () ==

                         adk::StatusCode::InvalidConfiguration &&
                     degenerate.snapshot ().quality ==

                         adk::OpticalQuality::DegenerateCalibration,
                 "degenerate calibration identified");

        config                  = reflectiveConfig ();

        config.activatePermille = 300;
        config.releasePermille  = 300;
        adk::ReflectiveObservationPolicy hysteresis (config);

        require (hysteresis.initialize ().error () ==

                     adk::StatusCode::InvalidConfiguration,
                 "invalid reflective hysteresis rejected");

        config       = reflectiveConfig ();

        config.dwell = adk::Duration (0x80000000UL);

        adk::ReflectiveObservationPolicy dwell (config);

        require (dwell.initialize ().error () == adk::StatusCode::InvalidConfiguration,

                 "half-range reflective dwell rejected");

        adk::BeamObservationConfig beam = beamConfig ();

        beam.interruptedLevel           = static_cast<adk::Level> (3);

        adk::BeamObservationPolicy level (beam);

        require (level.initialize ().error () == adk::StatusCode::InvalidConfiguration,

                 "invalid interrupted level rejected");

        beam              = beamConfig ();

        beam.restoreDwell = adk::Duration (0x80000000UL);

        adk::BeamObservationPolicy beamDwell (beam);

        require (beamDwell.initialize ().error () ==

                     adk::StatusCode::InvalidConfiguration,
                 "half-range beam dwell rejected");
    }

    void testReflectiveCalibrationAndBoundaries ()

    {
        adk::ReflectiveObservationPolicy policy (reflectiveConfig ());

        require (policy.initialize ().ok (), "calibration policy initializes");


        require (policy.update (reflectiveSample (0, 100)).ok (),

                 "dark endpoint accepted");
        require (policy.snapshot ().normalizedPermille == 0,

                 "dark endpoint normalizes to zero");
        policy.update (reflectiveSample (1, 500));

        require (policy.snapshot ().normalizedPermille == 500,

                 "calibration midpoint normalizes");
        policy.update (reflectiveSample (2, 900));

        require (policy.snapshot ().normalizedPermille == 1000,

                 "light endpoint normalizes to full scale");
        policy.update (reflectiveSample (3, 50));

        require (policy.snapshot ().normalizedPermille == 0 &&

                     policy.snapshot ().quality == adk::OpticalQuality::Valid,

                 "qualified minimum clamps and remains valid");
        policy.update (reflectiveSample (4, 950));

        require (policy.snapshot ().normalizedPermille == 1000 &&

                     policy.snapshot ().quality == adk::OpticalQuality::Valid,

                 "qualified maximum clamps and remains valid");
        policy.update (reflectiveSample (5, 49));

        require (policy.snapshot ().quality == adk::OpticalQuality::BelowQualifiedRange,

                 "minimum minus one is below range");
        policy.update (reflectiveSample (6, 951));

        require (policy.snapshot ().quality == adk::OpticalQuality::AboveQualifiedRange,

                 "maximum plus one is above range");

        adk::ReflectiveObservationConfig reverse = reflectiveConfig ();

        reverse.darkReference                    = 900;
        reverse.lightReference                   = 100;
        adk::ReflectiveObservationPolicy reversed (reverse);

        require (reversed.initialize ().ok (), "reversed calibration initializes");

        reversed.update (reflectiveSample (0, 900));

        require (reversed.snapshot ().normalizedPermille == 0,

                 "reversed dark endpoint normalizes to zero");
        reversed.update (reflectiveSample (1, 500));

        require (reversed.snapshot ().normalizedPermille == 500,

                 "reversed midpoint normalizes");
        reversed.update (reflectiveSample (2, 100));

        require (reversed.snapshot ().normalizedPermille == 1000,

                 "reversed light endpoint normalizes to full scale");
    }

    void testReflectiveHysteresisDwellAndEvents ()

    {
        adk::ReflectiveObservationPolicy policy (reflectiveConfig ());

        require (policy.initialize ().ok (), "reflective dwell initializes");


        policy.update (reflectiveSample (0, 500));

        policy.update (reflectiveSample (1, 260));

        policy.update (reflectiveSample (10, 260));

        require (!policy.snapshot ().markerActive,

                 "activation remains pending before exact dwell");
        policy.update (reflectiveSample (11, 260));

        require (policy.snapshot ().markerActive && policy.snapshot ().activationEvent,

                 "exact activation dwell emits event");
        require (policy.snapshot ().stableFor.milliseconds () == 0,

                 "transition restarts stable duration");

        policy.update (reflectiveSample (12, 300));

        require (policy.snapshot ().markerActive && !policy.snapshot ().activationEvent,

                 "hysteresis band holds and event clears");
        policy.update (reflectiveSample (20, 340));

        policy.update (reflectiveSample (25, 339));

        policy.update (reflectiveSample (30, 340));

        require (policy.snapshot ().markerActive, "release chatter restarts dwell");

        policy.update (reflectiveSample (40, 340));

        require (!policy.snapshot ().markerActive &&

                     policy.snapshot ().deactivationEvent,

                 "continuous release dwell emits event");

        policy.update (reflectiveSample (41, 260));

        policy.update (reflectiveSample (46, 49));

        policy.update (reflectiveSample (56, 260));

        require (!policy.snapshot ().markerActive,

                 "out-of-range evidence clears candidate");
        policy.update (reflectiveSample (66, 260));

        require (policy.snapshot ().markerActive, "new continuous candidate qualifies");


        adk::ReflectiveObservationConfig lightActive =
            reflectiveConfig (adk::Duration (), false);

        lightActive.activatePermille = 800;
        lightActive.releasePermille  = 700;
        adk::ReflectiveObservationPolicy lightPolicy (lightActive);

        require (lightPolicy.initialize ().ok (), "light-active policy initializes");

        lightPolicy.update (reflectiveSample (0, 740));

        require (lightPolicy.snapshot ().markerActive,

                 "light-active exact threshold qualifies immediately");
        lightPolicy.update (reflectiveSample (1, 660));

        require (!lightPolicy.snapshot ().markerActive,

                 "light-active exact release qualifies immediately");
    }

    void testBeamDwellChatterAndReplay ()

    {
        adk::BeamObservationPolicy beam (beamConfig ());

        require (beam.initialize ().ok (), "beam dwell initializes");

        beam.update (beamSample (0, adk::Level::High));

        beam.update (beamSample (1, adk::Level::Low));

        beam.update (beamSample (10, adk::Level::Low));

        require (!beam.snapshot ().interrupted,

                 "short interruption pulse does not qualify");
        beam.update (beamSample (11, adk::Level::Low));

        require (beam.snapshot ().interrupted && beam.snapshot ().interruptionEvent,

                 "exact interrupt dwell qualifies");
        const adk::BeamObservation event = beam.snapshot ();

        require (beam.update (beamSample (11, adk::Level::Low)).ok (),

                 "identical event sample replays");
        requireBeamPayloadEqual (beam.snapshot (), event,

                                 "identical replay retains event payload");
        beam.update (beamSample (12, adk::Level::Low));

        require (!beam.snapshot ().interruptionEvent,

                 "later sample clears interruption event");

        beam.update (beamSample (20, adk::Level::High));

        beam.update (beamSample (30, adk::Level::Low));

        beam.update (beamSample (31, adk::Level::High));

        beam.update (beamSample (50, adk::Level::High));

        require (beam.snapshot ().interrupted, "restoration chatter restarts dwell");

        beam.update (beamSample (51, adk::Level::High));

        require (!beam.snapshot ().interrupted && beam.snapshot ().restorationEvent,

                 "continuous restore dwell qualifies");

        adk::BeamObservationPolicy stuck (

            beamConfig (adk::Duration (), adk::Duration ()));

        require (stuck.initialize ().ok (), "zero-dwell beam initializes");

        stuck.update (beamSample (0, adk::Level::High));

        require (!stuck.snapshot ().interrupted, "stuck clear remains clear");

        stuck.update (beamSample (1, adk::Level::Low));

        require (stuck.snapshot ().interrupted, "stuck interrupted is represented");

        stuck.update (beamSample (2, adk::Level::Low));

        require (stuck.snapshot ().interrupted && !stuck.snapshot ().interruptionEvent,

                 "continued interruption is stable evidence");
    }

    void testFaultLatchingAndReset ()

    {
        adk::ReflectiveObservationPolicy reflective (reflectiveConfig ());

        reflective.initialize ();

        reflective.update (reflectiveSample (0, 500));

        require (reflective

                         .update (reflectiveSample (1, 500,

                                                    adk::StatusCode::HardwareFailure))
                         .error () == adk::StatusCode::HardwareFailure,

                 "analog source status is retained");
        require (reflective.snapshot ().quality == adk::OpticalQuality::SourceFault,

                 "analog source fault classified");
        const adk::ReflectiveObservation fault = reflective.snapshot ();

        reflective.update (reflectiveSample (2, 900));

        requireReflectivePayloadEqual (reflective.snapshot (), fault,

                                       "reflective source fault latches");
        reflective.reset ();

        require (reflective.update (reflectiveSample (3, 500)).ok (),

                 "reflective reset recovers");

        adk::ReflectiveObservationPolicy provenance (reflectiveConfig ());

        provenance.initialize ();

        provenance.update (reflectiveSample (0, 500));

        require (provenance.update (reflectiveSample (1, 500, adk::StatusCode::Ok, 8))

                         .error () == adk::StatusCode::InvalidArgument,

                 "reflective source identity mismatch faults");

        adk::BeamObservationPolicy beam (beamConfig ());

        beam.initialize ();

        beam.update (beamSample (0, adk::Level::High));

        require (beam.update (beamSample (1, static_cast<adk::Level> (2))).error () ==

                     adk::StatusCode::InvalidArgument,
                 "invalid beam level faults");
        require (beam.snapshot ().quality == adk::OpticalQuality::SourceFault,

                 "beam source fault classified");
        const adk::BeamObservation beamFault = beam.snapshot ();

        beam.update (beamSample (2, adk::Level::High));

        requireBeamPayloadEqual (beam.snapshot (), beamFault,

                                 "beam source fault latches");
        beam.reset ();

        require (beam.update (beamSample (3, adk::Level::High)).ok (),

                 "beam reset recovers");

        adk::BeamObservationPolicy unavailable (beamConfig ());

        unavailable.initialize ();

        require (unavailable

                         .update (beamSample (0, adk::Level::High,

                                              adk::StatusCode::Unsupported))
                         .error () == adk::StatusCode::Unsupported,

                 "digital unavailable evidence retained");
        require (unavailable.snapshot ().quality == adk::OpticalQuality::SourceFault,

                 "digital unavailable evidence classified");
    }

    void testTimingReplayRolloverAndFaults ()

    {
        adk::ReflectiveObservationPolicy reflective (reflectiveConfig ());

        reflective.initialize ();

        reflective.update (reflectiveSample (0xFFFFFFF8UL, 260));

        reflective.update (reflectiveSample (2, 260));

        require (reflective.snapshot ().markerActive,

                 "reflective dwell crosses rollover");
        const adk::ReflectiveObservation event = reflective.snapshot ();

        reflective.update (reflectiveSample (2, 260));

        requireReflectivePayloadEqual (reflective.snapshot (), event,

                                       "reflective identical replay is stable");
        require (reflective.update (reflectiveSample (2, 261)).error () ==

                     adk::StatusCode::InvalidArgument,
                 "changed same-time reflective sample faults");
        require (reflective.snapshot ().quality == adk::OpticalQuality::TimingFault,

                 "reflective timing fault classified");
        require (reflective.snapshot ().raw == event.raw &&

                     reflective.snapshot ().activationEvent,

                 "reflective timing fault retains prior payload and event");

        adk::BeamObservationPolicy backward (beamConfig ());

        backward.initialize ();

        backward.update (beamSample (100, adk::Level::High));

        const adk::BeamObservation before = backward.snapshot ();

        require (backward.update (beamSample (99, adk::Level::Low)).error () ==

                     adk::StatusCode::InvalidArgument,
                 "apparent backward beam time faults");
        require (backward.snapshot ().quality == adk::OpticalQuality::TimingFault,

                 "beam backward fault classified");
        requireBeamPayloadEqual (backward.snapshot (), before,

                                 "beam backward fault retains prior payload");
        backward.reset ();

        require (backward.update (beamSample (0, adk::Level::High)).ok (),

                 "beam timing reset recovers");

        adk::BeamObservationPolicy halfRange (beamConfig ());

        halfRange.initialize ();

        halfRange.update (beamSample (0, adk::Level::High));

        require (

            halfRange.update (beamSample (0x80000000UL, adk::Level::High)).error () ==

                adk::StatusCode::InvalidArgument,
            "exact half-range beam jump faults");

        adk::ReflectiveObservationPolicy beyond (reflectiveConfig ());

        beyond.initialize ();

        beyond.update (reflectiveSample (0, 500));

        require (beyond.update (reflectiveSample (0x80000001UL, 500)).error () ==

                     adk::StatusCode::InvalidArgument,
                 "greater-than-half-range reflective jump faults");
    }
} // namespace

int main ()

{
    testTraitsAndLifecycle ();

    testInvalidConfigurations ();

    testReflectiveCalibrationAndBoundaries ();

    testReflectiveHysteresisDwellAndEvents ();

    testBeamDwellChatterAndReplay ();

    testFaultLatchingAndReset ();

    testTimingReplayRolloverAndFaults ();


    std::cout << "All ADK optical observation tests passed.\n";
    return EXIT_SUCCESS;
}
