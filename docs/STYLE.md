# C++ style

One style across the library, tests, and examples. `make style` checks the
mechanical rules; alignment and naming are for people to judge.

## Layout

- Four spaces, no tabs, lines of at most 100 characters.
- A space before every call or declaration parenthesis: `led.on ()`,
  `void setup ()`.
- Every brace on its own line, except namespace braces. Nested namespaces share
  one line, and their contents are indented once.
- Braces around every `if`, `else`, and loop body. A one-line `case` may sit on
  its label line.
- Align related names, types, `=` signs, and call parentheses when that makes a
  group easier to scan. Alignment stays within one group; it does not cross a
  blank line.

```cpp
void Led::show (bool lit)
{
    digitalWrite (pin_, (lit == (polarity_ == ActiveHigh)) ? HIGH : LOW);
    lit_ = lit;
}
```

## Naming

- Types are `UpperCamelCase`, functions and variables `lowerCamelCase`, and
  acronyms are words: `RgbLed`, `IrReceiver`, `Ds18b20`.
- Stored members end in an underscore; parameters do not.
- Files and namespaces are lowercase, with underscores between words:
  `rgb_led.h`. `Adk.h` is the one exception, because Arduino looks for it.
- Name things for what they mean in the circuit, and use the same words in the
  code, the lesson, and the wiring table.

## Language

- C++11. The Arduino AVR core compiles libraries with `-std=gnu++11`.
- `struct`, not `class`.
- No heap, exceptions, or RTTI. No Arduino libraries (Wire, SPI, Servo) in the
  library: every one of them would cost every sketch flash and RAM.
- Headers declare; implementations go in the `.cpp` unless they are templates.
- Comments say why: an electrical reason, a timing limit, a surprising choice.
  They do not narrate the code.

## Examples

An example is teaching material. It should read from purpose to mechanism.

- Declare the circuit's parts at the top, in the order the lesson introduces
  them.
- `setup ()` calls `adk::setup ()` and does anything that happens once.
- `loop ()` starts with `adk::update ()` (or uses `adk::wait ()`), then reads,
  decides, and acts.
- Name helper functions for what they do in the project, such as `rollDice ()`
  or `showScore ()`, and put them after `loop ()`.
- Keep sketches short. A component lesson fits on one screen; a project fits
  in about 150 lines.
