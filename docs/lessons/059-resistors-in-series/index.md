---
lesson: 59
promise: See two resistors share 5 V, then change one resistor and watch the shares change.
time: 25 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 2 × 1 kΩ resistors (brown, black, black, brown, brown)
  - 2 kΩ resistor (red, black, black, brown, brown)
  - 4 jumper wires
  - Digital multimeter with DC volts (add-on, not in the kit)
ideas:
  - Parts in one path carry the same current
  - Series resistors share the supply voltage
---

## What you'll build

<!-- closeup -->

Two resistors make one path from the Mega's USB-powered 5 V to GND. A
multimeter shows how much voltage is across each resistor. You will change
one resistor and watch the two readings change.

## Predict

The two resistors are both **1 kΩ**. If the pair has about 5 V across it,
how much voltage do you expect across each one? Write down your two guesses
and check whether they add to 5 V.

## Build it

!!! warning "Unplug first"
    Unplug the Mega's USB cable before moving parts or wires. Use only its
    USB 5 V supply for this build. Check that the + and − rails are not
    joined by a wire before plugging in.

If E03's LED circuit is still on the breadboard, remove its LED, resistor
and two short jumpers while power is unplugged. Keep the Mega's GND wire in
the bottom − rail hole nearest the Mega. Follow the generated steps for the
whole new circuit, including the standard power wires.

<!-- bench -->

<!-- steps -->

The two resistor legs that meet in column 7 share one metal strip under the
breadboard. That is the **middle point**. There is no branch there: current
has just one way through both resistors and back to GND.

<!-- connections -->

## Try it

Set the multimeter to **DC volts (V⎓)**, using the 20 V range if it has
ranges. Put the black lead in **COM** and the red lead in **V**. Keep the
leads in those sockets for every reading here. Plug in the Mega's USB cable,
then touch the probes across one resistor at a time. Keep the metal probe
tips from touching each other.

<!-- measure -->

Record all three readings: first resistor, second resistor, and the pair.
Each equal resistor should show about **2.5 V** and the pair about **5 V**.
The exact numbers may differ slightly because the USB voltage and resistor
values are not exact. Add the first two readings. Do they come close to the
reading across the pair?

## Why it happens

Both resistors are in one path, so the **same current** passes through each.
The pair is 2 kΩ altogether. With about 5 V across it, the current is about
5 V ÷ 2 kΩ = **2.5 mA**. Each 1 kΩ resistor then takes about 2.5 V. The
voltage drops add to the voltage across the whole pair.

## Change one thing

Predict first: if you replace only the **second** 1 kΩ resistor with a
**2 kΩ** resistor, which resistor will have the larger voltage across it?
Write down your guess for each reading.

Unplug USB. Put the 2 kΩ resistor in the same two holes as the second
resistor, then check the path and plug USB back in. Take the same three
voltage readings. Expect about **1.7 V** across the 1 kΩ resistor and
**3.3 V** across the 2 kΩ resistor; together they should still be about
5 V. The one path still carries the same current everywhere, now about
5 V ÷ 3 kΩ = **1.7 mA**.

## Check your result

Complete this sentence: “The 2 kΩ resistor gets about twice the voltage
of the 1 kΩ resistor because both carry the same \_\_\_\_\_\_\_\_, and the 2 kΩ
resistor has twice the \_\_\_\_\_\_\_\_.” Then compare your prediction and your
readings. The missing words are **current** and **resistance**.

This is the fixed-resistor version of the divider inside the knob in
[Lesson 7](../007-dimmer/index.md): its middle point moves as you turn it.

## If it doesn't work

| What you see | What to check |
|---|---|
| The pair reads near 0 V | Make sure USB is connected for the reading. Unplug, then check the Mega's 5 V and GND wires. |
| One resistor reads near 0 V and the other near 5 V | Unplug and check that their legs meet in one column, with each resistor's legs in different strips. |
| The readings have minus signs | Swap the red and black probes; the size of the reading should stay about the same. |
| The changed readings do not divide about 1.7 V and 3.3 V | Unplug and check the 2 kΩ resistor's bands and that you replaced only the second resistor. |

## About the sketch

This passive circuit works as soon as USB powers the Mega. It does not need
a sketch upload. If you open the matching example in the Arduino IDE, it
contains the usual ADK `setup ()` and `update ()` calls, but claims no
signal pins:

<!-- sketch -->

These are expected readings from the circuit design. They have not been
confirmed on a physical breadboard for this lesson.
