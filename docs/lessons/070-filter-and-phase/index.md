---
lesson: 70
promise: See a resistor and capacitor shrink a faster wave and move its peaks later.
time: 25 minutes
level: 3
parts:
  - Arduino Mega 2560 (USB cable unplugged)
  - Breadboard
  - Battery-powered, isolated waveform generator with 0–4 V sine output
  - Battery-powered two-channel oscilloscope with two probes
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 1 µF nonpolar film capacitor
  - 3 jumper wires
  - 2 female-to-male wires
ideas:
  - An RC low-pass filter reduces fast changes and delays its output
---

## What you'll build

<!-- closeup -->

Move the resistor and film capacitor from [E14](../069-frequency-and-period/index.md)
to make a **low-pass filter**. Compare the generator's 0–4 V sine wave at
the input with the voltage across the capacitor at the output. At 1 kHz,
the output wave should be smaller than at 100 Hz, and its peaks should
follow the input peaks. The Mega holds its familiar GND wire; its USB
cable stays unplugged.

## Predict

At 100 Hz, will the capacitor's voltage follow most of each wave, or
barely move? What will happen to the output wave's height when the input
speeds up to 1 kHz? Draw an input peak and mark where you expect the
output peak.

## Build it

!!! warning "Switch off before changing parts"
    Switch off and unplug the isolated generator before moving the parts
    from E14. Keep the Mega's USB unplugged. Use only a
    battery-powered, isolated generator set to **0–4 V**. Keep its OUT
    lead off the Mega's pins and + rails. Both scope ground clips go on
    the common bottom − rail, never on a signal strip.

Keep the generator's **OUT to j6** and **GND to the bottom − rail by
column 5** connections from E13–E14. Keep the Mega's GND wire in
the bottom − rail hole nearest it. Remove the capacitor at g6–e6 and
the resistor at g10–e10. Put the **1 kΩ resistor at g6–e6** and the
**1 µF nonpolar film capacitor at g10–e10**, each across the center gap.
Keep the b6-to-j10 and a10-to-bottom − rail jumpers. The film capacitor
can face either way. Check the path before reconnecting the generator.

<!-- bench -->

<!-- steps -->

The path is **generator OUT → resistor → output at the capacitor's top
strip → capacitor → common GND → generator GND**. The output is at
**j10**, on the same strip as the capacitor's g10 leg. The input is at
**j6**, on the same strip as the resistor's g6 leg.

<!-- connections -->

## Try two speeds

1. With the generator still off, clip **both scope grounds** to free
   holes in the bottom − rail. Touch channel 1's tip to the input at
   **j6** and channel 2's tip to the output at **j10**. Use **DC
   coupling** for both channels and trigger on channel 1.
2. Set the generator to a **100 Hz sine wave from 0 to 4 V**: 4 V
   peak-to-peak with a +2 V offset. Start near 2 ms/div on the scope.
   Turn on the output and wait for the traces to settle. Record the
   input and output heights (peak-to-peak) and which peak comes first.
3. Change only the generator frequency to **1 kHz**, leaving its
   0–4 V range and the wiring alone. Start near 0.2 ms/div; adjust
   channel 2's vertical scale if needed. Wait for the traces to settle
   again. Record the same two observations.

| Frequency | Input height | Output height | Which peak comes first? |
|---|---|---|---|
| 100 Hz | ____ V | ____ V | ____ |
| 1 kHz | ____ V | ____ V | ____ |

## What to expect

The output should swing around roughly **2 V** at both frequencies.
Its wave should be a little smaller than the input at 100 Hz and much
smaller at 1 kHz. At each frequency, a channel 2 peak should occur
after the matching channel 1 peak. Compare your readings with your
prediction; use the peaks' order, rather than an exact delay, to check
the phase shift.

## Why it happens

The resistor limits how quickly charge can reach and leave the
capacitor. The output is the capacitor's voltage, so it cannot follow a
rapid input change as fully as a slow one. It also reaches its peak
after the input peak. This delay relative to the input wave is called a
**phase lag**. The pair's nominal time constant is **R × C = 1 ms**;
its nominal cutoff frequency is about **159 Hz**. Real part values and
the generator and scope can shift the readings. This is a circuit
example of the smoothing idea behind [Lesson 7's dimmer](../007-dimmer/index.md).

## Check your result

Is channel 2's wave smaller at 1 kHz than at 100 Hz? At both speeds, do
its peaks come after channel 1's peaks? Those two comparisons are the
filter and phase behavior to look for. Turn off the generator when done.

## If the traces surprise you

- **Channel 1 is flat:** Check the generator's 0–4 V sine setting and
  its OUT lead at j6. Keep its GND lead in the bottom − rail by column 5.
- **Channel 2 is flat:** Check the resistor at g6–e6, b6-to-j10 jumper,
  capacitor at g10–e10, and a10-to-bottom − rail jumper.
- **Channel 2 looks just like channel 1:** Check that channel 2's tip is
  at j10, not j6, and that no wire bypasses the resistor.
- **The waves seem to jump or drift:** Keep both ground clips on the
  bottom − rail, use DC coupling, and wait for the circuit to settle.

## About the sketch

This passive circuit needs no upload or Mega signal pin. Its matching
ADK sketch claims no I/O pins:

<!-- sketch -->

The traces described here are calculated expectations. This lesson has
not been recorded as tried on physical hardware.
