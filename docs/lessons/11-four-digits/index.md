---
lesson: 11
title: Four Digits
arc: Digits
promise: Light four digits by flashing them one at a time, faster than your eye can follow.
time: 1 hour
level: 2
sketch: Lesson11FourDigits
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 74HC595 shift register chip
  - Four-digit seven-segment display (5461AS)
  - 8 × 1 kΩ resistors (brown, black, black, brown, brown)
  - 30 jumper wires
ideas:
  - Multiplexing, one digit at a time
  - Persistence of vision
  - adk::Every, doing something on a steady beat
  - Showing numbers and words
---

## What you'll build

<!-- closeup -->

A proper four-digit display, like the one on a microwave or a scoreboard. It
says **HI**, then starts counting, ten numbers a second, the right-hand
digit a blur and the left-hand one creeping up. The trick behind it is one
of the neatest in electronics: only one digit is ever lit at a time.

## The idea

Four digits of eight segments each is thirty-two LEDs, but the display has
only twelve pins. Inside, every digit's a segment is joined to every other
digit's a segment, and the same for b, c and the rest: eight **segment
lines**, shared. Each digit also has its own **digit pin**, its common, which
the Mega pulls to GND to switch that digit on.

So the display can't show four different digits at once. Instead it shows
them **one at a time**: digit 1's pattern on the segment lines with digit 1
switched on, then, 2 milliseconds later, digit 2's pattern with digit 2 on,
and so on round all four, 125 times a second. This is called
**multiplexing**. Your eye can't follow flashes that fast, and blends them
into four steady digits. That's **persistence of vision**, the same reason
a film projector's flickering light looks steady.

The segment lines come from the 74HC595, as in Lesson 10, each through its
1 kΩ resistor, about 3 mA per lit segment. A digit pin carries the current
of all its lit segments together: an 8 with its dot is 8 × 3 = 24 mA, within
the 40 mA a Mega pin can take, and each digit is lit only a quarter of the
time.

Because the display must be refreshed every 2 milliseconds, the sketch must
never stop to wait. For things that happen on a beat, such as counting up
ten times a second, ADK has **`adk::Every`**: `tick.ticked ()` is true once
every 100 milliseconds, and the rest of the time `loop ()` just carries on.

!!! question "Predict"
    Suppose the display moved on to the next digit only every half second
    instead of every 2 milliseconds. What would you see? Write it down; you
    can try it at the end.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you start. The 74HC595 goes in with its
    **notch to the left**, and the display with its **decimal points at the
    bottom**. Press both in evenly, so no leg folds under. There are many
    wires: build one step at a time and tick each one off. If anything gets
    warm, unplug and check the chip.

<!-- bench -->

<!-- steps -->

??? info "The display's pins, and the crossing legs"
    The display's pin 1 is at the bottom left; pins 1 to 6 run along the
    bottom and 7 to 12 back along the top. The four digit pins are 12, 9, 8
    and 6, for digits 1 to 4. The other eight are the segment lines, which
    is why they are wired in the order they fall on the display, not the
    order of the chip's outputs:

    | Chip output | Q0 | Q1 | Q2 | Q3 | Q4 | Q5 | Q6 | Q7 |
    |---|---|---|---|---|---|---|---|---|
    | Segment | a | b | c | d | e | f | g | dot |
    | Display pin | 11 | 7 | 4 | 2 | 1 | 10 | 5 | 3 |

    The chip and the resistors for a, f and b stay where Lesson 10 had them;
    so do the wires that step up over the gap for e, d and c, and their
    resistors standing across the gap just before the display, now with a
    fourth beside them for the dot. g's resistor lies in row b. The lower
    legs of that row of four drop into the display's bottom pins in turn, so
    no two wires cross. The steps give every hole.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → Lesson11FourDigits**:

<!-- sketch -->

What's new:

- `adk::FourDigitDisplay display {37, 38, 39, 40, 41, 42, 43};` is the whole
  display: the 74HC595's data, clock and latch pins, then the pins for
  digits 1 to 4.
- `display.show ("HI")` shows text, starting from the left. The display can
  draw the digits and the letters that read clearly, such as A, b, C, d, E,
  F, H, L, n, o, P, r, t and U; any other character is left blank.
- `display.show (count)` shows a number, lined up on the right.
- `adk::Every tick {100};` beats every 100 milliseconds, and `tick.ticked ()`
  is true for one turn of `loop ()` on each beat. `count` goes up by one
  each time, and `% 10000` brings it back to 0 after 9999.

## Upload it

Upload the sketch. The display shows **HI** for a second and a half, then
counts: 1, 2, 3, and on, ten a second. The right-hand digit changes too
fast to read, the next one once a second, and the left-hand one only every
100 seconds. Look closely: all four digits seem steady and equally bright,
though each is dark three quarters of the time.

You predicted what a new digit every half second would look like. The trick
would give itself away: you would see one digit lit at a time, stepping from
left to right and round again, and never all four at once. The first
challenge below lets you watch it happen.

## If it doesn't work

| What you see | Try this |
|---|---|
| Nothing lights at all | Check the chip's notch is on the left and its supply: red wires from j18 and j24 to the top + rail, black from j21 to the top − rail and from a25 to the bottom − rail, and the black jumper in column 6 between the two − rails. |
| One digit stays dark | Its digit wire: pin 40 to j51 for digit 1, 41 to j54, 42 to j55, and 43 to a56. |
| The same segment is missing on every digit | That segment's resistor is loose or one column out: every digit shares it. |
| The numbers look scrambled | Two segment resistors are swapped. Check each against the table above. |
| Random flickering segments | Data, clock and latch are swapped: pin 37 to j20, 38 to j23, 39 to j22. |
| Only one digit lights at a time, flashing | Something in `loop ()` is stopping it. Use `adk::wait ()`, never `delay ()`. |

??? note "How it works"
    Every 2 milliseconds, inside `adk::update ()`, the display object turns
    off the lit digit by raising its pin, shifts the next digit's pattern
    into the 74HC595, and pulls that digit's pin low. Turning the old digit
    off first stops its pattern from flashing on the next digit.

    `display.show ()` only stores the four patterns; the refreshing all
    happens in `adk::update ()`. That is why the display needs `loop ()` to
    come round often, and why `adk::wait ()` keeps it lit while `delay ()`
    freezes it on one digit.

## Make it yours

1. **Test your prediction.** Add `delay (500);` at the end of `loop ()`.
   Arduino's `delay ()` stops everything, refreshing included, so you see
   multiplexing in slow motion. Then try `delay (10)`: where does it start
   to flicker?
2. **Your name.** Show your name, or a four-letter word made from the
   letters above, before the count starts.
3. **Countdown.** Count down from 100 instead, and show `End` at zero.
4. **Tenths.** Count in tenths of a second with a dot: text such as `"12.3"`
   lights the dot of the character before the `.`. Build the text with
   `snprintf ()`, as Lesson 12 does.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../01-blink/index.md#measure-it): DC volts (**V⎓**),
the black lead in **COM** and the red one in **V**. A meter is slow, like
your eye: it can't follow a digit that lights for 2 milliseconds at a time,
so it shows the average, and the average gives multiplexing away.

These readings don't depend on the number showing, so leave the sketch
counting. The digit pins' holes are close to the display's legs: keep each
probe tip in the hole shown, touching nothing else.

!!! question "Predict"
    A digit pin is at 0 V while its digit is lit, and at 5 V while it is
    dark. What will the meter show on digit 1's pin? Digit 1 stays blank
    until the count reaches 1000, while digit 4 always shows a number:
    will their pins read the same?

<!-- measure -->

What the numbers tell you:

- **Digit 1's pin** is at 5 V for three turns in every four, and at 0 V
  for the fourth, while its digit is lit. The meter blends the two, just
  as your eye blends the flashes: three quarters of 5 V is 3.75 V.
- **Digit 4's pin** reads about the same. Every digit gets its quarter of
  the time, 2 ms in every 8, whether it has anything to show or not.
- Now watch it in slow motion, slow enough for a meter: add
  `delay (2000);` at the end of `loop ()`, as in the first challenge, and
  keep the probes on digit 1's pin. It sits at 5 V for six seconds, then
  drops to 0 V for two while digit 1 has its turn. Take the delay out
  again when you're done.
