// Lesson 08: Light Meter
// A photoresistor on A1 measures the light, and five LEDs on pins 26 to 30
// show it as a bar.

#include <Adk.h>

adk::AnalogInput        sensor {A1};
adk::Array<adk::Led, 5> bar    {26, 27, 28, 29, 30};

adk::Smoother light {3};
adk::Timer    learning;

int darkest   = 1023;
int brightest = 0;

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
    learnTheRoom ();
}

void loop ()
{
    adk::update ();

    int level = light.add (sensor.read ());
    showBar (level);
    adk::println (Serial, "level:", level);

    adk::wait (20);
}

// For five seconds, while the white LED blinks, remember the darkest and
// the brightest readings. Cover the sensor, then shine a light on it.
void learnTheRoom ()
{
    bar[4].blink (250);
    learning.start (5000);

    while (learning.isRunning ())
    {
        adk::update ();

        int reading = sensor.read ();
        darkest     = min (darkest, reading);
        brightest   = max (brightest, reading);
    }

    // Keep the two apart, in case the light never changed.
    brightest = max (brightest, darkest + 60);
}

// The bar can show six things, from no LEDs to all five, so the range from
// darkest to brightest is cut into six equal slices, 0 to 5.
void showBar (int level)
{
    long lit = constrain (map (level, darkest, brightest, 0, 6), 0, 5);

    for (int led = 0; led < 5; led++)
    {
        bar[led].set (led < lit);
    }
}
