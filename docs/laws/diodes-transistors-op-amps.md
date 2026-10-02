# Diodes, transistors and op-amps

Parts made of semiconductor, which don't follow Ohm's law. A diode lets
current through one way only, at a nearly fixed voltage; a transistor
lets a small current control a much larger one; an op-amp, a chip of
many transistors, makes a copy of a voltage, larger or able to drive a
load. If you are working through the electricity course, do E09, E10,
E16 and E17 first: this page gives away what they find.

## Diodes and LEDs: one way, at a fixed voltage {#forward-voltage}

<!-- law forward-voltage -->

A diode conducts from its **anode** to its **cathode**, the end marked
with a band, and blocks the other way. Pointed the right way, it does
almost nothing until its voltage reaches a threshold, the **forward
voltage**, and then lets through as much current as the rest of the
circuit allows while its own voltage hardly rises:

| Diode | Forward voltage, at a few milliamps |
|---|---|
| Silicon diode, such as the 1N4007 | About 0.7 V |
| A transistor's base–emitter junction | About 0.7 V |
| Red or yellow LED | About 2 V |
| Green LED | About 2 to 3 V, depending on its type |
| Blue or white LED | About 3 V |

An LED is a diode that turns some of its power into light: the more
current, the brighter, up to its limit of about 20 mA.

Because its current climbs so steeply once the threshold is reached, a
diode or LED can't set its own current. Something else must: for an LED,
its resistor, [chosen with Ohm's law](ohms-law.md#led-resistor) from the
voltage left over. E03 shows the LED keeping about 2 V while the resistor
changes, and E09 shows the one-way rule: the LED lights only when the
1N4007 points along the path.

Backwards, a diode blocks up to its rated reverse voltage, 1000 V for
the 1N4007. An LED is rated to block only about 5 V: enough for a
backwards LED on the Mega's 5 V to stay dark unharmed, but no more.

Uses in the course:

- **One-way paths**, as in E09.
- **Flyback protection** across a coil, in Lesson 3, Lesson 35 and E12
  (see [Capacitors and coils](capacitors-and-coils.md#flyback)).
- **Lights**, everywhere from Lesson 1 on.

## The transistor switch {#transistor-switch}

<!-- law transistor-switch -->

The kit's S8050 is an **NPN transistor** with three legs: the **emitter**
(E), the **base** (B) and the **collector** (C). Current into the base,
out of the emitter, lets a much larger current flow from the collector to
the emitter. Before it is fully on, the ratio of collector current to
base current is its **gain**, β. Gain depends on the manufacturer, grade,
current and temperature; 100 is an example value, not a guaranteed
minimum. For example, [onsemi's SS8050 datasheet](https://www.onsemi.com/pdf/datasheet/ss8050-d.pdf#page=2)
specifies minima of 45 at 5 mA and 85 at 100 mA, both with 1 V from
collector to emitter. Those are gain tests, not saturation guarantees.
The base and emitter behave like a diode, so the base sits about 0.7 V
above the emitter at the small currents used here.

Used as a switch, the load goes between + and the collector and the
emitter goes to GND:

- **Off:** no base current, no collector current. A 10 kΩ resistor from
  base to GND makes sure of that while the Mega starts and its pin floats.
- **On:** enough base current to make the transistor fully on,
  **saturated**. At these small loads, expect about 0.1 to 0.2 V between
  collector and emitter, so the load gets nearly the whole supply.

### Choosing the base resistor {#base-resistor}

1. **The load's current:** Lesson 3's active buzzer can want 30 mA.
2. **Allow generous base drive:** use a collector/base current ratio of
   about 10 as a conservative starting point: 30 mA ÷ 10 = 3 mA into
   the base. This chosen ratio is not the transistor's gain. Onsemi also
   uses a ratio of 10 for its saturation test, at 800 mA and 80 mA.
3. **Allow some margin:** aim for about 4 mA, well within a pin's 20 mA.
   Check the exact part's specifications and the collector voltage under
   load; the datasheet's test at 800 mA does not specify every smaller load.
4. **Ohm's law** for the resistor, which has the pin's 5 V less the base's
   0.7 V across it: R = 4.3 V ÷ 4 mA ≈ 1 kΩ.

<!-- drawing 065-control-with-a-transistor closeup -->

E10 has you check exactly this. It predicts about 4 mA into the base
through 1 kΩ and 14 mA through the LED; with 10 kΩ, only about 0.36 mA
into the base, a fortieth of the LED's current, yet the LED should stay
as bright. That is a prediction to check with your transistor, not a
guarantee derived from an assumed gain of 100.

A single small transistor is enough for a buzzer or a few LEDs. Motors
and steppers use **driver chips** with several transistors and their
flyback diodes built in: the L293D in Lesson 20, the ULN2003 board in
Lesson 31. The relay module of Lesson 35 carries its own transistor.

## The op-amp with feedback {#op-amp}

<!-- law op-amp -->

An **operational amplifier**, or op-amp, has two inputs, + and −, and an
output. Its inputs draw almost no current, and it drives its output up
or down by a huge factor of the difference between them. Joined back
from the output to the − input, that output moves until the − input
matches the + input, and then holds there. That is **negative
feedback**, and everything an op-amp does in the course follows from
it.

### The follower {#follower}

Join the output straight to the − input and the output matches the +
input: a gain of 1. That sounds useless, but the + input draws nothing
while the output can give milliamps. E17 uses it to stop a load pulling
down the knob's wiper: hung straight on the wiper, a 1 kΩ load drags
2.0 V down to about 0.6 V (see [loading a divider](dividers.md#loading));
behind a follower, the output stays near 2.0 V.

### Gain {#gain}

Feed back only part of the output, through a divider of R<sub>f</sub>
from the output to the − input and R<sub>g</sub> from there to GND, and
the output must rise until that part matches the + input:

<p class="formula">gain = 1 + <span class="fraction"><span>R<sub>f</sub></span><span>R<sub>g</sub></span></span></p>

E16 uses 10 kΩ and 10 kΩ, a gain of 2: a wave swinging 0.5 V to 1.5 V
comes out swinging 1 V to 3 V. A second 10 kΩ in series in R<sub>f</sub>
makes the gain 3. An output can't swing beyond the chip's own supply:
the MCP6002 reaches to within a few hundredths of a volt of 0 V and 5 V,
so 1.5 V to 4.5 V still fits, but a gain of 4 would flatten the top of
the wave.

## Check yourself

1. A red LED and its 220 Ω resistor go across 5 V, but the LED is in
   backwards. What does a meter read across the LED, and across the
   resistor?
2. A blue LED keeps about 3 V. What current does 220 Ω give it from 5 V?
3. A load wants 100 mA. With a chosen collector/base current ratio of 10,
   is 1 kΩ from a 5 V pin enough base drive? Use 0.7 V at the base.
4. Why does the load go on the collector side, not between the emitter
   and GND?
5. An op-amp has R<sub>f</sub> = 30 kΩ and R<sub>g</sub> = 10 kΩ. What
   does a 1.0 V input give?

??? note "Answers"
    1. No current flows, so the resistor has nothing across it, 0 V, and
       the LED has the whole 5 V. By Kirchhoff's voltage law the two
       still add to 5 V.
    2. 2 V ÷ 220 Ω ≈ 9 mA, dimmer than a red LED on the same resistor.
    3. The chosen ratio calls for 100 mA ÷ 10 = 10 mA. The 1 kΩ
       resistor gives about 4.3 mA, so it does not meet that design rule.
       Gain alone cannot tell you whether it will saturate; check the
       transistor's ratings and saturation specifications before choosing
       a smaller resistor.
    4. With the emitter at GND, the base needs only 0.7 V to switch it
       fully on. A load below the emitter would lift the emitter, and the
       pin's 5 V could no longer turn the transistor fully on.
    5. The gain is 1 + 30 ÷ 10 = 4, so 4.0 V, if the chip's supply is
       5 V.

## Lessons that rely on them

<!-- relied on -->
