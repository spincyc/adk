// Lesson 52: Remote Stepper, Board A
// The knob picks an angle, and the bridge carries it to Board B, whose
// stepper turns a turntable to it. Board B answers with where the
// turntable has got to, and the screen shows both.

#include <Adk.h>

adk::AnalogInput knob {A0};
adk::Lcd         lcd  {31, 32, 33, 34, 35, 36};
adk::Every       tick {100};

// The bridge to Board B, over the LoRa modem on Serial3.
adk::LoraModem radio  {Serial3, 1, {.partner = 2,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

constexpr char degree = char (223);    // the LCD's own degree sign

long angle = 0;                        // what the knob asks for, in degrees

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    readKnob ();
    bridge.share ("angle", angle);

    if (tick.ticked ())
    {
        showAngles ();
    }
}

// 0 to 360 degrees in fives. The angle only changes once the knob has
// turned a whole step from it, so a knob resting on the line between two
// angles doesn't flick between them, and the turntable doesn't twitch.
void readKnob ()
{
    long reading = knob.read (0, 360);

    if (abs (reading - angle) >= 5)
    {
        angle = reading / 5 * 5;
    }
}

// The knob's angle on top; below it, where Board B says its turntable is.
void showAngles ()
{
    long at = bridge.value ("at");

    adk::print (lcd.at (0, 0), "Knob says ", angle, degree, "   ");

    if (!bridge.isConnected ())
    {
        adk::print (lcd.at (0, 1), "No word from B  ");
    }
    else if (at == angle)
    {
        adk::print (lcd.at (0, 1), "Arrived: ", at, degree, "    ");
    }
    else
    {
        adk::print (lcd.at (0, 1), "Turning: ", at, degree, "    ");
    }
}
