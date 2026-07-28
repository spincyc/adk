#include <course_marshal.h>

#include <cstdlib>
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

    void requireStatus (adk::Status status, adk::StatusCode expected,
                        const char* message)
    {
        require (status.error () == expected, message);
    }

    adk::PirPresenceState pirState (uint32_t      now,
                                    adk::PirPhase phase = adk::PirPhase::Motion,
                                    bool available = true, bool valid = true,
                                    bool        stale  = false,
                                    adk::Status status = adk::Status ())
    {
        return {
            available,
            {7, adk::TimePoint (now),
             phase == adk::PirPhase::ReadyClear ? adk::Level::Low : adk::Level::High,
             phase, false, false, adk::Duration (), status},
            adk::Duration (),
            valid,
            stale};
    }

    adk::CourseStartInput startInput (uint32_t now, bool pressed,
                                      adk::PirPhase phase = adk::PirPhase::Motion)
    {
        return {adk::TimePoint (now), 9, pressed, pirState (now, phase)};
    }

    adk::CourseStartEvent absentStart ()
    {
        return adk::CourseStartEvent ();
    }

    adk::CourseStartEvent acceptedStart (uint32_t now)
    {
        return {true,
                adk::CourseStartSource::ExplicitButtonWithPirEligibility,
                9,
                adk::TimePoint (now),

                pirState (now),

                adk::Status ()};
    }

    adk::OpticalPresenceState opticalState (uint32_t now, bool active = false,
                                            bool        event  = false,
                                            adk::Status status = adk::Status ())
    {
        return {true,
                {20, 4, adk::TimePoint (now)},

                status.ok () ? adk::OpticalQuality::Valid
                             : adk::OpticalQuality::SourceFault,
                adk::Duration (),

                status.ok (),
                false,
                active,
                event,
                false,
                status};
    }

    adk::RangePresenceState rangeState (uint32_t now, bool approach = false,
                                        adk::Status status = adk::Status ())
    {
        return {true,
                {30,
                 adk::TimePoint (now),

                 adk::TimePoint (now),

                 adk::MicrosecondTimePoint (1000),

                 adk::MicrosecondDuration (600),

                 {adk::RangeState::Valid, 250, adk::MicrosecondDuration (500), true},
                 status},
                adk::Duration (),

                status.ok (),
                false,
                approach};
    }

    adk::PresenceSnapshot presence (uint32_t      now,
                                    adk::PirPhase phase = adk::PirPhase::Motion,
                                    bool finish = false, bool finishEvent = false,
                                    bool approach = false)
    {
        return {pirState (now, phase),
                opticalState (now),

                opticalState (now, finish, finishEvent),

                rangeState (now, approach),
                phase == adk::PirPhase::Motion,
                false,
                false,
                adk::Duration (),
                adk::PresenceQuality::Valid,
                adk::Status ()};
    }

    adk::CheckpointEvent checkpoint (uint8_t slot, uint32_t now)
    {
        return {{static_cast<uint8_t> (slot + 1U)},
                slot % 2U == 0U ? adk::OpticalSourceKind::InterruptedDigital
                                : adk::OpticalSourceKind::ReflectiveAnalog,
                {static_cast<uint8_t> (10U + slot), static_cast<uint16_t> (100U + slot),
                 adk::TimePoint (now)},
                adk::OpticalQuality::Valid,
                adk::Status ()};
    }

    adk::CourseMarshalConfig config (uint8_t count = 2, bool nonmonotonicIds = false)
    {
        adk::CourseMarshalConfig value = {
            {{{1}, adk::OpticalSourceKind::InterruptedDigital, 10, 100},
             {{2}, adk::OpticalSourceKind::ReflectiveAnalog, 11, 101},
             {{3}, adk::OpticalSourceKind::InterruptedDigital, 12, 102},
             {{4}, adk::OpticalSourceKind::ReflectiveAnalog, 13, 103}},
            count,
            9,
            adk::Duration (20),

            adk::Duration (2),

            adk::Duration (3),

            adk::Duration (100)};

        for (uint8_t index = count; index < 4; ++index)
        {
            value.orderedCheckpoints[index] = {
                {0}, adk::OpticalSourceKind::ReflectiveAnalog, 0, 0};
        }
        if (nonmonotonicIds && count >= 2)
        {
            value.orderedCheckpoints[0].checkpointId.value = 42;
            value.orderedCheckpoints[1].checkpointId.value = 7;
        }
        return value;
    }

    adk::CourseMarshalInputView input (uint32_t                     now,
                                       const adk::PresenceSnapshot& currentPresence,
                                       adk::CourseStartEvent start = absentStart (),
                                       const adk::CheckpointEvent* events     = nullptr,
                                       uint8_t                     eventCount = 0)
    {
        return {adk::TimePoint (now), start, &currentPresence, events, eventCount};
    }

    struct MarshalFixture
    {
        adk::CourseRunStorage             runStorage;
        adk::CourseTriggerStorage         triggerStorage;
        adk::CourseTriggerPresenceStorage triggerPresenceStorage;
        adk::CourseReplayFrameStorage     replayFrameStorage;
        adk::CourseReplayPresenceStorage  replayPresenceStorage;
        adk::CourseReplayEventStorage     replayEventStorage;
        adk::CourseMarshalConfig          marshalConfig;
        adk::CourseMarshal                marshal;

        explicit MarshalFixture (uint8_t count = 2, bool nonmonotonicIds = false)
            : marshalConfig (config (count, nonmonotonicIds)),
              marshal (marshalConfig, runStorage, triggerStorage,
                       triggerPresenceStorage, replayFrameStorage,
                       replayPresenceStorage, replayEventStorage)
        {
        }
    };

    void makeReady (adk::CourseMarshal& marshal, uint32_t now)
    {
        adk::PresenceSnapshot eligible = presence (now);

        requireStatus (marshal.update (input (now, eligible)), adk::StatusCode::Ok,
                       "healthy evidence readies marshal");
        require (marshal.snapshot ().phase == adk::MarshalPhase::Ready,
                 "motion eligibility reaches ready");
    }

    void startRun (adk::CourseMarshal& marshal, uint32_t now)
    {
        adk::PresenceSnapshot eligible = presence (now);

        requireStatus (marshal.update (input (now, eligible, acceptedStart (now))),
                       adk::StatusCode::Ok, "explicit authorized start accepted");
        require (marshal.snapshot ().phase == adk::MarshalPhase::Running,
                 "authorized start begins running");
    }

    void testObjectBoundsAndOwnership ()
    {
        static_assert (sizeof (adk::CourseStartPolicy) <= 128,
                       "start policy exceeds AVR object limit");
        static_assert (sizeof (adk::CourseMarshal) <= 128,
                       "marshal exceeds AVR object limit");
        static_assert (sizeof (adk::CourseMarshalPresenter) <= 128,
                       "presenter exceeds AVR object limit");
        static_assert (sizeof (adk::CourseRunStorage) <= 128,
                       "run storage exceeds AVR object limit");
        static_assert (sizeof (adk::CourseTriggerStorage) <= 128,
                       "trigger storage exceeds AVR object limit");
        static_assert (sizeof (adk::CourseReplayFrameStorage) <= 128,
                       "replay frame storage exceeds AVR object limit");
        static_assert (sizeof (adk::CourseReplayEventStorage) <= 128,
                       "replay event storage exceeds AVR object limit");
        static_assert (sizeof (adk::CourseMarshalInputView) <= 128,
                       "input view exceeds AVR object limit");
        static_assert (sizeof (adk::CourseMarshalSnapshot) <= 128,
                       "snapshot exceeds AVR object limit");
        static_assert (sizeof (adk::CoursePresentationIntent) <= 128,
                       "presentation exceeds AVR object limit");
        static_assert (!std::is_copy_constructible<adk::CourseStartPolicy>::value,
                       "start policy state cannot be copied");
        static_assert (!std::is_copy_constructible<adk::CourseMarshal>::value,
                       "marshal retains caller storage and cannot be copied");
        static_assert (!std::is_copy_constructible<adk::CourseMarshalPresenter>::value,
                       "presenter state cannot be copied");
    }

    void testStartAuthority ()
    {
        adk::CourseStartPolicy policy (9);

        requireStatus (policy.update (startInput (1, true)),
                       adk::StatusCode::NotInitialized,
                       "start policy rejects update before initialize");
        requireStatus (policy.initialize (), adk::StatusCode::Ok,
                       "start policy initializes");
        requireStatus (policy.initialize (), adk::StatusCode::Ok,
                       "start policy initialize is idempotent");
        require (!policy.snapshot ().present, "initial start is absent");

        require (policy.snapshot ().source == adk::CourseStartSource::None,
                 "absent start has canonical source");

        requireStatus (policy.update (startInput (10, false)), adk::StatusCode::Ok,
                       "PIR-only frame is accepted as evidence");
        require (!policy.snapshot ().present, "PIR alone never starts");

        adk::CourseStartInput wrongSource = startInput (11, true);
        wrongSource.buttonSourceId        = 8;
        requireStatus (policy.update (wrongSource), adk::StatusCode::InvalidArgument,
                       "wrong button source rejected");
        require (!policy.snapshot ().present, "wrong button source cannot authorize");

        const adk::PirPhase ineligible[] = {
            adk::PirPhase::Warming, adk::PirPhase::ReadyClear,
            adk::PirPhase::StuckMotion, adk::PirPhase::Fault};
        uint32_t now = 20;
        for (const auto phase : ineligible)
        {
            policy.reset ();

            requireStatus (policy.update (startInput (now++, true, phase)),
                           adk::StatusCode::Ok,
                           "ineligible PIR phase remains a healthy frame");
            require (!policy.snapshot ().present,
                     "ineligible PIR phase cannot authorize");
        }

        policy.reset ();

        adk::CourseStartInput stale = startInput (30, true);
        stale.pir.stale             = true;
        requireStatus (policy.update (stale), adk::StatusCode::Ok,
                       "stale PIR is represented");
        require (!policy.snapshot ().present, "stale PIR cannot authorize");

        policy.reset ();

        adk::CourseStartInput invalidPir = startInput (35, true);
        invalidPir.pir.evidence.phase    = static_cast<adk::PirPhase> (255);
        requireStatus (policy.update (invalidPir), adk::StatusCode::InvalidArgument,
                       "invalid PIR phase is rejected without undefined behavior");

        policy.reset ();

        requireStatus (policy.update (startInput (40, true)), adk::StatusCode::Ok,
                       "eligible explicit press authorizes");
        const adk::CourseStartEvent event = policy.snapshot ();

        require (event.present, "eligible explicit press emits event");

        require (event.source ==
                     adk::CourseStartSource::ExplicitButtonWithPirEligibility,
                 "event identifies explicit authority");
        require (event.buttonSourceId == 9 && event.observedAt == adk::TimePoint (40),
                 "event preserves source and frame");

        requireStatus (policy.update (startInput (41, false)), adk::StatusCode::Ok,
                       "later no-press frame clears event");
        require (!policy.snapshot ().present,
                 "authorization is one frame and never queued");

        policy.reset ();

        const adk::CourseStartInput replay = startInput (50, true);

        requireStatus (policy.update (replay), adk::StatusCode::Ok,
                       "start replay first update");
        const adk::CourseStartEvent stable = policy.snapshot ();

        requireStatus (policy.update (replay), adk::StatusCode::Ok,
                       "identical start frame is idempotent");
        require (policy.snapshot ().present == stable.present &&
                     policy.snapshot ().observedAt == stable.observedAt,
                 "identical start replay remains stable");
        adk::CourseStartInput changed = replay;
        changed.buttonPressEvent      = false;
        requireStatus (policy.update (changed), adk::StatusCode::InvalidArgument,
                       "changed same-time start frame faults");
        require (policy.snapshot ().present == stable.present,
                 "changed same-time failure has no partial mutation");
    }

    void testLifecycleAndConfiguration ()
    {
        MarshalFixture        fixture;
        adk::PresenceSnapshot healthy = presence (1);

        requireStatus (fixture.marshal.update (input (1, healthy)),
                       adk::StatusCode::NotInitialized,
                       "marshal rejects update before initialize");
        requireStatus (fixture.marshal.initialize (), adk::StatusCode::Ok,
                       "marshal initializes");
        requireStatus (fixture.marshal.initialize (), adk::StatusCode::Ok,
                       "marshal initialize is idempotent");
        require (fixture.marshal.snapshot ().phase == adk::MarshalPhase::Disarmed,
                 "marshal starts disarmed");
        require (!fixture.marshal.snapshot ().hasRecord,
                 "marshal starts without record");
        makeReady (fixture.marshal, 1);

        requireStatus (fixture.marshal.initialize (), adk::StatusCode::Ok,
                       "repeated initialize succeeds");
        require (fixture.marshal.snapshot ().phase == adk::MarshalPhase::Ready,
                 "repeated initialize does not reset live state");

        for (uint8_t count : {static_cast<uint8_t> (0), static_cast<uint8_t> (5)})
        {
            MarshalFixture invalid (count);

            requireStatus (invalid.marshal.initialize (),
                           adk::StatusCode::InvalidConfiguration,
                           "checkpoint count outside capacity rejected");
        }

        adk::CourseMarshalConfig duplicate                 = config ();
        duplicate.orderedCheckpoints[1].checkpointId.value = 1;
        adk::CourseRunStorage             run;
        adk::CourseTriggerStorage         trigger;
        adk::CourseTriggerPresenceStorage triggerPresence;
        adk::CourseReplayFrameStorage     replayFrame;
        adk::CourseReplayPresenceStorage  replayPresence;
        adk::CourseReplayEventStorage     replayEvents;
        adk::CourseMarshal duplicateMarshal (duplicate, run, trigger, triggerPresence,
                                             replayFrame, replayPresence, replayEvents);
        requireStatus (duplicateMarshal.initialize (),
                       adk::StatusCode::InvalidConfiguration,
                       "duplicate checkpoint id rejected");

        adk::CourseMarshalConfig duplicateSource = config ();
        duplicateSource.orderedCheckpoints[1].sourceKind =
            duplicateSource.orderedCheckpoints[0].sourceKind;
        duplicateSource.orderedCheckpoints[1].sourceId =
            duplicateSource.orderedCheckpoints[0].sourceId;
        adk::CourseMarshal duplicateSourceMarshal (duplicateSource, run, trigger,
                                                   triggerPresence, replayFrame,
                                                   replayPresence, replayEvents);
        requireStatus (duplicateSourceMarshal.initialize (),
                       adk::StatusCode::InvalidConfiguration,
                       "duplicate source identity rejected");

        MarshalFixture nullPresence;
        requireStatus (nullPresence.marshal.initialize (), adk::StatusCode::Ok,
                       "null-presence fixture initializes");
        const adk::CourseMarshalInputView missingPresence = {
            adk::TimePoint (2), absentStart (), nullptr, nullptr, 0};
        requireStatus (nullPresence.marshal.update (missingPresence),
                       adk::StatusCode::InvalidArgument, "null presence view rejected");

        MarshalFixture nullEvents;
        requireStatus (nullEvents.marshal.initialize (), adk::StatusCode::Ok,
                       "null-events fixture initializes");
        adk::PresenceSnapshot validPresence = presence (2);

        requireStatus (nullEvents.marshal.update ({adk::TimePoint (2), absentStart (),
                                                   &validPresence, nullptr, 1}),
                       adk::StatusCode::InvalidArgument,
                       "nonzero count with null event view rejected");
    }

    void testAuthorizationAndOrdering ()
    {
        MarshalFixture fixture;
        requireStatus (fixture.marshal.initialize (), adk::StatusCode::Ok,
                       "ordering fixture initializes");
        makeReady (fixture.marshal, 10);

        adk::PresenceSnapshot eligible = presence (11);

        requireStatus (fixture.marshal.update (input (11, eligible, absentStart ())),
                       adk::StatusCode::Ok,
                       "ready evidence without button remains healthy");
        require (fixture.marshal.snapshot ().phase == adk::MarshalPhase::Ready,
                 "PIR level cannot start marshal");

        adk::CourseStartEvent aged = acceptedStart (10);

        requireStatus (fixture.marshal.update (input (11, eligible, aged)),
                       adk::StatusCode::InvalidArgument, "aged start event rejected");
        require (fixture.marshal.snapshot ().phase == adk::MarshalPhase::Fault &&
                     !fixture.marshal.snapshot ().hasRecord,
                 "aged start faults without beginning or recording a run");

        fixture.marshal.reset ();

        makeReady (fixture.marshal, 20);

        startRun (fixture.marshal, 21);

        adk::CheckpointEvent first = checkpoint (0, 25);

        adk::PresenceSnapshot runPresence = presence (25);

        requireStatus (
            fixture.marshal.update (input (25, runPresence, absentStart (), &first, 1)),
            adk::StatusCode::Ok, "first checkpoint accepted");
        require (fixture.marshal.snapshot ().acceptedCheckpointCount == 1 &&
                     fixture.marshal.snapshot ().expectedSlot == 1,
                 "first checkpoint advances semantic order");

        requireStatus (
            fixture.marshal.update (input (25, runPresence, absentStart (), &first, 1)),
            adk::StatusCode::Ok, "identical checkpoint frame is idempotent");
        require (fixture.marshal.snapshot ().acceptedCheckpointCount == 1,
                 "replay does not consume checkpoint twice");

        adk::CheckpointEvent changed = first;
        changed.quality              = adk::OpticalQuality::AboveQualifiedRange;
        requireStatus (fixture.marshal.update (
                           input (25, runPresence, absentStart (), &changed, 1)),
                       adk::StatusCode::InvalidArgument,
                       "changed same-time checkpoint frame faults atomically");
        require (fixture.marshal.snapshot ().acceptedCheckpointCount == 1,
                 "changed replay does not partially advance");

        MarshalFixture forged;
        requireStatus (forged.marshal.initialize (), adk::StatusCode::Ok,
                       "forged-start fixture initializes");
        makeReady (forged.marshal, 30);

        adk::PresenceSnapshot forgedPresence = presence (31);

        adk::CourseStartEvent wrongButton = acceptedStart (31);
        wrongButton.buttonSourceId        = 8;
        requireStatus (forged.marshal.update (input (31, forgedPresence, wrongButton)),
                       adk::StatusCode::InvalidArgument,
                       "marshal rejects forged button-source authorization");
        require (forged.marshal.snapshot ().hasRecord &&
                     forged.marshal.snapshot ().disposition ==
                         adk::RunDisposition::EvidenceFault,
                 "forged authorization freezes evidence-fault record");
        require (forged.marshal.trigger ().kind == adk::RunTriggerKind::Start &&
                     forged.marshal.record ().status.error () ==
                         adk::StatusCode::InvalidArgument &&
                     forged.marshal.trigger ().status.error () ==
                         adk::StatusCode::InvalidArgument,
                 "forged start preserves trigger and failure status");
        const uint32_t forgedSequence = forged.marshal.record ().sequence;

        forged.marshal.acknowledgeRecord ();

        adk::PresenceSnapshot secondPresence = presence (32);

        adk::CourseStartEvent secondWrong = acceptedStart (32);
        secondWrong.buttonSourceId        = 8;
        requireStatus (forged.marshal.update (input (32, secondPresence, secondWrong)),
                       adk::StatusCode::InvalidArgument,
                       "second forged start is independently recorded");
        require (forged.marshal.record ().sequence == forgedSequence + 1U,
                 "malformed start advances sequence after acknowledgment");

        MarshalFixture noncanonicalAbsent;
        requireStatus (noncanonicalAbsent.marshal.initialize (), adk::StatusCode::Ok,
                       "noncanonical-absent fixture initializes");
        makeReady (noncanonicalAbsent.marshal, 50);

        adk::PresenceSnapshot absentPresence = presence (51);

        adk::CourseStartEvent malformedAbsent = absentStart ();
        malformedAbsent.source =
            adk::CourseStartSource::ExplicitButtonWithPirEligibility;
        requireStatus (noncanonicalAbsent.marshal.update (
                           input (51, absentPresence, malformedAbsent)),
                       adk::StatusCode::InvalidArgument,
                       "noncanonical absent start is rejected");

        MarshalFixture alteredPir;
        requireStatus (alteredPir.marshal.initialize (), adk::StatusCode::Ok,
                       "altered-PIR fixture initializes");
        makeReady (alteredPir.marshal, 40);

        adk::PresenceSnapshot copiedPresence = presence (41);

        adk::CourseStartEvent forgedPir = acceptedStart (41);
        forgedPir.pir.evidence.sourceId = 99;
        requireStatus (
            alteredPir.marshal.update (input (41, copiedPresence, forgedPir)),
            adk::StatusCode::InvalidArgument,
            "marshal rejects start whose copied PIR differs from frame");
        require (alteredPir.marshal.snapshot ().phase == adk::MarshalPhase::Rejected &&
                     alteredPir.marshal.trigger ().kind == adk::RunTriggerKind::Start,
                 "forged copied evidence freezes start-fault evidence");
    }

    void testCapacityOrderAndTerminalRecord ()
    {
        MarshalFixture fixture (4);

        requireStatus (fixture.marshal.initialize (), adk::StatusCode::Ok,
                       "capacity fixture initializes");
        makeReady (fixture.marshal, 100);

        startRun (fixture.marshal, 101);

        adk::PresenceSnapshot active = presence (110);
        adk::CheckpointEvent  over[5];
        requireStatus (
            fixture.marshal.update (input (110, active, absentStart (), over, 5)),
            adk::StatusCode::CapacityExceeded,
            "above four checkpoint events rejected before reading array");
        require (fixture.marshal.snapshot ().acceptedCheckpointCount == 0,
                 "capacity failure is atomic");

        adk::CheckpointEvent reversed = checkpoint (1, 111);

        requireStatus (
            fixture.marshal.update (input (111, active, absentStart (), &reversed, 1)),
            adk::StatusCode::Ok, "reversed checkpoint produces terminal record");
        require (fixture.marshal.snapshot ().phase == adk::MarshalPhase::Rejected,
                 "reversed checkpoint rejects run");
        require (fixture.marshal.snapshot ().disposition ==
                         adk::RunDisposition::ReversedCheckpoint ||
                     fixture.marshal.snapshot ().disposition ==
                         adk::RunDisposition::SkippedCheckpoint,
                 "order failure identifies order disposition");
        require (fixture.marshal.snapshot ().hasRecord, "rejected run latches record");

        require (fixture.marshal.trigger ().kind == adk::RunTriggerKind::Checkpoint,
                 "order failure records checkpoint trigger");

        const uint32_t sequence = fixture.marshal.record ().sequence;

        fixture.marshal.reset ();

        require (!fixture.marshal.snapshot ().hasRecord,
                 "reset clears retained record");
        require (fixture.marshal.record ().sequence == 0,
                 "reset canonicalizes caller-owned record");
        makeReady (fixture.marshal, 200);

        startRun (fixture.marshal, 201);

        require (fixture.marshal.snapshot ().recordSequence == 0,
                 "active run does not expose terminal record sequence");
        require (sequence != 0, "terminal records use nonzero sequence");
    }

    void testMalformedStartFieldMatrix ()
    {
        for (uint8_t variant = 0; variant < 6; ++variant)
        {
            MarshalFixture fixture;
            requireStatus (fixture.marshal.initialize (), adk::StatusCode::Ok,
                           "malformed-start matrix fixture initializes");
            const uint32_t now = static_cast<uint32_t> (600U + variant * 10U);
            makeReady (fixture.marshal, now);

            adk::PresenceSnapshot frame = presence (now + 1U);

            adk::CourseStartEvent start = acceptedStart (now + 1U);
            switch (variant)
            {
                case 0: start.observedAt = adk::TimePoint (now); break;
                case 1: start.status = adk::StatusCode::HardwareFailure; break;
                case 2: start.pir.available = false; break;
                case 3: start.pir.valid = false; break;
                case 4: start.pir.stale = true; break;
                case 5: start.pir.evidence.rawLevel = adk::Level::Low; break;
            }
            requireStatus (fixture.marshal.update (input (now + 1U, frame, start)),
                           adk::StatusCode::InvalidArgument,
                           "each malformed present-start field is rejected");
            require (fixture.marshal.snapshot ().hasRecord &&
                         fixture.marshal.snapshot ().disposition ==
                             adk::RunDisposition::EvidenceFault &&
                         fixture.marshal.trigger ().kind == adk::RunTriggerKind::Start,
                     "each malformed present start freezes start evidence");
            require (fixture.marshal.trigger ().start.present &&
                         fixture.marshal.record ().start.present,
                     "malformed start tuple is retained in record and trigger");
        }
    }

    void testMalformedCheckpointPermutationAndAtomicReplay ()
    {
        for (uint8_t permutation = 0; permutation < 2; ++permutation)
        {
            MarshalFixture fixture (2, true);

            requireStatus (fixture.marshal.initialize (), adk::StatusCode::Ok,
                           "malformed-checkpoint fixture initializes");
            makeReady (fixture.marshal, 700);

            startRun (fixture.marshal, 701);
            const adk::TimePoint replayAt = fixture.replayFrameStorage.observedAt;

            adk::CheckpointEvent events[2] = {
                checkpoint (permutation == 0 ? 0 : 1, 710),

                checkpoint (permutation == 0 ? 1 : 0, 710)};
            events[0].checkpointId.value  = permutation == 0 ? 42 : 7;
            events[1].checkpointId.value  = permutation == 0 ? 7 : 42;
            events[0].quality             = adk::OpticalQuality::SourceFault;
            events[1].quality             = adk::OpticalQuality::SourceFault;
            adk::PresenceSnapshot current = presence (710);

            requireStatus (
                fixture.marshal.update (
                    input (710, current, absentStart (), events, 2)),
                adk::StatusCode::InvalidArgument,
                "malformed checkpoint set rejects independently of array order");
            require (fixture.marshal.trigger ().kind ==
                             adk::RunTriggerKind::Checkpoint &&
                         fixture.marshal.trigger ().checkpointCount == 2,
                     "all malformed checkpoints freeze in one dense trigger");
            require (
                fixture.marshal.trigger ().checkpoints[0].checkpointId.value == 42 &&
                    fixture.marshal.trigger ().checkpoints[1].checkpointId.value == 7,
                "malformed checkpoint trigger is semantic-slot sorted");
            require (fixture.marshal.record ().disposition ==
                             adk::RunDisposition::EvidenceFault &&
                         fixture.marshal.record ().acceptedCheckpointCount == 0,
                     "malformed checkpoint record preserves accepted prefix");
            require (fixture.replayFrameStorage.observedAt == replayAt &&
                         fixture.replayFrameStorage.eventCount == 0,
                     "failed preflight does not mutate replay frame storage");
        }
    }

    void testValidFinishAndAcknowledgment ()
    {
        MarshalFixture fixture (1);

        requireStatus (fixture.marshal.initialize (), adk::StatusCode::Ok,
                       "finish fixture initializes");
        makeReady (fixture.marshal, 300);

        startRun (fixture.marshal, 301);

        adk::CheckpointEvent event = checkpoint (0, 305);

        adk::PresenceSnapshot run = presence (305);

        requireStatus (
            fixture.marshal.update (input (305, run, absentStart (), &event, 1)),
            adk::StatusCode::Ok, "only checkpoint accepted");

        adk::PresenceSnapshot finish =
            presence (310, adk::PirPhase::Motion, true, true, true);
        requireStatus (fixture.marshal.update (input (310, finish)),
                       adk::StatusCode::Ok, "agreeing finish accepted");
        require (fixture.marshal.snapshot ().phase == adk::MarshalPhase::Finished,
                 "valid course finishes");
        require (fixture.marshal.snapshot ().disposition ==
                     adk::RunDisposition::Accepted,
                 "valid course accepted");
        require (fixture.marshal.record ().acceptedCheckpointCount == 1 &&
                     fixture.marshal.record ().elapsed == adk::Duration (9),
                 "record freezes prefix and elapsed");
        require (fixture.marshal.trigger ().kind == adk::RunTriggerKind::FinishGuard,
                 "accepted finish has discriminated finish trigger");

        const adk::CourseRunRecord terminal        = fixture.marshal.record ();
        adk::PresenceSnapshot      changedTerminal = finish;
        changedTerminal.passageEvent               = true;
        requireStatus (fixture.marshal.update (input (310, changedTerminal)),
                       adk::StatusCode::Ok,
                       "terminal record ignores later changed frame");
        require (fixture.marshal.snapshot ().phase == adk::MarshalPhase::Finished &&
                     fixture.marshal.record ().sequence == terminal.sequence &&

                     fixture.marshal.record ().elapsed == terminal.elapsed,
                 "terminal record remains immutable until acknowledgment");

        fixture.marshal.acknowledgeRecord ();

        require (!fixture.marshal.snapshot ().hasRecord,
                 "acknowledgment clears retained record");
        require (fixture.marshal.snapshot ().phase == adk::MarshalPhase::Disarmed,
                 "acknowledgment returns to no-run phase");
    }

    void testTimeout ()
    {
        MarshalFixture fixture (1);

        requireStatus (fixture.marshal.initialize (), adk::StatusCode::Ok,
                       "timeout fixture initializes");
        makeReady (fixture.marshal, 1000);

        startRun (fixture.marshal, 1001);

        adk::PresenceSnapshot active = presence (1102);

        requireStatus (fixture.marshal.update (input (1102, active)),
                       adk::StatusCode::Ok, "timeout is a terminal disposition");
        require (fixture.marshal.snapshot ().phase == adk::MarshalPhase::Rejected &&
                     fixture.marshal.snapshot ().disposition ==
                         adk::RunDisposition::TimedOut,
                 "maximum duration rejects timed-out run");
        require (fixture.marshal.trigger ().kind == adk::RunTriggerKind::RunTimeout,
                 "timeout trigger is discriminated");
    }

    void testFinishEvidenceFaultAndProjection ()
    {
        MarshalFixture fixture (1);

        requireStatus (fixture.marshal.initialize (), adk::StatusCode::Ok,
                       "finish-fault fixture initializes");
        makeReady (fixture.marshal, 400);

        startRun (fixture.marshal, 401);

        adk::CheckpointEvent event = checkpoint (0, 405);

        adk::PresenceSnapshot run = presence (405);

        requireStatus (
            fixture.marshal.update (input (405, run, absentStart (), &event, 1)),
            adk::StatusCode::Ok, "finish-fault checkpoint accepted");

        adk::PresenceSnapshot staleRange =
            presence (410, adk::PirPhase::Motion, true, true, true);
        staleRange.range.stale = true;
        requireStatus (fixture.marshal.update (input (410, staleRange)),
                       adk::StatusCode::InvalidArgument,
                       "stale finish range propagates failure");
        require (fixture.marshal.snapshot ().disposition ==
                         adk::RunDisposition::EvidenceFault &&
                     fixture.marshal.record ().status.error () ==
                         adk::StatusCode::InvalidArgument &&
                     fixture.marshal.trigger ().status.error () ==
                         adk::StatusCode::InvalidArgument,
                 "stale range freezes evidence-fault statuses");
        require (fixture.marshal.trigger ().kind == adk::RunTriggerKind::Range,
                 "stale range selects range trigger");
        const adk::PresenceSnapshot& selected = fixture.marshal.triggerPresence ();

        require (selected.range.available && selected.range.stale &&
                     selected.range.approachValid,
                 "range trigger retains selected range evidence");
        require (!selected.pir.available && !selected.beam.available &&
                     !selected.finishGuard.available &&
                     selected.quality == adk::PresenceQuality::Unqualified &&
                     selected.status.ok (),
                 "range trigger canonicalizes inactive presence payloads");
    }

    adk::CourseMarshalSnapshot presentationSnapshot (
        adk::MarshalPhase phase, uint8_t accepted = 0, uint8_t expected = 0,
        uint32_t elapsed = 0, bool hasRecord = false, uint32_t sequence = 0,
        adk::RunDisposition disposition = adk::RunDisposition::None,
        adk::Status status = adk::Status (), uint8_t checkpointCount = 4)
    {
        return {phase,
                checkpointCount,
                expected,
                {expected < checkpointCount ? static_cast<uint8_t> (expected + 1U)
                                            : static_cast<uint8_t> (0)},
                accepted,
                adk::Duration (elapsed),
                hasRecord,
                sequence,
                disposition,
                status};
    }

    void testPresenter ()
    {
        adk::CourseMarshalPresenter presenter (adk::Duration (10), adk::Duration (20));

        requireStatus (
            presenter.update (adk::TimePoint (1),
                              presentationSnapshot (adk::MarshalPhase::Ready)),
            adk::StatusCode::NotInitialized,
            "presenter rejects update before initialize");
        requireStatus (presenter.initialize (), adk::StatusCode::Ok,
                       "presenter initializes");
        require (presenter.intent ().phase == adk::CoursePresentationPhase::Starting &&
                     presenter.intent ().status.ok (),
                 "presenter begins with canonical starting intent");

        requireStatus (
            presenter.update (adk::TimePoint (100),
                              presentationSnapshot (adk::MarshalPhase::Ready, 0, 0)),
            adk::StatusCode::Ok, "presenter accepts ready snapshot");
        require (presenter.intent ().phase == adk::CoursePresentationPhase::Ready &&
                     presenter.intent ().displayCell == 0 &&

                     presenter.intent ().displayValue == 1,
                 "ready presentation emits expected checkpoint number");
        require (!presenter.intent ().heartbeat,
                 "first presentation establishes quiet heartbeat epoch");

        requireStatus (presenter.update (adk::TimePoint (120),
                                         presentationSnapshot (
                                             adk::MarshalPhase::Running, 3, 3, 1234)),
                       adk::StatusCode::Ok, "running presentation accepted");
        require (presenter.intent ().phase == adk::CoursePresentationPhase::Running &&
                     presenter.intent ().acceptedMask == 0x07 &&

                     presenter.intent ().displayCell == 1 &&

                     presenter.intent ().displayValue == 2 &&

                     presenter.intent ().heartbeat,
                 "running intent maps mask, digit, cell, and heartbeat");

        const adk::CoursePresentationIntent stable = presenter.intent ();

        requireStatus (presenter.update (adk::TimePoint (120),
                                         presentationSnapshot (
                                             adk::MarshalPhase::Running, 3, 3, 1234)),
                       adk::StatusCode::Ok, "identical presenter frame is idempotent");
        require (presenter.intent ().displayCell == stable.displayCell &&
                     presenter.intent ().displayValue == stable.displayValue,
                 "identical presenter replay remains stable");

        requireStatus (
            presenter.update (
                adk::TimePoint (120),

                presentationSnapshot (adk::MarshalPhase::Rejected, 3, 3, 1234, true, 5,
                                      adk::RunDisposition::FinishTooEarly)),
            adk::StatusCode::InvalidArgument,
            "changed same-time presenter frame faults");
        require (presenter.intent ().displayCell == stable.displayCell &&
                     presenter.intent ().displayValue == stable.displayValue,
                 "presenter timing fault retains copied value fields");

        presenter.reset ();

        require (presenter.initialized (), "presenter reset remains initialized");

        require (presenter.intent ().phase == adk::CoursePresentationPhase::Starting,
                 "presenter reset restores canonical intent");

        adk::CourseMarshalPresenter completed (adk::Duration (10), adk::Duration (20));

        requireStatus (completed.initialize (), adk::StatusCode::Ok,
                       "completed presenter fixture initializes");
        requireStatus (
            completed.update (adk::TimePoint (200),
                              presentationSnapshot (
                                  adk::MarshalPhase::Finished, 2, 2, 25, true, 7,
                                  adk::RunDisposition::Accepted, adk::Status (), 2)),
            adk::StatusCode::Ok,
            "two-checkpoint terminal snapshot with zero expected id is valid");
        completed.reset ();

        requireStatus (
            completed.update (
                adk::TimePoint (201),

                presentationSnapshot (adk::MarshalPhase::Running, 2, 2, 25, false, 0,
                                      adk::RunDisposition::None, adk::Status (), 2)),
            adk::StatusCode::Ok,
            "running after all checkpoints may await finish with zero expected id");

        adk::CourseMarshalPresenter malformed (adk::Duration (10), adk::Duration (20));

        requireStatus (malformed.initialize (), adk::StatusCode::Ok,
                       "malformed presenter fixture initializes");
        requireStatus (malformed.update (
                           adk::TimePoint (200),

                           presentationSnapshot (adk::MarshalPhase::Running, 5, 0, 10)),
                       adk::StatusCode::InvalidArgument,
                       "presenter rejects accepted count above capacity");
        malformed.reset ();

        requireStatus (malformed.update (
                           adk::TimePoint (201),

                           presentationSnapshot (static_cast<adk::MarshalPhase> (255))),
                       adk::StatusCode::InvalidArgument,
                       "presenter rejects unknown phase without undefined behavior");
        malformed.reset ();
        adk::CourseMarshalSnapshot zeroExpected =
            presentationSnapshot (adk::MarshalPhase::Ready, 0, 0);
        zeroExpected.expectedCheckpointId.value = 0;
        requireStatus (malformed.update (adk::TimePoint (202), zeroExpected),
                       adk::StatusCode::InvalidArgument,
                       "presenter rejects zero expected id before final slot");
        malformed.reset ();

        requireStatus (
            malformed.update (adk::TimePoint (203),
                              presentationSnapshot (adk::MarshalPhase::Ready, 1, 1)),
            adk::StatusCode::InvalidArgument,
            "presenter rejects impossible ready phase with accepted prefix");
    }
} // namespace

int main ()
{
    testObjectBoundsAndOwnership ();

    testStartAuthority ();

    testLifecycleAndConfiguration ();

    testAuthorizationAndOrdering ();

    testCapacityOrderAndTerminalRecord ();

    testMalformedStartFieldMatrix ();

    testMalformedCheckpointPermutationAndAtomicReplay ();

    testValidFinishAndAcknowledgment ();

    testTimeout ();

    testFinishEvidenceFaultAndProjection ();

    testPresenter ();
    return EXIT_SUCCESS;
}
