// Lesson 10: Dice
// Press the button on pin 22: the digit behind the 74HC595 on pins 37, 38 and 39 spins, slows
// down and lands on a number from 1 to 6.

#include <Adk.h>

adk::Button        button {22};
adk::ShiftRegister digit  {37, 38, 39};

// Each face as a byte: bit 0 lights segment a, bit 1 segment b, and so on
// round to bit 6, segment g, across the middle.
const uint8_t faces [] {
    0b00000110,     // 1: b c
    0b01011011,     // 2: a b d e g
    0b01001111,     // 3: a b c d g
    0b01100110,     // 4: b c f g
    0b01101101,     // 5: a c d f g
    0b01111101,     // 6: a c d e f g
};

void setup ()
{
    adk::setup ();
    randomSeed (analogRead (A7));

    digit.write (0b01000000);           // just g, a dash: ready to roll
}

void loop ()
{
    adk::update ();

    if (button.wasPressed ())
    {
        spin ();
        digit.write (faces[random (6)]);
    }
}

// One lit segment chases round the rim, a to f, slowing down like a die
// coming to rest. Shifting a 1 left by one place moves it to the next
// segment.
void spin ()
{
    for (int step = 0; step < 18; ++step)
    {
        digit.write (1 << (step % 6));
        adk::wait (20 + step * 8);
    }
}
