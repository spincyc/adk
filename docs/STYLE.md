# C++ style

ADK uses one mechanical style across library code, examples, and tests.

- Types use `UpperCamelCase`; functions and variables use `lowerCamelCase`.
- Acronyms are words: `RgbLed`, `PwmOutput`, `I2cBus`.
- Namespaces and filenames are lowercase; multiword filenames use underscores.
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
