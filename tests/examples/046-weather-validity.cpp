#include "radio_fixture.h"

#include <utility>
#include <vector>

long tenths (float celsius);

#include "../../examples/lessons/046-remote-weather/Garden/Garden.ino"

int main ()
{
    using namespace lesson_test;
    prepare ();
    arduino::setCallCost (4);
    arduino::pin (A1).analog = 512;
    arduino::pin (A2).analog = 512;

    // One good DHT reading, then a disconnected sensor. Time each bit on
    // the real driver's input; the example must report validity itself.
    std::vector<std::pair<int, unsigned long>> reply
        {{HIGH, 30}, {LOW, 80}, {HIGH, 80}};
    const uint8_t bytes [] {45, 0, 23, 6, 74};
    for (uint8_t byte : bytes)
    {
        for (int bit = 7; bit >= 0; --bit)
        {
            reply.push_back ({LOW, 50});
            reply.push_back ({HIGH, (byte & (1 << bit)) ? 70UL : 27UL});
        }
    }
    reply.push_back ({LOW, 50});
    bool answer = true;
    bool pending = true;
    unsigned long began = 0;
    arduino::onDigitalWrite = [&] (uint8_t pin, uint8_t value)
    {
        if (pin == 16 && value == LOW)
        {
            pending = true;
        }
    };
    arduino::onDigitalRead = [&] (uint8_t pin)
    {
        if (pin != 16 || !answer)
        {
            return HIGH;
        }
        if (pending)
        {
            pending = false;
            began = arduino::now ();
        }
        unsigned long elapsed = arduino::now () - began;
        for (const auto& phase : reply)
        {
            if (elapsed < phase.second)
            {
                return phase.first;
            }
            elapsed -= phase.second;
        }
        return HIGH;
    };

    setup ();
    runFor (1);
    runFor (1000);
    runFor (20);
    expect (dht.ok () && dht.temperature () > 23, "initial DHT reading is valid");
    answer = false;
    for (int step = 0; step < 30; ++step)
    {
        runFor (150);
    }
    expect (!dht.ok (), "the sensor has stopped answering");
    expect (Serial3.text.find ("air=1:-10000") != std::string::npos,
            "failed DHT sends missing marker instead of its retained temperature");
    expect (Serial3.text.find ("probe=1:-10000") != std::string::npos,
            "absent 18B20 sends missing marker instead of zero");
    expect (Serial3.text.find ("report=1:0") != std::string::npos,
            "each report carries its number");
    return result ();
}
