// Lesson 09: Light Theremin
// Wave a hand over the photoresistor on A1 to play the passive buzzer on pin 10. The knob on A0
// picks the octave, and the LEDs on pins 26 to 30 show which note of the scale is sounding.

#include <Adk.h>

adk::AnalogInput sensor {A1};
adk::Led         bar [] {{26}, {27}, {28}, {29}, {30}};
adk::AnalogInput knob   {A0};
adk::Speaker     buzzer {10};

adk::Smoother light {2};

// Two octaves of the pentatonic scale, C D E G A: five notes that sound good
// together in any order.
const uint16_t scale [] {
    adk::note::c4, adk::note::d4, adk::note::e4, adk::note::g4, adk::note::a4,
    adk::note::c5, adk::note::d5, adk::note::e5, adk::note::g5, adk::note::a5,
};
const int notes = 10;

// An octave up is exactly double the pitch.
const int octaveUp [] {1, 2, 4};

const adk::Note ready [] {
    {adk::note::c5, 120}, {adk::note::e5, 120}, {adk::note::g5, 120}, {adk::note::c6, 360},
};

int      open    = 0;           // the reading with no hand near the sensor
int      covered = 1023;        // the reading with the sensor covered
uint16_t playing = 0;           // the pitch sounding now, or 0 for silence

void setup ()
{
    adk::setup ();
    learnTheRoom ();
    buzzer.play (ready);
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
    for (int led = 0; led < 5; ++led)
    {
        bar[led].blink (250);
    }

    unsigned long start = millis ();
    while (millis () - start < 5000)
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
    return constrain (map (level, open, covered, 0, notes + 1), 0, notes);
}

// Sound one note of the scale in one of three octaves (0, 1 or 2), and light
// its LED. A note that is already sounding is left alone.
void playNote (int note, int octave)
{
    uint16_t pitch = scale[note] * octaveUp[octave];

    if (pitch != playing)
    {
        buzzer.tone (pitch);
        playing = pitch;
        showNote (note % 5);
    }
}

void fallSilent ()
{
    if (playing != 0)
    {
        buzzer.stop ();
        playing = 0;
        showNote (-1);
    }
}

// Light the LED for one of the five note names, C D E G A (0 to 4), or none
// for -1.
void showNote (int name)
{
    for (int led = 0; led < 5; ++led)
    {
        bar[led].set (led == name);
    }
}
