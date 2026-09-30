---
lesson: 75
promise: Press Set, let go, and see one bit stay stored in two NAND gates.
time: 25 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - SN74HC00N 14-pin PDIP NAND chip
  - 2 push buttons
  - Red LED
  - 2 10 kΩ resistors (brown, black, black, red, brown)
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 100 nF ceramic capacitor
  - 19 jumper wires
ideas:
  - Feedback between two NAND gates holds a bit after a button is released
---

## What you'll build

<!-- closeup -->

Keep the chip, buttons and LED from [Lesson 74](../074-nand-logic/index.md),
but join two NAND gates so each output feeds the other gate. The red LED
shows the bit they hold. Press **Set** and it should stay on after you
let go; press **Reset** and it should stay off. The Mega supplies USB
5 V and GND. No Mega signal pin or upload controls this latch.

## Rewire the two buttons

!!! warning "Unplug before moving wires"
    Unplug the Mega's USB cable first. Keep the LED's 1 kΩ resistor in
    series. Check the chip's notch, pin 14 at 5 V and pin 7 at GND
    before plugging USB back in. Press only one button at a time.

Keep Lesson 74's chip across columns 16–22, notch to the left; its
100 nF capacitor beside it at column 15; both buttons at columns 2
and 8; and the red LED and 1 kΩ resistor at columns 6–7. Keep the
chip's power and capacitor wires, the four wires grounding unused
inputs **9, 10, 12, 13**, the link between the − rails, and the LED
path from chip pin 3. Keep the Mega's GND and 5 V wires in their usual
rail holes.

Remove Lesson 74's two 10 kΩ **pull-down** resistors, both 5 V wires
to the buttons, both wires from the buttons' right sides to the chip,
and the two GND wires from chip pins 4 and 5. The generated steps below
show the complete new build. Put each **10 kΩ pull-up** from the top +
rail to its button's left strip. Connect each button's right strip to
the bottom − rail. Connect the Set button's left strip to chip **pin 1**
and the Reset button's left strip to **pin 4**. A released button now
holds its input high; a press pulls that input low.

<!-- bench -->

<!-- steps -->

Complete the two feedback paths: chip **pin 6 (Q-bar) → pin 2**, and
**pin 3 (Q) → pin 5**. Q at pin 3 also feeds the 1 kΩ resistor and
the LED's long leg; the short leg reaches GND. Unused outputs **8 and
11** remain open. The [SN74HC00N datasheet](https://www.ti.com/lit/ds/symlink/sn54hc00.pdf)
shows the 14-pin layout and an active-low set/reset latch example.

<!-- connections -->

## Start from a known state

When USB power first arrives, the latch's bit is **unknown**. Either
state may appear because the two gates can settle in different orders.
Plug in USB, press **Reset** once, then release it. This starts with the
LED off. Keep Set and Reset released between trials, and never press
them together.

## Predict, then try

Before the first trial, predict what the LED will show **while Set is
held**, **after Set is released**, and **after Reset is released**. Write
your three guesses. Then press Set and release it; press Reset and
release it. Record what you see after each action.

| Action | Your prediction | LED you see | Expected LED |
|---|---|---|---|
| Hold Set | ____ | ____ | On |
| Release Set | ____ | ____ | Still on |
| Hold Reset | ____ | ____ | Off |
| Release Reset | ____ | ____ | Still off |

## Why it remembers

Each 10 kΩ resistor holds its released input high. A Set press pulls
**pin 1 low**, so the first NAND output **Q goes high** and lights the
LED. Q feeds the second gate, whose output **Q-bar goes low** and feeds
back to the first. That feedback keeps Q high after Set is released.
Reset pulls **pin 4 low**, making Q-bar high and Q low; the feedback then
holds the LED off. This is one bit of state made by wires and gates,
similar to the remembered state in [Lesson 6's Simon game](../006-simon/index.md).

## If the LED surprises you

Unplug USB before checking a connection.

| What you see | Check |
|---|---|
| The LED never lights | Check pin 14 to 5 V, pin 7 to GND, Q at pin 3 through 1 kΩ to the LED's long leg, and its short leg to GND. |
| Set or Reset works only while held | Check both feedback jumpers: pin 6 to pin 2, and pin 3 to pin 5. |
| A button does the opposite job | Set's left strip goes to pin 1; Reset's left strip goes to pin 4. Each right strip goes to GND. |
| The LED changes without a press | Check both 10 kΩ pull-ups from 5 V to the left button strips, all four unused inputs at GND, and the 100 nF capacitor near the chip. |

## About the sketch

The matching ADK sketch claims no I/O pins. The gates hold the bit
without code; no upload is needed:

<!-- sketch -->

Unplug USB when finished. These LED states are calculated from the
datasheet's gate behavior; this lesson has not been recorded as tried
on physical hardware.
