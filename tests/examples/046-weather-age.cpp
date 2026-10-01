#include "radio_fixture.h"

bool hasNews ();
void showWeather ();
void showReading (const char* name, bool temperature);
adk::Color comfortOf (long tenths);

#include "../../examples/lessons/046-remote-weather/Indoors/Indoors.ino"

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    hear ("@1/0");
    expect (bridge.isConnected () && !hasNews (),
            "the garden heard, but no report yet: nothing to show");
    hear ("@1/1 report=1 air=236 humid=45 probe=231 ntc=230 light=50");
    expect (hasNews () && bridge.changed ("report"), "a report is news");
    runFor (1);
    runFor (1100);
    expect (light.color () == adk::color::green, "a mild garden lights green");
    hear ("@1/1 report=2 air=-10000 humid=-10000");
    runFor (1);
    runFor (1100);
    expect (light.color () == adk::color::off,
            "a thermometer that didn't answer gives no comfort color");
    hear ("@1/1 report=3 air=236 humid=45");
    hear ("@1/1 report=3 air=236 humid=45 probe=231 ntc=230 light=50");
    expect (!bridge.changed ("report"), "a repeated report is not a new one");

    // The garden falls silent: the readings go, and the light fades out.
    for (int step = 0; step < 60; ++step)
    {
        runFor (100);
    }

    expect (!hasNews (), "silence is no news");
    runFor (1100);
    expect (light.color () == adk::color::off, "no news, no comfort color");
    return result ();
}
