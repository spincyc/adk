// Lesson 03: Reaction Duel
// Wait for the light and beep, then press first. Too soon and you lose.

#include <Adk.h>

// Each player has a name, a button to press and a light that shows when
// they win.
struct Player
{
    const char* name;
    adk::Button button;
    adk::Led    light;
};

Player      red    {"Red",   22, 26};
Player      green  {"Green", 23, 28};
adk::Led    yellow {27};
adk::Buzzer buzzer {12};

enum class State { Waiting, Ready, Go };

State          state = State::Waiting;
adk::Timer     suspense;    // the random wait before the light
adk::Stopwatch reaction;    // from the light to the first press

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
    randomSeed (analogRead (A7));

    yellow.blink (1000);
    Serial.println ("Reaction Duel! Press a button to start.");
}

void loop ()
{
    adk::update ();

    if (suspense.expired ())
    {
        go ();
    }

    if (red.button.wasPressed ())
    {
        pressed (red, green);
    }

    if (green.button.wasPressed ())
    {
        pressed (green, red);
    }
}

// What a press means depends on the state of the game.
void pressed (Player& player, Player& rival)
{
    switch (state)
    {
        case State::Waiting: getReady ();                break;
        case State::Ready:   falseStart (player, rival); break;
        case State::Go:      win (player);               break;
    }
}

void getReady ()
{
    yellow.off ();
    red.light.off ();
    green.light.off ();

    suspense.start (random (2000, 5000));
    state = State::Ready;
    Serial.println ("Get ready...");
}

void go ()
{
    yellow.on ();
    buzzer.beep (200);
    reaction.restart ();
    state = State::Go;
}

void win (Player& winner)
{
    adk::println (Serial, winner.name, " wins in ", reaction.elapsed (),
                  " ms!");
    celebrate (winner);
}

void falseStart (Player& early, Player& winner)
{
    suspense.stop ();
    buzzer.beep (800);
    adk::println (Serial, early.name, " pressed too soon, so ",
                  winner.name, " wins!");
    celebrate (winner);
}

// The winner's light flashes until the next round starts.
void celebrate (Player& winner)
{
    yellow.off ();
    winner.light.blink (200);
    state = State::Waiting;

    // Let the loser's late press go by before a new round can start.
    adk::wait (1000);
    Serial.println ("Press a button to play again.");
}
