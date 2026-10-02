#include "radio_fixture.h"
#include "lcd_fixture.h"

void pressed (uint8_t button);
void keyIn (int digit);
bool countedDown ();
enum class State;
void enter (State next, adk::Color light, const char* message);

#include "../../examples/lessons/024-room-alarm/024-room-alarm.ino"

void pulse (unsigned long mark, unsigned long space)
{
    arduino::drive (2, LOW);
    arduino::advanceMicros (mark);
    arduino::drive (2, HIGH);
    arduino::advanceMicros (space);
    loop ();
}

void armAt (unsigned long after)
{
    using namespace lesson_test;
    prepare ();
    arduino::pin (A12).input = LOW;
    setup ();
    loop ();
    listenToLcd ();

    // Send a real NEC POWER frame ending on, or between, the old ticks.
    constexpr unsigned long frameUs = 9000 + 4500 + 32 * 562
                                       + 16 * 1687 + 16 * 562 + 562;
    unsigned long target = (arduino::now () / 1000 + after) * 1000;
    arduino::advanceMicros (target - arduino::now () - frameUs);
    loop ();
    uint32_t bits = 0x00FF00UL | uint32_t (adk::remote::power) << 16
                   | uint32_t (adk::remote::power ^ 0xFF) << 24;
    pulse (9000, 4500);

    for (int bit = 0; bit < 32; ++bit)
    {
        pulse (562, ((bits >> bit) & 1) ? 1687 : 562);
    }

    pulse (562, 0);
    expect (receiver.wasReceived (), "POWER reaches the actual IR receiver");
    expect (state == State::Leaving && countdown == 10,
            "POWER starts a full ten-second exit countdown");
    expect (display.find ("10") != std::string::npos, "the LCD starts at 10");
    runFor (1);    // the restarted Every records its start on this update
    runFor (999);
    expect (countdown == 10, "no decrement before a full second");
    runFor (1);
    expect (countdown == 9, "the first new tick displays 9");
}

int main ()
{
    armAt (1000);
    armAt (500);
    return lesson_test::result ();
}
