#include <digital_output.h>
#include <seven_segment_display.h>

#include <Arduino.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {
    namespace fake = adk::test::arduino;

    constexpr adk::ShiftRegisterPins displayPins = {22, 23, 24};

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    void testGlyphTable ()
    {
        const uint8_t expected[] = {
            0x3fU, 0x06U, 0x5bU, 0x4fU, 0x66U, 0x6dU,
            0x7dU, 0x07U, 0x7fU, 0x6fU, 0x77U, 0x7cU,
            0x39U, 0x5eU, 0x79U, 0x71U, 0x40U, 0x00U
        };

        fake::reset ();

        adk::ResourceRegistry    resources;
        adk::SevenSegmentDisplay display (
            resources,
            displayPins,
            adk::SevenSegmentPolarity::CommonCathode);

        require (display.initialize () == adk::Status::Ok,
                 "cathode display initializes");

        for (uint8_t index = 0; index < sizeof (expected); ++index)
        {
            const auto glyph = static_cast<adk::SevenSegmentGlyph> (index);

            require (display.show (glyph) == adk::Status::Ok,
                     "glyph is accepted");
            require (display.glyph () == glyph, "glyph is remembered");
            require (display.encodedValue () == expected[index],
                     "cathode glyph encoding");
        }
    }

    void testPolarityAndDecimalPoint ()
    {
        fake::reset ();

        adk::ResourceRegistry    resources;
        adk::SevenSegmentDisplay display (
            resources,
            displayPins,
            adk::SevenSegmentPolarity::CommonAnode);

        require (display.initialize () == adk::Status::Ok,
                 "anode display initializes");
        require (display.encodedValue () == 0xffU,
                 "anode initialization is blank");
        require (display.show (adk::SevenSegmentGlyph::Two, true) ==
                     adk::Status::Ok,
                 "anode glyph is shown");
        require (display.encodedValue () ==
                     static_cast<uint8_t> (~0xdbU),
                 "anode encoding is inverted");
        require (display.decimalPoint (), "decimal point is remembered");
        require (display.polarity () ==
                     adk::SevenSegmentPolarity::CommonAnode,
                 "polarity is reported");
        require (display.blank () == adk::Status::Ok, "display blanks");
        require (display.encodedValue () == 0xffU, "anode blank is inactive");
        require (!display.decimalPoint (), "blank clears decimal point");
    }

    void testLifecycleAndValidation ()
    {
        fake::reset ();

        adk::ResourceRegistry    resources;
        adk::SevenSegmentDisplay display (
            resources,
            displayPins,
            adk::SevenSegmentPolarity::CommonCathode);

        require (!display.initialized (), "display starts stopped");
        require (display.show (adk::SevenSegmentGlyph::Eight) ==
                     adk::Status::NotInitialized,
                 "stopped display rejects a glyph");
        require (display.initialize () == adk::Status::Ok,
                 "display initializes");

        fake::clearTrace ();

        require (display.initialize () == adk::Status::Ok,
                 "repeated initialization succeeds");
        require (fake::trace ().empty (), "repeated initialization is inert");
        require (display.show (static_cast<adk::SevenSegmentGlyph> (255)) ==
                     adk::Status::InvalidArgument,
                 "unknown glyph is rejected");
        require (fake::trace ().empty (), "unknown glyph touches no hardware");

        require (display.show (adk::SevenSegmentGlyph::Eight) ==
                     adk::Status::Ok,
                 "active glyph is shown");
        display.shutdown ();

        require (!display.initialized (), "shutdown stops display");
        require (display.glyph () == adk::SevenSegmentGlyph::Blank,
                 "shutdown reports blank");
        require (display.encodedValue () == 0x00U,
                 "cathode shutdown latches blank");

        display.shutdown ();
    }

    void testInvalidPolarity ()
    {
        fake::reset ();

        adk::ResourceRegistry    resources;
        adk::SevenSegmentDisplay display (
            resources,
            displayPins,
            static_cast<adk::SevenSegmentPolarity> (255));

        require (display.initialize () == adk::Status::InvalidArgument,
                 "unknown polarity is rejected");
        require (!display.initialized (), "invalid display stays stopped");
        require (fake::trace ().empty (), "invalid polarity touches no hardware");
    }

    void testResourceConflictAndReuse ()
    {
        fake::reset ();

        adk::ResourceRegistry     resources;
        adk::DigitalOutput       owner (
            resources, displayPins.clock);
        adk::SevenSegmentDisplay display (
            resources,
            displayPins,
            adk::SevenSegmentPolarity::CommonCathode);

        require (owner.initialize () == adk::Status::Ok, "pin owner initializes");

        fake::clearTrace ();

        require (display.initialize () == adk::Status::ResourceBusy,
                 "display reports a pin conflict");
        require (!display.initialized (), "conflicted display stays stopped");

        owner.shutdown ();

        require (display.initialize () == adk::Status::Ok,
                 "display reuses a released pin");
    }

    void testDestructorBlanksAndReleases ()
    {
        fake::reset ();

        adk::ResourceRegistry resources;

        {
            adk::SevenSegmentDisplay display (
                resources,
                displayPins,
                adk::SevenSegmentPolarity::CommonAnode);

            require (display.initialize () == adk::Status::Ok,
                     "scoped display initializes");
            require (display.show (adk::SevenSegmentGlyph::Eight) ==
                         adk::Status::Ok,
                     "scoped display is active");
        }

        adk::DigitalOutput data  (resources, displayPins.data);
        adk::DigitalOutput clock (resources, displayPins.clock);
        adk::DigitalOutput latch (resources, displayPins.latch);

        require (data.initialize () == adk::Status::Ok,
                 "destructor releases data pin");
        require (clock.initialize () == adk::Status::Ok,
                 "destructor releases clock pin");
        require (latch.initialize () == adk::Status::Ok,
                 "destructor releases latch pin");
    }
}

int main ()
{
    static_assert (
        !std::is_copy_constructible<adk::SevenSegmentDisplay>::value,
        "display must not copy");
    static_assert (
        !std::is_move_constructible<adk::SevenSegmentDisplay>::value,
        "display must not move");

    testGlyphTable                  ();
    testPolarityAndDecimalPoint     ();
    testLifecycleAndValidation      ();
    testInvalidPolarity             ();
    testResourceConflictAndReuse    ();
    testDestructorBlanksAndReleases ();

    std::cout << "seven segment display tests passed\n";
}
