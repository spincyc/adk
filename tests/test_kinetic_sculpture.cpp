#include <kinetic_sculpture.h>

#include <cstdint>
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

    adk::InteractionIntentConfig interactionConfig ()
    {
        return {{adk::Level::High, adk::Duration (8), adk::Duration (8),
                 adk::Duration (80), adk::Duration (2000)},
                adk::Duration (100), adk::Duration (100), 300, 200};
    }

    adk::StepperSequenceConfig sequenceConfig (int32_t minimum = -256,
                                                int32_t maximum = 256)
    {
        return {adk::Duration (60), adk::Duration (250), adk::Duration (2000),
                minimum, maximum, false};
    }

    adk::InteractionSource source (
        adk::InteractionSourceKind kind =
            adk::InteractionSourceKind::SyntheticFixture,
        uint8_t sourceId = 1, uint16_t revision = 1)
    {
        return {kind, sourceId, revision};
    }

    adk::SculptureInput input (uint32_t now, uint32_t frameId,
                               uint32_t touchSequence,
                               uint32_t directionalSequence,
                               uint32_t stopSequence,
                               int16_t x = 0, int16_t y = 0,
                               adk::Level touchLevel = adk::Level::Low)
    {
        return {adk::TimePoint (now),
                frameId,
                source (adk::InteractionSourceKind::SyntheticFixture, 3, 1),

                adk::TimePoint (now),
                stopSequence,
                false,
                adk::ContactQuality::Valid,
                adk::Status (),

                source (adk::InteractionSourceKind::SyntheticFixture, 1, 1),
                touchSequence,
                {adk::TimePoint (now), touchLevel, adk::Status ()},

                {source (adk::InteractionSourceKind::SyntheticFixture, 2, 1),
                 adk::TimePoint (now), directionalSequence, x, y, false,

                 adk::Status ()},
                adk::Status ()};
    }

    bool sourceEqual (const adk::InteractionSource& left,
                      const adk::InteractionSource& right)
    {
        return left.kind == right.kind && left.sourceId == right.sourceId &&
               left.configurationRevision == right.configurationRevision;
    }

    bool authorizationEqual (const adk::AuthorizationRecord& left,
                             const adk::AuthorizationRecord& right)
    {
        return left.originatingFrameId == right.originatingFrameId &&
               sourceEqual (left.contactSource, right.contactSource) &&

               sourceEqual (left.directionalSource, right.directionalSource) &&
               left.contactSequence == right.contactSequence &&
               left.directionalSequence == right.directionalSequence &&
               left.direction == right.direction &&
               left.disposition == right.disposition && left.status == right.status;
    }

    bool interactionEqual (const adk::InteractionIntent& left,
                           const adk::InteractionIntent& right)
    {
        return sourceEqual (left.contactSource, right.contactSource) &&
               sourceEqual (left.directionalSource, right.directionalSource) &&
               left.observedAt == right.observedAt &&
               left.contactSequence == right.contactSequence &&
               left.directionalSequence == right.directionalSequence &&
               left.direction == right.direction &&
               left.magnitudePermille == right.magnitudePermille &&
               left.touchActive == right.touchActive &&
               left.touchEvent == right.touchEvent &&
               left.touchReleaseEvent == right.touchReleaseEvent &&
               left.directionEvent == right.directionEvent &&
               left.quality == right.quality && left.contactAge == right.contactAge &&
               left.directionalAge == right.directionalAge &&
               left.directionalSaturated == right.directionalSaturated &&
               left.contactQuality == right.contactQuality &&
               left.contactStatus == right.contactStatus &&
               left.directionalStatus == right.directionalStatus &&
               left.status == right.status;
    }

    bool motionEqual (const adk::StepperSequenceSnapshot& left,
                      const adk::StepperSequenceSnapshot& right)
    {
        return left.commandId == right.commandId && left.phase == right.phase &&
               left.disposition == right.disposition &&
               left.direction == right.direction &&
               left.logicalPosition == right.logicalPosition &&
               left.requestedSteps == right.requestedSteps &&
               left.completedSteps == right.completedSteps &&
               left.coilIntent == right.coilIntent &&
               left.phaseSince == right.phaseSince &&
               left.nextStepAt == right.nextStepAt &&
               left.hasDeadline == right.hasDeadline && left.status == right.status;
    }

    bool snapshotEqual (const adk::SculptureSnapshot& left,
                        const adk::SculptureSnapshot& right)
    {
        return left.frameId == right.frameId && left.phase == right.phase &&
               interactionEqual (left.interaction, right.interaction) &&

               motionEqual (left.motion, right.motion) &&
               left.lights.shiftRegisterBits == right.lights.shiftRegisterBits &&
               left.lights.ready == right.lights.ready &&
               left.lights.running == right.lights.running &&
               left.lights.stopped == right.lights.stopped &&
               left.lights.fault == right.lights.fault &&
               left.lights.travelLimit == right.lights.travelLimit &&
               sourceEqual (left.stopSource, right.stopSource) &&
               left.stopObservedAt == right.stopObservedAt &&
               left.stopSequence == right.stopSequence &&
               left.stopActive == right.stopActive &&
               left.hasStopIdentity == right.hasStopIdentity &&
               left.travelLimit == right.travelLimit &&
               left.hasPendingAuthorization == right.hasPendingAuthorization &&
               authorizationEqual (left.pendingAuthorization,
                                   right.pendingAuthorization) &&
               left.hasLastTerminalAuthorization ==
                   right.hasLastTerminalAuthorization &&
               authorizationEqual (left.lastTerminalAuthorization,
                                   right.lastTerminalAuthorization) &&
               left.stopQuality == right.stopQuality &&
               left.stopStatus == right.stopStatus &&
               left.interactionStatus == right.interactionStatus &&
               left.motionStatus == right.motionStatus &&
               left.acceptedMotifCount == right.acceptedMotifCount &&
               left.status == right.status;
    }

    struct Fixture
    {
        adk::KineticLightSculpture sculpture;
        uint32_t                   frame;
        uint32_t                   touch;
        uint32_t                   direction;
        uint32_t                   stop;

        Fixture (int32_t minimum = -256, int32_t maximum = 256)
            : sculpture (interactionConfig (), sequenceConfig (minimum, maximum),
                         adk::Duration (250), adk::Duration (40)),
              frame (1), touch (1), direction (1), stop (1)
        {
            require (sculpture.initialize ().ok (), "fixture initializes");
        }

        adk::SculptureInput next (uint32_t now, int16_t x = 0, int16_t y = 0,
                                  adk::Level level = adk::Level::Low)
        {
            return input (now, frame++, touch++, direction++, stop++, x, y, level);
        }
    };

    void requireAllOff (const adk::SculptureSnapshot& snapshot,
                        const char* message)
    {
        require (snapshot.motion.coilIntent == 0 &&
                     snapshot.lights.shiftRegisterBits == 0 &&
                     !snapshot.lights.running,
                 message);
    }

    void qualifyTouch (Fixture& fixture, uint32_t start, int16_t x, int16_t y)
    {
        adk::SculptureInput baseline =
            fixture.next (start, x, y, adk::Level::Low);
        require (fixture.sculpture.update (baseline).ok (), "baseline accepted");
        adk::SculptureInput attack =
            fixture.next (start + 1, x, y, adk::Level::High);
        require (fixture.sculpture.update (attack).ok (), "attack starts");
        adk::SculptureInput qualified =
            fixture.next (start + 9, x, y, adk::Level::High);
        require (fixture.sculpture.update (qualified).ok (), "touch qualifies");

        require (fixture.sculpture.snapshot ().hasPendingAuthorization,
                 "qualified touch becomes pending");
    }

    void testLifecycleAndConfiguration ()
    {
        static_assert (
            !std::is_copy_constructible<adk::KineticLightSculpture>::value,
            "project cannot copy composed policies");
        static_assert (
            !std::is_move_constructible<adk::KineticLightSculpture>::value,
            "project cannot move composed policies");

        adk::KineticLightSculpture project (
            interactionConfig (), sequenceConfig (), adk::Duration (250),

            adk::Duration (40));
        require (!project.initialized (), "construction is inert");

        require (project.snapshot ().phase == adk::SculpturePhase::Inactive,
                 "construction publishes inactive");
        requireAllOff (project.snapshot (), "construction is all off");

        require (project.update (input (0, 1, 1, 1, 1)).error () ==
                     adk::StatusCode::NotInitialized,
                 "update before initialization rejected");
        require (project.initialize ().ok () && project.initialize ().ok (),
                 "initialization is idempotent");
        require (project.snapshot ().phase == adk::SculpturePhase::Ready,
                 "initialization publishes ready");
        require (!project.snapshot ().hasPendingAuthorization &&
                     !project.snapshot ().hasLastTerminalAuthorization,
                 "initialization clears authorization records");

        project.shutdown ();

        project.shutdown ();

        require (!project.initialized (), "shutdown is idempotent");

        require (project.snapshot ().phase == adk::SculpturePhase::Inactive,
                 "shutdown returns inactive");
        requireAllOff (project.snapshot (), "shutdown is all off");

        require (project.initialize ().ok (), "restart initializes");

        require (project.snapshot ().motion.logicalPosition == 0,
                 "power-loss restart restores no position claim");

        const uint32_t invalidAge[] = {0, 1001, 0x80000000UL};
        for (uint32_t age : invalidAge)
        {
            adk::KineticLightSculpture rejected (
                interactionConfig (), sequenceConfig (), adk::Duration (age),

                adk::Duration (0));
            require (rejected.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid frame age rejected");
            require (!rejected.initialized (), "invalid project remains inert");
        }
        adk::KineticLightSculpture badSkew (
            interactionConfig (), sequenceConfig (), adk::Duration (250),

            adk::Duration (251));
        require (badSkew.initialize ().error () ==
                     adk::StatusCode::InvalidConfiguration,
                 "source skew cannot exceed frame age");
    }

    void testStagedAuthorizationAndMotifs ()
    {
        struct Motif
        {
            int16_t                   x;
            int16_t                   y;
            adk::InteractionDirection direction;
            adk::StepDirection        stepDirection;
            uint32_t                  steps;
            uint32_t                  interval;
            uint8_t                   lights;
        };
        const Motif motifs[] = {
            {0, 1000, adk::InteractionDirection::North,
             adk::StepDirection::Forward, 8, 120, 0x81},
            {1000, 1000, adk::InteractionDirection::NorthEast,
             adk::StepDirection::Forward, 12, 100, 0xc3},
            {1000, 0, adk::InteractionDirection::East,
             adk::StepDirection::Forward, 16, 80, 0x42},
            {1000, -1000, adk::InteractionDirection::SouthEast,
             adk::StepDirection::Forward, 12, 100, 0x66},
            {0, -1000, adk::InteractionDirection::South,
             adk::StepDirection::Reverse, 8, 120, 0x18},
            {-1000, -1000, adk::InteractionDirection::SouthWest,
             adk::StepDirection::Reverse, 12, 100, 0x3c},
            {-1000, 0, adk::InteractionDirection::West,
             adk::StepDirection::Reverse, 16, 80, 0x24},
            {-1000, 1000, adk::InteractionDirection::NorthWest,
             adk::StepDirection::Reverse, 12, 100, 0x99}};

        for (const Motif& motif : motifs)
        {
            Fixture fixture;
            qualifyTouch (fixture, 0, motif.x, motif.y);

            const adk::SculptureSnapshot pending = fixture.sculpture.snapshot ();

            require (pending.phase == adk::SculpturePhase::Ready ||
                         pending.phase == adk::SculpturePhase::Preview,
                     "touch remains non-running before motion");
            require (pending.pendingAuthorization.direction == motif.direction &&
                         pending.pendingAuthorization.disposition ==
                             adk::AuthorizationDisposition::Pending,
                     "pending record retains direction and disposition");
            require (pending.motion.requestedSteps == 0,
                     "authorization frame consumes no motion");

            adk::SculptureInput consume =
                fixture.next (10, motif.x, motif.y, adk::Level::High);
            require (fixture.sculpture.update (consume).ok (),
                     "strictly-forward frame consumes authorization");
            const adk::SculptureSnapshot running = fixture.sculpture.snapshot ();

            require (!running.hasPendingAuthorization &&
                         running.hasLastTerminalAuthorization,
                     "accepted authorization moves to terminal record");
            require (running.lastTerminalAuthorization.disposition ==
                         adk::AuthorizationDisposition::Accepted,
                     "terminal disposition is accepted");
            require (running.phase == adk::SculpturePhase::Running &&
                         running.motion.direction == motif.stepDirection &&
                         running.motion.requestedSteps == motif.steps,
                     "direction selects fixed bounded motif");
            require (running.motion.nextStepAt.elapsedSince (adk::TimePoint (10)) ==
                         adk::Duration (motif.interval),
                     "motif selects fixed cadence");
            require (running.acceptedMotifCount == 1,
                     "accepted motif increments exactly once");

            adk::SculptureInput repeat = consume;
            const adk::Status repeated = fixture.sculpture.update (repeat);

            require (repeated.ok (), "exact repeated frame is admitted");

            require (fixture.sculpture.snapshot ().acceptedMotifCount == 1,
                     "exact repeat cannot authorize twice");
        }
    }

    void testStopDominanceAndRecovery ()
    {
        Fixture fixture;
        qualifyTouch (fixture, 0, 1000, 0);

        adk::SculptureInput collided =
            fixture.next (10, 1000, 0, adk::Level::High);
        collided.stopActive             = true;
        collided.directional.xPermille = 2000;
        collided.directional.status    = adk::StatusCode::HardwareFailure;
        require (fixture.sculpture.update (collided).ok (),
                 "valid active stop dominates malformed and faulted evidence");
        const adk::SculptureSnapshot stopped = fixture.sculpture.snapshot ();

        require (stopped.phase == adk::SculpturePhase::Stopped &&
                     stopped.lights.stopped && !stopped.lights.fault,
                 "stop is distinct from fault");
        requireAllOff (stopped, "stop clears motion and mirror");

        require (!stopped.hasPendingAuthorization &&
                     stopped.hasLastTerminalAuthorization &&
                     stopped.lastTerminalAuthorization.disposition ==
                         adk::AuthorizationDisposition::Inhibited,
                 "stop terminalizes pending authorization");
        require (stopped.acceptedMotifCount == 0,
                 "stop collision admits no motif");

        adk::SculptureInput held =
            fixture.next (11, 1000, 0, adk::Level::High);
        held.stopActive = true;
        require (fixture.sculpture.update (held).ok (), "held stop remains stopped");

        require (fixture.sculpture.snapshot ().phase ==
                     adk::SculpturePhase::Stopped,
                 "held stop cannot restart");

        adk::SculptureInput released =
            fixture.next (12, 1000, 0, adk::Level::High);
        require (fixture.sculpture.update (released).ok (),
                 "released valid stop evidence accepted");
        require (fixture.sculpture.snapshot ().phase ==
                     adk::SculpturePhase::Stopped,
                 "released stop remains latched");
        require (fixture.sculpture.snapshot ().acceptedMotifCount == 0,
                 "release alone cannot start");

        Fixture malformedFixture;
        adk::SculptureInput malformed =
            malformedFixture.next (13, 1000, 0, adk::Level::High);
        malformed.stopSource.sourceId = 0;
        require (!malformedFixture.sculpture.update (malformed).ok (),
                 "malformed stop fails closed");
        require (malformedFixture.sculpture.snapshot ().phase ==
                         adk::SculpturePhase::Fault &&
                     malformedFixture.sculpture.snapshot ().lights.fault,
                 "malformed stop publishes fault");
        requireAllOff (malformedFixture.sculpture.snapshot (),
                       "malformed stop cannot retain motion intent");
    }

    void testAtomicStructuralRejectionAndFaultGate ()
    {
        Fixture fixture;
        adk::SculptureInput baseline = fixture.next (100, 500, 0);

        require (fixture.sculpture.update (baseline).ok (), "baseline accepted");

        const adk::SculptureSnapshot before = fixture.sculpture.snapshot ();

        adk::SculptureInput malformed = fixture.next (101, 500, 0);
        malformed.directional.xPermille = 1001;
        require (!fixture.sculpture.update (malformed).ok (),
                 "malformed non-stop evidence rejected");
        require (snapshotEqual (before, fixture.sculpture.snapshot ()),
                 "structural rejection is project-atomic");

        malformed = fixture.next (101, 500, 0);
        malformed.frameId = 0;
        require (!fixture.sculpture.update (malformed).ok (),
                 "zero frame identity rejected");
        require (snapshotEqual (before, fixture.sculpture.snapshot ()),
                 "frame rejection cannot mutate either child");

        qualifyTouch (fixture, 102, 1000, 0);

        const adk::SculptureSnapshot pending = fixture.sculpture.snapshot ();
        adk::SculptureInput fault =
            fixture.next (112, 1000, 0, adk::Level::High);
        fault.directional.status = adk::StatusCode::HardwareFailure;
        require (!fixture.sculpture.update (fault).ok (),
                 "valid source fault publishes its non-OK status");
        const adk::SculptureSnapshot failed = fixture.sculpture.snapshot ();

        require (failed.phase == adk::SculpturePhase::Fault &&
                     failed.interactionStatus.error () ==
                         adk::StatusCode::HardwareFailure,
                 "admitted source fault retains attribution");
        requireAllOff (failed, "fault gate cancels motion");

        require (failed.motion.logicalPosition == pending.motion.logicalPosition &&
                     failed.motion.completedSteps == pending.motion.completedSteps &&
                     failed.acceptedMotifCount == pending.acceptedMotifCount,
                 "asymmetric health gate consumes no travel, step, or motif");
        require (!failed.hasPendingAuthorization &&
                     failed.lastTerminalAuthorization.disposition ==
                         adk::AuthorizationDisposition::Inhibited,
                 "fault terminalizes pending without loss");
        adk::SculptureInput attemptedRecovery =
            fixture.next (113, 1000, 0, adk::Level::Low);
        require (!fixture.sculpture.update (attemptedRecovery).ok (),
                 "source fault remains latched");
        require (snapshotEqual (failed, fixture.sculpture.snapshot ()),
                 "fresh input cannot recover a fault without reinitialization");
    }

    void testStopStatusAttribution ()
    {
        const adk::StatusCode failures[] = {
            adk::StatusCode::ResourceBusy,
            adk::StatusCode::Timeout,
            adk::StatusCode::HardwareFailure};
        for (adk::StatusCode failure : failures)
        {
            Fixture fixture;
            adk::SculptureInput failed = fixture.next (0);
            failed.stopQuality = adk::ContactQuality::SourceFault;
            failed.stopStatus  = failure;
            require (fixture.sculpture.update (failed).error () == failure,
                     "recognized stop failure returns exact status");
            const adk::SculptureSnapshot snapshot = fixture.sculpture.snapshot ();

            require (snapshot.phase == adk::SculpturePhase::Fault &&
                         snapshot.lights.fault,
                     "recognized stop failure fails closed");
            require (snapshot.stopQuality == adk::ContactQuality::SourceFault &&
                         snapshot.stopStatus.error () == failure &&

                         snapshot.status.error () == failure,
                     "recognized stop failure retains exact attribution");
            requireAllOff (snapshot, "recognized stop failure is all off");
        }

        Fixture malformed;
        adk::SculptureInput invalid = malformed.next (0);
        invalid.stopQuality = static_cast<adk::ContactQuality> (255);
        invalid.stopStatus  = static_cast<adk::StatusCode> (255);
        require (malformed.sculpture.update (invalid).error () ==
                     adk::StatusCode::InvalidArgument,
                 "malformed stop enum normalizes to invalid argument");
        const adk::SculptureSnapshot normalized = malformed.sculpture.snapshot ();

        require (normalized.phase == adk::SculpturePhase::Fault &&
                     normalized.stopQuality == adk::ContactQuality::Unqualified &&
                     normalized.stopStatus.error () ==
                         adk::StatusCode::InvalidArgument &&
                     normalized.status.error () ==
                         adk::StatusCode::InvalidArgument,
                 "malformed stop values never leak into snapshot");
        requireAllOff (normalized, "malformed stop enum is all off");
    }

    void testTravelBoundAndOppositeRecovery ()
    {
        Fixture fixture (-8, 8);

        qualifyTouch (fixture, 0, 1000, 0);

        adk::SculptureInput east = fixture.next (10, 1000, 0, adk::Level::High);

        fixture.sculpture.update (east);

        const adk::SculptureSnapshot limited = fixture.sculpture.snapshot ();

        require (limited.phase == adk::SculpturePhase::Ready &&
                     limited.travelLimit && limited.lights.travelLimit,
                 "complete-endpoint rejection raises travel limit");
        require (limited.lastTerminalAuthorization.disposition ==
                     adk::AuthorizationDisposition::BoundRejected,
                 "bound rejection is a terminal authorization outcome");
        require (limited.motion.logicalPosition == 0 &&
                     limited.acceptedMotifCount == 0,
                 "bound rejection consumes no travel or motif");

        Fixture recovery (-16, 16);

        qualifyTouch (recovery, 0, -1000, 0);
        adk::SculptureInput west =
            recovery.next (10, -1000, 0, adk::Level::High);
        require (recovery.sculpture.update (west).ok (),
                 "opposite direction within bound is accepted");
        require (recovery.sculpture.snapshot ().phase ==
                     adk::SculpturePhase::Running,
                 "fresh opposite authorization can recover");
    }

    void testAuthorizationShutdown ()
    {
        Fixture fixture;
        qualifyTouch (fixture, 0, 1000, 0);

        fixture.sculpture.shutdown ();

        const adk::SculptureSnapshot shutdown = fixture.sculpture.snapshot ();

        require (!shutdown.hasPendingAuthorization &&
                     shutdown.hasLastTerminalAuthorization &&
                     shutdown.lastTerminalAuthorization.disposition ==
                         adk::AuthorizationDisposition::Inhibited,
                 "shutdown terminalizes pending authorization");
        requireAllOff (shutdown, "shutdown after pending remains all off");

        require (fixture.sculpture.initialize ().ok (), "shutdown fixture restarts");

        require (!fixture.sculpture.snapshot ().hasPendingAuthorization &&
                     !fixture.sculpture.snapshot ().hasLastTerminalAuthorization,
                 "restart canonically clears both records");
    }

    void testReplayChronologyAndExpiry ()
    {
        Fixture first;
        Fixture second;
        adk::SculptureInput trace[] = {
            input (0, 1, 1, 1, 1, 0, 1000, adk::Level::Low),

            input (1, 2, 2, 2, 2, 0, 1000, adk::Level::High),

            input (9, 3, 3, 3, 3, 0, 1000, adk::Level::High),

            input (10, 4, 4, 4, 4, 0, 1000, adk::Level::High),

            input (130, 5, 5, 5, 5, 0, 1000, adk::Level::High)};
        for (const adk::SculptureInput& frame : trace)
        {
            require (first.sculpture.update (frame).ok () &&
                         second.sculpture.update (frame).ok (),
                     "golden trace accepted twice");
            require (snapshotEqual (first.sculpture.snapshot (),
                                   second.sculpture.snapshot ()),
                     "golden replay is byte-semantic identical");
        }
        const adk::SculptureSnapshot before = first.sculpture.snapshot ();
        adk::SculptureInput regression =
            input (131, 4, 6, 6, 6, 0, 1000, adk::Level::High);
        require (!first.sculpture.update (regression).ok (),
                 "frame regression rejected");
        require (snapshotEqual (before, first.sculpture.snapshot ()),
                 "chronology rejection is atomic");

        Fixture expiry;
        qualifyTouch (expiry, 0, 1000, 0);
        adk::SculptureInput start =
            expiry.next (10, 1000, 0, adk::Level::High);
        require (expiry.sculpture.update (start).ok (), "long motif starts");
        adk::SculptureInput release =
            expiry.next (11, 1000, 0, adk::Level::Low);
        require (expiry.sculpture.update (release).ok (), "release begins");
        adk::SculptureInput released =
            expiry.next (19, 1000, 0, adk::Level::Low);
        require (expiry.sculpture.update (released).ok (), "release qualifies");
        adk::SculptureInput overdue =
            expiry.next (2011, 1000, 0, adk::Level::Low);
        expiry.sculpture.update (overdue);

        require (expiry.sculpture.snapshot ().phase == adk::SculpturePhase::Fault,
                 "command expiry enters fault");
        requireAllOff (expiry.sculpture.snapshot (), "expiry is all off");
    }
}

int main ()
{
    testLifecycleAndConfiguration ();

    testStagedAuthorizationAndMotifs ();

    testStopDominanceAndRecovery ();

    testAtomicStructuralRejectionAndFaultGate ();

    testStopStatusAttribution ();

    testTravelBoundAndOppositeRecovery ();

    testAuthorizationShutdown ();

    testReplayChronologyAndExpiry ();
    return EXIT_SUCCESS;
}
// clang-format on
