# Change one thing, then explain it

A design challenge starts with a working circuit and a prediction. Draw
its nodes, decide which one change answers your question, and record the
result before changing anything else. If a result surprises you, use the
[diagnosis guide](diagnose.md) to check the build.

## Load a divider {#loaded-divider}

**Question:** A pair of equal resistors gives a tap near 2.5 V. Will that
tap stay there when another resistor draws current from it? This is a
complete USB-powered experiment using **three 1 kΩ resistors**, a Mega,
breadboard, jumper wires and a DC voltmeter from the kit-and-meter route.
No sketch or upload is needed. It builds on [E04's two-resistor idea](../lessons/059-resistors-in-series/index.md),
but the steps below include the entire circuit.

```text
                   1 kΩ
5 V ──[1 kΩ]──●────[    ]──── GND
              │ tap
              └────[1 kΩ]──── GND   ← add this load second
```

The upper 1 kΩ after the tap is the original lower divider resistor;
the bottom 1 kΩ is the added **load**. Both end at GND, so they are in
parallel. Predict the tap voltage **before** and **after** adding the
load. Write both numbers down.

1. **Unplug USB.** Connect the Mega's GND to the bottom − rail hole
   nearest it and 5 V to the top + rail hole nearest it. Wire the top +
   rail beside column 4 to **j4**. Put the first 1 kΩ resistor from
   **i4 to i7**, and the second from **g7 to g10**. Wire **f10** to the
   bottom − rail beside column 10. In the f–j half of column 7, the two
   resistors meet at the tap. Check that the only path from 5 V to GND
   goes through both resistors.
2. Set the meter to **DC volts**, with black lead in **COM** and red lead
   in **V**. Keep the metal tips apart. Plug USB in. Put black on a free
   hole in the grounded bottom − rail and red on **f7**. Record the
   unloaded tap: ____ V. It should be near **2.5 V**. Unplug USB.
3. Add the third 1 kΩ resistor from **h7 to h11**. Add a jumper from
   **j11** to the bottom − rail beside column 11. The load now joins
   the same tap at column 7 to GND. Check its two legs are in different
   strips and that no wire joins the top + rail straight to the bottom
   − rail. Plug USB in and measure **f7 to GND** again: ____ V.
4. Compare your readings with your two predictions. Unplug before moving
   or removing the load. Leave the meter's red lead in its voltage jack.

**Explain:** The two lower 1 kΩ resistors each connect the tap to GND,
so together they act like **500 Ω**. The upper 1 kΩ and that 500 Ω
share the supply: 5 V × 500 ÷ (1000 + 500) ≈ **1.67 V** at the tap.
The real USB voltage and resistor values can shift both readings. The
load draws current from the tap and lowers its voltage; a voltage
divider's unloaded result is not guaranteed once it feeds something.
This is why the [Lesson 7 knob](../lessons/007-dimmer/index.md) and
other sensor outputs must be considered with whatever they drive.

**Design follow-up:** Predict whether a **2 kΩ load** would pull the tap
down more or less than the 1 kΩ load. With USB unplugged, swap only the
third resistor, then measure again. A 2 kΩ load draws less current and
should leave the tap closer to its unloaded reading. Restore the 1 kΩ
load or remove it with USB unplugged.

These voltages are calculations to check against your own readings; no
physical trial has been recorded for this challenge.
