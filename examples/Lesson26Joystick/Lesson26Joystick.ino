// Lesson 26: Joystick
// An etch-a-sketch: the stick moves a dot around the matrix and leaves a
// trail. Click the stick to lift or lower the pen; the button wipes it clean.

#include <Adk.h>

adk::LedMatrix matrix      {47, 48, 49};
adk::Joystick  joystick    {A3, A4};
adk::Button    stick       {22};
adk::Button    clearButton {23};
adk::Every     step        {20};
adk::Every     blink       {200};

int  penX     = 350;    // in hundredths of a dot: 350 is halfway across dot 3
int  penY     = 350;
bool penDown  = true;
bool drawn    = true;   // whether the picture has a dot under the pen
bool penShown = true;

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    if (stick.wasPressed ())
    {
        penDown = !penDown;
        drawn   = drawn || penDown;
    }

    if (clearButton.wasPressed ())
    {
        matrix.clear ();
        drawn = penDown;
    }

    if (step.ticked ())
    {
        movePen ();
    }

    if (blink.ticked ())
    {
        penShown = !penShown;
    }

    matrix.set (penX / 100, penY / 100, penDown || penShown);
}

void movePen ()
{
    int oldX = penX / 100;
    int oldY = penY / 100;

    penX = constrain (penX + joystick.x () / 4, 0, 799);
    penY = constrain (penY - joystick.y () / 4, 0, 799);

    if (penX / 100 != oldX || penY / 100 != oldY)
    {
        matrix.set (oldX, oldY, drawn);
        drawn = penDown || matrix.get (penX / 100, penY / 100);
    }
}
