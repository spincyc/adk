---
lesson: 76
promise: Watch a capacitor and a Schmitt inverter make an LED blink, then slow it down.
time: 25 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - SN74HC14N Schmitt inverter, 14-pin DIP
  - Red LED
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 2 × 100 kΩ resistors (brown, black, black, orange, brown)
  - 10 µF polarized capacitor, rated at least 10 V
  - 100 nF ceramic capacitor
  - 16 jumper wires
  - 1 extra jumper for the slower comparison
ideas:
  - An RC path and Schmitt thresholds can make a repeating clock
---

## What you'll build

<!-- closeup -->

One inverter in the **SN74HC14N** watches a capacitor charge and
discharge. Its output changes each time the input crosses a switching
threshold, so a red LED should blink. The Mega supplies **5 V from USB**;
its sketch does not control the LED.

## Predict

The inverter's output feeds its input through a **100 kΩ resistor**. The
**10 µF capacitor** joins that input to GND. Before building, predict:
will the LED stay on, stay off, or blink when you connect USB? Write down
your guess.

## Build it

!!! warning "Unplug USB first"
    Disconnect USB before touching the circuit. Put the capacitor's
    **+ leg** at c16 and its **striped − leg** at c15. Keep the **1 kΩ
    resistor** between the chip's output and the LED. Never connect an
    output directly to GND.

Carry on from [E20](../075-set-reset-latch/index.md). The LED and
its 1 kΩ resistor remain at their home in columns 6 and 7. Keep the
nearby 100 nF supply capacitor and the standard Mega power wires.
Follow the generated steps to remove the NAND chip and old button
connections, then fit the SN74HC14N **across the middle gap with its notch
to the left**. In the [SN74HC14 datasheet](https://www.ti.com/lit/ds/symlink/sn74hc14.pdf),
pin 1 is the first inverter's input, pin 2 its output, pin 14 VCC and
pin 7 GND. The five unused inputs go to GND; their outputs stay open.

<!-- bench -->

<!-- steps -->

The finished circuit's connections are:

<!-- connections -->

## Watch the clock

Check the chip's notch, the capacitor stripe, the 100 nF supply
capacitor and the LED's 1 kΩ resistor. Plug in USB and watch the red LED.
Record whether it blinks, stays lit or stays dark. The exact blink pace
depends on the parts and the chip.

The SN74HC14 has two input switching thresholds. As the capacitor
charges through the feedback resistor and reaches the upper threshold,
the inverter changes its output. The capacitor then discharges through
the same resistor. At the lower threshold, the output changes back. This
repeats, making a clock without timed code.

## Slow it down

Predict whether **two 100 kΩ resistors in series** will make the LED
blink faster or slower. Write that down before changing anything.

1. **Unplug USB.** Leave the first 100 kΩ resistor in g11 and e11.
   Move only the **b11 end** of the input jumper to **b13**; its other
   end stays in b16 beside chip pin 1.
2. Put the second 100 kΩ resistor across the middle gap, from **g13
   to e13**. Add one jumper from **a11 to j13**. The path is now chip
   output → first resistor → second resistor → chip input. Leave both
   capacitors and the LED branch in place.
3. Check that neither resistor is bypassed, then reconnect USB. Watch
   for a slower blink and record what you see. Unplug again before
   restoring the one-resistor build shown above.

| Feedback path | Your prediction | Your observation |
|---|---|---|
| One 100 kΩ resistor | ____ | ____ |
| Two 100 kΩ resistors in series | ____ | ____ |

The second resistor makes it take longer for the capacitor to cross each
threshold. Compare your observations with your predictions. This is the
same RC timing idea as [E08](../063-time-an-rc-pair/index.md), now
repeated by the inverter. It is a physical cousin of the timed events in
[Lesson 12](../012-stopwatch/index.md).

## About the sketch

No upload is needed. The matching ADK example claims no signal pins;
USB powers the passive circuit:

<!-- sketch -->

## If it does not blink

Unplug USB before checking any connection.

- **LED stays dark:** Check the Mega's 5 V and GND rail wires, chip pin 14
  to 5 V and pin 7 to GND. Check the LED's long leg at b6 and short leg
  at b7.
- **LED stays lit:** Check the 100 kΩ path from pin 2 back to pin 1.
  The 10 µF capacitor's + leg goes in c16 and its striped − leg in c15.
- **Blink does not slow:** Check that the two 100 kΩ resistors connect
  end to end through a11 to j13, with the input jumper moved to b13.
- **Capacitor gets warm or smells:** Unplug at once. Check its stripe and
  both chip supply wires before reconnecting.

These are expected observations from the circuit and datasheet. This
lesson has not been recorded as tried on a physical breadboard.
