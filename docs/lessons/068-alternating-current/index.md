---
lesson: 68
promise: Watch a resistor's voltage cross zero as current changes direction.
time: 20 minutes
level: 1
parts:
  - Arduino Mega 2560 (USB cable unplugged)
  - Breadboard
  - Battery-powered, isolated waveform generator with 0–4 V sine output
  - Battery-powered two-channel oscilloscope
  - 1 µF nonpolar film capacitor
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 3 jumper wires and 2 female-to-male wires
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
    Mega input, the 5 V rail, or an AA pack. Keep both scope ground clips
    on the common bottom − rail, never on either signal point.

Remove the parts and wires from the previous build. Keep the Mega's GND
wire in the bottom − rail hole nearest it. Leave USB unplugged and do not
connect the Mega's 5 V rail. The generator's GND joins that bottom − rail.
Use a **nonpolar film** capacitor; either lead can face the generator.

<!-- bench -->

<!-- steps -->

Trace the complete path: generator OUT → capacitor → 1 kΩ resistor →
bottom − rail → generator GND. The resistor and capacitor each cross the
breadboard's middle gap.

<!-- connections -->

## Watch both voltages

1. With the generator output still off, attach **both scope ground clips**
   to free holes on the bottom − rail. Put channel 1's tip at **j6**, the
   generator side of the capacitor. Put channel 2's tip at **j10**, the
   top of the resistor. Set both channels to **DC coupling**.
2. Set the isolated generator to a **1 kHz sine wave from 0 to 4 V**:
   4 V peak-to-peak with a **+2 V DC offset**. Check those settings before
   enabling its output. Wait for the initial transient to settle.
3. Record each trace's highest and lowest voltage. Channel 1 should stay
   near **0–4 V**. Channel 2 should swing **above and below 0 V**. Compare
   what you see with your prediction, then turn the output off.

The capacitor blocks the generator's steady +2 V offset after it settles.
As the sine wave rises, current through the resistor goes toward GND, so
the top of the resistor is positive. As it falls, current reverses and
the top becomes negative relative to GND. The capacitor is what lets this
happen even though generator OUT itself stays between 0 and 4 V.

## If the traces surprise you

- **Channel 1 goes below 0 V:** Recheck the generator's +2 V offset and
  4 V peak-to-peak setting, and keep channel 1 on DC coupling.
- **Channel 2 stays above 0 V:** Check its DC coupling, its tip at j10,
  and that the capacitor really separates j6 from the resistor's strip.
  Wait for the initial transient to fade.
- **No trace appears:** With output off, check the OUT-to-j6 wire and
  generator GND-to-bottom − rail wire. Keep both scope ground clips on
  that same rail, then turn the output on again.

## About the sketch

This passive circuit needs no upload. Its matching sketch claims no Mega
signal pins, and the Mega's USB cable can stay unplugged:

<!-- sketch -->

This alternating current is related to the changing waveform behind
[Lesson 5's buzzer](../005-melody-maker/index.md). The expected traces
come from the circuit model and calculations; this lesson has not been
checked on physical hardware.
