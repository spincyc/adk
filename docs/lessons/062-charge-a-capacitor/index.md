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
  - 5 jumper wires
  - Digital multimeter with DC volts
ideas:
  - A capacitor stores separated charge
---

## What you'll build

<!-- closeup -->

The Mega's USB 5 V charges a **1000 µF capacitor** through a 10 kΩ
resistor. A meter across the capacitor shows its voltage rising. Then you
disconnect the supply and watch the capacitor hold its charge. Last, you
give that charge a path back through the same resistor, and the meter
shows the voltage falling.

## Predict

The capacitor has two legs. Charge gathers on one side while charge leaves
the other. Its voltage changes as that separation grows or shrinks. Before
you power the build, predict the meter reading just after plugging in USB
and about 10 seconds later. Will it jump straight to 5 V, or rise toward it?
What will the meter show when the capacitor is connected to nothing, and
then when the stored charge has a path through the resistor?

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
stays in the path for both charging and discharging. A black wire from the
bottom − rail to the top − rail at column 11 makes the top − rail GND too,
one hole above the red wire's rail end; you will use it to discharge the
capacitor.

<!-- bench -->

<!-- steps -->

The breadboard joins a–e in one column and f–j in that column. The gap
separates those strips, so current reaches the capacitor only by crossing
the resistor. In schematic form (the
[schematic key](../../electricity/schematics.md) names each symbol):

<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 600 270" width="600"
     role="img" aria-labelledby="charge-cap-title charge-cap-desc">
  <title id="charge-cap-title">Capacitor charging through a resistor schematic</title>
  <desc id="charge-cap-desc">Five volts reaches the red wire's rail end, then passes through a 10 kilohm resistor to the capacitor's + leg, where the meter's red probe touches d8. The 1000 microfarad electrolytic capacitor joins that node to ground. To hold the charge the wire's rail end moves to j7, and to discharge it moves to the top − rail, which is ground.</desc>
  <g fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round">
    <circle cx="70" cy="60" r="5"/>
    <path d="M75 60 L170.0 60"/>
    <path d="M240.0 60 L360 60"/>
    <rect x="170.0" y="47" width="70" height="26"/>
    <path d="M360 60 L360 104.0"/>
    <path d="M340 104.0 H380"/>
    <path d="M340 122.0 Q360 112.0 380 122.0"/>
    <path d="M360 117.0 L360 160"/>
    <path d="M360 160 V168 M340 168 H380 M347 176 H373 M354 184 H366"/>
  </g>
  <g fill="currentColor">
    <circle cx="360" cy="60" r="5"/>
  </g>
  <g fill="currentColor" font-size="17" font-family="system-ui, sans-serif">
    <text x="20" y="66">5 V</text>
    <text x="70" y="96" text-anchor="middle" font-size="15">rail end</text>
    <text x="205" y="36" text-anchor="middle">10 kΩ</text>
    <text x="360" y="36" text-anchor="middle">meter · d8</text>
    <text x="326" y="102.0" font-size="16">+</text>
    <text x="392" y="118">1000 µF</text>
    <text x="40" y="226" font-size="15">To hold the charge, the red wire's rail end moves to j7, where it</text>
    <text x="40" y="246" font-size="15">meets nothing; to let it go, into the top − rail, which is GND.</text>
  </g>
</svg>

These are the finished connections:

<!-- connections -->

## Try it

### Charge it

Set the meter to **DC volts (V⎓)**, with black lead in **COM** and red lead
in **V**; choose a 20 V range if the meter needs one. Keep it in voltage
mode throughout this lesson. Put the black probe in a free hole of the
bottom − rail and the red probe in **d8**, a free hole in column 8's lower
strip with the capacitor's + leg. Keep the metal probe tips apart.

<!-- measure -->

Check the capacitor's stripe and the resistor path once more, then plug in
USB. Watch the meter for about 30 seconds. If the capacitor began near
0 V, expect about **3.2 V after 10 seconds**, then a slower rise toward
the supply's roughly **5 V**. Record the reading near 10 seconds and
after it has nearly stopped changing. The exact values depend on the USB
voltage and the parts.

### Hold the charge, then let it go

Predict first: when the red jumper connects the capacitor to nothing, will
the meter fall to 0 V, fall gradually, or stay nearly still? When that
jumper then reaches GND, how will the reading change? Write down both
guesses. Leave the meter across the capacitor throughout.

1. **Unplug USB**, then straight away lift **only the rail end of the red
   jumper** out of the top + rail and push it into **j7**, a free hole
   whose strip holds nothing else. Its other end stays in j6. Be quick:
   until you lift it, the capacitor pushes charge back through the
   resistor into the Mega's own unpowered 5 V circuits, which its green
   ON light holds near 2 V. About 0.3 mA flows, enough to lower the
   reading by about 0.3 V every second, so a pause of two or three seconds
   costs 0.6–1 V. That is why the next step records the reading only once
   the end is in j7.
2. Watch the meter for 30 seconds and record the reading at the start and
   the end: \_\_\_\_ V, \_\_\_\_ V. With nowhere for its charge to go, the
   capacitor should hold its voltage: it may creep down a little, through
   the meter itself and a tiny leak inside the capacitor.
3. Now move that same end from j7 into the **top − rail by column 6**,
   one hole above where it started. The black wire at column 11 joins this
   rail to GND, so the jumper closes a path from the capacitor's + leg
   through the 10 kΩ resistor to its − leg, with no USB power connected.
4. Watch the meter fall gradually. About 10 seconds after moving the wire it
   should read about a third of its starting value, and after 30 seconds
   only a few tenths of a volt. Record the 10-second reading.
5. Keep USB **unplugged** and put that red rail end back in the **top +
   rail by column 6** before continuing to E08.

## Why it happens

While USB is plugged in, current flows through the resistor and piles
charge up on the capacitor's + side, while as much leaves its − side. The
capacitor's voltage grows with that separated charge, quickly at first,
then more slowly as it nears the supply's 5 V. The meter held still while
the capacitor had no path, because the charge had nowhere to go: the
capacitor stored it after the source left. It fell once the resistor gave
that charge a way back round to the − side. The 10 kΩ resistor keeps that
current small, so the fall takes several seconds. Never short the
capacitor's legs to make the reading fall faster.

## Check your result

Compare the rise, the hold and the fall with your predictions. In one
sentence, explain how the hold shows that the charge was still stored in
the capacitor after USB was unplugged.

## If it doesn't work

| What you see | Check, with USB unplugged |
|---|---|
| The meter never rises | Check the Mega's 5 V and GND rail wires, the red wire from top + to j6, the resistor in g6–e6, and the jumper from b6 to b8. Check the meter is on DC volts with its red lead in V. |
| The reading has a minus sign | Swap the meter probes. The capacitor's + leg belongs in a8 and its striped − leg in the bottom − rail. |
| The reading falls fast while the jumper's end is in j7 | Check that the end is in j7, not in a rail, and that nothing else is in column 7's upper strip. Check the meter is on DC volts. |
| The reading does not fall after moving the wire to the top − rail | Check that **only the rail end** of the red jumper moved, into the top − rail by column 6, and that the black wire joins the bottom − rail to the top − rail at column 11; j6 must still meet the resistor. |
| A part gets warm or smells | Unplug immediately. Check the capacitor's stripe, its rating, and that the 10 kΩ resistor separates 5 V from its + leg. |

## About the sketch

This is a passive circuit powered by the Mega's **5 V power pin**. It uses
no programmable I/O pins. No upload is needed to power it: USB powers the
rails even if the Mega has an older sketch. The matching ADK sketch is
short because the meter, rather than code, shows the change:

<!-- sketch -->

These are expected results; the circuit has not been recorded as tested on
hardware.
