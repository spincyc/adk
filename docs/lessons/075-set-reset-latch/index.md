---
lesson: 75
promise: Press Set, let go, and see one bit stay stored in two NAND gates.
time: 25 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - SN74HC00N 14-pin PDIP NAND chip
  - 2 push buttons
  - Red LED
  - 2 10 kΩ resistors (brown, black, black, red, brown)
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 100 nF ceramic capacitor
  - 17 jumper wires
ideas:
  - Feedback between two NAND gates holds a bit after a button is released
laws:
  - {law: logic-levels, section: why-it-happens, for: "Traces high and low levels round the latch"}
  - {law: pull, section: why-it-happens, for: "Holds released Set and Reset inputs high"}
---

## What you'll build

<!-- closeup -->

Keep the chip, buttons and LED from [E19](../074-nand-logic/index.md),
but join two NAND gates so each output feeds the other gate. The red LED
shows the bit they hold. Press **Set** and it should stay on after you
let go; press **Reset** and it should stay off. The Mega supplies USB
5 V and GND. No Mega signal pin or upload controls this latch.

## Predict

In E19 the LED followed the buttons only while you held them. Here each
button pulls its own input low, and each gate's output also feeds the
other gate. Predict what the LED will show **while Set is held**, **after
Set is released**, and **after Reset is released**. Write your three
guesses in the table under *Try it*.

## Build it

!!! warning "Unplug before moving wires"
    Unplug the Mega's USB cable first. Keep the LED's 1 kΩ resistor in
    series. Check the chip's notch, pin 14 at 5 V and pin 7 at GND
    before plugging USB back in. Press only one button at a time.

Keep E19's chip across columns 16–22, notch to the left; its
100 nF capacitor across the top rails by column 15; both buttons at
columns 2 and 8; and the red LED and 1 kΩ resistor at columns 6–7. Keep
the chip's power wires, the link between the − rails at column 23, the
four wires grounding unused inputs **9, 10, 12, 13**, and the LED path
from chip pin 3. Keep the Mega's GND and 5 V wires in their usual
rail holes.

Remove E19's two 10 kΩ **pull-down** resistors, both 5 V wires
to the buttons, both wires from the buttons' right sides to the chip,
and the two GND wires from chip pins 4 and 5. The generated steps below
show the complete new build. Put each **10 kΩ pull-up** from the top +
rail to its button's left strip. Connect each button's right strip to
the bottom − rail. Connect the Set button's left strip to chip **pin 1**
and the Reset button's left strip to **pin 4**. A released button now
holds its input high; a press pulls that input low.

<!-- bench -->

<!-- steps -->

Complete the two feedback paths: the white wire from chip **pin 6
(Q-bar) → pin 2**, and the yellow one from **pin 3 (Q) → pin 5**. They
end close together, so check each end against the drawing: swapped,
they make no latch. Q at pin 3 also feeds the 1 kΩ resistor and the
LED's long leg, through the other yellow wire; the short leg reaches
GND. Unused outputs **8 and 11** remain open. The
[SN74HC00N datasheet](https://www.ti.com/lit/ds/symlink/sn54hc00.pdf)
shows the 14-pin layout and an active-low set/reset latch example. The
schematic shows the two gates and their feedback (the
[schematic key](../../electricity/schematics.md) names each symbol):

<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 620 440" width="620"
     role="img" aria-labelledby="nand-latch-title nand-latch-desc">
  <title id="nand-latch-title">Set-reset latch schematic</title>
  <desc id="nand-latch-desc">Two NAND gates of the SN74HC00N. Gate 1 takes Set at pin 1 and gate 2's output at pin 2; its output Q, pin 3, lights the red LED through 1 kilohm and also feeds gate 2's pin 5. Gate 2 takes Reset at pin 4; its output, Q-bar at pin 6, feeds back to pin 2. Each button pulls its input to ground, and a 10 kilohm resistor to 5 volts holds it high when released.</desc>
  <g fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round">
    <path d="M300 85.0 H335 A35.0 35.0 0 0 1 335 155.0 H300 Z"/>
    <circle cx="376.0" cy="120" r="6"/>
    <path d="M300 245.0 H335 A35.0 35.0 0 0 1 335 315.0 H300 Z"/>
    <circle cx="376.0" cy="280" r="6"/>
    <path d="M100 32 L100 44.0"/>
    <path d="M100 90.0 L100 102"/>
    <rect x="87" y="44.0" width="26" height="46"/>
    <path d="M100 102 L300 102"/>
    <path d="M100 102 L100 116"/>
    <path d="M100 158 L100 172"/>
    <circle cx="100" cy="120" r="4"/>
    <circle cx="100" cy="154" r="4"/>
    <path d="M86 112 V162 M86 137.0 H72 M72 129.0 V145.0"/>
    <path d="M100 172 V180 M80 180 H120 M87 188 H113 M94 196 H106"/>
    <path d="M180 234 L180 243.0"/>
    <path d="M180 289.0 L180 298"/>
    <rect x="167" y="243.0" width="26" height="46"/>
    <path d="M180 298 L300 298"/>
    <path d="M180 298 L180 312"/>
    <path d="M180 354 L180 368"/>
    <circle cx="180" cy="316" r="4"/>
    <circle cx="180" cy="350" r="4"/>
    <path d="M166 308 V358 M166 333.0 H152 M152 325.0 V341.0"/>
    <path d="M180 368 V376 M160 376 H200 M167 384 H193 M174 392 H186"/>
    <path d="M382 120 L540 120"/>
    <path d="M420 120 L420 190 L260 190 L260 262 L300 262"/>
    <path d="M382 280 L440 280 L440 210 L280 210 L280 138 L300 138"/>
    <path d="M540 120 L540 140.0"/>
    <path d="M540 190.0 L540 210"/>
    <rect x="527" y="140.0" width="26" height="50"/>
    <path d="M540 220 L540.0 237.0"/>
    <path d="M540.0 263.0 L540 280"/>
    <path d="M525.0 237.0 L555.0 237.0 L540.0 263.0 Z"/>
    <path d="M525.0 263.0 L555.0 263.0"/>
    <path d="M520.0 244.0 L504.0 252.0"/>
    <path d="M504.0 252.0 L507.8 246.1"/>
    <path d="M504.0 252.0 L511.0 252.5"/>
    <path d="M520.0 256.0 L504.0 264.0"/>
    <path d="M504.0 264.0 L507.8 258.1"/>
    <path d="M504.0 264.0 L511.0 264.5"/>
    <path d="M540 210 L540 220"/>
    <path d="M540 280 V288 M520 288 H560 M527 296 H553 M534 304 H546"/>
  </g>
  <g fill="currentColor">
    <circle cx="100" cy="102" r="5"/>
    <circle cx="180" cy="298" r="5"/>
    <circle cx="420" cy="120" r="5"/>
  </g>
  <g fill="currentColor" font-size="17" font-family="system-ui, sans-serif">
    <text x="294" y="96" text-anchor="end" font-size="14">1</text>
    <text x="294" y="132" text-anchor="end" font-size="14">2</text>
    <text x="294" y="256" text-anchor="end" font-size="14">5</text>
    <text x="294" y="292" text-anchor="end" font-size="14">4</text>
    <text x="392" y="112" font-size="14">3</text>
    <text x="392" y="272" font-size="14">6</text>
    <text x="310" y="62">SN74HC00N</text>
    <text x="100" y="24" text-anchor="middle">5 V</text>
    <text x="82" y="72" text-anchor="end" font-size="15">10 kΩ</text>
    <text x="116" y="146">Set</text>
    <text x="180" y="226" text-anchor="middle">5 V</text>
    <text x="162" y="270" text-anchor="end" font-size="15">10 kΩ</text>
    <text x="196" y="342">Reset</text>
    <text x="470" y="112">Q</text>
    <text x="452" y="300">Q-bar</text>
    <text x="558" y="170">1 kΩ</text>
    <text x="588" y="260" text-anchor="middle" font-size="15">red</text>
    <text x="588" y="276" text-anchor="middle" font-size="15">LED</text>
    <text x="250" y="400" font-size="15">Pin 14 to 5 V and pin 7 to GND, with 100 nF</text>
    <text x="250" y="420" font-size="15">across them. Unused inputs 9, 10, 12, 13 to GND.</text>
  </g>
</svg>

<!-- connections -->

## Try it

When USB power first arrives, the latch's bit is **unknown**. Either
state may appear because the two gates can settle in different orders.
Plug in USB, press **Reset** once, then release it. This starts with the
LED off. Keep Set and Reset released between trials, and never press
them together.

Then press Set and release it; press Reset and release it. Record what
you see after each action.

| Action | Your prediction | LED you see | Expected LED |
|---|---|---|---|
| Hold Set | ____ | ____ | On |
| Release Set | ____ | ____ | Still on |
| Hold Reset | ____ | ____ | Off |
| Release Reset | ____ | ____ | Still off |

## Why it happens

Each 10 kΩ resistor holds its released input high. A Set press pulls
**pin 1 low**, so the first NAND output **Q goes high** and lights the
LED. Q feeds the second gate, whose output **Q-bar goes low** and feeds
back to the first. That feedback keeps Q high after Set is released.
Reset pulls **pin 4 low**, making Q-bar high and Q low; the feedback then
holds the LED off. This is one bit of state made by wires and gates,
similar to the remembered state in [Lesson 6's Simon game](../006-simon/index.md).

## Check your result

Compare the four rows you saw with your predictions. In one sentence,
explain what keeps the LED on after you let go of Set.

## If it doesn't work

Unplug USB before checking a connection.

| What you see | Check |
|---|---|
| The LED never lights | Check pin 14 to 5 V, pin 7 to GND, Q at pin 3 through 1 kΩ to the LED's long leg, and its short leg to GND. |
| Set or Reset works only while held | Check both feedback jumpers: white from pin 6 to pin 2, and yellow from pin 3 to pin 5. |
| A button does the opposite job | Set's left strip goes to pin 1; Reset's left strip goes to pin 4. Each right strip goes to GND. |
| The LED changes without a press | Check both 10 kΩ pull-ups from 5 V to the left button strips, all four unused inputs at GND, and the 100 nF capacitor near the chip. |

## About the sketch

The matching ADK sketch claims no I/O pins. The gates hold the bit
without code; no upload is needed:

<!-- sketch -->

Unplug USB when finished. These LED states are calculated from the
datasheet's gate behavior; this lesson has not been recorded as tried
on physical hardware.
