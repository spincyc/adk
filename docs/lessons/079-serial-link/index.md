---
lesson: 79
promise: Send one byte through a wire and see whether it comes back.
time: 25 minutes
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 1 kΩ resistor (brown, black, black, brown, brown)
  - 10 kΩ resistor (brown, black, black, red, brown)
  - 5 jumper wires
ideas:
  - A serial receiver finds bits by their timing and needs a complete signal path
---

## What you'll build

<!-- closeup -->

The Mega will send the letter **A** from its **Serial1 TX1** pin and read it
back on **Serial1 RX1**. A 1 kΩ resistor completes the path between those
pins. The USB Serial Monitor shows what was sent and received. You can
remove the link to see what happens when the path is open.

## Predict

Before wiring, predict what the Serial Monitor will say after the Mega
sends `A`. Will RX1 receive `A` if the resistor between TX1 and RX1 is
removed? Write down both predictions.

## Build it

!!! warning "Unplug before wiring"
    Take out the Mega's USB cable before moving any parts or wires. Keep
    the **1 kΩ resistor** between TX1 and RX1; do not replace it with a
    plain wire. Check that the 10 kΩ resistor reaches the **5 V** rail,
    not GND, before plugging USB back in.

Start with an empty breadboard and USB unplugged. The complete steps
below place the Mega's GND and 5 V wires in their usual rail holes,
then build this new signal path. It needs no LED or knob.

<!-- bench -->

<!-- steps -->

Pin **18 (TX1)** goes to the upper half of column 6. Put the **1 kΩ**
resistor across the center gap, from **g6 to e6**. Pin **19 (RX1)** goes
to the lower half of column 6. Put the **10 kΩ** resistor from the top +
rail beside column 7 to **j7**, then add a jumper from **h7 to d6**.
The 10 kΩ resistor holds RX1 high when the link is open. The 1 kΩ
resistor limits current if a pin is accidentally set as an output
against TX1.

<!-- connections -->

Both pins are on the **same Mega**, so they already share GND inside it.
The breadboard's GND rail keeps its usual connection, but this signal
path does not need another GND jumper.

## Code it

Open **File → Examples → Adk → lessons → 079-serial-link** in the
Arduino IDE:

<!-- sketch -->

Every two seconds the sketch sends `A` on Serial1 and tells the USB
Serial Monitor what came back. Serial1 uses pins 18 and 19; `Serial`
uses the USB cable for the monitor. Both are set to **9600 baud**. A few
pieces are new:

- **`struct SerialCable : adk::Object`** makes a part of your own. Every
  ADK part, from an LED to a knob, is an `adk::Object`, and
  `adk::setup ()` calls each one's `setup ()`. **`override`** says that
  this `setup ()` takes the place of the empty one every `adk::Object`
  has.
- **`adk::claimSerial (Serial1)`** claims pins 18 and 19 for Serial1, just
  as an LED claims its pin, and is false if another part already has
  them. Only then does **`Serial1.begin (9600)`** start Serial1, as
  `Serial.begin (9600)` starts the USB link.
- **`Serial1.write ('A')`** sends one byte. **`Serial1.available ()`**
  counts the bytes that have arrived and wait to be read, and
  **`Serial1.read ()`** takes the oldest of them, as a number.
- **`static_cast<char> (...)`** turns that number back into a character,
  so the monitor shows `A` rather than 65, the number that stands for it.

## Try it

1. Check the resistor and wires, then plug the Mega into USB. Choose
   **Tools → Board → ADK Boards → ADK Mega 2560** and the board's
   **Tools → Port**. Upload the sketch.
2. Open the **Serial Monitor** at **9600 baud**. Compare the lines with
   your prediction. About every two seconds, expect:

   ```text
   Sent: A
   Received: A
   ```

3. **Predict again:** If you lift the 1 kΩ resistor out, what will the
   monitor print? Unplug USB, take out only that resistor, and plug USB
   back in. The pull-up stays in place. After each `Sent: A`, expect
   `No byte returned` before the next send. RX1 stays high, so it sees
   no start of a new byte.
4. Unplug USB, put the 1 kΩ resistor back in **g6 and e6**, and reconnect.
   Look for `Received: A` again.

## Why it happens

A **bit** is a 0 or 1. Serial1 sends bits one after another on TX1. At
**9600 baud** in this two-level signal, each bit lasts about 1/9600 of a
second, about 104 µs. A byte travels in a **frame**: a low start bit,
eight data bits, then a high stop bit. RX1 uses that agreed timing to
read the bits back into a byte: from the start bit's edge, it looks at
the wire once every 104 µs. The wire does not carry the letter `A` all at
once.

The TX1 signal reaches RX1 through the 1 kΩ resistor. The 10 kΩ pull-up
holds RX1 at the idle high level whenever TX1 is not pulling it low.
Removing the 1 kΩ resistor breaks the TX1-to-RX1 path; the pull-up then
keeps RX1 high and no frame arrives. This is a **loopback** on one UART:
it checks the path, but Serial1 sends and receives with one clock, so it
cannot disagree with itself about timing. It is a small wired cousin of
the messages in [Lesson 38](../038-radio-messages/index.md).

## Change one thing

Your computer is a second receiver, with a clock of its own. The Mega's
USB chip reads the bits the Mega sends on pin 1, its TX for `Serial`, at
the speed set in the Serial Monitor's baud menu, and passes the bytes it
builds to the monitor.

Predict first: if the Serial Monitor listens at **4800 baud** while the
sketch still sends at 9600, what will it show?

1. With the sketch running and the link in place, change the Serial
   Monitor's baud menu from 9600 to **4800**. Watch a few sends.
2. Change it to **19200** and watch again.
3. Set it back to **9600**: the lines should be readable again. Unplug
   when finished.

At the wrong speed, expect a jumble of odd characters, or nothing
readable, in place of `Sent: A`. The Mega may restart as the monitor
changes speed; that does no harm. The wire and the bits on it have not
changed: only the receiver's timing has. Looking at the wire at the
wrong moments, it reads the wrong bits, so it builds the wrong bytes.

## Check your result

Did each of the three trials, link in place, link out and monitor at the
wrong speed, match your prediction? In one sentence, explain why the
monitor can read the Mega's bytes only when both use the same baud rate.

## If it doesn't work

Unplug USB before checking the build:

| What you see | Check |
|---|---|
| `No byte returned` with the link in place | Check 18 → j6, 1 kΩ across g6–e6, 19 → c6. |
| Unexpected letters appear | Set the monitor to 9600 baud; check 10 kΩ at j7 and the h7 → d6 jumper. |
| No monitor text appears | Check the upload and select the Mega's USB port. |

The expected lines describe what this circuit should do. This lesson has
not been recorded as tried on physical hardware.
