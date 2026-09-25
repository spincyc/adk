#include "check.h"

#include <Adk.h>
#include <Arduino.h>

#include <stdio.h>
#include <string.h>

namespace adk {

    void halt (Fault fault, Pin pin)
    {
        check::halted = {true, fault, pin};
    }
}

namespace check {

    Halt halted = {false, adk::Fault::None, 0};

    namespace {

        Case*       first    = nullptr;
        Case*       last     = nullptr;
        const char* running  = "";
        int         failures = 0;
    }

    Case::Case (const char* name, void (*body) ())
        : name_ (name)
        , body_ (body)
        , next_ (nullptr)
    {
        (last ? last->next_ : first) = this;
        last = this;
    }

    void fail (const char* file, int line, const char* expression)
    {
        printf ("%s:%d: %s: CHECK (%s) failed\n", file, line, running, expression);
        ++failures;
    }
}

// Run every case whose name contains the first argument, or all of them.
int main (int argumentCount, char** arguments)
{
    const char* filter = argumentCount > 1 ? arguments[1] : "";
    int         cases  = 0;

    for (check::Case* test = check::first; test; test = test->next_)
    {
        if (!strstr (test->name_, filter))
        {
            continue;
        }

        arduino::reset ();
        adk::releaseClaims ();
        check::halted = {false, adk::Fault::None, 0};

        check::running = test->name_;
        test->body_ ();
        ++cases;
    }

    printf ("%d cases, %d failures\n", cases, check::failures);
    return check::failures == 0 ? 0 : 1;
}
