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
  - 4 jumper wires
ideas:
  - A diode conducts mainly in one direction
---

## What you'll build

<!-- closeup -->

The Mega's USB-powered 5 V supply feeds one path through a resistor, a
1N4007 diode and a red LED to GND. You will turn only the 1N4007 around:
the LED should light in one direction and stay dark in the other.

## Predict

The 1N4007 has a pale **band** at one end. First its band will face the LED.
Will the LED light? What will happen when you turn only the diode around,
leaving every other part in place? Write down both guesses.

## Build it

!!! warning "Unplug before changing anything"
    Unplug the USB cable before placing or turning parts. Keep the **1 kΩ
    resistor** in the path for both trials. Never put the LED directly
    across the 5 V supply.

Remove any previous build while its power is disconnected. This circuit
uses only the Mega's USB 5 V: its 5 V wire goes to the top **+** rail near
the Mega, and its GND wire goes to the bottom **−** rail hole nearest the
Mega. Keep those wires if they are already in the shown holes. There is no
AA battery pack in this build.

Follow the generated steps. Current's route starts at the + rail, crosses
the resistor, then reaches the diode's **unbanded** end in d4. The diode's
**banded end**, called its **cathode**, goes in d9 toward the LED. The red
LED's long leg goes in b9; its short leg goes in b10, toward GND. Check
that the resistor is between + and *both* diodes before powering the board.

<!-- bench -->

<!-- steps -->

The finished forward-facing circuit makes these connections:

<!-- connections -->

## Try both directions

1. Check the path, then plug the Mega into USB. With the band facing the
   LED, the red LED should light. Record what you see.
2. **Unplug USB.** Lift only the 1N4007 from d4 and d9. Turn it end for
   end, and put its legs back in those same two holes. Leave the resistor,
   LED and wires in place. Plug USB back in. The LED should stay dark.
3. **Unplug USB again.** Turn only the 1N4007 back, with its unbanded end
   in d4 and its banded end in d9. Plug in once more. Leave the LED lit.

Compare each observation with your prediction. The 1N4007 lets current
pass mainly from its unbanded end to its banded end. In the first and last
builds, current can go from + through the resistor, 1N4007 and LED to GND.
Turning the 1N4007 around blocks that path, so the LED goes dark even
though the wires stay connected. The red LED also has a direction; its
short leg stays toward GND throughout this test.

## About the sketch

The LED gets power from the Mega's **5 V power pin**, not a programmable
signal pin. No upload is needed. The matching ADK example claims no I/O
pins:

<!-- sketch -->

## Check your result

Complete this sentence: “With the 1N4007's band facing the LED, the LED
is ______; with the band facing the resistor, the LED is ______.” The
expected words are **lit** and **dark**. The 1N4007 is also used as a
protective diode beside [Lesson 3's buzzer](../003-reaction-duel/index.md).

## If it doesn't work

| What you see | Check with USB unplugged |
|---|---|
| Dark in both directions | Check the 5 V and GND rail wires, the jumper from + to j4, the resistor in g4 and e4, and the return jumper from a10 to the − rail. Check that the LED's short leg is in b10. |
| Lit in both directions | Make sure you turned the 1N4007, not the LED, and that its legs are in d4 and d9. Look for a wire that bypasses the diode. |
| Dark after restoring the diode | Put its unbanded end in d4 and its banded end in d9. Check that both legs are seated. |

These are expected observations from the circuit design; this lesson has
not been recorded as tried on hardware.
