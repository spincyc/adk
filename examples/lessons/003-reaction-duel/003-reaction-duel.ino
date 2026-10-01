// Lesson 03: Reaction Duel
// Wait for the light and beep, then press first. Too soon and you lose.

#include <Adk.h>

adk::Button redButton   {22};
adk::Button greenButton {23};
adk::Led    red         {26};
adk::Led    yellow      {27};
adk::Led    green       {28};
adk::Buzzer buzzer      {12};

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

    if (redButton.wasPressed ())
    {
        redPressed ();
    }

    if (greenButton.wasPressed ())
    {
        greenPressed ();
    }
}

// What a press means depends on the state of the game.
void redPressed ()
{
    switch (state)
    {
        case State::Waiting:
            getReady ();
            break;

        case State::Ready:
            falseStart ();
            Serial.println ("Red pressed too soon, so Green wins!");
            green.blink (200);
            endRound ();
            break;

        case State::Go:
            adk::println (Serial, "Red wins in ", reaction.elapsed (), " ms!");
            red.blink (200);
            endRound ();
            break;
    }
}

// The same, with the colors the other way round.
void greenPressed ()
{
    switch (state)
    {
        case State::Waiting:
            getReady ();
            break;

        case State::Ready:
            falseStart ();
            Serial.println ("Green pressed too soon, so Red wins!");
            red.blink (200);
            endRound ();
            break;

        case State::Go:
            adk::println (Serial, "Green wins in ", reaction.elapsed (),
                          " ms!");
            green.blink (200);
            endRound ();
            break;
    }
}

void getReady ()
{
    yellow.off ();
    red.off ();
    green.off ();

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

// A long, cross buzz, and the light never comes on.
void falseStart ()
{
    suspense.stop ();
    buzzer.beep (800);
}

// The winner's light keeps flashing until the next round starts.
void endRound ()
{
    yellow.off ();
    state = State::Waiting;

    // Let the loser's late press go by before a new round can start.
    adk::wait (1000);
    Serial.println ("Press a button to play again.");
}
