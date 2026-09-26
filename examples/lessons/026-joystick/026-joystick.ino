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

// The pen keeps its place in hundredths of a dot, from 0 to 799 each way, so
// a gentle push adds up instead of being lost. 350 is halfway across dot 3.
int  penX     = 350;
int  penY     = 350;
bool penDown  = true;
bool penShown = true;    // while the pen is up, it blinks
bool drawn    = true;    // whether the picture has a dot under the pen

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

// The further the stick is pushed, the further the pen moves. The matrix
// counts y downwards, but the stick counts it upwards.
void movePen ()
{
    int oldX = penX / 100;
    int oldY = penY / 100;

    penX = constrain (penX + joystick.x () / 4, 0, 799);
    penY = constrain (penY - joystick.y () / 4, 0, 799);

    // Leaving a dot puts back what the picture has there.
    if (penX / 100 != oldX || penY / 100 != oldY)
    {
        matrix.set (oldX, oldY, drawn);
        drawn = penDown || matrix.get (penX / 100, penY / 100);
    }
}
