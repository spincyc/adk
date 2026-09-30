#include "radio_fixture.h"

bool fresh (uint8_t sensor);
void showWeather ();
void printReading (uint8_t sensor, bool temperature);
adk::Color comfortOf (long tenths);

#include "../../examples/lessons/046-remote-weather/Indoors/Indoors.ino"

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    hear ("@");
    expect (!fresh (0), "heartbeat alone is not weather data");
    hear ("@humid=1:45 probe=1:231 ntc=1:230 light=1:50");
    expect (!fresh (0), "other readings do not invent a missing air reading");
    hear ("@air=1:236");
    expect (fresh (0), "new air record is fresh");
    runFor (1);
    runFor (1100);
    expect (light.color () == adk::color::green,
            "valid mild temperature lights green");
    hear ("@air=2:-10000 humid=2:-10000 probe=2:-10000");
    runFor (1);
    runFor (1100);
    expect (light.color () == adk::color::off,
            "failed thermometer has no comfort color");
    hear ("@air=3:236");
    runFor (1);
    for (int report = 4; report < 10; ++report)
    {
        runFor (1900);
        std::string other = "@light=" + std::to_string (report) + ":50";
        hear (other.c_str ());
        hear ("@air=3:236");    // repeats are not new measurements
    }
    expect (bridge.isConnected () && fresh (4) && !fresh (0),
            "new light records and old air repeats cannot freshen old air");
    runFor (1100);
    expect (light.color () == adk::color::off, "stale air data fades out");
    return result ();
}
