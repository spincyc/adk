#pragma once

#include <Adk.h>

#include <stdio.h>
#include <string>

void loop ();

namespace lesson_test {

    inline int failures = 0;

    inline void expect (bool condition, const char* description)
    {
        if (!condition)
        {
            printf ("FAIL: %s\n", description);
            ++failures;
        }
    }

    inline void prepare ()
    {
        arduino::reset ();
        arduino::pin (A3).analog = 512;
        arduino::pin (A4).analog = 512;
        Serial3.onWrite = [] (uint8_t byte)
        {
            if (byte == '\n')
            {
                Serial3.input += "+OK\r\n";
            }
        };
    }

    inline void runFor (unsigned long ms)
    {
        arduino::advance (ms);
        loop ();
    }

    // A complete radio packet; omit a call to model a lost packet.
    inline void hear (const char* text)
    {
        Serial3.input += "+RCV=1," + std::to_string (strlen (text))
                       + ',' + text + ",-40,9\r\n";
        runFor (120);
    }

    inline int result ()
    {
        printf ("Lesson regression: %d failures\n", failures);
        return failures == 0 ? 0 : 1;
    }
}
