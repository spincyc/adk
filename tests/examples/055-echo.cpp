#include "radio_fixture.h"

#include <string>

void showHeard (const char* text);

#include "../../examples/lessons/055-reliability-meter/Echo/Echo.ino"

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    Serial3.text.clear ();
    hear ("1 1 efghijklmno");
    expect (Serial3.text.find ("AT+SEND=1,15,1 1 efghijklmno\r\n") != std::string::npos,
            "each message goes straight back to Board A");
    expect (matrix.get (0, 0), "message 1 lights the first dot");
    hear ("1 17 fghijklmno");
    hear ("1 64 fghijklmno");
    expect (matrix.get (0, 2) && matrix.get (7, 7), "17 and 64 light their own dots");
    hear ("2 2 efghijklmno");
    expect (!matrix.get (0, 0) && !matrix.get (7, 7) && matrix.get (1, 0),
            "a descending boundary starts a new test");
    hear ("2 1 efghijklmno");
    expect (matrix.get (0, 0) && matrix.get (1, 0), "reordering within a run keeps its dots");
    hear ("3 17 fghijklmno");
    expect (!matrix.get (0, 0) && !matrix.get (1, 0) && matrix.get (0, 2),
            "lost early packets still identify the new test");
    hear ("3 18 fghijklmno");
    hear ("4 18 fghijklmno");
    expect (!matrix.get (0, 2) && matrix.get (1, 2),
            "equal message numbers at the boundary still clear old dots");
    hear ("hello");
    expect (matrix.get (1, 2), "malformed traffic cannot erase the current run");
    return result ();
}
