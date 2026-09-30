// Lesson 54: Radio Pong
// Pong between two rooms. Your paddle is the bottom row of your matrix,
// and its top row opens onto the other player's matrix, over the bridge.
// Board A serves first; after a miss, whoever missed serves next.
// Only firstServer differs: true on Board A, false on Board B.

#include <Adk.h>

adk::LedMatrix matrix   {47, 48, 49};
adk::Joystick  joystick {A3, A4};
adk::Button    stick    {22};
adk::Speaker   speaker  {10};
adk::Led       linked   {LED_BUILTIN};
adk::Every     ballStep {250};
adk::Every     nudge    {80};         // a held stick moves the paddle

// One initial owner, even if both players click their sticks together.
constexpr bool firstServer = true;

adk::LoraModem radio  {Serial3, firstServer ? 1 : 2,
                                {.partner = firstServer ? 2 : 1,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

constexpr adk::Note crash [] {{adk::note::g4, 150}, {adk::note::c4, 400}};
constexpr adk::Note cheer [] {{adk::note::c5, 80}, {adk::note::g5, 160}};

// On this paddle waiting to be served, on this matrix, or on the other.
enum class Ball { Serving, Here, There };

Ball   ball   = firstServer ? Ball::Serving : Ball::There;
int8_t paddle = 2;                  // its left dot: it is three dots wide
int8_t x = 3, y = 6;                // the ball, while it's on this side
int8_t dx = 0, dy = -1;             // its way: dx across, dy down

// What crosses the bridge, kept apart from the ball so the bridge only
// sends when something happens: a count of crossings, with where, which
// way and how fast the ball went, and a count of misses for the score.
long crossings = 0;
long column    = 0;
long drift     = 0;
long pace      = 250;
long misses    = 0;

// The other board's counts as last heard, -1 before any.
long theirCrossings = -1;
long theirMisses    = -1;

adk::Text<12> score;

void setup ()
{
    adk::setup ();
    randomSeed (analogRead (A7));
}

void loop ()
{
    adk::update ();

    // The initial zero can be lost before the first handover arrives.
    bool firstBall = theirCrossings < 0 && bridge.changed ("ball")
                  && bridge.value ("ball") > 0;

    if (wentUp ("ball", theirCrossings) || firstBall)
    {
        catchBall ();
    }

    if (wentUp ("misses", theirMisses))
    {
        speaker.play (cheer);
        showScore ();
    }

    if (nudge.ticked ())
    {
        paddle = constrain (paddle + joystick.x () / 60, 0, 5);
    }

    if (ball == Ball::Serving)
    {
        x = paddle + 1;             // the ball rides on the paddle's middle
        y = 6;
    }

    if (ball == Ball::Serving && stick.wasPressed ())
    {
        dx = random (-1, 2);
        dy = -1;
        ballStep.period (250);
        ballStep.restart ();
        ball = Ball::Here;
    }

    if (ball == Ball::Here && ballStep.ticked ())
    {
        moveBall ();
    }

    long flight = pace * 32 + (drift + 1) * 8 + column;
    bridge.shareEvent ("ball", crossings, flight);
    bridge.share ("misses", misses);

    linked.set (bridge.isConnected ());
    draw ();
}

// Whether the other board's count by this name has gone up: something
// happened. The first count heard, or one that went down because the other
// board restarted, only says where it has got to, as in Lesson 51.
bool wentUp (const char* name, long& seen)
{
    if (!bridge.changed (name))
    {
        return false;
    }

    long count = bridge.value (name);
    bool up    = seen >= 0 && count > seen;

    seen = count;
    return up;
}

// In at the top. The other player faces this way, so their left is this
// side's right: the column and the drift come mirrored.
void catchBall ()
{
    long flight = bridge.payload ("ball");
    x  = 7 - flight % 8;
    y  = 0;
    dx = -(flight / 8 % 4 - 1);
    dy = 1;
    ballStep.period (flight / 32);
    ballStep.restart ();
    speaker.tone (adk::note::e5, 20);
    ball = Ball::Here;
}

// One step. The sides bounce the ball; the paddle's left dot sends it
// left and its right dot right, each hit a little faster.
void moveBall ()
{
    x += dx;
    y += dy;

    if (x < 0 || x > 7)
    {
        dx = -dx;
        x += 2 * dx;
    }

    if (y < 0)
    {
        sendOver ();
    }
    else if (y == 6 && dy > 0 && x >= paddle && x <= paddle + 2)
    {
        dy = -1;
        dx = x - paddle - 1;
        ballStep.period (max (100UL, ballStep.period () - 10));
        speaker.tone (adk::note::c6, 30);
    }
    else if (y == 7)
    {
        ++misses;
        speaker.play (crash);
        showScore ();
        ball = Ball::Serving;
    }
}

// Over the bridge, as a new count of crossings. With no one there, the
// top is a wall to practice against.
void sendOver ()
{
    if (!bridge.isConnected ())
    {
        dy = 1;
        y  = 1;
        return;
    }

    column = x;
    drift  = dx;
    pace   = ballStep.period ();
    ++crossings;
    ball   = Ball::There;
}

// This side's points first: each is a miss by the other player.
void showScore ()
{
    score.clear ();
    adk::print (score, bridge.value ("misses"), "-", misses, "  ");
    matrix.scroll (score.c_str ());
}

// The paddle and the ball as eight rows of dots. A scrolling score is
// left to finish, unless the ball is in play here.
void draw ()
{
    if (matrix.isScrolling () && ball != Ball::Here)
    {
        return;
    }

    adk::Array<uint8_t, 8> rows {};
    rows[7] = 0b11100000 >> paddle;

    if (ball != Ball::There)
    {
        rows[y] |= 0b10000000 >> x;
    }

    matrix.show (rows);
}
