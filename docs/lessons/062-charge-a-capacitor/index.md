---
lesson: 62
promise: Watch a capacitor store charge and give it back through a resistor.
time: 25 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 1000 µF polarized capacitor rated at least 10 V
  - 10 kΩ resistor (brown, black, black, red, brown)
  - 4 jumper wires
  - Digital multimeter with DC volts
ideas:
  - A capacitor stores separated charge
---

## What you'll build

<!-- closeup -->

The Mega's USB 5 V charges a **1000 µF capacitor** through a 10 kΩ
resistor. A meter across the capacitor shows its voltage rising. Then you
disconnect USB and let the capacitor send charge back through the same
resistor. The meter shows the voltage falling.

## Predict

The capacitor has two legs. Charge gathers on one side while charge leaves
the other. Its voltage changes as that separation grows or shrinks. Before
you power the build, predict the meter reading just after plugging in USB
and about 10 seconds later. Will it jump straight to 5 V, or rise toward it?
What will the meter show after you unplug USB and give the stored charge a
path through the resistor?

## Build it

!!! warning "Unplug first"
    Unplug the Mega's USB cable before placing parts or moving a wire. Use
    only its USB 5 V supply. The **striped − leg** of the capacitor must go
    in the bottom − rail; check the stripe before powering. Use a capacitor
    rated **at least 10 V**. Never join its legs directly with a wire or
    probe tips.

Take out E06's knob and its wires. Keep the Mega's GND wire in the
bottom − rail hole nearest it and its 5 V wire in the top + rail hole
nearest it. The red wire from the top + rail by column 6 to j6 feeds the
10 kΩ resistor; the resistor crosses the middle gap in g6–e6. The jumper
from b6 to b8 leads to the capacitor's **+ leg in a8**. Its striped − leg
goes into the **bottom − rail by column 9**. The resistor
stays in the path for both charging and discharging.

<!-- bench -->

<!-- steps -->

The breadboard joins a–e in one column and f–j in that column. The gap
separates those strips, so current reaches the capacitor only by crossing
the resistor. These are the finished connections:

<!-- connections -->

## Measure the charge

Set the meter to **DC volts (V⎓)**, with black lead in **COM** and red lead
in **V**; choose a 20 V range if the meter needs one. Keep it in voltage
mode throughout this lesson. Put the black probe in a free hole of the
bottom − rail and the red probe in a free hole in column 8's lower strip,
beside the capacitor's + leg. Keep the metal probe tips apart.

<!-- measure -->

Check the capacitor's stripe and the resistor path once more, then plug in
USB. Watch the meter for about 30 seconds. If the capacitor began near
0 V, expect about **3.2 V after 10 seconds**, then a slower rise toward
the supply's roughly **5 V**. Record the reading near 10 seconds and
after it has nearly stopped changing. The exact values depend on the USB
voltage and the parts.

## Let it discharge

Predict first: after USB is unplugged, will the meter fall immediately to
0 V or gradually? Write down your guess.

1. **Unplug USB.** Leave the meter across the capacitor and leave the
   resistor, short jumper, and capacitor where they are.
2. Move **only the rail end of the red jumper** from the top + rail by
   column 6 to the **bottom − rail by column 6**. Its other end stays in
   j6. This closes a path from the capacitor's + leg through the 10 kΩ
   resistor to its − leg, with no USB power connected.
3. Watch the meter fall gradually toward 0 V. Record its reading about
   10 seconds after moving the wire. Compare the rise and fall with your
   predictions.
4. Keep USB **unplugged** and put that red rail end back in the **top +
   rail by column 6** before continuing to the next lesson.

The meter falls because the capacitor held separated charge after USB
was removed. The 10 kΩ resistor gives that charge a limited path back.
Never short the capacitor's legs to make the reading fall faster.

## If it doesn't work

| What you see | Check, with USB unplugged |
|---|---|
| The meter never rises | Check the Mega's 5 V and GND rail wires, the red wire from top + to j6, the resistor in g6–e6, and the jumper from b6 to b8. Check the meter is on DC volts with its red lead in V. |
| The reading has a minus sign | Swap the meter probes. The capacitor's + leg belongs in a8 and its striped − leg in the bottom − rail. |
| The reading does not fall after moving the wire | Check that USB is unplugged and that **only the rail end** of the red jumper moved to the bottom − rail by column 6; j6 must still meet the resistor. |
| A part gets warm or smells | Unplug immediately. Check the capacitor's stripe, its rating, and that the 10 kΩ resistor separates 5 V from its + leg. |

## About the sketch

This is a passive circuit powered by the Mega's **5 V power pin**. It uses
no programmable I/O pins. No upload is needed to power it: USB powers the
rails even if the Mega has an older sketch. The matching ADK sketch is
short because the meter, rather than code, shows the change:

<!-- sketch -->
