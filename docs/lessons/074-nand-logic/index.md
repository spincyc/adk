---
lesson: 74
promise: Press two buttons to fill in the four rows of a NAND truth table.
time: 25 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - SN74HC00N 14-pin DIP NAND chip
  - 2 push buttons
  - Red LED
  - 2 10 kΩ resistors (brown, black, black, red, brown)
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 100 nF ceramic capacitor
  - 19 jumper wires
ideas:
  - A NAND gate gives a low output only when both inputs are high
---

## What you'll build

<!-- closeup -->

Two buttons set the inputs of one gate inside an SN74HC00N chip. Its
output lights a red LED. The buttons and chip do the thinking; the Mega
supplies **USB 5 V and GND**, with no signal pin or upload needed. The
four button combinations make a **truth table**.

## Predict

A released button means **0** (low); a pressed button means **1** (high).
Before powering the build, predict whether the LED will be on or off
with neither button pressed, A alone, B alone, and both pressed. Write
your guesses in the table below.

## Build it

!!! warning "Unplug before wiring"
    Take out the Mega's USB cable before moving parts or wires. Use only
    its USB 5 V supply. Keep the **1 kΩ resistor in series with the LED**.
    Check the chip's notch and all supply wires before plugging in USB.

Take out everything from Lesson 73 except the Mega's GND wire in the
bottom − rail hole nearest it, its 5 V wire in the top + rail hole
nearest it, and the red LED in its familiar holes. Replace the LED's
220 Ω resistor with **1 kΩ**. The buttons go in their familiar positions.
Place the **14-pin DIP** chip across the center gap with its
notch facing left. Seen from above, pin 1 is at the lower-left corner;
numbers run along the lower edge to pin 7, then back along the upper
edge to pin 14. The [SN74HC00N datasheet](https://www.ti.com/lit/ds/symlink/sn54hc00.pdf)
shows this pinout.

<!-- bench -->

<!-- steps -->

Check the power path: chip **pin 14 to 5 V** and **pin 7 to GND**. The
100 nF capacitor sits beside the chip across its supply. Button A goes
to **pin 1**, button B to **pin 2**, and each has its own 10 kΩ path to
GND so it reads 0 when released. **Pin 3 → 1 kΩ → red LED → GND**. The
six unused inputs (pins 4, 5, 9, 10, 12, and 13) are tied to GND; leave
their three output pins (6, 8, and 11) unconnected. A loose input can
give an unpredictable result, so check these wires before powering.

<!-- connections -->

## Try all four inputs

Plug in USB after checking the wiring. For each row, hold the named
button or buttons, record what the LED actually does, then release them.
Try the rows in this order:

| A | B | Your prediction | LED you see | Expected LED |
|---|---|---|---|---|
| 0: released | 0: released | ____ | ____ | On (1) |
| 0: released | 1: pressed | ____ | ____ | On (1) |
| 1: pressed | 0: released | ____ | ____ | On (1) |
| 1: pressed | 1: pressed | ____ | ____ | Off (0) |

## Why it happens

The chip's gate is called **NAND**, short for “not AND.” Its output is
low only when **both** inputs are high. The 10 kΩ resistors hold released
inputs low; a press connects that input to 5 V. A high output sends a
small current through the 1 kΩ resistor and LED. Compare the four rows
you saw with your prediction. This is the button decision from
[Lesson 2](../002-buttons/index.md) made in hardware.

## If the LED surprises you

Unplug USB before checking a connection:

| What you see | Check |
|---|---|
| The LED never lights | Pin 14 reaches 5 V, pin 7 reaches GND, and the LED's long leg faces the 1 kΩ resistor. |
| The LED never goes out | Each button straddles the gap and its pressed side reaches pin 1 or 2; try holding both firmly. |
| A released button changes the result | Its own 10 kΩ resistor must connect that input to GND; check that no chip input is loose. |

## About the sketch

The matching ADK sketch claims no I/O pins. This hardware gate works
from the Mega's USB power even if another sketch is already loaded:

<!-- sketch -->

Unplug USB when finished. The expected LED states follow the datasheet's
NAND truth table; this lesson has not been recorded as tried on physical
hardware.
