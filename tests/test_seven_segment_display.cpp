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

    void requireOk (adk::Status status, const char* message)
    {
        require (status.ok (), message);
    }

    void requireError (
        adk::Status     status,
        adk::StatusCode error,
        const char*     message)
    {
        require (status.error () == error, message);
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

        requireOk (display.initialize (), "cathode display initializes");

        for (uint8_t index = 0; index < sizeof (expected); ++index)
        {
            const auto glyph = static_cast<adk::SevenSegmentGlyph> (index);

            require (adk::validSevenSegmentGlyph (glyph),
                     "canonical glyph is valid");
            require (adk::encodeSevenSegmentGlyph (
                         glyph,
                         adk::SevenSegmentPolarity::CommonCathode) ==
                         expected[index],
                     "pure cathode glyph encoding");
            require (adk::encodeSevenSegmentGlyph (
                         glyph,
                         adk::SevenSegmentPolarity::CommonAnode) ==
                         static_cast<uint8_t> (~expected[index]),
                     "pure anode glyph encoding");
            require (adk::encodeSevenSegmentGlyph (
                         glyph,
                         adk::SevenSegmentPolarity::CommonCathode,
                         true) ==
                         static_cast<uint8_t> (expected[index] | 0x80U),
                     "pure cathode decimal-point encoding");
            require (adk::encodeSevenSegmentGlyph (
                         glyph,
                         adk::SevenSegmentPolarity::CommonAnode,
                         true) ==
                         static_cast<uint8_t> (
                             ~ (expected[index] | 0x80U)),
                     "pure anode decimal-point encoding");
            requireOk (display.show (glyph), "glyph is accepted");
            require   (display.glyph () == glyph, "glyph is remembered");
            require   (display.encodedValue () == expected[index],
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

        requireOk (display.initialize (), "anode display initializes");
        require   (display.encodedValue () == 0xffU,
                   "anode initialization is blank");
        requireOk (display.show (adk::SevenSegmentGlyph::Two, true),
                   "anode glyph is shown");
        require   (display.encodedValue () ==
                       static_cast<uint8_t> (~0xdbU),
                   "anode encoding is inverted");
        require   (display.decimalPoint (), "decimal point is remembered");
        require   (display.polarity () ==
                       adk::SevenSegmentPolarity::CommonAnode,
                   "polarity is reported");
        requireOk (display.blank (), "display blanks");
        require   (display.encodedValue () == 0xffU, "anode blank is inactive");
        require   (!display.decimalPoint (), "blank clears decimal point");
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

        requireError (display.show (adk::SevenSegmentGlyph::Eight),
                      adk::StatusCode::NotInitialized,
                      "stopped display rejects a glyph");

        requireOk (display.initialize (), "display initializes");

        fake::clearTrace ();

        requireOk (display.initialize (), "repeated initialization succeeds");
        require   (fake::trace ().empty (), "repeated initialization is inert");

        requireError (
            display.show (static_cast<adk::SevenSegmentGlyph> (255)),
            adk::StatusCode::InvalidArgument,
            "unknown glyph is rejected");

        require (fake::trace ().empty (), "unknown glyph touches no hardware");
        require (!adk::validSevenSegmentGlyph (
                     static_cast<adk::SevenSegmentGlyph> (255)),
                 "pure encoder validation rejects unknown glyph");
        require (adk::encodeSevenSegmentGlyph (
                     static_cast<adk::SevenSegmentGlyph> (255),
                     adk::SevenSegmentPolarity::CommonCathode,
                     true) == 0x00U,
                 "invalid cathode glyph encodes inactive");
        require (adk::encodeSevenSegmentGlyph (
                     static_cast<adk::SevenSegmentGlyph> (255),
                     adk::SevenSegmentPolarity::CommonAnode,
                     true) == 0xffU,
                 "invalid anode glyph encodes inactive");

        requireOk (display.show (adk::SevenSegmentGlyph::Eight),
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

        requireError (display.initialize (),
                      adk::StatusCode::InvalidArgument,
                      "unknown polarity is rejected");
        require (!display.initialized (), "invalid display stays stopped");
        require (fake::trace ().empty (), "invalid polarity touches no hardware");
        require (!adk::validSevenSegmentPolarity (
                     static_cast<adk::SevenSegmentPolarity> (255)),
                 "pure encoder validation rejects unknown polarity");
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

        requireOk (owner.initialize (), "pin owner initializes");

        fake::clearTrace ();

        requireError (display.initialize (),
                      adk::StatusCode::ResourceBusy,
                      "display reports a pin conflict");
        require (!display.initialized (), "conflicted display stays stopped");

        owner.shutdown ();

        requireOk (display.initialize (), "display reuses a released pin");
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

            requireOk (display.initialize (), "scoped display initializes");
            requireOk (display.show (adk::SevenSegmentGlyph::Eight),
                       "scoped display is active");
        }

        adk::DigitalOutput data  (resources, displayPins.data);
        adk::DigitalOutput clock (resources, displayPins.clock);
        adk::DigitalOutput latch (resources, displayPins.latch);

        requireOk (data.initialize (), "destructor releases data pin");
        requireOk (clock.initialize (), "destructor releases clock pin");
        requireOk (latch.initialize (), "destructor releases latch pin");
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
