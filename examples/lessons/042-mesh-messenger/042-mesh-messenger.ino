// Lesson 42: Mesh Messenger
// Texts from a phone reach the Mega through two Meshtastic boards. "lamp
// on" and "lamp off" switch the lamp, "temp?" gets the DHT11's reading
// back, and anything else shows on the screen with who sent it. The
// button says hello to everyone on the channel.

#include <Adk.h>

#include <string.h>

adk::MeshNode node   {Serial1};
adk::Lcd      lcd    {31, 32, 33, 34, 35, 36};
adk::RgbLed   lamp   {5, 6, 7};
adk::Dht11    dht    {16};
adk::Button   button {23};
adk::Every    scroll {300};             // a long message, along the bottom row

adk::Text<adk::MeshNode::MaxLength> message;    // the message on the screen
size_t                              first = 0;  // its letter at the left

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);

    lcd.print ("Mesh Messenger");
    lcd.at (0, 1).print ("Waiting...");
}

void loop ()
{
    adk::update ();

    if (node.wasReceived ())
    {
        adk::println (Serial, node.sender (), ": ", node.text ());
        obey (node.sender (), node.text ());
    }

    if (button.wasPressed ())
    {
        reply ("Hello from the Mega");
    }

    if (scroll.ticked () && message.size () > 16)
    {
        first = (first + 1) % (message.size () + 3);
        showBottomRow ();
    }
}

// A command is the whole message, in capitals or not, as a phone often
// starts a message with a capital: "Lamp on" works too.
void obey (const char* sender, const char* text)
{
    if (strcasecmp (text, "lamp on") == 0)
    {
        lamp.fadeTo (adk::color::white, 500);
        reply ("The lamp is on");
    }
    else if (strcasecmp (text, "lamp off") == 0)
    {
        lamp.fadeTo (adk::color::off, 500);
        reply ("The lamp is off");
    }
    else if (strcasecmp (text, "temp?") == 0)
    {
        sendWeather ();
    }
    else if (strcasecmp (text, "help") == 0)
    {
        reply ("Try lamp on, lamp off or temp?");
    }
    else
    {
        show (sender, text);
    }
}

void sendWeather ()
{
    adk::Text<40> weather;

    if (dht.ok ())
    {
        adk::print (weather, "It's ", adk::fixed (dht.temperature (), 0),
                    "C and ", adk::fixed (dht.humidity (), 0), "% humid");
    }
    else
    {
        adk::print (weather, "No reading yet");
    }

    reply (weather.c_str ());
}

// To everyone on the channel. The node takes a message at most every
// 1.5 s, and turns down one that comes sooner.
void reply (const char* text)
{
    if (node.send (text))
    {
        adk::println (Serial, "Mega: ", text);
        show ("Mega", text);
    }
    else
    {
        adk::println (Serial, "Too soon to send: ", text);
    }
}

// Who, on the top row, and what they said below it.
void show (const char* sender, const char* text)
{
    message.clear ();
    adk::print (message, text);
    first = 0;

    lcd.clear ();
    adk::print (lcd, sender, " says:");
    showBottomRow ();
}

// Sixteen letters of the message, from the one at first. A long one goes
// round to its start again after a gap of three spaces.
void showBottomRow ()
{
    size_t length = message.size ();

    lcd.at (0, 1);

    for (size_t column = 0; column < 16; ++column)
    {
        size_t at = first + column;

        if (length > 16)
        {
            at = at % (length + 3);
        }

        lcd.print (at < length ? message.c_str ()[at] : ' ');
    }
}
