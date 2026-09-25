#pragma once

#include <Arduino.h>

namespace adk {

    // Print several things in a row, to Serial or anything else that prints,
    // such as an Lcd. Each prints as Serial.print () would print it alone.
    void print (Print& out, const auto&... parts)
    {
        (out.print (parts), ...);
    }

    // The same, then end the line.
    void println (Print& out, const auto&... parts)
    {
        print (out, parts...);
        out.println ();
    }
}
