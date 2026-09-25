// Lesson 27: Snake
// The classic game: steer the snake with the joystick to eat the blinking
// food. Each bite makes it longer and faster. Don't hit a wall or yourself.

#include <Adk.h>

adk::LedMatrix matrix   {47, 48, 49};
adk::Joystick  joystick {A3, A4};
adk::Button    stick    {22};
adk::Speaker   speaker  {10};
adk::Every     step     {400};
adk::Every     blink    {150};

constexpr adk::Note fanfare [] {{adk::note::c5, 100}, {adk::note::e5, 100},
                                {adk::note::g5, 200}};
constexpr adk::Note gulp    [] {{adk::note::c6, 40}, {adk::note::g6, 60}};
constexpr adk::Note crash   [] {{adk::note::g4, 200}, {adk::note::e4, 200},
                                {adk::note::c4, 500}};

// A dot on the matrix: x from 0 on the left, y from 0 at the top.
struct Dot
{
    int x;
    int y;

    bool operator== (const Dot&) const = default;
};

adk::Deque<Dot, 64>      snake;      // its head at the front, tail at the back
adk::Joystick::Direction turn;       // the way it goes on its next step
Dot                      food;
bool                     playing = false;
char                     message [24] = "SNAKE! CLICK TO PLAY   ";

void setup ()
{
    adk::setup ();
    randomSeed (analogRead (A7));
}

void loop ()
{
    adk::update ();

    if (!playing)
    {
        matrix.scroll (message);

        if (stick.wasPressed ())
        {
            newGame ();
        }
    }
    else
    {
        steer ();

        if (step.ticked ())
        {
            moveSnake ();
        }

        if (blink.ticked ())
        {
            matrix.set (food.x, food.y, !matrix.get (food.x, food.y));
        }
    }
}

// Three dots in the middle row, heading right.
void newGame ()
{
    matrix.clear ();
    snake.clear ();

    for (int x = 1; x <= 3; ++x)
    {
        snake.push_front ({x, 4});
        matrix.set (x, 4);
    }

    turn = adk::Joystick::Right;
    step.period (400);
    step.restart ();
    placeFood ();
    speaker.play (fanfare);
    playing = true;
}

// Any way but straight back into its own neck.
void steer ()
{
    auto way = joystick.direction ();

    if (way != adk::Joystick::Center && ahead (way) != snake[1])
    {
        turn = way;
    }
}

// A new head goes on at the front and the tail comes off the back, unless
// the snake is eating: then it keeps its tail, and grows.
void moveSnake ()
{
    auto head   = ahead (turn);
    auto tail   = snake.back ();
    bool eating = head == food;
    bool wall   = head.x < 0 || head.x > 7 || head.y < 0 || head.y > 7;

    // The tail moves on as the head moves, so the head may take its place.
    if (wall || (onSnake (head) && head != tail))
    {
        gameOver ();
        return;
    }

    if (!eating)
    {
        snake.pop_back ();
        matrix.set (tail.x, tail.y, false);
    }

    snake.push_front (head);
    matrix.set (head.x, head.y);

    if (eating)
    {
        speaker.play (gulp);
        step.period (max (120UL, step.period () - 20));
        placeFood ();
    }
}

// The dot next to the snake's head, the given way.
Dot ahead (adk::Joystick::Direction way)
{
    auto dot = snake.front ();

    switch (way)
    {
        case adk::Joystick::Up:    --dot.y; break;
        case adk::Joystick::Down:  ++dot.y; break;
        case adk::Joystick::Left:  --dot.x; break;
        case adk::Joystick::Right: ++dot.x; break;
        default:                            break;
    }

    return dot;
}

bool onSnake (Dot dot)
{
    for (auto part : snake)
    {
        if (part == dot)
        {
            return true;
        }
    }

    return false;
}

// Anywhere the snake isn't, unless it fills the whole matrix.
void placeFood ()
{
    do
    {
        food.x = random (8);
        food.y = random (8);
    }
    while (onSnake (food) && !snake.full ());
}

void gameOver ()
{
    int score = snake.size () - 3;

    speaker.play (crash);
    adk::wait (1500);
    snprintf (message, sizeof message, "SCORE %d   ", score);
    playing = false;
}
