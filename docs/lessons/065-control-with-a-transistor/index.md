---
lesson: 65
promise: Press a button to let a small base current switch a separate LED path.
time: 25 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Push button
  - Red LED
  - S8050 transistor (check its E–B–C pin order)
  - 220 Ω resistor (red, red, black, black, brown)
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 2 × 10 kΩ resistors (brown, black, black, red, brown), one for the comparison
  - 7 jumper wires
  - Digital multimeter with DC volts, for the measurements
ideas:
  - A small current into a transistor's base switches a separate path through its collector
---

## What you'll build

<!-- closeup -->

A button turns a red LED on while you hold it. The button does not sit in
the LED's path. Instead, it lets a small current into a transistor, which
acts as the LED's switch. The Mega supplies 5 V from USB; no signal pin or
upload is needed.

## Predict

The LED's path has a gap at the transistor. A 10 kΩ resistor holds the
transistor's base near GND until you press the button. Before you plug in
USB, predict whether the LED will be on or off with the button released.
Then predict what will happen while you press it and when you let go.
Write down all three guesses.

## Build it

!!! warning "Unplug first"
    Unplug USB before changing wires. Keep the **220 Ω resistor in series
    with the LED**. Never connect an LED straight across the rails. If a
    part gets hot or smells, unplug at once and check the wiring.

Keep the Mega's GND wire in the bottom − rail hole nearest it and its 5 V
wire in the top + rail hole nearest it. Keep E09's red LED in its home
holes and take out E09's diode, resistor and black jumper. The button goes
in its familiar position; the transistor goes in the same holes as in
Lesson 3.

Before inserting the transistor, read its marking. This drawing is for an
**S8050 whose pins are E–B–C**, left to right with its marked flat face
toward you and its legs pointing down. Check the pin diagram for your
marked device; [onsemi's SS8050 datasheet](https://www.onsemi.com/pdf/datasheet/ss8050-d.pdf)
shows that order for its S8050-marked TO-92 parts. Kit versions can differ.
Do not substitute a similarly shaped transistor by appearance. If its
order differs, follow its own datasheet before building.

<!-- bench -->

<!-- steps -->

The button's left legs and right legs are each joined inside. Pressing it
joins those two sides. The 1 kΩ resistor limits current into the base; the
10 kΩ resistor holds the base at GND after you release it. The 220 Ω
resistor stays in the other path, with the LED.

<!-- connections -->

## Try it

1. With USB unplugged, trace both paths in the drawing: **+ → 220 Ω → LED
   → collector → emitter → −**, and **+ → button → 1 kΩ → base → emitter
   → −**. Check that the 10 kΩ resistor goes from base to −.
2. Leave the button alone and plug the Mega into USB. Record whether the
   LED is on or off.
3. Press and hold the button. Record what changes. Let go and record the
   LED's state again.

The expected result is **off, on, off**. Now measure the two currents
while the LED is lit. Set the meter to **DC volts (V⎓)**, black lead in
**COM**, red lead in **V**. Each reading needs the button held: ask a
helper to press it, or press it with one hand and hold both probes in the
other. Keep the metal tips apart.

4. Hold the button and measure across the LED's **220 Ω** resistor, red
   on **h6** and black on **d6**. Expect about **3.0 V**.
5. Hold the button and measure across the **1 kΩ** base resistor. Its
   button end in c32 shares a strip, through the wire from a4 to a32,
   with column 4's lower holes, so put red on **c4** and black on the
   10 kΩ resistor's leg in **b30**, on the base's strip. Expect about
   **4.2 V**.

<!-- measure -->

Each resistor's voltage divided by its resistance gives its current:
about 3.0 V ÷ 220 Ω ≈ **14 mA** through the LED and collector, and about
4.2 V ÷ 1 kΩ ≈ **4.2 mA** into the base.

## Why it happens

When you press, current flows through the button and 1 kΩ resistor into
the transistor's base and out of its emitter. That base current lets a
current flow through the separate LED and collector path. Releasing the
button removes the base current, and the 10 kΩ resistor keeps the switch
off. The LED current never needs to pass through the button.

With the 1 kΩ resistor, though, the base current is not very small: the
collector current is only about three times as large. The transistor is
switched fully on, and once it is, the LED's own 220 Ω resistor sets its
current. Does it need that much base current to stay fully on?

## Change one thing

Predict first: if the 1 kΩ base resistor becomes **10 kΩ**, ten times as
large, will the LED dim, go out, or stay as it was? What will happen to
the base current?

1. Unplug USB. Lift out only the 1 kΩ resistor from **c32 and c30**, and
   put the spare 10 kΩ resistor in those same two holes.
2. Plug USB in, hold the button and look at the LED. Measure across the
   220 Ω resistor and across the new 10 kΩ base resistor, in the same
   holes as before.
3. Unplug USB and put the 1 kΩ resistor back in c32 and c30.

| Base resistor | Across 220 Ω | LED current | Across base resistor | Base resistor current |
|---|---:|---:|---:|---:|
| 1 kΩ | ____ V | ____ mA | ____ V | ____ mA |
| 10 kΩ | ____ V | ____ mA | ____ V | ____ mA |

The LED should look just as bright, and the 220 Ω reading should stay
near 3.0 V: about 14 mA still flows through the collector. The base
resistor still has about 4.2 V across it, but ten times the resistance
passes a tenth of the current, about **0.43 mA**. A little of that, about
0.07 mA, goes down the 10 kΩ pull-down rather than into the base, so the
base gets about **0.36 mA**. Now the collector current is about **40
times** the base current: a small base current controls a much larger
collector current.

## Check your result

Compare the LED with your prediction for each base resistor. Which
current changed tenfold, and which stayed the same? In one sentence,
explain what the button controls and what the LED's 220 Ω resistor sets.

## If it doesn't work

Unplug USB before each check:

- **LED never lights:** Check that its long leg faces the 220 Ω resistor and
  its short leg reaches the collector. Check the S8050 marking, E–B–C
  order, and the emitter's − rail wire.
- **LED stays on without a press:** Check that the button straddles the
  middle gap and the 10 kΩ resistor reaches from base to −. Look for a
  wire bridging the button's two sides.
- **LED dims or goes out with the 10 kΩ base resistor:** Check its bands
  are brown, black, black, red, brown: a 100 kΩ resistor (orange fourth
  band) passes far too little base current.
- **LED changes only when a wire moves:** Press the parts firmly into their
  holes. Check that the button's and transistor's legs do not touch above
  the board.
- **A part gets hot or smells:** Unplug immediately. Check both current
  limiting resistors and that no wire joins + directly to −.

## About the sketch

The circuit takes power from the Mega's **5 V supply pin**. No upload is
needed: USB powers it even if the Mega has an older sketch. The matching
ADK sketch claims no I/O pins:

<!-- sketch -->

When you finish, release the button and unplug USB. Compare this physical
switch with [Lesson 3's active buzzer](../003-reaction-duel/index.md): there
a Mega pin supplies the base current, while the buzzer has its own path
through the transistor. These are expected results; the circuit has not been
recorded as tested on hardware.
