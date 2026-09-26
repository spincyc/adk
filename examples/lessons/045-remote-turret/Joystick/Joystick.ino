// Lesson 45: Remote Turret, Board A
// Push the stick left or right to turn the turret on Board B, and press
// it to start or stop the fan. The screen shows where the turret aims,
// whether the fan is on, and how far away Board B's sensor sees something,
// with a warning when it's close.

#include <Adk.h>

adk::LoraModem radio   {Serial3, 1, {.partner = 2,
                                     .speed   = adk::LoraSpeed::Quick,
                                     .power   = 10}};
adk::Bridge    bridge  {radio};
adk::Lcd       lcd     {31, 32, 33, 34, 35, 36};
adk::Joystick  stick   {A3, A4};
adk::Button    press   {22};
adk::Every     steer   {50};     // the aim moves a little at a time
adk::Every     refresh {250};

constexpr int  nothing = 400;          // cm: no echo, nothing within 4 m
constexpr int  closeBy = 30;           // cm: nearer than this, a warning
constexpr char degree  = char (223);   // the LCD's own degree sign

int  aim = 90;       // degrees: where the turret should point
bool fan = false;    // whether the fan should blow

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    // Pushed further, it turns faster: up to 4 degrees a step, 80 a second.
    if (steer.ticked ())
    {
        aim = constrain (aim - stick.x () / 25, 0, 180);
    }

    if (press.wasPressed ())
    {
        fan = !fan;
    }

    bridge.share ("aim", aim);
    bridge.share ("fan", fan);

    if (refresh.ticked ())
    {
        showTurret ();
    }
}

// The aim and the fan on the top row. Below them, what Board B's sensor
// sees. The spaces at the end rub out what a longer line left.
void showTurret ()
{
    adk::print (lcd.at (0, 0), "Aim ", aim, degree, "   ");
    lcd.at (9, 0).print (fan ? "Fan on " : "Fan off");

    long cm = bridge.value ("dist");

    if (!bridge.isConnected ())
    {
        lcd.at (0, 1).print ("Not connected   ");
    }
    else if (cm >= nothing)
    {
        lcd.at (0, 1).print ("Nothing ahead   ");
    }
    else
    {
        adk::print (lcd.at (0, 1), cm < closeBy ? "CLOSE! " : "Ahead  ", cm,
                    " cm    ");
    }
}
