---
lesson: 74
promise: Press two buttons to fill in the four rows of a NAND truth table.
time: 25 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - SN74HC00N 14-pin DIP NAND chip
  - 2 push buttons
  - Red LED
  - 2 10 kΩ resistors (brown, black, black, red, brown)
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 100 nF ceramic capacitor
  - 17 jumper wires
ideas:
  - A NAND gate gives a low output only when both inputs are high
laws:
  - {law: logic-levels, section: why-it-happens, for: "Treats 5 V as high and GND as low"}
  - {law: pull, section: why-it-happens, for: "Holds released button inputs low"}
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

Start with an empty breadboard and USB unplugged. If you just finished
another investigation, remove its parts and wires first. The complete
steps below put the Mega's GND and 5 V wires in their usual rail holes,
then the red LED in its familiar holes with a **1 kΩ** resistor. The
buttons go in their familiar positions.
Place the **14-pin DIP** chip across the center gap with its
notch facing left. Seen from above, pin 1 is at the lower-left corner;
numbers run along the lower edge to pin 7, then back along the upper
edge to pin 14. The [SN74HC00N datasheet](https://www.ti.com/lit/ds/symlink/sn54hc00.pdf)
shows this pinout.

<!-- bench -->

<!-- steps -->

Check the power path: chip **pin 14 to 5 V** and **pin 7 to GND**. The
100 nF capacitor stands across the top + and − rails by column 15, beside
pin 14's supply wire; a black wire at column 23 makes the top − rail GND.
Button A goes to **pin 1**, button B to **pin 2**, and each has its own
10 kΩ path to GND so it reads 0 when released. **Pin 3 → 1 kΩ → red LED → GND**. The
six unused inputs (pins 4, 5, 9, 10, 12, and 13) are tied to GND; leave
their three output pins (6, 8, and 11) unconnected. A loose input can
give an unpredictable result, so check these wires before powering. The
schematic shows gate 1 with its pin numbers (the
[schematic key](../../electricity/schematics.md) names each symbol):

<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 620 440" width="620"
     role="img" aria-labelledby="nand-gate-title nand-gate-desc">
  <title id="nand-gate-title">NAND truth-table schematic</title>
  <desc id="nand-gate-desc">Button A joins 5 volts to gate 1's input A, pin 1, and button B joins 5 volts to its input B, pin 2. Each input has a 10 kilohm resistor to ground. The output, pin 3, drives a 1 kilohm resistor and red LED to ground.</desc>
  <g fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round">
    <path d="M60 40 H200"/>
    <path d="M200 40 L200 54"/>
    <path d="M200 96 L200 110"/>
    <circle cx="200" cy="58" r="4"/>
    <circle cx="200" cy="92" r="4"/>
    <path d="M186 50 V100 M186 75.0 H172 M172 67.0 V83.0"/>
    <path d="M200 110 L200 130"/>
    <path d="M200 130 L200 140.0"/>
    <path d="M200 190.0 L200 200"/>
    <rect x="187" y="140.0" width="26" height="50"/>
    <path d="M200 200 V208 M180 208 H220 M187 216 H213 M194 224 H206"/>
    <path d="M200 130 L380 130 L380 172 L400 172"/>
    <path d="M80 40 L80 50"/>
    <path d="M80 50 L80 64"/>
    <path d="M80 106 L80 120"/>
    <circle cx="80" cy="68" r="4"/>
    <circle cx="80" cy="102" r="4"/>
    <path d="M66 60 V110 M66 85.0 H52 M52 77.0 V93.0"/>
    <path d="M80 120 L80 260"/>
    <path d="M80 260 L80 275.0"/>
    <path d="M80 325.0 L80 340"/>
    <rect x="67" y="275.0" width="26" height="50"/>
    <path d="M80 340 V348 M60 348 H100 M67 356 H93 M74 364 H86"/>
    <path d="M80 260 L380 260 L380 208 L400 208"/>
    <path d="M400 155.0 H435 A35.0 35.0 0 0 1 435 225.0 H400 Z"/>
    <circle cx="476.0" cy="190" r="6"/>
    <path d="M482.0 190 L540 190"/>
    <path d="M540 190 L540 205.0"/>
    <path d="M540 255.0 L540 270"/>
    <rect x="527" y="205.0" width="26" height="50"/>
    <path d="M540 280 L540.0 297.0"/>
    <path d="M540.0 323.0 L540 340"/>
    <path d="M525.0 297.0 L555.0 297.0 L540.0 323.0 Z"/>
    <path d="M525.0 323.0 L555.0 323.0"/>
    <path d="M520.0 304.0 L504.0 312.0"/>
    <path d="M504.0 312.0 L507.8 306.1"/>
    <path d="M504.0 312.0 L511.0 312.5"/>
    <path d="M520.0 316.0 L504.0 324.0"/>
    <path d="M504.0 324.0 L507.8 318.1"/>
    <path d="M504.0 324.0 L511.0 324.5"/>
    <path d="M540 270 L540 280"/>
    <path d="M540 340 V348 M520 348 H560 M527 356 H553 M534 364 H546"/>
  </g>
  <g fill="currentColor">
    <circle cx="200" cy="130" r="5"/>
    <circle cx="80" cy="260" r="5"/>
  </g>
  <g fill="currentColor" font-size="17" font-family="system-ui, sans-serif">
    <text x="20" y="46">5 V</text>
    <text x="216" y="80">A</text>
    <text x="182" y="172" text-anchor="end" font-size="15">10 kΩ</text>
    <text x="372" y="164" text-anchor="end" font-size="15">pin 1</text>
    <text x="96" y="90">B</text>
    <text x="62" y="306" text-anchor="end" font-size="15">10 kΩ</text>
    <text x="372" y="230" text-anchor="end" font-size="15">pin 2</text>
    <text x="400" y="140">SN74HC00N gate 1</text>
    <text x="500" y="180" text-anchor="middle" font-size="15">pin 3</text>
    <text x="558" y="236">1 kΩ</text>
    <text x="470" y="330" text-anchor="end" font-size="15">red LED</text>
    <text x="230" y="400" font-size="15">Pin 14 to 5 V and pin 7 to GND, with 100 nF</text>
    <text x="230" y="420" font-size="15">across them. The six unused inputs go to GND.</text>
  </g>
</svg>

<!-- connections -->

## Try it

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
small current through the 1 kΩ resistor and LED. This is the button
decision from [Lesson 2](../002-buttons/index.md) made in hardware.

## Check your result

Compare the four rows you saw with your prediction. Which row is the only
one with the LED off, and why does the name “not AND” fit it?

## If it doesn't work

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
