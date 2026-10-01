---
lesson: 1
promise: Make a light blink, and write the program that tells it to.
time: 30 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Red LED
  - 220 Ω resistor (red, red, black, black, brown)
  - 3 jumper wires
ideas:
  - Pins you can switch on and off
  - Which way round an LED goes
  - Why an LED needs a resistor
  - setup () and loop ()
---

## What you'll build

<!-- closeup -->

A red LED on your breadboard flashes on for half a second, off for half a
second, for as long as the Mega has power. It is the "hello, world" of
electronics: once it blinks, you have wired a working circuit, written a
program, and sent it to a computer the size of a candy bar.

## The idea

In this build, pin 26 acts like a switch your program controls. When it is
**on**, it is about 5 volts above ground (**GND**); when it is **off**, it is
at 0 volts. A voltage is always measured between two places.

Electricity only flows around a complete loop. In this circuit it leaves pin
26, goes through a resistor, through the LED, and back into the Mega at GND.
Break the loop anywhere and the LED goes dark.

An LED lets current through only one way: in at its **long leg** (+) and out
at its **short leg** (−). The flat edge of its rim marks the short-leg side.
If you put it in backwards, it stays dark. The **resistor** limits the current
so the LED and the Mega's pin stay safe. Keep it in the circuit.

Want to see the same loop work **before writing code**? Try
[E01–E03](../../electricity/index.md#1-dc-paths-and-measurements): power a
steady LED, open its return path, then measure and change its resistor.

!!! question "Predict"
    If the black wire between the Mega's GND and the − rail were missing,
    would the LED still blink? Write down your guess, then test it after
    the first upload.

## Build it

!!! warning "Unplug first"
    Always unplug the USB cable before you change any wiring, and check your
    wiring before you plug it back in.

Pin 26 will supply the LED when it turns on. Connect only the bottom **−**
rail to **GND**; leave both **+** rails empty in this build.

<!-- bench -->

<!-- steps -->

??? info "How a breadboard joins things"
    Under each column of five holes, **a** to **e**, is one metal strip, so
    anything pushed into those five holes is joined. Rows **f** to **j** are
    a separate strip, across the gap. The long rows along the top and bottom
    edges, marked **+** and **−**, are the rails: each runs the whole length
    of the board. A rail carries 5 V or GND only after you connect it.
    Any hole along a rail will do, but the steps name the one by a column,
    as "the bottom − rail by column 7", to keep each wire short.

    That is why the circuit climbs down column 6. The orange wire (j6) and
    the resistor's top leg (g6) share the upper strip; the resistor stands
    across the gap, and its lower leg (e6) shares the lower strip with the
    LED's long leg (b6). The LED's short leg (b7) and the black jumper (a7)
    share column 7's strip, and the jumper carries it to the − rail, which
    the black wire from the Mega's GND joins at its first hole.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → lessons → 001-blink**:

<!-- sketch -->

Read it from the top:

- The first two lines start with `//`. Everything from `//` to the end of
  its line is a **comment**: a note for people reading the sketch, which
  the Mega ignores. Comments say what a sketch is for, or why a line is
  the way it is.
- `#include <Adk.h>` brings in the ADK library.
- `adk::Led led {26};` says there is an LED on pin 26 and names it `led`.
  In the code, each part of the circuit is an **object**: a thing with a
  name that knows how to do its job. Every part gets a line like this, at
  the top of the sketch.
- `setup ()` runs once when the Mega starts. `adk::setup ()` gets every part
  ready: here, it makes pin 26 an output and turns the LED off.
- `loop ()` runs again and again, forever.
- `led.on ();` **calls** a function: it asks `led` to switch on. The
  brackets hold whatever the function needs to know. `led.on ()` needs
  nothing, but `adk::wait (500);` needs to know how long to wait: 500
  milliseconds, half a second. A semicolon ends each instruction.
- So `loop ()` turns the LED on, waits, turns it off, and waits again: one
  blink a second, for as long as the Mega has power.

Each lesson explains the new pieces of C++ in its sketch, the first time
they appear. [The C++ you've met](../../cpp.md) lists them all, with the
lesson that explains each one, for when you want to look one up again.

## Upload it

1. Plug the Mega into your computer with the USB cable.
2. In the Arduino IDE, choose **Tools → Board → ADK Boards → ADK Mega 2560**,
   and the port it appears on under **Tools → Port**. If the board isn't
   there, [Getting started](../../start.md) shows how to add it.
3. Press **Upload** (the arrow button). After a few seconds the IDE says
   *Done uploading*.

The LED should flash: on for half a second, off for half a second.

Now test your prediction: unplug the USB cable, remove only the black wire
from the Mega's GND to the bottom − rail, and plug the cable back in. The LED
stays dark because the path back to GND is broken. Unplug again, replace the
wire in the same holes, and plug in once more. It blinks again.

!!! tip "From the command line"
    With `arduino-cli` installed, `make upload EXAMPLE=lessons/001-blink` in the
    ADK folder compiles and uploads the same sketch.

## If it doesn't work

| What you see | Try this |
|---|---|
| The LED never lights | Turn the LED round: the long leg goes in b6. |
| Still dark | Check the resistor stands in g6 and e6, the LED's long leg is in the same column (b6), the black jumper joins a7 to the − rail, and the orange wire is in pin 26, not 27. |
| The LED is always on | The orange wire may be in 5 V instead of pin 26. |
| Upload fails | Pick the right board and port in the **Tools** menu, and try a different USB cable: some only carry power. |
| The little **L** LED on the Mega blinks long and short flashes | ADK found a wiring mistake in the sketch and is blinking the pin number. See [Faults](../../library/index.md#faults). |

??? note "How it works"
    `adk::setup ()` does more than it looks. Before anything runs, it checks
    every part the sketch declared: that each pin exists, and that no two
    parts share a pin. If something is wrong it stops and blinks the pin
    number on the Mega's own **L** LED, long flashes for tens and short
    flashes for ones. It cannot see your wires: declaring pin 27 when the
    LED is wired to pin 26 still passes these checks, because 27 exists and
    is free. Check the pin numbers against your build too.

    `adk::wait (500)` pauses like Arduino's `delay (500)`, with one difference
    that matters from the next lesson on: while it waits, every part keeps
    working. A button is still watched, a melody keeps playing.

## Make it yours

1. **Heartbeat.** Make the LED beat: on 100 ms, off 100 ms, on 100 ms, off
   700 ms.
2. **SOS.** Flash the Morse code for SOS: three short, three long, three
   short. A short flash is 200 ms, a long one 600 ms, with 200 ms between
   flashes and a second's pause after each SOS.
3. **One line.** Delete everything inside `loop ()` and put
   `led.blink (1000);` at the end of `setup ()`, then add `adk::update ();`
   inside `loop ()`. The LED blinks just the same. Add
   `adk::Led builtIn {LED_BUILTIN};` and make the Mega's own LED blink too,
   at a different speed.
4. **Change the resistor.** Predict whether a 1 kΩ resistor (brown, black,
   black, brown, brown) will make the LED brighter or dimmer. Unplug, swap
   it for the 220 Ω resistor, then plug in and check. Restore the 220 Ω
   resistor before Lesson 2.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. A
meter lets you see the voltages the idea above talks about, and after a few
readings "5 V shared between the resistor and the LED" stops being words and
becomes something you have seen.

Turn the dial to DC volts (**V⎓**), on the 20 V range if yours asks for one,
plug the black lead into **COM** and the red one into **V**. Each reading
below needs the LED on, and half a second is too short to read a meter, so
change both `500`s in the sketch to `3000` and upload again: the LED now
stays on for three seconds at a time. Touch the probe tips to the holes
shown, red first, and wait for the number to settle.

<!-- measure -->

What the numbers tell you:

- **Pin 26 to GND** is the whole 5 V the pin gives while it is on. When the
  LED goes off, the reading drops to 0.
- **Across the resistor** and **across the LED** share that 5 V: add your
  two readings and you get the first one back. The LED keeps about 2 V for
  itself, whatever resistor you use; the resistor takes the rest.
- Try the 1 kΩ resistor from *Make it yours*. The LED's share barely moves,
  so the resistor still has about 3 V across it, and with five times the
  resistance the current is a fifth: that is why the LED is dimmer.

With the 220 Ω resistor, the red LED keeps about 2 V and the resistor has
the other 3 V. Ohm's law gives about (5 V − 2 V) ÷ 220 Ω = 14 mA through
both parts, below the Mega pin's 20 mA operating limit. With 1 kΩ, the
current is about 3 mA.
