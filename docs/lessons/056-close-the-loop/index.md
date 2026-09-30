---
lesson: 56
promise: Break and restore an LED's return path to see why current needs a complete loop.
time: 15 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - Red LED
  - 220 Ω resistor (red, red, black, black, brown)
  - 3 jumper wires
ideas:
  - Current needs a complete return path
---

## What you'll build

<!-- closeup -->

A red LED blinks while its path back to the Mega is complete. You will open
that path, see the LED go dark, then close it and see the blink return.

## Predict

The sketch turns pin 26 on and off. If you remove the wire from the Mega's
**GND** to the breadboard, will the LED still blink? Write down your guess.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before building or changing any wires. Check the
    circuit before plugging it back in. Keep the 220 Ω resistor in series
    with the LED throughout the experiment.

This is [Lesson 1's LED circuit](../001-blink/index.md). Pin 26 supplies the
LED when it turns on. Only the bottom **−** rail is grounded; both **+**
rails are unused.

<!-- bench -->

<!-- steps -->

The finished circuit makes these connections:

<!-- connections -->

## Code it

Open **File → Examples → Adk → lessons → 056-close-the-loop** in the Arduino
IDE. The sketch switches the LED on for half a second, then off for half a
second.

<!-- sketch -->

## Upload and observe

1. Plug the Mega into your computer.
2. Choose **Tools → Board → ADK Boards → ADK Mega 2560** and the Mega's
   **Tools → Port** entry. [Getting started](../../start.md) shows how to add
   the board if it is missing.
3. Press **Upload**. Watch the red LED on the breadboard: it should blink.

Now test your prediction:

1. **Unplug** the USB cable. Remove only the black wire that joins the Mega's
   GND to the bottom − rail. Leave the LED and its resistor in place.
2. Plug the USB cable back in. The red LED should stay dark.
3. **Unplug** again. Put that same black wire back in its original holes,
   then plug in once more. The red LED should blink again.

Current needs a complete loop: from pin 26, through the resistor and LED,
then back to the Mega's GND. Removing the return wire opens that loop.

## If it doesn't work

| What you see | Check |
|---|---|
| The red LED never blinks | Check the upload, pin 26, the LED's direction and its 220 Ω resistor. Check that the bottom − rail reaches the Mega's GND. |
| The red LED blinks with the GND wire removed | Unplug. Check for another wire joining the LED's return path to GND. The + rails should be empty. |
| The red LED stays dark after you restore the wire | Unplug. Seat the wire in the same GND pin and bottom − rail hole it used before. |
| Upload fails | Check the board and port in the **Tools** menu. Try a USB data cable. |

Leave the GND wire in place and the LED blinking for the next investigation.
