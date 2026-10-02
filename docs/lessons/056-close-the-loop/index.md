---
lesson: 56
promise: Open and close a steady LED circuit to see why current needs a complete loop.
time: 15 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Red LED
  - 220 Ω resistor (red, red, black, black, brown)
  - 4 jumper wires
ideas:
  - A source, a load, and a return make a complete path
---

## What you'll build

<!-- closeup -->

A red LED lights steadily from the Mega's USB-powered **5 V** supply. You
will open its path back to **GND**, see what happens, and restore it. This
first electricity experiment needs no upload: the LED has its own path
through a resistor.

## Predict

Trace the path in the drawing with a finger: from the top **+** rail, through
the resistor and LED, to the bottom **−** rail, then back to the Mega. If you
remove the Mega's GND wire from the bottom rail, will the LED stay lit?
Write down your prediction.

## Build it

!!! warning "Unplug before wiring"
    Unplug the USB cable before building or changing the circuit. Never
    connect 5 V straight to GND, and never power the LED without its 220 Ω
    resistor. Check that the LED's short leg points toward GND before
    plugging in. If anything gets hot or smells, unplug at once.

Start with an empty breadboard, then put the Mega's GND and 5 V wires in
their usual rail holes as the steps show. The resistor and red LED go in their
usual column 6 holes. A short red jumper brings power from the **top +
rail** to the resistor.
The black jumper takes the LED's short-leg side to the **bottom − rail**.

<!-- bench -->

<!-- steps -->

The finished circuit makes these connections:

<!-- connections -->

## Try it

1. Check the whole path and plug the Mega into USB. Predict whether the LED
   will light immediately. Look at it and record what you see: __________.
2. **Unplug USB.** Lift only the Mega's GND wire from the bottom − rail.
   Leave the LED, resistor, and all other wires in place. Plug USB back in.
   Is the LED on or off? Record it: __________.
3. **Unplug USB again.** Put the GND wire back in the same bottom − rail
   hole nearest the Mega. Plug USB back in. Record what you see: __________.

## Why it happens

Current can pass only around a complete loop, and this loop has three
parts: a **source** that pushes the current, here the Mega's USB 5 V; a
**load** that uses it, here the LED; and a **return** that carries it back
to the source, here the GND wire. Removing the GND wire opens that loop,
even though the top + rail still has 5 V. The 220 Ω resistor limits
current so the LED can stay lit safely.

## Check your result

Compare your three observations with your prediction. In one sentence,
explain why lifting only the GND wire turned the LED off while the top +
rail still had 5 V.

## If it doesn't work

| What you see | Check after unplugging USB |
|---|---|
| The LED never lights | Check the 5 V and GND rail wires, the red jumper to j6, the resistor across the middle gap, and the LED's direction. |
| The LED stays lit with GND removed | Look for another wire connecting the LED's short-leg side to the Mega's GND. |
| The LED stays dark after restoring GND | Put the black wire back in the bottom − rail hole nearest the Mega and check the other end is in the Mega's GND. |

## About the sketch

There is no code to upload for this circuit. The example below has empty
`setup ()` and `loop ()` functions because Arduino examples need those
functions. It claims no signal pins. USB alone supplies the power used here.

<!-- sketch -->

Leave the complete circuit in place for E02, and unplug USB if you are
stopping now. This is the expected behavior; the circuit has not been
recorded as tested on hardware.
