---
lesson: 11
promise: Light four digits by flashing them one at a time, faster than your eye can follow.
time: 1 hour
level: 2
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - 74HC595 shift register chip
  - Four-digit seven-segment display (5461AS)
  - 8 × 2 kΩ resistors (red, black, black, brown, brown)
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
into what looks like four steady digits. That's **persistence of vision**,
the same reason a film projector's flickering light looks steady. The price
is brightness: each digit is lit only a quarter of the time, so it looks
dimmer than it would lit all the time.

The segment lines come from the 74HC595, as in Lesson 10, each through its
2 kΩ resistor, about 1.5 mA per lit segment. A digit pin carries the current
of all its lit segments together: an 8 with its dot is about 8 × 1.5 = 12 mA.
That leaves room below the Mega pin’s recommended 20 mA even while a digit
is on. The four-digit display uses larger resistors than Lesson 10.

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

??? info "The display's pins, and how the wires reach them"
    The display's pin 1 is at the bottom left; pins 1 to 6 run along the
    bottom and 7 to 12 back along the top. The four digit pins are 12, 9, 8
    and 6, for digits 1 to 4. The other eight are the segment lines, which
    is why they are wired in the order they fall on the display, not the
    order of the chip's outputs:

    | Chip output | Q0 | Q1 | Q2 | Q3 | Q4 | Q5 | Q6 | Q7 |
    |---|---|---|---|---|---|---|---|---|
    | Segment | a | b | c | d | e | f | g | dot |
    | Display pin | 11 | 7 | 4 | 2 | 1 | 10 | 5 | 3 |

    The chip stays where Lesson 10 had it. Replace its 1 kΩ segment
    resistors with **2 kΩ** ones; the steps show which to take out and add.
    All but g's go back in the same holes. The wires that step up over the
    gap for e, d and c stay, with a fourth resistor beside theirs for the
    dot. The lower legs of that row of four drop into the display's bottom
    pins in turn, so no two wires cross. g's resistor moves from column 30
    to lie along row b. The steps give every hole.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → lessons → 011-four-digits**:

<!-- sketch -->

What's new:

- `adk::FourDigitDisplay display {37, 38, 39, 40, 41, 42, 43};` is the whole
  display: the 74HC595's data, clock and latch pins, then the pins for
  digits 1 to 4.
- `display.show ("HI")` shows text, starting from the left. Seven bars can't
  draw every letter. The display draws the digits; the letters A to F,
  with b and d always small, since B and D would look like 8 and 0; the
  letters that still read clearly, G, H, I, J, L, N, O, P, Q, R, S, T, U
  and Y; and `-`, `_` and a space. Letters work in either case, and a small
  h, o or u gets a small shape of its own. Any other character is left
  blank.
- `display.show (count)` shows a number, lined up on the right.
- `adk::Every tick {100};` beats every 100 milliseconds, and `tick.ticked ()`
  is true for one turn of `loop ()` on each beat. `count` goes up by one
  each time, and `% 10000` brings it back to 0 after 9999.

## Upload it

Upload the sketch. The display shows **HI** for a second and a half, then
counts: 1, 2, 3, and on, ten a second. The right-hand digit changes too
fast to read, the next one once a second, and the left-hand one only every
100 seconds. A digit stays blank until the count reaches it, so all four
are lit only after 100 seconds, at 1000. Look closely at the lit digits:
they should look steady, though each is dark three quarters of the time.
They are not bright. Each lit segment gets about 1.5 mA for a quarter of
the time, about 0.4 mA on average, so shade the display from a bright lamp
or window if it is hard to read.

You predicted what a new digit every half second would look like. The trick
would give itself away: you would see one digit lit at a time, stepping from
left to right and round again, and never all four at once. The first
challenge below lets you watch it happen.

## If it doesn't work

| What you see | Try this |
|---|---|
| Nothing lights at all | Check the chip's notch is on the left and its supply: red wires from j18 and j24 to the top + rail, black from j21 to the top − rail and from a25 to the bottom − rail, and the black jumper in column 41 between the two − rails. |
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
4. **Tenths.** Count in tenths of a second with a dot:
   `display.show (count, 1)` puts the last digit after the dot, so a count
   of 123 shows as `12.3`, as Lesson 12's stopwatch does.

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): DC volts (**V⎓**),
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

## Check yourself

1. The four digits share eight segment lines. How does the display still
   show four different numbers?
2. The same segment is missing on every digit. Where would you look first,
   and why?
3. Why does this sketch count with `adk::Every` instead of waiting with
   `delay (100)`?

??? note "Answers"
    1. It lights one digit at a time, with that digit's pattern on the
       segment lines, then the next, round all four many times a second.
       Your eye blends the flashes into what looks like four steady
       digits.
    2. At that segment's resistor and wire from the chip. Every digit shares
       the same segment line, so one loose resistor takes that segment out
       of all four.
    3. The display is refreshed inside `adk::update ()`, so `loop ()` must
       never stop, and `delay ()` would freeze it on one digit. With
       `adk::Every`, `tick.ticked ()` is true once every 100 ms and `loop ()`
       carries on the rest of the time.
