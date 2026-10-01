---
lesson: 57
promise: Measure the supply, resistor, and LED voltages, then calculate their shared current.
time: 20 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Red LED
  - 220 Ω resistor (red, red, black, black, brown)
  - 4 jumper wires
  - Digital multimeter with DC volts
ideas:
  - Voltage is measured between two points
  - Current passes through a complete path
---

## What you'll build

<!-- closeup -->

Keep [E01's steady LED circuit](../056-close-the-loop/index.md). A
multimeter is required. You will measure the voltage between two points at
a time, then use the resistor reading to calculate the current through the
one path. The Mega supplies power from USB; no upload is needed.

## Predict

The supply is near 5 V. Will the resistor and LED **each** have 5 V across
them, or will their readings add up to the supply reading? Write down a
guess before using the meter.

## Build it

Unplug USB before building if you are starting here. Keep the red LED,
220 Ω resistor, and all four wires in exactly the same holes as E01.
The resistor must remain in series with the LED; neither meter lead becomes
part of the circuit. Check the whole path before plugging USB in.

<!-- bench -->

<!-- steps -->

<!-- connections -->

## Try it

Set the meter to **DC volts (V⎓)** and the 20 V range if it has ranges.
Put the black lead in **COM** and the red lead in **V**. Keep the lead in the
V jack throughout this lesson. Never put voltage probes in a current jack
or across the supply while the meter is set to current: that can short the
Mega. Keep the metal probe tips from touching each other.

1. Predict whether the top + rail will read close to 0 V or 5 V above the
   bottom − rail. Plug USB in. Touch the probes to the two rail holes in
   the first drawing and record the steady reading.
2. Predict how much of that voltage lies across the resistor. Touch red
   to **h6** and black to **d6**, free holes in its two strips. Record
   the reading without moving the resistor.
3. Predict how much lies across the LED. Touch red to **c6** by its long
   leg and black to **c7** by its short leg. Record the reading.

<!-- measure -->

| Supply | Resistor | LED | Resistor + LED |
|---:|---:|---:|---:|
| ____ V | ____ V | ____ V | ____ V |

Add the resistor and LED readings. How close is that sum to your supply
reading? The LED may have about 2 V across it and the resistor the rest,
but use your own readings.

The resistor also gives a safe way to find the current without opening
the circuit or moving a meter lead. Divide **your** voltage across it by
220 Ω:

<p class="formula">current in mA = 1000 × resistor voltage in V ÷ 220 Ω</p>

For example, 3.0 V gives `1000 × 3.0 ÷ 220 ≈ 13.6 mA`. Your value may
differ.

## Why it happens

A voltage is always a difference **between two points**, so each reading
needs two probes, one on each side of a part. The resistor and LED share
the supply's voltage between them, which is why their two readings add up
to the supply's. Current is different: it passes **through** the parts.
The resistor and LED are in one path with no branch, so the current you
calculated from the resistor passes through the LED too, and around the
whole loop.

## Check your result

Compare your readings with your prediction. In one sentence, explain why
the voltage readings used two probes across points while the calculated
current belongs to the whole path.

## If it doesn't work

| What you see | Check |
|---|---|
| The LED is dark | Unplug USB. Check the four jumpers, resistor, LED direction, and the return to the Mega's GND. |
| A reading has a minus sign | Swap the red and black probes; their order sets the sign. |
| A reading is near zero when you expected a voltage | Check the meter is on DC volts, the red lead is in V, and the probes touch opposite sides of the part. |
| The readings do not add closely | Check that each probe touches the intended strip and the LED remains steadily lit. |

## About the sketch

The example has empty Arduino functions and claims no signal pins. It does
not control the LED; the USB supply and wired path keep it lit.

<!-- sketch -->

Leave the wiring in place for the next investigation. These are expected
readings; no hardware test has been recorded.
