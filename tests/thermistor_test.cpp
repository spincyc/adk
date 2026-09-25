#include "check.h"

#include <Arduino.h>
#include <adk/thermistor.h>

namespace {

    bool about (float value, float expected)
    {
        return value > expected - 0.01f && value < expected + 0.01f;
    }

    // The temperature a fresh thermistor on A0 shows for a steady reading.
    float celsiusAt (int reading, float beta = 3950)
    {
        adk::Thermistor thermistor {A0, beta};

        arduino::pin (A0).analog = reading;
        adk::setup ();
        return thermistor.celsius ();
    }
}

TEST (thermistorNeedsAnAnalogPin)
{
    adk::Thermistor thermistor {22};

    adk::setup ();

    CHECK (check::halted.fault == adk::Fault::NotAnalog);
    CHECK (check::halted.pin == 22);
}

TEST (thermistorAtMidScaleIsAtRoomTemperature)
{
    adk::Thermistor thermistor {A0};

    arduino::pin (A0).analog = 512;
    adk::setup ();

    CHECK (!check::halted.happened);
    CHECK (about (thermistor.celsius (), 25.044f));
    CHECK (about (thermistor.fahrenheit (), 77.079f));
}

TEST (thermistorMatchesHandWorkedReadings)
{
    // R = 10000 (1023 / r - 1); 1/T = 1/298.15 + ln (R / 10000) / 3950.
    CHECK (about (celsiusAt (256), 2.194f));
    CHECK (about (celsiusAt (500), 23.991f));
    CHECK (about (celsiusAt (768), 52.064f));
    CHECK (about (celsiusAt (300, 3435), 3.851f));
}

TEST (thermistorEndsAreClampedInsteadOfDividingByZero)
{
    CHECK (celsiusAt (0) == celsiusAt (1));
    CHECK (about (celsiusAt (0), -77.391f));

    CHECK (celsiusAt (1023) == celsiusAt (1022));
    CHECK (celsiusAt (1023) > 350);
}

TEST (thermistorSamplesEvery100msAndSmooths)
{
    adk::Thermistor thermistor {A0};

    arduino::pin (A0).analog = 512;
    adk::setup ();
    adk::update (1000);

    // One sample moves the smoothed reading an eighth of the way: 544.
    arduino::pin (A0).analog = 768;
    adk::update (1099);
    CHECK (about (thermistor.celsius (), 25.044f));

    adk::update (1100);
    CHECK (about (thermistor.celsius (), 27.892f));

    adk::update (1199);
    CHECK (about (thermistor.celsius (), 27.892f));

    adk::update (1200);
    CHECK (thermistor.celsius () > 28);
}
