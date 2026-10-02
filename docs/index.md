---
title: Build real circuits
hide:
  - navigation
  - toc
---

<div class="hero" markdown>
<div markdown>

# Build real circuits. Understand every line.

Fifty-five project lessons for the Arduino Mega 2560 start with one blinking
LED and grow into games, musical instruments, a weather station, radio links
and a game of Pong played between two rooms. A parallel set of twenty-four
electricity investigations starts with a DC loop and works through measured
circuits, alternating signals and digital logic.
{ .hero-lead }

[Get set up](start.md){ .md-button .md-button--primary }
[See the whole course](course.md){ .md-button }
[Follow the guided route](guided.md){ .md-button }
[Explore electricity](electricity/index.md){ .md-button }

Already set up? [Start with Lesson 1](lessons/001-blink/index.md).
{ .hero-aside }

</div>

<!-- drawing 001-blink closeup -->

</div>

## Code that reads like the circuit

Each part of your circuit is one line of code. ADK checks the wiring in your
code before anything runs, and keeps every part working while your program
gets on with the interesting bits.

<div class="split" markdown>

```cpp title="A button that switches an LED"
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

- **One line per part.** `adk::Led led {26};` is an LED on pin 26. There is a
  part for everything in the kits: buttons, buzzers, displays, motors,
  servos, sensors, the keypad, the remote control and the RFID reader.
- **Mistakes found early.** If two parts share a pin, `adk::setup ()` stops
  and blinks the pin number on the Mega's own LED.
- **Parts update together.** `adk::update ()` debounces buttons, advances
  melodies and refreshes displays. Some devices briefly block other
  updates and can make multiplexed displays flicker; see the
  [timing limits](ARCHITECTURE.md#blocking) when combining parts.

</div>

## Two paths through the course

The project path has eighteen three-lesson arcs and one extra. The
electricity path has eight three-investigation modules. Start either path
at its first lesson and keep each build's parts in their home positions.

The cards below show the project path. See the [Electricity syllabus](electricity/index.md)
for E01–E24 and the equipment each module needs. Teaching a class? The
[teacher guide](teachers.md) has a plan for each arc, what the room needs
and how to print the lessons as worksheets.

<!-- arcs -->

## Drawings checked against the code

Every breadboard drawing is generated from the same description of the
circuit that the checks hold the lesson's code to. If a wire in the
picture went to a pin the sketch doesn't use, or a button sat where the
sketch drives an LED, those checks would fail, and the site would not be
published with the mistake. They can't tell two parts of the same kind
apart, though: swap the wires of two LEDs and the checks still pass.

!!! note "Not yet built on a real bench"
    The sketches compile and the library's tests pass, but nobody has yet
    built these lessons on real hardware and recorded the result. Until
    someone does, what each lesson says you'll see is a careful
    prediction. If you build one, whether it works or not, please
    [report your build](builds.md).

[What's in the kit](kit.md){ .md-button }
[Safety first](safety.md){ .md-button }
