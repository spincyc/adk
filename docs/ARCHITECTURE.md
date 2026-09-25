# How ADK works

ADK turns a circuit into a list of objects. A sketch declares one object per
part, calls `adk::setup ()` once, and calls `adk::update ()` at the top of
every `loop ()`.

```cpp
#include <Adk.h>

adk::Led    led    {13};
adk::Button button {22};

void setup ()
{
    adk::setup ();
}

void loop ()
{
    adk::update ();

    if (button.wasPressed ())
    {
        led.toggle ();
    }
}
```

## Objects

Every part is an `adk::Object`. Its constructor links it into one list, in
declaration order. It never touches hardware, because global constructors run
before the Arduino core has set the board up. Objects override three hooks:

| Hook | Called by | Does |
|---|---|---|
| `setup ()` | `adk::setup ()` | Claim pins and timers, and configure them. |
| `update (Millis now)` | `adk::update ()` | Advance anything that depends on time. |
| `stop ()` | `adk::stop ()` | Enter the safe state: outputs off, sound and motion stopped. |

The free functions drive the whole list:

| Function | Use |
|---|---|
| `adk::setup ()` | Once, in `setup ()`. Halts on a fault (below). |
| `adk::setup (Serial)` | The same, and prints what went wrong first. |
| `adk::update ()` | At the top of `loop ()`. Reads `millis ()` once and passes it to every object. |
| `adk::wait (ms)` | Instead of `delay ()`. Keeps every object updating while it waits. |
| `adk::stop ()` | Put everything in its safe state at once. |

An object is a whole device. Its parts (pins, a debouncer, a buffer) are plain
members, never objects of their own, so a device is set up and updated as one.

## Pins, timers, and faults

A device claims what it uses from its `setup ()`:

| Claim | Checks |
|---|---|
| `claimOutput (pin, high)` | The pin exists and is free. Sets the level, then makes it an output. |
| `claimInput (pin, pullUp)` | The pin exists and is free. |
| `claimPwm (pin, high)` | The pin can do PWM and its timer is not taken over. |
| `claimAnalog (pin)` | The pin is A0-A15 (or 0-15). |
| `claimInterrupt (pin, pullUp)` | The pin can interrupt: 2, 3, 18, 19, 20, or 21. |
| `claimShared (pin)` | Free, or already shared by a bus such as I2C. |
| `claimTimer (timer, pin)` | No PWM pin and no other device uses the timer. |

The first failed claim is a fault. `adk::setup ()` then releases every pin and
blinks the fault's pin number on the built-in LED forever: long flashes for
tens, short flashes for ones. Two long and two short means pin 22. With
`adk::setup (Serial)` it also prints a sentence such as
`adk: pin 9 needs a timer that is already in use`.

Timers matter on the Mega:

| Timer | PWM pins | Also used by |
|---|---|---|
| 0 | 4, 13 | `millis ()`; never taken over |
| 1 | 11, 12 | |
| 2 | 9, 10 | `Speaker` (Arduino `tone ()`) |
| 3 | 2, 3, 5 | |
| 4 | 6, 7, 8 | |
| 5 | 44, 45, 46 | `Servo` |

## Time

Time enters an object only through `update (now)`. An object never calls
`millis ()` itself, so a test can drive it with any timestamps it likes and get
the same result every time.

A command that starts something timed (`blink ()`, `beep ()`, `fadeTo ()`,
`play ()`) sets a flag, and the next `update ()` records the start time. The
effect that can happen immediately, such as the first flash, does.

Compare times by subtraction, `now - start >= length`, which stays correct when
`millis ()` wraps after 49 days.

## Events

Something that happens at an instant (a press, a key, a received code, a beat)
is an event. An event method is true for exactly one `update ()`, the one in
which it happened, and reading it does not clear it:

```cpp
if (button.wasPressed ())   // true for one update
if (button.isPressed ())    // true while held
```

## Blocking

`update ()` should return quickly so everything else keeps moving. A few
protocols cannot be interrupted: a DHT11 reading holds the processor for about
4 ms with interrupts off, and an HC-SR04 echo can take up to 25 ms. Those
devices do their blocking work at most once per reading period and say so in
their header. A multiplexed display will flicker while they do.

## Memory

The Mega has 8 KB of RAM and 256 KB of flash. Objects stay small: store pins
as `Pin` (one byte), times as `Millis`, and flags as `bool`. Tables larger than
a few bytes (fonts, glyphs, step sequences) live in flash with `PROGMEM`. The
library never allocates. Nothing costs anything unless a sketch declares it:
unused code is removed by the linker, and the library uses no Arduino libraries
whose global objects would be linked in anyway.

## Buses

I2C and SPI are small register-level masters in `i2c.cpp` and `spi.cpp`,
compiled only for the AVR (`#ifdef __AVR__`). Host tests link fakes instead
that play the part of each chip. Devices on a bus claim the bus pins with
`claimShared ()`; a chip-select pin is claimed exclusively.

## Testing

`make test` builds the library for the host against `tests/arduino/`, a fake
Arduino core where pins are memory and time only moves when a test says so.
Hooks let a test act as the device at the other end of a wire:

```cpp
arduino::onPulseIn = [] (uint8_t, uint8_t, unsigned long) { return 1160UL; };
adk::update (0);
CHECK (ranger.distance () == 20);
```

`make examples` compiles every example for the Mega with all warnings, and
fails on any warning from the library or an example. Neither replaces trying a
circuit on a real board.

## Adding a device

1. `src/adk/<name>.h`: a comment saying what the part is and how to wire it,
   then the declaration.
2. `src/adk/<name>.cpp`: the implementation.
3. Add the header to `src/Adk.h`.
4. `tests/<name>_test.cpp`: setup, claims, the normal behaviour, timing edges,
   and `stop ()`.
5. Use it in a lesson, and list it on the matching library page in `docs/library/`.
