// Lesson 16: Keypad
// A calculator: a number, A to D for + - x /, a number, then #. * clears.

#include <Adk.h>

adk::Lcd    lcd    {31, 32, 33, 34, 35, 36};
adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};

constexpr adk::Array symbols {'+', '-', 'x', '/'};   // for A, B, C and D

long first     = 0;        // the number before the operation
long number    = 0;        // the number being typed
int  digits    = 0;        // its digits: four at most, so 9999 × 9999 fits
char operation = 0;        // one of the symbols, once it's chosen
bool answered  = false;

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    char key   = keypad.key ();
    bool digit = key >= '0' && key <= '9';

    // * clears, and so does a new number typed after an answer.
    if (key == '*' || (digit && answered))
    {
        clearAll ();
    }

    if (digit && digits < 4)
    {
        // Each digit shifts the number one place left and joins on the
        // end: 4, then 2, makes 4 × 10 + 2 = 42.
        number = number * 10 + (key - '0');
        digits++;
        lcd.print (key);
    }
    else if (key >= 'A' && key <= 'D' && operation == 0)
    {
        operation = symbols[key - 'A'];
        first     = number;
        number    = 0;
        digits    = 0;
        adk::print (lcd, ' ', operation, ' ');
    }
    else if (key == '#' && operation != 0 && !answered)
    {
        showAnswer ();
    }
}

void showAnswer ()
{
    lcd.at (0, 1);

    if (operation == '/' && number == 0)
    {
        lcd.print ("Divide by 0? No!");
    }
    else
    {
        adk::print (lcd, "= ", calculate ());
    }

    answered = true;
}

// Whole numbers make a whole answer: 7 / 2 is 3, the remainder dropped.
long calculate ()
{
    switch (operation)
    {
        case '+': return first + number;
        case '-': return first - number;
        case 'x': return first * number;
        default:  return first / number;
    }
}

void clearAll ()
{
    first     = 0;
    number    = 0;
    digits    = 0;
    operation = 0;
    answered  = false;
    lcd.clear ();
}
