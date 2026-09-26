// Lesson 28: Tilt
// A spirit level: tilt the breadboard and a bubble on the matrix floats to the
// high side. When the board is level both ways, the bubble sits in a frame.

#include <Adk.h>

adk::Mpu6050   tilt   {0x68};
adk::LedMatrix matrix {47, 48, 49};

constexpr float degreesPerDot = 3;    // the tilt that moves the bubble a dot

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    if (!tilt.ok ())
    {
        matrix.scroll ("NO SENSOR ");
    }
    else if (tilt.measured ())
    {
        showBubble (tilt.pitch (), tilt.roll ());
    }
}

// The bubble is 2 x 2 dots. Its top-left corner goes from 0 to 6, with 3 in
// the middle, so it can move three dots each way.
void showBubble (float pitch, float roll)
{
    int x = constrain (3 + lround (pitch / degreesPerDot), 0, 6);
    int y = constrain (3 - lround (roll / degreesPerDot), 0, 6);

    matrix.clear ();
    matrix.set (x, y);
    matrix.set (x + 1, y);
    matrix.set (x, y + 1);
    matrix.set (x + 1, y + 1);

    if (fabs (pitch) < 1 && fabs (roll) < 1)
    {
        drawFrame ();
    }
}

void drawFrame ()
{
    matrix.row (0, 0b11111111);
    matrix.row (7, 0b11111111);

    for (int y = 1; y < 7; ++y)
    {
        matrix.set (0, y);
        matrix.set (7, y);
    }
}
