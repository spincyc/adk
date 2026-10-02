#include "radio_fixture.h"

#include <string>

void        sendNext ();
adk::Millis airTime ();
void        finish ();
void        showSettings (const char* below);

#include "../../examples/lessons/055-reliability-meter/Meter/Meter.ino"

void click ()
{
    arduino::drive (22, LOW);
    lesson_test::runFor (1);
    lesson_test::runFor (25);
    arduino::drive (22, HIGH);
    lesson_test::runFor (25);
}

// Board B's echo of the message that is out now.
void echo ()
{
    std::string out = message.c_str ();
    lesson_test::hear (out.c_str ());
}

int main ()
{
    using namespace lesson_test;
    prepare ();
    setup ();
    expect (Serial3.text.find ("AT+CRFOP=0\r\n") != std::string::npos,
            "the meter's modem sends at its lowest power");
    expect (Serial3.text.find ("AT+PARAMETER=7,7,1,4\r\n") != std::string::npos,
            "the meter starts at Quick");

    // A stick held right for a third of a second adds five letters, once.
    arduino::pin (A3).analog = 1023;
    for (int ms = 0; ms < 330; ms += 10)
    {
        runFor (10);
    }
    arduino::pin (A3).analog = 512;
    runFor (300);
    expect (length == 25, "holding the stick right adds five letters");
    length = 20;

    Serial3.text.clear ();
    click ();
    expect (testing && sent == 1 && message.size () == 20, "a click sends message 1");
    expect (Serial3.text.find ("AT+SEND=2,20,1 1 efghijklmnopqrst\r\n") != std::string::npos,
            "message 1 goes to Board B, numbered and filled to its length");

    echo ();
    expect (heard == 1 && matrix.get (0, 0) && back >= 100 && back <= 150,
            "its echo lights dot 1, with its time there and back");
    expect (sent == 2, "message 2 follows at once");

    // Message 2 is lost: after 2 * airTime () + 500 = 600 ms the meter
    // moves on, and its late echo is not counted as message 3.
    std::string two = message.c_str ();
    for (int ms = 0; ms < 550; ms += 10)
    {
        runFor (10);
    }
    expect (sent == 2, "the meter still waits for echo 2");
    for (int ms = 0; ms < 100; ms += 10)
    {
        runFor (10);
    }
    expect (sent == 3, "it gives up on message 2 and sends 3");
    hear (two.c_str ());
    expect (heard == 1 && !matrix.get (1, 0), "a late echo is not counted");

    while (sent < 64)
    {
        echo ();
    }

    // The last packet's echo arrives only when the next run starts.
    std::string oldFinalEcho = message.c_str ();
    while (testing)
    {
        runFor (10);
    }
    expect (sent == 64 && heard == 62, "the old run lost messages 2 and 64");
    expect (!matrix.get (1, 0) && !matrix.get (7, 7), "both losses leave gaps");
    expect (arduino::pin (10).tone == adk::note::c4, "a low beep for a loss");

    arduino::drive (22, LOW);
    runFor (1);
    Serial3.input += "+RCV=1," + std::to_string (oldFinalEcho.size ())
                   + ',' + oldFinalEcho + ",-40,9\r\n";
    runFor (25);
    expect (stick.wasPressed () && radio.wasReceived (),
            "the old echo and new-test click arrive in the same update");
    expect (heard == 0, "the start update rejects the previous run's retained message");
    arduino::drive (22, HIGH);
    runFor (25);
    expect (testing && sent == 1 && !matrix.get (7, 7), "a new test clears the matrix");
    hear ("1 1 efghijklmnopqrst");
    expect (heard == 0 && !matrix.get (0, 0), "a previous test's echo is rejected");
    expect (run == 2, "the next test has its own identity");
    while (testing)
    {
        echo ();
    }
    expect (heard == 64 && matrix.get (7, 7), "the new run counts its own 64 echoes");
    expect (arduino::pin (10).tone == adk::note::c6, "a high beep for a complete run");
    run = 4294967294UL;
    length = 15;
    click ();
    expect (message.size () == 15 && message == "4294967295 1 no",
            "the minimum length fits a full-width test number");
    return result ();
}
