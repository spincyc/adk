# Build and test

ADK can be installed as an Arduino library or explored directly from its
repository. The local workflow targets Arch Linux, uses only official Arch
packages, and compiles firmware for the Arduino Mega 2560.

## Prepare Arch Linux

From the repository root:

```sh
make bootstrap
```

This explicit provisioning target runs a normal
`pacman -Syu --needed` transaction, installs the C++ and documentation tools,
installs the official Arduino AVR core, and checks access to connected serial
devices. Ordinary build targets never install packages or use `sudo`.

## Common targets

| Command | Result |
|---|---|
| `make` or `make check` | Build and run host tests, then run the project style checker |
| `make host-test` | Build and run native deterministic tests |
| `make style-check` | Check project rules that clang-format cannot express |
| `make format-check` | Verify clang-format without changing files |
| `make format` | Format C++ sources, lessons, and tests |
| `make arduino` | Compile every supported example for the Mega 2560 |
| `make arduino-Lesson001DigitalOutput` | Compile one named example |
| `make boards` | List connected boards and their exact ports |
| `make upload EXAMPLE=... PORT=...` | Compile and upload one explicit example |
| `make monitor PORT=...` | Watch timestamped serial output interactively |
| `make serial-log PORT=...` | Watch and save timestamped serial output |
| `make lessons` | Build every lesson PDF |
| `make lessons-check` | Build PDFs and validate their basic structure and size |
| `make clean` | Remove the marked local build directory |

The host build uses C++17, size-oriented optimization and link-time
optimization. It disables exceptions and RTTI to verify ADK's internal
contract. ADK objects are nevertheless designed to clean up correctly when an
exception-enabled application unwinds through them.

## Upload explicitly

Uploading is never part of a default build:

```sh
make upload EXAMPLE=Lesson001DigitalOutput PORT=/dev/ttyACM0
```

Inspect the board, wiring, lesson safety gate, selected example, and serial port
before uploading. No developer-specific port or installation path is committed.

Serial is an optional second view:

```sh
make monitor PORT=/dev/ttyACM0 BAUD=115200
make serial-log PORT=/dev/ttyACM0 \
    SERIAL_LOG=build/serial/lesson001.log
```

Stop monitoring with Ctrl-C. Every lesson still requires a circuit-native
signal or test point; a serial message alone is not acceptance evidence.

## Use ADK from another project

Place or link the repository in an Arduino library search path and include:

```cpp
#include <Adk.h>
```

The library metadata currently declares AVR support. The Mega 2560 is the
reference board and the only board assumed by the lesson wiring and acceptance
checks.
