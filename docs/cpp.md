# The C++ you've met

Every sketch in the course is written in C++. Each lesson explains the new
pieces of C++ in its sketch the first time they appear, and this page lists
them all, with the lesson that explains each one, so you can look one up
again. The parts themselves, such as `adk::Led` and `adk::Timer`, are in
[the library reference](library/index.md).

## The shape of a sketch

| You write | What it means | Explained in |
|---|---|---|
| `// a note` | A **comment**: everything from `//` to the end of the line is for people, and the Mega ignores it | Lesson 1 |
| `#include <Adk.h>` | Brings in the ADK library | Lesson 1 |
| `#include <EEPROM.h>`, `#include <string.h>` | Bring in Arduino's EEPROM, and C's functions for text | Lessons 18 and 42 |
| `adk::Led led {26};` | An **object**: one part of the circuit, with a name, and the pin it uses | Lesson 1 |
| `setup ()`, `loop ()` | `setup ()` runs once at the start; `loop ()` runs again and again | Lesson 1 |
| `led.on ();` | Calls a function: the brackets hold what it needs to know, and `;` ends each instruction | Lesson 1 |
| `void countPress () { ... }` | A function of your own; `void` means it hands nothing back | Lesson 2 |
| `void yourTurn (int pressed)` | A **parameter**: a value the function takes in its brackets, which each call fills in; commas separate two or more | Lessons 6 and 9 |
| `int pressedKey () { ... }` | A function that hands back a value, here a whole number, with `return` | Lesson 6 |
| `return;` | Leaves a function at once, before its end | Lesson 18 |
| `struct SerialCable : adk::Object`, `void setup () override` | A part of your own, built on ADK's `adk::Object`, with its own `setup ()` in place of ADK's | E24 |

## Values

| You write | What it means | Explained in |
|---|---|---|
| `int presses = 0;` | A **variable**: a named box holding a whole number, here starting at 0 | Lesson 2 |
| `int8_t x = 3, y = 6;` | Two variables of one type, made in one line | Lesson 54 |
| `bool`, `true`, `false` | A value that is only ever true or false | Lesson 2 |
| `"Presses: "` | A piece of text, in double quotes | Lesson 2 |
| `constexpr` | A value settled when the sketch is compiled, which never changes | Lesson 4 |
| `uint8_t`, `uint16_t` | Whole numbers that are never negative: 0 to 255, and 0 to 65 535 | Lessons 4 and 5 |
| `int8_t` | A one-byte whole number that can go below zero: −128 to 127 | Lesson 27 |
| `long`, `unsigned long` | Whole numbers with far more room than an `int` | Lessons 8 and 12 |
| `uint32_t`, `int32_t` | Whole numbers of 32 bits, never negative or either way, as `unsigned long` and `long` are on the Mega | Lessons 34 and 51 |
| `size_t` | The type that sizes and places in a list come in, never negative | Lesson 18 |
| `auto total = modes[current];` | A variable whose type the compiler works out from its first value | Lesson 12 |
| `120UL`, `8L` | A number written as an `unsigned long`, or a `long` | Lessons 27 and 48 |
| `0b01100110` | A number written in binary, one bit per digit | Lesson 10 |
| `' '`, `char` | One character, in single quotes | Lessons 13 and 14 |
| `'\0'` | The character that means "none" | Lesson 16 |
| `float` | A number with decimals, such as 23.4 | Lesson 14 |
| `0.9`, `10.0` | A number written with a decimal point, which keeps its fractions in a sum | Lessons 30 and 37 |
| `const char*` | The type of a piece of text | Lesson 14 |
| `0x45` | A number written in hexadecimal, counting in sixteens | Lesson 22 |
| `enum class State { Waiting, Ready, Go };` | A new kind of value, with a name for each of its values | Lesson 3 |
| `adk::Joystick::Direction`, `adk::Joystick::Right` | Names inside a part's own name: its type of direction, and one of them | Lesson 27 |
| `struct Key { ... };` | A new type that bundles values that belong together; `key.pitch` reaches inside | Lesson 5 |
| `nearest = {angle, cm};` | Fills in every value of a `struct` at once, in order | Lesson 21 |
| `{.partner = 2, .power = 10}` | Fills in a `struct`'s values by name; the ones left out keep their usual values | Lesson 43 |
| `adk::DateTime heardAt {};` | Empty braces: a value with nothing in it yet, every number 0 | Lesson 46 |
| `using Picture = adk::Array<uint8_t, 8>;` | A name of your own for a type | Lesson 25 |
| `const` | A promise not to change something | Lesson 18 |
| `Item& level = menu[0];` | A **reference**: another name for a value that lives somewhere else, not a copy | Lesson 29 |
| `long& times` as a parameter | A reference parameter: the function changes the very variable it was handed, not a copy | Lesson 47 |
| `char (223)`, `int (...)` | Turns a value into another type | Lessons 10 and 14 |

Lessons 49 and 51 use the same form to carry values across the radio:
`char (...)` for a key, and `uint32_t (...)` and `int32_t (...)` for a
card's number. Lesson 53 adds `uint8_t (...)` and `uint16_t (...)`. Arduino
code elsewhere often writes `static_cast<int> (...)`, which does the same
more strictly; E24's sketch uses `static_cast<char> (...)`.

## Sums

| You write | What it means | Explained in |
|---|---|---|
| `+`, `-`, `*` | Add, take away and multiply, as on paper | Lessons 4, 6 and 9 |
| `/` | Divide; between whole numbers it throws the remainder away | Lessons 7 and 9 |
| `%` | The remainder after dividing: `7 % 5` is 2 | Lessons 2 and 4 |
| `presses++`, `left--` | Add one; take one away | Lessons 2 and 18 |
| `++brightness`, `--brightness` | The same, written before the name | Lesson 22 |
| `angle += step`, `angle -= step` | Add `step` to `angle`, or take it away | Lesson 21 |
| `1 << step` | A 1 moved `step` places to the left, in binary | Lesson 10 |
| `0b10000000 >> column` | The same, moved to the right | Lesson 30 |
| `row & bit` | Works bit by bit, keeping only the 1s both numbers share; not the same as `&&` | Lesson 30 |
| `pattern | 0b10000000` | Works bit by bit, keeping every 1 from either number; not the same as `||` | Lesson 10 |
| `rows[y] |= dot` | Adds `dot`'s 1s to `rows[y]` | Lesson 54 |
| `min ()`, `max ()`, `constrain ()`, `map ()` | The smaller or bigger of two, a number kept in a range, and a number moved from one range to another | Lesson 8 |
| `abs ()` | A whole number without its sign: `abs (-40)` is 40 | Lesson 21 |
| `fabs ()`, `lround ()` | A decimal number without its sign, and a decimal number rounded to the nearest whole one | Lesson 28 |

## Questions and decisions

| You write | What it means | Explained in |
|---|---|---|
| `if (...) { ... } else { ... }` | Runs the first lines when the answer is yes, the others when it is no | Lesson 2 |
| `else if (...)` | A second question, asked only when the first answer is no | Lesson 3 |
| `==`, `!=` | Are two values equal? Are they different? | Lessons 2 and 4 |
| `<`, `>`, `<=`, `>=` | Smaller? Bigger? Smaller or equal? Bigger or equal? | Lesson 4 |
| `&&`, `||`, `!` | And, or, not: both true, at least one true, the opposite | Lesson 4 |
| `switch`, `case`, `break` | Jumps to the lines for one value, and stops at `break` | Lesson 3 |
| `case 0:` | A `switch` on a number, with a `case` for each | Lesson 46 |
| `default:` | The `case` for every value without one of its own | Lesson 15 |
| `question ? yes : no` | A value chosen by a question | Lesson 12 |
| `a ? x : b ? y : z` | Several questions in a row, each asked only when the one before says no | Lesson 49 |

## Lists and loops

| You write | What it means | Explained in |
|---|---|---|
| `adk::Array moods {...};` | A list of a fixed size: `moods[0]` is its first, and `moods.size ()` how many | Lesson 4 |
| `adk::Note tune [] = {...};` | C++'s own kind of list, which counts its items for you | Lesson 5 |
| `uint8_t heart [8] {...};` | The same, with its size in the brackets | Lesson 13 |
| `char secret [] = "SLS";` | A piece of text kept as a list of letters | Lesson 35 |
| `pictures[1][0]` | A list of lists: the first item of the second list | Lesson 25 |
| `for (auto& key : keys)` | Runs its lines once for each item in a list; `auto` works out the type, and `&` means the real item, not a copy | Lesson 5 |
| `for (auto number : sequence)` | The same, with each item a copy, for a loop that only reads them | Lesson 6 |
| `for (int number = 0; number < 4; number++)` | A counting loop: `number` goes 0, 1, 2, 3 | Lesson 6 |
| `adk::Vector<uint8_t, 100>` | A list that grows, with room for 100; the angle brackets give its type and its room | Lesson 6 |
| `adk::Deque` | A list you can add to and take from at both ends | Lesson 27 |
| `adk::Span<const char>` | A view of a list kept somewhere else, of any length | Lesson 18 |
| `while (...) { ... }` | Repeats its lines for as long as the answer is yes | Lesson 8 |
| `do { ... } while (...);` | The same, asking at the end, so its lines run at least once | Lesson 21 |
| `break` in a loop | Leaves the loop early | Lesson 21 |

## Text and the Serial Monitor

| You write | What it means | Explained in |
|---|---|---|
| `Serial.begin (9600);` | Opens the USB link to your computer, at 9600 bits a second | Lesson 2 |
| `adk::println (Serial, "Presses: ", presses);` | Prints its pieces in a row and ends the line; `adk::print ()` leaves the line open | Lessons 2 and 14 |
| `Serial.println ("...")` | Arduino's own printing, one piece at a time | Lesson 34 |
| `Serial.print (card, HEX)` | Prints a number in hexadecimal | Lesson 51 |
| `Serial1`, `Serial2`, `Serial3` | The Mega's three other serial links, on pins 14 to 19 | Lessons 40 and 45 |
| `Serial1.available ()`, `.read ()`, `.write ()` | How many characters have arrived, take one, and send one | E24 |
| `adk::Text<24> message;` | Text you print into, with room for 24 characters | Lesson 27 |
| `message.c_str ()` | The text inside, for a function that wants a `const char*`; `message.c_str ()[at]` is one character of it | Lessons 27 and 42 |
| `strcasecmp ()` | Compares two pieces of text, taking no notice of capitals | Lesson 42 |
| `atoi ()` | Turns text such as `"20"` into the number 20 | Lesson 55 |

## From Arduino

| You write | What it means | Explained in |
|---|---|---|
| `random (2000, 5000)`, `random (4)` | A whole number from 2000 up to 4999, or from 0 up to 3 | Lessons 3 and 6 |
| `randomSeed (analogRead (A7));` | Starts `random ()` from the reading of a floating pin | Lesson 3 |
| `millis ()` | The milliseconds since the Mega started | Lesson 3 |
| `A0` to `A15` | The pins that measure a voltage | Lesson 7 |
| `LED_BUILTIN` | Pin 13, which has the Mega's own **L** LED | Lessons 1 and 35 |
| `EEPROM.read ()`, `EEPROM.update ()` | Reads and writes memory that keeps its contents without power | Lesson 18 |
