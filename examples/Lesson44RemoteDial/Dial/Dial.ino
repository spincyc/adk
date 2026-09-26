// Lesson 44: Remote Dial, Board A
// Turn the knob to choose an angle, and the servo on Board B turns to it.
// The screen shows the angle asked for, and the angle Board B says its
// servo has got to. Press the knob to send the servo back to the middle.

#include <Adk.h>

adk::LoraModem     radio   {Serial3, 1, {.partner = 2,
                                         .speed   = adk::LoraSpeed::Quick,
                                         .power   = 10}};
adk::Bridge        bridge  {radio};
adk::Lcd           lcd     {31, 32, 33, 34, 35, 36};
adk::RotaryEncoder knob    {18, 19};
adk::Button        click   {22};
adk::Every         refresh {200};

constexpr int  step   = 5;             // degrees a click
constexpr char degree = char (223);    // the LCD's own degree sign

int angle = 90;    // the angle asked for, from 0 to 180

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    angle = constrain (angle + knob.turned () * step, 0, 180);

    if (click.wasPressed ())
    {
        angle = 90;
    }

    bridge.share ("angle", angle);

    if (refresh.ticked ())
    {
        showAngles ();
    }
}

// The angle asked for on the top row. Below it, where Board B says the
// servo has got to, or that Board B can't be heard. The spaces at the end
// rub out what a longer line left.
void showAngles ()
{
    adk::print (lcd.at (0, 0), "Asked  ", angle, degree, "   ");

    if (bridge.isConnected ())
    {
        adk::print (lcd.at (0, 1), "Servo  ", bridge.value ("at"), degree,
                    "      ");
    }
    else
    {
        lcd.at (0, 1).print ("Not connected   ");
    }
}
