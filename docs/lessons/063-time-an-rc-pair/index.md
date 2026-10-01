---
lesson: 63
promise: Double the resistance and watch a capacitor take about twice as long to charge.
time: 30 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 2 × 10 kΩ resistors (brown, black, black, red, brown)
  - 1000 µF polarized capacitor, rated at least 10 V
  - 5 jumper wires
  - Digital multimeter with DC volts
  - Stopwatch
ideas:
  - Resistance times capacitance sets the charging timescale
---

## What you'll build

<!-- closeup -->

Add one 10 kΩ resistor to [E07's capacitor circuit](../062-charge-a-capacitor/index.md).
Then time how long the capacitor takes to reach about **3.2 V**. With one
resistor it takes roughly **10 seconds**; with two in series, roughly
**20 seconds**.

## Predict

The capacitor charges quickly at first, then more slowly as its voltage
approaches the supply's roughly 5 V. One useful marker is **63% of the
final voltage**: about 3.2 V for a 5 V supply. The time to reach that
marker is called the **time constant** and is written **τ = R × C**.

E07 used one 10 kΩ resistor and a 1000 µF capacitor. Their time
constant is about 10 seconds. Putting another 10 kΩ resistor in the same
path makes 20 kΩ altogether. The capacitor and supply stay the same.

Before either timed run, write down how long you expect the capacitor to
take to reach 3.2 V with one resistor and with two. Will the second
resistor change the final voltage too?

## Build it

!!! warning "Unplug first"
    Unplug the Mega's USB cable before moving a wire or resistor. Keep
    the capacitor's **+** leg at a8 and its striped **−** leg in the
    bottom − rail by column 9. Check the stripe before each power-up.
    Never join the capacitor's legs directly with a wire.

Keep E07's red supply wire from the top + rail by column 6 to j6,
the 10 kΩ resistor across the gap from g6 to e6, the capacitor, and the
black wire joining the two − rails at column 11.
Keep the Mega's GND and 5 V rail wires in their usual holes nearest the
Mega. Remove the short jumper from b6 to b8. Add the wire from **b6 to
j8** and the second **10 kΩ** resistor from **g8 to e8**, across the
middle gap. Now current must pass through both resistors to reach the
capacitor's + leg. The drawing shows the finished two-resistor circuit.

<!-- bench -->

<!-- steps -->

These are the finished circuit's connections:

<!-- connections -->

## Try it

Set the meter to **DC volts (V⎓)**, using its 20 V range if needed. Put
the black lead in **COM** and the red lead in **V**, and keep them in
those sockets throughout the lesson. Touch the black probe to a free hole
in the bottom − rail and the red probe to **d8**, a free hole in column
8's lower strip with the capacitor's + leg at a8. Keep the probe tips
apart.

Start each timed run with the capacitor empty. **Unplug USB**, move only
the **rail end** of the red supply wire from the top + rail by column 6 to
the top − rail by column 6, one hole above, and leave its other end in
j6. E07's black wire at column 11 makes that rail GND, so the capacitor's
charge leaves slowly through the resistors. Wait until the meter reads
near **0 V**, then, with USB still unplugged, return the rail end to the
**top + rail by column 6**. Never connect the capacitor's legs directly.

1. **One resistor.** Unplug USB and move the **j8** end of the wire from
   b6 into **b8**. The wire now joins b6 to b8, as in E07, and the second
   resistor's g8 leg meets nothing, so current reaches the capacitor
   through the first 10 kΩ alone. Empty the capacitor, which takes about
   40 seconds. Check the capacitor's stripe, plug USB back in and start
   the stopwatch as power returns. Stop it when the meter first reaches
   about **3.2 V**, and record the time. Expect roughly **10 seconds**.
2. **Two resistors.** Unplug USB and move that wire end back from b8 to
   **j8**, as drawn: current must now pass through both resistors.
   Empty the capacitor again; with twice the resistance this takes about
   a minute and a half. Plug USB in and time the charge to 3.2 V in the
   same way. Expect roughly **20 seconds**. Repeat either run if you want
   a steadier comparison.

<!-- measure -->

| Charging path | Predicted time to about 3.2 V | Your time |
|---|---:|---:|
| One 10 kΩ resistor | ____ s | ____ s |
| Two 10 kΩ resistors | ____ s | ____ s |

Meter response, the moment you start the watch, the USB voltage, and
capacitor and resistor tolerances all shift the exact times. Use **63% of
your measured final voltage** as the target if your supply is noticeably
different from 5 V.

## Why it happens

The two resistors are in series: the charging current passes through one
and then the other. Together they make twice the resistance. The same
capacitor therefore charges toward the same final voltage, but takes
about twice as long to reach any given fraction of it. That is what
**τ = R × C** predicts: 10 kΩ × 1000 µF = 10 seconds, and 20 kΩ ×
1000 µF = 20 seconds.

The seconds in this circuit are a physical counterpart to the timed waits
in [Lesson 12's stopwatch](../012-stopwatch/index.md). The stopwatch
sketch counts time in code; here, the resistor and capacitor set the pace.

## Check your result

Does the two-resistor time come close to twice the one-resistor time?
Compare the ratio of your two times, and each time with your prediction.
Did the second resistor change the final voltage?

## If it doesn't work

| What you see | Check with USB unplugged |
|---|---|
| Voltage stays near 0 V during a charge | Return the red wire to the top + rail, check the Mega's USB connection, and check both resistor legs cross the middle gap. |
| Voltage jumps straight to about 5 V | Check that neither resistor has been bypassed by a jumper and that the red probe is on the capacitor's + strip at column 8. |
| Voltage rises very slowly or not at all | Check the b6-to-j8 wire and both 10 kΩ values. Keep the black probe on the bottom − rail. |
| Voltage will not fall during discharge | Check that the red wire's rail end is in the top − rail, its other end remains in j6, and the black wire joins the two − rails at column 11. Check the b6-to-j8 link. |
| Both runs take the same time | Check that the wire's end really moved between b8 and j8: in b8 it skips the second resistor, in j8 it uses it. |
| Capacitor gets warm or smells | Unplug at once. Check the striped − leg is in the bottom − rail and the + leg is at a8. |

## About the sketch

This passive circuit needs no sketch upload. Its example has the usual
ADK `setup ()` and `update ()` calls, but claims no signal pins:

<!-- sketch -->

These timings are calculated expectations. This lesson has not been
checked on a physical breadboard.
