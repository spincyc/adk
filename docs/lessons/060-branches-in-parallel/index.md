---
lesson: 60
promise: See two LED branches share one voltage while their currents add.
time: 25 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 2 red LEDs
  - 2 × 1 kΩ resistors (brown, black, black, brown, brown)
  - 6 jumper wires
  - Digital multimeter with DC volts (for the measurements)
ideas:
  - Parallel branches share a voltage, and their currents add
---

## What you'll build

<!-- closeup -->

Two red LEDs light from the Mega's USB-powered 5 V supply. Each has its own
1 kΩ resistor. You can disconnect one branch and watch the other LED, then
measure the voltage and work out the current in each branch.

## The idea

A **branch** is one complete path from the + rail to the − rail. Here the
first path goes through one resistor and one LED. The second path goes
through another resistor and LED. They meet only at the rails, so they are
**in parallel**. Each branch has the same voltage between its ends: about
5 V. Current can split at the + rail and come back together at the − rail.

!!! question "Predict"
    Start with only the first branch connected. When you add the second
    branch across the same rails, will the first LED get brighter, dimmer,
    or stay about as bright? What will happen to the current supplied to
    the two branches together? Write down your guesses before the trial.

## Build it

!!! warning "Unplug first"
    Take the USB cable out before changing any wire. Check that each LED
    has a 1 kΩ resistor in its own path before powering the build.

Keep the Mega's GND wire in the bottom − rail hole nearest it and its 5 V
wire in the top + rail hole nearest it. Take out the other parts and wires
from Lesson 59. The two LED paths start at separate holes of the same top
+ rail and end at separate holes of the same bottom − rail. The long leg of
each LED faces its resistor; the short leg faces the − rail.

<!-- bench -->

<!-- steps -->

The breadboard joins holes a–e in each column and holes f–j in that column,
but not across the middle gap. Each resistor crosses that gap. The branches
do not join each other except through the long power rails.

<!-- connections -->

## Try it

1. Check both paths, then plug the Mega into USB. Both LEDs should light.
2. Unplug USB. Lift only the red jumper from the top + rail to j12. Leave
   the second resistor and LED in place. Plug USB back in. Only the first
   LED should light; notice its brightness. If you have a meter, follow
   *Measure it* below and record the voltage across the first resistor.
3. Unplug USB again. Put that red jumper back in the same holes, then plug
   USB in. Compare the first LED with your prediction and look for the
   second LED. Record what you see.

The first LED should look about as bright in both trials. The LEDs need
not match each other exactly, even when both are red.

## Measure it

Before measuring, predict whether the two whole-branch voltages will match.
Will the two branch currents together be greater than the one-branch current?

With both branches connected and lit, set the meter to DC volts (**V⎓**):
black lead in **COM**, red lead in **V**. If it has a manual range, choose
20 V. Touch the probes across the places below without letting their metal
tips touch. Leave the meter in voltage mode throughout this lesson.

<!-- measure -->

Write down each reading. Each whole branch should have about 5 V from
+ to −. Across either 1 kΩ resistor you should see roughly 3 V; the red
LED uses roughly the other 2 V. The exact readings depend on your LEDs.
Compare the first resistor's reading here with its one-branch reading from
step 2. They should be close.

## Why it happens

Each branch begins at the same + rail and ends at the same − rail, so both
get the same supply voltage. The second branch gives current another path;
it does not put a second resistor in the first LED's path. That is why
adding it leaves the first LED about as bright.

Use the measured voltage across each resistor to estimate its branch's
current. For a 1 kΩ resistor, **1 V across it means 1 mA through it**.
If you measure 3 V across each one, the branch currents are about 3 mA and
3 mA. The two branches then take about **3 + 3 = 6 mA** together. This is
the current feeding the LED branches, not the whole USB current: the Mega
itself also uses power.

## Check your prediction

Did the first LED's brightness change when you reconnected the second
branch? Were the voltages across the whole branches close to each other?
Add your two calculated branch currents and compare that sum with the
one-branch trial. Write one sentence explaining the split and the sum.

## If it doesn't work

| What you see | Try this, with USB unplugged |
|---|---|
| Neither LED lights | Check the Mega's 5 V and GND rail wires, then each branch's rail wires. |
| One LED is dark | Check its long leg faces its resistor and its short leg faces −. |
| The first LED goes dark | Check the second red jumper goes from + to j12. |
| The meter reads a negative voltage | Swap its probes; keep the red lead in the V jack. |

## About the sketch

The circuit lights from the Mega's **5 V power pin**, not a programmable
signal pin. No upload is needed: plugging in USB powers the build even if
the Mega has an older sketch. The matching ADK sketch is intentionally
short and claims no I/O pins:

<!-- sketch -->

These are expected results from the circuit design; this lesson has not
been confirmed on a physical breadboard.
