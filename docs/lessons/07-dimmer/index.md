---
lesson: 7
title: Dimmer
arc: The analog world
promise: Turn a knob and watch an LED glide from dark to full brightness.
time: 45 minutes
level: 1
sketch: Lesson07Dimmer
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
  - Scaling a reading with read (low, high)
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
brightness of 0 to 255, and `knob.read (0, 255)` does exactly that: it scales
the reading into the range you ask for. A reading of 512 becomes 127.

!!! question "Predict"
    With the knob exactly halfway, the brightness is 127, so the LED is on
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
    e8 to e10. Some potentiometers have their legs a little further apart:
    if yours don't fit, put them in every other hole (e8, e10, e12) and move
    the knob's three wires to the same columns. The middle leg is always the
    wiper.

    Which way is "up" depends only on which outer leg gets 5 V. If your LED
    brightens as you turn the knob counterclockwise and you'd rather it went
    clockwise, swap the two short wires on the outer legs. Nothing is wrong
    either way: you have just turned the voltage divider round.

This time the LED's resistor sits after the LED, on its short leg's side,
standing across the middle gap. The current is the same all the way round a
loop, so the resistor limits it wherever it is. The LED's legs go two holes
apart, f2 and f4, so it stands clear of the resistor: spread them gently. A
white LED keeps about 3 V for itself, so through 220 Ω it takes about 9 mA
at full brightness. Pin 3 is one of the Mega's pins that can do PWM.

When you are done, these are the connections your circuit makes:

<!-- connections -->

## Code it

Open the Arduino IDE and choose **File → Examples → Adk → Lesson07Dimmer**:

<!-- sketch -->

What's new:

- `adk::AnalogInput knob {A0};` says there is something to measure on pin
  A0. `adk::PwmOutput led {3};` is a pin that can dim. Only some pins can do
  PWM (on the Mega, 2 to 13 and 44 to 46); if you ask for one that can't,
  `adk::setup ()` stops and blinks its number, as you saw in Lesson 1.
- `Serial.begin (9600);` and `adk::setup (Serial);` are as in Lesson 2: if
  something is wrong, the Serial Monitor says what.
- `knob.read ()` gives the raw reading, 0 to 1023. `knob.read (0, 255)`
  gives the same reading scaled to 0 to 255, ready for `led.write ()`.
- `plot ()` prints a line such as `knob:512 brightness:127`. The Serial
  Plotter reads each `name:number` pair and draws it as a line of its own.
- `adk::wait (20)` takes 50 readings a second: quick enough to follow your
  hand, and slow enough for the graph to scroll at a comfortable pace.

## Upload it

Plug in the Mega and upload the sketch as in Lesson 1. Turn the knob slowly
from one end to the other. The LED should glide from fully off to fully
bright, with no steps you can see.

Now choose **Tools → Serial Plotter** and set it to **9600 baud**. Two lines
scroll across: `knob`, between 0 and 1023, and `brightness`, between 0 and
255. Turn the knob and both move together, the brightness line always about
a quarter of the height of the knob line.

## If it doesn't work

| What you see | Try this |
|---|---|
| The LED never lights, wherever the knob is | Turn the LED round: its long leg goes in f2. Check the wire from pin 3 is in j2, and the resistor really crosses the gap, from g4 to e4. |
| The LED flickers or changes by itself | A0 isn't reaching the wiper: its wire must be in a9, the middle leg's column. |
| The plotter's knob line sits at 0 or 1023 whatever you do | One outer leg has lost its supply. Check the red wire to B+3, the black one to B-3, and the two short wires on the knob's outer legs. |
| Full brightness comes at the "wrong" end | Nothing is wrong. Swap the wires in a8 and a10 if you'd like it the other way round. |
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

1. **Night light.** Make the LED brightest when the knob is at its lowest:
   write `255 - brightness` instead of `brightness`.
2. **Test your prediction.** You probably found that halfway looks brighter
   than half. Eyes notice changes in dim light far more than in bright light.
   Make the knob feel even by squaring: `long level = knob.read (0, 255);`
   then `led.write (level * level / 255);`. It needs a `long`, because
   255 × 255 is too big for an `int` on the Mega.
3. **Speed knob.** Use the knob to set a blink speed instead: turn the LED on
   with `led.write (255)`, wait `knob.read (50, 1000)` milliseconds, turn it
   off, and wait again.
4. **Warning light.** Add Lesson 1's red LED on pin 26 as an `adk::Led`, and
   light it only while the knob is past three quarters (`knob.read () > 768`).
