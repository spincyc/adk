---
lesson: 58
promise: Swap one resistor and compare LED brightness with calculated current.
time: 25 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Red LED
  - 220 Ω resistor (red, red, black, black, brown)
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 2 kΩ resistor (red, black, black, brown, brown)
  - 4 jumper wires
  - Digital multimeter with DC volts
ideas:
  - More resistance reduces current in the same LED path
laws:
  - {law: prefixes, section: try-it, for: "Converts kΩ to Ω before finding each current in mA"}
  - {law: ohms-law, section: why-it-happens, for: "Shows more resistance gives less current for 3 V"}
  - {law: voltage-law, section: why-it-happens, for: "Gives each resistor the 3 V the LED leaves"}
  - {law: forward-voltage, section: why-it-happens, for: "Uses the LED's steady 2 V to find resistor voltage"}
---

## What you'll build

<!-- closeup -->

Keep [E02's steady LED circuit](../057-measure-across-and-through/index.md).
Swap only the resistor, then compare how the LED looks and calculate its
current. The Mega supplies 5 V from USB; no upload or signal pin is needed.

## Predict

A resistor's **resistance** says how strongly it holds back current. It
is measured in **ohms**, written Ω: 1 kΩ, one kilohm, is 1000 ohms.

Which resistor will give the brightest LED and largest current: **220 Ω**,
**1 kΩ**, or **2 kΩ**? Which will give the smallest? Put them in order in
your notes before changing anything. Predict whether the LED will still
light with 2 kΩ.

## Build it

Unplug USB before building if you are starting here. Keep the 5 V and GND
rail wires, red LED, and its two short jumpers in the same holes as E02.
Begin with the **220 Ω** resistor across the middle gap in **g6 and e6**.
Each replacement goes in those same two holes. The 5 V feed and LED return
stay in place.

!!! warning "Unplug before every swap"
    Never change a resistor or wire while USB is plugged in. Never leave
    the LED powered with no resistor. Check that each resistor crosses the
    middle gap before reconnecting USB. If a part gets hot or smells,
    unplug at once.

<!-- bench -->

<!-- steps -->

The drawing and connections show the starting circuit with 220 Ω:

<!-- connections -->

## Try it

Keep the same USB source, LED, room light, and viewing angle for all three
trials. Make each brightness note before the next swap.

1. With **220 Ω** in place, predict whether its current will be above or
   below 10 mA. Plug USB in. Note the LED's brightness and measure the
   voltage across the resistor as shown below. Unplug USB.
2. Predict how the LED and current will change with **1 kΩ**. Lift only
   the 220 Ω resistor and put the 1 kΩ resistor in **g6 and e6**. Plug USB
   in, note brightness, and measure the resistor voltage. Unplug USB.
3. Predict how the LED and current will change with **2 kΩ**. Put it in
   the same two holes, plug USB in, note brightness, and measure again.
   Unplug USB when done.

For each voltage reading, set the meter to **DC volts (V⎓)**, black lead in
**COM**, red lead in **V**, and 20 V range if needed. Touch red to **h6**
and black to **d6**. Those free holes are on opposite sides of whichever
resistor is fitted. Keep the meter leads in the voltage jacks. Never put
a meter set to current across the supply.

<!-- measure -->

Calculate each current from **its own** resistor voltage:

<p class="formula">current in mA = 1000 × resistor voltage in V ÷ resistance in Ω</p>

| Resistor | LED brightness | Voltage across resistor | Calculated current |
|---|---|---|---|
| 220 Ω | | ____ V | ____ mA |
| 1 kΩ = 1000 Ω | | ____ V | ____ mA |
| 2 kΩ = 2000 Ω | | ____ V | ____ mA |

**Leave USB unplugged.** Put the 220 Ω resistor back in g6 and e6, so the
build matches the drawing again.

## Why it happens

The LED keeps roughly 2 V across it, so each resistor has roughly the
other 3 V. A larger resistor lets less current through for the same
voltage: about 14 mA with 220 Ω, 3 mA with 1 kΩ and 1.5 mA with 2 kΩ.
That current passes through the same unbranched path, LED included, so
the LED usually looks dimmer. The LED's own voltage can shift a little as
current changes, so the three currents need not be in exact resistance
ratios.

## Check your result

Compare your observations with the order you predicted. Which change did
the meter show more clearly than your eyes?

## If it doesn't work

| What you see | Check after unplugging USB |
|---|---|
| The LED stays dark with every resistor | Check the 5 V feed, LED direction, and GND return. |
| It goes dark after a swap | Check the resistor value and make sure its legs cross the middle gap in g6 and e6. |
| The meter stays near zero | Check DC volts, the V jack, and the h6 and d6 probe holes. |
| Brightness is hard to judge | Use the calculated currents; small visual changes can be difficult to see. |

## About the sketch

The example has empty Arduino functions because the USB-powered circuit
works without code. It claims no signal pins.

<!-- sketch -->

These are expected results; the circuit has not been recorded as tested on
hardware.
