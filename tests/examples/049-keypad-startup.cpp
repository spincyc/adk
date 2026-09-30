#include "radio_fixture.h"

void showAnswer ();

#include "../../examples/lessons/049-remote-keypad/Door/Door.ino"

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    hear ("@");
    expect (!readyForKeys, "radio heartbeat does not acknowledge reset");
    hear ("@ack=7");
    expect (!readyForKeys, "an old entry count cannot start a new session");
    runFor (120);
    expect (Serial3.text.find ("@key=0:0") != std::string::npos,
            "sender keeps sharing its zero baseline while waiting");
    hear ("@ack=0");
    expect (readyForKeys, "receiver acknowledgement allows the first key");
    return result ();
}
