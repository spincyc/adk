// Lesson 49: Remote Keypad, Board A (the door)
// Every key goes by radio to Board B, inside, which knows the code and
// works the latch. This board only passes keys on and shows B's answer.

#include <Adk.h>

adk::LoraModem radio  {Serial3, 1, {.partner = 2,
                                    .speed   = adk::LoraSpeed::Quick,
                                    .power   = 10}};
adk::Bridge    bridge {radio};
adk::Keypad    keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};
adk::Lcd       lcd    {31, 32, 33, 34, 35, 36};

long presses = 0;        // keys pressed since the sketch started
char lastKey = '\0';     // the latest of them
bool linked  = false;    // whether Board B could be heard, as shown

void setup ()
{
    adk::setup ();
    showAnswer ();
}

void loop ()
{
    adk::update ();

    char key = keypad.key ();

    if (key != '\0')
    {
        ++presses;
        lastKey = key;
    }

    // A press is an event, so it crosses as a count: B takes the key
    // when the count goes up by one.
    bridge.share ("presses", presses);
    bridge.share ("key", lastKey);

    if (bridge.changed ("typed") || bridge.changed ("door")
        || bridge.changed ("wrong") || bridge.isConnected () != linked)
    {
        linked = bridge.isConnected ();
        showAnswer ();
    }
}

// The top row says what Board B decided; the bottom row has a star for
// each digit B has heard.
void showAnswer ()
{
    long wrong = bridge.value ("wrong");

    lcd.clear ();

    if (!linked)
    {
        lcd.print ("Calling B...");
    }
    else if (bridge.value ("door") == 1)
    {
        lcd.print ("Open! # locks");
    }
    else if (wrong > 0 && bridge.value ("typed") == 0)
    {
        adk::print (lcd, "Wrong! Tries: ", wrong);
    }
    else
    {
        lcd.print ("Locked. Code?");
    }

    lcd.at (0, 1);

    for (long star = 0; star < bridge.value ("typed"); ++star)
    {
        lcd.print ('*');
    }
}
