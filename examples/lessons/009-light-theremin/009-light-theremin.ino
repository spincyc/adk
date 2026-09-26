// Lesson 09: Light Theremin
// Wave a hand over the photoresistor on A1 to play the passive buzzer on
// pin 10. The knob on A0 picks the octave, and the LEDs on pins 26 to 30
// show which note of the scale is sounding.

#include <Adk.h>

adk::AnalogInput        sensor  {A1};
adk::Array<adk::Led, 5> bar     {26, 27, 28, 29, 30};
adk::AnalogInput        knob    {A0};
adk::Speaker            speaker {10};

adk::Smoother light {2};
adk::Timer    learning;

// Two octaves of the pentatonic scale, C D E G A: five notes that sound
// good together in any order.
constexpr adk::Array scale {
    adk::note::c4, adk::note::d4, adk::note::e4, adk::note::g4,
    adk::note::a4, adk::note::c5, adk::note::d5, adk::note::e5,
    adk::note::g5, adk::note::a5,
};

// An octave up is exactly double the pitch.
constexpr adk::Array octaveUp {1, 2, 4};

constexpr adk::Note ready [] {
    {adk::note::c5, 120}, {adk::note::e5, 120},
    {adk::note::g5, 120}, {adk::note::c6, 360},
};

int  open     = 0;       // the reading with no hand near the sensor
int  covered  = 1023;    // the reading with the sensor covered
bool sounding = false;   // whether one of the theremin's notes is sounding

void setup ()
{
    adk::setup ();
    learnTheRoom ();
    speaker.play (ready);
}

void loop ()
{
    adk::update ();

    int slice = shadowSlice (light.add (sensor.read ()));

    if (slice == 0)
    {
        fallSilent ();
    }
    else
    {
        playNote (slice - 1, knob.read () / 342);
    }

    adk::wait (10);
}

// For five seconds, while the LEDs blink, learn the light with no hand near
// and with the sensor covered.
void learnTheRoom ()
{
    for (auto& led : bar)
    {
        led.blink (250);
    }

    learning.start (5000);

    while (learning.isRunning ())
    {
        adk::update ();

        int reading = sensor.read ();
        open        = max (open, reading);
        covered     = min (covered, reading);
    }

    // Keep the two apart, in case the light never changed.
    covered = min (covered, open - 110);
    showNote (-1);
}

// How deep the shadow is, from open to covered, cut into eleven slices:
// slice 0 is no hand at all, and slices 1 to 10 are the ten notes.
int shadowSlice (int level)
{
    return constrain (map (level, open, covered, 0, 11), 0, 10);
}

// Sound one note of the scale in one of three octaves (0, 1 or 2), and
// light its LED. Asking for the note that is already sounding changes
// nothing, so this can run on every pass.
void playNote (int note, int octave)
{
    speaker.tone (scale[note] * octaveUp[octave]);
    showNote (note % 5);
    sounding = true;
}

// Stop the note, once: until the first note, the ready tune plays on.
void fallSilent ()
{
    if (sounding)
    {
        speaker.stop ();
        showNote (-1);
        sounding = false;
    }
}

// Light the LED for one of the five note names, C D E G A (0 to 4), or none
// for -1.
void showNote (int name)
{
    for (int led = 0; led < 5; led++)
    {
        bar[led].set (led == name);
    }
}
