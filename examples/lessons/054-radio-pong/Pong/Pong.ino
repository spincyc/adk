// Lesson 54: Radio Pong
// Pong between two rooms. Your paddle is the bottom row of your matrix,
// and its top row opens onto the other player's matrix, over the bridge.
// Click the stick to serve. Both boards run this sketch; only the radio's
// first line differs: Board A, Ping, is address 1, and Board B, Pong, 2.

#include <Adk.h>

adk::LedMatrix matrix   {47, 48, 49};
adk::Joystick  joystick {A3, A4};
adk::Button    stick    {22};
adk::Speaker   speaker  {10};
adk::Led       linked   {LED_BUILTIN};
adk::Every     ballStep {250};
adk::Every     nudge    {80};         // a held stick moves the paddle

// The bridge to the other board, over the LoRa modem on Serial3.
adk::LoraModem radio  {Serial3, 2, {.partner = 1,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};

constexpr adk::Note crash [] {{adk::note::g4, 150}, {adk::note::c4, 400}};
constexpr adk::Note cheer [] {{adk::note::c5, 80}, {adk::note::g5, 160}};

// On this paddle waiting to be served, on this matrix, or on the other.
enum class Ball { Serving, Here, There };

Ball   ball   = Ball::Serving;
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

adk::Text<12> score;

void setup ()
{
    adk::setup ();
    randomSeed (analogRead (A7));
}

void loop ()
{
    adk::update ();

    if (bridge.changed ("ball") && bridge.value ("ball") > 0)
    {
        catchBall ();
    }

    if (bridge.changed ("misses") && bridge.value ("misses") > 0)
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

    bridge.share ("ball", crossings);
    bridge.share ("column", column);
    bridge.share ("drift", drift);
    bridge.share ("pace", pace);
    bridge.share ("misses", misses);

    linked.set (bridge.isConnected ());
    draw ();
}

// In at the top. The other player faces this way, so their left is this
// side's right: the column and the drift come mirrored.
void catchBall ()
{
    x  = 7 - bridge.value ("column");
    y  = 0;
    dx = -bridge.value ("drift");
    dy = 1;
    ballStep.period (bridge.value ("pace"));
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
