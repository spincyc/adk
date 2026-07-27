#include "greenhouse_health_pattern.h"

#include <Arduino.h>

#include <cstdlib>
#include <iostream>

namespace {

    using namespace adk;
    namespace fake = adk::test::arduino;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    struct Fixture
    {
        ResourceRegistry        resources;
        RgbLed                  led;
        GreenhouseHealthPattern pattern;

        Fixture ()
            : resources (), led (resources, {2, 220}, {3, 220}, {4, 220}), pattern (led)
        {
        }
    };

    void requireColor (const Rgb& actual, const Rgb& expected, const char* message)
    {
        require (actual == expected, message);
    }

    void testLifecycleAndDeduplication ()
    {
        fake::reset ();
        Fixture fixture;

        require (!fixture.pattern.initialized (), "construction is inert");
        require (
            fixture.pattern.update (TimePoint (0), GreenhouseMode::Starting).error () ==
                StatusCode::NotInitialized,
            "inactive update rejected");
        require (fixture.pattern.initialize ().ok (), "initialize");
        require (fixture.pattern.initialize ().ok (), "repeat initialize");

        fake::clearTrace ();
        require          (fixture.pattern.update (TimePoint (0), GreenhouseMode::Starting).ok (),
                 "first phase");
        requireColor                              (fixture.led.color (), Rgb (0, 0, 160), "starting blue");
        const size_t firstTraceSize = fake::trace ().size ();

        require (fixture.pattern.update (TimePoint (0), GreenhouseMode::Starting).ok (),
                 "repeat timestamp");
        require (fake::trace ().size () == firstTraceSize,
                 "repeat timestamp makes no write");

        fixture.pattern.shutdown ();
        require                  (!fixture.pattern.initialized (), "shutdown lifecycle");
        requireColor             (fixture.led.color (), Rgb (), "shutdown off");
        fixture.pattern.shutdown ();
    }

    void testEveryModeAndPhaseBoundary ()
    {
        struct Case
        {
            GreenhouseMode mode;
            uint32_t       at;
            Rgb            expected;
        };

        const Case cases[] = {{GreenhouseMode::Starting, 0, Rgb (0, 0, 160)},
                              {GreenhouseMode::Starting, 999, Rgb       (0, 0, 160)},
                              {GreenhouseMode::Starting, 1000, Rgb      ()},
                              {GreenhouseMode::Monitoring, 0, Rgb       (0, 160, 0)},
                              {GreenhouseMode::Monitoring, 100, Rgb     ()},
                              {GreenhouseMode::Watering, 0, Rgb         (0, 128, 128)},
                              {GreenhouseMode::Watering, 100, Rgb       ()},
                              {GreenhouseMode::Inhibited, 700, Rgb      (160, 80, 0)},
                              {GreenhouseMode::SensorFault, 200, Rgb    (160, 0, 0)},
                              {GreenhouseMode::SensorFault, 300, Rgb    ()},
                              {GreenhouseMode::OutputFault, 700, Rgb    (160, 0, 0)},
                              {GreenhouseMode::DisplayFault, 400, Rgb   (160, 80, 0)},
                              {GreenhouseMode::DisplayFault, 500, Rgb   ()},
                              {GreenhouseMode::RecordFault, 600, Rgb    (128, 0, 128)},
                              {GreenhouseMode::RecordFault, 700, Rgb    ()},
                              {GreenhouseMode::MultipleFaults, 499, Rgb (160, 0, 0)},
                              {GreenhouseMode::MultipleFaults, 500, Rgb (128, 0, 128)}};

        for (const Case& test : cases)
        {
            fake::reset ();
            Fixture fixture;

            require (fixture.pattern.initialize ().ok (), "initialize case");
            require (fixture.pattern.update (TimePoint (100), test.mode).ok (),
                     "anchor mode");
            require (
                fixture.pattern.update (TimePoint (100 + test.at), test.mode).ok (),
                "advance phase");
            requireColor (fixture.led.color (), test.expected, "mode phase color");
        }
    }

    void testModeChangeRolloverAndFailure ()
    {
        fake::reset ();
        Fixture fixture;

        require (fixture.pattern.initialize ().ok (), "initialize");
        require (fixture.pattern
                     .update (TimePoint (0xfffffff0UL), GreenhouseMode::Monitoring)
                     .ok     (),
                 "rollover anchor");
        requireColor (fixture.led.color (), Rgb (0, 160, 0), "heartbeat begins");
        require      (
            fixture.pattern.update (TimePoint (84), GreenhouseMode::Monitoring).ok (),
            "rollover boundary");
        requireColor (fixture.led.color (), Rgb (), "heartbeat ends at 100 ms");

        require (
            fixture.pattern.update (TimePoint (90), GreenhouseMode::OutputFault).ok (),
            "mode change");
        requireColor (fixture.led.color (), Rgb (160, 0, 0),
                      "mode change restarts phase");

        fixture.led.shutdown ();
        require              (
            fixture.pattern.update (TimePoint (600), GreenhouseMode::Monitoring)
                    .error () == StatusCode::NotInitialized,
            "LED failure returned");
        fixture.pattern.shutdown ();
    }
} // namespace

int main ()
{
    testLifecycleAndDeduplication    ();
    testEveryModeAndPhaseBoundary    ();
    testModeChangeRolloverAndFailure ();

    std::cout << "greenhouse health pattern tests passed\n";
    return EXIT_SUCCESS;
}
