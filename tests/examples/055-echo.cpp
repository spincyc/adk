#include "radio_fixture.h"

#include <string>

void showHeard (int number);

#include "../../examples/lessons/055-reliability-meter/Echo/Echo.ino"

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    Serial3.text.clear ();
    hear ("1 cdefg");
    expect (Serial3.text.find ("AT+SEND=1,7,1 cdefg\r\n") != std::string::npos,
            "each message goes straight back to Board A");
    expect (matrix.get (0, 0), "message 1 lights the first dot");
    hear ("17 defg");
    hear ("64 defg");
    expect (matrix.get (0, 2) && matrix.get (7, 7), "17 and 64 light their own dots");
    hear ("2 cdefg");
    expect (!matrix.get (0, 0) && !matrix.get (7, 7) && matrix.get (1, 0),
            "a lower number starts a new test");
    hear ("hello");
    expect (!matrix.get (1, 0), "a message with no number lights nothing");
    return result ();
}
