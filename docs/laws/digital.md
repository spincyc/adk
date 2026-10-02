# Logic levels and serial

A digital pin doesn't measure a voltage; it decides whether the voltage
is HIGH or LOW. These are the rules for that decision, the resistors that
keep it from being a guess, and the timing that lets a string of HIGHs
and LOWs carry a message. If you are working through the electricity
course, do E19 to E21 and E24 first: this page gives away what they find.

## Logic levels {#logic-levels}

<!-- law logic-levels -->

The Mega's chip, on 5 V, reads anything below 0.3 × 5 V = 1.5 V as LOW
and anything above 0.6 × 5 V = 3 V as HIGH, by its datasheet. Between the
two it may read either, so a good signal never lingers there. The 74HC
logic chips of E19 to E21 read below about 1.5 V as LOW and above about
3.5 V as HIGH. An output
does the reverse: a Mega pin set HIGH is close to 5 V and LOW close to
0 V, as long as it gives only a little current.

| Signal | Voltage | Reads as |
|---|---|---|
| A pin wired to GND, or another pin set LOW | close to 0 V | LOW |
| Between the thresholds | 1.5 V to 3 V | either; avoid it |
| A 3.3 V module's HIGH | about 3.3 V | HIGH, though only just |
| A pin wired to 5 V, or another pin set HIGH | close to 5 V | HIGH |

### 3.3 V parts beside a 5 V Mega {#three-volts}

Many modules, the radios and the RFID reader among them, run on 3.3 V.
Their HIGH, about 3.3 V, is above the Mega's 3 V, so the Mega hears it.
The other way is the danger: a Mega pin's 5 V HIGH is more than a 3.3 V
input can take. So the course drops most Mega outputs with a
[divider](dividers.md#divider), 1 kΩ and 2 kΩ, to about 3.3 V (Lessons 34,
38 and 40 on). Where a wire carries signals both ways, the Mega only ever
pulls it down and lets a pull-up to 3.3 V lift it (Lesson 37), or, for
I2C, it uses a level shifter (Lesson 28).

### A shared ground {#ground}

A voltage is always between two points, so a HIGH from one part means
something to another only if both measure from the same GND. That is why
every servo, motor driver and module wired to the Mega, Lesson 42's
Heltec board among them, has its GND joined to the Mega's. Two boards
that talk only by radio need no shared GND.

## Pull-up and pull-down resistors {#pull}

<!-- law pull -->

An input pin with nothing connected **floats**: it picks up stray charge
and reads HIGH or LOW at random. A resistor to 5 V (a **pull-up**) or to
GND (a **pull-down**) holds it at a known level. A button wired straight
to the other side then wins whenever it is pressed, because a wire is a
far smaller resistance than the resistor.

The resistor is large so that little current flows while the button is
pressed. The Mega's own pull-up, which Lesson 2 switches on in the chip,
is between 20 kΩ and 50 kΩ: by [Ohm's law](ohms-law.md), at most
5 V ÷ 20 kΩ = 0.25 mA.

- **Buttons and switches** read HIGH until pressed, from Lesson 2 on; a
  keypad's columns too (Lesson 16).
- **I2C's two wires** are pulled up and only ever pulled down by the
  parts on them (Lessons 28 and 37).
- **A transistor's base** is pulled down by 10 kΩ, so the transistor stays
  off while the Mega starts and its pins float (Lesson 3, E10).
- **Logic chips' inputs** need one too: E19's buttons have pull-downs,
  E20's latch inputs pull-ups, and E24's receiver a pull-up that holds it
  at the idle HIGH.

## Schmitt triggers and hysteresis {#schmitt}

<!-- law schmitt -->

An input with one threshold, fed a voltage that creeps slowly past it or
carries a little noise, flickers between HIGH and LOW while the voltage
is near the threshold. Two thresholds fix that: the input is recognized
as HIGH only once it rises past the upper one, and as LOW only once it
falls past the lower one. Between them it keeps its previous recognized
state. The gap between them is called **hysteresis**.

A **Schmitt trigger** input does this in hardware. The 74HC14 is also an
**inverter**: a rising input crossing the upper threshold makes its output
LOW; a falling input crossing the lower threshold makes its output HIGH.
That is the input/output relationship in
[TI's function table](https://www.ti.com/lit/ds/symlink/sn74hc14.pdf#page=10).
On 5 V the thresholds are roughly 2.7 V rising and 1.7 V falling;
they vary between chips. E21 has you watch them on a meter,
and makes a clock from them: its output charges a capacitor through a
resistor until the input reaches 2.7 V, flips LOW, lets the capacitor
fall to 1.7 V, flips back, and so on. Each half takes part of a
[time constant](capacitors-and-coils.md#rc-time), about 0.8 × R × C for
a whole blink.

Sketches use the same idea in software: Lesson 15's comfort light and
Lesson 26's joystick each have two thresholds with a gap, so a reading
near one can't make them flicker.

## Serial timing {#serial}

<!-- law serial -->

A serial link sends a byte one bit at a time, each bit lasting a fixed
time set by the **baud rate**, in bits per second. At 9600 baud each bit
lasts 1 ÷ 9600 s, about 104 µs. A byte travels with a start bit and a
stop bit, ten bits in all, so about 1 ms; a line of 20 letters takes
about 20 ms.

Both ends must agree on the rate: the receiver samples each bit at the
time it expects it, and at the wrong rate it reads a jumble, as E24
shows. Between bytes the line rests HIGH, which is why a meter on a busy
TX pin reads close to 5 V. Some of the rates in the course:

| Link | Rate | One byte |
|---|---|---|
| The Mega to the Serial Monitor | 9600 baud | about 1 ms |
| The Mega to a LoRa modem | 115 200 baud | about 87 µs |
| The 433 MHz radio (Lesson 38) | 2000 bits a second | 6 ms a letter: twelve bits of 0.5 ms |
| LoRa over the air (Lesson 40) | about 1000 bits a second | about 8 ms a letter |

## Radio airtime {#airtime}

<!-- law airtime -->

Radios that share a band must share the time on it. In Europe the rules
allow each radio to send for at most a tenth of the time on the bands
Lessons 38, 41 and the two-board lessons use, and only a hundredth on the
868.1 MHz that Lesson 40 uses there (see [Radios](../safety.md#radios)). A message's **time on the air** grows with its
length. Lesson 55 works it out from the radio chip's datasheet for the
LoRa modems' quick setting: about 20 ms plus 1.5 ms a letter.

**Worked:** a 30-letter message takes about 20 + 30 × 1.5 = 65 ms on the
air. Sent once a second, that is 65 ms in every 1000 ms, 6.5 %: inside a
tenth. Sent twice a second it is 13 %, too much. Shorter messages, or
sending only when something changes, keep a sketch inside the limit.

## Check yourself

1. A sensor module's output reads 2.2 V when it means HIGH. Will the Mega
   read it reliably?
2. A button joins its pin to GND when pressed, and nothing else is wired
   to the pin. What does the pin read once the button is released?
3. How long does one byte take at 115 200 baud?
4. E21's clock uses 100 kΩ and 10 µF. About how long is one blink?
5. A sketch sends a 20-letter message every half second. Is it inside a
   tenth of the time?

??? note "Answers"
    1. No: 2.2 V is between 1.5 V and 3 V, where the Mega may read either.
    2. Anything: the pin floats, and may read HIGH or LOW at random. A
       pull-up, such as the Mega's own, holds it HIGH until the button
       pulls it LOW.
    3. Ten bits at 1 ÷ 115 200 s each: about 87 µs.
    4. R × C = 100 kΩ × 10 µF = 1 s, so about 0.8 s.
    5. 20 + 20 × 1.5 = 50 ms in every 500 ms is exactly a tenth: right at
       the limit, with nothing to spare.

## Lessons that rely on them

<!-- relied on -->
