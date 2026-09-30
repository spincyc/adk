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

The sketch claims TX1 and RX1 as one Serial1 connection. Every two
seconds it sends `A` and tells the USB Serial Monitor what came back.
Serial1 uses pins 18 and 19; `Serial` uses the USB cable for the monitor.
Both are set to **9600 baud**.

## Try the complete path

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
   Look for `Received: A` again. Unplug when finished.

## Why it happens

A **bit** is a 0 or 1. Serial1 sends bits one after another on TX1. At
**9600 baud** in this two-level signal, each bit lasts about 1/9600 of a
second. A byte travels in a **frame**: a low start bit, eight data bits,
then a high stop bit. RX1 uses that agreed timing to read the bits back
into a byte. The wire does not carry the letter `A` all at once.

The TX1 signal reaches RX1 through the 1 kΩ resistor. The 10 kΩ pull-up
holds RX1 at the idle high level whenever TX1 is not pulling it low.
Removing the 1 kΩ resistor breaks the TX1-to-RX1 path; the pull-up then
keeps RX1 high and no frame arrives. This is a **loopback** on one UART:
it checks the path, but it cannot test whether two separate devices have
their baud rates set alike. It is a small wired cousin of the messages in
[Lesson 38](../038-radio-messages/index.md).

## If the byte does not return

Unplug USB before checking the build:

| What you see | Check |
|---|---|
| `No byte returned` with the link in place | Check 18 → j6, 1 kΩ across g6–e6, 19 → c6. |
| Unexpected letters appear | Set the monitor to 9600 baud; check 10 kΩ at j7 and the h7 → d6 jumper. |
| No monitor text appears | Check the upload and select the Mega's USB port. |

The expected lines describe what this circuit should do. This lesson has
not been recorded as tried on physical hardware.
