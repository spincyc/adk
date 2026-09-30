#include <Adk.h>

#include <cassert>
#include <cstdio>
#include <cstring>

// The Arduino builder supplies these declarations before compiling a sketch.
struct Dot;
void newGame ();
void steer ();
void moveSnake ();
Dot ahead (adk::Joystick::Direction way);
bool onSnake (Dot dot);
void placeFood ();
void gameOver ();

#include "../../examples/lessons/027-snake/027-snake.ino"

int main ()
{
    arduino::reset ();
    arduino::pin (A3).analog = 512;
    arduino::pin (A4).analog = 512;
    setup ();
    newGame ();

    food = {7, 7};
    moveSnake ();
    assert (snake.size () == 3);
    assert ((snake.front () == Dot {4, 4}));
    assert (!matrix.get (1, 4));

    food = {5, 4};
    moveSnake ();
    assert (snake.size () == 4);
    assert (playing);
    assert (!onSnake (food));

    // A continuous 63-dot path leaves just (0, 1), beside its head, free.
    snake.clear ();
    matrix.clear ();
    snake.push_back ({0, 0});

    for (int8_t y = 0; y < 8; ++y)
    {
        for (int8_t offset = 0; offset < 7; ++offset)
        {
            auto x = static_cast<int8_t> (y % 2 == 0 ? 1 + offset : 7 - offset);
            snake.push_back ({x, y});
        }
    }

    for (int8_t y = 7; y >= 2; --y)
    {
        snake.push_back ({0, y});
    }

    for (auto dot : snake)
    {
        matrix.set (dot.x, dot.y);
    }

    food = {0, 1};
    turn = adk::Joystick::Down;
    assert (snake.size () == 63);
    assert (!onSnake (food));

    // A food blink coincides with the final bite and the end of the pause.
    step.period (150);
    step.restart ();
    blink.restart ();
    adk::update (0);
    arduino::advance (150);
    loop ();

    assert (snake.full ());
    assert (!playing);
    assert (blink.ticked ());
    assert (strcmp (message.c_str (), "YOU WIN! SCORE 61   ") == 0);

    for (int y = 0; y < 8; ++y)
    {
        for (int x = 0; x < 8; ++x)
        {
            assert (matrix.get (static_cast<uint8_t> (x), static_cast<uint8_t> (y)));
        }
    }

    loop ();
    assert (matrix.isScrolling ());

    newGame ();
    assert (playing);
    assert (snake.size () == 3);
    assert (step.period () == 400);
    assert (!onSnake (food));

    // Losing after the restart still reports the ordinary score.
    food = {7, 7};

    for (int moves = 0; moves < 5; ++moves)
    {
        moveSnake ();
    }

    assert (!playing);
    assert (strcmp (message.c_str (), "SCORE 0   ") == 0);
    std::puts ("Snake: movement, growth, full-board win and restart passed");
}
