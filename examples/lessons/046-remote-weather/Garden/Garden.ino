// Lesson 46: Remote Weather, Board B, the garden
// Four sensors measure the garden, and every five seconds the bridge
// carries a report indoors: its number, then every reading as a whole
// number.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 2, {.partner = 1,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

adk::Dht11       dht        {16};
adk::Ds18b20     probe      {17};
adk::Thermistor  thermistor {A2};
adk::AnalogInput light      {A1};
adk::Led         online     {28};    // lit while indoors is heard
adk::Every       report     {5000};

constexpr long noReading = -10000;    // a thermometer that didn't answer

long reports = 0;    // how many reports have gone: 0 means none yet

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    online.set (bridge.isConnected ());

    if (report.ticked ())
    {
        ++reports;
        bridge.share ("report", reports);
        bridge.share ("air", dht.ok () ? tenths (dht.temperature ())
                                       : noReading);
        bridge.share ("humid", dht.ok () ? lround (dht.humidity ())
                                         : noReading);
        bridge.share ("probe", probe.ok () ? tenths (probe.celsius ())
                                           : noReading);
        bridge.share ("ntc", tenths (thermistor.celsius ()));
        bridge.share ("light", light.read (0, 100));
    }
}

// The bridge carries whole numbers, so 21.5 degrees goes as 215 tenths.
long tenths (float celsius)
{
    return lround (celsius * 10);
}
