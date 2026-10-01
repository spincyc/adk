// Lesson 16: Keypad
// An adding machine: type a number, then A to add it to the total or B to
// take it away. * starts the total again from 0.

#include <Adk.h>

adk::Lcd    lcd    {31, 32, 33, 34, 35, 36};
adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};

long total  = 0;    // on the top row
long number = 0;    // being typed, on the bottom row
int  digits = 0;    // in the number: four at most, up to 9999

void setup ()
{
    adk::setup ();
    showTotal ();
}

void loop ()
{
    adk::update ();

    char key = keypad.pressedKey ();

    if (key >= '0' && key <= '9' && digits < 4)
    {
        // Each digit moves the others one place left: 4, then 2, is 42.
        number = number * 10 + (key - '0');
        digits++;
        lcd.print (key);
    }
    else if (key == 'A')
    {
        total = total + number;
        showTotal ();
    }
    else if (key == 'B')
    {
        total = total - number;
        showTotal ();
    }
    else if (key == '*')
    {
        total = 0;
        showTotal ();
    }
}

// The total on the top row, and an empty bottom row for the next number.
void showTotal ()
{
    number = 0;
    digits = 0;
    lcd.clear ();
    adk::print (lcd, "Total ", total);
    lcd.at (0, 1);
}
