#include <Adk.h>

#include <cstdio>

// The Arduino builder supplies these declarations before compiling a sketch.
void redPressed ();
void greenPressed ();
void getReady ();
void go ();
void falseStart ();
void endRound ();

#include "../../examples/lessons/003-reaction-duel/003-reaction-duel.ino"

namespace {

    constexpr uint8_t       redPin   = 22;
    constexpr uint8_t       greenPin = 23;
    constexpr unsigned long never    = ~0UL;

    bool          armed     = false;    // once the round has reached Go
    unsigned long roundOver = never;    // when it went back to Waiting
    unsigned long greenLate = 0;        // how long after that green goes down

    unsigned long nowMs ()
    {
        return arduino::now () / 1000;
    }

    // Green's button goes down greenLate ms after the round ends, and stays
    // down. The round ends inside red's loop (), as endRound () starts its
    // adk::wait (1000), so the first read that sees Waiting marks it.
    int readPin (uint8_t pin)
    {
        if (pin == greenPin)
        {
            if (armed && roundOver == never && state == State::Waiting)
            {
                roundOver = nowMs ();
            }

            if (roundOver != never && nowMs () >= roundOver + greenLate)
            {
                return LOW;
            }
        }

        return arduino::pin (pin).input;
    }

    void runUntil (State wanted, unsigned long limit)
    {
        for (unsigned long ms = 0; ms < limit && state != wanted; ++ms)
        {
            arduino::advance (1);
            loop ();
        }
    }

    // Red starts a round and wins it, then green goes down late ms after
    // the round ends. The state a second after red's winning loop ().
    State play (unsigned long late)
    {
        arduino::reset ();
        arduino::onDigitalRead = readPin;
        armed     = false;
        roundOver = never;
        greenLate = late;
        setup ();

        arduino::pin (redPin).input = LOW;
        runUntil (State::Ready, 100);
        arduino::pin (redPin).input = HIGH;
        runUntil (State::Go, 6000);

        armed = true;
        arduino::pin (redPin).input = LOW;
        runUntil (State::Waiting, 100);
        runUntil (State::Ready, 1000);
        return state;
    }
}

int main ()
{
    int failures = 0;

    // A press of green during endRound ()'s wait, even one that the
    // debouncer accepts in the wait's very last update, 980 ms in, must
    // not start the next round.
    for (unsigned long late = 0; late <= 980; late += 5)
    {
        if (play (late) != State::Waiting)
        {
            printf ("Green down %lu ms into the wait started a round\n", late);
            failures++;
        }
    }

    // One after the wait still does.
    if (play (1100) != State::Ready)
    {
        printf ("Green down after the wait didn't start a round\n");
        failures++;
    }

    printf ("Reaction duel lesson: 198 checks, %d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
