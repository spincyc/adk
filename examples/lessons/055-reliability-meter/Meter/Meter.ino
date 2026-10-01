// Lesson 55: Reliability Meter, Board A
// Click the stick to send 64 numbered messages to Board B, which sends
// each one straight back. A dot lights on the matrix for every message
// that returns, and the screen says what share came back and how long one
// took there and back. Push the stick left or right to change their length.

#include <Adk.h>

// Quick or Far: the same on both boards, or they can't hear each other.
constexpr adk::LoraSpeed speed = adk::LoraSpeed::Quick;

adk::LoraModem radio    {Serial3, 1, {.partner = 2,
                                      .speed   = speed,
                                      .power   = 0}};    // its lowest
adk::LedMatrix matrix   {47, 48, 49};
adk::Joystick  joystick {A3, A4};
adk::Button    stick    {22};
adk::Speaker   speaker  {10};
adk::Lcd       lcd      {31, 32, 33, 34, 35, 36};
adk::Stopwatch trip;                  // one message, there and back
adk::Timer     giveUp;                // for an echo that never comes
adk::Every     nudge    {250};        // a held stick changes the length

constexpr int tries = 64;             // one for each dot of the matrix

int         length  = 20;             // letters in each message, 5 to 60
int         sent    = 0;              // messages sent in this test
int         heard   = 0;              // echoes that came back whole
adk::Millis back    = 0;              // how long the latest echo took
bool        testing = false;

adk::Text<60> message;                // the message that is out now

void setup ()
{
    adk::setup ();
    showSettings (radio.ok () ? "Click to test" : "No modem reply");
}

void loop ()
{
    adk::update ();

    if (!testing && nudge.ticked () && joystick.x () / 60 != 0)
    {
        length = constrain (length + joystick.x () / 60 * 5, 5, 60);
        showSettings ("Click to test");
    }

    if (!testing && stick.wasPressed ())
    {
        testing = true;
        sent    = 0;
        heard   = 0;
        matrix.clear ();
    }

    // The message that is out came back, word for word: light its dot.
    if (testing && radio.wasReceived () && message == radio.text ())
    {
        back = trip.elapsed ();
        giveUp.stop ();
        ++heard;
        matrix.set ((sent - 1) % 8, (sent - 1) / 8);
    }

    // Nothing is out, or the meter has given up on it: on to the next.
    if (testing && !giveUp.isRunning ())
    {
        if (sent < tries)
        {
            sendNext ();
        }
        else
        {
            finish ();
        }
    }
}

// A numbered message, 1 to 64, filled out with letters to its length, so
// each one is different and exactly as long as the stick chose.
void sendNext ()
{
    message.clear ();
    adk::print (message, sent + 1, ' ');

    while (int (message.size ()) < length)
    {
        adk::print (message, char ('a' + message.size () % 26));
    }

    if (radio.send (message.c_str ()))
    {
        ++sent;
        trip.restart ();
        giveUp.start (2 * airTime () + 500);    // there and back, and spare
        adk::print (lcd.at (0, 1), "Sent ", sent, " heard ", heard, "  ");
    }
}

// About how long one message of this length takes on the air: the toll,
// and then so much for each letter.
adk::Millis airTime ()
{
    if (speed == adk::LoraSpeed::Quick)
    {
        return 20 + length * 3 / 2;
    }

    return 200 + length * 8;
}

// The share that came back, and how long the latest took there and back.
// A high beep if every one came back, a low one if any were lost.
void finish ()
{
    testing = false;
    adk::Text<16> result;

    if (heard == 0)
    {
        adk::print (result, "None came back");
    }
    else
    {
        adk::print (result, heard * 100 / tries, "% back ", back, "ms");
    }

    showSettings (result.c_str ());
    speaker.tone (heard == tries ? adk::note::c6 : adk::note::c4, 200);
}

// The speed and the length on top; below, what to do or what happened.
void showSettings (const char* below)
{
    lcd.clear ();
    adk::print (lcd, speed == adk::LoraSpeed::Quick ? "Quick " : "Far ",
                length, " letters");
    adk::print (lcd.at (0, 1), below);
}
