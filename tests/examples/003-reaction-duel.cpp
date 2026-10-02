#include <Adk.h>

#include <cstdio>
#include <string>

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

    void runFor (unsigned long limit)
    {
        for (unsigned long ms = 0; ms < limit; ++ms)
        {
            arduino::advance (1);
            loop ();
        }
    }

    // Red's press starts a round, and the game waits in Ready.
    void startRound (unsigned long late)
    {
        arduino::reset ();
        arduino::onDigitalRead = readPin;
        armed     = false;
        roundOver = never;
        greenLate = late;
        Serial.text.clear ();
        setup ();

        arduino::pin (redPin).input = LOW;
        runUntil (State::Ready, 100);
        arduino::pin (redPin).input = HIGH;
    }

    // Red starts a round and wins it, then green goes down late ms after
    // the round ends. The state a second after red's winning loop ().
    State play (unsigned long late)
    {
        startRound (late);
        runUntil (State::Go, 6000);
        runFor (200);

        armed = true;
        arduino::pin (redPin).input = LOW;
        runUntil (State::Waiting, 100);
        runUntil (State::Ready, 1000);
        return state;
    }

    // How the game called a press: a false start, a win, or neither, when
    // it printed no verdict at all.
    enum class Call { FalseStart, Win, Neither };

    // Red goes down at ms after the light comes on, or before it if at is
    // negative. How the game called it.
    Call callAt (long at)
    {
        startRound (never);

        if (at < 0)
        {
            while (state == State::Ready
                   && suspense.remaining () > static_cast<unsigned long> (-at))
            {
                arduino::advance (1);
                loop ();
            }
        }
        else
        {
            runUntil (State::Go, 6000);

            while (reaction.elapsed () < static_cast<unsigned long> (at))
            {
                arduino::advance (1);
                loop ();
            }
        }

        arduino::pin (redPin).input = LOW;
        runUntil (State::Waiting, 100);
        arduino::pin (redPin).input = HIGH;

        bool tooSoon = Serial.text.find ("Red pressed too soon") != std::string::npos;
        bool won     = Serial.text.find ("Red wins in") != std::string::npos;

        if (tooSoon == won)
        {
            return Call::Neither;
        }

        return tooSoon ? Call::FalseStart : Call::Win;
    }
}

int main ()
{
    int failures = 0;
    int checks   = 0;

    // A press of green during endRound ()'s wait, even one that the
    // debouncer accepts in the wait's very last update, 980 ms in, must
    // not start the next round.
    for (unsigned long late = 0; late <= 980; late += 5)
    {
        checks++;

        if (play (late) != State::Waiting)
        {
            printf ("Green down %lu ms into the wait started a round\n", late);
            failures++;
        }
    }

    // One after the wait still does.
    checks++;

    if (play (1100) != State::Ready)
    {
        printf ("Green down after the wait didn't start a round\n");
        failures++;
    }

    // A press is noticed 20 ms after it begins, once the button settles.
    // One that begins before the light, even in its last 20 ms, loses; so
    // does one that begins under 80 ms after it, noticed under 100 ms: too
    // quick to be a reaction. One after that wins.
    for (long at = -40; at <= 300; ++at)
    {
        if (at > 75 && at < 85)
        {
            continue;
        }

        checks++;
        Call wanted = at < 80 ? Call::FalseStart : Call::Win;
        Call called = callAt (at);

        if (called != wanted)
        {
            printf ("Red down %ld ms from the light %s\n", at,
                    called == Call::Win          ? "won"
                    : called == Call::FalseStart ? "was called a false start"
                                                 : "neither won nor lost");
            failures++;
        }
    }

    printf ("Reaction duel lesson: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
