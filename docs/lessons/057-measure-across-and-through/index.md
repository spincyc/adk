---
lesson: 57
promise: Measure voltage across an LED and resistor, then find the current through them.
time: 20 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Red LED
  - 220 Ω resistor (red, red, black, black, brown)
  - 3 jumper wires
  - Digital multimeter with DC volts
ideas:
  - Voltage is measured between two points
  - Current passes through a path
---

## What you'll find

<!-- closeup -->

The red LED from [Lesson 1](../001-blink/index.md) blinks slowly while you
measure it. A multimeter is **required** for this investigation. You will
find two voltage readings that add up to the voltage from pin 26 to GND,
then find the current through the path.

## Predict

When the LED is on, pin 26 is about 5 V above GND. Will the resistor and
the LED **each** have 5 V across them? Write down your guess before measuring.

Voltage is a difference between **two points**, so a voltage reading needs
one probe on each side of what you measure. Current goes **through** a path.
Here, pin 26, the resistor, the LED and GND make one path with no branch.

## Build it

Unplug the Mega's USB cable before changing wires. Keep the red LED and
220 Ω resistor in their [Lesson 1](../001-blink/index.md) home holes, with
the resistor always in series with the LED. If you are starting here, follow
the generated steps below. Check the whole path before plugging in.

<!-- bench -->

<!-- steps -->

<!-- connections -->

## Run the slow blink

Open **File → Examples → Adk → lessons → 057-measure-across-and-through**
in the Arduino IDE, choose **Tools → Board → ADK Boards → ADK Mega 2560**
and the board's port, then upload:

<!-- sketch -->

The LED stays on for three seconds, then off for three seconds. This gives
the meter time to settle while it is on.

## Measure across

Set the meter to DC volts (**V⎓**), on the 20 V range if it has ranges.
Put the black lead in **COM** and the red lead in **V**. Keep the metal
probe tips apart. For each drawing, touch the red and black probes at the
marked points during an **on** interval; wait for the number to settle and
write it down. Keep the meter in DC volts and the red lead in **V** for all
three readings.

<!-- measure -->

Record your readings: pin 26 to GND = ___ V; across the resistor = ___ V;
across the LED = ___ V. Add the last two: ___ V. How close is that sum to
the first reading? Then watch the pin-to-GND reading during an **off**
interval. Predict it first; it should fall near 0 V.

## Explain the current

The resistor may read about 3 V and the LED about 2 V while lit. Their
voltages **add** to about 5 V because together they span pin 26 to GND.
Your numbers can differ a little. Use your own resistor reading to find
the current: **current = resistor voltage ÷ 220 Ω**. For example,
3 V ÷ 220 Ω is about 0.014 A, or **14 mA**. That current passes through
both parts because this path has no branch.

In one sentence, explain why a voltage reading uses two points while the
current you calculated belongs to the whole path. Compare your answer with
your prediction.

## Optional: measure through

Predict whether a current meter would read differently before the resistor
and after the LED. Use this step only if your meter has a **fused DC mA**
input and a range above 20 mA (often 200 mA), and its probes seat securely
in the named holes. If a probe will not seat securely, skip this step and use
the current calculated above. The meter must **replace a wire in the path**.
Never put current-mode probes across a supply or a part: that can short the
Mega.

1. **Unplug the USB cable.** Remove the orange jumper between pin 26 and
   j6. Move the meter's red lead from **V** to its fused **mA** jack; keep
   black in **COM**. Set the dial to DC mA on a range above 20 mA.
2. Put the red probe into **pin 26** and the black probe into the free
   hole **i6**, beside the resistor's top leg in g6. Check that the resistor
   and LED still lead from i6 to GND. Plug in and read the current while the
   LED is on. Record it: before the resistor = ___ mA.
3. **Unplug again.** Lift both probes. Restore the orange jumper from pin
   26 to j6. Remove the black jumper between a7 and the bottom − rail by
   column 7. Put the red probe in **a7** and the black probe in the rail
   hole just vacated by that jumper. Plug in and read the current while
   the LED is on: after the LED = ___ mA.
4. **Unplug again.** Lift both probes and immediately move the red lead
   back to the **V** jack. Set the dial to DC volts, restore the black
   jumper from a7 to the bottom − rail by column 7, then plug in.

The two current readings should be close to each other and to your
calculation from the resistor voltage. The meter can change the current a
little while it is in the path. Both positions put the meter **through**
the same unbranched path; the earlier voltage readings put its probes
**across** two points.
