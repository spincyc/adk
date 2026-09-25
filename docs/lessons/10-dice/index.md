---
lesson: 10
title: Dice
arc: Digits
promise: Build an electronic die that spins, slows and lands on a number.
time: 60 minutes
level: 2
sketch: Lesson10Dice
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 74HC595 shift register chip
  - One-digit seven-segment display (5161AS)
  - 7 × 1 kΩ resistors (brown, black, black, brown, brown)
  - Push button
  - 17 jumper wires
ideas:
  - The 74HC595, eight outputs from three pins
  - Bits and bytes, written in binary
  - Seven-segment patterns, one byte per picture
  - Animation by shifting a bit along
---

## What you'll build

<!-- closeup -->

A die that never rolls off the table. Press the button and a single glowing
bar races round the edge of the digit, slowing down like a die tumbling to a
stop, then lands on a number from 1 to 6. Nobody, not even you, knows which
number is coming.

## The idea

A seven-segment digit is seven bar-shaped LEDs, called **a** to **g**, plus a
dot. Light the right bars and you draw any number:

```text
   aaa
  f   b
  f   b
   ggg
  e   c
  e   c
   ddd   dp
```

That needs eight wires, one per LED, and four digits would need thirty-two.
The Mega would soon run out of pins. The **74HC595 shift register** is a chip
that gives you eight outputs from just three pins: **data**, **clock** and
**latch**. The Mega puts one bit on the data pin and pulses the clock; inside
the chip, every bit moves along one place, like people shuffling up a queue.
After eight pulses, a pulse on the latch copies all eight to the chip's
outputs, **Q0** to **Q7**, at the same moment.

A **bit** is a single 0 or 1: off or on. Eight bits make a **byte**. In C++
you can write a byte in binary, starting with `0b`: the right-hand digit is
bit 0 and the left-hand one bit 7. Here bit 0 drives Q0, which lights segment
a, bit 1 lights b, and so on up to bit 6, segment g. To draw a 4 you need b,
c, f and g, which are bits 1, 2, 5 and 6:

| Bit | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|---|---|---|---|---|---|---|---|---|
| Segment | dot | g | f | e | d | c | b | a |
| A 4 | 0 | 1 | 1 | 0 | 0 | 1 | 1 | 0 |

Read along the bottom row and you have the byte: `0b01100110`.

Each segment has its own 1 kΩ resistor. A red segment keeps about 2 V for
itself, so it takes about (5 V − 2 V) ÷ 1 kΩ = 3 mA. All seven together take
about 21 mA through the chip, comfortably under the 70 mA it can handle. With
220 Ω resistors it would be nearly 100 mA: too much.

!!! question "Predict"
    The sketch spins the lit bar with `1 << step`, which means a 1 moved
    `step` places to the left: `1 << 0` is `0b00000001`, segment a, at the
    top. Which segment does `1 << 3` light? As `step` counts 0, 1, 2, 3, 4,
    5, which way round does the bar travel?

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you start: this build has a lot of wires.
    The 74HC595 goes in with its **notch to the left**; the wrong way round,
    it can get hot. Press it in gently and evenly so no leg folds under. The
    digit goes in with its **decimal point at the bottom right**. If anything
    gets warm, unplug at once and check the chip's notch and its 5 V and GND
    wires.

<!-- bench -->

<!-- steps -->

??? info "Finding your way round the chip and the digit"
    Chips are numbered from pin 1, beside the notch, counting round the chip
    the opposite way to a clock's hands. On the 74HC595, pins 16 (VCC) and
    10 (MR) go to 5 V; pins 8 (GND) and 13 (OE) go to GND; pins 14, 11 and
    12 take data, clock and latch from the Mega; and the outputs are Q0 on
    pin 15 and Q1 to Q7 on pins 1 to 7. The dice doesn't use Q7.

    The digit is numbered the same way: pin 1 is at the bottom left, under
    the e bar. Its two common pins, 3 and 8, are joined inside, so one wire
    from pin 3 to GND serves both. The decimal point, pin 5, is left free.

    Some resistor legs pass over holes that other parts use, on their way to
    the digit's top row. They only touch the holes they go into: follow the
    steps, which give every hole.

    Datasheets suggest a small 100 nF capacitor, marked **104**, across the
    chip's supply to smooth it. If your digit ever shows stray segments, put
    one from the top + rail to the top − rail right beside the chip.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → Lesson10Dice**:

<!-- sketch -->

What's new:

- `adk::ShiftRegister digit {37, 38, 39};` is the 74HC595, with its data,
  clock and latch pins in that order. `digit.write (byte)` sends the byte
  and latches it, so all eight outputs change at once.
- `faces` is a table of six bytes, one per face of the die, written in
  binary so you can see which segments each one lights.
- `randomSeed (analogRead (A7));` is Lesson 3's trick: nothing is wired to
  A7, so its reading drifts, and the dice start differently every time.
  `random (6)` gives 0 to 5, which picks `faces[0]` (the 1) to `faces[5]`.
- In `spin ()`, `step % 6` is the remainder after dividing by 6, so it
  counts 0 to 5 and starts again, three times round. `1 << (step % 6)` lights
  a, b, c, d, e, f in turn, and the wait grows from 20 to 156 milliseconds,
  so the bar slows down.

## Upload it

Upload the sketch. The digit shows a dash: just segment g, waiting. Press
the button. A single bar runs round the rim three times, clockwise, slowing
as it goes, and the digit lands on a number from 1 to 6. Press again for
another roll.

## If it doesn't work

| What you see | Try this |
|---|---|
| Nothing lights at all | Check the chip's notch is on the left, the red wires from j6 and j12 reach the top + rail, and the black wires from j9 and a13 reach the top and bottom − rails. |
| The chip gets warm | Unplug now. The chip is in backwards, or a 5 V wire is on a GND pin. |
| The numbers look scrambled | A resistor is in the wrong column, so a Q output lights the wrong segment. Check each one against the connections list. |
| One segment never lights | Its resistor or wire is loose, or one column out. |
| The dash shows, but the button does nothing | The wire from pin 22 goes in a1, the black wire from a3 to the − rail, and the button straddles the gap. |
| Segments flicker or light at random | Data, clock and latch are swapped: pin 37 to j8, 38 to j11, 39 to j10. |
| The first roll is the same every time | Nothing may be plugged into A7: its drifting reading is what shuffles the dice. |

??? note "How it works"
    `digit.write ()` pulls the latch pin low, then calls Arduino's
    `shiftOut ()`, which sets the data pin to each bit in turn, bit 7 first,
    and pulses the clock after each. When all eight are in, it raises the
    latch, and the chip copies them to its outputs together. Until the next
    latch the outputs stay put on their own, so the Mega is free to do
    anything else.

    Before `adk::setup ()` runs, the chip's outputs hold whatever it woke up
    with, which is why a digit can flash a random pattern at power-up.
    `adk::setup ()` sends zeros first thing.

## Make it yours

1. **Light the dot.** Add an eighth 1 kΩ resistor from Q7 (column 12) to
   the digit's decimal point (pin 5, column 31), then light bit 7 while the
   die spins: `digit.write ((1 << (step % 6)) | 0b10000000);`.
2. **Tumble.** Instead of a spinning bar, flash random faces that slow down,
   the way a real die bounces before it settles.
3. **Loaded die.** Make 6 come up twice as often as the other numbers. Is it
   still fair to call it a die?
4. **Let the library draw.** Swap `adk::ShiftRegister` for
   `adk::SevenSegment`, which is wired the same way and already knows the
   patterns: `digit.show (random (1, 7));`. What can't it do that your own
   bytes could?
