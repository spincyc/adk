// Lesson 38: Radio Messages
// Press the button, or type a line in the Serial Monitor, and the
// transmitter sends it at 433 MHz. Every message the receiver hears shows
// on the LCD, with a count. After each message the transmitter rests for
// at least 10 seconds, because the law lets it send only now and then.

#include <Adk.h>

adk::Lcd              lcd         {31, 32, 33, 34, 35, 36};
adk::RadioReceiver    receiver    {43};
adk::RadioTransmitter transmitter {46};
adk::Button           button      {23};
adk::LineReader<60>   typed;          // a line from the Serial Monitor

// What the button sends, one after another.
constexpr adk::Array messages {"Hello!", "Are you there?", "Dinner's ready",
                               "Good night"};

int next  = 0;    // the message the button sends next
int heard = 0;    // how many messages have arrived

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
    lcd.print ("Listening...");
}

void loop ()
{
    adk::update ();

    if (button.wasPressed () && send (messages[next]))
    {
        next = (next + 1) % messages.size ();
    }

    if (typed.read (Serial))
    {
        send (typed.line ());
    }

    if (receiver.wasReceived ())
    {
        showMessage (receiver.text ());
    }
}

// Send a message, and say whether it went. The transmitter turns one down
// while the last is going out and during the rest after it, so then say
// how long until it can send again, in whole seconds, rounded up.
bool send (const char* text)
{
    if (transmitter.send (text))
    {
        adk::println (Serial, "Sent: ", text);
        return true;
    }

    long wait = (transmitter.restLeft () + 999) / 1000;

    lcd.clear ();
    lcd.print ("Resting...");
    adk::print (lcd.at (0, 1), "Wait ", wait, " s");
    adk::println (Serial, "Resting: send again in ", wait, " s");
    return false;
}

// The count on the top row and the message below it. The screen has room
// for its first 16 letters; the Serial Monitor shows it all.
void showMessage (const char* text)
{
    adk::Text<16> start;
    adk::print (start, text);

    ++heard;
    lcd.clear ();
    adk::print (lcd, "Message ", heard, ':');
    adk::print (lcd.at (0, 1), start.c_str ());
    adk::println (Serial, "Heard: ", text);
}
