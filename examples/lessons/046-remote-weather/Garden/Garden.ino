// Lesson 46: Remote Weather, Board A, the garden
// Four sensors measure the garden, and every five seconds the bridge
// carries a report indoors: every reading as a whole number.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 1, {.partner = 2,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

adk::Dht11       dht        {16};
adk::Ds18b20     probe      {17};
adk::Thermistor  thermistor {A2};
adk::AnalogInput light      {A1};
adk::Led         online     {28};    // lit while indoors is heard
adk::Every       report     {5000};

constexpr long noReading = -10000;    // outside either thermometer's range

long reports = 0;    // how many reports have gone, which marks a new one

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
        bridge.shareEvent ("air", reports, dht.ok ()
                        ? tenths (dht.temperature ()) : noReading);
        bridge.shareEvent ("humid", reports, dht.ok ()
                        ? lround (dht.humidity ()) : noReading);
        bridge.shareEvent ("probe", reports, probe.ok ()
                        ? tenths (probe.celsius ()) : noReading);
        bridge.shareEvent ("ntc", reports, tenths (thermistor.celsius ()));
        bridge.shareEvent ("light", reports, light.read (0, 100));
    }
}

// The bridge carries whole numbers, so 21.5 degrees goes as 215 tenths.
long tenths (float celsius)
{
    return lround (celsius * 10);
}
