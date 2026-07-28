#include <bounded_homing_policy.h>

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



    adk::BoundedHomingConfig config ()

    {

        return {-8,

                8,

                0,

                -1,

                3,

                5,

                adk::Duration (40),

                adk::Duration (60),

                adk::Duration (10),

                adk::Duration (30),

                adk::Duration (3)};

    }



    adk::CopiedBinaryEvidence evidence (adk::CarouselSourceKind kind, uint32_t at,

                                        uint32_t sequence, bool active,

                                        uint32_t epoch = 7, bool qualified = true,

                                        adk::Status status = adk::StatusCode::Ok)

    {

        return {{kind, 1, 2}, adk::TimePoint (at), sequence, active, qualified, epoch,

                status};

    }



    adk::HomingInput input (uint32_t at, uint32_t sequence, bool home,

                            bool stop = false, uint32_t epoch = 7)

    {

        return {

            adk::TimePoint (at), sequence,

            evidence (adk::CarouselSourceKind::SyntheticHome, at, sequence, home,

                      epoch),

            evidence (adk::CarouselSourceKind::SyntheticStop, at, sequence, stop, 9)};

    }



    adk::HomingCommand command (bool home = false, bool move = false,

                                int32_t target = 0)

    {

        return {home, move, target};

    }



    adk::Status apply (adk::BoundedHomingPolicy& policy, uint32_t now,

                       const adk::HomingInput& frame, const adk::HomingCommand& request)

    {

        adk::HomingPreview candidate;

        const adk::Status  status =

            policy.preview (adk::TimePoint (now), frame, request, candidate);

        if (!status.ok ())

        {

            return status;

        }

        require (policy.canCommit (candidate), "accepted preview committable");

        return policy.commit (candidate);

    }



    bool equal (const adk::HomingSnapshot& a, const adk::HomingSnapshot& b)

    {

        return a.phase == b.phase && a.fault == b.fault &&

               a.logicalPosition == b.logicalPosition &&

               a.positionKnown == b.positionKnown &&

               a.stepDirection == b.stepDirection &&

               a.stepRequested == b.stepRequested &&

               a.requestedStepDirection == b.requestedStepDirection &&

               a.stopIntent == b.stopIntent && a.homingSteps == b.homingSteps &&

               a.homeEpoch == b.homeEpoch &&

               a.acceptedFrameSequence == b.acceptedFrameSequence &&

               a.status == b.status;

    }



    void homeInactive (adk::BoundedHomingPolicy& policy, uint32_t base = 0,

                       uint32_t sequence = 1)

    {

        require (

            apply (policy, base, input (base, sequence, false), command (true)).ok (),

            "inactive home request accepted");

        require (policy.snapshot ().phase == adk::HomingPhase::SeekingHome,

                 "inactive request seeks home");

        require (apply (policy, base + 10, input (base + 10, sequence + 1, false),

                        command ())

                     .ok (),

                 "search step accepted");

        require (policy.snapshot ().stepRequested &&

                     policy.snapshot ().requestedStepDirection == -1,

                 "search emits configured signed step");

        require (

            apply (policy, base + 20, input (base + 20, sequence + 2, true), command ())

                .ok (),

            "qualified acquisition accepted");

        require (policy.snapshot ().phase == adk::HomingPhase::Homed &&

                     policy.snapshot ().positionKnown &&

                     policy.snapshot ().logicalPosition == 0 &&

                     policy.snapshot ().homeEpoch != 0 &&

                     !policy.snapshot ().stepRequested,

                 "edge establishes session logical zero");

    }



    void testLifecycleAndConfiguration ()

    {

        static_assert (!std::is_copy_constructible<adk::BoundedHomingPolicy>::value,

                       "homing policy cannot copy");

        static_assert (!std::is_move_constructible<adk::BoundedHomingPolicy>::value,

                       "homing policy cannot move");

        static_assert (std::is_trivially_destructible<adk::BoundedHomingPolicy>::value,

                       "destruction has no endpoint effect");



        adk::BoundedHomingPolicy policy (config ());

        adk::HomingPreview       candidate;

        require (!policy.initialized (), "construction is inert");

        require (!policy

                      .preview (adk::TimePoint (0), input (0, 1, false), command (true),

                                candidate)

                      .ok (),

                 "preview before initialize rejected");

        require (policy.initialize ().ok () && policy.initialize ().ok (),

                 "initialize is idempotent");

        require (policy.snapshot ().phase == adk::HomingPhase::PositionUnknown &&

                     policy.snapshot ().stopIntent,

                 "initialize begins unknown and stopped");



        const adk::HomingExcursionBounds bounds = policy.excursionBounds ();

        require (bounds.minimum == -5 && bounds.maximum == 3,

                 "excursion bounds include release and search");



        adk::BoundedHomingConfig bad[] = {

            {0, 8, 0, -1, 3, 5, adk::Duration (40), adk::Duration (60),

             adk::Duration (10), adk::Duration (30), adk::Duration (3)},

            {-8, 8, 1, -1, 3, 5, adk::Duration (40), adk::Duration (60),

             adk::Duration (10), adk::Duration (30), adk::Duration (3)},

            {-8, 8, 0, 0, 3, 5, adk::Duration (40), adk::Duration (60),

             adk::Duration (10), adk::Duration (30), adk::Duration (3)},

            {-8, 8, 0, -1, 0, 5, adk::Duration (40), adk::Duration (60),

             adk::Duration (10), adk::Duration (30), adk::Duration (3)},

            {-8, 8, 0, -1, 3, 0, adk::Duration (40), adk::Duration (60),

             adk::Duration (10), adk::Duration (30), adk::Duration (3)}};

        for (const auto& invalid : bad)

        {

            adk::BoundedHomingPolicy rejected (invalid);

            require (!rejected.initialize ().ok () && !rejected.initialized (),

                     "invalid configuration rejected");

            require (rejected.snapshot ().fault ==

                         adk::HomingFault::InvalidConfiguration,

                     "configuration fault attributed");

        }



        homeInactive (policy);

        policy.reset ();

        require (policy.initialized () &&

                     policy.snapshot ().phase == adk::HomingPhase::PositionUnknown &&

                     !policy.snapshot ().positionKnown &&

                     policy.snapshot ().homeEpoch == 0,

                 "reset invalidates logical position");

        policy.shutdown ();

        require (!policy.initialized () &&

                     policy.snapshot ().phase == adk::HomingPhase::Uninitialized &&

                     policy.snapshot ().stopIntent,

                 "shutdown is inert and invalidates session");

    }



    void testReleaseSearchAndBounds ()

    {

        adk::BoundedHomingPolicy policy (config ());

        require (policy.initialize ().ok (), "release fixture initializes");

        require (apply (policy, 0, input (0, 1, true), command (true)).ok (),

                 "active request accepted");

        require (policy.snapshot ().phase == adk::HomingPhase::SeekingHomeRelease,

                 "active start seeks release");

        require (apply (policy, 10, input (10, 2, true), command ()).ok () &&

                     policy.snapshot ().requestedStepDirection == 1,

                 "release moves opposite search");

        require (apply (policy, 20, input (20, 3, false), command ()).ok () &&

                     policy.snapshot ().phase == adk::HomingPhase::SeekingHome,

                 "qualified release starts approach");

        require (apply (policy, 30, input (30, 4, true), command ()).ok () &&

                     policy.snapshot ().phase == adk::HomingPhase::Homed,

                 "new acquisition edge homes");



        for (int target : {-8, 8})

        {

            adk::BoundedHomingPolicy travel (config ());

            require (travel.initialize ().ok (), "travel fixture initializes");

            homeInactive (travel);

            require (

                apply (travel, 40, input (40, 4, true), command (false, true, target))

                    .ok (),

                "inclusive target accepted");

            uint32_t now = 50;

            uint32_t seq = 5;

            while (travel.snapshot ().logicalPosition != target)

            {

                require (apply (travel, now, input (now, seq, true), command ()).ok (),

                         "one bounded travel step accepted");

                now += 10;

                ++seq;

            }

            require (travel.snapshot ().phase == adk::HomingPhase::Homed,

                     "target completion returns homed");

        }



        const adk::HomingSnapshot before = policy.snapshot ();

        require (

            !apply (policy, 40, input (40, 5, true), command (false, true, 9)).ok (),

            "out-of-range target rejected");

        require (equal (before, policy.snapshot ()),

                 "rejected target does not mutate state");

    }



    void testEvidenceStopAndAtomicity ()

    {

        adk::BoundedHomingPolicy policy (config ());

        require (policy.initialize ().ok (), "evidence fixture initializes");

        homeInactive (policy);



        adk::HomingInput malformed = input (40, 4, true, true);

        malformed.home.source.kind = adk::CarouselSourceKind::SyntheticKey;

        malformed.frameAt          = adk::TimePoint (999);

        require (apply (policy, 40, malformed, command (false, true, 4)).ok (),

                 "independently valid stop dominates malformed home/frame");

        require (policy.snapshot ().phase == adk::HomingPhase::Stopped &&

                     policy.snapshot ().stopIntent &&

                     policy.snapshot ().positionKnown &&

                     policy.snapshot ().fault == adk::HomingFault::None,

                 "idle stop dominates command and preserves known position");



        policy.reset ();

        adk::HomingInput badStop         = input (50, 5, false, true);

        badStop.stop.status              = adk::StatusCode::HardwareFailure;

        const adk::HomingSnapshot before = policy.snapshot ();

        require (!apply (policy, 50, badStop, command (true)).ok (),

                 "malformed stop rejects whole frame");

        require (equal (before, policy.snapshot ()), "malformed stop rejection atomic");



        adk::HomingPreview first;

        require (policy

                     .preview (adk::TimePoint (60), input (60, 6, false),

                               command (true), first)

                     .ok (),

                 "candidate prepared");

        adk::BoundedHomingPolicy foreign (config ());

        require (foreign.initialize ().ok () && !foreign.canCommit (first) &&

                     !foreign.commit (first).ok (),

                 "foreign candidate rejected");

        require (policy.commit (first).ok () && !policy.canCommit (first) &&

                     !policy.commit (first).ok (),

                 "candidate consumed exactly once");



        adk::HomingPreview stale;

        require (

            policy

                .preview (adk::TimePoint (70), input (70, 7, false), command (), stale)

                .ok (),

            "stale candidate prepared");

        policy.reset ();

        require (!policy.canCommit (stale) && !policy.commit (stale).ok (),

                 "reset invalidates outstanding preview");

    }



    void testTimingReplayAndProvenance ()

    {

        adk::BoundedHomingPolicy policy (config ());

        require (policy.initialize ().ok (), "timing fixture initializes");

        require (apply (policy, std::numeric_limits<uint32_t>::max () - 5,

                        input (std::numeric_limits<uint32_t>::max () - 5, 1, false),

                        command (true))

                     .ok (),

                 "rollover fixture starts");

        require (apply (policy, 4, input (4, 2, false), command ()).ok () &&

                     policy.snapshot ().stepRequested,

                 "step interval is rollover safe");



        adk::HomingInput          changed = input (14, 2, true);

        const adk::HomingSnapshot before  = policy.snapshot ();

        require (!apply (policy, 14, changed, command ()).ok (),

                 "changed same-sequence payload rejected");

        require (equal (before, policy.snapshot ()),

                 "replay rejection does not mutate");



        adk::HomingInput future = input (30, 3, false);

        future.home.observedAt  = adk::TimePoint (31);

        require (!apply (policy, 30, future, command ()).ok (),

                 "future evidence rejected before skew admission");



        policy.reset ();

        require (apply (policy, 40, input (40, 4, true), command (true)).ok (),

                 "new active attempt starts");

        adk::HomingInput          changedEpoch = input (50, 5, false, false, 8);

        const adk::HomingSnapshot attempt      = policy.snapshot ();

        require (!apply (policy, 50, changedEpoch, command ()).ok (),

                 "qualification epoch change rejected");

        require (equal (attempt, policy.snapshot ()),

                 "qualification epoch rejection is atomic");

    }

    void testBoundaryRegressions ()

    {

        adk::BoundedHomingPolicy lateRelease (config ());

        require (lateRelease.initialize ().ok (), "late release initializes");

        require (apply (lateRelease, 0, input (0, 1, true), command (true)).ok (),

                 "late release attempt starts");

        require (apply (lateRelease, 40, input (40, 2, false), command ()).ok (),

                 "release at exact duration accepted");



        adk::BoundedHomingPolicy timeout (config ());

        require (timeout.initialize ().ok (), "timeout fixture initializes");

        require (apply (timeout, 0, input (0, 1, true), command (true)).ok (),

                 "timeout attempt starts");

        adk::HomingPreview fault;

        require (timeout.preview (adk::TimePoint (41), input (41, 2, false),

                                  command (), fault)

                     .error () == adk::StatusCode::Timeout,

                 "release one tick late times out");

        require (timeout.canCommit (fault), "timeout transition is committable");

        require (timeout.commit (fault).error () == adk::StatusCode::Timeout &&

                     timeout.snapshot ().phase == adk::HomingPhase::Fault,

                 "timeout latches fault");

        require (apply (timeout, 42, input (42, 3, false, true), command ()).ok (),

                 "valid stop remains admissible while faulted");

        require (timeout.snapshot ().phase == adk::HomingPhase::Fault &&

                     timeout.snapshot ().fault ==

                         adk::HomingFault::HomeStuckActive &&

                     timeout.snapshot ().stopIntent,

                 "stop preserves latched fault and cannot recover");



        adk::BoundedHomingPolicy rehome (config ());

        require (rehome.initialize ().ok (), "rehome fixture initializes");

        homeInactive (rehome);

        require (apply (rehome, 40, input (40, 4, true),

                        command (false, true, 4))

                     .ok (),

                 "nonzero target accepted");

        for (uint32_t sequence = 5; sequence < 9; ++sequence)

        {

            const uint32_t now = sequence * 10;

            require (apply (rehome, now, input (now, sequence, true), command ())

                         .ok (),

                     "nonzero excursion advances");

        }

        require (rehome.snapshot ().logicalPosition == 4,

                 "fixture reaches nonzero coordinate");

        const uint32_t priorEpoch = rehome.snapshot ().homeEpoch;

        require (apply (rehome, 90, input (90, 9, false), command (true)).ok () &&

                     rehome.snapshot ().logicalPosition == -1 &&

                     rehome.snapshot ().homingSteps == 1,

                 "new homing attempt steps from reset synthetic origin");

        require (apply (rehome, 100, input (100, 10, false), command ()).ok (),

                 "second session search advances");

        require (apply (rehome, 110, input (110, 11, true), command ()).ok () &&

                     rehome.snapshot ().phase == adk::HomingPhase::Homed &&

                     rehome.snapshot ().homeEpoch == priorEpoch + 1,

                 "consecutive successful home increments session epoch");



        adk::BoundedHomingPolicy lateHome (config ());

        require (lateHome.initialize ().ok (), "late home fixture initializes");

        require (apply (lateHome, 0, input (0, 1, false), command (true)).ok (),

                 "late home search starts");

        adk::HomingPreview searchFault;

        require (lateHome.preview (adk::TimePoint (61), input (61, 2, true),

                                   command (), searchFault)

                     .error () == adk::StatusCode::Timeout,

                 "home edge one tick beyond search duration times out");

        require (lateHome.canCommit (searchFault) &&

                     lateHome.commit (searchFault).error () ==

                         adk::StatusCode::Timeout &&

                     lateHome.snapshot ().fault == adk::HomingFault::HomeNotFound,

                 "late home edge latches not-found fault");



        adk::BoundedHomingPolicy replay (config ());

        require (replay.initialize ().ok (), "replay fixture initializes");

        const adk::HomingInput firstFrame = input (10, 1, false);

        require (apply (replay, 10, firstFrame, command (true)).ok (),

                 "first frame accepted");

        const adk::HomingSnapshot accepted = replay.snapshot ();

        require (!apply (replay, 10, firstFrame, command (true)).ok (),

                 "identical same-frame replay conservatively rejected");

        require (equal (accepted, replay.snapshot ()),

                 "identical replay is idempotent and creates no second event");

        adk::HomingInput changedTime = firstFrame;

        changedTime.frameAt          = adk::TimePoint (11);

        require (!apply (replay, 11, changedTime, command (true)).ok () &&

                     equal (accepted, replay.snapshot ()),

                 "same sequence with changed frame time rejects atomically");

        require (!apply (replay, 10, firstFrame, command (false, true, 1)).ok () &&

                     equal (accepted, replay.snapshot ()),

                 "same frame with changed command rejects atomically");

        require (!apply (replay, 12, input (12, 0xffffffffU, false), command ())

                      .ok () &&

                     equal (accepted, replay.snapshot ()),

                 "backward frame sequence rejects atomically");

        require (!apply (replay, 12, input (12, 0x80000001U, false), command ())

                      .ok () &&

                     equal (accepted, replay.snapshot ()),

                 "half-range frame sequence rejects atomically");

    }

} // namespace

// clang-format on

int main ()

{
    testLifecycleAndConfiguration ();

    testReleaseSearchAndBounds ();

    testEvidenceStopAndAtomicity ();

    testTimingReplayAndProvenance ();

    testBoundaryRegressions ();

    std::cout << "bounded homing policy tests passed\n";
}
