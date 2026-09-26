# Contributing

ADK is a library, a set of example sketches, and this website, all in one
repository. Everything builds with `make`, and everything it builds goes in
`build/`.

## Layout

| Path | What |
|---|---|
| `src/Adk.h`, `src/adk/` | The library: one header and one source file per part |
| `examples/` | One sketch per lesson (one per board in a two-board lesson), also the library's Arduino examples |
| `tests/` | Host tests, a fake Arduino core, the pins check, the circuit model's tests and the style check |
| `docs/` | This website: pages in Markdown, lessons in `docs/lessons/` |
| `docs/_theme/` | The site's theme, build hook, circuit model (`bench.py`), drawing engine and course list |
| `boards/` | ADK Boards, the Arduino IDE board package: `avr/` is the platform, `toolchain.json` its compiler downloads, `published.txt` every version the site has published |

## Commands

| Command | Does |
|---|---|
| `make deps` | Install what the build needs: on Arch Linux its packages (with `sudo pacman`), then the Arduino core, the compiler and the site's Python; elsewhere it says what to install |
| `make test` | Build and run the host tests |
| `make sanitize` | The same tests under AddressSanitizer and UBSan |
| `make toolchain` | Fetch the C++23 avr-gcc the examples build with |
| `make examples` | Compile every example for the Mega, failing on any library or sketch warning |
| `make 001-blink` | Compile one lesson's sketches; `make lessons` lists every lesson's name |
| `make upload-001-blink PORT=…` | Compile one lesson's sketch and upload it to the Mega on `PORT` |
| `make pins` | Test the circuit model (`tests/circuits.py`), then hold each sketch to its circuit (below) |
| `make size` | Flash and RAM used by each example |
| `make site` | Build this website into `build/site` |
| `make pdf` | Print every lesson to `build/site/pdf` |
| `make serve` | Preview the website at <http://127.0.0.1:8000> |
| `make style` | Check the mechanical rules of the [style guide](STYLE.md) |
| `make boards` | Install the site's ADK Boards package into `build/boards` and compile two lessons with it |
| `make check` | All of the above, as CI runs it |
| `make upload EXAMPLE=… PORT=…` | Upload one example, by its folder in `examples/` |

The website needs Python 3: `make site` creates `build/venv` from
`docs/requirements.txt`, a hashed lock that pip-compile makes from
`docs/requirements.in` (the command is at its top). The PDFs need Chromium.
`make deps` installs all of it.

## Adding a part

[How ADK works](ARCHITECTURE.md#adding-a-device) lists the steps: a header
that starts with how to wire the part, its source, host tests, and a line in
`src/Adk.h`.

## Adding a lesson

1. Add or check its entry in `docs/_theme/course.yml`. The lesson's title
   and arc come from there, so its page doesn't repeat them.
2. Write the sketch in `examples/lessons/NNN-name/NNN-name.ino`, named as
   the lesson's folder is: `examples/lessons/013-hello-lcd/013-hello-lcd.ino`
   for `docs/lessons/013-hello-lcd`. The number has three digits, so the
   lessons sort in order however many there are.
3. Describe the build in `docs/lessons/NNN-name/circuit.py`: every part in its
   holes, every wire from pin to hole. The site draws the bench from it. Put
   each part in its [breadboard home](kit.md#breadboard-homes) and on its
   home pins, so the build carries on from the lesson before; keep every
   part and wire the lesson before already has just where it was. A part
   with no home goes where a tidy builder would put it: in the next free
   columns, in the order the current meets it.
4. Write `docs/lessons/NNN-name/index.md` from Lesson 1's shape, with the
   markers `<!-- bench -->`, `<!-- closeup -->`, `<!-- steps -->`,
   `<!-- connections -->` and `<!-- sketch -->` where those belong. Its front
   matter gives `lesson: NN`, the promise, time, level, parts and ideas.
5. `make pins site` must pass.

`make pins` runs each sketch's `setup ()` on the host and holds it to its
circuit. It fails unless:

- the pins the sketch claims are exactly the Mega pins the circuit wires,
  but for the built-in LED on pin 13, which needs no wire; and
- where the circuit shows what a pin does, the sketch claims it the same
  way: as an output for a pin that drives an LED, a buzzer or a module's
  input, as an input for one that reads a button, a knob or a sensor. A
  resistor passes the question on to what is beyond it.

So a wire in the wrong hole, a pin the sketch doesn't use, or an LED swapped
with a button fails the check. Two pins wired to parts of one kind, such as
two LEDs, can still be swapped without it noticing.

The build steps show what carries on: when a lesson keeps parts in the same
holes, or wires between the same two points, as the lesson before, its steps
say what to keep, what to take out and what to add. When no part carries
over, they begin "Take out everything from Lesson N except the Mega's GND
and 5V wires." (naming the power wires that stay), and then build the rest.
If the page tells the learner what to keep or move, make it say the same.

## Two-board lessons

The projects in an arc marked `boards: 2` in `course.yml` run on two Megas
that talk to each other, Board A and Board B. Board A is the learner's first
Mega, and carries on from the lesson before; Board B is a second one.

- **The circuit.** `circuit.py` makes a `Bench` for each board, each naming
  its sketch, and lists them in order:

  ```python
  dial = Bench ("Board A: a rotary dial and a LoRa modem", columns=(1, 30), sketch="Dial")
  dial.wire (...)
  servo = Bench ("Board B: a servo and a LoRa modem", columns=(1, 30), sketch="Servo")
  servo.wire (...)
  boards = {"A": dial, "B": servo}
  ```

- **The sketches** are in folders of their own inside the lesson's example,
  each named as its board's `sketch=`:
  `examples/lessons/044-remote-dial/Dial/Dial.ino` and
  `examples/lessons/044-remote-dial/Servo/Servo.ino`. The Arduino IDE shows them
  as a folder of two examples. `make pins` holds each sketch to its own
  board's circuit, and `make size` lists them both.
- **The page** gives every marker a board's letter: `<!-- bench A -->`,
  `<!-- closeup A -->`, `<!-- steps A -->`, `<!-- connections A -->`,
  `<!-- measure A -->` and `<!-- sketch A -->`, and the same with `B`. A
  marker without a letter, or with a letter the circuit has no board for,
  is an error.
- **The build steps** carry each board on from the same board in the lesson
  before, "Keep from Lesson 44's Board A: …". After a one-board lesson,
  Board A carries on from its bench and Board B starts from nothing.
- **Make.** `make 044-remote-dial` compiles both sketches; each board has
  its own pair of targets, named by its sketch in lower case:
  `make 044-remote-dial-servo` and
  `make upload-044-remote-dial-servo PORT=/dev/ttyACM1`.

## Describing a build

`docs/_theme/bench.py` explains every call; these are the ones that keep
lessons alike. [The kit page](kit.md#breadboard-homes) gives each part's home.

- **Power.** Never wire the Mega's 5V or GND to a rail: the site does it the
  same way in every lesson, from the outer GND at the end of the long header
  into B-3 and the outer 5V at its top into T+3, and joins the other rail of
  a pair at the far end (B-60 to T-60, T+61 to B+61) only when a part uses
  it. Those two header pins are kept for the rails. With the power module,
  which sits at the right end, set its top jumper off: the Mega's 5V then
  feeds the top rails, for the screen and the sensors, whose signals come
  from the Mega too, so nothing is ever powered backwards through its
  inputs; the module feeds only the bottom rails, for motors and servos (or
  3.3 V for LoRa modems); and the Mega's GND joins them all at B-3.
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
  `(x, y)` in inches. Wires run on a 0.05 inch grid, so two points closer
  than that are an error.
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
  The `SDA` and `SCL` pins by AREF are pins 20 and 21.
- **Module pins** go by the names printed on the module, or by a common
  other name: `GND` for −, `S` for `OUT`, `VCC` for + or `VDD`. A voltage
  only finds a pin that takes it: `servo.5V` is the servo's +, but
  `lora_modem.5V` is an error, since the modem's VDD takes 3.3 V.
- **The power module** sits at the right end and sets each pair of rails
  with its own jumper: `bench.power_module ("right", top="5V",
  bottom="off")` powers the top rails at 5 V and leaves the bottom + rail
  unpowered. Either side may also be `"3.3V"`.

A circuit that could never work stops the site: a Mega pin whose wire
reaches no part's leg or module's pin, a pin joined straight to GND, 5V or
3.3V, a part with two of its legs in one strip, and a label with no room
of its own in a drawing.

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
of the reading, what to expect, where each probe goes and when to take it.
Say in the text how to set the meter, and slow the sketch down when a
reading needs something to stay on.

## The board package

`make site` also writes `package_adk_index.json` and the ADK Boards archive
into the site. The package takes the library's version: when
`library.properties` changes version, change `boards/avr/platform.txt` to
match, or the site will not build. To add a compiler for another computer,
run the Toolchain workflow and add the entries it prints to
`boards/toolchain.json`; `make toolchain` reads it too.

The site goes live from main, and Boards Manager never fetches a version it
already has, so a published version must never change. `boards/published.txt`
records each one's archive checksum and compiler, and `make site` fails if
the version it builds differs from its record. To change `boards/avr` or the
compiler, raise the version in `library.properties` and `platform.txt`, run
`make site`, and add the line it prints. The archive holds only files git
tracks.
