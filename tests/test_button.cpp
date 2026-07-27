#include <Arduino.h>
#include <button.h>

#include <cstdlib>
#include <iostream>

namespace {
    constexpr adk::PinId buttonPin = adk::PinId (7);

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    void sample (adk::Button& button, uint8_t level, uint32_t timeMs)
    {
        adk::test::arduino::setDigitalInput (7, level);

        button.update (adk::TimePoint (timeMs));
    }

    void testInitializationAndDiagnostics ()
    {
        adk::test::arduino::reset           ();
        adk::test::arduino::setDigitalInput (7, HIGH);

        adk::ResourceRegistry resources;
        adk::Button           button (
            resources, {buttonPin, adk::Pull::Up, adk::Level::Low, adk::Duration (20)});

        require (!button.initialized (), "button starts stopped");
        require (!button.rawPressed (), "raw state starts released");
        require (!button.pressed (), "stable state starts released");
        require (!button.pressEvent (), "press event starts clear");
        require (!button.releaseEvent (), "release event starts clear");

        button.update (adk::TimePoint (4));

        require (!button.rawPressed (), "stopped update is inert");
        require (button.initialize ().ok (), "button initializes");
        require (button.initialized (), "button reports initialized");
        require (button.input ().initialized (), "owned input initializes");
        require (adk::test::arduino::mode (7) == INPUT_PULLUP, "pull-up configured");
        require (!button.rawPressed (), "released raw diagnostic");

        sample (button, LOW, 5);

        require (button.rawPressed (), "active-low raw diagnostic");
        require (!button.pressed (), "raw state precedes debounced state");

        button.shutdown ();
        button.shutdown ();

        require (!button.initialized (), "shutdown is idempotent");
        require (!button.rawPressed (), "shutdown clears raw state");
        require (!button.pressed (), "shutdown clears stable state");
    }

    void testDebounceBoundaryAndSnapshots ()
    {
        adk::test::arduino::reset           ();
        adk::test::arduino::setDigitalInput (7, HIGH);

        adk::ResourceRegistry resources;
        adk::Button           button (
            resources, {buttonPin, adk::Pull::Up, adk::Level::Low, adk::Duration (20)});
        require (button.initialize ().ok (), "boundary initializes");

        sample (button, LOW, 100);

        require (button.rawPressed (), "candidate appears immediately");
        require (!button.pressEvent (), "candidate has no event");

        sample (button, LOW, 119);

        require (!button.pressed (), "one tick before boundary");

        sample (button, LOW, 120);

        require (button.pressed (), "exact boundary commits");
        require (button.pressEvent (), "boundary emits press");
        require (!button.releaseEvent (), "press is not release");

        require (button.pressEvent (), "event query is non-consuming");
        require (button.pressEvent (), "event remains a snapshot");

        sample (button, LOW, 121);

        require (!button.pressEvent (), "next update clears press snapshot");
        require (button.pressed (), "held press remains stable");

        sample (button, HIGH, 200);
        sample (button, HIGH, 219);

        require (button.pressed (), "release waits through boundary minus one");

        sample (button, HIGH, 220);

        require (!button.pressed (), "release commits at boundary");
        require (button.releaseEvent (), "boundary emits release");
        require (button.releaseEvent (), "release query is non-consuming");

        sample (button, HIGH, 221);

        require (!button.releaseEvent (), "next update clears release snapshot");
    }

    void testBounceRestartsTiming ()
    {
        adk::test::arduino::reset           ();
        adk::test::arduino::setDigitalInput (7, HIGH);

        adk::ResourceRegistry resources;
        adk::Button           button (
            resources, {buttonPin, adk::Pull::Up, adk::Level::Low, adk::Duration (20)});
        require (button.initialize ().ok (), "bounce initializes");

        sample (button, LOW, 10);
        sample (button, LOW, 29);
        sample (button, HIGH, 30);
        sample (button, LOW, 31);
        sample (button, LOW, 50);

        require (!button.pressed (), "last bounce restarts debounce");
        require (!button.pressEvent (), "bounce emits no event");

        sample (button, LOW, 51);

        require (button.pressed (), "quiet candidate eventually commits");
        require (button.pressEvent (), "quiet candidate emits one event");

        sample (button, HIGH, 60);
        sample (button, LOW, 65);
        sample (button, HIGH, 70);
        sample (button, HIGH, 89);

        require (button.pressed (), "release bounce retains press");
        require (!button.releaseEvent (), "release bounce emits no event");

        sample (button, HIGH, 90);

        require (!button.pressed (), "quiet release commits");
        require (button.releaseEvent (), "quiet release emits one event");
    }

    void testHeldAtStartupRequiresRelease ()
    {
        adk::test::arduino::reset           ();
        adk::test::arduino::setDigitalInput (7, LOW);

        adk::ResourceRegistry resources;
        adk::Button           button (
            resources, {buttonPin, adk::Pull::Up, adk::Level::Low, adk::Duration (20)});
        require (button.initialize ().ok (), "held initializes");
        require (button.rawPressed (), "held startup raw state");
        require (button.pressed (), "held startup stable state");
        require (!button.pressEvent (), "held startup suppresses press");

        sample (button, LOW, 100);

        require (!button.pressEvent (), "held input remains suppressed");

        sample (button, HIGH, 101);
        sample (button, HIGH, 121);

        require (button.releaseEvent (), "stable release is observable");

        sample (button, LOW, 122);
        sample (button, LOW, 142);

        require (button.pressEvent (), "release arms next press");
    }

    void testWraparound ()
    {
        adk::test::arduino::reset           ();
        adk::test::arduino::setDigitalInput (7, HIGH);

        adk::ResourceRegistry resources;
        adk::Button           button (
            resources, {buttonPin, adk::Pull::Up, adk::Level::Low, adk::Duration (20)});
        require (button.initialize ().ok (), "wrap initializes");

        sample (button, LOW, 0xfffffff8U);
        sample (button, LOW, 0x0000000bU);

        require (!button.pressed (), "wrapped elapsed boundary minus one");

        sample (button, LOW, 0x0000000cU);

        require (button.pressed (), "wrapped elapsed reaches boundary");
        require (button.pressEvent (), "wrapped press emits event");
    }

    void testActiveHighAndZeroDebounce ()
    {
        adk::test::arduino::reset           ();
        adk::test::arduino::setDigitalInput (7, LOW);

        adk::ResourceRegistry resources;
        adk::Button           button (
            resources, {buttonPin, adk::Pull::None, adk::Level::High, adk::Duration (0)});

        require (button.initialize ().ok (), "active-high initializes");
        require (!button.rawPressed (), "active-high low is released");
        require (adk::test::arduino::mode (7) == INPUT, "no-pull mode configured");

        sample (button, HIGH, 8);

        require (button.pressed (), "zero debounce commits immediately");
        require (button.pressEvent (), "zero debounce emits press");
    }
} // namespace

int main ()
{
    testInitializationAndDiagnostics  ();
    testDebounceBoundaryAndSnapshots  ();
    testBounceRestartsTiming          ();
    testHeldAtStartupRequiresRelease  ();
    testWraparound                    ();
    testActiveHighAndZeroDebounce     ();

    std::cout << "All Button tests passed.\n";
}
