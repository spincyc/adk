---
title: Build real circuits
hide:
  - navigation
  - toc
---

<div class="hero" markdown>
<div markdown>

# Build real circuits. Understand every line.

Thirty-six hands-on lessons for the Arduino Mega 2560 and the parts in the
Elegoo starter and sensor kits, six more with add-on radios, and twelve that
join two boards across a house. Start with one blinking LED; finish with
games, musical instruments, a weather station, a door that opens for the
right card, and a game of Pong played between two rooms.
{ .hero-lead }

[Get set up](start.md){ .md-button .md-button--primary }
[See the whole course](course.md){ .md-button }

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
- **Nothing blocks.** `adk::update ()` keeps buttons debounced, melodies
  playing and displays lit, so there is no juggling of `delay ()`.

</div>

## Eighteen builds, three lessons each

Two lessons each introduce a part; the third puts them together into
something worth showing off.

<!-- arcs -->

## Drawings you can trust

Every breadboard drawing is generated from the same description of the
circuit that the site checks against the lesson's code. If a wire in the
picture went to a different pin from the one the sketch uses, the site would
refuse to build. What you see is what the code expects.

[What's in the kit](kit.md){ .md-button }
[Safety first](safety.md){ .md-button }
