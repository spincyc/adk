#include <analog_joystick.h>

#include <Arduino.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <vector>

namespace {
    namespace fake = adk::test::arduino;

    constexpr adk::PinId xPin      = 54;
    constexpr adk::PinId yPin      = 55;
    constexpr adk::PinId selectPin = 22;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::AnalogJoystickConfig config (bool xInverted = false, bool yInverted = false)
    {
        return {{xPin, 512, 0, 1023, 100, xInverted},
                {yPin, 400, 100, 900, 50, yInverted},
                {selectPin, adk::Pull::Up, adk::Level::Low, adk::Duration (20)}};
    }

    void setInputs (int x, int y, uint8_t selected)
    {
        fake::setAnalogInput  (xPin, x);
        fake::setAnalogInput  (yPin, y);
        fake::setDigitalInput (selectPin, selected);
    }

    void requireOperation (std::size_t index, fake::OperationKind kind, uint8_t pin)
    {
        const auto& trace = fake::trace ();

        require (index < trace.size (), "missing hardware operation");
        require (trace[index].kind == kind, "wrong hardware operation");
        require (trace[index].pin == pin, "wrong hardware pin");
    }

    bool sameSnapshot (const adk::AnalogJoystickSnapshot& left,
                       const adk::AnalogJoystickSnapshot& right)
    {
        return left.x.raw == right.x.raw && left.x.position == right.x.position &&
               left.x.centered == right.x.centered &&
               left.x.saturated == right.x.saturated && left.y.raw == right.y.raw &&
               left.y.position == right.y.position &&
               left.y.centered == right.y.centered &&
               left.y.saturated == right.y.saturated &&
               left.rawSelected == right.rawSelected &&
               left.selected == right.selected &&
               left.selectEvent == right.selectEvent &&
               left.releaseEvent == right.releaseEvent && left.status == right.status;
    }

    bool sameTrace (const std::vector<fake::Operation>& left,
                    const std::vector<fake::Operation>& right)
    {
        if (left.size () != right.size ())
        {
            return false;
        }

        for (std::size_t index = 0; index < left.size (); ++index)
        {
            if (left[index].kind != right[index].kind ||
                left[index].pin != right[index].pin ||
                left[index].value != right[index].value ||
                left[index].timeUs != right[index].timeUs)
            {
                return false;
            }
        }

        return true;
    }

    void testLifecycleAndSampleOrder ()
    {
        fake::reset ();
        setInputs   (512, 400, HIGH);

        adk::ResourceRegistry resources;
        adk::AnalogJoystick   joystick (resources, config ());

        require (!joystick.initialized (), "joystick starts inactive");
        require (joystick.snapshot ().status.error () ==
                     adk::StatusCode::NotInitialized,
                 "initial snapshot reports inactive");
        require (joystick.initialize ().ok (), "joystick initializes");
        require (joystick.initialized (), "joystick reports initialized");
        require (joystick.xInput ().initialized (), "x input initialized");
        require (joystick.yInput ().initialized (), "y input initialized");
        require (joystick.selectButton ().initialized (), "select button initialized");

        fake::clearTrace ();
        require          (joystick.initialize ().ok (), "repeated initialize succeeds");
        require          (fake::trace ().empty (), "repeated initialize is inert");

        setInputs        (123, 789, LOW);
        require          (joystick.update (adk::TimePoint (10)).ok (), "update succeeds");
        require          (fake::trace ().size () == 3, "one read per input");
        requireOperation (0, fake::OperationKind::AnalogRead, xPin);
        requireOperation (1, fake::OperationKind::AnalogRead, yPin);
        requireOperation (2, fake::OperationKind::DigitalRead, selectPin);

        const auto sampled = joystick.snapshot ();
        require                                (sampled.x.raw == 123, "raw x retained");
        require                                (sampled.y.raw == 789, "raw y retained");
        require                                (sampled.rawSelected, "raw selection retained");

        fake::clearTrace  ();
        joystick.shutdown ();
        require           (!joystick.initialized (), "shutdown stops joystick");
        require           (joystick.snapshot ().status.error () ==
                     adk::StatusCode::NotInitialized,
                 "shutdown reports inactive");
        require          (fake::trace ().size () == 3, "shutdown releases three pins");
        requireOperation (0, fake::OperationKind::PinMode, selectPin);
        requireOperation (1, fake::OperationKind::PinMode, yPin);
        requireOperation (2, fake::OperationKind::PinMode, xPin);

        fake::clearTrace  ();
        joystick.shutdown ();
        require           (fake::trace ().empty (), "repeated shutdown is inert");
        require           (joystick.update (adk::TimePoint (11)).error () ==
                     adk::StatusCode::NotInitialized,
                 "inactive update rejected");
    }

    void testAxisMappingBoundaries ()
    {
        fake::reset ();
        setInputs   (512, 400, HIGH);

        adk::ResourceRegistry resources;
        adk::AnalogJoystick   joystick (resources, config ());

        require (joystick.initialize ().ok (), "mapping joystick initializes");

        struct AxisCase
        {
            int     raw;
            int16_t position;
            bool    centered;
            bool    saturated;
        };

        const AxisCase cases[] = {{-20, -1000, false, true}, {0, -1000, false, true},
                                  {411, -2, false, false},   {412, 0, true, false},
                                  {512, 0, true, false},     {612, 0, true, false},
                                  {613, 2, false, false},    {1023, 1000, false, true},
                                  {1200, 1000, false, true}};

        uint32_t now = 1;
        for (const AxisCase& axisCase : cases)
        {
            setInputs (axisCase.raw, 400, HIGH);
            require   (joystick.update (adk::TimePoint (now++)).ok (),
                     "axis boundary update");

            const auto x = joystick.snapshot ().x;
            require                          (x.position == axisCase.position, "axis mapped position");
            require                          (x.centered == axisCase.centered, "axis centered flag");
            require                          (x.saturated == axisCase.saturated, "axis saturation flag");
        }
    }

    void testAsymmetryAndInversion ()
    {
        fake::reset ();
        setInputs   (0, 900, HIGH);

        adk::ResourceRegistry resources;
        adk::AnalogJoystick   joystick (resources, config (true, true));

        require                           (joystick.initialize ().ok (), "inverted joystick initializes");
        auto snapshot = joystick.snapshot ();
        require                           (snapshot.x.position == 1000, "inverted x minimum");
        require                           (snapshot.y.position == -1000, "inverted y maximum");

        setInputs (1023, 100, HIGH);
        require   (joystick.update (adk::TimePoint (1)).ok (),
                 "opposite inversion update");
        snapshot = joystick.snapshot ();
        require                      (snapshot.x.position == -1000, "inverted x maximum");
        require                      (snapshot.y.position == 1000, "inverted y minimum");
    }

    void testConfigurationFailures ()
    {
        const auto expectInvalid =
            [] (const adk::AnalogJoystickConfig& bad, const char* message)
        {
            fake::reset ();
            adk::ResourceRegistry resources;
            adk::AnalogJoystick   joystick (resources, bad);

            require (joystick.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     message);
            require (fake::trace ().empty (), "invalid configuration touched hardware");
        };

        expectInvalid ({{xPin, 0, 0, 1023, 0}, {yPin, 400, 100, 900, 50}, {selectPin}},
                       "center at observed endpoint rejected");
        expectInvalid (
            {{xPin, 512, 512, 512, 0}, {yPin, 400, 100, 900, 50}, {selectPin}},
            "zero span rejected");
        expectInvalid (
            {{xPin, 512, 0, 1023, 512}, {yPin, 400, 100, 900, 50}, {selectPin}},
            "overlapping dead zone rejected");
        expectInvalid (
            {{xPin, 512, 0, 1023, 100}, {xPin, 400, 100, 900, 50}, {selectPin}},
            "duplicate analog pins rejected");
        expectInvalid ({{xPin, 512, 0, 1023, 100}, {yPin, 400, 100, 900, 50}, {xPin}},
                       "duplicate select pin rejected");

        fake::reset ();
        adk::ResourceRegistry unsupportedResources;
        adk::AnalogJoystick   unsupported (
            unsupportedResources,
            {{53, 512, 0, 1023, 100}, {yPin, 400, 100, 900, 50}, {selectPin}});
        require (unsupported.initialize ().error () == adk::StatusCode::Unsupported,
                 "digital-only x pin rejected");
        require (fake::trace ().empty (), "unsupported pin touched hardware");
    }

    void testBusyRollbackAndClaimReuse ()
    {
        fake::reset ();
        setInputs   (512, 400, HIGH);

        adk::ResourceRegistry resources;
        adk::AnalogInput      yOwner (resources, yPin);
        require                      (yOwner.initialize ().ok (), "y owner initializes");

        fake::clearTrace ();
        {
            adk::AnalogJoystick joystick (resources, config ());

            require (joystick.initialize ().error () == adk::StatusCode::ResourceBusy,
                     "busy y rejects initialization");
            require          (!joystick.initialized (), "partial initialization rolled back");
            require          (fake::trace ().size () == 3, "x rollback trace");
            requireOperation (0, fake::OperationKind::PinMode, xPin);
            requireOperation (1, fake::OperationKind::AnalogRead, xPin);
            requireOperation (2, fake::OperationKind::PinMode, xPin);
        }

        fake::clearTrace        ();
        adk::AnalogInput xReuse (resources, xPin);
        require                 (xReuse.initialize ().ok (), "rolled-back x claim reusable");

        xReuse.shutdown  ();
        yOwner.shutdown  ();
        fake::clearTrace ();

        adk::DigitalInput selectOwner (resources, selectPin);
        require                       (selectOwner.initialize ().ok (), "select owner initializes");
        fake::clearTrace              ();

        adk::AnalogJoystick blockedAtSelect (resources, config ());
        require                             (blockedAtSelect.initialize ().error () ==
                     adk::StatusCode::ResourceBusy,
                 "busy select rejects initialization");
        require (!blockedAtSelect.initialized (),
                 "select failure rolls back analog inputs");
        require (!resources.claimed ({adk::ResourceKind::Pin, xPin}),
                 "x released after select failure");
        require (!resources.claimed ({adk::ResourceKind::Pin, yPin}),
                 "y released after select failure");
    }

    void testButtonEventsBounceAndRollover ()
    {
        fake::reset ();
        setInputs   (512, 400, HIGH);

        adk::ResourceRegistry resources;
        adk::AnalogJoystick   joystick (resources, config ());
        require                        (joystick.initialize ().ok (), "button joystick initializes");

        setInputs       (512, 400, LOW);
        joystick.update (adk::TimePoint (0xfffffff8U));
        joystick.update (adk::TimePoint (0x0000000bU));
        require         (!joystick.snapshot ().selected, "wrapped debounce not early");

        joystick.update (adk::TimePoint (0x0000000cU));
        require         (joystick.snapshot ().selected, "wrapped debounce commits");
        require         (joystick.snapshot ().selectEvent, "press event emitted");
        require         (joystick.snapshot ().selectEvent, "event is non-consuming");

        joystick.update (adk::TimePoint (0x0000000dU));
        require         (!joystick.snapshot ().selectEvent, "next update clears press");

        setInputs       (512, 400, HIGH);
        joystick.update (adk::TimePoint (100));
        setInputs       (512, 400, LOW);
        joystick.update (adk::TimePoint (105));
        setInputs       (512, 400, HIGH);
        joystick.update (adk::TimePoint (110));
        joystick.update (adk::TimePoint (129));
        require         (joystick.snapshot ().selected, "release bounce retained press");
        joystick.update (adk::TimePoint (130));
        require         (!joystick.snapshot ().selected, "release debounce commits");
        require         (joystick.snapshot ().releaseEvent, "release event emitted");
    }

    struct Replay
    {
        adk::AnalogJoystickSnapshot  snapshot;
        std::vector<fake::Operation> trace;
    };

    Replay replayTrace ()
    {
        fake::reset     ();
        fake::setTimeUs (7500);
        setInputs       (512, 400, HIGH);

        adk::ResourceRegistry resources;
        adk::AnalogJoystick   joystick (resources, config ());
        require                        (joystick.initialize ().ok (), "replay initializes");

        setInputs       (17, 876, LOW);
        joystick.update (adk::TimePoint (100));
        joystick.update (adk::TimePoint (120));

        return {joystick.snapshot (), fake::trace ()};
    }

    void testDeterministicReplayAndDestruction ()
    {
        const Replay first  = replayTrace ();
        const Replay second = replayTrace ();

        require (sameSnapshot (first.snapshot, second.snapshot),
                 "replayed snapshots differ");
        require (sameTrace (first.trace, second.trace),
                 "replayed hardware traces differ");

        fake::reset ();
        setInputs   (512, 400, HIGH);
        adk::ResourceRegistry resources;
        {
            adk::AnalogJoystick joystick (resources, config ());
            require                      (joystick.initialize ().ok (), "lifetime initializes");
            fake::clearTrace             ();
        }

        require (fake::trace ().size () == 3, "destructor shuts down endpoints");

        adk::AnalogInput xReuse (resources, xPin);
        require                 (xReuse.initialize ().ok (), "destructor released x claim");
    }
} // namespace

int main ()
{
    static_assert (!std::is_copy_constructible<adk::AnalogJoystick>::value, "");
    static_assert (!std::is_copy_assignable<adk::AnalogJoystick>::value, "");
    static_assert (!std::is_move_constructible<adk::AnalogJoystick>::value, "");
    static_assert (!std::is_move_assignable<adk::AnalogJoystick>::value, "");

    testLifecycleAndSampleOrder           ();
    testAxisMappingBoundaries             ();
    testAsymmetryAndInversion             ();
    testConfigurationFailures             ();
    testBusyRollbackAndClaimReuse         ();
    testButtonEventsBounceAndRollover     ();
    testDeterministicReplayAndDestruction ();

    std::cout << "All analog-joystick tests passed.\n";
}
