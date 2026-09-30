// Lesson 49: Remote Keypad, Board B (inside)
// Keys come by radio from Board A, at the door. This board keeps the
// secret code, decides, works the latch, and sends its answer back.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 2, {.partner = 1,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};
adk::Servo     latch  {44};
adk::RgbLed    light  {5, 6, 7};

constexpr adk::Array code        {'1', '2', '3', '4'};    // never sent
constexpr uint8_t    lockedAngle = 0;
constexpr uint8_t    openAngle   = 90;
constexpr adk::Color waiting     {0, 0, 40};              // dim blue
constexpr adk::Color alone       {40, 20, 0};             // dim orange

adk::Vector<char, 4> typed;             // the digits heard so far
long                 presses  = -1;     // -1 until Board A's first count
int                  wrong    = 0;      // wrong codes in a row
bool                 unlocked = false;

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    // One more press is a new key. Any other jump means a board restarted
    // or a message was lost: discard the incomplete entry and start again.
    if (bridge.changed ("key"))
    {
        long count = bridge.value ("key");

        if (presses >= 0 && count == presses + 1)
        {
            takeKey (static_cast<char> (bridge.payload ("key")));
        }
        else
        {
            typed.clear ();
        }

        presses = count;
    }

    bridge.share ("ack", presses);    // latest count, not a session proof
    bridge.share ("typed", typed.size ());
    bridge.share ("door", unlocked);
    bridge.share ("wrong", wrong);

    // The light: green while open, red after a wrong code, dim blue while
    // waiting, and dim orange while Board A can't be heard.
    latch.moveTo (unlocked ? openAngle : lockedAngle, 500);
    light.fadeTo (!bridge.isConnected ()          ? alone
                  : unlocked                      ? adk::color::green
                  : wrong > 0 && typed.empty ()   ? adk::color::red
                                                  : waiting, 300);
}

void takeKey (char key)
{
    if (key == '*' || (unlocked && key == '#'))
    {
        unlocked = false;
        typed.clear ();
    }
    else if (key == '#')
    {
        unlocked = adk::equal (typed, code);
        wrong    = unlocked ? 0 : wrong + 1;
        typed.clear ();
    }
    else if (key >= '0' && key <= '9' && !unlocked && !typed.full ())
    {
        typed.push_back (key);
    }
}
