// Lesson 25: LED Matrix
// The matrix fills dot by dot, then each press of the button shows the next
// picture, and after the last picture a message scrolls past.

#include <Adk.h>

adk::LedMatrix matrix {47, 48, 49};
adk::Button    button {22};

// A picture is eight rows of eight dots, from the top down. A 1 is a lit dot.
using Picture = adk::Array<uint8_t, 8>;

constexpr Picture smiley
{
    0b00111100,
    0b01000010,
    0b10100101,
    0b10000001,
    0b10100101,
    0b10011001,
    0b01000010,
    0b00111100
};

constexpr Picture heart
{
    0b01100110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00000000
};

constexpr Picture invader
{
    0b00011000,
    0b00111100,
    0b01111110,
    0b11011011,
    0b11111111,
    0b00100100,
    0b01011010,
    0b10100101
};

constexpr adk::Array pictures {smiley, heart, invader};

uint8_t slide = 0;    // which picture shows; one past the last is the message

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
        slide = (slide + 1) % (pictures.size () + 1);
    }

    if (slide < pictures.size ())
    {
        matrix.show (pictures[slide]);
    }
    else
    {
        matrix.scroll ("HELLO!");
    }
}

// Light every dot in turn, along each row from the top-left corner, so you
// can see which way round the matrix is.
void fillDotByDot ()
{
    for (int y = 0; y < 8; ++y)
    {
        for (int x = 0; x < 8; ++x)
        {
            matrix.set (x, y);
            adk::wait (30);
        }
    }

    adk::wait (500);
}
