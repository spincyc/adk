#include <bounded_stepper_sequence.h>

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

    adk::StepperSequenceConfig
    config (uint32_t minimumInterval = 2, uint32_t maximumInterval = 100,

            uint32_t maximumAge = 1000, int32_t minimumPosition = -256,
            int32_t maximumPosition = 256, bool holdAtRest = false)
    {
        return {adk::Duration (minimumInterval),

                adk::Duration (maximumInterval),

                adk::Duration (maximumAge),

                minimumPosition,
                maximumPosition,
                holdAtRest};
    }

    adk::StepperCommand command (uint32_t id, uint32_t issuedAt,

                                 adk::StepDirection direction, uint32_t steps,
                                 uint32_t interval = 10, bool cancel = false,
                                 adk::Status status = adk::StatusCode::Ok)
    {
        return {id,    adk::TimePoint (issuedAt), direction,

                steps, adk::Duration (interval),  cancel,

                status};
    }

    bool snapshotEqual (const adk::StepperSequenceSnapshot& left,

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

    adk::Status apply (adk::BoundedStepperSequence& policy, uint32_t now,

                       const adk::StepperCommand& input)
    {
        adk::StepperSequencePreview preview;
        const adk::Status           status =
            policy.preview (adk::TimePoint (now), input, preview);


        if (!status.ok ())

        {
            return status;
        }
        require (policy.canCommit (preview), "successful preview is committable");

        return policy.commit (preview);

    }

    void requireOff (const adk::StepperSequenceSnapshot& snapshot, const char* message)

    {
        require (snapshot.coilIntent == 0, message);

    }

    void testLifecycleAndConfiguration ()

    {
        static_assert (!std::is_copy_constructible<adk::BoundedStepperSequence>::value,

                       "step sequence does not copy");
        static_assert (!std::is_move_constructible<adk::BoundedStepperSequence>::value,

                       "step sequence does not move");
        static_assert (

            std::is_trivially_destructible<adk::BoundedStepperSequence>::value,
            "step sequence destruction has no endpoint side effects");

        adk::BoundedStepperSequence policy (config ());

        adk::StepperSequencePreview preview;

        require (!policy.initialized (), "policy starts inert");

        require (policy.preview (adk::TimePoint (0),

                                 command (1, 0, adk::StepDirection::Forward, 1),

                                 preview)
                         .error () == adk::StatusCode::NotInitialized,

                 "preview before initialize rejected");
        require (!policy.commit (preview).ok (), "unbound preview rejected");

        requireOff (policy.snapshot (), "construction is all off");

        require (policy.initialize ().ok () && policy.initialize ().ok (),

                 "initialize is idempotent");
        require (policy.initialized (), "initialized state visible");

        requireOff (policy.snapshot (), "initialization remains all off");


        require (

            apply (policy, 0, command (1, 0, adk::StepDirection::Forward, 2)).ok (),

            "valid motion starts");
        policy.reset ();

        require (policy.initialized (), "reset preserves valid initialization");

        requireOff (policy.snapshot (), "reset clears coil intent");

        require (policy.snapshot ().phase == adk::StepSequencePhase::Inactive &&

                     policy.snapshot ().logicalPosition == 0,

                 "reset restores logical baseline");

        const adk::StepperSequenceConfig invalid[] = {
            config (0, 100, 1000),

            config (2, 0, 1000),

            config (2, 100, 0),

            config (0x80000000UL, 0x80000000UL, 1000),

            config (2, 0x80000000UL, 1000),

            config (2, 100, 0x80000000UL),

            config (100, 2, 1000),

            config (2, 100, 1000, 1, 0),

            config (2, 100, 1000, -5, -1),

            config (2, 100, 1000, 1, 5)};

        for (const auto& rejectedConfig : invalid)

        {
            adk::BoundedStepperSequence rejected (rejectedConfig);

            require (rejected.initialize ().error () ==

                         adk::StatusCode::InvalidConfiguration,
                     "invalid configuration rejected");
            require (!rejected.initialized (),

                     "invalid configuration stays uninitialized");
            requireOff (rejected.snapshot (),

                        "invalid configuration cannot energize intent");
        }

        adk::BoundedStepperSequence boundary (

            config (1, 0x7fffffffUL, 0x7fffffffUL, std::numeric_limits<int32_t>::min (),

                    std::numeric_limits<int32_t>::max ()));

        require (boundary.initialize ().ok (),

                 "wrap-safe durations and full signed travel initialize");
    }

    void testCommandValidationAndAtomicity ()

    {
        adk::BoundedStepperSequence policy (config ());

        require (policy.initialize ().ok (), "validation policy initializes");


        const adk::StepperCommand invalid[] = {
            command (0, 0, adk::StepDirection::Forward, 1),

            command (1, 0, static_cast<adk::StepDirection> (3), 1),

            command (1, 0, adk::StepDirection::Stopped, 1),

            command (1, 0, adk::StepDirection::Forward, 0),

            command (1, 0, adk::StepDirection::Reverse, 0),

            command (1, 0, adk::StepDirection::Forward, 1, 1),

            command (1, 0, adk::StepDirection::Forward, 1, 101),

            command (1, 0, adk::StepDirection::Forward, 1, 10, false,

                     static_cast<adk::StatusCode> (255))};
        for (const auto& rejected : invalid)

        {
            const adk::StepperSequenceSnapshot before = policy.snapshot ();

            adk::StepperSequencePreview        candidate;
            require (!policy.preview (adk::TimePoint (0), rejected, candidate).ok (),

                     "malformed command rejected");
            require (!policy.canCommit (candidate), "rejected preview remains unbound");

            require (snapshotEqual (before, policy.snapshot ()),

                     "rejected command cannot mutate state");
        }

        adk::StepperSequencePreview future;
        require (policy.preview (adk::TimePoint (9),

                                 command (1, 10, adk::StepDirection::Forward, 1),

                                 future)
                         .error () == adk::StatusCode::InvalidArgument,

                 "future issuance rejected");

        adk::BoundedStepperSequence bounded (config (2, 100, 1000, -2, 2));

        require (bounded.initialize ().ok (), "bounded policy initializes");

        adk::StepperSequencePreview tooFar;
        require (bounded.preview (adk::TimePoint (0),

                                  command (1, 0, adk::StepDirection::Forward, 3),

                                  tooFar)
                         .error () == adk::StatusCode::InvalidArgument,

                 "endpoint beyond upper bound rejected");
        require (bounded.preview (adk::TimePoint (0),

                                  command (1, 0, adk::StepDirection::Reverse, 3),

                                  tooFar)
                         .error () == adk::StatusCode::InvalidArgument,

                 "endpoint beyond lower bound rejected");
        requireOff (bounded.snapshot (), "travel rejection remains all off");


        adk::StepperSequencePreview sourceFault;
        require (policy.preview (adk::TimePoint (0),

                                 command (1, 0, adk::StepDirection::Forward, 1, 10,

                                          false, adk::StatusCode::HardwareFailure),
                                 sourceFault)
                             .error () == adk::StatusCode::HardwareFailure &&

                     policy.canCommit (sourceFault),

                 "recognized source fault prepares terminal candidate");
        require (policy.commit (sourceFault).ok () &&

                     policy.snapshot ().phase == adk::StepSequencePhase::Fault,

                 "source fault commits explicit fault");
        requireOff (policy.snapshot (), "source fault clears intent");

    }

    void testEveryFrameAndDirection ()

    {
        const uint8_t forwardFrames[] = {0x01, 0x03, 0x02, 0x06,
                                         0x04, 0x0c, 0x08, 0x09};
        const uint8_t reverseFrames[] = {0x09, 0x08, 0x0c, 0x04,
                                         0x06, 0x02, 0x03, 0x01};

        for (unsigned directionIndex = 0; directionIndex < 2; ++directionIndex)

        {
            adk::BoundedStepperSequence policy (config (2, 100, 1000, -256, 256, true));

            require (policy.initialize ().ok (), "frame policy initializes");

            const adk::StepDirection direction = directionIndex == 0
                                                     ? adk::StepDirection::Forward
                                                     : adk::StepDirection::Reverse;
            const uint8_t*           expected =
                directionIndex == 0 ? forwardFrames : reverseFrames;

            require (apply (policy, 100, command (1, 100, direction, 8)).ok (),

                     "eight-step command admitted");
            for (unsigned step = 0; step < 8; ++step)

            {
                const uint32_t now = 110 + step * 10;
                require (apply (policy, now, command (1, 100, direction, 8)).ok (),

                         "due step commits");
                const adk::StepperSequenceSnapshot snapshot = policy.snapshot ();

                require (snapshot.coilIntent == expected[step],

                         "half-step frame order is exact");
                require (snapshot.completedSteps == step + 1,

                         "one call completes exactly one step");
                require (snapshot.logicalPosition ==

                             (directionIndex == 0 ? static_cast<int32_t> (step + 1)
                                                  : -static_cast<int32_t> (step + 1)),
                         "logical position follows direction");
            }
            require (policy.snapshot ().phase == adk::StepSequencePhase::Complete,

                     "eighth transition completes command");
            require (policy.snapshot ().coilIntent == expected[7],

                     "held completion exposes eighth frame");
        }

        adk::BoundedStepperSequence released (config ());

        require (released.initialize ().ok (), "no-hold policy initializes");

        require (

            apply (released, 0, command (1, 0, adk::StepDirection::Forward, 1)).ok () &&

                apply (released, 10, command (1, 0, adk::StepDirection::Forward, 1))

                    .ok (),

            "one-step no-hold command completes");
        requireOff (released.snapshot (), "no-hold completion clears coil intent");


        adk::BoundedStepperSequence held (config (2, 100, 1000, -256, 256, true));

        require (held.initialize ().ok (), "hold policy initializes");

        require (

            apply (held, 0, command (1, 0, adk::StepDirection::Forward, 1)).ok () &&

                apply (held, 10, command (1, 0, adk::StepDirection::Forward, 1)).ok (),

            "one-step held command completes");
        require (held.snapshot ().phase == adk::StepSequencePhase::Complete &&

                     held.snapshot ().coilIntent == 0x01,

                 "hold completion retains final logical frame");
    }

    void testDeadlineCatchupAndRollover ()

    {
        adk::BoundedStepperSequence policy (config (2, 100, 1000));

        require (policy.initialize ().ok (), "timing policy initializes");

        const adk::StepperCommand input =
            command (1, 100, adk::StepDirection::Forward, 4, 10);


        require (apply (policy, 100, input).ok (), "command admitted");

        require (policy.snapshot ().completedSteps == 0,

                 "admission alone does not step");
        require (apply (policy, 109, input).ok (),

                 "immediately-before deadline accepted");
        require (policy.snapshot ().completedSteps == 0,

                 "before deadline does not step");
        require (apply (policy, 110, input).ok (), "at deadline steps");

        require (policy.snapshot ().completedSteps == 1, "deadline is inclusive");

        require (apply (policy, 135, input).ok (), "overdue update accepted");

        require (policy.snapshot ().completedSteps == 2 &&

                     policy.snapshot ().nextStepAt == adk::TimePoint (130),

                 "overdue call advances one frame without stretching");
        require (apply (policy, 135, input).ok () &&

                     policy.snapshot ().completedSteps == 3,

                 "same-now catch-up advances one further frame");
        require (apply (policy, 135, input).ok () &&

                     policy.snapshot ().completedSteps == 3,

                 "same-now stops when next deadline is future");

        const adk::StepperSequenceSnapshot beforeRegression = policy.snapshot ();

        adk::StepperSequencePreview        regression;
        require (!policy.preview (adk::TimePoint (134), input, regression).ok (),

                 "policy-time regression rejected");
        require (snapshotEqual (beforeRegression, policy.snapshot ()),

                 "time regression cannot mutate");

        adk::BoundedStepperSequence rollover (config (2, 100, 1000));

        require (rollover.initialize ().ok (), "rollover policy initializes");

        const adk::StepperCommand wrapped =
            command (1, 0xfffffff8UL, adk::StepDirection::Forward, 2, 10);

        require (apply (rollover, 0xfffffff8UL, wrapped).ok (),

                 "near-wrap command admitted");
        require (apply (rollover, 2, wrapped).ok () &&

                     rollover.snapshot ().completedSteps == 1,

                 "deadline crosses TimePoint rollover");
        require (apply (rollover, 12, wrapped).ok () &&

                     rollover.snapshot ().completedSteps == 2,

                 "second wrapped deadline completes");
    }

    void testReplayReplacementAndIdentifiers ()

    {
        adk::BoundedStepperSequence policy (config ());

        require (policy.initialize ().ok (), "identifier policy initializes");

        const adk::StepperCommand first =
            command (0xffffffffUL, 0, adk::StepDirection::Forward, 4);

        require (apply (policy, 0, first).ok (), "maximum ID accepted");

        const adk::StepperSequenceSnapshot admitted = policy.snapshot ();

        require (apply (policy, 1, first).ok (), "exact replay is idempotent");

        require (policy.snapshot ().completedSteps == admitted.completedSteps,

                 "exact replay does not fabricate step");

        adk::StepperSequencePreview changed;
        require (

            !policy
                 .preview (adk::TimePoint (1),

                           command (0xffffffffUL, 0, adk::StepDirection::Forward, 3),

                           changed)
                 .ok (),

            "same-ID mutation rejected");

        require (

            apply (policy, 2, command (1, 2, adk::StepDirection::Reverse, 2)).ok (),

            "modular wrapped nonzero ID replaces live command");
        require (policy.snapshot ().commandId == 1 &&

                     policy.snapshot ().disposition ==

                         adk::StepSequenceDisposition::Replaced,
                 "replacement attribution published");

        const adk::StepperSequenceSnapshot before = policy.snapshot ();

        require (

            !policy
                 .preview (adk::TimePoint (3),

                           command (0x80000001UL, 3, adk::StepDirection::Forward, 1),

                           changed)
                 .ok (),

            "ambiguous half-range ID rejected");
        require (

            !policy
                 .preview (adk::TimePoint (3),

                           command (0xfffffffeUL, 3, adk::StepDirection::Forward, 1),

                           changed)
                 .ok (),

            "regressed ID rejected");
        require (snapshotEqual (before, policy.snapshot ()),

                 "identifier rejection is atomic");
    }

    void testCancellationStopExpiryAndRecovery ()

    {
        adk::BoundedStepperSequence policy (config (2, 100, 20));

        require (policy.initialize ().ok (), "terminal policy initializes");

        require (

            apply (policy, 0, command (1, 0, adk::StepDirection::Forward, 4)).ok (),

            "motion admitted");
        require (

            apply (policy, 10, command (1, 0, adk::StepDirection::Forward, 4)).ok (),

            "motion energizes intent");
        require (policy.snapshot ().coilIntent != 0, "motion has logical frame");


        require (apply (policy, 11,

                        command (2, 11, adk::StepDirection::Stopped, 0, 10, true))

                     .ok (),

                 "valid cancel accepted");
        require (policy.snapshot ().phase == adk::StepSequencePhase::Cancelled &&

                     policy.snapshot ().disposition ==

                         adk::StepSequenceDisposition::Cancelled,
                 "cancel terminal state published");
        requireOff (policy.snapshot (), "cancel clears all intent bits");


        adk::StepperSequencePreview idleCancel;
        require (

            !policy
                 .preview (adk::TimePoint (12),

                           command (3, 12, adk::StepDirection::Stopped, 0, 10, true),

                           idleCancel)
                 .ok (),

            "cancel requires a live command");

        policy.reset ();

        require (

            apply (policy, 20, command (1, 20, adk::StepDirection::Reverse, 2)).ok (),

            "reset permits fresh logical command");
        require (policy.stop (adk::TimePoint (21)).ok () &&

                     policy.stop (adk::TimePoint (21)).ok (),

                 "stop is idempotent");
        require (policy.snapshot ().phase == adk::StepSequencePhase::Cancelled,

                 "stop records cancellation");
        requireOff (policy.snapshot (), "stop clears all bits");


        policy.reset ();

        const adk::StepperCommand expiring =
            command (1, 100, adk::StepDirection::Forward, 2);

        require (apply (policy, 100, expiring).ok (), "expiring command admitted");

        adk::StepperSequencePreview expired;
        require (policy.preview (adk::TimePoint (121), expiring, expired).error () ==

                     adk::StatusCode::Timeout,
                 "age expiry reports timeout");
        require (policy.canCommit (expired), "expiry candidate is committable");

        require (policy.commit (expired).ok (), "expiry candidate commits");

        require (policy.snapshot ().phase == adk::StepSequencePhase::Fault,

                 "expiry enters fault");
        requireOff (policy.snapshot (), "expiry fault clears all bits");


        adk::StepperSequencePreview faulted;
        require (!policy

                      .preview (adk::TimePoint (122),

                                command (2, 122, adk::StepDirection::Forward, 1),

                                faulted)
                      .ok (),

                 "fault requires reset before motion");
        policy.reset ();

        require (

            apply (policy, 122, command (1, 122, adk::StepDirection::Reverse, 1)).ok (),

            "reset recovers from fault");
    }

    void testTerminalReplayAndReversalContinuity ()

    {
        const uint8_t frames[] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0c, 0x08, 0x09};

        for (unsigned current = 0; current < 8; ++current)

        {
            adk::BoundedStepperSequence policy (config (1, 100, 1000, -256, 256, true));

            require (policy.initialize ().ok (), "reversal fixture initializes");

            const adk::StepperCommand forward =
                command (1, 0, adk::StepDirection::Forward, 16, 1);

            require (apply (policy, 0, forward).ok (), "forward fixture admitted");

            for (unsigned step = 0; step <= current; ++step)

            {
                require (apply (policy, step + 1, forward).ok (),

                         "forward fixture advances");
            }
            require (policy.snapshot ().coilIntent == frames[current],

                     "fixture reaches selected current frame");

            const adk::StepperCommand reverse =
                command (2, current + 2, adk::StepDirection::Reverse, 1, 1);

            require (apply (policy, current + 2, reverse).ok (),

                     "reverse replacement admitted");
            require (policy.snapshot ().disposition ==
                         adk::StepSequenceDisposition::Replaced,

                     "live reversal is recorded as replacement");
            require (apply (policy, current + 3, reverse).ok (),

                     "reverse replacement advances");
            require (policy.snapshot ().coilIntent == frames[(current + 7) % 8],

                     "reversal continues to legal neighboring frame");
        }

        adk::BoundedStepperSequence complete (config (1, 100, 1000, -256, 256, true));

        require (complete.initialize ().ok (), "complete replay initializes");

        const adk::StepperCommand one =
            command (1, 10, adk::StepDirection::Forward, 1, 1);

        require (apply (complete, 10, one).ok () && apply (complete, 11, one).ok (),

                 "complete terminal fixture created");
        const adk::StepperSequenceSnapshot completed = complete.snapshot ();

        require (apply (complete, 12, one).ok () &&

                     snapshotEqual (completed, complete.snapshot ()),

                 "complete exact replay is idempotent");
        adk::StepperSequencePreview terminalId;
        require (!complete

                      .preview (

                          adk::TimePoint (12),

                          command (0xffffffffUL, 12, adk::StepDirection::Forward, 1, 1),

                          terminalId)
                      .ok (),

                 "complete rejects regressed ID");
        require (!complete

                      .preview (

                          adk::TimePoint (12),

                          command (0x80000001UL, 12, adk::StepDirection::Forward, 1, 1),

                          terminalId)
                      .ok (),

                 "complete rejects ambiguous ID");

        const adk::StepperCommand holding =
            command (2, 12, adk::StepDirection::Stopped, 0, 1);

        require (apply (complete, 12, holding).ok (), "holding fixture admitted");

        const adk::StepperSequenceSnapshot held = complete.snapshot ();

        require (apply (complete, 13, holding).ok () &&

                     snapshotEqual (held, complete.snapshot ()),

                 "holding exact replay is idempotent");

        adk::BoundedStepperSequence cancelled (config (1, 100, 1000));

        require (cancelled.initialize ().ok (), "cancel replay initializes");

        require (apply (cancelled, 0, command (1, 0, adk::StepDirection::Forward, 2, 1))

                     .ok (),

                 "cancel replay motion admitted");
        const adk::StepperCommand cancel =
            command (2, 1, adk::StepDirection::Stopped, 0, 1, true);

        require (apply (cancelled, 1, cancel).ok (), "cancel terminal fixture created");

        const adk::StepperSequenceSnapshot cancelledSnapshot = cancelled.snapshot ();

        require (apply (cancelled, 2, cancel).ok () &&

                     snapshotEqual (cancelledSnapshot, cancelled.snapshot ()),

                 "cancelled exact replay is idempotent");
        require (!cancelled

                      .preview (adk::TimePoint (2),

                                command (1, 2, adk::StepDirection::Forward, 1, 1),

                                terminalId)
                      .ok (),

                 "cancelled rejects regressed ID");
        require (

            !cancelled
                 .preview (adk::TimePoint (2),

                           command (0x80000002UL, 2, adk::StepDirection::Forward, 1, 1),

                           terminalId)
                 .ok (),

            "cancelled rejects ambiguous ID");
        require (apply (cancelled, 2, command (3, 2, adk::StepDirection::Reverse, 1, 1))

                     .ok (),

                 "cancelled accepts forward new ID");

        adk::BoundedStepperSequence faulted (config (1, 100, 5));

        require (faulted.initialize ().ok (), "fault replay initializes");

        const adk::StepperCommand sourceFault =
            command (1, 0, adk::StepDirection::Forward, 1, 1, false,

                     adk::StatusCode::HardwareFailure);
        adk::StepperSequencePreview sourceCandidate;
        require (faulted.preview (adk::TimePoint (0), sourceFault, sourceCandidate)

                             .error () == adk::StatusCode::HardwareFailure &&

                     faulted.commit (sourceCandidate).ok (),

                 "source-fault terminal fixture created");
        const adk::StepperSequenceSnapshot sourceFaultSnapshot = faulted.snapshot ();

        require (sourceFaultSnapshot.direction == adk::StepDirection::Stopped &&

                     sourceFaultSnapshot.coilIntent == 0 &&
                     !sourceFaultSnapshot.hasDeadline &&
                     sourceFaultSnapshot.completedSteps == 0 &&
                     sourceFaultSnapshot.status.error () ==

                         adk::StatusCode::HardwareFailure,
                 "source-fault snapshot is coherent and all off");
        adk::StepperSequencePreview sourceReplay;
        require (

            faulted.preview (adk::TimePoint (1), sourceFault, sourceReplay).error () ==

                    adk::StatusCode::HardwareFailure &&
                faulted.canCommit (sourceReplay) &&

                faulted.commit (sourceReplay).ok () &&

                snapshotEqual (sourceFaultSnapshot, faulted.snapshot ()),

            "source-fault exact replay is idempotent");

        faulted.reset ();

        const adk::StepperCommand expires =
            command (1, 10, adk::StepDirection::Forward, 2, 1);

        require (apply (faulted, 10, expires).ok (), "expiry replay motion admitted");

        adk::StepperSequencePreview expiry;
        require (faulted.preview (adk::TimePoint (16), expires, expiry).error () ==

                         adk::StatusCode::Timeout &&
                     faulted.commit (expiry).ok (),

                 "expiry terminal fixture created");
        const adk::StepperSequenceSnapshot expirySnapshot = faulted.snapshot ();

        require (expirySnapshot.direction == adk::StepDirection::Stopped &&

                     expirySnapshot.coilIntent == 0 && !expirySnapshot.hasDeadline &&
                     expirySnapshot.status.error () == adk::StatusCode::Timeout,

                 "expiry-fault snapshot is coherent and all off");
        adk::StepperSequencePreview expiryReplay;
        require (

            faulted.preview (adk::TimePoint (17), expires, expiryReplay).error () ==

                    adk::StatusCode::Timeout &&
                faulted.commit (expiryReplay).ok () &&

                snapshotEqual (expirySnapshot, faulted.snapshot ()),

            "expiry-fault exact replay is idempotent");

        adk::StepperSequencePreview changed;
        require (!faulted

                      .preview (adk::TimePoint (17),

                                command (1, 10, adk::StepDirection::Forward, 3, 1),

                                changed)
                      .ok (),

                 "terminal same-ID changed payload rejected");
        require (!faulted

                      .preview (adk::TimePoint (17),

                                command (2, 17, adk::StepDirection::Forward, 1, 1),

                                changed)
                      .ok (),

                 "fault rejects forward new ID until reset");
        faulted.reset ();

        require (apply (faulted, 17, command (1, 17, adk::StepDirection::Reverse, 1, 1))

                     .ok (),

                 "reset clears fault and terminal identity");
    }

    void testPreviewBinding ()

    {
        adk::BoundedStepperSequence first (config ());

        adk::BoundedStepperSequence second (config ());

        require (first.initialize ().ok () && second.initialize ().ok (),

                 "binding policies initialize");
        adk::StepperSequencePreview candidate;
        require (first

                     .preview (adk::TimePoint (0),

                               command (1, 0, adk::StepDirection::Forward, 2),

                               candidate)
                     .ok (),

                 "candidate prepared");
        require (first.canCommit (candidate), "owner accepts candidate");

        require (!second.canCommit (candidate), "foreign owner rejects candidate");

        require (second.commit (candidate).error () == adk::StatusCode::InvalidArgument,

                 "foreign commit rejected");

        adk::StepperSequencePreview stale;
        require (first

                     .preview (adk::TimePoint (0),

                               command (1, 0, adk::StepDirection::Forward, 2), stale)

                     .ok (),

                 "stale candidate prepared");
        require (first.commit (candidate).ok (), "first candidate commits");

        const adk::StepperSequenceSnapshot committed = first.snapshot ();

        require (!first.canCommit (candidate) && !first.canCommit (stale),

                 "commit invalidates same-generation candidates");
        require (first.commit (candidate).error () == adk::StatusCode::InvalidArgument,

                 "double commit rejected");
        require (snapshotEqual (committed, first.snapshot ()),

                 "double commit cannot mutate");

        adk::StepperSequencePreview beforeReset;
        require (first

                     .preview (adk::TimePoint (1),

                               command (1, 0, adk::StepDirection::Forward, 2),

                               beforeReset)
                     .ok (),

                 "pre-reset candidate prepared");
        first.reset ();

        require (!first.canCommit (beforeReset),

                 "reset invalidates prepared candidate");
        require (first.commit (beforeReset).error () ==

                     adk::StatusCode::InvalidArgument,
                 "pre-reset candidate cannot commit");
        requireOff (first.snapshot (), "rejected stale commit stays all off");

    }

    void testExactTravelAndDeterministicReplay ()

    {
        adk::BoundedStepperSequence forward (config (1, 100, 1000, -256, 256));

        require (forward.initialize ().ok (), "maximum travel initializes");

        const adk::StepperCommand maximum =
            command (1, 0, adk::StepDirection::Forward, 256, 1);

        require (apply (forward, 0, maximum).ok (), "maximum travel admitted");

        for (uint32_t step = 0; step < 256; ++step)

        {
            require (apply (forward, step + 1, maximum).ok (),

                     "maximum travel step commits");
        }
        require (forward.snapshot ().logicalPosition == 256 &&

                     forward.snapshot ().completedSteps == 256,

                 "exact positive travel bound reached");
        requireOff (forward.snapshot (), "maximum travel completes all off");

        adk::BoundedStepperSequence reverse (
            config (1, 100, 1000, -256, 256));

        require (reverse.initialize ().ok (), "negative maximum travel initializes");

        const adk::StepperCommand minimum =
            command (1, 0, adk::StepDirection::Reverse, 256, 1);

        require (apply (reverse, 0, minimum).ok (), "negative maximum travel admitted");

        for (uint32_t step = 0; step < 256; ++step)

        {
            require (apply (reverse, step + 1, minimum).ok (),

                     "negative maximum travel step commits");
        }

        require (reverse.snapshot ().logicalPosition == -256 &&

                     reverse.snapshot ().completedSteps == 256,

                 "exact negative travel bound reached");

        requireOff (reverse.snapshot (), "negative maximum travel completes all off");


        adk::BoundedStepperSequence left (config ());

        adk::BoundedStepperSequence right (config ());

        require (left.initialize ().ok () && right.initialize ().ok (),

                 "replay policies initialize");
        for (uint32_t frame = 0; frame < 12; ++frame)

        {
            const adk::StepperCommand input =
                frame < 7 ? command (1, 100, adk::StepDirection::Reverse, 6, 3)

                          : command (2, 121, adk::StepDirection::Forward, 3, 3);

            const uint32_t    now         = 100 + frame * 3;
            const adk::Status leftStatus  = apply (left, now, input);

            const adk::Status rightStatus = apply (right, now, input);

            require (leftStatus == rightStatus &&

                         snapshotEqual (left.snapshot (), right.snapshot ()),

                     "identical inputs produce field-identical traces");
        }
    }
} // namespace
int main ()

{
    testLifecycleAndConfiguration ();

    testCommandValidationAndAtomicity ();

    testEveryFrameAndDirection ();

    testDeadlineCatchupAndRollover ();

    testReplayReplacementAndIdentifiers ();

    testCancellationStopExpiryAndRecovery ();

    testTerminalReplayAndReversalContinuity ();

    testPreviewBinding ();

    testExactTravelAndDeterministicReplay ();

}
// clang-format on
