// Lesson 02: Buttons
// Tap to switch the red LED, hold to light the yellow; count every tap.

#include <Adk.h>

adk::Button leftButton  {22};
adk::Button rightButton {23};
adk::Led    red         {26};
adk::Led    yellow      {27};

int presses = 0;

void setup ()
{
    Serial.begin (9600);
    adk::setup (Serial);
}

void loop ()
{
    adk::update ();

    if (leftButton.wasPressed ())
    {
        red.toggle ();
        countPress ();
    }

    yellow.set (rightButton.isPressed ());
}

void countPress ()
{
    presses = presses + 1;

    Serial.print ("Presses: ");
    Serial.println (presses);
}
