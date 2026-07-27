#include "traffic_junction.h"

#include <cstdlib>
#include <iostream>

namespace {

    using namespace adk;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (1);

        }
    }

    TrafficConfig testConfig ()
    {
        TrafficConfig config;

        config.startupAllRed       = Duration (10);

        config.vehicleAllRed       = Duration (2);

        config.mainGreen           = Duration (20);

        config.mainYellow          = Duration (3);

        config.sideGreen           = Duration (15);

        config.sideYellow          = Duration (3);

        config.pedestrianWalk      = Duration (8);

        config.pedestrianClearance = Duration (4);


        return config;
    }

    void requireSafeSignals (const TrafficSnapshot& snapshot, const char* message)
    {
        const TrafficSignals signals    = snapshot.signals;
        const unsigned       mainActive = static_cast<unsigned> (signals.mainRed) +
                                          static_cast<unsigned> (signals.mainYellow) +
                                          static_cast<unsigned> (signals.mainGreen);

        const unsigned       sideActive = static_cast<unsigned> (signals.sideRed) +
                                          static_cast<unsigned> (signals.sideYellow) +
                                          static_cast<unsigned> (signals.sideGreen);


        require (mainActive == 1, message);

        require (sideActive == 1, message);

        require (signals.pedestrianStop != signals.pedestrianWalk, message);

        require (!(signals.mainGreen && signals.sideGreen), message);

        require (!(signals.mainGreen && signals.pedestrianWalk), message);

        require (!(signals.sideGreen && signals.pedestrianWalk), message);

    }

    void requirePhase (const TrafficJunction& junction, TrafficPhase phase,
                       const char* message)
    {
        const TrafficSnapshot snapshot = junction.snapshot ();


        require (snapshot.phase == phase, message);

        requireSafeSignals (snapshot, message);

    }

    void testLifecycleAndConfiguration ()
    {
        TrafficJunction junction (testConfig ());


        require (!junction.initialized (), "constructed junction is inert");

        require (junction.update (TimePoint (0), TrafficInput ()) ==
                     Status::NotInitialized,
                 "update before initialize");

        require (junction.initialize () == Status::Ok, "initialize");

        require (junction.initialized (), "initialized state");

        require (junction.update (TimePoint (0), TrafficInput (true)) == Status::Ok,
                 "state before repeated initialize");

        const TrafficSnapshot beforeRepeatInitialize = junction.snapshot ();


        require (junction.initialize () == Status::Ok,
                 "repeat initialize succeeds");

        const TrafficSnapshot afterRepeatInitialize = junction.snapshot ();


        require (afterRepeatInitialize.phase == beforeRepeatInitialize.phase,
                 "repeat initialize preserves phase");

        require (afterRepeatInitialize.pedestrianRequestPending,
                 "repeat initialize preserves pending request");

        require (afterRepeatInitialize.hasDeadline ==
                     beforeRepeatInitialize.hasDeadline,
                 "repeat initialize preserves timing state");

        require (junction.reset () == Status::Ok, "explicit reset");


        const TrafficSnapshot snapshot = junction.snapshot ();


        require (snapshot.phase == TrafficPhase::AllRed, "initial all red");

        require (snapshot.status == Status::Ok, "initial status");

        require (!snapshot.hasDeadline, "deadline waits for first update");

        require (!snapshot.phaseChanged, "initialize has no transition event");

        require (snapshot.transitionCount == 0, "initialize clears transitions");

        requireSafeSignals (snapshot, "initial signals safe");

        junction.shutdown ();

        require (!junction.initialized (), "shutdown clears initialized state");

        require (junction.snapshot ().status == Status::NotInitialized,
                 "shutdown status");

        requirePhase (junction, TrafficPhase::AllRed, "shutdown forces all red");

        require (junction.update (TimePoint (1), TrafficInput ()) ==
                     Status::NotInitialized,
                 "shutdown blocks updates");

        junction.shutdown ();

        require (!junction.initialized (), "repeat shutdown is inert");

        require (junction.reset () == Status::NotInitialized,
                 "reset requires initialized lifecycle");

        require (junction.initialize () == Status::Ok,
                 "initialize after shutdown");

        require (junction.initialized (), "reinitialized state");


        TrafficConfig invalid = testConfig ();


        invalid.mainGreen = Duration (0);

        TrafficJunction zeroDuration (invalid);

        require (zeroDuration.initialize () == Status::InvalidArgument,
                 "zero duration rejected");

        requirePhase (zeroDuration, TrafficPhase::Fault, "invalid config faults");


        invalid           = testConfig ();

        invalid.sideGreen = Duration (0x80000000u);

        TrafficJunction ambiguousDuration (invalid);

        require (ambiguousDuration.initialize () == Status::InvalidArgument,
                 "ambiguous duration rejected");

    }

    void testNominalCycleAndExactDeadlines ()
    {
        TrafficJunction junction (testConfig ());


        require (junction.initialize () == Status::Ok, "cycle initialize");

        require (junction.update (TimePoint (100), TrafficInput ()) == Status::Ok,
                 "anchor startup");


        TrafficSnapshot snapshot = junction.snapshot ();


        require (snapshot.phaseSince == TimePoint (100), "startup anchor");

        require (snapshot.nextDeadline == TimePoint (110), "startup deadline");

        require (snapshot.hasDeadline, "anchored deadline");


        require (junction.update (TimePoint (109), TrafficInput ()) == Status::Ok,
                 "before startup deadline");

        requirePhase (junction, TrafficPhase::AllRed, "no early startup transition");


        require (junction.update (TimePoint (110), TrafficInput ()) == Status::Ok,
                 "exact startup deadline");

        requirePhase (junction, TrafficPhase::MainGreen, "main green");

        require (junction.snapshot ().phaseChanged, "deadline transition event");

        require (junction.snapshot ().transitionCount == 1, "first transition count");


        require (junction.update (TimePoint (130), TrafficInput ()) == Status::Ok,
                 "main green deadline");

        requirePhase (junction, TrafficPhase::MainYellow, "main yellow");


        require (junction.update (TimePoint (133), TrafficInput ()) == Status::Ok,
                 "main yellow deadline");

        requirePhase (junction, TrafficPhase::AllRed, "interlocked all red");


        snapshot = junction.snapshot ();

        require (snapshot.signals.mainRed && snapshot.signals.sideRed,
                 "vehicle all-red assertion");

        require (snapshot.nextDeadline == TimePoint (135),
                 "vehicle clearance deadline");


        require (junction.update (TimePoint (135), TrafficInput ()) == Status::Ok,
                 "side clearance deadline");

        requirePhase (junction, TrafficPhase::SideGreen, "side green");


        require (junction.update (TimePoint (150), TrafficInput ()) == Status::Ok,
                 "side green deadline");

        requirePhase (junction, TrafficPhase::SideYellow, "side yellow");


        require (junction.update (TimePoint (153), TrafficInput ()) == Status::Ok,
                 "side yellow deadline");

        requirePhase (junction, TrafficPhase::AllRed, "return all red");


        require (junction.update (TimePoint (155), TrafficInput ()) == Status::Ok,
                 "main clearance deadline");

        requirePhase (junction, TrafficPhase::MainGreen, "cycle returns main");

        require (junction.snapshot ().transitionCount == 7,
                 "complete transition count");

    }

    void testRequestsAndPedestrianSequence ()
    {
        TrafficJunction junction (testConfig ());


        require (junction.initialize () == Status::Ok, "request initialize");

        require (junction.update (TimePoint (0), TrafficInput (true)) == Status::Ok,
                 "startup request");

        require (junction.snapshot ().requestAccepted, "request accepted event");

        require (junction.snapshot ().pedestrianRequestPending, "request latched");


        require (junction.update (TimePoint (1), TrafficInput (true)) == Status::Ok,
                 "duplicate request");

        require (!junction.snapshot ().requestAccepted, "duplicate not reaccepted");


        require (junction.update (TimePoint (10), TrafficInput ()) == Status::Ok,
                 "serve startup request");

        requirePhase (junction, TrafficPhase::PedestrianWalk, "pedestrian walk");

        require (!junction.snapshot ().pedestrianRequestPending, "request consumed");


        require (junction.update (TimePoint (18), TrafficInput (true)) == Status::Ok,
                 "request during walk");

        requirePhase (junction, TrafficPhase::PedestrianClearance,
                      "pedestrian clearance");

        require (junction.snapshot ().requestAccepted, "later request accepted");

        require (junction.snapshot ().pedestrianRequestPending,
                 "later request retained");


        require (junction.update (TimePoint (22), TrafficInput ()) == Status::Ok,
                 "clearance complete");

        requirePhase (junction, TrafficPhase::AllRed, "all red after crossing");


        require (junction.update (TimePoint (24), TrafficInput ()) == Status::Ok,
                 "repeat request service");

        requirePhase (junction, TrafficPhase::PedestrianWalk, "pending request served");

    }

    void testLateUpdatesAdvanceOnce ()
    {
        TrafficJunction junction (testConfig ());


        require (junction.initialize () == Status::Ok, "late initialize");

        require (junction.update (TimePoint (0), TrafficInput ()) == Status::Ok,
                 "late anchor");

        require (junction.update (TimePoint (1000), TrafficInput ()) == Status::Ok,
                 "late update");

        requirePhase (junction, TrafficPhase::MainGreen, "late update advances once");

        require (junction.snapshot ().phaseSince == TimePoint (1000),
                 "late transition starts at observation");

        require (junction.snapshot ().transitionCount == 1,
                 "late update has one transition");

    }

    void testVehicleRequestWaitsForSafeService ()
    {
        TrafficJunction junction (testConfig ());

        require (junction.initialize () == Status::Ok, "vehicle request initialize");

        require (junction.update (TimePoint (0), TrafficInput ()) == Status::Ok,
                 "vehicle request anchor");

        require (junction.update (TimePoint (10), TrafficInput ()) == Status::Ok,
                 "enter main green");

        require (junction.update (TimePoint (11), TrafficInput (true)) == Status::Ok,
                 "request during main green");

        require (junction.snapshot ().requestAccepted, "vehicle request accepted");

        require (junction.update (TimePoint (30), TrafficInput ()) == Status::Ok,
                 "finish main green");

        require (junction.update (TimePoint (33), TrafficInput ()) == Status::Ok,
                 "finish main yellow");

        require (junction.update (TimePoint (35), TrafficInput ()) == Status::Ok,
                 "clear before side");

        requirePhase (junction, TrafficPhase::SideGreen, "request does not skip side");

        require (junction.snapshot ().pedestrianRequestPending,
                 "request remains pending through side");

        require (junction.update (TimePoint (50), TrafficInput ()) == Status::Ok,
                 "finish side green");

        require (junction.update (TimePoint (53), TrafficInput ()) == Status::Ok,
                 "finish side yellow");

        require (junction.update (TimePoint (55), TrafficInput ()) == Status::Ok,
                 "clear before pedestrian");

        requirePhase (junction, TrafficPhase::PedestrianWalk, "request served safely");

        require (!junction.snapshot ().pedestrianRequestPending,
                 "served vehicle request consumed");
    }

    void testRollover ()
    {
        TrafficJunction junction (testConfig ());


        require (junction.initialize () == Status::Ok, "rollover initialize");

        require (junction.update (TimePoint (0xfffffff8u), TrafficInput ()) ==
                     Status::Ok,
                 "rollover anchor");

        require (junction.snapshot ().nextDeadline == TimePoint (2),
                 "rollover deadline");

        require (junction.update (TimePoint (1), TrafficInput ()) == Status::Ok,
                 "before wrapped deadline");

        requirePhase (junction, TrafficPhase::AllRed, "wrapped deadline not early");

        require (junction.update (TimePoint (2), TrafficInput ()) == Status::Ok,
                 "exact wrapped deadline");

        requirePhase (junction, TrafficPhase::MainGreen, "wrapped transition");

    }

    void testFaultLatchAndReset ()
    {
        TrafficJunction junction (testConfig ());


        require (junction.initialize () == Status::Ok, "fault initialize");

        require (junction.update (TimePoint (100), TrafficInput ()) == Status::Ok,
                 "fault anchor");

        require (junction.update (TimePoint (101), TrafficInput (false, false)) ==
                     Status::HardwareFailure,
                 "hardware fault");

        requirePhase (junction, TrafficPhase::Fault, "hardware fault phase");


        TrafficSnapshot snapshot = junction.snapshot ();


        require (snapshot.signals.mainRed && snapshot.signals.sideRed,
                 "fault forces all red");

        require (snapshot.signals.pedestrianStop && !snapshot.signals.pedestrianWalk,
                 "fault stops pedestrians");

        require (!snapshot.hasDeadline, "fault has no deadline");


        require (junction.update (TimePoint (102), TrafficInput ()) ==
                     Status::HardwareFailure,
                 "healthy input cannot clear latch");

        requirePhase (junction, TrafficPhase::Fault, "fault remains latched");


        require (junction.initialize () == Status::Ok,
                 "repeat initialize succeeds while faulted");

        requirePhase (junction, TrafficPhase::Fault,
                      "repeat initialize preserves fault latch");

        require (junction.reset () == Status::Ok, "reset clears fault");

        requirePhase (junction, TrafficPhase::AllRed, "reset enters all red");

        require (junction.snapshot ().transitionCount == 0, "reset clears count");


        require (junction.update (TimePoint (200), TrafficInput ()) == Status::Ok,
                 "time fault anchor");

        require (junction.update (TimePoint (199), TrafficInput ()) ==
                     Status::InvalidArgument,
                 "backward time faults");

        requirePhase (junction, TrafficPhase::Fault, "time fault phase");

    }

    void testDeterministicReplay ()
    {
        const TimePoint    times[]  = {TimePoint (7),  TimePoint (16), TimePoint (17),
                                       TimePoint (37), TimePoint (40), TimePoint (42),
                                       TimePoint (57), TimePoint (60), TimePoint (62)};
        const TrafficInput inputs[] = {
            TrafficInput (), TrafficInput (true), TrafficInput (),
            TrafficInput (), TrafficInput (),     TrafficInput (),
            TrafficInput (), TrafficInput (),     TrafficInput ()};
        TrafficJunction first (testConfig ());

        TrafficJunction second (testConfig ());


        require (first.initialize () == Status::Ok, "first replay initialize");

        require (second.initialize () == Status::Ok, "second replay initialize");


        for (unsigned index = 0; index < sizeof (times) / sizeof (times[0]); ++index)
        {
            require (first.update (times[index], inputs[index]) ==
                         second.update (times[index], inputs[index]),
                     "replay status");


            const TrafficSnapshot left  = first.snapshot ();

            const TrafficSnapshot right = second.snapshot ();


            require (left.phase == right.phase, "replay phase");

            require (left.status == right.status, "replay snapshot status");

            require (left.phaseSince == right.phaseSince, "replay phase time");

            require (left.nextDeadline == right.nextDeadline, "replay deadline");

            require (left.pedestrianRequestPending == right.pedestrianRequestPending,
                     "replay pending request");

            require (left.phaseChanged == right.phaseChanged, "replay change event");

            require (left.requestAccepted == right.requestAccepted,
                     "replay request event");

            require (left.hasDeadline == right.hasDeadline, "replay deadline state");

            require (left.transitionCount == right.transitionCount,
                     "replay transition count");

            requireSafeSignals (left, "replay signals remain safe");

        }
    }
} // namespace

int main ()
{
    testLifecycleAndConfiguration ();

    testNominalCycleAndExactDeadlines ();

    testRequestsAndPedestrianSequence ();

    testLateUpdatesAdvanceOnce ();

    testVehicleRequestWaitsForSafeService ();

    testRollover ();

    testFaultLatchAndReset ();

    testDeterministicReplay ();


    std::cout << "traffic junction tests passed\n";
    return 0;
}
