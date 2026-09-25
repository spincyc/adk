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
   holes, every wire from pin to hole. The site draws the bench from it. Put
   each part in its [breadboard home](kit.md#breadboard-homes) and on its
   home pins, so the build carries on from the lesson before; keep every
   part and wire the lesson before already has just where it was. A part
   with no home goes where a tidy builder would put it: in the next free
   columns, in the order the current meets it.
4. Write `docs/lessons/NN-name/index.md` from Lesson 1's shape, with the
   markers `<!-- bench -->`, `<!-- closeup -->`, `<!-- steps -->`,
   `<!-- connections -->` and `<!-- sketch -->` where those belong.
5. `make pins site` must pass. `make pins` runs each sketch's `setup ()` on
   the host and fails if the pins it claims and the pins the circuit wires
   differ by a single pin.

The build steps show what carries on: when a lesson keeps parts in the same
holes, or wires between the same two points, as the lesson before, its steps
say what to keep, what to take out and what to add. If the page tells the
learner what to keep or move, make it say the same.

## Describing a build

`docs/_theme/bench.py` explains every call; these are the ones that keep
lessons alike. [The kit page](kit.md#breadboard-homes) gives each part's home.

- **Power.** Never wire the Mega's 5V or GND to a rail: the site does it the
  same way in every lesson, from the outer GND at the end of the long header
  into B-3 and the outer 5V at its top into T+3, and joins the other rail of
  a pair at the far end (B-60 to T-60, T+61 to B+61) only when a part uses
  it. Those two header pins are kept for the rails. With the power module,
  which sits at the right end, the Mega's GND still joins the rails at B-3
  and its 5V stays off them.
- **An LED and its resistor.** The pin's wire into row j of column *c*, the
  resistor standing across the middle gap from g*c* to e*c*, the LED's long
  leg in b*c* and its short leg in b*c+1*, and a black jumper from a*c+1*
  to the bottom − rail: `bench.wire ("26", "j6")`,
  `bench.resistor ("220 Ω", "g6", "e6")`,
  `bench.led ("red", anode="b6", cathode="b7")`, `bench.wire ("a7", "B-7")`.
- **A button** across the middle gap, fed from row j of its left column,
  its right column to the − rail.
- **The screen.** `bench.screen ()` builds the LCD at its home with its
  contrast knob, power, and pins 31 to 36, wired the same in every lesson,
  so it can stay on the breadboard from one lesson to the next.
- **Wires** find their own way round parts, labels and each other. When a
  wire needs to go a particular way, name holes or points it passes
  through: `bench.wire ("36", "e18", via=["j14"])`, where a point is
  `(x, y)` in inches.
- **Power pins.** `"GND"` and `"5V"` take the free pin nearest the wire's
  other end. These names pick a particular one:

  | Name | Pin |
  |---|---|
  | `GND.top` | The GND beside pin 13 |
  | `5V.power`, `3.3V`, `VIN` | On the power header |
  | `GND.power`, `GND.power2` | The power header's two GNDs, left then right |
  | `5V.long` | The inner 5V at the top of the long double header |
  | `GND.long` | The inner GND at its bottom end |

  The outer pins of those two pairs feed the rails and are kept for them.
- **The power module** sits at the right end and sets each pair of rails
  with its own jumper: `bench.power_module ("right", top="5V",
  bottom="off")` powers the top rails at 5 V and leaves the bottom + rail
  unpowered. Either side may also be `"3.3V"`.

## Measuring with a multimeter

A lesson may end with a *Measure it* section for learners who have a
multimeter. In `circuit.py`, after the build, record each reading:

```python
bench.measure ("Across the LED", red="b6", black="b7", expect="about 2 V", when="LED on")
```

A probe may go on a hole that holds, or shares a strip with, something in
the build; on a pin number such as `"26"`, which puts it in a free hole of
the strip that pin's wire lands in; or on `"GND"` or `"5V"`, a free hole of
the rail that carries it, nearest the other probe. The site refuses a probe
that touches nothing. The marker `<!-- measure -->` in the page draws each
reading as a small close-up with the meter below the board, set to DC
volts and showing the first number in `expect`, and lists them in a table
of probes, expected reading and when to take it. Say in the text how to set
the meter, and slow the sketch down when a reading needs something to stay
on.

## The board package

`make site` also writes `package_adk_index.json` and the ADK Boards archive
into the site. The package takes the library's version: when
`library.properties` changes version, change `boards/avr/platform.txt` to
match, or the site will not build. To add a compiler for another computer,
run the Toolchain workflow and add the entries it prints to
`boards/toolchain.json`.
