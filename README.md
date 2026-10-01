# ADK

**Build real circuits on an Arduino Mega 2560, and understand every line.**

ADK has fifty-five project lessons, from a blinking LED to a game of Pong
played between two boards over radio, plus twenty-four electricity
investigations that run alongside them. Its small C++ library makes the
project builds simple. Every lesson is a
web page and a printable PDF, with pencil drawings of the breadboard that
are checked against the code's pins.

**[Start the course →](https://spincyc.github.io/adk/)** ·
**[Explore electricity →](https://spincyc.github.io/adk/electricity/)** ·
**[Teach it →](https://spincyc.github.io/adk/teachers/)**

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
  and accelerometer: everything in the Elegoo Mega and sensor kits. And a
  few add-on radios: an FM radio, 433 MHz modules, and LoRa radios that
  reach a kilometer or a phone, with a bridge that keeps numbers the same
  on two boards.
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

ADK is C++23, newer than the Arduino IDE's own compiler, so it comes with a
board package: the same Mega 2560, built with avr-gcc 16.

1. In Arduino IDE 2, add
   `https://spincyc.github.io/adk/package_adk_index.json` under **File →
   Preferences → Additional boards manager URLs**.
2. In **Tools → Board → Boards Manager**, install **ADK Boards** (and
   **Arduino AVR Boards**, if the IDE hasn't already).
3. **Code → Download ZIP** on this page, then **Sketch → Include Library →
   Add .ZIP Library**. Every lesson's sketch is then under **File → Examples
   → Adk → lessons**, from `001-blink` on.
4. Choose **Tools → Board → ADK Boards → ADK Mega 2560**, and the port.

The compiler comes for Windows, macOS (Apple silicon and Intel) and Linux
(x86-64 and ARM). [Getting started](https://spincyc.github.io/adk/start/)
has the details.

## Build

Everything builds with `make` and lands in `build/`.

```sh
make deps                                # install what the build needs (Arch Linux; others: a list)
make test                                # host tests
make examples                            # compile every lesson for the Mega
make 001-blink                           # compile one lesson; make lessons lists them all
make upload-001-blink PORT=/dev/ttyACM0  # compile it and upload it to the Mega
make site                                # the website, in build/site
make pdf                                 # every lesson as a PDF
make check                               # what CI runs on Linux
make help                                # everything else
```

[Contributing](https://spincyc.github.io/adk/contributing/) explains the
layout, [How ADK works](https://spincyc.github.io/adk/ARCHITECTURE/) the
design, and [the style guide](docs/STYLE.md) the code.

## Status

All seventy-nine lessons are written, each with its sketch and a build drawn
from one description. The site can also make each as a PDF. The library is
host-tested, and every sketch compiles for the Mega. The circuit results are
predictions and have not yet been checked on physical hardware. If you build
a lesson, [report your build](https://spincyc.github.io/adk/builds/), whether
it works or not.

## License

[MIT](LICENSE). The bundled fonts are under the SIL Open Font License; see
[docs/assets/fonts](docs/assets/fonts).
