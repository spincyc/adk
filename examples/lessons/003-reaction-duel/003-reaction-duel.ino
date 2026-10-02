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
    adk::println (Serial, "Reaction Duel! Press a button to start.");
}

void loop ()
{
    adk::update ();

    if (suspense.expired ())
    {
        go ();
    }

    // One press a pass: a round ends with a wait, and a press left over
    // from it must not start the next round.
    if (redButton.wasPressed ())
    {
        redPressed ();
    }
    else if (greenButton.wasPressed ())
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
            adk::println (Serial, "Red pressed too soon, so Green wins!");
            green.blink (200);
            endRound ();
            break;

        case State::Go:
            // Nobody reacts in under 100 ms. A press that quick is a
            // guess, or began before the light and showed late, as a
            // button takes 20 ms to settle: a false start too.
            if (reaction.elapsed () < 100)
            {
                falseStart ();
                adk::println (Serial, "Red pressed too soon, so Green wins!");
                green.blink (200);
            }
            else
            {
                adk::println (Serial, "Red wins in ", reaction.elapsed (),
                              " ms!");
                red.blink (200);
            }
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
            adk::println (Serial, "Green pressed too soon, so Red wins!");
            red.blink (200);
            endRound ();
            break;

        case State::Go:
            if (reaction.elapsed () < 100)
            {
                falseStart ();
                adk::println (Serial, "Green pressed too soon, so Red wins!");
                red.blink (200);
            }
            else
            {
                adk::println (Serial, "Green wins in ", reaction.elapsed (),
                              " ms!");
                green.blink (200);
            }
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
    adk::println (Serial, "Get ready...");
}

void go ()
{
    yellow.on ();
    buzzer.beep (200);
    reaction.restart ();
    state = State::Go;
}

// A long, cross buzz, and the light's countdown is called off.
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
    adk::println (Serial, "Press a button to play again.");
}
