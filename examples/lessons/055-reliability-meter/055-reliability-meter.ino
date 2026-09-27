// Lesson 55: Reliability Meter
// Turn the dial to choose how many letters each message carries, and press
// it to send ten such messages through the air. The screen shows how long
// one message takes to send, and how many of the ten came back whole.

#include <Adk.h>

adk::Lcd              lcd         {31, 32, 33, 34, 35, 36};
adk::RadioReceiver    receiver    {43};
adk::RadioTransmitter transmitter {46};
adk::RotaryEncoder    dial        {18, 19};
adk::Button           click       {22};
adk::Stopwatch        onAir;          // how long the latest message took
adk::Timer            pause;          // a breath between messages

constexpr int tries = 10;    // messages in one test: 5 seconds at most

int  length  = 20;       // letters in each message, from 5 to 60
int  sent    = 0;        // messages sent in this test
int  heard   = 0;        // messages that came back whole
bool testing = false;
bool tested  = false;    // a test has run at this length

adk::Text<60> message;   // the message going out now

void setup ()
{
    adk::setup ();
    showLength ();
}

void loop ()
{
    adk::update ();

    if (!testing && dial.turned () != 0)
    {
        length = constrain (length + dial.turned () * 5, 5, 60);
        tested = false;
        showLength ();
    }

    if (!testing && click.wasPressed ())
    {
        testing = true;
        sent    = 0;
        heard   = 0;
        sendMessage ();
    }

    if (receiver.wasReceived () && message == receiver.text ())
    {
        ++heard;
    }

    // The message has gone: wait a moment for the receiver, then go on.
    if (onAir.isRunning () && !transmitter.isSending ())
    {
        onAir.stop ();
        pause.start (50);
    }

    if (pause.expired ())
    {
        if (sent < tries)
        {
            sendMessage ();
        }
        else
        {
            testing = false;
            tested  = true;
            showLength ();
        }
    }
}

// A numbered message, #1 to #10, filled out with letters to its length,
// so each one is different and exactly as long as the dial says.
void sendMessage ()
{
    ++sent;
    message.clear ();
    adk::print (message, '#', sent, ' ');

    while (int (message.size ()) < length)
    {
        adk::print (message, char ('a' + message.size () % 26));
    }

    transmitter.send (message.c_str ());
    onAir.restart ();
    adk::print (lcd.at (0, 1), "Sending ", sent, " of ", tries, "   ");
}

// The length, how long one message took, and the last test's result.
void showLength ()
{
    lcd.clear ();
    adk::print (lcd, length, " letters");

    if (!tested)
    {
        lcd.at (0, 1).print ("Press to test");
        return;
    }

    adk::print (lcd, ' ', onAir.elapsed (), "ms");
    adk::print (lcd.at (0, 1), "Heard ", heard, '/', tries, ' ',
                heard * 100 / tries, '%');
}
