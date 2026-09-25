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

const adk::Note fanfare [] = {{adk::note::c5, 100}, {adk::note::e5, 100}, {adk::note::g5, 200}};
const adk::Note gulp    [] = {{adk::note::c6, 40}, {adk::note::g6, 60}};
const adk::Note crash   [] = {{adk::note::g4, 200}, {adk::note::e4, 200}, {adk::note::c4, 500}};

bool    playing = false;
uint8_t snake [64];         // the dots the snake covers, head first: x + 8 * y
uint8_t length;
uint8_t food;
int8_t  headingX, headingY; // the way the snake last moved
int8_t  turnX, turnY;       // the way it moves next
char    message [24] = "SNAKE! CLICK TO PLAY   ";

void setup ()
{
    adk::setup ();
    randomSeed (analogRead (A0));
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
            light (food, !matrix.get (food % 8, food / 8));
        }
    }
}

void newGame ()
{
    matrix.clear ();
    length = 3;

    for (uint8_t i = 0; i < length; ++i)
    {
        snake[i] = 3 - i + 8 * 4;
        light (snake[i], true);
    }

    headingX = turnX = 1;
    headingY = turnY = 0;
    step.period (400);
    placeFood ();
    speaker.play (fanfare);
    playing = true;
}

// Any way but straight back into its own neck.
void steer ()
{
    int8_t x = 0;
    int8_t y = 0;

    switch (joystick.direction ())
    {
        case adk::Joystick::Up:    y = -1; break;
        case adk::Joystick::Down:  y = 1;  break;
        case adk::Joystick::Left:  x = -1; break;
        case adk::Joystick::Right: x = 1;  break;
        default:                   return;
    }

    if (x != -headingX || y != -headingY)
    {
        turnX = x;
        turnY = y;
    }
}

void moveSnake ()
{
    headingX = turnX;
    headingY = turnY;

    int  x      = snake[0] % 8 + headingX;
    int  y      = snake[0] / 8 + headingY;
    int  head   = x + 8 * y;
    bool eating = (head == food);

    // The tail moves on as the head moves, so the head may take its place.
    if (x < 0 || x > 7 || y < 0 || y > 7 || onSnake (head, length - 1))
    {
        gameOver ();
        return;
    }

    if (eating)
    {
        ++length;
        speaker.play (gulp);
        step.period (max (120UL, step.period () - 20));
    }
    else
    {
        light (snake[length - 1], false);
    }

    for (uint8_t i = length - 1; i > 0; --i)
    {
        snake[i] = snake[i - 1];
    }

    snake[0] = head;
    light (head, true);

    if (eating)
    {
        placeFood ();
    }
}

bool onSnake (int dot, uint8_t count)
{
    for (uint8_t i = 0; i < count; ++i)
    {
        if (snake[i] == dot)
        {
            return true;
        }
    }

    return false;
}

void placeFood ()
{
    do
    {
        food = random (64);
    }
    while (length < 64 && onSnake (food, length));
}

void light (uint8_t dot, bool lit)
{
    matrix.set (dot % 8, dot / 8, lit);
}

void gameOver ()
{
    speaker.play (crash);
    adk::wait (1500);
    snprintf (message, sizeof message, "SCORE %d   ", length - 3);
    playing = false;
}
