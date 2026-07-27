# ADK

ADK is a deterministic, no-exception C++ component library and electronics
course for the Arduino Mega 2560.

The first-class API through lesson 030 is host verified and experimental.
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
`initialize()`, return an explicit `Status` value with `ok()`, `error()`, and
`transient()`, clean up through idempotent
`shutdown() noexcept`, and release ownership during destruction. ADK does not
use heap allocation, exceptions, RTTI, or a hidden global dispatcher.

## Course

- 001 — `DigitalOutput` and visible diagnostics
- 002 — `DigitalInput` and pull-up wiring
- 003 — `Button` plus deterministic Reaction Timer
- 004 — `PwmOutput` and `RgbLed`
- 005 — nonblocking `PiezoSounder`
- 006 — deterministic Simon engine
- 007 — explicit `AnalogInput` sampling
- 008 — deterministic calibration and filtering
- 009 — adaptive Night Light with hysteresis and fault indication
- 010 — owned shift-register and seven-segment output
- 011 — explicit timed traffic states
- 012 — deterministic tabletop Traffic Junction
- 013 — validated DHT11 climate observations
- 014 — staged character-display output and stable records
- 015 — deterministic Environmental Station
- 016 — matrix scanning and release-gated keypad events
- 017 — bounded servo intent, versioned configuration, and safe pulse evidence
- 018 — inert access trainer with visible policy and bounded audit intent
- 019 — ultrasonic timing with explicit timeout and range validity
- 020 — motor intent with reversal dead time and stop dominance
- 021 — deterministic bench-rover supervision with inert command evidence
- 022 — owned I2C/SPI devices, explicit RTC state, and durable-record models
- 023 — mutually exclusive inert loads and deterministic watering policy
- 024 — observable greenhouse composition with durable decision records
- 025 — owned infrared capture with classic NEC evidence and stable records
- 026 — exact receive-only telemetry packets, freshness, and visible evidence
- 027 — deterministic telemetry health, selection, acknowledgement, and records
- 028 — inert channel observations with explicit fault and freshness evidence
- 029 — deterministic inert cue scheduling and bounded replayable audit records
- 030 — physically inert show-cue simulation with continuity-gated audit replay
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

Long-range research also explores 8K HDMI transport and a full USB 3 matrix
over switched networks. Those are explicit FPGA/SoC and Linux-class data-plane
investigations; the Mega 2560 is a control and observation endpoint, not the
high-speed bridge.

MIT licensed.
