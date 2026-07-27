# ADK

ADK is a small C++ library and progressive electronics course for the Arduino
Mega 2560. The firmware remains compatible with the AVR toolchain; native host
tests use C++17. ADK models circuit parts as non-copyable component objects,
then uses each interface to teach the physical circuit, resource ownership,
lifetime, testing, and composition.

ADK is available under the [MIT License](LICENSE).

The project works either as an Arduino library or as a standalone checkout on
Arch Linux.

## Start locally

Provision the complete Arch Linux development environment:

```sh
make bootstrap
```

`bootstrap` is an explicit Arch-only provisioning action. It performs a normal
full `pacman -Syu --needed` transaction, installs only official repository
packages, installs the official Arduino AVR core, and reports whether the
current user needs membership in Arch's `uucp` serial-device group. Ordinary
build and test targets never invoke it.

Then run the host checks or compile all Mega 2560 lessons:

```sh
make
make arduino
```

If dependencies are managed separately, host checks require a C++17 toolchain
and Python; firmware builds require `arduino-cli` with the `arduino:avr` core.

Upload is always explicit:

```sh
make upload LESSON=001 PORT=/dev/ttyACM0
```

No port or developer-specific installation path is committed.

## Make targets

| Target | Purpose |
|---|---|
| `make` or `make check` | Build and run host tests and project style checks |
| `make arduino` | Compile every lesson for the Mega 2560 |
| `make lessons` | Generate all lesson PDFs |
| `make lessons-check` | Generate PDFs and validate their basic structure and size |
| `make site` | Build the GitHub Pages artifact in `build/site` |
| `make site-check` | Build and validate site structure, links, assets, and PDFs |
| `make site-serve` | Preview the site at `http://127.0.0.1:8000` |
| `make style-check` | Enforce ADK rules that ClangFormat cannot fully express |
| `make clean` | Remove generated build intermediates |

The default target does not compile firmware or generate PDFs.

## Use as a library

Install or link this repository in an Arduino library search path, then:

```cpp
#include <adk.h>

adk::led::Mono status(LED_BUILTIN);

void setup()
{
    adk::initialize();
}

void loop()
{
    status.on  ();
    delay      (100);
    status.off ();
    delay      (100);
}
```

Current examples are in `lessons/001` through `lessons/003`. Their printable
lesson plans are built with `make lessons`. Programs using components with
periodic behavior must also call `adk::update()` from `loop()`.

## Project map

- `src/` — installable Arduino library
- `lessons/` — compilable Mega 2560 sketches
- `docs/lessons/` — lesson sources and pencil orientation plates
- `doc/lessons/` — generated PDFs
- `tests/` — host-native hardware fakes and regression tests
- `mk/` — focused Make fragments
- `docs/` — architecture, style, research, and roadmap

Read [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) before adding a component.
