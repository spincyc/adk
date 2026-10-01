---
lesson: 78
promise: Turn a knob, brighten an LED, and measure a steady voltage made from pulses.
time: 30 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - White LED
  - 220 Ω resistor (red, red, black, black, brown)
  - 10 kΩ potentiometer (the knob)
  - 10 kΩ resistor (brown, black, black, red, brown)
  - 100 µF polarized capacitor rated at least 10 V
  - 9 jumper wires
  - Digital multimeter with DC volts
  - Battery-powered two-channel oscilloscope and probes (optional)
ideas:
  - PWM changes the share of time a pin is on
  - A resistor and capacitor smooth PWM into an average voltage
---

## What you'll build

<!-- closeup -->

Turn a knob to brighten a white LED. A meter on a second branch of the
same PWM pin rises from about 0 V toward 5 V. The LED branch still gets
full-height pulses; a resistor and capacitor smooth the second branch.

## Predict

Pin 3 switches between 0 V and 5 V about 490 times each second. At the
knob's middle, it spends about half the time at each level. Predict how
bright the LED will look. Will a meter connected after the filter show
only 0 V and 5 V, or a number between them? Write down both guesses.

## Build it

!!! warning "Unplug before wiring"
    Unplug USB before moving parts or probes. Keep the white LED's 220 Ω
    resistor. Check the capacitor's **striped − leg** goes to the bottom
    − rail; its **+ leg** goes after the 10 kΩ resistor. Never put the
    capacitor straight between pin 3 and GND. Keep the meter on **DC
    volts**, with its red lead in **V** and black lead in **COM**.

Keep [E22's](../077-sampling/index.md) white LED, 220 Ω resistor,
knob, and their wires in place. They use their [Lesson 7](../007-dimmer/index.md)
homes: the LED in columns 38–39, its resistor across the gap at column
38, and the knob in columns 39–41 with A0 at its wiper in column 40.
Keep the Mega's GND wire in the bottom − rail hole nearest it and its
5 V wire in the top + rail hole nearest it. Add only the filter branch
shown below.

<!-- bench -->

<!-- steps -->

The new filter starts at **h38**, on pin 3's strip before the LED
resistor. A wire reaches **j44**, then the 10 kΩ resistor crosses the
middle gap from **g44 to e44**. The capacitor's **+ leg** is in **a44**;
its striped **− leg** is in the bottom − rail by column 45. Column 44's
lower strip is the filtered meter point. The capacitor does not touch
pin 3's strip directly.

This is the resistor-and-capacitor charging idea from
[E08](../063-time-an-rc-pair/index.md), repeated many times per second.
You can complete the voltage comparison with a DC meter; the later scope
section shows the individual pulses if that instrument is available.

<!-- connections -->

The LED uses roughly (5 − 3.2) V ÷ 220 Ω ≈ **8 mA** while on. The
separate filter branch can draw at most 5 V ÷ 10 kΩ = **0.5 mA** when
its capacitor starts empty. Together they stay below the Mega's
20 mA per-pin operating limit. The 10 kΩ and 100 µF pair has a time
constant of about **1 second**: after a knob change, wait about
**3 seconds** for the reading to get close to its new value.

## Code it

Open **File → Examples → Adk → lessons → 078-pwm-average** in the Arduino
IDE and upload it:

<!-- sketch -->

`knob.read ()` gives 0 to 1023. Dividing by four gives a PWM setting
from 0 to 255. `led.write (duty)` sets pin 3's on-time and therefore
controls both the LED and the filtered branch. The Serial Monitor shows
the knob reading and duty setting at **9600 baud**. The resistor and
capacitor do the smoothing; there is no second output pin.

## Try it

First turn the knob slowly from the GND end to the 5 V end. Predict the
LED's direction before you turn it. It should brighten as the printed
`duty` grows from 0 toward 255. This is the visible result, even without
a meter or scope.

Set the meter to **DC volts (V⎓)**. Put its black probe in a free hole
of the bottom − rail and its red probe in a free hole of column 44's
**lower** strip, such as **c44**. Keep the tips apart. Turn the knob
until the Serial Monitor shows `knob:256` or near it, then wait at least
three seconds. Before looking, predict whether the reading will be near
0 V, 1.25 V, or 5 V. Repeat near `knob:768`: predict the new reading,
wait, then record what you see.

| Knob reading | Predicted filtered voltage | Your reading |
|---|---:|---:|
| About 256 | About 1.25 V | ____ V |
| About 768 | About 3.75 V | ____ V |

<!-- measure -->

The meter on pin 3 before the filter can also show about 1.25 V at
one-quarter on-time.

### See the pulses, if you have a scope

Use a **battery-powered two-channel scope**. With USB unplugged, put
both probe ground clips on the bottom − rail, channel 1's tip at **i38**
and channel 2's tip at **c44**. Never put a ground clip on either signal
point. Plug in USB and set the knob near halfway. Before looking,
predict which trace will jump between 0 V and 5 V and which will sit
near 2.5 V. Start around **1 ms per division** and **1 V per division**
with DC coupling; adjust to see the roughly 490 Hz pulses.

Channel 1 should still switch almost the full 0–5 V while channel 2
stays near the average, with only a small ripple. Turning the knob
changes the width of channel 1's high part and the level of channel 2.

## Why it happens

Pin 3 is only ever at 0 V or 5 V; PWM changes how much of each cycle it
spends at 5 V. A meter on pin 3 itself is too slow to show each pulse,
so it shows about their average: 1.25 V at one-quarter on-time. The
filtered point gives a similar DC reading **and** changes only a few
millivolts between pulses. The capacitor charges through the 10 kΩ
resistor during each high part and gives charge back during each low
part; with a time constant of about a second, far longer than one
2 ms cycle, it settles at the average. More on-time lets it settle
higher. The filter did not turn pin 3 into a true analog output; it made
a smoothed voltage at the capacitor's own point.

## Check your result

Compare your two filtered readings with your predictions: did three
times the on-time give about three times the voltage? In one sentence,
explain why the capacitor's voltage barely moves between pulses.

## If it doesn't work

| What you see | Check with USB unplugged |
|---|---|
| LED never lights | Pin 3's wire ends at j38, the 220 Ω resistor crosses the gap at column 38, and the LED's short leg reaches the bottom − rail. |
| LED lights but filtered point stays near 0 V | The new wire joins h38 to j44; the 10 kΩ resistor crosses g44 to e44; the red probe touches column 44's lower strip. |
| Filtered point follows poorly | Wait three seconds after turning the knob. Check that the capacitor's + leg is in a44 and its striped − leg is in the bottom − rail. |
| Knob reading stays at one end | Check A0 reaches the knob's wiper at a40 and the two outer legs reach the top − and + rails. |
| A part gets warm or smells | Unplug at once. Check the capacitor's stripe and make sure no wire bypasses either resistor. |

These are calculated expectations. This lesson has not been checked on a
physical breadboard.
