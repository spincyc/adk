// Runs a sketch's setup () and then its loop () for a few hundred passes on
// the host, against the fake core, while time moves on and the inputs change
// now and then, as buttons, knobs and sensors would. The Makefile builds it
// with the sanitizers, linked with the sketch as Arduino preprocessed it, so
// a crash, undefined behavior or a sketch that never comes back fails. It
// checks nothing a lesson is for; that is its own test's business.

#include <Adk.h>
#include <Arduino.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

void setup ();
void loop ();

namespace {

    constexpr int           Passes    = 400;
    constexpr unsigned long PassMs    = 10;    // between the starts of two passes
    constexpr int           Change    = 50;    // passes between changes of the inputs
    constexpr unsigned      HungAfter = 30;    // seconds of the host's own time

    const char* stage = "setup ()";

    struct StandardError : Print
    {
        size_t write (uint8_t byte) override
        {
            return fputc (byte, stderr) == EOF ? 0 : 1;
        }
    };

    // Only what is safe in a signal handler.
    void hung (int)
    {
        const char said [] = "smoke: never came back from ";

        ::write (STDERR_FILENO, said, sizeof said - 1);
        ::write (STDERR_FILENO, stage, strlen (stage));
        ::write (STDERR_FILENO, "\n", 1);
        _exit (1);
    }

    // A sketch's globals share the C library's names, and one may be
    // called alarm or signal, so set the clock with calls none would be.
    void giveUpAfter (unsigned seconds)
    {
        struct sigaction action {};
        action.sa_handler = hung;
        sigaction (SIGALRM, &action, nullptr);

        itimerval timer {};
        timer.it_value.tv_sec = seconds;
        setitimer (ITIMER_REAL, &timer, nullptr);
    }

    // Every digital input flips, and the analog inputs step through low,
    // middle, high and between, so buttons are pressed and let go and
    // knobs turn, wherever the sketch has them.
    void changeInputs (int round)
    {
        static constexpr int levels [] = {0, 512, 1023, 300, 700};

        for (adk::Pin pin = 0; pin < NUM_DIGITAL_PINS; ++pin)
        {
            arduino::pin (pin).input = round % 2 == 0 ? HIGH : LOW;
        }

        for (uint8_t channel = 0; channel < NUM_ANALOG_INPUTS; ++channel)
        {
            arduino::pin (static_cast<uint8_t> (A0 + channel)).analog
                = levels[(round + channel) % 5];
        }
    }
}

namespace adk {

    void halt (Fault fault, Pin pin)
    {
        StandardError error;

        explain (error, fault, pin);
        exit (1);
    }
}

int main ()
{
    giveUpAfter (HungAfter);

    // A sketch may wait in setup () or loop (), for a sensor or a glide,
    // and a part may time a pin by polling it, so let time pass as they run.
    arduino::setClockStep (100);
    arduino::setCallCost (1);
    setup ();

    stage = "loop ()";

    for (int pass = 0; pass < Passes; ++pass)
    {
        if (pass % Change == Change - 1)
        {
            changeInputs (pass / Change);
        }

        arduino::advance (PassMs);
        loop ();
    }
}
