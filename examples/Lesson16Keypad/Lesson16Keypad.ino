// Lesson 16: Keypad
// A calculator: a number, A to D for + - x /, a number, then #. * clears.

#include <Adk.h>

adk::Lcd    lcd    {31, 32, 33, 34, 35, 36};
adk::Keypad keypad {{22, 23, 24, 25}, {26, 27, 28, 29}};

constexpr adk::Array symbols {'+', '-', 'x', '/'};   // for A, B, C and D

long first     = 0;
long second    = 0;
int  digits    = 0;        // typed so far in the number being typed
char operation = 0;        // one of the symbols, once it's chosen
bool answered  = false;

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    char key = keypad.key ();

    if (key >= '0' && key <= '9')
    {
        typeDigit (key - '0');
    }
    else if (key >= 'A' && key <= 'D' && operation == 0)
    {
        operation = symbols[key - 'A'];
        digits    = 0;
        adk::print (lcd, ' ', operation, ' ');
    }
    else if (key == '#' && operation != 0 && !answered)
    {
        showAnswer ();
    }
    else if (key == '*')
    {
        clearAll ();
    }
}

// Each digit shifts the number one place left and joins on the end: typing
// 4 then 2 makes 4, then 4 × 10 + 2 = 42. Four digits are plenty, and even
// 9999 × 9999 fits in a long.
void typeDigit (int digit)
{
    if (answered)
    {
        clearAll ();
    }

    if (digits < 4)
    {
        long& number = operation == 0 ? first : second;

        number = number * 10 + digit;
        ++digits;
        lcd.print (digit);
    }
}

void showAnswer ()
{
    lcd.at (0, 1);

    if (operation == '/' && second == 0)
    {
        lcd.print ("Divide by 0? No!");
    }
    else if (operation == '/')
    {
        lcd.print ("= ");
        lcd.print (float (first) / second, 3);
    }
    else
    {
        adk::print (lcd, "= ", calculate ());
    }

    answered = true;
}

long calculate ()
{
    switch (operation)
    {
        case '+': return first + second;
        case '-': return first - second;
        default:  return first * second;
    }
}

void clearAll ()
{
    first     = 0;
    second    = 0;
    digits    = 0;
    operation = 0;
    answered  = false;
    lcd.clear ();
}
