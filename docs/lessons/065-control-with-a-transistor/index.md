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
laws:
  - {law: ohms-law, section: try-it, for: "Finds LED and base currents from resistor voltages"}
  - {law: current-law, section: try-it, for: "Subtracts the pull-down's 0.08 mA to give the base 4.1 mA"}
  - {law: current-law, section: change-one-thing, for: "Splits base-resistor current between base and pull-down"}
  - {law: forward-voltage, section: try-it, for: "Expects 4.2 V, not 5 V, across the base resistor"}
  - {law: transistor-switch, section: why-it-happens, for: "Explains a small base current switching the LED path"}
  - {law: pull, section: why-it-happens, for: "Keeps the base at GND so the switch stays off"}
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

The same two paths in schematic form (the
[schematic key](../../electricity/schematics.md) names each symbol):

<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 560 460" width="560"
     role="img" aria-labelledby="transistor-switch-title transistor-switch-desc">
  <title id="transistor-switch-title">Transistor switching an LED schematic</title>
  <desc id="transistor-switch-desc">Five volts feeds two paths. A 220 ohm resistor and the red LED lead to the collector of the S8050 transistor, whose emitter goes to ground. The push button and a 1 kilohm resistor feed the base, and a 10 kilohm resistor joins the base to ground.</desc>
  <g fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round">
    <path d="M70 50 H370"/>
    <path d="M70 50 L70 330"/>
    <path d="M70 330 L84 330"/>
    <path d="M136 330 L150 330"/>
    <circle cx="88" cy="330" r="4"/>
    <circle cx="132" cy="330" r="4"/>
    <path d="M80 316 H140 M110.0 316 V302 M102.0 302 H118.0"/>
    <path d="M150 330 L175.0 330"/>
    <path d="M245.0 330 L270 330"/>
    <rect x="175.0" y="317" width="70" height="26"/>
    <path d="M270 330 L270 340.0"/>
    <path d="M270 410.0 L270 420"/>
    <rect x="257" y="340.0" width="26" height="70"/>
    <path d="M270 420 V428 M250 428 H290 M257 436 H283 M264 444 H276"/>
    <path d="M270 330 L288 330"/>
    <path d="M288 310 V350"/>
    <path d="M288 321 L370 300 L370 280"/>
    <path d="M288 339 L370 360 L370 420"/>
    <path d="M333.1 350.6 L323.5 353.4"/>
    <path d="M333.1 350.6 L326.1 343.4"/>
    <circle cx="331.0" cy="330" r="30"/>
    <path d="M370 420 V428 M350 428 H390 M357 436 H383 M364 444 H376"/>
    <path d="M370 50 L370 65.0"/>
    <path d="M370 135.0 L370 177.0"/>
    <rect x="357" y="65.0" width="26" height="70"/>
    <path d="M355.0 177.0 L385.0 177.0 L370.0 203.0 Z"/>
    <path d="M355.0 203.0 L385.0 203.0"/>
    <path d="M350.0 184.0 L334.0 192.0"/>
    <path d="M334.0 192.0 L337.8 186.1"/>
    <path d="M334.0 192.0 L341.0 192.5"/>
    <path d="M350.0 196.0 L334.0 204.0"/>
    <path d="M334.0 204.0 L337.8 198.1"/>
    <path d="M334.0 204.0 L341.0 204.5"/>
    <path d="M370 203.0 L370 280"/>
  </g>
  <g fill="currentColor">
    <circle cx="270" cy="330" r="5"/>
  </g>
  <g fill="currentColor" font-size="17" font-family="system-ui, sans-serif">
    <text x="20" y="56">5 V</text>
    <text x="82" y="285">button</text>
    <text x="210" y="310" text-anchor="middle">1 kΩ</text>
    <text x="215" y="385" text-anchor="end">10 kΩ</text>
    <text x="410" y="345">S8050</text>
    <text x="388" y="106">220 Ω</text>
    <text x="394" y="196">red LED</text>
    <text x="384" y="274" font-size="14">collector, column 31</text>
    <text x="300" y="300" text-anchor="end" font-size="14">base</text>
  </g>
</svg>

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

The base resistor gets about 4.2 V, not the whole 5 V, because the base
and emitter behave like a diode: while current flows into the base, it
sits about 0.7–0.8 V above the emitter, its
[forward voltage](../../laws/diodes-transistors-op-amps.md#forward-voltage).

Each resistor's voltage divided by its resistance gives its current:
about 3.0 V ÷ 220 Ω ≈ **14 mA** through the LED and collector, and about
4.2 V ÷ 1 kΩ ≈ **4.2 mA** through the base resistor. The 10 kΩ pull-down,
the resistor from base to − that holds the switch off, has the base's
0.8 V across it and takes about 0.08 mA of the 4.2 mA, so the base gets
about **4.1 mA**.

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
resistor now has about 4.3 V across it, a little more than before, since
with less current the base sits only about 0.70 V above the emitter. Ten
times the resistance passes about a tenth of the current, **0.43 mA**. A
little of that, about 0.07 mA, goes down the 10 kΩ pull-down rather than
into the base, so the base gets about **0.36 mA**. Now the collector
current is about **40 times** the base current: a small base current
controls a much larger collector current.

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
