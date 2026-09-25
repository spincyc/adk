# Working on ADK

ADK is an Arduino Mega 2560 library and a 36-lesson course, in one
repository. Read these before changing anything:

1. `docs/ARCHITECTURE.md`: how the library works, and how to add a part.
2. `docs/STYLE.md`: the C++ style, which `make style` checks.
3. `docs/contributing.md`: the layout, the commands, and how to add a lesson.

## Rules

- **Everything built goes in `build/`**, which git ignores. No tool, test or
  script may write anywhere else in the tree.
- **The library stays small and honest.** C++23, with no heap, exceptions,
  RTTI, coroutines or Arduino libraries. One `adk::Object` per part; pins are
  claimed in `setup ()`; time enters only through `update (now)`; events last
  exactly one update. Every part has host tests, and `make examples` must stay free of
  warnings.
- **A lesson's wiring is described once**, in its `circuit.py`. The drawings,
  build steps and connection list come from it, and the site refuses to build
  if the sketch and the circuit disagree about a pin. Use each part's home
  pins from `docs/kit.md`, and lay the breadboard out from column 1 (the end
  nearest the Mega) in the order the current flows.
- **Lessons continue each other.** The Mega's GND always lands in the same
  − rail hole, the one nearest the Mega, and power always comes in the same
  way. A part that recurs keeps its home position on the breadboard, so each
  lesson adds to or takes from the previous build instead of rewiring it.
- **Lessons are for beginners.** Plain words, one new idea at a time, a
  prediction before each experiment, and something that visibly works at the
  end. Keep sketches short: one screen for a part, about 150 lines for a
  project.
- **Safety** follows `docs/safety.md`: nothing touches mains, no motor or
  servo runs from a pin, every LED has a resistor, the RFID reader gets 3.3 V.
- **Say what is verified.** Host tests and compiling are not a working
  circuit. Never claim a lesson or part works on hardware unless someone has
  built it and recorded that they did.
- **No personal information.** Commits use the repository's configured
  identity, ADK Project with the GitHub no-reply address. Never add a
  person's name, email address or home directory to a file or commit.

## Commands

| Command | Does |
|---|---|
| `make test` | Host tests |
| `make sanitize` | Host tests under ASan and UBSan |
| `make examples` | Compile every example for the Mega |
| `make site` / `make pdf` | The website, and every lesson as a PDF |
| `make style` | The mechanical style rules |
| `make check` | Everything CI runs |
