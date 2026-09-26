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

// A maze is a picture, as in Lesson 25, but each 1 is a wall. Every maze is
// open at the top-left, where the ball starts, and at the exit, bottom-right.
using Maze = adk::Array<uint8_t, 8>;

constexpr adk::Array mazes
{
    Maze {0b00000000, 0b11111100, 0b00000000, 0b00111111,
          0b00000000, 0b11111100, 0b00000000, 0b00000000},
    Maze {0b00010000, 0b01010110, 0b01000100, 0b01111001,
          0b00000010, 0b01111010, 0b01000010, 0b00011000},
    Maze {0b00100000, 0b00101110, 0b00101000, 0b00001011,
          0b11101000, 0b00001110, 0b01111110, 0b00000000},
    Maze {0b01000000, 0b01011110, 0b01010010, 0b01010110,
          0b00010000, 0b11110111, 0b00000010, 0b01111000}
};

constexpr adk::Note go    [] {{adk::note::g5, 80}, {adk::note::c6, 160}};
constexpr adk::Note cheer [] {{adk::note::c5, 100}, {adk::note::e5, 100},
                              {adk::note::g5, 100}, {adk::note::c6, 400}};

// Where the ball is, in dots, and how far it rolls with each reading.
struct Ball
{
    float x;
    float y;
    float speedX;
    float speedY;
};

Ball  ball;
int   maze      = 0;
bool  playing   = false;
bool  exitLit   = false;
float flatPitch = 0;    // the tilt that counts as level
float flatRoll  = 0;

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

        if (lround (ball.x) == 7 && lround (ball.y) == 7)
        {
            celebrate ();
        }
    }
}

void chooseMaze ()
{
    if (knob.turned () != 0)
    {
        // Adding the count first keeps it from going below zero.
        maze = (maze + mazes.size () + knob.turned ()) % mazes.size ();
        speaker.tone (adk::note::c6, 10);
    }

    matrix.show (mazes[maze]);

    if (click.wasPressed ())
    {
        ball      = {0, 0, 0, 0};
        flatPitch = tilt.pitch ();
        flatRoll  = tilt.roll ();
        speaker.play (go);
        playing   = true;
    }
}

// The ball speeds up downhill and slows a little on its own, like a marble
// on a tray. A raised right end rolls it left; a raised far edge, towards
// you. It tries each way on its own, so it slides along a wall.
void rollBall ()
{
    ball.speedX = ball.speedX * 0.9 - (tilt.pitch () - flatPitch) * 0.001;
    ball.speedY = ball.speedY * 0.9 + (tilt.roll () - flatRoll) * 0.001;

    if (!isFree (ball.x + ball.speedX, ball.y))
    {
        ball.speedX = bump (ball.speedX);
    }

    ball.x += ball.speedX;

    if (!isFree (ball.x, ball.y + ball.speedY))
    {
        ball.speedY = bump (ball.speedY);
    }

    ball.y += ball.speedY;
}

// A dot is free if it is on the matrix and its bit in the maze is 0.
bool isFree (float x, float y)
{
    long column = lround (x);
    long row    = lround (y);
    bool inside = column >= 0 && column <= 7 && row >= 0 && row <= 7;

    return inside && (mazes[maze][row] & (0b10000000 >> column)) == 0;
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
    matrix.set (lround (ball.x), lround (ball.y));
}

// Out through the exit: a cheer, then on to choose the next maze.
void celebrate ()
{
    speaker.play (cheer);
    adk::wait (1500);
    maze    = (maze + 1) % mazes.size ();
    playing = false;
}
