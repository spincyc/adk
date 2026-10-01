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
| `adk::Led led {26};` | An **object**: one part of the circuit, with a name, and the pin it uses | Lesson 1 |
| `setup ()`, `loop ()` | `setup ()` runs once at the start; `loop ()` runs again and again | Lesson 1 |
| `led.on ();` | Calls a function: the brackets hold what it needs to know, and `;` ends each instruction | Lesson 1 |
| `void countPress () { ... }` | A function of your own; `void` means it hands nothing back | Lesson 2 |
| `int pressedKey () { ... }` | A function that hands back a value, here a whole number, with `return` | Lesson 6 |
| `return;` | Leaves a function at once, before its end | Lesson 21 |

## Values

| You write | What it means | Explained in |
|---|---|---|
| `int presses = 0;` | A **variable**: a named box holding a whole number, here starting at 0 | Lesson 2 |
| `bool`, `true`, `false` | A value that is only ever true or false | Lesson 2 |
| `"Presses: "` | A piece of text, in double quotes | Lesson 2 |
| `constexpr` | A value settled when the sketch is compiled, which never changes | Lesson 4 |
| `uint8_t`, `uint16_t` | Whole numbers that are never negative: 0 to 255, and 0 to 65 535 | Lessons 4 and 5 |
| `long`, `unsigned long` | Whole numbers with far more room than an `int` | Lessons 8 and 12 |
| `120UL` | A number written as an `unsigned long` | Lesson 27 |
| `0b01100110` | A number written in binary, one bit per digit | Lesson 10 |
| `' '`, `char` | One character, in single quotes | Lessons 13 and 14 |
| `float` | A number with decimals, such as 23.4 | Lesson 14 |
| `const char*` | The type of a piece of text | Lesson 14 |
| `0x45` | A number written in hexadecimal, counting in sixteens | Lesson 22 |
| `enum class State { Waiting, Ready, Go };` | A new kind of value, with a name for each of its values | Lesson 3 |
| `struct Key { ... };` | A new type that bundles values that belong together; `key.pitch` reaches inside | Lesson 5 |
| `using Picture = adk::Array<uint8_t, 8>;` | A name of your own for a type | Lesson 25 |
| `const` | A promise not to change something | Lesson 18 |
| `char (223)`, `int (...)` | Turns a value into another type | Lessons 10 and 14 |

Lesson 49 uses the same form, `char (...)` and `uint32_t (...)`, to carry a
key and a card's number across the radio. Arduino code elsewhere often
writes `static_cast<int> (...)`, which does the same more strictly.

## Sums

| You write | What it means | Explained in |
|---|---|---|
| `+`, `-`, `*`, `/` | Add, take away, multiply and divide; `/` between whole numbers throws the remainder away | Lessons 7 and 9 |
| `%` | The remainder after dividing: `7 % 5` is 2 | Lessons 2 and 4 |
| `presses++`, `left--` | Add one; take one away | Lessons 2 and 18 |
| `++brightness`, `--brightness` | The same, written before the name | Lesson 22 |
| `angle += step`, `angle -= step` | Add `step` to `angle`, or take it away | Lesson 21 |
| `1 << step` | A 1 moved `step` places to the left, in binary | Lesson 10 |
| `0b10000000 >> column` | The same, moved to the right | Lesson 30 |
| `row & bit` | Works bit by bit, keeping only the 1s both numbers share; not the same as `&&` | Lesson 30 |
| `min ()`, `max ()`, `constrain ()`, `map ()` | The smaller or bigger of two, a number kept in a range, and a number moved from one range to another | Lesson 8 |

## Questions and decisions

| You write | What it means | Explained in |
|---|---|---|
| `if (...) { ... } else { ... }` | Runs the first lines when the answer is yes, the others when it is no | Lesson 2 |
| `else if (...)` | A second question, asked only when the first answer is no | Lesson 5 |
| `==`, `!=` | Are two values equal? Are they different? | Lessons 2 and 4 |
| `<`, `>`, `<=`, `>=` | Smaller? Bigger? Smaller or equal? Bigger or equal? | Lesson 4 |
| `&&`, `||`, `!` | And, or, not: both true, at least one true, the opposite | Lesson 4 |
| `switch`, `case`, `break` | Jumps to the lines for one value, and stops at `break` | Lesson 3 |
| `default:` | The `case` for every value without one of its own | Lesson 15 |
| `question ? yes : no` | A value chosen by a question | Lesson 12 |

## Lists and loops

| You write | What it means | Explained in |
|---|---|---|
| `adk::Array moods {...};` | A list of a fixed size: `moods[0]` is its first, and `moods.size ()` how many | Lesson 4 |
| `adk::Note tune [] = {...};` | C++'s own kind of list, which counts its items for you | Lesson 5 |
| `for (auto& key : keys)` | Runs its lines once for each item in a list; `auto` works out the type, and `&` means the real item, not a copy | Lesson 5 |
| `for (int number = 0; number < 4; number++)` | A counting loop: `number` goes 0, 1, 2, 3 | Lesson 6 |
| `adk::Vector<uint8_t, 100>` | A list that grows, with room for 100; the angle brackets give its type and its room | Lesson 6 |
| `adk::Span<const char>` | A view of a list kept somewhere else, of any length | Lesson 18 |
| `while (...) { ... }` | Repeats its lines for as long as the answer is yes | Lesson 8 |
| `do { ... } while (...);` | The same, asking at the end, so its lines run at least once | Lesson 21 |
| `break` in a loop | Leaves the loop early | Lesson 21 |
