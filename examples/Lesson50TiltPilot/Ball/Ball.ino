// Lesson 50: Tilt Pilot, Board B (the ball)
// Board A's tilt comes by radio, up to ten times a second. A ball rolls
// across the matrix the way A leans, and the servo leans with it.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 2, {.partner = 1,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};
adk::LedMatrix matrix {47, 48, 49};
adk::Servo     arm    {44};
adk::Every     frame  {20};      // the ball moves fifty times a second
adk::Every     second {1000};

float x      = 3.5;    // where the ball is, in dots
float y      = 3.5;
float speedX = 0;      // and how far it rolls each frame
float speedY = 0;
int   heard  = 0;      // new tilts heard this second

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
}

void loop ()
{
    adk::update ();

    long pitch = bridge.value ("pitch");
    long roll  = bridge.value ("roll");

    if (bridge.changed ("pitch") || bridge.changed ("roll"))
    {
        ++heard;
    }

    if (!bridge.isConnected ())
    {
        matrix.scroll ("CALLING A ");
    }
    else if (bridge.value ("sensor") == 0)
    {
        matrix.scroll ("NO SENSOR ");
    }
    else if (frame.ticked ())
    {
        rollBall (pitch, roll);
    }

    // Each new angle is a glide of a tenth of a second, about the time
    // until the next one arrives, so the arm moves smoothly, not in jumps.
    arm.moveTo (constrain (90 + pitch, 0, 180), 100);

    if (second.ticked ())
    {
        adk::println (Serial, "New tilts this second: ", heard);
        heard = 0;
    }
}

// As on Lesson 30's tray: the ball speeds up downhill and slows a little
// on its own, and it stops dead at an edge.
void rollBall (long pitch, long roll)
{
    speedX = speedX * 0.9 - pitch * 0.001;
    speedY = speedY * 0.9 + roll * 0.001;
    x      = constrain (x + speedX, 0, 7);
    y      = constrain (y + speedY, 0, 7);
    speedX = (x == 0 || x == 7) ? 0 : speedX;
    speedY = (y == 0 || y == 7) ? 0 : speedY;

    matrix.clear ();
    matrix.set (lround (x), lround (y));
}
