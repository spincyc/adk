// Lesson 37: FM Radio
// An FM radio you tune with the rotary knob: turn it to step along the
// band, press it to find the next station. A knob on A0 sets the volume,
// and the LCD shows what you are listening to.

#include <Adk.h>

// Where you listen: adk::FmBand::Americas in North and South America,
// where stations are 0.2 MHz apart, and adk::FmBand::World elsewhere.
constexpr adk::FmBand band = adk::FmBand::World;

adk::Lcd           lcd        {31, 32, 33, 34, 35, 36};
adk::FmRadio       radio      {40, 41, 42, band};
adk::RotaryEncoder dial       {18, 19};
adk::Button        dialButton {22};
adk::AnalogInput   volumeKnob {A0};
adk::Every         tick       {200};

constexpr uint8_t block = 0xFF;    // the LCD's character with every dot lit

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    radio.step (dial.turned ());

    if (dialButton.wasPressed ())
    {
        radio.seekUp ();
    }

    if (tick.ticked ())
    {
        setVolume ();
        showStation ();
    }
}

// Only when the knob has moved: each change is a message to the radio.
void setVolume ()
{
    int volume = volumeKnob.read (0, 15);

    if (volume != radio.volume ())
    {
        radio.setVolume (volume);
    }
}

// The frequency and stereo on the top row. Below, the station's name and
// its signal as a bar: a block for every 10 dBµV.
void showStation ()
{
    if (!radio.ok ())
    {
        adk::print (lcd.at (0, 0), "No radio found! ");
        adk::print (lcd.at (0, 1), "Check pins 40-42");
        return;
    }

    int frequency = radio.frequency ();

    adk::print (lcd.at (0, 0), frequency < 1000 ? " " : "",
                adk::fixed (frequency / 10.0, 1), " MHz ",
                radio.isStereo () ? "Stereo" : "  Mono");

    adk::print (lcd.at (0, 1), "                ");
    adk::print (lcd.at (0, 1),
                radio.isTuning () ? "Tuning" : radio.stationName ());
    lcd.at (9, 1);

    for (int bar = 0; bar < radio.signal () / 10; ++bar)
    {
        lcd.write (block);
    }
}
