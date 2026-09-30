---
lesson: 61
promise: Turn a knob and measure the changing voltage at its middle leg.
time: 20 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 10 kΩ potentiometer (the knob)
  - 6 jumper wires
  - Digital multimeter with DC volts
ideas:
  - A potentiometer's wiper gives an adjustable fraction of its supply voltage
---

## What you'll build

<!-- closeup -->

The knob sits between the Mega's 5 V and GND. Its middle leg, called the
**wiper**, feeds A0. A meter shows the voltage at the wiper while the
Arduino IDE Serial Plotter draws A0's reading. This is the knob circuit from
[Lesson 7](../007-dimmer/index.md), without its LED.

## Predict

The knob's two outer legs sit at opposite ends of a resistive strip. Turning
the knob moves the wiper along that strip. If you turn it to about the
middle, what voltage do you expect between the wiper and GND? What A0
reading do you expect between 0 and 1023? Write down both guesses.

In [E04](../059-resistors-in-series/index.md), two resistors shared a
voltage. Here the wiper moves the point where you tap that shared voltage.

## Build it

!!! warning "Unplug first"
    Unplug the Mega before moving any wires. Remove any AA battery pack from
    the previous build. This knob uses only the Mega's USB 5 V. Keep the
    wiper on A0 alone: joining it to a power rail could short 5 V to GND
    when the knob reaches an end.

Use the knob's home from Lesson 7: its outer legs go in **f39** and **f41**,
across the breadboard gap from the wiper in **d40**. The black jumper from
**j39** reaches the top − rail; the red jumper from **j41** reaches the top +
rail. A0's wire enters **a40**, in the wiper's strip. Follow the steps below
for the rail feeds and the − rail link.

<!-- bench -->

<!-- steps -->

The finished circuit makes these connections:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 061-tap-a-divider** in the Arduino
IDE. The sketch reads A0 and prints one labeled number for the Serial
Plotter.

<!-- sketch -->

`adk::AnalogInput knob {A0};` claims A0 as an input. `knob.read ()` gives
0 near GND and 1023 near 5 V. `adk::wait (50)` leaves a short pause between
points on the graph.

## Upload and measure

1. Check the knob's three wires and the rail feeds. Plug the Mega into USB.
2. In the Arduino IDE, select **Tools → Board → ADK Boards → ADK Mega 2560**
   and the Mega's **Tools → Port** entry, then press **Upload**. See
   [Getting started](../../start.md) if the board choice is missing.
3. Open **Tools → Serial Plotter** at **9600 baud**. Turn the knob slowly
   from one end to the other. The A0 trace should move from near 0 to near
   1023. If you want to read an exact number, close the Plotter and open
   **Tools → Serial Monitor** at the same baud rate.
4. Set the meter to **DC volts** (V⎓), with black lead in **COM** and red
   lead in **V**. Touch black to a free hole in the − rail and red to a
   free hole in the wiper's strip at column 40. Keep the leads in the
   voltage jacks; a current setting across this circuit could short it.
5. Turn to about the middle. Record the meter voltage and A0 reading, then
   compare them with your prediction. Move slowly to both ends and record
   each pair too. Leave the meter in place while you turn; unplug before
   moving any wires.

<!-- measure -->

At the GND end, the wiper should be near **0 V** and A0 near **0**. At the
5 V end, it should be near **5 V** and A0 near **1023**. Around **2.5 V**,
expect about **512**. The wiper taps a fraction of the voltage across the
strip, so the plotted number and meter voltage rise together. A mechanical
half-turn may not put the wiper at exactly half the electrical range; the
USB supply and meter readings may also differ a little from these rounded
values. Compare the measured pairs, not an exact halfway position.

## If it doesn't work

| What you see | Check |
|---|---|
| No A0 trace | Select the Mega's port, set 9600 baud, and close the Serial Monitor before opening the Plotter. |
| A0 stays near 0 or 1023 as you turn | Unplug. Check the outer-leg wires from j39 to the top − rail and j41 to the top + rail, and A0 at a40. Check the rail feeds and the − rail link. |
| A0 jumps around | Unplug. Press the knob fully into f39, d40 and f41, and reseat the A0 jumper at a40. |
| Meter reads 0 V while A0 changes | Check that the meter is on DC volts, its black lead touches the − rail, and its red lead touches a free hole in column 40's lower strip. |
| Knob or wire gets warm | Unplug at once. Check that the wiper's strip connects only to A0 and that the outer legs reach different rails. |

This investigation adds a voltage measurement to [Lesson 7's divider](../007-dimmer/index.md).
A later sampling investigation looks more closely at the numbered steps
inside the A0 reading.
