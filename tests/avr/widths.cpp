// A few of the library's pure-logic tests, run on the AVR itself, where int
// is 16 bits, long 32 and double no wider than float, as on the Mega. The
// host tests run with wider types and can't see what these widths do.
//
// make avr-test copies this into build/ as a sketch, widths.ino (the
// Arduino library rules allow sketches only under examples/), builds it as
// a lesson is built and runs it in the
// simulator that comes with avr-gdb. That simulates the processor alone,
// with no timers or pins behind it, so millis () never moves and every test
// gives adk::update () its time. The result is left in adkReport, which
// avr-gdb reads back once the sketch stops at a break instruction.

#include <Adk.h>
#include <string.h>

static_assert (sizeof (int) == 2 && sizeof (long) == 4 && sizeof (double) == 4,
               "these tests are for the AVR's own widths");

char adkReport [96];

namespace {

    adk::Text<sizeof adkReport - 1> failed;
    int                             checks   = 0;
    int                             failures = 0;

    void check (bool passed, int line)
    {
        ++checks;

        if (!passed)
        {
            ++failures;
            adk::print (failed, ' ', line);
        }
    }

    // One end of a perfect radio: a line sent arrives at the other end in
    // the next update, which is soon enough for a Bridge, as it sends at
    // most one line an update.
    struct Wire : adk::Object, adk::Link
    {
        bool sendLine (const char* text) override
        {
            strncpy (other->waiting, text, sizeof waiting - 1);
            return true;
        }

        const char* heardLine () const override
        {
            return line[0] != '\0' ? line : nullptr;
        }

        void update (adk::Millis) override
        {
            strcpy (line, waiting);
            waiting[0] = '\0';
        }

        Wire* other = nullptr;
        char  waiting [64] {};
        char  line    [64] {};
    };

    Wire        wireA;
    Wire        wireB;
    adk::Bridge bridgeA {wireA};
    adk::Bridge bridgeB {wireB};
    adk::Millis now = 0;

    void run (adk::Millis ms)
    {
        for (adk::Millis step = 0; step < ms; step += 10)
        {
            now += 10;
            adk::update (now);
        }
    }

    constexpr long Lowest  = -2147483647L - 1;
    constexpr long Highest = 2147483647L;
}

#define CHECK(expression) check ((expression), __LINE__)

void setup ()
{
    // Printing at the ends of each type's range.
    adk::Text<80> text;
    adk::print (text, Lowest, ' ', Highest, ' ', -32768, ' ', 65535U, ' ',
                adk::dec (4294967295UL, 2), ' ', adk::dec (7, 2), ' ',
                adk::hex (0xBEEFUL, 8), ' ', adk::fixed (21.46, 1));
    CHECK (text == "-2147483648 2147483647 -32768 65535 "
                   "4294967295 07 0000BEEF 21.5");

    // Glides and fades over the longest times.
    CHECK (adk::interpolate (0, 65535, 0x80000000, 0xFFFFFFFF) == 32768);
    CHECK (adk::interpolate (65535, 0, 0x40000000, 0xFFFFFFFF) == 49151);
    CHECK (adk::interpolate (100, 300, 250, 1000) == 150);
    CHECK (adk::interpolate (300, 100, 250, 1000) == 250);

    // Time across the wrap of millis ().
    adk::StartTime start;
    start.restart (0xFFFFFFC0);
    CHECK (!start.beat (0x0000001F, 100));
    CHECK (start.beat (0x00000024, 100));
    CHECK (start.elapsed (0x00000030) == 0x0C);

    // The clock's date, read from flash.
    adk::DateTime built = adk::compiledAt (F ("Sep 24 2026"), F ("20:30:00"));
    CHECK (built.year == 2026 && built.month == 9 && built.day == 24);
    CHECK (built.hour == 20 && built.minute == 30 && built.second == 0);

    // A ring that wraps round its end, counted in 16-bit sizes.
    adk::Deque<int, 4> ring;

    for (int item = 1; item <= 6; ++item)
    {
        if (ring.full ())
        {
            ring.pop_front ();
        }

        ring.push_back (item * 1000);
    }

    CHECK (ring.size () == 4 && ring.front () == 3000 && ring.back () == 6000);
    CHECK (ring[1] == 4000);

    // A bridge carries a Mega's whole long, both ways round, and turns down
    // a number one past it.
    wireA.other = &wireB;
    wireB.other = &wireA;
    CHECK (adk::start ());

    bridgeA.shareEvent ("low", 0, 0);
    bridgeA.share ("big", 0);
    run (500);
    bridgeA.shareEvent ("low", 1, Lowest);
    bridgeA.share ("big", Highest);
    run (500);
    CHECK (bridgeB.payload ("low") == Lowest);
    CHECK (bridgeB.value ("low") == 1);
    CHECK (bridgeB.value ("big") == Highest);

    strcpy (wireB.waiting, "@1/1 big=2147483648");
    run (10);
    CHECK (bridgeB.value ("big") == Highest);
    CHECK (bridgeB.isConnected () && bridgeA.isConnected ());

    adk::Text<sizeof adkReport - 1> report;
    adk::print (report, "avr: ", checks, " checks, ", failures, " failures");

    if (failures != 0)
    {
        adk::print (report, ", at lines", failed.c_str ());
    }

    strcpy (adkReport, report.c_str ());

    // Stop the simulator, with everything written where avr-gdb looks.
    asm volatile ("break" : : "r" (adkReport) : "memory");
}

void loop ()
{
}
