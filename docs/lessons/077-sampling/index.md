---
lesson: 77
promise: Watch a smooth knob voltage become separate numbered readings.
time: 25 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - White LED
  - 220 Ω resistor (red, red, black, black, brown)
  - 10 kΩ potentiometer (the knob)
  - 8 jumper wires
  - Digital multimeter with DC volts
ideas:
  - A sample is one voltage reading taken at one moment
  - The Mega's 10-bit ADC reports whole numbers from 0 to 1023
---

## What you'll build

<!-- closeup -->

Rebuild [Lesson 7's dimmer](../007-dimmer/index.md): a knob controls a white
LED. This time, compare the knob's voltage on a meter with the separate
numbers the Mega sends to the Serial Plotter. The LED still shows that the
knob controls the circuit, while the graph shows what the Mega reads.

## Predict

The knob's wiper moves along a resistive strip between GND and 5 V. A meter
can follow its changing voltage. The Mega measures A0 repeatedly, but each
measurement must be a whole number from **0 to 1023**. Before uploading,
write down your guesses:

1. What number should the Mega print when the meter shows about **2.5 V**?
2. When you turn the knob slowly, will the numbers change in whole steps or
   in fractions? What might the graph miss if you turn the knob faster?

## Build it

!!! warning "Unplug before wiring"
    Unplug the Mega's USB cable before changing the build. Start with an
    empty breadboard. Use only the Mega's USB power
    for this circuit. Check that the knob's lone middle leg reaches **A0
    only**: joining that strip to a supply rail could short 5 V to GND at
    an end of the knob's travel. Every LED needs its 220 Ω resistor.

The complete steps below place the knob at its Lesson 7 home: its outer
legs sit in **f39** and
**f41**, and the wiper in **d40**. Its outer legs reach the top − and +
rails, and A0 reaches the wiper's lower strip at **a40**. The white LED and
its resistor go in their home at columns 38–39. Follow the generated steps
for every wire and the two rail feeds.

<!-- bench -->

<!-- steps -->

Check the finished connections before plugging in USB:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 077-sampling** in the Arduino IDE.
The sketch uses Lesson 7's dimmer code: it reads A0 once, sets the LED's
brightness from that reading, and prints the reading. A single read makes
the LED and the printed number describe the same sample.

<!-- sketch -->

`adk::AnalogInput knob {A0};` claims A0 as an input. Each `knob.read ()`
takes one **sample**: a measurement of the voltage at that moment. The
`adk::wait (20)` pause leaves space between readings. At 9600 baud, sending
each line also takes time, so the sketch does not take exactly 50 samples
each second.

## Try it

### Watch the samples

1. Check the knob's three legs, the LED's resistor and the rail wires. Plug
   the Mega into USB. Select **Tools → Board → ADK Boards → ADK Mega 2560**
   and its **Tools → Port**, then upload the sketch.
2. Open **Tools → Serial Plotter** at **9600 baud**. Watch the `knob` trace
   while turning slowly from the GND end to the 5 V end. It should rise
   from near 0 to near 1023; the `brightness` trace follows lower down.
   The graph's scale has to fit both traces, so a step of one count is far
   too small to see there; step 4 shows those.
3. Return slowly toward the middle, then hold still. Compare that section
   of the graph with a faster turn over the same distance. Does the faster
   turn leave fewer plotted points along the way? The Plotter joins points
   with lines; the Mega measured only at those separate moments.
4. Close the Plotter and open **Tools → Serial Monitor** at 9600 baud to
   see the exact whole numbers. Turn the knob very slowly and watch the
   `knob` number change one whole step at a time, never by a fraction.
   Then hold the knob still and write down three nearby readings. Switch
   back to the Plotter when finished; only one window can use the port at
   a time.

### Measure the voltage

Set a digital meter to **DC volts** (V⎓), with its black lead in **COM**
and red lead in **V**. Leave it on volts: a current setting across the
supply could short it. Put the black probe in a free hole of the − rail
and the red probe in a free hole of the wiper's lower strip in column 40.
Keep the metal probe tips apart. Turn the knob until the meter shows about
**2.5 V**; hold it there and read the Serial Monitor. Then compare near
**1 V** and **4 V**. You are measuring the same wiper voltage at each
setting; unplug USB before moving any wires.

<!-- measure -->

| Knob setting | Your meter voltage | Your A0 reading |
|---|---:|---:|
| Near 1 V | ____ V | ____ |
| Near 2.5 V | ____ V | ____ |
| Near 4 V | ____ V | ____ |

## Why it happens

The Mega's **analog-to-digital converter** (ADC) puts each voltage into
one of **1024** numbered levels, 0 through 1023. With a nominal 5 V
reference, one level spans about **5 V ÷ 1024 ≈ 0.0049 V**, or **4.9 mV**.
That makes 2.5 V read near 512, 1 V near 205 and 4 V near 819. The
meter voltage can move smoothly while the printed number stays the same,
then changes by one. That is why, in the Serial Monitor, a slow turn
shows whole steps.

When you turn faster, the knob can pass several levels between two reads.
The graph connects the readings, but it has no measurement of the voltage
between them. At a boundary between levels, nearby numbers may alternate
even with the knob still: electrical noise and the contact settling can
move the voltage a little. The meter often averages or updates more
slowly, and may not resolve one 4.9 mV step. Compare the general trend
rather than expecting its display to match every changing count. The
Mega's supply may differ from exactly 5 V, so the expected counts are
approximate too.

## Check your result

Were your middle reading and the slow and fast turns close to what you
guessed? Explain why a line on the Plotter does not mean the Mega measured
every point on that line.

Then test the explanation: hold the knob until the Serial Monitor shows a
reading near **600**. Predict the meter voltage using **600 × 4.9 mV**
before looking at the meter. How close is it? Try a slower and a faster
turn across that setting and count how many different numbers you catch
in each pass.

## If it doesn't work

| What you see | Check |
|---|---|
| No trace or unreadable text | Select the Mega's port and 9600 baud. Close the Serial Monitor before opening the Plotter. |
| A0 stays near 0 or 1023 as you turn | Unplug USB. Check the knob's outer wires from j39 to the top − rail and j41 to the top + rail, the rail feeds, the − rail link, and A0 at a40. |
| The meter reads 0 V while A0 changes | Check DC volts, black probe on the − rail, and red probe on the wiper's lower strip in column 40. |
| The knob or a wire gets warm | Unplug at once. Check that the wiper's strip joins only A0 and that its outer legs reach opposite rails. |

These are expected results from the circuit and the Mega's ADC; no physical
hardware trial has been recorded for this lesson.
