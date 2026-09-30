// Lesson 79: Send a Byte Down a Wire
// Serial1 sends 'A' through a resistor to its own receiver.
// USB Serial reports it.

#include <Adk.h>

struct SerialCable : adk::Object
{
    void setup () override
    {
        if (adk::claimSerial (Serial1))
        {
            Serial1.begin (9600);
        }
    }
};

SerialCable cable;
adk::Every  sendEvery {2000};
bool        sent = false;
bool        heard = false;

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
    adk::println (Serial, "Serial1 loopback: TX1 pin 18 to RX1 pin 19");
}

void loop ()
{
    adk::update ();

    while (Serial1.available () > 0)
    {
        char received = static_cast<char> (Serial1.read ());
        adk::println (Serial, "Received: ", received);
        heard = true;
    }

    if (sendEvery.ticked ())
    {
        if (sent && !heard)
        {
            adk::println (Serial, "No byte returned");
        }

        Serial1.write ('A');
        adk::println (Serial, "Sent: A");
        sent = true;
        heard = false;
    }
}
