---
lesson: 7
promise: Turn a knob and watch an LED glide from dark to full brightness.
time: 45 minutes
level: 1
parts:
  - Arduino Mega 2560 and its USB cable
  - Breadboard
  - White LED
  - 220 Ω resistor (red, red, black, black, brown)
  - 10 kΩ potentiometer (the knob)
  - 7 jumper wires
ideas:
  - Analog input, read as a number from 0 to 1023
  - The potentiometer as a voltage divider
  - Scaling a reading to the range you need
  - Dimming with PWM, and the Serial Plotter
---

## What you'll build

<!-- closeup -->

A knob on your breadboard now controls a light. Turn it one way and a white
LED fades smoothly up to full brightness; turn it back and it sinks to dark.
Open the Serial Plotter and you can watch the numbers behind it rise and fall
as a live graph, like the needle of a machine.

## The idea

Until now every pin has been either on or off. That is **digital**. The pins
marked **A0** to **A15** can do something more: they measure a voltage
anywhere between 0 V and 5 V and turn it into a number, from 0 at 0 V up to
1023 at 5 V. That is **analog input**. Halfway, 2.5 V, reads about 512.

A **potentiometer** makes those voltages. Inside it is a curved strip of
resistance joining its two outer legs, and a contact called the **wiper**,
joined to the middle leg, that slides along the strip as you turn the knob.
Put 5 V on one outer leg and GND on the other, and the strip shares the 5 V
out along its length. The wiper picks off the voltage wherever it sits. This
is a **voltage divider**. With the wiper a quarter of the way from the GND
end, it sits at a quarter of 5 V:

<p class="formula">reading ≈ <span class="fraction"><span>1.25 V</span><span>5 V</span></span> × 1023 ≈ 256</p>

To dim the LED, the Mega uses **PWM**, as the Mood Lamp did in Lesson 4: the
pin switches fully on and off about 490 times a second, too fast to see, and
the share of time it spends on sets the brightness. `led.write (0)` is always
off, `led.write (255)` is always on, and `led.write (64)` is on a quarter of
the time. So the sketch needs to turn a reading of 0 to 1023 into a
brightness of 0 to 255. There are four readings for every brightness, so it
divides by 4. The Mega divides whole numbers into whole numbers and drops
the fraction: 1023 ÷ 4 is 255¾, which becomes 255, and a reading of 512
becomes 128.

!!! question "Predict"
    With the knob exactly halfway, the brightness is 128, so the LED is on
    half the time. Will it *look* half as bright as when the knob is turned
    all the way up, or brighter, or dimmer? Write down your guess, then try it.

## Build it

!!! warning "Unplug first"
    Unplug the USB cable before you change any wiring. The knob's **middle
    leg goes only to A0**. If it were joined to 5 V or GND as well, turning
    the knob to the end would connect 5 V straight to GND. If the knob ever
    feels warm, unplug at once and check its wires.

<!-- bench -->

<!-- steps -->

??? info "About the knob"
    The drawing shows the knob's three legs in three neighboring holes,
    e45 to e47. Some potentiometers have their legs a little further apart:
    if yours don't fit, put them in every other hole, starting at e45, and
    move each of the knob's three wires to its leg's new column. The middle
    leg is always the wiper.

    Which way is "up" depends only on which outer leg gets 5 V. If your LED
    brightens as you turn the knob counterclockwise and you'd rather it went
    clockwise, swap the outer legs' wires, so the left leg goes to the + rail
    and the right leg to the − rail. Nothing is wrong either way: you have
    just turned the voltage divider round. (The sketch can turn it round
    too, without moving a wire: see `knob.read (255, 0)` below.)

The white LED is built like Lesson 1's red one, in column 38, where the LED
that dims always goes: pin 3's wire comes into j38, the resistor stands
across the middle gap from g38 to e38, the LED's long leg shares column 38
in b38, and its short leg in b39 has a short black wire down to the − rail.
A white LED keeps about 3.2 V for itself, like the blue one, so through
220 Ω it takes about 8 mA at full brightness. Pin 3 is one of the Mega's pins that can do PWM.

The knob stands over the middle gap with its legs in e45, e46 and e47. A
black wire takes its left leg from a45 to the − rail, A0's wire goes into
a46 beside the middle leg, and a red wire takes its right leg from d47 up to
the top + rail, which the Mega's 5 V feeds at T+3. These are the knob's
own holes: Lesson 9 puts it back in exactly the same place.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → lessons → 007-dimmer**:

<!-- sketch -->

What's new:

- `adk::AnalogInput knob {A0};` says there is something to measure on pin
  A0. `adk::PwmOutput led {3};` is a pin that can dim. Only some pins can do
  PWM (on the Mega, 2 to 13 and 44 to 46); if you ask for one that can't,
  `adk::setup ()` stops and blinks its number, as in Lesson 4.
- `knob.read ()` gives the raw reading, 0 to 1023, and `reading / 4`
  turns it into a brightness, 0 to 255, ready for `led.write ()`. The
  sketch reads the knob once and keeps the reading, so the brightness and
  the number it prints always come from the same measurement.
- Dividing by 4 only works because 1024 is exactly 4 × 256. For any other
  range, `knob.read (low, high)` reads the knob and scales it in one go:
  `low` is what a reading of 0 becomes, and `high` what 1023 becomes. So
  `knob.read (0, 255)` gives much the same brightness as `reading / 4`,
  `knob.read (0, 180)` gives an angle, as the servo in Lesson 17 will
  want, and `knob.read (255, 0)` runs the other way round: 0 becomes 255,
  and 1023 becomes 0.
- `adk::println ()`, from Lesson 2, prints a line such as
  `knob:512 brightness:128`. The Serial Plotter reads each `name:number`
  pair and draws it as a line of its own.
- `adk::wait (20)` takes 50 readings a second: quick enough to follow your
  hand, and slow enough for the graph to scroll at a comfortable pace.

## Upload it

Plug in the Mega and upload the sketch as in Lesson 1. Turn the knob slowly
from one end to the other. The LED should glide from fully off to fully
bright, with no steps you can see. If it gets brighter the way you'd rather
it got dimmer, swap the knob's outer wires, as *About the knob* says, or
write `int brightness = knob.read (255, 0);` and upload again.

How did your prediction do? With the knob halfway, the LED looks much more
than half as bright: nearer three quarters, to most people. Your eyes notice
a change in dim light far more than the same change in bright light, so the
first few steps up from dark look big, and the last few before full look
small. The second challenge below makes the knob feel even.

Now choose **Tools → Serial Plotter** and set it to **9600 baud**. Two lines
scroll across: `knob`, between 0 and 1023, and `brightness`, between 0 and
255. Turn the knob and both move together, the brightness line always about
a quarter of the height of the knob line.

## If it doesn't work

| What you see | Try this |
|---|---|
| The LED never lights, wherever the knob is | Turn the LED round: its long leg goes in b38. Check the wire from pin 3 is in j38, the resistor really crosses the gap, from g38 to e38, and the black wire joins a39 to the − rail. |
| The LED flickers or changes by itself | A0 isn't reaching the wiper: its wire must be in a46, the middle leg's column. |
| The plotter's knob line sits at 0 or 1023 whatever you do | One outer leg has lost its supply. Check the Mega's red wire into T+3 and black wire into B-3, and the knob's two short wires: black from a45 to the − rail, red from d47 to the top + rail. |
| Full brightness comes at the "wrong" end | Nothing is wrong. If you'd like it the other way round, swap the outer legs' wires, as "About the knob" says, or use `knob.read (255, 0)` for the brightness. |
| The knob or a wire gets warm | Unplug now. The middle leg is joined to 5 V or GND; it must go only to A0. |
| The Serial Plotter shows nothing, or nonsense | Pick 9600 baud, and close the Serial Monitor: only one of them can use the port at a time. |

??? note "How it works"
    `knob.read ()` calls Arduino's `analogRead ()`. The Mega's analog to
    digital converter finds the voltage by guessing: is it above 2.5 V? Then
    above 3.75 V? Each answer halves what's left, and after ten answers it
    knows the voltage to within about 5 mV, one step in 1024. That takes about
    a tenth of a millisecond. `read (low, high)` then works out
    `low + (high − low) × reading ÷ 1023` in whole numbers.

    `adk::PwmOutput` claims pin 3 when the sketch starts and checks that it
    can do PWM. `write ()` tells the timer behind pin 3 how many of every 256
    ticks to keep the pin high, and the timer does the switching on its own,
    so your sketch never has to.

## Make it yours

1. **Never quite dark.** Make the lowest setting a faint glow instead of
   off, like a night light: change `reading / 4` to `knob.read (10, 255)`.
   What does the plotter's brightness line do now at the bottom of the
   knob?
2. **Test your prediction.** Make the knob feel even by squaring: change
   `int brightness` to `long brightness`, and write
   `led.write (brightness * brightness / 255);`. A `long` is a whole number
   with far more room than an `int`, which on the Mega stops at 32 767:
   255 × 255 is 65 025. Lesson 8 says more about `long`. Does halfway look
   like half now?
3. **Speed knob.** Use the knob to set a blink speed instead: turn the LED on
   with `led.write (255)`, wait `knob.read (50, 1000)` milliseconds, turn it
   off, and wait again.
4. **Warning light.** Add Lesson 1's red LED on pin 26 as an `adk::Led`, and
   light it only while the knob is past three quarters (`reading > 768`).

## Measure it

This part is for anyone with a multimeter; there isn't one in the kit. Set
it up as in [Lesson 1](../001-blink/index.md#measure-it): the dial on DC
volts (**V⎓**), the black lead in **COM** and the red one in **V**. Keep it
on DC volts: on a current setting the meter joins its two probes together,
and across 5 V and GND that is a short circuit.

The knob stays wherever you leave it, so nothing in the sketch needs to
change. Open the Serial Monitor at 9600 baud and turn the knob until it
shows `knob:256` or near it, a quarter of the way round from the end wired
to GND. Then leave it there.

!!! question "Predict"
    With the knob reading 256, what will the meter show between the wiper
    and GND? Pin 3 is only ever at 0 V or 5 V, switching about 490 times a
    second, far too fast for a meter to follow. What do you think the meter
    will show there?

<!-- measure -->

What the numbers tell you:

- **The knob's wiper** is the voltage A0 measures, and the sketch's number
  is that voltage counted in steps: 1.25 V × 1023 ÷ 5 ≈ 256. Turn the knob
  slowly and the meter and the Serial Monitor move together: any reading
  × 5 ÷ 1023 gives the meter's volts.
- **From 5 V down to the wiper** is the rest of the 5 V. Add it to the
  first reading and you get 5 V back: the knob's strip shares the 5 V out,
  a quarter below the wiper and three quarters above it. That is the
  voltage divider.
- **Pin 3, averaged**, reads about the same as the wiper. With the
  brightness at about 64 of 255, the pin is high a quarter of the time,
  and the meter, too slow to follow the switching, shows the average: a
  quarter of 5 V. Dividing the reading by 4 keeps the pin's average in
  step with the wiper: turn the knob to either end and pin 3 reads 0 V or
  the full 5 V, the same as the wiper.
