#include "radio_fixture.h"

void showAnswer ();

#include "../../examples/lessons/049-remote-keypad/Door/Door.ino"

namespace {

    // Plays the keypad's 5: its column, pin 27, reads low while its row,
    // pin 23, is driven low.
    bool fiveHeld = false;

    void pressFive ()
    {
        fiveHeld = true;
        for (int step = 0; step < 10; ++step)
        {
            lesson_test::runFor (10);
        }
        fiveHeld = false;
        for (int step = 0; step < 10; ++step)
        {
            lesson_test::runFor (10);
        }
    }
}

int main ()
{
    using namespace lesson_test;
    prepare ();
    arduino::onDigitalRead = [] (uint8_t pin)
    {
        bool joined = fiveHeld && pin == 27 && arduino::pin (23).mode == OUTPUT;
        return joined ? LOW : HIGH;
    };
    setup ();
    pressFive ();
    expect (presses == 0, "a key pressed while Board B can't hear goes nowhere");
    hear ("@1/0 typed=0 door=0 wrong=0");
    expect (bridge.isConnected (), "Board B is heard");
    pressFive ();
    expect (presses == 1 && lastKey == '5', "now the key counts");
    runFor (120);
    expect (Serial3.text.find ("@1/1 key=1:53") != std::string::npos,
            "the key crosses with its count");
    pressFive ();
    runFor (120);
    expect (Serial3.text.find ("@1/1 key=2:53") != std::string::npos,
            "the same key again goes with the next count");
    return result ();
}
