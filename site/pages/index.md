# ADK

ADK is a small C++ library and a progressive electronics course for the
Arduino Mega 2560. It treats circuit parts as objects so that wiring, resource
ownership, object lifetime, testing, and composition can be learned together.

The project is in active development. Lessons 001–003 use the current
compatibility API: `adk::initialize()`, `adk::led::Mono`, and
`adk::led::Rgb`. The newer RAII component hierarchy begins with inputs and
buttons and will replace the compatibility layer in documented stages.

## Choose a path

- [Set up ADK and build it locally](start.md)
- [Follow the course in order](lessons/index.md)
- Use ADK as an Arduino library by including `<adk.h>` and linking or installing
  this repository in an Arduino library search path

The Mega 2560 is the reference board. Host-native tests run on Arch Linux, and
the Arduino sketches are compiled with `arduino-cli`.

## What every lesson provides

Each complete lesson pairs a compilable sketch with a printable field lesson.
The field lesson includes predictions, an exact schematic where external
wiring is required, a pencil-style orientation drawing, diagnostic checks,
evidence tables, and an extension.

The drawings communicate orientation and intent. They are not substitutes for
the exact schematic, connection table, component datasheet, or board
documentation.

## Safety scope

ADK begins with low-voltage, USB-powered learning circuits. Disconnect every
power source before changing wiring. Stop immediately for heat, smoke, odor,
unexpected resets, or a host over-current warning.

Later projects may simulate operator panels and show cues using inert loads.
ADK does not provide pyrotechnic firing circuits or cloned launch protocols.
