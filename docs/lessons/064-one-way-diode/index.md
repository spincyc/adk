---
lesson: 64
promise: Turn one diode around and see it stop current through a red LED.
time: 15 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 1N4007 diode
  - Red LED
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 3 jumper wires
ideas:
  - A diode conducts mainly in one direction
laws:
  - {law: forward-voltage, section: why-it-happens, for: "Explains the reversed diode blocking current"}
---

## What you'll build

<!-- closeup -->

The Mega's USB-powered 5 V supply feeds one path through a 1N4007 diode,
a resistor and a red LED to GND. You will turn only the 1N4007 around:
the LED should light in one direction and stay dark in the other.

## Predict

The 1N4007 has a pale **band** at one end. First its band will point away
from the + rail, toward the resistor and LED. Will the LED light? What will
happen when you turn only the diode around, leaving every other part in
place? Write down both guesses.

## Build it

!!! warning "Unplug before changing anything"
    Unplug the USB cable before placing or turning parts. Keep the **1 kΩ
    resistor** in the path for both trials. Never put the LED directly
    across the 5 V supply.

Remove any previous build while its power is disconnected. This circuit
uses only the Mega's USB 5 V: its 5 V wire goes to the top **+** rail near
the Mega, and its GND wire goes to the bottom **−** rail hole nearest the
Mega. Keep those wires if they are already in the shown holes.

Follow the generated steps. The 1N4007 stands where E01's red jumper was:
its **unbanded** end in the top + rail by column 6 and its **banded end**,
called its **cathode**, in **j6**. From there current's route crosses the
1 kΩ resistor in g6–e6 and reaches the red LED at its usual home: long leg
in b6, short leg in b7, toward GND. Check that the resistor is in the same
single path as the diode and the LED before powering the board.

<!-- bench -->

<!-- steps -->

In schematic form (the
[schematic key](../../electricity/schematics.md) names each symbol):

<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 560 220" width="560"
     role="img" aria-labelledby="one-way-title one-way-desc">
  <title id="one-way-title">Diode, resistor and LED in one path schematic</title>
  <desc id="one-way-desc">Five volts passes through the 1N4007 diode, from its unbanded end to its banded end in j6, then through a 1 kilohm resistor and the red LED to ground.</desc>
  <g fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round">
    <path d="M60 60 L132.0 60"/>
    <path d="M132.0 75.0 L132.0 45.0 L158.0 60 Z"/>
    <path d="M158.0 75.0 L158.0 45.0"/>
    <path d="M158.0 60 L240.0 60"/>
    <path d="M310.0 60 L410 60"/>
    <rect x="240.0" y="47" width="70" height="26"/>
    <path d="M410 60 L410 97.0"/>
    <path d="M395.0 97.0 L425.0 97.0 L410.0 123.0 Z"/>
    <path d="M395.0 123.0 L425.0 123.0"/>
    <path d="M390.0 104.0 L374.0 112.0"/>
    <path d="M374.0 112.0 L377.8 106.1"/>
    <path d="M374.0 112.0 L381.0 112.5"/>
    <path d="M390.0 116.0 L374.0 124.0"/>
    <path d="M374.0 124.0 L377.8 118.1"/>
    <path d="M374.0 124.0 L381.0 124.5"/>
    <path d="M410 123.0 L410 160"/>
    <path d="M410 160 V168 M390 168 H430 M397 176 H423 M404 184 H416"/>
  </g>
  <g fill="currentColor" font-size="17" font-family="system-ui, sans-serif">
    <text x="14" y="66">5 V</text>
    <text x="145" y="32" text-anchor="middle">1N4007</text>
    <text x="158" y="100" text-anchor="middle" font-size="15">band · j6</text>
    <text x="275" y="36" text-anchor="middle">1 kΩ</text>
    <text x="434" y="116">red LED</text>
    <text x="438" y="182">GND</text>
  </g>
</svg>

The finished forward-facing circuit makes these connections:

<!-- connections -->

## Try it

1. Check the path, then plug the Mega into USB. With the band in j6,
   toward the LED, the red LED should light. Record what you see.
2. **Unplug USB.** Lift only the 1N4007 from the + rail and j6. Turn it
   end for end, and put its legs back in those same two holes, band now
   in the + rail. Leave the resistor, LED and wires in place. Plug USB
   back in. The LED should stay dark.
3. **Unplug USB again.** Turn only the 1N4007 back, with its unbanded end
   in the + rail and its banded end in j6. Plug in once more. Leave the
   LED lit.

## Why it happens

The 1N4007 lets current pass mainly from its unbanded end to its banded
end. In the first and last
builds, current can go from + through the 1N4007, resistor and LED to GND.
Turning the 1N4007 around blocks that path, so the LED goes dark even
though the wires stay connected. The red LED also has a direction; its
short leg stays toward GND throughout this test.

## Check your result

Compare each observation with your prediction, then complete this
sentence: “With the 1N4007's band toward the resistor and LED, the LED is
\_\_\_\_\_\_; with the band in the + rail, the LED is \_\_\_\_\_\_.” The
expected words are **lit** and **dark**. The 1N4007 is also used as a
protective diode beside [Lesson 3's buzzer](../003-reaction-duel/index.md).

## If it doesn't work

| What you see | Check with USB unplugged |
|---|---|
| Dark in both directions | Check the 5 V and GND rail wires, the diode from the + rail to j6, the resistor in g6 and e6, and the return jumper from a7 to the − rail. Check that the LED's short leg is in b7. |
| Lit in both directions | Make sure you turned the 1N4007, not the LED, and that its legs are in the + rail by column 6 and j6. Look for a wire that bypasses the diode. |
| Dark after restoring the diode | Put its unbanded end in the + rail and its banded end in j6. Check that both legs are seated. |

## About the sketch

The LED gets power from the Mega's **5 V power pin**, not a programmable
signal pin. No upload is needed. The matching ADK example claims no I/O
pins:

<!-- sketch -->

These are expected observations from the circuit design; this lesson has
not been recorded as tried on hardware.
