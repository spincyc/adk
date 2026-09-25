// Lesson 30: Tilt Maze
// Turn the knob to choose a maze and click to start. Then tilt the breadboard
// to roll the ball from the top-left corner to the blinking exit.

#include <Adk.h>

adk::Mpu6050       tilt    {0x68};
adk::LedMatrix     matrix  {47, 48, 49};
adk::RotaryEncoder knob    {18, 19};
adk::Button        click   {22};
adk::Speaker       speaker {10};
adk::Every         blink   {200};

const uint8_t mazes [4][8] =
{
    {0b00000000, 0b11111100, 0b00000000, 0b00111111,
     0b00000000, 0b11111100, 0b00000000, 0b00000000},
    {0b00010000, 0b01010110, 0b01000100, 0b01111001,
     0b00000010, 0b01111010, 0b01000010, 0b00011000},
    {0b00100000, 0b00101110, 0b00101000, 0b00001011,
     0b11101000, 0b00001110, 0b01111110, 0b00000000},
    {0b01000000, 0b01011110, 0b01010010, 0b01010110,
     0b00010000, 0b11110111, 0b00000010, 0b01111000}
};

const adk::Note go    [] = {{adk::note::g5, 80}, {adk::note::c6, 160}};
const adk::Note cheer [] = {{adk::note::c5, 100}, {adk::note::e5, 100}, {adk::note::g5, 100},
                            {adk::note::c6, 400}};

uint8_t maze    = 0;
bool    playing = false;
float   ballX, ballY;           // where the ball is, in dots
float   speedX, speedY;         // how far it rolls each reading, in dots
float   flatPitch, flatRoll;    // the tilt that counts as level
bool    exitLit;

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
    else if (!playing)
    {
        chooseMaze ();
    }
    else
    {
        if (blink.ticked ())
        {
            exitLit = !exitLit;
        }

        if (tilt.measured ())
        {
            rollBall ();
            drawGame ();
        }

        if (lround (ballX) == 7 && lround (ballY) == 7)
        {
            celebrate ();
        }
    }
}

void chooseMaze ()
{
    if (knob.turned () != 0)
    {
        maze = (maze + knob.turned () + 4) % 4;
        speaker.tone (adk::note::c6, 10);
    }

    matrix.show (mazes[maze]);

    if (click.wasPressed ())
    {
        ballX = ballY = 0;
        speedX = speedY = 0;
        flatPitch = tilt.pitch ();
        flatRoll  = tilt.roll ();
        speaker.play (go);
        playing = true;
    }
}

// The ball speeds up downhill and slows a little on its own, like a marble
// on a tray. A raised right end rolls it left; a raised far edge, towards you.
void rollBall ()
{
    speedX = speedX * 0.9 - (tilt.pitch () - flatPitch) * 0.001;
    speedY = speedY * 0.9 + (tilt.roll () - flatRoll) * 0.001;

    if (isFree (ballX + speedX, ballY))
    {
        ballX += speedX;
    }
    else
    {
        speedX = bump (speedX);
    }

    if (isFree (ballX, ballY + speedY))
    {
        ballY += speedY;
    }
    else
    {
        speedY = bump (speedY);
    }
}

bool isFree (float x, float y)
{
    long column = lround (x);
    long row    = lround (y);

    return column >= 0 && column < 8 && row >= 0 && row < 8
        && !(mazes[maze][row] & (0x80 >> column));
}

// A wall stops the ball dead, with a knock if it hit hard enough to hear.
float bump (float speed)
{
    if (fabs (speed) > 0.05)
    {
        speaker.tone (adk::note::c3, 20);
    }

    return 0;
}

void drawGame ()
{
    matrix.show (mazes[maze]);
    matrix.set (7, 7, exitLit);
    matrix.set (lround (ballX), lround (ballY));
}

// Out through the exit: a cheer, then on to choose the next maze.
void celebrate ()
{
    speaker.play (cheer);
    adk::wait (1500);
    maze    = (maze + 1) % 4;
    playing = false;
}
