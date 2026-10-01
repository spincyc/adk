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
  - 10 kΩ resistor (brown, black, black, red, brown)
  - 7 jumper wires
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
   LED's state again. Compare all three observations with your prediction.

The expected result is **off, on, off**. When you press, a small base
current flows through the button and 1 kΩ resistor. That lets current
flow through the separate LED and collector path. Releasing the button
removes the base current, and the 10 kΩ resistor keeps the switch off.
The LED current never needs to pass through the button.

## If it doesn't work

Unplug USB before each check:

- **LED never lights:** Check that its long leg faces the 220 Ω resistor and
  its short leg reaches the collector. Check the S8050 marking, E–B–C
  order, and the emitter's − rail wire.
- **LED stays on without a press:** Check that the button straddles the
  middle gap and the 10 kΩ resistor reaches from base to −. Look for a
  wire bridging the button's two sides.
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
