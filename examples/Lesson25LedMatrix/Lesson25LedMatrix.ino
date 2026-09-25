// Lesson 25: LED Matrix
// The matrix fills dot by dot, then each press of the button shows the next
// picture, and after the last picture a message scrolls past.

#include <Adk.h>

adk::LedMatrix matrix {47, 48, 49};
adk::Button    button {22};

const uint8_t pictures [3][8] =
{
    {   // a smiley
        0b00111100,
        0b01000010,
        0b10100101,
        0b10000001,
        0b10100101,
        0b10011001,
        0b01000010,
        0b00111100
    },
    {   // a heart
        0b01100110,
        0b11111111,
        0b11111111,
        0b11111111,
        0b01111110,
        0b00111100,
        0b00011000,
        0b00000000
    },
    {   // a space invader
        0b00011000,
        0b00111100,
        0b01111110,
        0b11011011,
        0b11111111,
        0b00100100,
        0b01011010,
        0b10100101
    }
};

int slide = 0;

void setup ()
{
    adk::setup ();
    fillDotByDot ();
}

void loop ()
{
    adk::update ();

    if (button.wasPressed ())
    {
        slide = (slide + 1) % 4;
    }

    if (slide < 3)
    {
        matrix.show (pictures[slide]);
    }
    else
    {
        matrix.scroll ("HELLO!");
    }
}

void fillDotByDot ()
{
    for (uint8_t y = 0; y < 8; ++y)
    {
        for (uint8_t x = 0; x < 8; ++x)
        {
            matrix.set (x, y);
            adk::wait (30);
        }
    }

    adk::wait (500);
}
