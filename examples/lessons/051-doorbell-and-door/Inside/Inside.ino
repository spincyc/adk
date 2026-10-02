// Lesson 51: Doorbell and Door, Board A (inside)
// Board B, at the door, counts rings, knocks and cards. This board says
// who's there and chimes, and it opens the latch for a friend's card, or
// when you press the button to let someone in.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 1, {.partner = 2,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};
adk::Lcd       lcd    {31, 32, 33, 34, 35, 36};
adk::Button    button {23};
adk::Speaker   chime  {10};
adk::Servo     latch  {44};

// A friend is a card's number, from the Serial Monitor, and its owner.
struct Friend
{
    uint32_t    card;
    const char* name;
};

constexpr adk::Array friends
{
    Friend {0x12345678, "Ada"},
    Friend {0x9ABCDEF0, "Sam"}
};

constexpr adk::Note dingDong [] {{adk::note::e5, 500}, {adk::note::c5, 800}};
constexpr adk::Note knock    [] {{adk::note::g3, 60}};
constexpr adk::Note welcome  [] {{adk::note::c5, 120}, {adk::note::e5, 120},
                                 {adk::note::g5, 240}};
constexpr adk::Note stranger [] {{adk::note::c3, 500}};

constexpr uint8_t     lockedAngle = 0;
constexpr uint8_t     openAngle   = 90;
constexpr adk::Millis openTime    = 5000;
constexpr adk::Millis newsTime    = 10000;    // how long news stays shown

adk::Timer unlocked;      // runs while the door is open
adk::Timer news;          // runs while the screen shows news
bool       linked = false;    // whether Board B could be heard, as shown

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
    showQuiet ();
}

void loop ()
{
    adk::update ();

    // Each is true once for each new event at the door, for one update.
    // The screen and chime show one at a time, so when several come in one
    // message, a card goes before the bell and the bell before a knock; the
    // rest are let go.
    if (bridge.changed ("cards"))
    {
        checkCard (uint32_t (bridge.payload ("cards")));
    }
    else if (bridge.changed ("rings"))
    {
        tell ("Ding dong!", "Press to let in");
        chime.play (dingDong);
    }
    else if (bridge.changed ("knocks"))
    {
        tell ("Knock knock!", "Press to let in");
        chime.play (knock);
    }

    if (button.wasPressed ())
    {
        tell ("Door open", "Come in!");
        unlocked.start (openTime);
    }

    // Quiet again once the news is old, or when B is lost or found.
    bool linkChanged = bridge.isConnected () != linked;

    if (news.expired () || (linkChanged && !news.isRunning ()))
    {
        showQuiet ();
    }

    latch.moveTo (unlocked.isRunning () ? openAngle : lockedAngle, 500);
    bridge.share ("door", unlocked.isRunning ());
}

void checkCard (uint32_t card)
{
    for (const Friend& person : friends)
    {
        if (person.card == card)
        {
            tell ("Welcome home,", person.name);
            chime.play (welcome);
            unlocked.start (openTime);
            return;
        }
    }

    adk::println (Serial, "Card 0x", adk::hex (card, 8));
    tell ("Unknown card", "at the door");
    chime.play (stranger);
}

// News stays on the screen for ten seconds.
void tell (const char* top, const char* bottom)
{
    lcd.clear ();
    adk::print (lcd.at (0, 0), top);
    adk::print (lcd.at (0, 1), bottom);
    news.start (newsTime);
}

void showQuiet ()
{
    linked = bridge.isConnected ();
    lcd.clear ();
    adk::print (lcd.at (0, 0), "Front door");
    adk::print (lcd.at (0, 1), linked ? "All quiet" : "Can't hear it");
}
