// Lesson 40: LoRa
// Two LoRa modems on one Mega. Press the button, or type a line in the
// Serial Monitor, and modem A sends it; modem B hears it, and the LCD
// shows it with how strong it arrived.

#include <Adk.h>

adk::LoraModem      modemA {Serial1, 1};    // address 1
adk::LoraModem      modemB {Serial3, 2};    // address 2
adk::Lcd            lcd    {31, 32, 33, 34, 35, 36};
adk::Button         button {23};
adk::LineReader<60> typed;                  // a line from the Serial Monitor

int presses = 0;

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);

    lcd.print (modemA.ok () ? "Modem A is ready" : "No reply from A");
    lcd.at (0, 1).print (modemB.ok () ? "Modem B is ready" : "No reply from B");
}

void loop ()
{
    adk::update ();

    if (button.wasPressed ())
    {
        adk::Text<16> message;
        ++presses;
        adk::print (message, "Press ", presses);
        sendFromA (message.c_str ());
    }

    if (typed.read (Serial))
    {
        sendFromA (typed.line ());
    }

    if (modemB.wasReceived ())
    {
        showMessage ();
    }
}

// To address 2, modem B. A turns a message down while it is still sending
// the last one, or if it didn't answer at setup.
void sendFromA (const char* text)
{
    if (modemA.send (2, text))
    {
        adk::println (Serial, "A sends: ", text);
    }
    else
    {
        adk::println (Serial, "A can't send just now: try again");
    }
}

// The message on the top row, the signal and the margin below it.
void showMessage ()
{
    lcd.clear ();
    adk::print (lcd.at (0, 0), modemB.text ());
    adk::print (lcd.at (0, 1), modemB.signal (), " dBm  ",
                modemB.margin (), " dB");

    adk::println (Serial, "B hears from ", modemB.sender (), ": ",
                  modemB.text ());
    adk::println (Serial, "  signal ", modemB.signal (), " dBm, margin ",
                  modemB.margin (), " dB");
}
