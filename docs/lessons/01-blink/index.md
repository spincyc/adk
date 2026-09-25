---
lesson: 1
title: Blink
arc: First light
promise: Make a light blink, and write the program that tells it to.
time: 30 minutes
level: 1
sketch: Lesson01Blink
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
program, and sent it to a computer the size of a biscuit.

## The idea

Every numbered pin on the Mega is a switch your program controls. Switched
**on**, the pin connects to 5 volts; switched **off**, to 0 volts, which is
called ground, or **GND**.

Electricity only flows around a complete loop. In this circuit it leaves pin
26, goes through a resistor, through the LED, and back into the Mega at GND.
Break the loop anywhere and the LED goes dark.

An LED is a one-way street. Current flows in at its **long leg** (+) and out
at its **short leg** (−), and the rim of the LED has a flat edge on the short
leg's side. Put one in backwards and it simply stays dark; nothing is harmed.

An LED on its own would let far too much current through and could damage
the pin. The **resistor** sets the current. A red LED keeps about 2 V for
itself, so the resistor has the other 3 V across it, and Ohm's law gives the
current:

<p class="formula">current = <span class="fraction"><span>5 V − 2 V</span><span>220 Ω</span></span> ≈ 14 mA</p>

That is bright, and comfortably below the 20 mA a Mega pin is happy to give.

!!! question "Predict"
    If you swapped the 220 Ω resistor for a 1 kΩ one (brown, black, black, brown, brown), would
    the LED be brighter, dimmer, or the same? Write down your guess. You can
    test it at the end.

## Build it

!!! warning "Unplug first"
    Always unplug the USB cable before you change any wiring, and check your
    wiring before you plug it back in.

<!-- bench -->

<!-- steps -->

??? info "How a breadboard joins things"
    Under each column of five holes, **a** to **e**, is one metal strip, so
    anything pushed into those five holes is joined. Rows **f** to **j** are
    a separate strip, across the gap. The long rows along the top and bottom
    edges, marked **+** and **−**, are the rails: each runs the whole length
    of the board, ready to carry 5 V and GND to wherever they're needed.

    That is why the circuit climbs down column 6. The orange wire (j6) and
    the resistor's top leg (g6) share the upper strip; the resistor stands
    across the gap, and its lower leg (e6) shares the lower strip with the
    LED's long leg (b6). The LED's short leg (b7) and the black jumper (a7)
    share column 7's strip, and the jumper carries it to the − rail, which
    the black wire from the Mega's GND joins at its first hole.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → Lesson01Blink**:

<!-- sketch -->

Read it from the top:

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

## Upload it

1. Plug the Mega into your computer with the USB cable.
2. In the Arduino IDE, choose **Tools → Board → ADK Boards → ADK Mega 2560**,
   and the port it appears on under **Tools → Port**. If the board isn't
   there, [Getting started](../../start.md) shows how to add it.
3. Press **Upload** (the arrow button). After a few seconds the IDE says
   *Done uploading*.

The LED should flash: on for half a second, off for half a second.

You predicted what a 1 kΩ resistor would do. It makes the LED dimmer: with
nearly five times the resistance, only about a fifth of the current flows,
(5 V − 2 V) ÷ 1000 Ω = 3 mA. Try it in the last challenge below.

!!! tip "From the command line"
    With `arduino-cli` installed, `make upload EXAMPLE=Lesson01Blink` in the
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
    flashes for ones, so a wiring mistake in the code can never quietly drive
    the wrong pin.

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
4. **Test your prediction.** Swap the 220 Ω resistor for a 1 kΩ one, and see
   how much dimmer 3 mA is than 14 mA.

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
