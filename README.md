# ADK

ADK is a deterministic, no-exception C++ component library and electronics
course for the Arduino Mega 2560.

The first-class API through lesson 006 is host verified and experimental.
Physical Mega 2560 acceptance is still open and tracked separately; a
successful firmware build is not a bench result. The original preview is
frozen under `legacy/`.

## First circuit

```cpp
#include <Adk.h>

adk::Runtime       runtime;
adk::DigitalOutput led (runtime.resources (), LED_BUILTIN);

void setup ()
{
    led.initialize ();
}

void loop ()
{
    led.write (adk::Level::High);
    delay     (500);
    led.write (adk::Level::Low);
    delay     (500);
}
```

`Runtime` owns fixed resource claims. Components are inert until
`initialize()`, return explicit `Status`, clean up through idempotent
`shutdown() noexcept`, and release ownership during destruction. ADK does not
use heap allocation, exceptions, RTTI, or a hidden global dispatcher.

## Course

- 001 — `DigitalOutput` and visible diagnostics
- 002 — `DigitalInput` and pull-up wiring
- 003 — `Button` plus deterministic Reaction Timer
- 004 — `PwmOutput` and `RgbLed`
- 005 — nonblocking `PiezoSounder`
- 006 — deterministic Simon engine
- Every third lesson is a multi-component project

See the [live course](https://spincyc.github.io/adk/),
[canonical curriculum](docs/CURRICULUM.md), and
[project briefs](docs/PROJECTS.md).

## Arch Linux

```sh
make bootstrap
make help
make boards
make check
make arduino
make lessons
make site
```

Upload an example:

```sh
make upload EXAMPLE=Lesson001DigitalOutput PORT=/dev/ttyACM0
```

Observe or record optional serial diagnostics:

```sh
make monitor PORT=/dev/ttyACM0 BAUD=115200
make serial-log PORT=/dev/ttyACM0 SERIAL_LOG=build/serial/lesson001.log
```

Circuit-native evidence remains authoritative. See the
[complete command-line workflow](docs/CLI.md).

## Contracts

- [Command-line workflow](docs/CLI.md)
- [Development hierarchy](docs/DEVELOPMENT.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Components](docs/COMPONENTS.md)
- [Projects](docs/PROJECTS.md)
- [Testing](docs/TESTING.md)
- [Safety](docs/SAFETY_MODEL.md)
- [Style](docs/STYLE.md)
- [Packaging](docs/PACKAGING.md)
- [PDF policy](docs/PDF_POLICY.md)

The fireworks capstone is an inert cue simulator only. ADK does not control
igniters or launchers and does not clone or transmit unknown remote protocols.

MIT licensed.
