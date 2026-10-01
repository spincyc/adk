---
lesson: 68
promise: Watch a resistor's voltage cross zero as current changes direction.
time: 20 minutes
level: 3
parts:
  - Arduino Mega 2560 (USB cable unplugged)
  - Breadboard
  - Battery-powered, isolated waveform generator with 0–4 V sine output
  - Battery-powered two-channel oscilloscope
  - 1 µF nonpolar film capacitor
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 3 jumper wires (and one spare for the comparison)
ideas:
  - An alternating signal drives current first one way, then the other
---

## What you'll build

<!-- closeup -->

An isolated generator sends a **0–4 V sine wave at 1 kHz** through a
capacitor and a resistor in series. A two-channel scope compares the
generator output with the voltage across the resistor. The Mega can stay
unplugged; it holds the familiar ground wire, but no Mega input is wired.

## Predict

The generator output never goes below 0 V. Before connecting the scope,
predict whether the voltage at the **top of the resistor** can go below
0 V. If it does, which way must current pass through the resistor then?

## Build it

!!! warning "Switch off before wiring"
    Turn the generator output off and unplug the Mega's USB cable before
    moving anything. Use only a battery-powered, isolated generator set to
    **0–4 V**; do not use a negative supply or connect its output to a
    Mega input or the 5 V rail. Keep both scope ground clips on the common
    bottom − rail, never on either signal point.

Start with an empty breadboard. The complete steps below put the Mega's
GND wire in its usual bottom − rail hole nearest it and join the
generator's GND to that rail. Leave USB unplugged and do not connect the
Mega's 5 V rail. Use a **nonpolar film** capacitor; either lead can face
the generator.

<!-- bench -->

<!-- steps -->

Trace the complete path: generator OUT → capacitor → 1 kΩ resistor →
bottom − rail → generator GND. The resistor and capacitor each cross the
breadboard's middle gap.

<!-- connections -->

## Try it

If this is your first time with the scope and generator, read the
[scope and generator primer](../../electricity/skills.md#scope-and-generator)
and do its output check before you connect the generator to this circuit.

The two probes go as drawn here: each tip in a free hole, each ground clip
in a free hole of the bottom − rail.

<!-- probe -->

1. With the generator output still off, clip **both scope ground leads**
   to the bottom − rail, by columns 6 and 9. Put channel 1's tip in **i6**,
   on the generator side of the capacitor. Put channel 2's tip in **i10**,
   at the top of the resistor. Set both channels to **DC coupling**.
2. Set the isolated generator to a **1 kHz sine wave from 0 to 4 V**:
   4 V peak-to-peak with a **+2 V DC offset**. Check those settings before
   enabling its output. Wait for the initial transient to settle.
3. Record each trace's highest and lowest voltage. Channel 1 should stay
   near **0–4 V**. Channel 2 should swing **above and below 0 V**, about
   2 V each way. Turn the output off.

## Why it happens

Within a few milliseconds the capacitor charges to the generator's
**2 V average** and then holds it, so channel 2 is roughly the input with
that 2 V taken away. While the input is **above** its 2 V average, current
flows down through the resistor toward GND and the top of the resistor is
positive. While the input is **below** 2 V, the current reverses and the
top of the resistor is negative relative to GND, by up to about 2 V. The
capacitor is what lets this happen even though generator OUT itself stays
between 0 and 4 V. At 1 kHz, channel 2's peaks come very slightly before
channel 1's, by about a fortieth of a cycle (25 µs); you may not notice it.

## Change one thing

Predict first: if the capacitor is replaced by a plain wire, will channel
2 still go below 0 V?

1. **Turn the generator output off.** Lift out only the capacitor and
   bridge its two strips with a spare jumper from **g6 to e6**, the holes
   its legs used.
2. Turn the output on with the same settings. Channel 2 should now match
   channel 1: a 0–4 V wave that never goes below 0 V.
3. Turn the output off and put the capacitor back in g6 and e6.

Without the capacitor, the generator's 2 V average reaches the resistor
too. The current still grows and shrinks, up to about 4 mA, but it always
flows toward GND: it never reverses.

## Check your result

Compare channel 2's lowest voltage, with the capacitor and with the wire,
with your predictions. In one sentence, explain how the capacitor lets
the current reverse while the generator's output never goes below 0 V.

## If it doesn't work

- **Channel 1 goes below 0 V:** Recheck the generator's +2 V offset and
  4 V peak-to-peak setting, and keep channel 1 on DC coupling.
- **Channel 2 stays above 0 V:** Check its DC coupling, its tip in i10,
  and that the capacitor really separates j6 from the resistor's strip.
  Wait for the initial transient to fade.
- **No trace appears:** With output off, check the generator's blue OUT
  lead in j6 and its black GND lead in the bottom − rail. Keep both scope ground clips on
  that same rail, then turn the output on again.

## About the sketch

This passive circuit needs no upload. Its matching sketch claims no Mega
signal pins, and the Mega's USB cable can stay unplugged:

<!-- sketch -->

This alternating current is related to the changing waveform behind
[Lesson 5's buzzer](../005-melody-maker/index.md). The expected traces
come from the circuit model and calculations; this lesson has not been
checked on physical hardware.
