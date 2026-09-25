#pragma once

// The smallest useful test runner. TEST defines a case that runs against a
// freshly reset fake board; CHECK records a failure with its location and
// carries on, so one run reports every broken expectation.

#include <Adk.h>

namespace check {

    // What adk::halt () was last asked to show. Tests replace the halting
    // handler, so a fault can be inspected instead of blinking forever.
    struct Halt
    {
        bool       happened;
        adk::Fault fault;
        adk::Pin   pin;
    };

    extern Halt halted;

    struct Case
    {
        Case (const char* name, void (*body) ());

        const char* name_;
        void      (*body_) ();
        Case*       next_;
    };

    void fail (const char* file, int line, const char* expression);
}

#define TEST(name)                                                             \
    static void           name ();                                             \
    static check::Case    name##Case (#name, name);                            \
    static void           name ()

#define CHECK(expression)                                                      \
    ((expression) ? (void) 0 : check::fail (__FILE__, __LINE__, #expression))
