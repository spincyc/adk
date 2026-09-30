#include "radio_fixture.h"

#include "../../examples/lessons/053-remote-lamp/Repeater/Repeater.ino"

// Decode the actual outgoing NEC pulse spaces from the fake timer writes.
uint32_t sentFrame ()
{
    std::vector<unsigned long> widths;
    bool on = false;
    unsigned long from = 0;
    for (auto [time, value] : TCCR3A.history)
    {
        bool now = value & _BV (COM3C1);
        if (now != on)
        {
            if (on || !widths.empty ())
            {
                widths.push_back (time - from);
            }
            from = time;
            on = now;
        }
    }
    lesson_test::expect (widths.size () == 67, "one complete NEC frame sent");
    if (widths.size () != 67)
    {
        return 0;
    }
    uint32_t bits = 0;
    for (int bit = 0; bit < 32; ++bit)
    {
        bits |= static_cast<uint32_t> (widths[3 + 2 * bit] > 1000) << bit;
    }
    return bits;
}

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    arduino::setCallCost (1);
    hear ("@press=0:0");
    hear ("@press=1:12");
    expect (sentFrame () == 0xF30CFF00, "first command reaches the IR LED");
    TCCR3A.history.clear ();
    // Lose count 2: a different address and command, repeated at count 3.
    hear ("@press=3:1193042");
    expect (sentFrame () == 0xAD521234,
            "repeat after loss forwards its own address and command");
    TCCR3A.history.clear ();
    hear ("@press=3:1193042");
    expect (TCCR3A.history.empty (), "refresh does not replay an old press");
    return result ();
}
