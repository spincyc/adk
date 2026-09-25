# Contributing

ADK is a library, a set of example sketches, and this website, all in one
repository. Everything builds with `make`, and everything it builds goes in
`build/`.

## Layout

| Path | What |
|---|---|
| `src/Adk.h`, `src/adk/` | The library: one header and one source file per part |
| `examples/` | One sketch per lesson, also the library's Arduino examples |
| `tests/` | Host tests, a fake Arduino core, and the style check |
| `docs/` | This website: pages in Markdown, lessons in `docs/lessons/` |
| `docs/_theme/` | The site's theme, build hook, drawing engine and course list |
| `boards/` | ADK Boards, the Arduino IDE board package: `avr/` is the platform, `toolchain.json` its compiler downloads |

## Commands

| Command | Does |
|---|---|
| `make test` | Build and run the host tests |
| `make sanitize` | The same tests under AddressSanitizer and UBSan |
| `make toolchain` | Fetch the C++23 avr-gcc the examples build with |
| `make examples` | Compile every example for the Mega, failing on any library warning |
| `make pins` | Check each example claims exactly the pins its lesson's circuit wires |
| `make size` | Flash and RAM used by each example |
| `make site` | Build this website into `build/site` |
| `make pdf` | Print every lesson to `build/site/pdf` |
| `make serve` | Preview the website at <http://127.0.0.1:8000> |
| `make style` | Check the mechanical rules of the [style guide](STYLE.md) |
| `make boards` | Install the site's ADK Boards package into `build/boards` and compile two lessons with it |
| `make check` | All of the above, as CI runs it |
| `make upload EXAMPLE=… PORT=…` | Upload one example to a Mega |

The website needs Python 3; `make site` creates a virtual environment in
`build/venv` the first time. The PDFs need Chromium.

## Adding a part

[How ADK works](ARCHITECTURE.md#adding-a-device) lists the steps: a header
that starts with how to wire the part, its source, host tests, and a line in
`src/Adk.h`.

## Adding a lesson

1. Add or check its entry in `docs/_theme/course.yml`.
2. Write the sketch in `examples/LessonNNName/LessonNNName.ino`.
3. Describe the build in `docs/lessons/NN-name/circuit.py`: every part in its
   holes, every wire from pin to hole. The site draws the bench from it. Lay
   it out the way a tidy builder would: start at column 1, the end nearest
   the Mega, and place parts left to right in the order the current meets
   them, each in the next free columns; use each part's home pins from
   [the kit page](kit.md), in order.
4. Write `docs/lessons/NN-name/index.md` from Lesson 1's shape, with the
   markers `<!-- bench -->`, `<!-- closeup -->`, `<!-- steps -->`,
   `<!-- connections -->` and `<!-- sketch -->` where those belong.
5. `make pins site` must pass. `make pins` runs each sketch's `setup ()` on
   the host and fails if the pins it claims and the pins the circuit wires
   differ by a single pin.

## The board package

`make site` also writes `package_adk_index.json` and the ADK Boards archive
into the site. The package takes the library's version: when
`library.properties` changes version, change `boards/avr/platform.txt` to
match, or the site will not build. To add a compiler for another computer,
run the Toolchain workflow and add the entries it prints to
`boards/toolchain.json`.
