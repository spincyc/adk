---
lesson: 69
promise: Count one cycle at 100 Hz and ten at 1 kHz in the same 10 ms window.
time: 20 minutes
level: 3
parts:
  - Arduino Mega 2560
  - Breadboard
  - Isolated, battery-powered 0–4 V waveform generator
  - Battery-powered two-channel oscilloscope with two probes
  - 1 µF nonpolar film capacitor
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 3 jumper wires
ideas:
  - Frequency counts cycles per second, and period is the time for one cycle
laws:
  - {law: frequency, section: why-it-happens, for: "Shows period as one second divided by frequency"}
  - {law: filter, section: why-it-happens, for: "Explains the high-pass shrinking the 100 Hz output"}
---

## What you'll build

<!-- closeup -->

Keep [E13's circuit](../068-alternating-current/index.md). Change only
the isolated generator's frequency while a scope shows the same **10 ms**
window. At **100 Hz**, expect about **one cycle**; at **1 kHz**, about
**ten cycles**. The Mega provides its usual GND wire, but no signal pins
or USB power are needed.

## Predict

One **cycle** goes from a point on the wave to the next matching point,
such as one rising crossing to the next. **Frequency** says how many
cycles happen in one second. **Period** says how long one cycle takes.
At 100 Hz, 100 cycles fit in a second, so one takes about **10 ms**.
At 1 kHz, 1000 cycles fit in a second, so one takes about **1 ms**.

Sketch a 10 ms window at 100 Hz and at 1 kHz. How many complete cycles
should each contain? Mark the time from one peak to the next on each.

## Build it

!!! warning "Power off before touching wires"
    Switch off and unplug the generator before changing any connection.
    Use only an isolated, battery-powered 0–4 V generator. Keep its output
    off the Mega's pins and the + rails. Put **both** scope ground clips
    on the common bottom − rail, never on either signal point.

Leave E13's **1 µF nonpolar capacitor at g6–e6**, **1 kΩ resistor
at g10–e10**, generator and jumpers in their holes. Keep the Mega's GND
wire in the bottom − rail hole nearest the Mega; its USB may stay unplugged.
If you need to rebuild, follow the drawing and generated steps.

<!-- bench -->

<!-- steps -->

The path is generator OUT → capacitor → resistor → common GND → generator
GND. The generator's ground and both scope clips meet at that common rail.

<!-- connections -->

## Try it

The [scope and generator primer](../../electricity/skills.md#scope-and-generator)
explains the timebase, trigger and generator settings used here.

The two probes go as drawn here: each tip in a free hole, each ground clip
in a free hole of the bottom − rail.

<!-- probe -->

1. With the generator off, put channel 1's tip in **i6**, in the strip of
   the generator's OUT lead, and channel 2's tip in **i10**, at the
   **resistor top**. Clip both ground leads to the bottom − rail, by
   columns 6 and 9. Set both channels to **DC coupling** and trigger on
   channel 1.
2. Set the generator to a **0–4 V sine wave at 100 Hz** (2 V offset).
   Set the scope to show **10 ms** across the screen, then turn the
   generator on. Count complete cycles on channel 1. Briefly widen the
   view until two successive peaks are visible; measure the time between
   them. Record: cycles in 10 ms = \_\_\_; period = \_\_\_ ms.
3. Return the scope to a **10 ms** window. Change only the generator
   frequency to **1 kHz**. Count again on channel 1 and measure between
   successive peaks.
   Record: cycles in 10 ms = \_\_\_; period = \_\_\_ ms.

Your counts should be near **1 and 10**, and the periods near **10 ms
and 1 ms**. Use channel 1 for counting if channel 2 is hard to see.

## Why it happens

Frequency and period describe the same wave from two sides: the period is
one second divided by the frequency. The 1 kHz wave repeats ten times as
often as the 100 Hz one, so each cycle takes one tenth as long, and ten
of them fit where one did. The capacitor and resistor make a high-pass RC
path: at 100 Hz the resistor's signal is attenuated more than at 1 kHz,
so the two output heights need not match. The cycle timing should still
match the input. Changing a note's frequency in
[Lesson 5's melody maker](../005-melody-maker/index.md) changes its pitch
for the same reason.

## Check your result

Compare your two counts and periods with the sketches you made first.
In one sentence, explain why the higher frequency has the shorter period.

## If it doesn't work

- **Neither trace appears:** Check the 0–4 V sine setting, OUT at j6,
  generator GND at the bottom − rail, scope trigger and 10 ms window.
- **Only channel 2 is missing:** Check the capacitor at g6–e6, the
  b6-to-j10 jumper, the resistor at g10–e10 and a10-to-GND jumper.
  Probe the resistor's top strip.
- **The count differs:** Check the 100 Hz or 1 kHz setting and the
  10 ms window. Count from one rising crossing to the next.
- **Traces shift when a clip moves:** Keep both ground clips and generator
  GND on the common bottom − rail.

## About the sketch

This passive circuit needs no upload or Mega signal pin. The matching
sketch claims none:

<!-- sketch -->

These counts are calculated expectations. This lesson has not been
recorded as tried on hardware.
