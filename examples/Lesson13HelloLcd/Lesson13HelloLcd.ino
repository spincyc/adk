// Lesson 13: Hello, LCD
// Says hello on the LCD, and walks a little figure along the bottom row.

#include <Adk.h>

adk::Lcd   lcd  {31, 32, 33, 34, 35, 36};
adk::Every pace {250};

// Five dots across and eight down, a row at a time: each 1 is a dot that
// lights.
constexpr uint8_t heart    [8] {0b00000, 0b01010, 0b11111, 0b11111,
                                0b01110, 0b00100, 0b00000, 0b00000};
constexpr uint8_t standing [8] {0b01110, 0b01110, 0b00100, 0b11111,
                                0b00100, 0b00100, 0b01010, 0b01010};
constexpr uint8_t striding [8] {0b01110, 0b01110, 0b00100, 0b01110,
                                0b10101, 0b00100, 0b01010, 0b10001};

int column = 0;

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

// Rub the figure out, move one column right, and draw it again: standing
// on even columns, striding on odd ones.
void takeStep ()
{
    lcd.at (column, 1).print (' ');
    column = (column + 1) % 16;
    lcd.at (column, 1).write (column % 2 == 0 ? 2 : 3);
}
