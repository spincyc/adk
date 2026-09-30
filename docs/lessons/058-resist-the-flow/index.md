---
lesson: 58
promise: Change one resistor and use a meter to see why the LED gets dimmer.
time: 35 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Red LED
  - 220 Ω resistor (red, red, black, black, brown)
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 2 kΩ resistor (red, black, black, brown, brown)
  - 3 jumper wires
  - Multimeter with DC volts
ideas:
  - Greater resistance reduces current through the same LED
---

## What you'll build

<!-- closeup -->

The red LED from [Lesson 57](../057-measure-across-and-through/index.md)
blinks slowly: on for ten seconds, off for two. You will change only its
resistor and compare how brightly it glows. Then you will use a meter to
estimate the current with each resistor.

## The idea

Pin 26 supplies about 5 V when it is on. Current follows one path: from
pin 26, through the resistor and LED, then back to GND. A resistor makes it
harder for current to pass. With the same supply and LED, a **larger
resistance means less current** and usually a dimmer LED.

A voltage reading across the resistor tells us how much of the supply is
across it. Divide that reading by the resistance to estimate the current:

<p class="formula">current in mA ≈ 1000 × resistor voltage in V ÷ resistance in Ω</p>

For example, if 3.0 V is across a 220 Ω resistor, the current is about
`1000 × 3.0 ÷ 220 = 13.6 mA`. The same current passes through the LED because
there is only one path. As current changes, the LED's own voltage changes
slightly too, so the resistor's voltage may change. Your three currents
will not necessarily be in the exact ratios of the three resistances, and
brightness does not follow a simple ratio either.

!!! question "Predict"
    Write down which resistor you think will give the **brightest** LED
    and which will give the **smallest current**: 220 Ω, 1 kΩ, or 2 kΩ.
    Will all three still let the LED light?

## Build it

!!! warning "Unplug before every swap"
    Unplug the USB cable before changing the resistor or any wire. Never
    run the LED with no resistor; that can damage the LED and the Mega's
    pin. Check the two resistor legs are in the same holes before plugging
    the cable back in.

Keep Lesson 57's red LED on pin 26, its 220 Ω resistor, and the return to
the bottom − rail. The drawing shows this starting circuit. If you took
it apart, build it as shown. The resistor's legs go in **g6 and e6**,
across the middle gap. You will put the other resistors in exactly those
two holes for the experiment, one at a time. Keep the Mega's GND wire in
the bottom − rail hole nearest the Mega.

<!-- bench -->

<!-- steps -->

These are the connections with the 220 Ω resistor in place:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 058-resist-the-flow** in the
Arduino IDE:

<!-- sketch -->

`adk::Led led {26};` tells ADK which pin drives the LED. `adk::setup ()`
gets the pin ready. In `loop ()`, the LED stays on for 10,000 milliseconds
(ten seconds), then off for 2,000 milliseconds (two seconds). This gives
the meter time to settle while the LED is on.

## Upload it

1. With the 220 Ω resistor in place, plug the Mega into your computer.
2. Choose **Tools → Board → ADK Boards → ADK Mega 2560** and the board's
   **Tools → Port**. [Getting started](../../start.md) shows how to add the
   board if it is missing.
3. Press **Upload**. When the IDE says *Done uploading*, watch a full blink:
   ten seconds on, two seconds off.

!!! tip "From the command line"
    With `arduino-cli` installed, `make upload EXAMPLE=lessons/058-resist-the-flow`
    compiles and uploads this sketch.

## Change one resistor

Keep the USB supply, LED, sketch, wires, and viewing angle the same for
all three trials. Watch an on period with **220 Ω** and record how bright
the LED looks. Then:

1. Unplug the USB cable. Take out only the 220 Ω resistor. Put the **1 kΩ**
   resistor in g6 and e6, and check both legs are seated. Plug the cable
   back in. Watch a full on period and record the brightness.
2. Predict whether **2 kΩ** will make the LED brighter or dimmer than 1 kΩ.
   Unplug, swap only the resistor, and plug in again to check.

All three resistors limit the current. The LED should glow less strongly
as the resistance rises. Its change in brightness may be easier to see
between 220 Ω and 2 kΩ than between 1 kΩ and 2 kΩ.

## Measure it

Set the meter to **DC volts (V⎓)**, with black lead in **COM** and red lead
in **V**. Use the 20 V range if your meter asks for one. Keep the leads in
the voltage jacks throughout this experiment. During the ten-second on
period, use the probe positions below. The red probe touches **h6** (the
resistor's pin 26 side); the black probe touches **d6** (its LED side).
Those holes stay on the correct strips for every resistor. Do not move or
remove the resistor while the USB cable is plugged in.

<!-- measure -->

Measure with each resistor, unplugging before each swap. Fill in the
readings and calculate `1000 × voltage ÷ resistance`:

| Resistor | LED brightness | Voltage across resistor | Approximate current |
|---|---|---|---|
| 220 Ω | | ____ V | ____ mA |
| 1 kΩ = 1000 Ω | | ____ V | ____ mA |
| 2 kΩ = 2000 Ω | | ____ V | ____ mA |

The resistor voltage may stay near 3 V rather than rising in step with
resistance. The LED takes about 2 V, but its share shifts a little when
current changes. Your measurements, not the example number above, give
the currents to compare. The current should fall as resistance rises.

When you finish, **unplug and restore the 220 Ω resistor** in g6 and e6.
Leave that starting circuit ready for the next lesson.

## If it doesn't work

| What you see | Try this |
|---|---|
| The LED is dark with every resistor | Wait through the two-second off period. Check the orange wire reaches pin 26, the LED's long leg is in b6, and the black jumper joins a7 to the bottom − rail. |
| The LED stays dark after a swap | Unplug. Check both resistor legs go across the middle gap, in g6 and e6, rather than into one strip. Check its value against the [resistor bands](../../kit.md#reading-resistors). |
| The reading jumps between a voltage and zero | Take the reading during the ten-second on period. Keep the probes in h6 and d6 until the number settles. |
| The meter reads close to zero throughout the on period | Check that the red lead is in the V jack and the dial is on DC volts, then check the probe holes. |
| Upload fails | Check the board and port in the **Tools** menu. Some USB cables carry power but no data. |

## Check your prediction

Compare your table with the order you predicted. Which change did the
meter show more clearly than your eyes did? Explain your result with the
one path through the resistor and LED: increasing the resistance reduced
the current in that path. These are expected results from the circuit;
this lesson has not been recorded as tested on hardware.
