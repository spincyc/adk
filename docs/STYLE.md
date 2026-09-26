# C++ style

One style across the library, tests, and examples. `make style` checks
tabs, trailing spaces, the final newline, line lengths, the space before
each parenthesis, where a block's braces go, and that the library's
constants are `constexpr`; alignment, naming and the other language rules
are for people to judge.

## Layout

- Four spaces, no tabs, lines of at most 100 characters.
- A space before every call or declaration parenthesis: `led.on ()`,
  `void setup ()`.
- The braces of a block (a function, a type, a lambda or a statement's
  body) go on lines of their own, except namespace braces and a body short
  enough for one line: a template's accessors, such as
  `size () const { return size_; }`, and a small lambda. Braces that hold
  values, such as an initializer or an enum's names, stay with their code.
  Nested namespaces share one line, and their contents are indented once.
- Braces around every `if`, `else`, `switch` and loop body. A `do` loop's
  `while (...);` goes on the line after its closing brace. A one-line `case`
  may sit on its label line.
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

- C++23, GNU dialect. The Arduino core's own compiler, GCC 7, stops at C++17,
  so sketches build with avr-gcc 16: the ADK Mega 2560 board in the Arduino
  IDE, or `make toolchain` here. `Adk.h` refuses an older compiler by name.
- `struct`, not `class`.
- No heap, exceptions, RTTI, or coroutines. No Arduino libraries (Wire, SPI,
  Servo) in the library: every one of them would cost every sketch flash and
  RAM.
- The AVR has no C++ standard library. `adk::Array`, `Vector`, `Deque` and
  `Span` stand in for their `std` namesakes, with the same member names, and
  reserve all their room when they are declared.
- Prefer `auto` to spelling out a template: `void print (Print& out, const
  auto&... parts)` rather than `template <typename... Parts>`. Write
  `template <...>` only where a size or type must be named, as in
  `Vector<T, Capacity>`.
- Constants are `constexpr` (`inline constexpr` in a header), states are an
  `enum class`, and loops over a collection are range-`for` loops.
- Headers declare; implementations go in the `.cpp` unless they are templates.
- Comments say why: an electrical reason, a timing limit, a surprising choice.
  They do not narrate the code.

## Examples

An example is teaching material. It should read from purpose to mechanism, so
the interesting part of a lesson is what the reader sees first.

- Declare the circuit's parts at the top, in the order the lesson introduces
  them. Parts that belong together go together in a `struct`, such as a
  player's button and light, and parts that repeat go in an `adk::Array`.
- `setup ()` acquires, configures, then starts: `adk::setup ()` first, then
  anything that happens once.
- `loop ()` observes, decides, then acts: it starts with `adk::update ()` (or
  uses `adk::wait ()`), then reads, decides, and acts.
- Keep time with ADK's parts (`adk::Every`, `adk::Timer`, `adk::Stopwatch`)
  rather than arithmetic on `millis ()`, and name states with an `enum class`.
- Name helper functions for what they do in the project, such as `rollDice ()`
  or `showScore ()`, and put them after `loop ()`. A helper earns its place by
  naming a step of the project, not by hiding one line.
- Print with `adk::println (Serial, "Red wins in ", time, " ms")` rather than
  a `print` call per piece.
- Keep sketches short. A component lesson fits on one screen; a project fits
  in about 150 lines. Keep their lines to 80 characters, so they fit the
  lesson page and its PDF without scrolling.

## The Makefile

The Makefile reads as a set of tables, section by section under a
`# Name ---` header.

- A list has one item to a line, continued with a backslash, and the
  backslashes of one list share a column a couple of spaces past its
  longest line.
- The `:=` and `?=` of a section's assignments line up, and so do the
  descriptions of the `## target  description` comments `make help` prints.
- Recipes are indented with one tab; tabs appear nowhere else.
- Comments say why, and no line is longer than 100 characters.

`make style` checks the backslashes, the tabs, trailing spaces and line
lengths; the rest is for people to keep.
