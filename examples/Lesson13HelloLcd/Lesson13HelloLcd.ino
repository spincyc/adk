// Lesson 13: Hello, LCD
// Says hello on the LCD, and walks a little figure along the bottom row.

#include <Adk.h>

adk::Lcd   lcd  {31, 32, 33, 34, 35, 36};
adk::Every pace {250};

// Five dots across and eight down: each 1 is a dot that lights.
const uint8_t heart    [8] {0b00000, 0b01010, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000, 0b00000};
const uint8_t standing [8] {0b01110, 0b01110, 0b00100, 0b11111, 0b00100, 0b00100, 0b01010, 0b01010};
const uint8_t striding [8] {0b01110, 0b01110, 0b00100, 0b01110, 0b10101, 0b00100, 0b01010, 0b10001};

uint8_t column = 0;

void setup ()
{
    adk::setup ();

    lcd.createChar (1, heart);
    lcd.createChar (2, standing);
    lcd.createChar (3, striding);

    lcd.print ("Hello, LCD! ");
    lcd.write (1);
}

void loop ()
{
    adk::update ();

    if (pace.ticked ())
    {
        takeStep ();
    }
}

void takeStep ()
{
    lcd.setCursor (column, 1);
    lcd.print (' ');

    column = (column + 1) % 16;

    lcd.setCursor (column, 1);
    lcd.write (column % 2 == 0 ? 2 : 3);
}
