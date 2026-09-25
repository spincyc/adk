# ADK

**Build real circuits on an Arduino Mega 2560, and understand every line.**

ADK is thirty-six hands-on lessons, from a blinking LED to a tilt-controlled
maze, and the small C++ library that makes them simple. Every lesson is a
web page and a printable PDF, with pencil drawings of the breadboard that
match the code pin for pin.

**[Start the course →](https://spincyc.github.io/adk/)**

```cpp
#include <Adk.h>

adk::Led    led    {26};
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

## The library

- **One line per part.** LEDs, RGB LEDs, buzzers, buttons, switches and
  sensor modules, knobs, keypads, rotary encoders, joysticks, seven-segment
  displays, an LCD, an LED matrix, servos, steppers, DC motors, relays, the
  ultrasonic ranger, DHT11, DS18B20, IR remote, RFID reader, real-time clock
  and accelerometer: everything in the Elegoo Mega and sensor kits.
- **Mistakes found early.** `adk::setup ()` checks that every pin exists, is
  used once, and can do what its part needs. If not, it stops and blinks the
  pin number on the Mega's own LED, or explains in words with
  `adk::setup (Serial)`.
- **Nothing blocks.** `adk::update ()` keeps buttons debounced, melodies
  playing, displays refreshed and sensors read, and `adk::wait ()` is a
  `delay ()` that keeps them all going.
- **Small.** No heap, no exceptions, no Arduino libraries: a part you don't
  declare costs nothing. Blink is 2.9 KB of flash and 76 bytes of RAM.
- **Tested.** Every part has host tests against a fake Arduino core, run
  under the address and undefined-behavior sanitizers, and every example
  compiles for the Mega with all warnings on.

## Install

In the Arduino IDE: **Code → Download ZIP** on this page, then **Sketch →
Include Library → Add .ZIP Library**. Every lesson's sketch is then under
**File → Examples → Adk**. [Getting started](https://spincyc.github.io/adk/start/)
has the details.

## Build

Everything builds with `make` and lands in `build/`.

```sh
make test       # host tests
make examples   # compile every lesson for the Mega (needs arduino-cli)
make site       # the website, in build/site
make pdf        # every lesson as a PDF (needs Chromium)
make check      # all of it, as CI runs it
make help       # everything else
```

[Contributing](https://spincyc.github.io/adk/contributing/) explains the
layout, [How ADK works](https://spincyc.github.io/adk/ARCHITECTURE/) the
design, and [the style guide](docs/STYLE.md) the code.

## Status

The library is complete and host-tested, and every example compiles for the
Mega. Nothing here has yet been checked on a physical board; lessons say so
until they have been. Lesson 1 is written; the other thirty-five are being
written in order.

## License

[MIT](LICENSE). The bundled fonts are under the SIL Open Font License; see
[docs/assets/fonts](docs/assets/fonts).
