---
lesson: 10
promise: Build an electronic die that spins, slows and lands on a number.
time: 1 hour
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 74HC595 shift register chip
  - One-digit seven-segment display (5161AS)
  - 7 × 1 kΩ resistors (brown, black, black, brown, brown)
  - Push button
  - 27 jumper wires
ideas:
  - The 74HC595, eight outputs from three pins
  - Bits and bytes, written in binary
  - Seven-segment patterns, one byte per picture
  - Animation by shifting a bit along
  - Splitting a number with / and %
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

    Each output has its own resistor. Q0's lies in row i beside the chip;
    the three whose segments sit on the digit's top row (f, b and g) stand
    across the middle gap in columns 27, 28 and 30, so each signal crosses
    over through its resistor; and the three for the bottom row (e, d and c)
    stand across the gap in columns 39 to 41, just before the digit, fed from
    above by wires that step up over the gap. The black jumper in column 6
    joins the two − rails, so OE can take GND from the top one. Follow the
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
- `faces` is an `adk::Array` of six bytes, one per face of the die: a
  `uint8_t`, from Lesson 4, is exactly one byte. They are written in binary
  so you can see which segments each one lights.
- `randomSeed (analogRead (A7));` is Lesson 3's trick: nothing is wired to
  A7, so its reading drifts, and the dice start differently every time.
  `random (6)` gives 0 to 5, which picks `faces[0]` (the 1) to `faces[5]`.
- In `spin ()`, `1 << (step % 6)` is a 1 moved `step % 6` places to the
  left, so it lights a, b, c, d, e, f in turn. The wait grows from 20 to
  156 milliseconds, so the bar slows down.
- `step % 6` is the remainder after dividing by 6, as in Lesson 4, so it
  counts 0 to 5 and starts again, three times round. Its partner is `/`,
  which divides and throws the remainder away, as the octave knob did in
  Lesson 9: `step / 6` would count the laps, 0, 1 and 2. Together they
  **split a number**. Step 14 is 14 / 6 = 2 laps and 14 % 6 = 2 segments
  more, segment c. With 10 in place of 6 they split a number into its
  digits: 47 / 10 is 4, the tens, and 47 % 10 is 7, the ones. That is how
  a display shows a number with more than one digit, and how Lesson 12's
  stopwatch finds its seconds and tenths.

## Upload it

Upload the sketch. The digit shows a dash: just segment g, waiting. Press
the button. A single bar runs round the rim three times, clockwise, slowing
as it goes, and the digit lands on a number from 1 to 6. Press again for
another roll.

Your prediction: `1 << 3` is `0b00001000`, bit 3, which lights segment d,
the bottom bar. As `step` counts up, the lit bit moves left, from a to b to
c and on round, so the bar travels clockwise: along the top, down the
right, along the bottom and up the left.

## If it doesn't work

| What you see | Try this |
|---|---|
| Nothing lights at all | Check the chip's notch is on the left, the red wires from j18 and j24 reach the top + rail, the black wires from j21 and a25 reach the top and bottom − rails, and the black jumper in column 6 joins the two − rails. |
| The chip gets warm | Unplug now. The chip is in backwards, or a 5 V wire is on a GND pin. |
| The numbers look scrambled | A resistor is in the wrong column, so a Q output lights the wrong segment. Check each one against the connections list. |
| One segment never lights | Its resistor or wire is loose, or one column out. |
| The dash shows, but the button does nothing | The wire from pin 22 goes in j2, the black wire from a4 to the − rail, and the button straddles the gap. |
| Segments flicker or light at random | Data, clock and latch are swapped: pin 37 to j20, 38 to j23, 39 to j22. |
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

1. **Light the dot.** Wire Q7 (column 24) through an eighth 1 kΩ resistor
   to the digit's decimal point (pin 5, column 50), then light bit 7 while
   the die spins: `digit.write ((1 << (step % 6)) | 0b10000000);`.
2. **Tumble.** Instead of a spinning bar, flash random faces that slow down,
   the way a real die bounces before it settles.
3. **Loaded die.** Make 6 come up twice as often as the other numbers. Is it
   still fair to call it a die?
4. **Let the library draw.** Swap `adk::ShiftRegister` for
   `adk::SevenSegment`, which is wired the same way and already knows the
   patterns: `digit.show (int (random (1, 7)));`. (`random ()` gives back
   a `long`, and `int (...)` makes it the `int` that `show ()` expects.)
   What can't it do that your own bytes could?
5. **Two dice.** Roll two dice, add them up, and show the total, 2 to 12,
   one digit at a time: the tens, `total / 10`, for half a second, then the
   ones, `total % 10`. Leave out the tens when they are 0. You'll need
   patterns for 0 and 7 to 9, or challenge 4's `adk::SevenSegment`.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. Here the meter turns a
byte back into what it really is on the chip: eight legs, each at 5 V or at
0 V.

Nothing needs to change in the sketch: the dash stays on until you press the
button, and each number the die lands on stays until the next roll. Put the
probe tips in the breadboard holes shown, not on the chip's legs, where a
tip could touch two legs at once and join two outputs together.

!!! question "Predict"
    Before you press the button, the digit shows its dash, the byte
    `0b01000000`. Which of the chip's outputs should be at 5 V? What will
    its neighbor Q5 read? And how much of the 5 V will g's resistor get?

<!-- measure -->

What the numbers tell you:

- **Q6** is at about 5 V: bit 6 is the only 1 in the byte, and the chip
  holds it on its Q6 leg for as long as the dash shows, with no help from
  the Mega.
- **Q5** reads 0 V, because bit 5 is a 0. Every 1 in the byte is an output
  at 5 V, and every 0 an output at 0 V. Roll a 4, `0b01100110`, and Q1,
  Q2, Q5 and Q6 read 5 V while the others read 0 V. The chip in the
  drawings is labeled, so you can find each output's column.
- **Across segment g's resistor** is about 3 V. The red segment keeps about
  2 V of the 5 V for itself, and the resistor takes the rest. By Ohm's law,
  3 V across 1 kΩ is 3 mA: the current worked out in the idea above.
