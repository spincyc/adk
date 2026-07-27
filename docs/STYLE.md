# C++ style

ADK uses one mechanical style across library code, examples, and tests.

- Types use `UpperCamelCase`; functions and variables use `lowerCamelCase`.
- Acronyms are words: `RgbLed`, `PwmOutput`, `I2cBus`.
- Namespaces and filenames are lowercase; multiword filenames use underscores.
  `Adk.h` is the sole filename exception because Arduino metadata exposes that
  normalized CamelCase umbrella header to library consumers.
- Stored members have a trailing underscore. Parameters do not.
- Use `struct`, not `class`.
- Every brace occupies its own line except namespace braces: nested namespaces
  share one line, and their contents receive one four-space indentation level.
- Braces are mandatory for every conditional and loop.
- A one-line `case` action may remain on its label line.
- Align related types, names, opening parentheses, assignments, and calls when
  the alignment makes a logical group easier to scan.
- Keep headers declarative. Put implementations out of line unless a template,
  constant expression, or measured performance requirement prevents it.
- Do not use exceptions, RTTI-dependent designs, or heap allocation internally.
  Destructors remain `noexcept`, so ADK objects clean up correctly if an
  exception-enabled caller unwinds through them.
- Treat warnings as errors in host builds.

Alignment is local to a related block; it does not cross unrelated declarations
or blank-line-separated operations. `.clang-format` captures the rules it can.

## Narrative code

Examples read from intent to mechanism.

- Declare objects in dependency order: platform, resources, endpoints,
  components, then behaviors.
- Let `setup()` read as acquire, configure, start.
- Let `loop()` read as observe, decide, actuate.
- Name helpers for domain actions such as `readButtons()`, `chooseCue()`, and
  `showCue()`, not vague implementation fragments.
- Put high-level flow before low-level helper definitions.
- Use the same nouns and verbs in code, diagrams, lessons, and reference prose.
- Prefer a few cohesive operations over many one-line forwarding helpers.
- Comment constraints, electrical reasons, and surprising decisions. Do not
  narrate syntax or compensate for unclear names with comments.

An example is teaching material, not a compressed test fixture. Keep its
control flow visible while leaving resource mechanics inside the library.

## Circuit observability

Make debug behavior part of the circuit narrative. Every example provides a
non-Serial verification signal or named test point. Its code and lesson use the
same signal name. Prefer a domain action such as `showReady()` to scattered
diagnostic writes.

Keep two questions distinct: did the object acquire its resource, and did the
circuit enter its safe state? Do not overload one LED state to prove both
without an explicit, observable sequence. Serial messages may explain an
observation, but must not be the only observation.
