#include "radio_fixture.h"
#include "lcd_fixture.h"
#include "fake_i2c.h"

bool hasNews ();
bool hasReading (const char* name);
void showWeather ();
void showReading (const char* name, bool temperature);
adk::Color comfortOf (long tenths);

#include "../../examples/lessons/046-remote-weather/Indoors/Indoors.ino"

int main ()
{
    using namespace lesson_test;
    prepare ();
    fake_i2c::Chip clock {0x68};
    clock.registers[0] = 0x05;
    clock.registers[1] = 0x32;
    clock.registers[2] = 0x14;
    clock.registers[3] = 0x05;
    clock.registers[4] = 0x01;
    clock.registers[5] = 0x10;
    clock.registers[6] = 0x26;
    setup ();
    hear ("@1/0");
    expect (bridge.isConnected () && !hasNews (),
            "the garden heard, but no report yet: nothing to show");
    hear ("@1/1 report=1:0 air=1:236 humid=1:45 probe=1:231 ntc=1:230");
    expect (hasNews () && bridge.changed ("report"), "a report is news");
    listenToLcd ();
    showReading ("light", false);
    expect (display == "----", "a lost first light packet is missing, not zero");
    hear ("@1/1 light=1:50");
    display.clear ();
    showReading ("light", false);
    expect (display == "50", "a matching light reading completes the report");
    runFor (1);
    runFor (1100);
    expect (light.color () == adk::color::green, "a mild garden lights green");
    clock.registers[0] = 0x06;
    hear ("@1/1 report=2:0 air=2:-10000 humid=2:-10000");
    display.clear ();
    shown = 2;
    showWeather ();
    expect (display == "Light ----%Heard   14:32:06",
            "the new timestamp is paired with dashes for the missing field");
    clock.registers[0] = 0x07;
    hear ("@1/1 light=1:50");
    expect (!hasReading ("light"), "a delayed old field is still stale");
    expect (heardAt.second == 6, "a late field cannot move the report's timestamp");
    runFor (1);
    runFor (1100);
    expect (light.color () == adk::color::off,
            "a thermometer that didn't answer gives no comfort color");
    hear ("@1/1 report=3:0 air=3:236 humid=3:45");
    hear ("@1/1 report=3:0 air=3:236 humid=3:45 probe=3:231 ntc=3:230");
    expect (!bridge.changed ("report"), "a repeated report is not a new one");
    hear ("@1/1 light=3:50");
    expect (hasReading ("light"), "an unchanged measurement is fresh with its report number");
    display.clear ();
    showReading ("humid", false);
    expect (display == "45", "a new valid reading replaces the sensor failure marker");

    // The garden falls silent: the readings go, and the light fades out.
    for (int step = 0; step < 60; ++step)
    {
        runFor (100);
    }

    expect (!hasNews (), "silence is no news");
    runFor (1100);
    expect (light.color () == adk::color::off, "no news, no comfort color");
    hear ("@0/1");
    expect (!hasNews (), "restart clears report counts before any new measurements");
    hear ("@2/1 report=1:0 air=1:236");
    expect (hasReading ("air") && !hasReading ("light"),
            "after restart only readings from the new run are fresh");
    return result ();
}
