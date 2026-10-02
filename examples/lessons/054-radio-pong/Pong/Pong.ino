// Lesson 54: Radio Pong
// Pong between two rooms. Your paddle is the bottom row of your matrix,
// and its top row opens onto the other player's matrix, over the bridge.
// Only firstServer differs: true on Board A, false on Board B.

#include <Adk.h>

// Board A has the ball at the start, so there is only ever one.
constexpr bool firstServer = false;

adk::LoraModem radio    {Serial3, firstServer ? 1 : 2,
                         {.partner = firstServer ? 2 : 1,
                          .speed   = adk::LoraSpeed::Quick,
                          .power   = 10}};
adk::Bridge    bridge   {radio};
adk::LedMatrix matrix   {47, 48, 49};
adk::Joystick  joystick {A3, A4};
adk::Button    stick    {22};
adk::Speaker   speaker  {10};
adk::Led       linked   {LED_BUILTIN};
adk::Every     ballStep {250};
adk::Every     nudge    {80};         // a held stick moves the paddle

constexpr adk::Note crash [] {{adk::note::g4, 150}, {adk::note::c4, 400}};
constexpr adk::Note cheer [] {{adk::note::c5, 80}, {adk::note::g5, 160}};

// On this paddle waiting to be served, on this matrix, or on the other.
enum class Ball { Serving, Here, There };

Ball   ball   = firstServer ? Ball::Serving : Ball::There;
int8_t paddle = 2;                  // its left dot: it is three dots wide
int8_t x = 3, y = 6;                // the ball, while it's on this side
int8_t dx = 0, dy = -1;             // its way: dx across, dy down

// What crosses the bridge: how many times the ball has gone over, with
// how it went the last time, and how many times this player has missed.
long crossings = 0;
long flight    = 0;
long misses    = 0;

// The other player's misses as last heard: -1 until the first count.
long heardMisses = -1;

adk::Text<12> score;

void setup ()
{
    adk::setup ();
    randomSeed (analogRead (A7));
}

void loop ()
{
    adk::update ();

    // A ball coming over the bridge, or one already here moving on.
    if (bridge.changed ("ball"))
    {
        catchBall ();
    }
    else if (ball == Ball::Here && ballStep.ticked ())
    {
        moveBall ();
    }

    // The other player missed: a point to this side. The first count
    // heard only says where theirs stands, and one that falls means their
    // board has started again.
    if (bridge.changed ("misses"))
    {
        if (heardMisses >= 0 && bridge.value ("misses") > heardMisses)
        {
            speaker.play (cheer);
            showScore ();
        }

        heardMisses = bridge.value ("misses");
    }

    if (nudge.ticked ())
    {
        paddle = constrain (paddle + joystick.x () / 60, 0, 5);
    }

    if (ball == Ball::Serving)
    {
        x = paddle + 1;             // the ball rides on the paddle's middle
        y = 6;

        if (stick.wasPressed ())
        {
            dx = random (-1, 2);
            dy = -1;
            ballStep.period (250);
            ballStep.restart ();
            ball = Ball::Here;
        }
    }

    bridge.shareEvent ("ball", crossings, flight);
    bridge.share ("misses", misses);

    linked.set (bridge.isConnected ());
    draw ();
}

// In at the top. The other player faces this way, so their left is this
// side's right: the column and the drift come mirrored.
void catchBall ()
{
    long from = bridge.payload ("ball");
    x  = 7 - from % 8;
    y  = 0;
    dx = -(from / 8 % 4 - 1);
    dy = 1;
    ballStep.period (from / 32);
    ballStep.restart ();
    speaker.tone (adk::note::e5, 20);
    ball = Ball::Here;
}

// One step. The sides bounce the ball; the top sends it over the bridge,
// as one more crossing with its pace, drift and column packed into one
// number, or with nobody there, bounces it back. The paddle's left dot
// sends it left and its right dot right, each hit a little faster.
void moveBall ()
{
    x += dx;
    y += dy;

    if (x < 0 || x > 7)
    {
        dx = -dx;
        x += 2 * dx;
    }

    if (y < 0 && bridge.isConnected ())
    {
        flight = ballStep.period () * 32 + (dx + 1) * 8 + x;
        ++crossings;
        ball = Ball::There;
    }
    else if (y < 0)
    {
        dy = 1;
        y  = 1;
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
