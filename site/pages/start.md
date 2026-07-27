# Start with ADK

## What you need

- An Arduino Mega 2560
- A known-good USB data cable
- An Arch Linux development computer
- This ADK checkout

Lessons with external circuits list their additional parts on their landing
pages and in their printable PDFs.

## Prepare an Arch Linux checkout

From the repository root:

```sh
make bootstrap
```

This explicit, Arch-only target performs a normal
`pacman -Syu --needed` transaction using official repository packages, installs
the Arduino AVR core, and reports whether serial-device permissions need
attention. Ordinary build targets never install packages.

Build and run the native tests and style checks:

```sh
make
```

Compile every lesson for the Mega 2560:

```sh
make arduino
```

Build and validate the printable lessons:

```sh
make lessons-check
```

## Upload one lesson

Connect the board, identify its serial device, and upload explicitly:

```sh
make upload LESSON=001 PORT=/dev/ttyACM0
```

Replace the lesson number and port as needed. ADK does not commit a
machine-specific port and does not upload as part of an ordinary build.

## Use ADK as a library

Install or link the repository in an Arduino library search path. Current
programs include the public umbrella header and initialize the compatibility
object registry during `setup()`:

```cpp
#include <adk.h>

adk::led::Mono status (LED_BUILTIN);

void setup ()
{
    adk::initialize ();
}

void loop ()
{
    status.on  ();
    delay      (100);
    status.off ();
    delay      (100);
}
```

Programs using current components with periodic behavior must also call
`adk::update()` from `loop()`. Do not confuse this compatibility lifecycle
with the planned per-component `initialize()`/`shutdown()` RAII interfaces.

## Before powering a circuit

1. Disconnect USB, barrel-jack power, batteries, programmers, and powered
   modules.
2. Build from the exact schematic and verify each connection.
3. Confirm polarity and part pinouts from their datasheets.
4. Check that every LED channel has its own series resistor.
5. Remove loose conductors and inspect for shorts.
6. Apply power only after the powerless inspection passes.

Continue with [the lesson sequence](lessons/index.md).
