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
**j6**, on the same strip as the resistor's g6 leg. In schematic form
(the [schematic key](../../electricity/schematics.md) names each symbol):

<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 600 230" width="600"
     role="img" aria-labelledby="rc-filter-title rc-filter-desc">
  <title id="rc-filter-title">RC low-pass filter schematic</title>
  <desc id="rc-filter-desc">The generator's output, the input, where channel 1 probes i6, passes through a 1 kilohm resistor to the output, where channel 2 probes i10. A 1 microfarad capacitor joins the output to ground, and the generator's ground joins the same ground.</desc>
  <g fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round">
    <circle cx="150" cy="120" r="24"/>
    <path d="M137 120 q6.5 -12 13 0 q6.5 12 13 0"/>
    <path d="M150 96 L150 60 L240 60"/>
    <path d="M240 60 L295.0 60"/>
    <path d="M365.0 60 L420 60"/>
    <rect x="295.0" y="47" width="70" height="26"/>
    <path d="M420 60 L500 60"/>
    <path d="M500 60 L500 114.0"/>
    <path d="M500 126.0 L500 180"/>
    <path d="M480 114.0 H520"/>
    <path d="M480 126.0 H520"/>
    <path d="M150 144 L150 180 L500 180"/>
    <path d="M330 180 V188 M310 188 H350 M317 196 H343 M324 204 H336"/>
  </g>
  <g fill="currentColor">
    <circle cx="240" cy="60" r="5"/>
    <circle cx="420" cy="60" r="5"/>
    <circle cx="330" cy="180" r="5"/>
  </g>
  <g fill="currentColor" font-size="17" font-family="system-ui, sans-serif">
    <text x="20" y="112">generator</text>
    <text x="20" y="132">0–4 V sine</text>
    <text x="240" y="36" text-anchor="middle">ch 1 · i6</text>
    <text x="330" y="36" text-anchor="middle">1 kΩ</text>
    <text x="420" y="36" text-anchor="middle">ch 2 · i10</text>
    <text x="530" y="126">1 µF</text>
    <text x="350" y="214">GND</text>
  </g>
</svg>

<!-- connections -->

## Try two speeds

The [scope and generator primer](../../electricity/skills.md#scope-and-generator)
explains the scope settings and peak-to-peak heights used here.

The two probes go as drawn here: each tip in a free hole, each ground clip
in a free hole of the bottom − rail.

<!-- probe -->

1. With the generator still off, clip **both scope ground leads** to the
   bottom − rail, by columns 6 and 9. Put channel 1's tip in **i6**, at
   the input, and channel 2's tip in **i10**, at the output. Use **DC
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
  in i10, not i6, and that no wire bypasses the resistor.
- **The waves seem to jump or drift:** Keep both ground clips on the
  bottom − rail, use DC coupling, and wait for the circuit to settle.

## About the sketch

This passive circuit needs no upload or Mega signal pin. Its matching
ADK sketch claims no I/O pins:

<!-- sketch -->

The traces described here are calculated expectations. This lesson has
not been recorded as tried on physical hardware.
